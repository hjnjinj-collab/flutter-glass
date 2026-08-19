#include "native_glass.h"
#include <thread>
#include <chrono>
#include <iostream>

using flutter::EncodableMap;
using flutter::EncodableValue;
using flutter::EncodableList;

class NativeGlassTexture : public flutter::Texture {
 public:
  NativeGlassTexture(flutter::TextureRegistrar *registrar)
      : registrar_(registrar), width_(800), height_(450), texture_id_(0) {
    // Placeholder: in a full implementation we'd create a GPU-backed texture and manage frames
  }

  ~NativeGlassTexture() override = default;

  // Returns a handle that Flutter can use. On Windows desktop, Flutter will use the
  // texture to composite native content. Implementation depends on the Flutter engine's
  // desktop embedding API; here we leave placeholders.
  std::unique_ptr<flutter::TextureVariant> GetTextureVariant() override {
    // TODO: Return a valid TextureVariant (e.g., PixelBufferTexture or ExternalTexture)
    return nullptr;
  }

  // Start a render thread that updates the native texture and notifies Flutter via
  // registrar_->MarkTextureFrameAvailable or equivalent.
  void StartRenderLoop() {
    running_ = true;
    thread_ = std::thread([this]() {
      while (running_) {
        // TODO: Render into GPU texture (D3D11) using shaders (liquid glass pass)
        // After rendering, notify Flutter that a new frame is available.
        // Example: registrar_->MarkTextureFrameAvailable(texture_id_);

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60fps
      }
    });
  }

  void StopRenderLoop() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
  }

 private:
  flutter::TextureRegistrar *registrar_;
  int width_;
  int height_;
  int64_t texture_id_;
  std::thread thread_;
  bool running_ = false;
};

void NativeGlassPlugin::RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar) {
  auto plugin = std::make_unique<NativeGlassPlugin>(registrar);
  // The plugin instance intentionally leaked to match typical Flutter plugin patterns
  // where the plugin lifetime == application lifetime.
  plugin.release();
}

NativeGlassPlugin::NativeGlassPlugin(flutter::PluginRegistrarWindows *registrar)
    : registrar_(registrar) {
  auto channel = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
      registrar_->messenger(), "native_glass",
      &flutter::StandardMethodCodec::GetInstance());

  channel->SetMethodCallHandler(
      [this](const auto &call, auto result) { HandleMethodCall(call, std::move(result)); });

  channel_ = std::move(channel);

  // Create texture entry
  auto texture_registrar = registrar_->texture_registrar();
  texture_entry_ = std::make_unique<NativeGlassTexture>(texture_registrar);
  // TODO: Register the texture with the texture registrar and save returned id
  // Example (pseudo): texture_id = texture_registrar->RegisterTexture(texture_entry->GetTextureVariant());

  // Start rendering loop
  // texture_entry_->StartRenderLoop();
}

NativeGlassPlugin::~NativeGlassPlugin() {
  if (texture_entry_) texture_entry_->StopRenderLoop();
}

void NativeGlassPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  const std::string method_name = call.method_name();
  if (method_name == "getTextureId") {
    // TODO: return the texture id assigned when registering the texture
    // return 0 as placeholder
    result->Success(flutter::EncodableValue(0));
  } else if (method_name == "start") {
    // Start rendering (if not already)
    // texture_entry_->StartRenderLoop();
    result->Success();
  } else if (method_name == "stop") {
    // texture_entry_->StopRenderLoop();
    result->Success();
  } else {
    result->NotImplemented();
  }
}
