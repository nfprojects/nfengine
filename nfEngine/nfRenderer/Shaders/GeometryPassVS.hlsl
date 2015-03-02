#include "GBuffer.h"

cbuffer Global : register(b0)
{
	XGeometryPassGlobalCBuffer g_GlobalCB;

	/*
	float4x4 ViewMatrix;
	float4x4 ProjMatrix;
	float4x4 ViewProjMatrix;
	float4x4 SecondaryViewProjMatrix;

	float4 g_CameraVelocity;
	float4 g_CameraAngularVelocity;

	float g_dt;
	*/
}

cbuffer PerInstance : register(b1)
{
	float4x4 g_WorldMatrix;
}

struct VS_IN
{
	float3 Pos : POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Normal : NORMAL;
	float4 Tangent : TANGENT;
	
	
#if (INSTANCING > 0)
	float4 WordMat0 : TEXCOORD1;
	float4 WordMat1 : TEXCOORD2;
	float4 WordMat2 : TEXCOORD3;
	float4 Velocity : TEXCOORD4;
	float4 AngularVelocity : TEXCOORD5;
#endif
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 PosVS : TEXCOORD6;
	float4 WorldPos : POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : TEXCOORD1;
	float3 Tangent : TEXCOORD2;
	float3 Binormal : TEXCOORD3;
	
	float4 ScreenPos : TEXCOORD4;
	float4 ScreenPos_dt : TEXCOORD5;
};


//---------------------------------------------------------------

#define MOTION_BLUR_DT (0.01)

VS_OUTPUT main(VS_IN In)
{
	VS_OUTPUT Out = (VS_OUTPUT)0;
	

#if (INSTANCING > 0)	
	float4x4 worldMatrix = transpose(float4x4(In.WordMat0, In.WordMat1, In.WordMat2, float4(0, 0, 0, 1)));
#else
	float4x4 worldMatrix = g_WorldMatrix;
#endif

	//position transformation
	Out.WorldPos = mul(float4(In.Pos, 1.0f), worldMatrix);
	Out.PosVS = mul(Out.WorldPos, g_GlobalCB.ViewMatrix);
	Out.Pos = mul(Out.WorldPos, g_GlobalCB.ViewProjMatrix);
	Out.ScreenPos = Out.Pos;
	
	
#if (USE_MOTION_BLUR > 0)
	//vertex position in moment +dt
	float3 rotPos = mul(In.Pos.xyz, (float3x3)worldMatrix); //rotated vertex
	float3 angularDisplacement = cross(In.AngularVelocity.xyz, rotPos) * MOTION_BLUR_DT;
	float4 localPos_dt = float4(rotPos + worldMatrix[3].xyz + angularDisplacement, 1);

	//float3 angularDisplacement = cross(In.AngularVelocity.xyz, In.Pos) * MOTION_BLUR_DT;
	//float4 localPos_dt = mul(float4(In.Pos + angularDisplacement, 1), worldMatrix);
	
	//float4 WorldPos_dt = localPos_dt + float4(In.Velocity.xyz - g_CameraVelocity.xyz, 0.0f) * MOTION_BLUR_DT;
	float4 WorldPos_dt = localPos_dt + float4(In.Velocity.xyz, 0) * MOTION_BLUR_DT;
	float4 Pos_dt = mul(WorldPos_dt, g_GlobalCB.SecondaryViewProjMatrix);
	Out.ScreenPos_dt = Pos_dt;
#else
	Out.ScreenPos_dt = Out.Pos;
#endif
	
	
	Out.TexCoord = In.TexCoord;
	
	//TBN matrix
	Out.Normal = mul(In.Normal.xyz, (float3x3)worldMatrix);
	Out.Tangent = mul(In.Tangent.xyz, (float3x3)worldMatrix);
	Out.Binormal = cross(Out.Tangent, Out.Normal);
	
	// logarithmic depth buffer
	//float C = 0.001f;
	//float zFar = 1000000.0f;
	//Out.Pos.z = log(C*Out.Pos.w + 1) / log(C*zFar + 1) * Out.Pos.w;

	return Out;
}


