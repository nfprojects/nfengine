#if (SAMPLES_NUM > 1)
Texture2DMS<float4, SAMPLES_NUM> gTexture : register(t0);
#else
Texture2D<float4> gTexture : register(t0);
#endif

struct VertexShaderOutput
{
    float2 TexCoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
    float4 Pos : SV_POSITION;
};

float4 main(VertexShaderOutput In) : SV_TARGET0
{
    uint2 texDimensions;
    uint samplesNum;
#if (SAMPLES_NUM > 1)
    gTexture.GetDimensions(texDimensions.x, texDimensions.y, samplesNum);
#else
    gTexture.GetDimensions(texDimensions.x, texDimensions.y);
#endif // SAMPLES_NUM
    float2 invTexDimensions = rcp((float2)texDimensions);
    int2 texelCoords = int2(float2(texDimensions) * In.TexCoord);

#if (MODE == 0) // primary texture

    float4 sum = float4(0, 0, 0, 0);
    const int BLUR_SIZE = 2;
    [unroll] for (int i = -BLUR_SIZE; i <= BLUR_SIZE; ++i)
    {
        [unroll] for (int j = -BLUR_SIZE; j <= BLUR_SIZE; ++j)
        {
            int2 offset = int2(i, j);
#if (SAMPLES_NUM > 1)
            [unroll] for (int s = 0; s < SAMPLES_NUM; ++s)
                sum += gTexture.Load(texelCoords + offset, s) / (float)SAMPLES_NUM;
#else
            sum += gTexture.Load(int3(texelCoords + offset, 0));
#endif // SAMPLES_NUM
        }
    }
    return In.Color * sum / ((2 * BLUR_SIZE + 1) * (2 * BLUR_SIZE + 1));

#elif (MODE == 1) // depth buffer

    float color = 0.0f;
#if (SAMPLES_NUM > 1)
    // resolve multisampled texture via simple average
    [unroll] for (int i = 0; i < SAMPLES_NUM; ++i)
        color += gTexture.Load(texelCoords, i).r;
    color /= (float)SAMPLES_NUM;
#else
    color = gTexture.Load(int3(texelCoords, 0)).r;
#endif // SAMPLES_NUM
    return In.Color * float4(color, color, color, color);

#elif (MODE == 2) // secondary texture

    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
#if (SAMPLES_NUM > 1)
    // resolve multisampled texture via simple average
    [unroll] for (int i = 0; i < SAMPLES_NUM; ++i)
        color += gTexture.Load(texelCoords, i);
    color /= (float)SAMPLES_NUM;
#else
    color = gTexture.Load(int3(texelCoords, 0));
#endif // SAMPLES_NUM
    return In.Color * color;

#endif // MODE
}
