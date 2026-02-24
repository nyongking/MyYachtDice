struct VS_IN
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
    uint instanceID : SV_InstanceID;
};

struct VS_OUT
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

cbuffer WorldViewBuffer : register(b0)
{
    row_major float4x4 viewMatrix;
    row_major float4x4 projMatrix;
    row_major float4x4 viewprojMatrix;
};


struct InstanceData
{
    matrix worldMatrix;
};

StructuredBuffer<InstanceData> InstanceDatas : register(t0);

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    
    // SV_InstanceID를 인덱스로 사용하여 해당 객체의 행렬을 가져옴
    matrix world = InstanceDatas[input.instanceID].worldMatrix;
    
    // VP 행렬은 여전히 cbuffer에서 가져오는 것이 효율적입니다.
    float4 worldPos = mul(float4(input.position, 1.0f), world);
    output.position = mul(worldPos, viewprojMatrix);
    output.texcoord = input.texcoord;
    
    return output;
    
   
}