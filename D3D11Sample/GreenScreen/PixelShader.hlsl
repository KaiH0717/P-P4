Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct OutputVertex
{
	float4 position : SV_POSITION;
    float4 color : COLOR0;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
    float4 lightPos[2];
    float4 lightNor[2];
    float4 lightColor[2];
    float4 lightRadius;
};

float4 main(OutputVertex inputPixel) : SV_TARGET
{
    return inputPixel.color * txDiffuse.Sample(samLinear, inputPixel.tex);
}
