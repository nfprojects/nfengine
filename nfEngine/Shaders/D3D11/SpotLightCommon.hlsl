#include "LightCommon.hlsl"

cbuffer SpotLightProps : register(b1)
{
    float4 gLightPos;
    float4 gDirection;
    float4 gLightColor;
    float4 gFarDist;
    float4x4 gLightViewProjMatrix;
    float4x4 gLightViewProjMatrixInv;
    float4 gShadowMapProps;  // x = shadow map resolution inverse
};