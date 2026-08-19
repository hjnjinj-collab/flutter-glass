# Windows plugin design & build notes

Quick notes for implementers:

1. Rendering backend choices
- ANGLE + D3D11: If Impeller on your Flutter build uses ANGLE, rendering against D3D11 is compatible. Use ID3D11Device/Context and create an ID3D11Texture2D as render target.
- Native D3D11: Use a native D3D11 device and create a shared texture if you need to interop with other processes or components.

2. Flutter texture registration
- Use flutter::TextureRegistrar (plugin registrar->texture_registrar()) to register a TextureVariant. Implement PixelBufferTexture or a custom Texture that drives frames by calling OnFrameAvailable.

3. Shader workflow
- Implement separable Gaussian blur (horizontal + vertical) in HLSL/GLSL, generate normal map (from blurred image) and do UV offset + chromatic aberration in a final pass.

4. Synchronization
- After rendering each frame into the texture, call texture_entry->OnFrameAvailable() so Flutter picks up the new frame.

5. Fallback for full Flutter scene capture
- If you need to capture Flutter's scene, two choices: (A) rely on engine-side capture (preferred), (B) use periodic toImage or platform-specific GPU buffer APIs with managed copying (not fully zero-copy).
