#include "d3d11_renderer.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <stdexcept>
#include <vector>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

D3D11Renderer::D3D11Renderer() {}

D3D11Renderer::~D3D11Renderer() {
  // Release resources
}

static const char* kVertexShaderSrc = R"(
struct VSOut { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };
VSOut main(uint vid : SV_VertexID) {
  VSOut o;
  float2 pos[3] = { float2(-1.0, -1.0), float2(-1.0, 3.0), float2(3.0, -1.0) };
  float2 p = pos[vid];
  o.pos = float4(p, 0.0, 1.0);
  o.uv = (p + 1.0) * 0.5;
  return o;
}
)";

static const char* kPixelShaderSrc = R"(
cbuffer Params : register(b0) {
    float2 u_resolution;
    float u_refractionStrength;
    float u_sdfScale;
    float u_edgeSoftness;
    float u_glowStrength;
    float u_time;
};

struct VSOut { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };

float4 main(VSOut input) : SV_TARGET {
    float2 uv = input.uv;
    // generate a procedural scene
    float2 center = float2(0.5, 0.5);
    float2 d = uv - center;
    float dist = length(d);

    // procedural rings
    float rings = 0.5 + 0.5 * sin(dist * 40.0 - u_time * 2.0);
    float3 base = lerp(float3(0.2,0.3,0.6), float3(0.8,0.9,0.7), rings);

    // fake normal using sin waves for demo
    float2 n = float2(sin(uv.y * 10.0 + u_time) * 0.02, cos(uv.x * 10.0 + u_time) * 0.02);
    float2 ruv = uv + n * u_refractionStrength;

    // sample the procedural scene using refracted uv
    float2 d2 = ruv - center;
    float dist2 = length(d2);
    float rings2 = 0.5 + 0.5 * sin(dist2 * 40.0 - u_time * 2.0);
    float3 scene = lerp(float3(0.2,0.3,0.6), float3(0.8,0.9,0.7), rings2);

    // edge/alpha
    float edge = smoothstep(0.5, 0.45, dist);
    float glow = smoothstep(0.4, 0.3, dist) * u_glowStrength;

    float3 finalColor = scene + glow * float3(1.0, 0.9, 0.8);
    float alpha = edge;
    return float4(finalColor, alpha);
}
)";

bool D3D11Renderer::Initialize(int width, int height) {
  width_ = width;
  height_ = height;

  UINT createDeviceFlags = 0;
#ifdef _DEBUG
  createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};
  D3D_FEATURE_LEVEL featureLevel;
  HRESULT hr = D3D11CreateDevice(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevels,
      ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &device_, &featureLevel, &context_);
  if (FAILED(hr)) return false;

  // Create render target texture
  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = width_;
  desc.Height = height_;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

  hr = device_->CreateTexture2D(&desc, nullptr, &render_target_tex_);
  if (FAILED(hr)) return false;

  hr = device_->CreateRenderTargetView(render_target_tex_.Get(), nullptr, &rtv_);
  if (FAILED(hr)) return false;

  hr = device_->CreateShaderResourceView(render_target_tex_.Get(), nullptr, &srv_);
  if (FAILED(hr)) return false;

  // Compile shaders
  ComPtr<ID3DBlob> vsBlob;
  ComPtr<ID3DBlob> psBlob;
  ComPtr<ID3DBlob> errorBlob;

  hr = D3DCompile(kVertexShaderSrc, strlen(kVertexShaderSrc), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
  if (FAILED(hr)) {
    if (errorBlob) {
      OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
    }
    return false;
  }

  hr = D3DCompile(kPixelShaderSrc, strlen(kPixelShaderSrc), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, &errorBlob);
  if (FAILED(hr)) {
    if (errorBlob) {
      OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
    }
    return false;
  }

  hr = device_->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertex_shader_);
  if (FAILED(hr)) return false;

  hr = device_->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixel_shader_);
  if (FAILED(hr)) return false;

  // Input layout is not needed when using SV_VertexID vertex shader, but create a dummy one to be safe
  D3D11_INPUT_ELEMENT_DESC layoutDesc[] = { {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0} };
  hr = device_->CreateInputLayout(layoutDesc, 1, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &input_layout_);
  if (FAILED(hr)) {
    // Not fatal
    input_layout_.Reset();
  }

  // Create constant buffer
  D3D11_BUFFER_DESC cbd = {};
  cbd.Usage = D3D11_USAGE_DEFAULT;
  cbd.ByteWidth = sizeof(float) * 8; // u_resolution(2) + 4 params + time
  cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbd.CPUAccessFlags = 0;
  hr = device_->CreateBuffer(&cbd, nullptr, &constant_buffer_);
  if (FAILED(hr)) return false;

  // Sampler
  D3D11_SAMPLER_DESC sampDesc = {};
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  hr = device_->CreateSamplerState(&sampDesc, &sampler_);
  if (FAILED(hr)) return false;

  return true;
}

