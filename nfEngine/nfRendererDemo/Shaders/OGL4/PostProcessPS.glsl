#if (SAMPLES_NUM > 1)
layout (binding = 0) uniform sampler2DMS gTexture;
#else
layout (binding = 0) uniform sampler2D gTexture;
#endif

in VertexShaderOutput
{
    vec2 TexCoord;
    vec4 Color;
} Input;

out vec4 outColor;

vec4 mode0()
{
    vec4 sum = vec4(0, 0, 0, 0);
    const int BLUR_SIZE = 2;
    for (int i = -BLUR_SIZE; i <= BLUR_SIZE; ++i)
    {
        for (int j = -BLUR_SIZE; j <= BLUR_SIZE; ++j)
        {
            ivec2 offset = ivec2(i, j);
#if (SAMPLES_NUM > 1)
            for (int s = 0; s < SAMPLES_NUM; ++s)
                sum += texelFetch(gTexture, texelCoords + offset, 0, s) / (float)SAMPLES_NUM;
#else
            sum += texelFetch(gTexture, texelCoords + offset, 0);
#endif // SAMPLES_NUM
        }
    }
    return Input.Color * sum / ((2 * BLUR_SIZE + 1) * (2 * BLUR_SIZE + 1));
}

vec4 mode1()
{
    /* TODO
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
    return Input.Color * color;*/
}

void main()
{
    ivec2 texDimensions;

    texDimensions = textureSize(gTexture, 0);
    vec2 invTexDimensions = rcp((float2)texDimensions);
    ivec2 texelCoords = int2(float2(texDimensions) * In.TexCoord);

#if (MODE == 0) // primary texture
    outColor = mode0();
#elif (MODE == 1) // depth buffer
    outColor = mode1();
#endif // MODE
}
