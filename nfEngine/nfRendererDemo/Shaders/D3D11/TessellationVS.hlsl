struct IA_OUTPUT
{
    float3 pos : POSITION;
};

struct VS_OUTPUT
{
    float3 pos : POSITION;
};

VS_OUTPUT main(IA_OUTPUT input)
{
    VS_OUTPUT output;
    output.pos = input.pos;
    return output;
}
