// Liquid glass GLSL adapted for Flutter/Impeller and sampler input
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

// Improved squircle SDF closer to common implementations
float squircleSDF(vec2 p) {
    // p is expected in -0.5..0.5 range scaled by u_sdfScale
    float rx = abs(p.x);
    float ry = abs(p.y);
    // blend between circle (n=2) and squircle (n=4)
    float n = 4.0;
    float r = 0.5;
    float d = pow(pow(rx, n) + pow(ry, n), 1.0 / n) - r;
    return d;
}

// Numerical normal for SDF
vec2 sdfNormal(vec2 p) {
    float eps = 0.001 * u_params.u_sdfScale;
    float dx = squircleSDF(p + vec2(eps, 0.0)) - squircleSDF(p - vec2(eps, 0.0));
    float dy = squircleSDF(p + vec2(0.0, eps)) - squircleSDF(p - vec2(0.0, eps));
    return normalize(vec2(dx, dy));
}

void main() {
    vec2 uv = v_texcoord;
    vec2 p = (uv - 0.5) * u_params.u_sdfScale;
    float dist = squircleSDF(p);

    // Outside the glass shape -> discard alpha 0
    if (dist > 0.0) {
        discard;
    }

    vec2 n = sdfNormal(p);

    // refraction offset in UV space — simple screen-space approximation
    vec2 refractedUV = uv + n * u_params.u_refractionStrength;
    refractedUV = clamp(refractedUV, vec2(0.0), vec2(1.0));

    vec3 sceneColor = vec3(refractedUV.x, refractedUV.y, 0.8);
    #ifdef HAS_U_SCENE
    // When a sampler is bound, sample the provided scene texture
    sceneColor = texture(u_scene, refractedUV).rgb;
    #endif

    float edge = smoothstep(0.0, -u_params.u_edgeSoftness, dist);
    float glow = smoothstep(0.02, 0.0, dist) * u_params.u_glowStrength;
    vec3 glowColor = vec3(1.0);

    vec3 finalColor = sceneColor + glow * glowColor;
    float alpha = edge;

    fragColor = vec4(finalColor, alpha);
}
