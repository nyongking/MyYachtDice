// G-Buffer 입력
Texture2D    gAlbedo   : register(t0);  // RGB: Albedo,   A: Metallic
Texture2D    gNormal   : register(t1);  // RGB: Normal,   A: Roughness
Texture2D    gWorldPos : register(t2);  // RGB: WorldPos, A: unused

SamplerState gSampler  : register(s1);  // linear clamp

#define MAX_LIGHTS 64

struct LightData
{
    uint   type;        // 0=Directional, 1=Point, 2=Spot
    float3 color;
    float3 position;
    float  range;
    float3 direction;
    float  intensity;
    float  spotInner;   // cos(inner half-angle)
    float  spotOuter;   // cos(outer half-angle)
    float2 _pad;
};

cbuffer LightBuffer : register(b0)
{
    int      g_lightCount;
    float3   _cbPad;
    LightData g_lights[MAX_LIGHTS];
};

struct PS_IN
{
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};

float4 main(PS_IN input) : SV_TARGET0
{
    float3 albedo   = gAlbedo.Sample(gSampler, input.uv).rgb;
    float3 normal   = normalize(gNormal.Sample(gSampler, input.uv).xyz);
    float3 worldPos = gWorldPos.Sample(gSampler, input.uv).xyz;

    float3 result = albedo * 0.1f;  // ambient

    for (int i = 0; i < g_lightCount; ++i)
    {
        LightData light = g_lights[i];

        if (light.type == 0)  // Directional
        {
            float3 L     = normalize(-light.direction);
            float  NdotL = max(dot(normal, L), 0.f);
            result += albedo * light.color * light.intensity * NdotL;
        }
        else if (light.type == 1)  // Point
        {
            float3 toLight = light.position - worldPos;
            float  dist    = length(toLight);
            if (dist < light.range)
            {
                float3 L     = toLight / dist;
                float  NdotL = max(dot(normal, L), 0.f);
                float  atten = 1.f - saturate(dist / light.range);
                result += albedo * light.color * light.intensity * NdotL * atten;
            }
        }
        else if (light.type == 2)  // Spot
        {
            float3 toLight = light.position - worldPos;
            float  dist    = length(toLight);
            if (dist < light.range)
            {
                float3 L          = toLight / dist;
                float  NdotL      = max(dot(normal, L), 0.f);
                float  atten      = 1.f - saturate(dist / light.range);
                float  spotDot    = dot(-L, normalize(light.direction));
                float  spotFactor = smoothstep(light.spotOuter, light.spotInner, spotDot);
                result += albedo * light.color * light.intensity * NdotL * atten * spotFactor;
            }
        }
    }
    
    return float4(result, 1.f);
}
