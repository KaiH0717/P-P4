Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct OutputVertex
{
	float4 position : SV_POSITION;
    float4 color : COLOR0;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
    float4 worldPosition : WORLDPOSITION;
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

float4 main(OutputVertex inputPixel) : SV_TARGET
{
    // directional lighting
    float ratio = saturate(dot((float3) -lightNor[0], inputPixel.normal) + 0.45f);
    float4 color1 = lerp(float4(0.0f, 0.0f, 0.0f, 1.0f), lightColor[0], ratio);
    // point lighting
    float3 pointLightDir = (float3) normalize(lightPos[1] - inputPixel.worldPosition);
    float pointLightRatio = saturate(dot(pointLightDir, inputPixel.normal) + 0.45f);
    float attenuation = 1.0f - saturate((length(lightPos[1] - inputPixel.worldPosition) / lightRadius.x));
    pointLightRatio = attenuation * attenuation * pointLightRatio;
    float4 color2 = lerp(float4(0.0f, 0.0f, 0.0f, 1.0f), lightColor[1], pointLightRatio);
    // spot light
    float3 spotLightDir = (float3) normalize(lightPos[2] - inputPixel.worldPosition);
    float surfaceRatio = saturate(dot(spotLightDir, (float3) -lightNor[2]));
    float spotLightRatio = saturate(dot(spotLightDir, inputPixel.normal) + 0.45f);
    attenuation = 1.0f - saturate((coneRatio.x - surfaceRatio) / (coneRatio.x - coneRatio.y));
    spotLightRatio = attenuation * attenuation * spotLightRatio;
    float4 color3 = lerp(float4(0.0f, 0.0f, 0.0f, 1.0f), lightColor[2], spotLightRatio);
    // specular component
    float3 viewDir = normalize(cameraPosition - inputPixel.worldPosition);
    float3 halfVector = normalize(((float3) -lightNor[0]) + viewDir);
    float intensity = saturate(pow(dot(inputPixel.normal, halfVector), 2.2f));
    float4 color4 = lerp(float4(0.0f, 0.0f, 0.0f, 1.0f), lightColor[0], intensity * 1.25f);
    inputPixel.color = (color1 + color2 + color3 + color4) * txDiffuse.Sample(samLinear, inputPixel.tex);
    if (coneRatio.z == 1.0f)
    {
        float grey = (inputPixel.color.x + inputPixel.color.y + inputPixel.color.z) / 3.0f;
        inputPixel.color = float4(grey, grey, grey, 1.0f);
    }
    //inputPixel.tex.x += sin(inputPixel.tex.y * 0.1f + lightRadius.z) * lightRadius.w;
    //inputPixel.tex.y += cos(inputPixel.tex.x * 0.1f + lightRadius.z) * lightRadius.w;
    return inputPixel.color;
}
