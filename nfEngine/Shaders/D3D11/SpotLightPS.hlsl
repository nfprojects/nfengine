// g-buffer
Texture2D<float4> gGBufferTex0 : register(t0);
Texture2D<float4> gGBufferTex1 : register(t1);
Texture2D<float4> gGBufferTex2 : register(t2);
Texture2D<float4> gGBufferTex3 : register(t3);
Texture2D<float> gDepthTex : register(t4);

#if (USE_LIGHT_MAP > 0)
    SamplerState gLightMapSampler : register(s0);
    Texture2D<float3> gLightMap : register(t5);
#endif

cbuffer Global : register(b0)
{
    float4x4 gCameraMatrix;
    float4x4 gViewMatrix;
    float4x4 gProjMatrix;
    float4 gViewportResInv;
    float4 gScreenScale;
};

cbuffer SpotLightProps : register(b1)
{
    float4 gLightPos;
    float4 gDirection;
    float4 gLightColor;
    float4 gFarDist;
    float4x4 gLightViewProjMatrix;
    float4x4 gLightViewProjMatrixInv;
};

struct VertexShaderOutput
{
    float4 pos : SV_POSITION;
    float3 viewPos : POSITION;
};

float3 TransformLightCoords(float4 coord)
{
    float3 result = coord.xyz / coord.w;
    result.x = result.x * 0.5 + 0.5;
    result.y = -result.y * 0.5 + 0.5;
    return result;
}

struct PixelShaderOutput
{
    float depth : SV_Depth;
    float4 color : SV_TARGET0;
};

static float gMaxDepth = 10000.0f;
static float gInfinityDist = 0.999999f;

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.depth = input.viewPos.z;

    int3 texelCoords = int3((int2)input.pos.xy, 0);
    float4 color0 = gGBufferTex0.Load(texelCoords);    // normal
    float4 color1 = gGBufferTex1.Load(texelCoords);    // color
    // float4 color2 = gGBufferTex2.Load(texelCoords); // <unused>
    // float4 color3 = gGBufferTex3.Load(texelCoords); // <unused>

    float depth = gDepthTex.Load(texelCoords); //depth
    clip(gInfinityDist - depth);
    depth *= gMaxDepth;

    float2 projectedXY = input.pos.xy * gViewportResInv.xy * 2.0 - float2(1.0, 1.0); //scale to [-1 .. 1]
    projectedXY.y = -projectedXY.y;

    float3 viewSpaceRay = gCameraMatrix[2].xyz +
        gScreenScale.x * projectedXY.x * gCameraMatrix[0].xyz +
        gScreenScale.y * projectedXY.y * gCameraMatrix[1].xyz;

    float3 cameraPosition = gCameraMatrix[3].xyz;
    float3 pos = cameraPosition + viewSpaceRay * depth;
    float3 normal = color0.xyz;
    float specularFactor = color0.w;
    float specularPower = color1.w;

    float3 lightSpacePos = TransformLightCoords(mul(float4(pos, 1), gLightViewProjMatrix));

    // clip pixels that are outside light frustum
    float3 termA = lightSpacePos > float3(0, 0, 0);
    float3 termB = lightSpacePos < float3(1, 1, 1);
    clip(dot(termA, termB) - 3.0f);

    float3 lightVec = gLightPos.xyz - pos;
    float lightDist = length(lightVec);
    lightVec /= lightDist; //normalize

    float NdotL = dot(normal, lightVec);
    NdotL = max(NdotL, 0);
    clip(NdotL);
    clip(gFarDist.x - lightDist);

#if (USE_LIGHT_MAP > 0)
    float3 lightMapColor = gLightMap.SampleLevel(gLightMapSampler, lightSpacePos.xy, 0);
    clip(dot(lightMapColor, float3(1, 1, 1)) - 0.00001f);
#endif

    // TODO: shadows

    float fadeOut = 1.0f - lightDist / gFarDist.x;
    fadeOut /= (lightDist * lightDist);

    // specular
    float3 specular = float3(0, 0, 0);
    float3 eyeVector = normalize(viewSpaceRay);
    float3 reflectVector = reflect(eyeVector, normal);
    float RdotL = dot(reflectVector, lightVec);
    if (RdotL > 0)
        specular = specularFactor * pow(RdotL, specularPower);

    float3 result = gLightColor.xyz * fadeOut * (color1.xyz * NdotL + specular);

#if (USE_LIGHT_MAP > 0)
    result *= lightMapColor;
#endif

    output.color = float4(result, 1);
    return output;
}