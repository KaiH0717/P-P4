Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct OutputVertex
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 normal : NORMAL;
};

float4 main(OutputVertex inputPixel) : SV_TARGET
{
    return txDiffuse.Sample(samLinear, inputPixel.tex);
}
