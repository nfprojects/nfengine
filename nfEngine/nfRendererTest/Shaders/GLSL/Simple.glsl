layout (location=0) in vec3 InPos;
layout (location=1) in vec4 InColor;
layout (location=2) in vec2 InTexCoord;

out VS_OUTPUT
{
    vec4 Color;
} Output;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout (row_major,binding=1) uniform TestCBuffer
{
    mat4 viewMatrix;
};

layout (binding=3) uniform sampler2D TestTexture; // just for IODesc test

layout (binding=5) uniform sampler2D TestSampler;

void main()
{
    gl_Position = vec4(InPos, 1.0f);
    gl_Position = gl_Position * viewMatrix;
    Output.Color = InColor * texture2D(TestSampler, InTexCoord);
}
