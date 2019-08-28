#pragma pack_matrix(row_major)

struct OutputVertex
{
    float4 position : POSITION;
    float4 tangent : TANGENT;
    float4 binormal : BINORMAL;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
    float4 worldPosition : WORLDPOSITION;
};

struct GSOut
{
    float4 position : SV_POSITION;
    float4 tangent : TANGENT;
    float4 binormal : BINORMAL;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
    float4 worldPosition : WORLDPOSITION;
};

cbuffer Matrix_ConstantBuffer : register(b0)
{
    float4x4 world[5];
    float4x4 view;
    float4x4 projection;
    float4 cameraPosition;
};

// number of vertices we want to create
[maxvertexcount(3)]
// we want to operate on a point only, primitive topology needs to be same as the primitive type we are operating on
void main(point OutputVertex input[1], inout TriangleStream<GSOut> output)
{
    GSOut verts[3] = { input[0], input[0], input[0] };

    verts[0].position.x -= 5.5f;
    verts[2].position.x += 5.5f;
    verts[1].position.y += 5.5f;

    // bring it to project space for rasterization
	for (uint i = 0; i < 3; i++)
	{
        verts[i].position = mul(verts[i].position, view);
        verts[i].position = mul(verts[i].position, projection);
    }

    output.Append(verts[0]);
    output.Append(verts[1]);
    output.Append(verts[2]);
    output.RestartStrip();

}