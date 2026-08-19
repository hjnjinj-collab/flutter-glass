// Simple HLSL shader for placeholder liquid glass effect
// This is a simplified example; real implementation should implement multi-pass blur and refraction.

cbuffer Params : register(b0) {
    float2 u_resolution;
    float u_refractionStrength;
    float u_sdfScale;
    float u_edgeSoftness;
    float u_glowStrength;
};

Texture2D sceneTex : register(t0);
SamplerState sceneSampler : register(s0);

struct VS_OUT {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

float4 mainPS(VS_OUT input) : SV_TARGET {
    float2 uv = input.uv;
    // Simple fake refraction: offset uv by a sin-based normal
    float2 n = float2(sin(uv.y * 20.0) * 0.02, cos(uv.x * 20.0) * 0.02);
    float2 ruv = uv + n * u_refractionStrength;
    float3 col = sceneTex.Sample(sceneSampler, ruv).rgb;
    return float4(col, 1.0);
}
