//g-buffer
Texture2D<float4> tex0 : register(t0);
Texture2D<float4> tex1 : register(t1);
Texture2D<float4> tex2 : register(t2);
Texture2D<float4> tex3 : register(t3);
Texture2D<float> tex_depth : register(t4);

cbuffer Global : register(b0)
{
    float4x4 g_CameraMatrix;
    float4x4 g_ViewMatrix;
    float4x4 g_ProjMatrix;
    float4x4 g_ProjInverseMatrix;
    float4x4 g_ViewProjMatrix;
    float4x4 g_ViewProjInverseMatrix;
    float4 g_ViewportResInv;
    float4 g_ScreenScale;
};

cbuffer OmniLightProps : register(b1)
{
    float4 LightPos;
    float4 LightRadius;
    float4 LightColor;
    float4 ShadowMapResInv;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 ViewPos : POSITION;
};

//---------------------------------------------------------------

struct PixelShaderOutput
{
    float depth : SV_Depth;
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VS_OUTPUT In)
{
    PixelShaderOutput output;
    output.depth = In.ViewPos.z * 0.001f;

    int3 texelCoords = int3((int2)In.Pos.xy, 0);
    float4 color0 = tex0.Load(texelCoords); //normal
    float4 color1 = tex1.Load(texelCoords); //color
    //float4 color2 = tex2.Load(texelCoords); //<unused>
    //float4 color3 = tex3.Load(texelCoords); //<unused>

    float depth = tex_depth.Load(texelCoords); //depth
    clip(0.99999f - depth);
    depth *= 1000.0f;

    // depth = 0.05f + (1000.0 - 0.05f) * depth;

    //scale to [-1 .. 1]
    float2 projectedXY = In.Pos.xy * g_ViewportResInv.xy * 2.0 - float2(1.0, 1.0);
    projectedXY.y = -projectedXY.y;

    float3 viewSpaceRay = g_CameraMatrix[2].xyz + 
                          g_ScreenScale.x * projectedXY.x * g_CameraMatrix[0].xyz +
                          g_ScreenScale.y * projectedXY.y * g_CameraMatrix[1].xyz;

    float3 CameraPosition = g_CameraMatrix[3].xyz;
    float3 Pos = CameraPosition + viewSpaceRay * depth;


    float3 Normal = color0.xyz;
    float3 Color = color1.xyz;
    float SpecularFactor = color0.w;
    float SpecularPower = color1.w;
    
    float3 LightVec = LightPos.xyz - Pos;
    float LightDist = length(LightVec);
    LightVec /= LightDist; //normalize
    
    float NdotL = dot(Normal, LightVec);
    clip(NdotL);
    clip(LightRadius.x - LightDist);

    // TODO: shadows
    float ShadowValue = 1;

    float fadeOut = 1.0f - LightDist / LightRadius.x;
    fadeOut *= fadeOut;
    fadeOut /= (LightDist*LightDist);
    
    //specular
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