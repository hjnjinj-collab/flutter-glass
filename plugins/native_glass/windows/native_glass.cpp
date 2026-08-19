// Update native_glass.cpp to try D3D11 path and fallback to PixelBufferTexture
#include "native_glass.h"
#include "d3d11_renderer.h"

#include <flutter/texture_registrar.h>
#include <flutter/texture_variant.h>
#include <flutter/pixel_buffer.h>
#include <flutter/pixel_buffer_texture.h>

#include <thread>
#include <chrono>
#include <iostream>

using flutter::EncodableMap;
using flutter::EncodableValue;
using flutter::EncodableList;
using flutter::PixelBuffer;
using flutter::PixelBufferTexture;

class NativeGlassTextureImpl {
 public:
  NativeGlassTextureImpl(flutter::TextureRegistrar* registrar, int width, int height)
      : registrar_(registrar), width_(width), height_(height) {
    // Try D3D11 renderer
    renderer_ = std::make_unique<D3D11Renderer>();
    bool ok = renderer_->Initialize(width_, height_);
    use_gpu_ = ok;

    if (!use_gpu_) {
      // Setup PixelBufferTexture fallback (CPU path)
      FlutterDesktopPixelBuffer buffer = {};
    }

    StartRenderLoop();
  }

  ~NativeGlassTextureImpl() { StopRenderLoop(); }

  int64_t Register() {
    // Register PixelBufferTexture which calls our callback to fill pixels
    PixelBufferTexture::CopyCallback copy_callback = [this](PixelBuffer& buffer) -> bool {
      // Fill pixel buffer (RGBA) either from GPU copy or CPU procedural
      if (use_gpu_) {
        std::vector<uint8_t> pixels;
        if (renderer_->CopyToCPU(pixels)) {
          // copy into buffer
          if (buffer.buffer) {
            memcpy(buffer.buffer, pixels.data(), pixels.size());
            buffer.width = width_;
            buffer.height = height_;
            buffer.format = PixelFormat::kRGBA8888;
            return true;
          }
        }
      }

      // CPU fallback: generate a simple gradient
      if (buffer.buffer) {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(buffer.buffer);
        for (int y = 0; y < height_; ++y) {
          for (int x = 0; x < width_; ++x) {
            int idx = (y * width_ + x) * 4;
            ptr[idx + 0] = static_cast<uint8_t>((x * 255) / width_); // R
            ptr[idx + 1] = static_cast<uint8_t>((y * 255) / height_); // G
            ptr[idx + 2] = 200; // B
            ptr[idx + 3] = 255; // A
          }
        }
        buffer.width = width_;
        buffer.height = height_;
        buffer.format = PixelFormat::kRGBA8888;
        return true;
      }
      return false;
    };

    pixel_texture_ = std::make_shared<PixelBufferTexture>(copy_callback);
    auto variant = std::make_unique<flutter::TextureVariant>(pixel_texture_);
    texture_id_ = registrar_->RegisterTexture(std::move(variant));
    return texture_id_;
  }

  void StartRenderLoop() {
    running_ = true;
    thread_ = std::thread([this]() {
      float t = 0.0f;
      while (running_) {
        auto start = std::chrono::high_resolution_clock::now();
        if (use_gpu_) renderer_->Render(t);
        // notify Flutter a frame is available
        registrar_->MarkTextureFrameAvailable(texture_id_);
        t += 0.016f;
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        (void)start;
      }
    });
  }

  void StopRenderLoop() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
  }

 private:
  flutter::TextureRegistrar* registrar_;
  int width_;
  int height_;
  std::unique_ptr<D3D11Renderer> renderer_;
  bool use_gpu_ = false;
  std::shared_ptr<PixelBufferTexture> pixel_texture_;
  int64_t texture_id_ = -1;
  std::thread thread_;
  bool running_ = false;
};

// Plugin wiring
void NativeGlassPlugin::RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar) {
  auto plugin = new NativeGlassPlugin(registrar);
  // intentionally leaked to match simple plugin lifetime
}

NativeGlassPlugin::NativeGlassPlugin(flutter::PluginRegistrarWindows *registrar)
    : registrar_(registrar) {
  auto channel = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
      registrar_->messenger(), "native_glass",
      &flutter::StandardMethodCodec::GetInstance());

  channel->SetMethodCallHandler(
      [this](const auto &call, auto result) { HandleMethodCall(call, std::move(result)); });

  channel_ = std::move(channel);

  auto texture_registrar = registrar_->texture_registrar();
  texture_impl_ = std::make_unique<NativeGlassTextureImpl>(texture_registrar, 800, 450);
  int64_t id = texture_impl_->Register();
  texture_id_ = id;
}

NativeGlassPlugin::~NativeGlassPlugin() {
  if (texture_impl_) texture_impl_.reset();
}

void NativeGlassPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  const std::string method_name = call.method_name();
  if (method_name == "getTextureId") {
    result->Success(flutter::EncodableValue(static_cast<int64_t>(texture_id_)));
  } else if (method_name == "start") {
    // TODO: manage start/stop
    result->Success();
  } else if (method_name == "stop") {
    result->Success();
  } else {
    result->NotImplemented();
  }
}
