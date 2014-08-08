// Enter your shader in this window
//TILE-BASED DEFERRED SHADING - COMPUTE SHADER

//some constants
#define MAX_TILES_FRUSTUMS 512
#define MAX_LIGHTS_POWER 10
#define MAX_LIGHTS (1<<MAX_LIGHTS_POWER)
#define COMPUTE_SHADER_TILE_GROUP_DIM 16
#define COMPUTE_SHADER_TILE_GROUP_SIZE (COMPUTE_SHADER_TILE_GROUP_DIM*COMPUTE_SHADER_TILE_GROUP_DIM)

//G-Buffer
Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
Texture2D tex2 : register(t2);
Texture2D tex3 : register(t3);
Texture2D tex_depth : register(t4);

//render target
RWTexture2D<float4> g_OutBuff : register(u0);

cbuffer g_GlobalParams : register (b0)
{
	uint4 g_OutResolution;
	float4x4 g_ViewProjInverseMatrix;
	float4x4 g_ProjInverseMatrix;
	float4x4 g_ViewInverseMatrix;	
	float4x4 g_ProjMatrix;
	float4x4 g_ViewTransposeMatrix;
	float4x4 g_CameraMatrix;
	float4 g_ScreenScale;

	float4 g_AmbientLight;
	float4 g_BackgroundColor;
};

cbuffer g_PerPassParams : register (b1)
{
	uint4 g_LightsCount;
};


struct PointLight
{
    float3 Position;
    float Radius;
	float RadiusInv;
    float3 Color;
};

struct SurfaceData
{
	float3 ViewSpaceRay;
	float3 PosScreen; //pixel position in screen space
	float3 PosWorld; //pixel position in world space
	float3 Normal;
	float3 DiffuseColor;
	float3 SpecularColor;
	float SpecularPower;
	float ViewDepth;
	float RawDepth;
};

struct Frustum
{
	float4 Planes[6];
};

StructuredBuffer<PointLight> g_Lights : register(t5);
StructuredBuffer<float4> g_TilesFrustums : register(t6);

//group shader variables
groupshared uint sMinZ;
groupshared uint sMaxZ;
groupshared uint sTileNumLights;
groupshared uint sTileLightIndices[MAX_LIGHTS];


float4 PlaneFromPoints(float3 p1, float3 p2, float3 p3)
{
	float3 v21 = p1 - p2;
	float3 v31 = p1 - p3;
	float3 n = normalize(cross(v21, v31));
	float d = dot(n, p1);
	return float4(n, -d);
}

int IntersectSphereFrustum(float3 SpherePos, float Radius, Frustum frustum)
{
	int inFrustum;
	float4 d;
	float4 radius = float4(Radius, Radius, Radius, Radius);
	float4 pos = float4(SpherePos, 1.0f);

	d.x = dot(frustum.Planes[0], pos);
	d.y = dot(frustum.Planes[1], pos);
	d.z = dot(frustum.Planes[2], pos);
	d.w = dot(frustum.Planes[3], pos);
	inFrustum = all(d < radius);

	d.x = dot(frustum.Planes[4], pos);
	d.y = dot(frustum.Planes[5], pos);
	inFrustum &= all(d.xy < radius.xy);

	return inFrustum;
}

/*
int IntersectSphereFrustum(float3 SpherePos, float Radius, Frustum frustum)
{
	int inFrustum = 1;
	[unroll] for (uint i = 0; i < 6; ++i)
	{
		float d = dot(frustum.Planes[i], float4(SpherePos, 1.0f));
		inFrustum &= (d < Radius);
	}
	return inFrustum;
}*/

float3 CalculateLight(SurfaceData sd, PointLight light)
{
	float3 LightVec = light.Position - sd.PosWorld;
	float LightDist = length(LightVec);
	float LightDistInv = 1.0 / LightDist;
	LightVec *= LightDistInv; //normalize
	float NdotL = dot(sd.Normal, LightVec);

	int cull = 0;
	cull |= (NdotL <= 0.0);
	cull |= (LightDist >= light.Radius);
	if (cull) return float3(0, 0, 0);
	
	float fadeOut = 1.0f - LightDist * light.RadiusInv;
	fadeOut *= fadeOut * LightDistInv * LightDistInv;
	
	//specular
	float3 Specular = float3(0, 0, 0);
	

	float3 EyeVector = normalize(sd.ViewSpaceRay);
	float3 ReflectVector = reflect(EyeVector, sd.Normal);
	float RdotL = dot(ReflectVector, LightVec);
	if (RdotL > 0)
	{
		Specular = sd.SpecularColor * pow(RdotL, sd.SpecularPower);
	}
	
	return light.Color * fadeOut * (sd.DiffuseColor * NdotL + Specular);
}

