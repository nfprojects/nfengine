Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct VertexShaderOutput
{
    float2 TexCoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
    float4 Pos : SV_POSITION;
};

float4 main(VertexShaderOutput In) : SV_TARGET0
{
#if (MODE == 0) // primary texture

    float2 texDimensions;
    gTexture.GetDimensions(texDimensions.x, texDimensions.y);
    float2 invTexDimensions = rcp(texDimensions);

    float4 sum = float4(0, 0, 0, 0);
    const int BLUR_SIZE = 4;
    for (int i = -BLUR_SIZE; i <= BLUR_SIZE; ++i)
    {
        for (int j = -BLUR_SIZE; j <= BLUR_SIZE; ++j)
        {
            float2 offset = float2(i, j) * invTexDimensions;
            sum += gTexture.Sample(gSampler, In.TexCoord + offset);
        }
    }
    return In.Color * sum / ((2 * BLUR_SIZE + 1) * (2 * BLUR_SIZE + 1));

#elif (MODE == 1) // depth buffer

    return In.Color * float4(gTexture.Sample(gSampler, In.TexCoord).rrr, 1.0f);

#elif (MODE == 2) // secondary texture

    return In.Color * gTexture.Sample(gSampler, In.TexCoord);

#endif
}
