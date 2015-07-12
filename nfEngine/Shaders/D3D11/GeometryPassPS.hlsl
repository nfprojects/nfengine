Texture2D gDiffuseTexture : register(t0);
Texture2D gNormalTexture : register(t1);
Texture2D gSpecularTexture : register(t2);
Texture2D gEmissionTexture : register(t3);

SamplerState gTextureSampler : register(s0);


cbuffer Global : register(b0)
{
    float4x4 gViewMatrix;
    float4x4 gProjMatrix;
    float4x4 gViewProjMatrix;
    float4x4 gSecondaryViewProjMatrix;
    float4 gCameraVelocity;
    float4 gCameraAngularVelocity;
};

cbuffer Material : register(b1)
{
    float4 gDiffuseColor;
    float4 gSpecularColor;
    float4 gEmissionColor;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 PosVS : TEXCOORD6;
    float3 WorldPos : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float3 Tangent : TEXCOORD2;
    float3 Binormal : TEXCOORD3;
    float4 ScreenPos : TEXCOORD4;
#if (USE_MOTION_BLUR > 0)
    float4 ScreenPos_dt : TEXCOORD5;
#endif // (USE_MOTION_BLUR > 0)
};

struct PixelShaderOutput
{
    float4 color0 : SV_TARGET0;
    float4 color1 : SV_TARGET1;
    float4 color2 : SV_TARGET2;
    float2 color3 : SV_TARGET3;
};

//---------------------------------------------------------------

#define MOTION_BLUR_DT (0.01)

PixelShaderOutput main(VS_OUTPUT In)
{
    PixelShaderOutput output = (PixelShaderOutput)0;

    // --- GEOMETRY BUFFER LAYOUT ---
    // DSV  [float32]    | depth |
    // TEX0 [float16]    |          normal.xyz         | spec.factor |
    // TEX1 [float16]    |       diffuseColor.rgb      | spec.power  |
    // TEX2 [float16]    |      emissionColor.rgb      |  <unused>   |
    // TEX3 [float16]    | motionVector.uv  | <unused> |  <unused>   |

    float4 diffuseColor = gDiffuseColor * gDiffuseTexture.Sample(gTextureSampler, In.TexCoord);

    // TODO: gamma correction
    // diffuseColor.rgb *= diffuseColor.rgb;

    output.color1 = float4(diffuseColor.rgb, gSpecularColor.w); //difuse color + spec power

    //specular mapping
    float specularFactor = gSpecularTexture.Sample(gTextureSampler, In.TexCoord).r * gSpecularColor.r;


    //normal mapping
    float3 Normal = In.Normal;
    float3 NormalMapValue = 2 * gNormalTexture.Sample(gTextureSampler, In.TexCoord).rgb - float3(1.0, 1.0, 1.0);
    NormalMapValue.y *= -1;
    float3x3 TBN = transpose(float3x3(In.Tangent, In.Binormal, Normal));
    Normal = normalize(mul(TBN, NormalMapValue));

    //normal
    output.color0 = float4(Normal, specularFactor);

    output.color2 = float4(gEmissionColor.xyz, 0);

#if (USE_MOTION_BLUR > 0)
    // calculate screen space motion vector
    float3 screenPos = In.ScreenPos.xyz / In.ScreenPos.w;
    float3 screenPos_dt = In.ScreenPos_dt.xyz / In.ScreenPos_dt.w;
    float3 ScreenVelocity = (screenPos_dt - screenPos);
    ScreenVelocity.y = -ScreenVelocity.y;

    //store motion vector
    output.color3 = ScreenVelocity.xy / MOTION_BLUR_DT;
#endif // (USE_MOTION_BLUR > 0)

    return output;
}