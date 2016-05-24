#include "OmniLightCommon.hlsl"

// Binding slot 2: shadow map
#if (USE_SHADOW_MAP > 0)
    SamplerComparisonState gShadowSampler : register(s6);
    TextureCube<float> gShadowMap : register(t6);
#endif

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput In)
{
    PixelShaderOutput output;

    int3 texelCoords = int3((int2)In.pos.xy, 0);
    float4 color0 = gGBufferTex0.Load(texelCoords); // normal
    float4 color1 = gGBufferTex1.Load(texelCoords); // color
    // float4 color2 = gGBufferTex2.Load(texelCoords); // <unused>
    // float4 color3 = gGBufferTex3.Load(texelCoords); // <unused>

    float depth = gDepthTex.Load(texelCoords); //depth
    clip(gInfinityDist - depth);
    depth *= gMaxDepth;

    //scale to [-1 .. 1]
    float2 projectedXY = In.pos.xy * gViewportResInv.xy * 2.0 - float2(1.0, 1.0);
    projectedXY.y = -projectedXY.y;

    // decode world-space pixel position
    float3 viewSpaceRay = gCameraMatrix[2].xyz +
                          gScreenScale.x * projectedXY.x * gCameraMatrix[0].xyz +
                          gScreenScale.y * projectedXY.y * gCameraMatrix[1].xyz;
    float3 worldPos = gCameraMatrix[3].xyz + viewSpaceRay * depth;


    float3 normal = color0.xyz; // world-space normal vector
    float3 color = color1.xyz; // diffuse color
    float specularFactor = color0.w;
    float specularPower = color1.w;

    float3 lightVec = gLightPos.xyz - worldPos;
    float lightDist = length(lightVec);
    lightVec /= lightDist; // normalize

    float NdotL = dot(normal, lightVec);
    clip(NdotL);
    clip(gLightRadius.x - lightDist);


#if (USE_SHADOW_MAP > 0)

    float3 offsetU;
    float3 offsetV;

    // PCF sampling offset depends on texture cube side
    if ((lightVec.x > sqrt(2.0f) / 2.0f) || (lightVec.x < -sqrt(2.0f) / 2.0f))
    {
        offsetU = float3(0.0f, 0.0f, 1.0f);
        offsetV = float3(0.0f, 1.0f, 0.0f);
    }
    else if ((lightVec.z > sqrt(2.0f) / 2.0f) || (lightVec.z < -sqrt(2.0f) / 2.0f))
    {
        offsetU = float3(1.0f, 0.0f, 0.0f);
        offsetV = float3(0.0f, 1.0f, 0.0f);
    }
    else
    {
        offsetU = float3(1.0f, 0.0f, 0.0f);
        offsetV = float3(0.0f, 0.0f, 1.0f);
    }

    const int PCF_SIZE = 1;
    float shadowValue = 0.0f;
    // simple Percentage Closer Filtering
    for (int x = -PCF_SIZE; x <= PCF_SIZE; x++)
        for (int y = -PCF_SIZE; y <= PCF_SIZE; y++)
        {
            float3 offset = (x * offsetU + y * offsetV) * gShadowMapProps.x;
            shadowValue += gShadowMap.SampleCmpLevelZero(gShadowSampler,
                                                         -lightVec + offset,
                                                         lightDist).x;
        }
    shadowValue /= (2 * PCF_SIZE + 1) * (2 * PCF_SIZE + 1);
#else
    float shadowValue = 1.0f;
#endif


    float fadeOut = 1.0f - lightDist / gLightRadius.x;
    fadeOut *= fadeOut;
    fadeOut /= (lightDist * lightDist);

    // specular
    float3 specular = float3(0, 0, 0);
    float3 eyeVector = normalize(viewSpaceRay);
    float3 reflectVector = reflect(eyeVector, normal);
    float RdotL = dot(reflectVector, lightVec);
    if (RdotL > 0)
    {
        specular = specularFactor * pow(abs(RdotL), specularPower);
    }

    output.color = float4(shadowValue * gLightColor.xyz * fadeOut * (color * NdotL + specular), 1.0f);
    return output;
}