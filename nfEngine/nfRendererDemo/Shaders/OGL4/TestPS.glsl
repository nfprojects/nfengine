#if USE_TEXTURE == 1
layout (binding = 0) uniform sampler2D gTextureSampler;

layout (row_major, binding=0) uniform TestCBuffer
{
    mat4 viewMatrix;
};
#endif

in VertexShaderOutput
{
    vec2 TexCoord;
    vec4 Color;
} Input;

out vec4 color;

void main()
{
#if USE_TEXTURE == 1
    color = Input.Color * texture2D(gTextureSampler, Input.TexCoord);
#else
    color = Input.Color;
#endif
}
