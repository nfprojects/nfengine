#if USE_TEXTURE == 1
layout (set = 1, binding = 0) uniform sampler2D gTexture;
#endif

layout (location = 0) in VertexShaderOutput
{
    vec2 TexCoord;
    vec4 Color;
} Input;

layout (location = 0) out vec4 color;

void main()
{
#if USE_TEXTURE == 1
    color = Input.Color * texture(gTexture, Input.TexCoord);
#else
    color = Input.Color;
#endif
}
