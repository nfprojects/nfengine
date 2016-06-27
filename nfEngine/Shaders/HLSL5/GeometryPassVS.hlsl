#include "Common.hlsl"

cbuffer Global : register(b0)
{
    float4x4 gViewMatrix;
    float4x4 gProjMatrix;
    float4x4 gViewProjMatrix;
    float4x4 gSecondaryViewProjMatrix;
    float4 gCameraVelocity;
    float4 gCameraAngularVelocity;
}

struct VertexShaderOutput
{
    float4 pos : SV_POSITION;        //< screen-space vertex position
    float4 worldPos : POSITION;      //< world-space vertex position
    float4 viewPos : TEXCOORD0;      //< view-space vertex position
    float2 texCoord : TEXCOORD1;     //< texture coordinate
    float3 normal : TEXCOORD2;       //< world-space normal vector
    float3 tangent : TEXCOORD3;      //< world-space tangent vector
    float3 binormal : TEXCOORD4;     //< world-space binormal vector
#if (USE_MOTION_BLUR > 0)
    float4 screenPos : TEXCOORD5;    //< screen-space normal vector
    float4 screenPos_dt : TEXCOORD6;
#endif // (USE_MOTION_BLUR > 0)
};

//---------------------------------------------------------------

#define MOTION_BLUR_DT (0.01)

VertexShaderOutput main(MeshVertexShaderInput In)
{
    VertexShaderOutput Out;

    // decode world matrix from per-instance data
    float4x4 worldMatrix = transpose(float4x4(In.worldMat0,
                                              In.worldMat1,
                                              In.worldMat2,
                                              float4(0, 0, 0, 1)));

    // position transformation
    Out.worldPos = mul(float4(In.pos, 1.0f), worldMatrix);
    Out.pos = mul(Out.worldPos, gViewProjMatrix);
    Out.viewPos = mul(Out.worldPos, gViewMatrix);

#if (USE_MOTION_BLUR > 0)
    // vertex position in moment +dt
    float3 rotPos = mul(In.pos.xyz, (float3x3)worldMatrix); // rotated vertex
    float3 angularDisplacement = cross(In.angularVelocity.xyz, rotPos) * MOTION_BLUR_DT;
    float4 localPos_dt = float4(rotPos + worldMatrix[3].xyz + angularDisplacement, 1);
    float4 worldPos_dt = localPos_dt + float4(In.velocity.xyz, 0) * MOTION_BLUR_DT;
    float4 pos_dt = mul(worldPos_dt, gSecondaryViewProjMatrix);
    Out.screenPos = Out.pos;
    Out.screenPos_dt = pos_dt;
#endif // (USE_MOTION_BLUR > 0)

    Out.texCoord = In.texCoord;

    // TBN matrix
    Out.normal = mul(In.normal.xyz, (float3x3)worldMatrix);
    Out.tangent = mul(In.tangent.xyz, (float3x3)worldMatrix);
    Out.binormal = cross(Out.tangent, Out.normal);

    return Out;
}