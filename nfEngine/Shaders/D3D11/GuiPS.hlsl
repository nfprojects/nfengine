#include "GuiCommon.hlsl"

#if (USE_TEXTURE > 0)
    Texture2D gTexture : register(t0);
    SamplerState gSampler : register(s0);
#endif

float4 main(GS_OUT In) : SV_TARGET0
{
    float4 color = In.Color;

#if (USE_TEXTURE == 1) // normal mode
    color *= gTexture.Sample(gSampler, In.TexCoord);
#elif (USE_TEXTURE == 2) // alpha mode
    color.a *= gTexture.Sample(gSampler, In.TexCoord).r;
#endif

    return color;
}