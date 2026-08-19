// native_glass.cpp - skeleton plugin for Windows
#include <flutter/plugin_registrar_windows.h>
#include <flutter/texture_registrar.h>
#include <windows.h>

// This file is a skeleton demonstrating how to register a texture and provide a place
// to run a native GPU render loop that draws into a GPU texture and notifies Flutter.

// NOTE: Implementation is platform-specific and currently left as TODO for render logic.

namespace native_glass {

class NativeGlassPlugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar) {
    // TODO: create texture entry & keep handle
    // auto* texture_registrar = registrar->texture_registrar();
    // texture_entry_ = texture_registrar->RegisterTexture(std::move(texture));
    // Save registrar for future calls
  }

  NativeGlassPlugin() {}
  ~NativeGlassPlugin() {}

  // TODO: lifecycle: start render thread, render to ID3D11Texture2D, call texture_entry->OnFrameAvailable()
};

}  // namespace native_glass

// Extern C registration
extern "C" __declspec(dllexport) void NativeGlassPluginRegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar) {
  native_glass::NativeGlassPlugin::RegisterWithRegistrar(registrar);
}
