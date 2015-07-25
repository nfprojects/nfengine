// g-buffer
Texture2D<float4> gGBufferTex0 : register(t0);
Texture2D<float4> gGBufferTex1 : register(t1);
Texture2D<float4> gGBufferTex2 : register(t2);
Texture2D<float4> gGBufferTex3 : register(t3);
Texture2D<float> gDepthTex : register(t4);

cbuffer Global : register(b0)
{
    float4x4 gCameraMatrix;
    float4x4 gViewMatrix;
    float4x4 gProjMatrix;
    float4 gViewportResInv;
    float4 gScreenScale;
};

cbuffer OmniLightProps : register(b1)
{
    float4 gLightPos;
    float4 gLightRadius;
    float4 gLightColor;
};

struct VertexShaderInput
{
    float4 pos : SV_POSITION;
    float4 viewPos : POSITION;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

static float gMaxDepth = 10000.0f;
static float gInfinityDist = 0.999999f;

PixelShaderOutput main(VertexShaderInput In)
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

    // TODO: shadows

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
        specular = specularFactor * pow(RdotL, specularPower);
    }

    output.color = float4(gLightColor.xyz * fadeOut * (color * NdotL + specular), 1.0f);
    return output;
}