#include "LightCommon.hlsl"

cbuffer OmniLightProps : register(b1)
{
    float4 gLightPos;
    float4 gLightRadius;
    float4 gLightColor;
    float4 gShadowMapProps;  // x = shadow map resolution inverse
};