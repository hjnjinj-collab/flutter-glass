import 'dart:ui' as ui;
import 'package:flutter/material.dart';

import 'liquid_glass_painter.dart';

void main() {
  runApp(const GlassApp());
}

class GlassApp extends StatelessWidget {
  const GlassApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Impeller Liquid Glass',
      home: Scaffold(
        backgroundColor: Colors.black,
        body: Center(
          child: AspectRatio(
            aspectRatio: 16 / 9,
            child: CustomPaint(
              painter: LiquidGlassPainter(),
            ),
          ),
        ),
      ),
    );
  }
}
