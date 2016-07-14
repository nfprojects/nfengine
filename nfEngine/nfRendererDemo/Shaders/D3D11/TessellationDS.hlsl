struct HS_CONSTANT_OUTPUT
{
    float edges[2] : SV_TessFactor;
};

struct HS_OUTPUT
{
    float3 pos : POSITION;
};

struct DS_OUTPUT
{
    float4 position : SV_Position;
};

[domain("isoline")]
DS_OUTPUT main(HS_CONSTANT_OUTPUT input, OutputPatch<HS_OUTPUT, 4> op, float2 uv : SV_DomainLocation)
{
    DS_OUTPUT output;

    float t = uv.x;

    float3 pos = pow(1.0f - t, 3.0f) * op[0].pos +
                 3.0f * pow(1.0f - t, 2.0f) * t * op[1].pos +
                 3.0f * (1.0f - t) * pow(t, 2.0f) * op[2].pos +
                 pow(t, 3.0f) * op[3].pos;

    output.position = float4(pos, 1.0f);

    return output;
}