#include "GBuffer.h"

cbuffer Global : register(b0)
{
	float4x4 ViewMatrix;
	float4x4 ProjMatrix;
	float4x4 ViewProjMatrix;
	float4x4 SecondaryViewProjMatrix;

	float4 g_CameraVelocity;
	float4 g_CameraAngularVelocity;
	float g_dt;
};

cbuffer Material : register(b1)
{
	float4 g_DiffuseColor;
	float4 g_SpecularColor;
	float4 g_EmissionColor;
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

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 PosVS : TEXCOORD6;
	float3 WorldPos : POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : TEXCOORD1;
	float3 Tangent : TEXCOORD2;
	float3 Binormal : TEXCOORD3;
	
	float4 ScreenPos : TEXCOORD4;
	float4 ScreenPos_dt : TEXCOORD5;

	noperspective float4 Dist : Dist;
};


float ComputeDist(const float2 pos0, const float2 pos1, const float2 pos2, out uint major_dir)
{
	float2 dir = normalize(pos1 - pos0);
	float2 normal = float2(-dir.y, dir.x);
	float dist = dot(pos0, normal) - dot(pos2, normal);

	// Check major direction
	bool x_gt_y = (abs(normal.x) > abs(normal.y));

	major_dir = x_gt_y;
	return dist / (x_gt_y ? normal.x : normal.y);
}

PS_INPUT MakeVertex(const VS_OUTPUT In, const float4 Dist)
{
	PS_INPUT Out;
		Out.Pos			= In.Pos;
		Out.PosVS		= In.PosVS;
		Out.WorldPos	= In.WorldPos;
		Out.TexCoord	= In.TexCoord;
		Out.Normal		= In.Normal;
		Out.Tangent     = In.Tangent;
		Out.Binormal	= In.Binormal;
		Out.ScreenPos	= In.ScreenPos;
		Out.ScreenPos_dt= In.ScreenPos_dt;
		Out.Dist = Dist;
	return Out;
}

//---------------------------------------------------------------

[maxvertexcount(3)]
void main(in triangle VS_OUTPUT In[3], inout TriangleStream<PS_INPUT> Stream)
{
	const float4 ScaleBias = float4(800.0/2.0, -600.0/2.0, 800.0/2.0, 600.0/2.0);

	float2 pos0 = (In[0].Pos.xy / In[0].Pos.w) * ScaleBias.xy + ScaleBias.zw;
	float2 pos1 = (In[1].Pos.xy / In[1].Pos.w) * ScaleBias.xy + ScaleBias.zw;
	float2 pos2 = (In[2].Pos.xy / In[2].Pos.w) * ScaleBias.xy + ScaleBias.zw;

	uint3 major_dirs;
	float dist0 = ComputeDist(pos0, pos1, pos2, major_dirs.x);
	float dist1 = ComputeDist(pos1, pos2, pos0, major_dirs.y);
	float dist2 = ComputeDist(pos2, pos0, pos1, major_dirs.z);

	// Pass flags in last component. Add 1.0f (0x3F800000) and put something in LSB bits to give the interpolator some slack for precision.
	float major_dir = asfloat((major_dirs.x << 4) | (major_dirs.y << 5) | (major_dirs.z << 6) | 0x3F800008);

	Stream.Append(MakeVertex(In[0], float4(0, dist1, 0, major_dir)));
	Stream.Append(MakeVertex(In[1], float4(0, 0, dist2, major_dir)));
	Stream.Append(MakeVertex(In[2], float4(dist0, 0, 0, major_dir)));
}
