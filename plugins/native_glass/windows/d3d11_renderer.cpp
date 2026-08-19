#include "d3d11_renderer.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <stdexcept>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

D3D11Renderer::D3D11Renderer() {}

D3D11Renderer::~D3D11Renderer() {}

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

  float clearColor[4] = {0.08f, 0.12f, 0.18f, 1.0f};
  context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), nullptr);
  context_->ClearRenderTargetView(rtv_.Get(), clearColor);

  // Simple procedural coloring to simulate a background. Real implementation will run HLSL passes.
  // For now we just clear; adding full shader pipeline would require compiling HLSL and drawing a full-screen quad.
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
