Texture2D gDiffuseTexture : register(t0);
Texture2D gNormalTexture : register(t1);
Texture2D gSpecularTexture : register(t2);

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

struct VertexShaderOutput
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float4 viewPos : TEXCOORD0;
    float2 texCoord : TEXCOORD1;
    float3 normal : TEXCOORD2;
    float3 tangent : TEXCOORD3;
    float3 binormal : TEXCOORD4;
#if (USE_MOTION_BLUR > 0)
    float4 screenPos : TEXCOORD5;
    float4 screenPos_dt : TEXCOORD6;
#endif // (USE_MOTION_BLUR > 0)
};

struct PixelShaderOutput
{
    float depth : SV_Depth;
    float4 color0 : SV_TARGET0;
    float4 color1 : SV_TARGET1;
    float4 color2 : SV_TARGET2;
    // float2 color3 : SV_TARGET3;
};

#define MOTION_BLUR_DT (0.01)

static float gMaxDepth = 10000.0f;

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output = (PixelShaderOutput)0;
    output.depth = input.viewPos.z / gMaxDepth;

    // --- GEOMETRY BUFFER LAYOUT ---
    // DSV  [float32]    | depth |
    // TEX0 [float16]    |          normal.xyz         | spec.factor |
    // TEX1 [float16]    |       diffuseColor.rgb      | spec.power  |
    // TEX2 [float16]    |      emissionColor.rgb      |  <unused>   |
    // TEX3 [float16]    | motionVector.uv  | <unused> |  <unused>   |

    float4 diffuseColor = gDiffuseColor * gDiffuseTexture.Sample(gTextureSampler, input.texCoord);

    // gamma correction
    diffuseColor.rgb *= diffuseColor.rgb;  // TODO: use SRGB format for color textures

    output.color1 = float4(diffuseColor.rgb, gSpecularColor.w); //difuse color + spec power

    //specular mapping
    float specularFactor = gSpecularTexture.Sample(gTextureSampler, input.texCoord).r *
                           gSpecularColor.r;

    //normal mapping
    float3 normal = input.normal;
    float3 normalMapValue = 2.0 * gNormalTexture.Sample(gTextureSampler, input.texCoord).rgb -
                            float3(1.0, 1.0, 1.0);
    normalMapValue.y *= -1;
    float3x3 TBN = transpose(float3x3(input.tangent, input.binormal, normal));
    normal = normalize(mul(TBN, normalMapValue));

    //normal
    output.color0 = float4(normal, specularFactor);

    output.color2 = float4(gEmissionColor.xyz, 0);

#if (USE_MOTION_BLUR > 0)
    // calculate screen space motion vector
    float3 screenPos = In.screenPos.xyz / In.screenPos.w;
    float3 screenPos_dt = In.screenPos_dt.xyz / In.screenPos_dt.w;
    float3 screenVelocity = (screenPos_dt - screenPos);
    screenVelocity.y = -screenVelocity.y;

    //store motion vector
    output.color3 = screenVelocity.xy / MOTION_BLUR_DT;
#endif // (USE_MOTION_BLUR > 0)

    return output;
}