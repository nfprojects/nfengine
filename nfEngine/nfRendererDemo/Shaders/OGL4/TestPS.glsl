#version 330 core
#extension GL_ARB_separate_shader_objects : enable

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
