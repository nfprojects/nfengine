#if (SAMPLES_NUM > 1)
layout (set = 1, binding = 0) uniform sampler2DMS gTexture;
#else
layout (set = 1, binding = 0) uniform sampler2D gTexture;
#endif

layout (location = 0) in VertexShaderOutput
{
    vec2 TexCoord;
    vec4 Color;
} Input;

layout (location = 0) out vec4 outColor;

// primary texture
vec4 mode0(ivec2 texelCoords)
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
                sum += texelFetch(gTexture, texelCoords + offset, s) / float(SAMPLES_NUM);
#else
            sum += texelFetch(gTexture, texelCoords + offset, 0);
#endif // SAMPLES_NUM
        }
    }
    return Input.Color * sum / ((2 * BLUR_SIZE + 1) * (2 * BLUR_SIZE + 1));
}

// depth buffer
vec4 mode1(ivec2 texelCoords)
{
    float color = 0.0f;
#if (SAMPLES_NUM > 1)
    // resolve multisampled texture via simple average
    for (int i = 0; i < SAMPLES_NUM; ++i)
        color += texelFetch(gTexture, texelCoords, i).r;
    color /= float(SAMPLES_NUM);
#else
    color = texelFetch(gTexture, texelCoords, 0).r;
#endif // SAMPLES_NUM
    return Input.Color * vec4(color, color, color, color);
}

// secondary texture
vec4 mode2(ivec2 texelCoords)
{
    vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
#if (SAMPLES_NUM > 1)
    // resolve multisampled texture via simple average
    for (int i = 0; i < SAMPLES_NUM; ++i)
        color += texelFetch(gTexture, texelCoords, i);
    color /= float(SAMPLES_NUM);
#else
    color = texelFetch(gTexture, texelCoords, 0);
#endif // SAMPLES_NUM
    return Input.Color * color;
}

void main()
{
    ivec2 texDimensions;

#if (SAMPLES_NUM > 1)
    texDimensions = textureSize(gTexture);
#else
    texDimensions = textureSize(gTexture, 0);
#endif

    vec2 invTexDimensions = vec2(1.0f / texDimensions.x, 1.0f / texDimensions.y);
    ivec2 texelCoords = ivec2(vec2(texDimensions) * Input.TexCoord);

#if (MODE == 0) // primary texture
    outColor = mode0(texelCoords);
#elif (MODE == 1) // depth buffer
    outColor = mode1(texelCoords);
#elif (MODE == 2) // secondary texture
    outColor = mode2(texelCoords);
#endif // MODE
}
