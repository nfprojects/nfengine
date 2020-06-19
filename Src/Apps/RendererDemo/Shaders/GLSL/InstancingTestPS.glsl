layout (location = 0) in VertexShaderOutput
{
    vec4 Color;
} Input;

layout (location = 0) out vec4 color;

void main()
{
    color = Input.Color;
}
