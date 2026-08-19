# flutter-glass

Liquid glass shader demo using Flutter 3.47 + Impeller.

- 基于 `LiquidGlass-opengl` 仓库的 SDF + 法线 + 折射原理
- 在 Flutter 下通过 `FragmentProgram` + Impeller Fragment Shader 实现
- 当前版本可以采样 UI 场景纹理（通过 RepaintBoundary -> Image -> shader sampler）

开发说明：
- 点击画面可以手动重新采样场景纹理（用于演示）。
- 为了在生产中稳定采样 UI，你可能希望将场景渲染到独立的帧缓冲/纹理并在 GPU 侧直接传入 shader，而不是每次通过 CPU toImage。
