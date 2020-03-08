#include "TessellationCommon.hlsl"

[domain("isoline")]
DomainShaderOutput main(HullShaderConstantOutput input,
                        OutputPatch<HullShaderOutput, 4> outputPatch,
                        float2 uv : SV_DomainLocation)
{
    DomainShaderOutput output;

    float t = uv.x;

    // Bezier interpolation
    float3 pos = pow(1.0f - t, 3.0f) * outputPatch[0].pos +
                 3.0f * pow(1.0f - t, 2.0f) * t * outputPatch[1].pos +
                 3.0f * (1.0f - t) * pow(t, 2.0f) * outputPatch[2].pos +
                 pow(t, 3.0f) * outputPatch[3].pos;

    output.position = float4(pos, 1.0f);

    return output;
}