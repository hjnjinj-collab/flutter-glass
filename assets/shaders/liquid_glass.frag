#version 450

layout(set = 0, binding = 0) uniform sampler2D u_scene;

layout(set = 0, binding = 1) uniform GlassParams {
    vec2  u_resolution;
    float u_refractionStrength;
    float u_sdfScale;
    float u_edgeSoftness;
    float u_glowStrength;
} u_params;

layout(location = 0) in vec2 v_texcoord;
layout(location = 0) out vec4 fragColor;

// 简化版 squircle SDF，可根据原仓库微调
float squircleSDF(vec2 p) {
    float n = 4.0;
    float r = 0.5;
    float x = abs(p.x);
    float y = abs(p.y);
    float d = pow(x, n) + pow(y, n);
    return pow(d, 1.0 / n) - r;
}

vec2 sdfNormal(vec2 p) {
    float eps = 0.002;
    float dx = squircleSDF(p + vec2(eps, 0.0)) - squircleSDF(p - vec2(eps, 0.0));
    float dy = squircleSDF(p + vec2(0.0, eps)) - squircleSDF(p - vec2(0.0, eps));
    return normalize(vec2(dx, dy));
}

void main() {
    vec2 p = (v_texcoord - 0.5) * u_params.u_sdfScale;
    float dist = squircleSDF(p);

    if (dist > 0.0) {
        discard;
    }

    vec2 n = sdfNormal(p);
    vec2 refractedUV = v_texcoord + n * u_params.u_refractionStrength;
    refractedUV = clamp(refractedUV, vec2(0.0), vec2(1.0));

    // 暂时用一个简单的渐变背景代替 u_scene
    // 真正接场景纹理时，改成：texture(u_scene, refractedUV)
    vec3 sceneColor = vec3(refractedUV.x, refractedUV.y, 0.8);

    float edge = smoothstep(0.0, -u_params.u_edgeSoftness, dist);
    float glow = smoothstep(0.02, 0.0, dist) * u_params.u_glowStrength;
    vec3 glowColor = vec3(1.0);

    vec3 finalColor = sceneColor + glow * glowColor;
    float alpha = edge;

    fragColor = vec4(finalColor, alpha);
}
