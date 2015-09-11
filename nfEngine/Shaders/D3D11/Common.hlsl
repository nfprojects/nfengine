/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Common data structures and functions
 */

// Vertex Layout for Geometry Renderer
struct MeshVertexShaderInput
{
    /// per-vertex data
    float3 pos      : POSITION;
    float2 texCoord : TEXCOORD0;
    float4 normal   : TEXCOORD1;
    float4 tangent  : TEXCOORD2;

    // per-instance data
    float4 wordMat0 : TEXCOORD3;
    float4 wordMat1 : TEXCOORD4;
    float4 wordMat2 : TEXCOORD5;
    float4 velocity : TEXCOORD6;
    float4 angularVelocity : TEXCOORD7;  // TODO: remove when MB is off
};