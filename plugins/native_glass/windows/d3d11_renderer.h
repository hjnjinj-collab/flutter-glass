# D3D11 renderer helper (header)
#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <memory>

using Microsoft::WRL::ComPtr;

class D3D11Renderer {
 public:
  D3D11Renderer();
  ~D3D11Renderer();

  bool Initialize(int width, int height);
  void Resize(int width, int height);
  void Render(float time);

  // Try to copy GPU render target into a CPU-accessible buffer (RGBA8) for fallback
  // Returns true if succeeded and fills outPixels (size = width*height*4)
  bool CopyToCPU(std::vector<uint8_t>& outPixels);

 private:
  int width_ = 0;
  int height_ = 0;
  ComPtr<ID3D11Device> device_;
  ComPtr<ID3D11DeviceContext> context_;
  ComPtr<ID3D11Texture2D> render_target_tex_;
  ComPtr<ID3D11RenderTargetView> rtv_;
  ComPtr<ID3D11ShaderResourceView> srv_;
};
