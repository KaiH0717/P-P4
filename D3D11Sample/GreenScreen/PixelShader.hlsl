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
    float4 lightPos[3];
    float4 lightNor[3];
    float4 lightColor[3];
    float4 lightRadius;
};

float4 main(OutputVertex inputPixel) : SV_TARGET
{
    //return float4(inputPixel.normal, 1.0f);
    inputPixel.tex.x += sin(inputPixel.tex.y * 0.1f + lightRadius.z) * lightRadius.w;
    inputPixel.tex.y += cos(inputPixel.tex.x * 0.1f + lightRadius.z) * lightRadius.w;
    return inputPixel.color * txDiffuse.Sample(samLinear, inputPixel.tex);
}
