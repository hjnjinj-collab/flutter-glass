# Windows-focused design for GPU-native liquid glass

This document describes a Windows-first plan to implement a GPU-only liquid glass effect for Flutter + Impeller without modifying the upstream engine where possible, and with an engine-side patch plan when full zero-copy sampling of arbitrary Flutter scene content is required.

Goals
- Provide a high-quality GPU-only liquid glass effect on Windows (desktop).  
- Prefer solutions that avoid forking the engine for quick delivery, but prepare an engine patch/plan for full zero-copy capture of the Flutter scene when necessary.  

Overview (two-track strategy)
1. Plugin-first (short-term, no engine change):
   - Provide a native Windows plugin that renders the liquid glass effect entirely on the native side (Direct3D11 via ANGLE / or D3D backend) to an offscreen GPU texture.
   - Expose the rendered texture to Dart using Flutter's TextureRegistry (Texture widget).  
   - This approach is zero-copy inside the native/GPU domain for the plugin-rendered content, but the plugin will not automatically get the full Flutter-composited scene texture unless the background is either natively renderable or provided as a GPU buffer by Flutter (which normally requires engine hooks).

2. Engine patch (long-term, required for full zero-copy of arbitrary Flutter UI):
   - Implement a compositor hook / BackdropGroup capture path on Windows (Impeller-D3D via ANGLE) to render a chosen layer/subtree to an internal ID3D11Texture2D in the Impeller compositor and bind that texture as the sampler for a later shader pass.
   - Expose a small Dart API or Layer (e.g. OffscreenBackdrop/LiquidGlassScope) that requests engine to capture the subtree into a GPU texture and run a custom fragment pass using that texture as input.

Why Windows is special
- On mobile (Vulkan/Metal) Impeller + platform often provide native-backed capture paths (liquid_glass_widgets uses those).  
- On Windows, Impeller typically runs through ANGLE/D3D and Flutter's default public APIs do not expose arbitrary scene textures to FragmentProgram.  
- To achieve per-frame zero-copy capture of the Flutter scene on Windows we need either an engine compositor hook or acceptance of a plugin-level compromise (native background or periodic GPU buffer transfer).

Recommended immediate deliverables
- A Windows plugin skeleton that:  
  - registers a Flutter texture and provides a C++ render loop placeholder for future shader code (D3D11 via ANGLE or native D3D if available),  
  - supports params from Dart (refractionStrength, sdfScale, edgeSoftness, glowStrength, downsample),  
  - demonstrates showing the native texture via Texture widget in the Flutter example.
- A design doc (this file) plus a detailed engine patch plan (patch stub) showing where the compositor should capture to an ID3D11Texture2D and how to bind it for a shader pass.

Next steps if you approve
- I will add plugin skeleton files under plugins/native_glass/windows/ and docs/ and patches/ in the repo main branch.  
- After that I can (A) implement a prototype native renderer (D3D11) that renders a placeholder background and runs a simplified refraction shader, or (B) produce a full engine patch ready for local testing.

Please confirm whether to:  
- create the Windows plugin skeleton + design doc + patch stub now (I will commit to main), or  
- create the plugin plus also implement a working D3D11 prototype now.
