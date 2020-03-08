#include "TessellationCommon.hlsl"

HullShaderConstantOutput HSConst()
{
    HullShaderConstantOutput output;
    output.edges[0] = 1.0f; // Detail factor
    output.edges[1] = 60.0; // Density factor
    return output;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(4)]
[patchconstantfunc("HSConst")]
HullShaderOutput main(InputPatch<VertexShaderOutput, 4> inputPatch,
                      uint id : SV_OutputControlPointID)
{
    HullShaderOutput output;
    output.pos = inputPatch[id].pos;
    return output;
}