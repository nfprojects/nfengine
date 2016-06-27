in VertexShaderOutput
{
    vec4 Color;
} Input;

out vec4 color;

void main()
{
    color = Input.Color;
}
