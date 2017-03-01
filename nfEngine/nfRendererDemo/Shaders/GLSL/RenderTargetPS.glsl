in VertexShaderOutput
{
    vec2 TexCoord;
    vec4 Color;
} Input;

layout (location = 0) out vec4 color0;
#if (TARGETS > 1)
layout (location = 1) out vec4 color1;
#endif

void main()
{
    color0 = Input.Color;
#if (TARGETS > 1)
    color1 = vec4(1.0f, 1.0f, 1.0f, 1.0f) - Input.Color;
#endif
}
