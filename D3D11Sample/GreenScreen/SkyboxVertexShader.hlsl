#pragma pack_matrix(row_major)

struct InputVertex
{
    float4 position : POSITION;
    float4 color : COLOR0;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

struct OutputVertex
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
    float4 worldPosition : WORLDPOSITION;
    float4 localPosition : LOCALPOSITION;
};

cbuffer Matrix_ConstantBuffer : register(b0)
{
    float4x4 world[5];
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

OutputVertex main(InputVertex input, uint instanceID : SV_InstanceID)
{
    OutputVertex output = (OutputVertex) 0;
    output.position = input.position;
    output.localPosition = input.position;
    output.position = mul(output.position, world[instanceID]);
    output.worldPosition = output.position;
    output.normal = input.normal;
    output.normal = mul(float4(output.normal, 1), world[instanceID]).xyz;
    output.position.x += sin(output.position.y * 0.1f + lightRadius.z) * lightRadius.w;
    output.position.y += cos(output.position.x * 0.1f + lightRadius.z) * lightRadius.w;
    output.position = mul(output.position, view);
    output.position = mul(output.position, projection);
    output.tex = input.tex;
    return output;
}