void D3D11Renderer::Resize(int width, int height) {
  if (width == width_ && height == height_) return;
  render_target_tex_.Reset();
  rtv_.Reset();
  srv_.Reset();
  Initialize(width, height);
}

void D3D11Renderer::Render(float time) {
  if (!context_ || !rtv_) return;

  // Bind render target
  context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), nullptr);

  // Viewport
  D3D11_VIEWPORT vp;
  vp.Width = static_cast<FLOAT>(width_);
  vp.Height = static_cast<FLOAT>(height_);
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  context_->RSSetViewports(1, &vp);

  // Clear
  float clearColor[4] = {0.08f, 0.12f, 0.18f, 1.0f};
  context_->ClearRenderTargetView(rtv_.Get(), clearColor);

  // Update constant buffer: u_resolution + params + time
  float cbData[8];
  cbData[0] = static_cast<float>(width_);
  cbData[1] = static_cast<float>(height_);
  cbData[2] = 0.06f; // refractionStrength
  cbData[3] = 1.4f; // sdfScale
  cbData[4] = 0.08f; // edgeSoftness
  cbData[5] = 0.6f; // glow
  cbData[6] = time; // time
  // cbData[7] unused
  context_->UpdateSubresource(constant_buffer_.Get(), 0, nullptr, cbData, 0, 0);

  // IA (no vertex buffer) using SV_VertexID
  if (input_layout_) context_->IASetInputLayout(input_layout_.Get());
  context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  // Set shaders
  context_->VSSetShader(vertex_shader_.Get(), nullptr, 0);
  context_->PSSetShader(pixel_shader_.Get(), nullptr, 0);
  context_->PSSetConstantBuffers(0, 1, constant_buffer_.GetAddressOf());
  context_->PSSetSamplers(0, 1, sampler_.GetAddressOf());

  // Draw full-screen triangle
  context_->Draw(3, 0);
}

bool D3D11Renderer::CopyToCPU(std::vector<uint8_t>& outPixels) {
  if (!device_ || !context_ || !render_target_tex_) return false;

  D3D11_TEXTURE2D_DESC desc = {};
  render_target_tex_->GetDesc(&desc);

  // Create a staging texture CPU-readable
  D3D11_TEXTURE2D_DESC stagingDesc = desc;
  stagingDesc.Usage = D3D11_USAGE_STAGING;
  stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  stagingDesc.BindFlags = 0;
  stagingDesc.MiscFlags = 0;

  ComPtr<ID3D11Texture2D> stagingTex;
  HRESULT hr = device_->CreateTexture2D(&stagingDesc, nullptr, &stagingTex);
  if (FAILED(hr)) return false;

  context_->CopyResource(stagingTex.Get(), render_target_tex_.Get());

  D3D11_MAPPED_SUBRESOURCE mapped = {};
  hr = context_->Map(stagingTex.Get(), 0, D3D11_MAP_READ, 0, &mapped);
  if (FAILED(hr)) return false;

  int rowPitch = mapped.RowPitch;
  outPixels.resize(width_ * height_ * 4);
  uint8_t* src = reinterpret_cast<uint8_t*>(mapped.pData);
  for (int y = 0; y < height_; ++y) {
    memcpy(&outPixels[y * width_ * 4], src + y * rowPitch, width_ * 4);
  }

  context_->Unmap(stagingTex.Get(), 0);
  return true;
}
