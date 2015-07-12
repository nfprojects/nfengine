cbuffer Global : register(b0)
{
    float4x4 gViewMatrix;
    float4x4 gProjMatrix;
    float4x4 gViewProjMatrix;
    float4x4 gSecondaryViewProjMatrix;
    float4 gCameraVelocity;
    float4 gCameraAngularVelocity;
}

struct VertexShaderInput
{
    /// per-vertex data
    float3 Pos      : POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Normal   : TEXCOORD1;
    float4 Tangent  : TEXCOORD2;

    // per-instance data
    float4 WordMat0 : TEXCOORD3;
    float4 WordMat1 : TEXCOORD4;
    float4 WordMat2 : TEXCOORD5;
    float4 Velocity : TEXCOORD6;
    float4 AngularVelocity : TEXCOORD7;  // TODO: remove when MB is off
};

struct VertexShaderOutput
{
    float4 Pos : SV_POSITION;        //< screen-space vertex position
    float4 PosVS : TEXCOORD6;        //< view-space vertex position
    float4 WorldPos : POSITION;      //< world-space vertex position
    float2 TexCoord : TEXCOORD0;     //< texture coordinate
    float3 Normal : TEXCOORD1;       //< world-space normal vector
    float3 Tangent : TEXCOORD2;      //< world-space tangent vector
    float3 Binormal : TEXCOORD3;     //< world-space binormal vector
    float4 ScreenPos : TEXCOORD4;    //< screen-space normal vector (TODO: duplicate?)
#if (USE_MOTION_BLUR > 0)
    float4 ScreenPos_dt : TEXCOORD5;
#endif // (USE_MOTION_BLUR > 0)
};

//---------------------------------------------------------------

#define MOTION_BLUR_DT (0.01)

VertexShaderOutput main(VertexShaderInput In)
{
    VertexShaderOutput Out;

    // decode world matrix from per-instance data
    float4x4 worldMatrix = transpose(float4x4(In.WordMat0, In.WordMat1, In.WordMat2, float4(0, 0, 0, 1)));

    // position transformation
    Out.WorldPos = mul(float4(In.Pos, 1.0f), worldMatrix);
    Out.PosVS = mul(Out.WorldPos, gViewMatrix);
    Out.Pos = mul(Out.WorldPos, gViewProjMatrix);
    Out.ScreenPos = Out.Pos;

#if (USE_MOTION_BLUR > 0)
    // vertex position in moment +dt
    float3 rotPos = mul(In.Pos.xyz, (float3x3)worldMatrix); // rotated vertex
    float3 angularDisplacement = cross(In.AngularVelocity.xyz, rotPos) * MOTION_BLUR_DT;
    float4 localPos_dt = float4(rotPos + worldMatrix[3].xyz + angularDisplacement, 1);
    float4 WorldPos_dt = localPos_dt + float4(In.Velocity.xyz, 0) * MOTION_BLUR_DT;
    float4 Pos_dt = mul(WorldPos_dt, gSecondaryViewProjMatrix);
    Out.ScreenPos_dt = Pos_dt;
#endif // (USE_MOTION_BLUR > 0)

    Out.TexCoord = In.TexCoord;

    // TBN matrix
    Out.Normal = mul(In.Normal.xyz, (float3x3)worldMatrix);
    Out.Tangent = mul(In.Tangent.xyz, (float3x3)worldMatrix);
    Out.Binormal = cross(Out.Tangent, Out.Normal);

    return Out;
}