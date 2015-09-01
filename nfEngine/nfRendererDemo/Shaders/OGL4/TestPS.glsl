#version 330 core

in VertexShaderOutput
{
    vec2 TexCoord;
    vec4 Color;
    vec4 Pos;
} Input;

out vec3 color;

void main()
{
    color = vec3(Input.Color);
}
