layout (location=0) in vec3 InPos;
layout (location=1) in vec2 InTexCoord;
layout (location=2) in vec4 InColor;

layout (row_major, set=0, binding=0) uniform TestCBuffer
{
    mat4 viewMatrix;
} cbuffer;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout (location = 0) out VertexShaderOutput
{
    vec2 TexCoord;
    vec4 Color;
} Output;


void main()
{
    gl_Position = vec4(InPos, 1.0);

#ifdef VULKAN
    gl_Position.y = -gl_Position.y;
#endif

#if USE_CBUFFER == 1
    gl_Position = gl_Position * cbuffer.viewMatrix;
#endif

    Output.TexCoord = InTexCoord;
    Output.Color = InColor;
}
