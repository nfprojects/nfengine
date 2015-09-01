#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout (location=0) in vec3 InPos;
layout (location=1) in vec2 InTexCoord;
layout (location=2) in vec4 InColor;

out gl_PerVertex
{
    vec4 gl_Position;
};

out VertexShaderOutput
{
    vec2 TexCoord;
    vec4 Color;
} Output;


void main()
{
    gl_Position = vec4(InPos, 1.0);
    Output.TexCoord = InTexCoord;
    Output.Color = InColor;
}
