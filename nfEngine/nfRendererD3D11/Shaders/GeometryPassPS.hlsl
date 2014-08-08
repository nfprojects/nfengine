// Geometry Pass - Pixel Shader

#include "GBuffer.h"

// Layer 0
Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D SpecularTexture : register(t2);
Texture2D EmissionTexture : register(t3);


SamplerState tex_sampler : register(s0);

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
	float3 WorldPos : POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : TEXCOORD1;
	float3 Tangent : TEXCOORD2;
	float3 Binormal : TEXCOORD3;
	
	float4 ScreenPos : TEXCOORD4;
	float4 ScreenPos_dt : TEXCOORD5;

#if (ANTIALIASING > 0)
	noperspective float4 Dist : Dist;
#endif

};

struct PS_OUTPUT
{
	float depth : SV_Depth;
	float4 color0 : SV_TARGET0;
	float4 color1 : SV_TARGET1;
	float4 color2 : SV_TARGET2;
	float2 color3 : SV_TARGET3;
};

//---------------------------------------------------------------

#define MOTION_BLUR_DT (0.01)

PS_OUTPUT main(VS_OUTPUT In)
{
	PS_OUTPUT Out = (PS_OUTPUT)0;
	Out.depth = In.PosVS.z / 1000.0f; //view-space depth

	// --- GEOMETRY BUFFER LAYOUT ---
	// DSV  [float32]	: depth
	// TEX0 [float16]	: normal.x	| normal.y	| normal.z	| spec.factor
	// TEX1 [float16]	: color.r	| color.g	| color.b	| spec.power
	// TEX2 [float16]	: emiss.r	| emiss.g	| emiss.b	| *
	// TEX3 [float16]	: motion.u	| motion.v	| *			| *

	float4 diffuseColor = g_DiffuseColor * DiffuseTexture.Sample(tex_sampler, In.TexCoord);
	clip(diffuseColor.a - 0.5);

#if (GAMMA_CORRECTION > 0)	
	diffuseColor.xyz *= diffuseColor.xyz;
#endif

	Out.color1 = float4(diffuseColor.xyz, g_SpecularColor.w); //difuse color + spec power

	//specular mapping
	float specularFactor = SpecularTexture.Sample(tex_sampler, In.TexCoord).r * g_SpecularColor.r;


	//normal mapping
	float3 Normal = In.Normal;
//#if (USE_NORMAL_TEXTURE > 0)
	float3 NormalMapValue = 2 * NormalTexture.Sample(tex_sampler, In.TexCoord).rgb - float3(1.0f, 1.0f, 1.0f);
	NormalMapValue.y *= -1;
	float3x3 TBN = transpose(float3x3(In.Tangent, In.Binormal, Normal));
	Normal = normalize(mul(TBN, NormalMapValue));
//#endif	

	//normal
	Out.color0 = float4(Normal, specularFactor);


	Out.color2 = float4(g_EmissionColor.xyz, 0);

	
//#if (USE_MOTION_BLUR > 0)
	//calculate screen space motion vector
	float3 screenPos = In.ScreenPos.xyz / In.ScreenPos.w;
	float3 screenPos_dt = In.ScreenPos_dt.xyz / In.ScreenPos_dt.w;
	float3 ScreenVelocity = (screenPos_dt - screenPos);
	ScreenVelocity.y = -ScreenVelocity.y;

	//store motion vector
	Out.color3 = ScreenVelocity.xy / MOTION_BLUR_DT;

	return Out;
}

