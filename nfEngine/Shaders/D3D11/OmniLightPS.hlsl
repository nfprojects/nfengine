//g-buffer
Texture2D<float4> tex0 : register(t0);
Texture2D<float4> tex1 : register(t1);
Texture2D<float4> tex2 : register(t2);
Texture2D<float4> tex3 : register(t3);
Texture2D<float> tex_depth : register(t4);

cbuffer Global : register(b0)
{
    float4x4 gCameraMatrix;
    float4x4 gViewMatrix;
    float4x4 gProjMatrix;
    float4x4 gProjInverseMatrix;
    float4x4 gViewProjMatrix;
    float4x4 gViewProjInverseMatrix;
    float4 gViewportResInv;
    float4 gScreenScale;
};

cbuffer OmniLightProps : register(b1)
{
    float4 LightPos;
    float4 LightRadius;
    float4 LightColor;
};

struct VertexShaderInput
{
    float4 Pos : SV_POSITION;
    float4 ViewPos : POSITION;
};

//---------------------------------------------------------------

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

static float gMaxDepth = 10000.0f;
static float gInfinityDist = 0.999999f;

PixelShaderOutput main(VertexShaderInput In)
{
    PixelShaderOutput output;

    int3 texelCoords = int3((int2)In.Pos.xy, 0);
    float4 color0 = tex0.Load(texelCoords); // normal
    float4 color1 = tex1.Load(texelCoords); // color
    // float4 color2 = tex2.Load(texelCoords); // <unused>
    // float4 color3 = tex3.Load(texelCoords); // <unused>

    float depth = tex_depth.Load(texelCoords); //depth
    clip(gInfinityDist - depth);
    depth *= gMaxDepth;

    //scale to [-1 .. 1]
    float2 projectedXY = In.Pos.xy * gViewportResInv.xy * 2.0 - float2(1.0, 1.0);
    projectedXY.y = -projectedXY.y;

    // decode world-space pixel position
    float3 viewSpaceRay = gCameraMatrix[2].xyz + 
                          gScreenScale.x * projectedXY.x * gCameraMatrix[0].xyz +
                          gScreenScale.y * projectedXY.y * gCameraMatrix[1].xyz;
    float3 Pos = gCameraMatrix[3].xyz + viewSpaceRay * depth;


    float3 Normal = color0.xyz;
    float3 Color = color1.xyz;
    float SpecularFactor = color0.w;
    float SpecularPower = color1.w;
    
    float3 LightVec = LightPos.xyz - Pos;
    float LightDist = length(LightVec);
    LightVec /= LightDist; // normalize
    
    float NdotL = dot(Normal, LightVec);
    clip(NdotL);
    clip(LightRadius.x - LightDist);

    // TODO: shadows
    float ShadowValue = 1;

    float fadeOut = 1.0f - LightDist / LightRadius.x;
    fadeOut *= fadeOut;
    fadeOut /= (LightDist*LightDist);
    
    // specular
    float3 Specular = float3(0, 0, 0);
    float3 EyeVector = normalize(viewSpaceRay);
    float3 ReflectVector = reflect(EyeVector, Normal);
    float RdotL = dot(ReflectVector, LightVec);
    if (RdotL > 0)
    {
        Specular = SpecularFactor * pow(RdotL, SpecularPower);
    }

    output.color = float4(LightColor.xyz * fadeOut * ShadowValue * (Color * NdotL + Specular), 1.0f);
    return output;
}