#version 400 core

layout (location=0) in vec3 InPos;
layout (location=1) in vec2 InTexCoord;
layout (location=2) in vec4 InColor;

out VertexShaderOutput
{
    vec2 TexCoord;
    vec4 Color;
    vec4 Pos;
} Output;


void main()
{
    Output.Pos = vec4(InPos, 1.0);
    Output.TexCoord = InTexCoord;
    Output.Color = InColor;
}
