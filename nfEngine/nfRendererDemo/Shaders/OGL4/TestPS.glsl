#version 400 core

in VertexShaderOutput
{
    vec2 TexCoord;
    vec4 Color;
} Input;

out vec3 color;

void main()
{
    color = vec3(Input.Color);
}
