layout (location = 0) in vec3 InPos;
layout (location = 1) in vec4 InColor;

#if USE_INSTANCING > 0
layout (location = 2) in vec3 InInstancePos;
layout (location = 3) in vec4 InInstanceColor;
#endif // USE_INSTANCING > 0


out gl_PerVertex
{
    vec4 gl_Position;
};

layout (location = 0) out VertexShaderOutput
{
    vec4 Color;
} Output;


void main()
{
    gl_Position = vec4(InPos, 1.0f);
    Output.Color = InColor;

#if USE_INSTANCING > 0
    gl_Position += vec4(InInstancePos, 0.0f);
    Output.Color *= InInstanceColor;
#endif // USE_INSTANCING > 0
}
