#pragma pack_matrix(row_major)

struct InputVertex
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float4 color : COLOR0;
};

struct OutputVertex
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float4 color : COLOR0;
};

cbuffer ConstantBuffer : register( b0 )
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
    float4 lightPos[2];
    float4 lightNor[2];
    float4 lightColor[2];
    float4 lightRadius;
};

OutputVertex main(InputVertex input)
{
	OutputVertex output = (OutputVertex)0;
	// do vertex shader math here to modify vertex (look up shader intrinsics for more on shader variables)
    output.position = input.position;
    output.position = mul(output.position, world);
	output.normal = input.normal;
    output.normal = mul(float4(input.normal, 1), world).xyz;
    // directional lighting
    float ratio = saturate(dot((float3) lightNor[0], output.normal) + 0.25f);
    float4 color1 = lerp(float4(0.0f, 0.0f, 0.0f, 1.0f), lightColor[0], ratio);
    // point lighting
    float3 pointLightDir = normalize(lightPos[1] - output.position);
    float pointLightRatio = saturate(dot(pointLightDir, output.normal));
    float attenuation = 1.0f - saturate((length(lightPos[1] - output.position) / lightRadius.x));
    pointLightRatio = attenuation * attenuation * pointLightRatio;
    float4 color2 = lerp(float4(0.0f, 0.0f, 0.0f, 1.0f), lightColor[1], pointLightRatio);

    output.position = mul(output.position, view);
    output.position = mul(output.position, projection);
	output.tex = input.tex;
    output.color = color1 + color2;
	return output;
}
