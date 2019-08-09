#pragma pack_matrix(row_major)

struct InputVertex
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float4 normal : NORMAL;
};

struct OutputVertex
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 normal : NORMAL;
};

cbuffer ConstantBuffer : register( b0 )
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

OutputVertex main(InputVertex input)
{
	OutputVertex output = (OutputVertex)0;
	// do vertex shader math here to modify vertex (look up shader intrinsics for more on shader variables)
    output.position = input.position;
    output.position = mul(output.position, world);
    output.position = mul(output.position, view);
    output.position = mul(output.position, projection);
	output.normal = input.normal;
	output.tex = input.tex;

	return output;
}