SurfaceData GetSurfaceData(uint2 viewportCoords)
{
	SurfaceData data;

	int3 texelCoords = int3(viewportCoords, 0);
	float4 color0 = tex0.Load(texelCoords); //normal
	float4 color1 = tex1.Load(texelCoords); //color
	float4 color2 = tex2.Load(texelCoords); //specular
	//float4 color3 = tex3.Load(texelCoords); //<unused>	
	float depth = tex_depth.Load(texelCoords).x; //depth
	data.RawDepth = depth;
	depth *= 1000.0f;

	float2 projectedXY = (float2)viewportCoords / (float2)g_OutResolution.xy * 2.0 - float2(1.0, 1.0); //scale to [-1 .. 1]
	projectedXY.y = -projectedXY.y;

	data.ViewSpaceRay = g_CameraMatrix[2] + g_ScreenScale.x * projectedXY.x * g_CameraMatrix[0] + g_ScreenScale.y * projectedXY.y * g_CameraMatrix[1];
	data.PosScreen = float3(projectedXY, depth); //screenPos.xyz;
	data.PosWorld = g_CameraMatrix[3].xyz + data.ViewSpaceRay * depth;
	data.Normal = color0.xyz;
	data.DiffuseColor = color1.xyz;
	data.SpecularColor = color2.xyz;
	data.SpecularPower = color2.w;
	data.ViewDepth = depth;

	return data;
}

[numthreads(COMPUTE_SHADER_TILE_GROUP_DIM, COMPUTE_SHADER_TILE_GROUP_DIM, 1)]
void main(uint3 groupId 			: SV_GroupID,
		  uint3 dispatchThreadId 	: SV_DispatchThreadID,
		  uint3 groupThreadId    	: SV_GroupThreadID)
{
	// This is currently necessary rather than just using SV_GroupIndex to work around a compiler bug on Fermi.
	uint groupIndex = groupThreadId.y * COMPUTE_SHADER_TILE_GROUP_DIM + groupThreadId.x;

	bool insideRendertarget = all(dispatchThreadId.xy < g_OutResolution.xy);
	SurfaceData sd = GetSurfaceData(dispatchThreadId.xy);	
	
    // Initialize shared memory light list and Z bounds
    if (groupIndex == 0)
	{
        sTileNumLights = 0;
        sMinZ = 0x7F7FFFFF;      // Max float
        sMaxZ = 0;
    }
    GroupMemoryBarrierWithGroupSync();
	
	if (insideRendertarget && (sd.RawDepth < 1.0f))
	{
		uint uViewDepth = asuint(sd.ViewDepth); //cast float to uint
		InterlockedMin(sMinZ, uViewDepth);
		InterlockedMax(sMaxZ, uViewDepth);
	}
    GroupMemoryBarrierWithGroupSync();

    float minTileZ = asfloat(sMinZ) - 0.0001f;
    float maxTileZ = asfloat(sMaxZ) + 0.0001f;
	
	
	// ------------------------------------------------------------------------
	// Calculate tile's frustum planes
	// ------------------------------------------------------------------------
	Frustum tileFrustum;
	
	//front, back planes - it works!!!
	tileFrustum.Planes[0] = float4(0.0f, 0.0f,  1.0f, -maxTileZ);
	tileFrustum.Planes[1] = float4(0.0f, 0.0f, -1.0f,  minTileZ);
	
	//transform frustum planes to world space
	[unroll] for (uint i = 0; i < 2; i++)
		tileFrustum.Planes[i] = mul(tileFrustum.Planes[i], g_ViewTransposeMatrix);
	
	tileFrustum.Planes[2] = g_TilesFrustums[groupId.x]; //left plane
	tileFrustum.Planes[3] = -g_TilesFrustums[groupId.x+1]; //right plane
	tileFrustum.Planes[4] = g_TilesFrustums[MAX_TILES_FRUSTUMS/2 + groupId.y]; //top plane
	tileFrustum.Planes[5] = -g_TilesFrustums[MAX_TILES_FRUSTUMS/2 + groupId.y+1]; //bottom plane
	


	// TODO: cull lights for the tile
	uint totalLights = min(MAX_LIGHTS, g_LightsCount.x);
	for (uint lightIndex = groupIndex; lightIndex < totalLights; lightIndex += COMPUTE_SHADER_TILE_GROUP_SIZE)
	{
		PointLight light = g_Lights[lightIndex];

		//sphere vs tile frustum
		[branch] if (IntersectSphereFrustum(light.Position, light.Radius, tileFrustum))
		{
			// Append light to group-shared list
			uint listIndex;
			InterlockedAdd(sTileNumLights, 1, listIndex);
			sTileLightIndices[listIndex] = lightIndex;
		}
	}
	GroupMemoryBarrierWithGroupSync();
	uint numLights = sTileNumLights;
	

	// Only process onscreen pixels (tiles can span screen edges)
	if (insideRendertarget)
	{
		float3 result;

		if (sd.RawDepth < 1.0f)
		{
			// evaluate lighting equations for visible light in the tile
			result = sd.DiffuseColor * g_AmbientLight; //ambient light
			for (uint i = 0; i<numLights; ++i)
			{
				PointLight light = g_Lights[sTileLightIndices[i]];
				result += CalculateLight(sd, light);
			}
		}
		else
		{
			result = g_BackgroundColor;
		}


		g_OutBuff[dispatchThreadId.xy] = float4(result, 0);
		//g_OutBuff[dispatchThreadId.xy] = float4((float)numLights * 0.05f, 0, 0, 0);
	}
}