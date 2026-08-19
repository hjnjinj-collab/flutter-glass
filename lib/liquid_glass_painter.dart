import 'dart:ui' as ui;
import 'package:flutter/material.dart';

class LiquidGlassPainter extends CustomPainter {
  late final ui.FragmentProgram _program;

  LiquidGlassPainter() {
    // 注意：实际运行时要改成 async 初始化，这里是示意
    ui.FragmentProgram.fromAsset('assets/shaders/liquid_glass.frag')
        .then((p) => _program = p);
  }

  @override
  void paint(Canvas canvas, Size size) {
    if (_program == null) {
      // 简单占位背景
      final bgPaint = Paint()..color = Colors.blueGrey.shade900;
      canvas.drawRect(Offset.zero & size, bgPaint);
      return;
    }

    final shader = _program.shader(
      floatUniforms: Float32List.fromList([
        size.width, size.height, // u_resolution
        0.06,                    // u_refractionStrength
        1.4,                     // u_sdfScale
        0.08,                    // u_edgeSoftness
        0.6,                     // u_glowStrength
      ]),
    );

    final paint = Paint()..shader = shader;
    canvas.drawRect(Offset.zero & size, paint);
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}
