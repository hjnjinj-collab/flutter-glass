#pragma once

#include <memory>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/texture_registrar.h>
#include <flutter/standard_method_codec.h>

#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

// Simple config
struct GlassParams {
  float refraction_strength = 0.06f;
  float sdf_scale = 1.4f;
  float edge_softness = 0.08f;
  float glow_strength = 0.6f;
  float downsample = 0.5f;
};

class NativeGlassTexture; // forward

class NativeGlassPlugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

 private:
  explicit NativeGlassPlugin(flutter::PluginRegistrarWindows *registrar);
  ~NativeGlassPlugin();

  // Method channel handlers
  void HandleMethodCall(const flutter::MethodCall<flutter::EncodableValue> &call,
                        std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  flutter::PluginRegistrarWindows *registrar_;
  std::unique_ptr<NativeGlassTexture> texture_entry_;
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;
};
