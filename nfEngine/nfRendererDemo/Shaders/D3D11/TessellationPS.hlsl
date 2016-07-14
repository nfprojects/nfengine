struct DS_OUTPUT
{
    float4 position : SV_Position;
};

float4 main(DS_OUTPUT input) : SV_Target0
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}