// TODO: restore when multishaders and textures are supported
/*
#if (USE_TEXTURE > 0)
    Texture2D tex0 : register(t0);
    SamplerState tex_sampler : register(s0);
#endif
*/

struct VertexShaderOutput
{
    float4 color    : COLOR;
    float2 texCoord : TEXCOORD0;
    float4 pos      : SV_POSITION;
};

//---------------------------------------------------------------

float4 main(VertexShaderOutput input) : SV_TARGET0
{
    float4 color = input.color;

    // TODO: restore when multishaders and textures are supported
    /*
#if (USE_TEXTURE > 0)
    color *= tex0.Sample(tex_sampler, In.TexCoord);
#endif
    */

    return color;
}