cbuffer AlbedoColor : register(b0) { float4 g_albedoColor; };

Texture2D    gAlbedoTex : register(t0);
SamplerState gSampler   : register(s0);

struct PS_IN
{
    float4 pos      : SV_Position;
    float3 worldPos : TEXCOORD0;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD1;
};

struct PS_OUT
{
    float4 albedo   : SV_TARGET0;  // RGB: Albedo,   A: Metallic
    float4 normal   : SV_TARGET1;  // RGB: Normal,   A: Roughness
    float4 worldPos : SV_TARGET2;  // RGB: WorldPos, A: unused
};

PS_OUT main(PS_IN input)
{
    PS_OUT o;

    float4 texColor = gAlbedoTex.Sample(gSampler, input.uv);
    float4 albedo   = lerp(g_albedoColor, texColor, texColor.a);  // 텍스처 없으면 albedoColor 사용

    o.albedo   = float4(albedo.rgb, 0.f);                    // Metallic  = 0
    o.normal   = float4(normalize(input.normal), 0.f);       // Roughness = 0
    o.worldPos = float4(input.worldPos, 1.f);

    return o;
}
