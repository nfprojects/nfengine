#include "GuiCommon.hlsl"

// simple passthrough to the geometry shader
VS_INPUT_OUTPUT main(VS_INPUT_OUTPUT input)
{
    return input;
}