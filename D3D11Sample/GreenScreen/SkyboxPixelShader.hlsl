TextureCube txDiffuse : register(t0);

SamplerState sample_state : register(s0);

struct OutputVertex
{
    float4 position : SV_POSITION;
    float4 tangent : TANGENT;
    float4 binormal : BINORMAL;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
    float4 worldPosition : WORLDPOSITION;
    float4 localPosition : LOCALPOSITION;
};

cbuffer Matrix_ConstantBuffer : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
    float4 cameraPosition;
};

cbuffer Light_ConstantBuffer : register(b1)
{
    // [0] = directional lighting, [1] = point lighting, [2] = spot lighting
    float4 lightPos[3];
    float4 lightNor[3];
    float4 lightColor[3];
    // x = radius, y = rotation, z = time, w = wavy toggle
    float4 lightRadius;
    // x = inner cone ratio, y = outer cone ratio, z = black and white toggle
    float4 coneRatio;
}

float4 main(OutputVertex inputPixel) : SV_TARGET
{
    float4 outputColor = txDiffuse.Sample(sample_state, (float3) inputPixel.localPosition);
    return outputColor;
}
