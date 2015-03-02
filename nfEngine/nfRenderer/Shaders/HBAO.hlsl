//AMBIENT LIGHT - PIXEL SHADER

//g-buffer
Texture2D<float4> tex0 : register(t0);
Texture2D<float4> tex1 : register(t1);
Texture2D<float4> tex2 : register(t2);
Texture2D<float4> tex3 : register(t3);
Texture2D<float> tex_depth : register(t4);

Texture2D<float3> g_RanomDirTex : register(t5);


SamplerState g_PointSampler : register(s2);

cbuffer g_Params : register (b1)
{
	float4 g_OutResolution; //zw - inverse
	float4 g_AmbientLight;
	float4 g_BackgroundColor;
	
	float4 g_FocalLen; //zw - inverse
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
};

//---------------------------------------------------------------


#define M_PI 3.14159265f

//----------------------------------------------------------------------------------
float tangent(float3 P, float3 S)
{
    return (P.z - S.z) / length(S.xy - P.xy);
}

//----------------------------------------------------------------------------------
float3 uv_to_eye(float2 uv, float eye_z)
{
    uv = (uv * float2(2.0, -2.0) - float2(1.0, -1.0));
    return float3(uv * g_FocalLen.zw * eye_z, eye_z);
}

//----------------------------------------------------------------------------------
float3 fetch_eye_pos(float2 uv)
{
    float z = tex_depth.SampleLevel(g_PointSampler, uv, 0).x;
    return uv_to_eye(uv, z);
}

//----------------------------------------------------------------------------------
float3 tangent_eye_pos(float2 uv, float4 tangentPlane)
{
    // view vector going through the surface point at uv
    float3 V = fetch_eye_pos(uv);
    float NdotV = dot(tangentPlane.xyz, V);
    // intersect with tangent plane except for silhouette edges
    if (NdotV < 0.0) V *= (tangentPlane.w / NdotV);
    return V;
}

float length2(float3 v) { return dot(v, v); } 

//----------------------------------------------------------------------------------
float3 min_diff(float3 P, float3 Pr, float3 Pl)
{
    float3 V1 = Pr - P;
    float3 V2 = P - Pl;
    return (length2(V1) < length2(V2)) ? V1 : V2;
}

static const float g_Attenuation = 1.0f;

//----------------------------------------------------------------------------------
float falloff(float r)
{
    return 1.0f - g_Attenuation*r*r;
}

//----------------------------------------------------------------------------------
float2 snap_uv_offset(float2 uv)
{
    return round(uv * g_OutResolution.xy) * g_OutResolution.zw;
}

float2 snap_uv_coord(float2 uv)
{
    return uv - (frac(uv * g_OutResolution.xy) - 0.5f) * g_OutResolution.zw;
}

//----------------------------------------------------------------------------------
float tan_to_sin(float x)
{
    return x * rsqrt(x*x + 1.0f);
}

//----------------------------------------------------------------------------------
float3 tangent_vector(float2 deltaUV, float3 dPdu, float3 dPdv)
{
    return deltaUV.x * dPdu + deltaUV.y * dPdv;
}

//----------------------------------------------------------------------------------
float invlength(float2 v)
{
    return rsqrt(dot(v,v));
}

//----------------------------------------------------------------------------------
float tangent(float3 T)
{
    return -T.z * invlength(T.xy);
}


static const float g_AngleBias = 0.1f;

//----------------------------------------------------------------------------------
float biased_tangent(float3 T)
{
    float phi = atan(tangent(T)) + g_AngleBias;
    return tan(min(phi, M_PI*0.5));
}

static const float g_R = 1.0f;
static const float g_sqr_R = 1.0f;
static const float g_inv_R = 1.0f;

static const float g_Contrast = 1.25f;


//----------------------------------------------------------------------------------
float AccumulatedHorizonOcclusion(float2 deltaUV, float2 uv0, float3 P, float numSteps, float randstep, float3 dPdu, float3 dPdv)
{
    // Randomize starting point within the first sample distance
    float2 uv = uv0 + snap_uv_offset( randstep * deltaUV );
    
    // Snap increments to pixels to avoid disparities between xy 
    // and z sample locations and sample along a line
    deltaUV = snap_uv_offset( deltaUV );

    // Compute tangent vector using the tangent plane
    float3 T = deltaUV.x * dPdu + deltaUV.y * dPdv;

    float tanH = biased_tangent(T);
    float sinH = tanH / sqrt(1.0f + tanH*tanH);

    float ao = 0;
    for(float j = 1; j <= numSteps; ++j)
	{
        uv += deltaUV;
        float3 S = fetch_eye_pos(uv);
        
        // Ignore any samples outside the radius of influence
        float d2  = length2(S - P);
        if (d2 < g_sqr_R)
		{
            float tanS = tangent(P, S);

            [branch]
            if(tanS > tanH)
			{
                // Accumulate AO between the horizon and the sample
                float sinS = tanS / sqrt(1.0f + tanS*tanS);
                float r = sqrt(d2) * g_inv_R;
                ao += falloff(r) * (sinS - sinH);
                
                // Update the current horizon angle
                tanH = tanS;
                sinH = sinS;
            }
        } 
    }

    return ao;
}

//----------------------------------------------------------------------------------
float2 rotate_direction(float2 Dir, float2 CosSin)
{
    return float2(Dir.x*CosSin.x - Dir.y*CosSin.y, Dir.x*CosSin.y + Dir.y*CosSin.x);
}


static const float g_NumDir = 12;
static const float g_NumSteps = 16;

float4 main(VS_OUTPUT In) : SV_TARGET0
{
	int3 texelCoords = int3((int2)In.Pos.xy, 0);
	float2 fTexelCoords = In.Pos.xy * g_OutResolution.zw;
	
	
	float3 P = fetch_eye_pos(fTexelCoords);

	// Project the radius of influence g_R from eye space to texture space.
	// The scaling by 0.5 is to go from [-1,1] to [0,1].
	float2 step_size = 0.5 * g_R  * g_FocalLen.xy / P.z;

	// Early out if the projected radius is smaller than 1 pixel.
	float numSteps = min(g_NumSteps, min(step_size.x * g_OutResolution.x, step_size.y * g_OutResolution.y));
	if( numSteps < 1.0 ) return 1.0;
	step_size = step_size / ( numSteps + 1 );

	// Nearest neighbor pixels on the tangent plane
	float3 Pr, Pl, Pt, Pb;
	float4 tangentPlane;
	float3 N = tex0.SampleLevel(g_PointSampler, fTexelCoords, 0).xyz;
	N = normalize(N);
	
	tangentPlane = float4(N, dot(P, N));
	Pr = tangent_eye_pos(fTexelCoords + float2(g_OutResolution.z, 0), tangentPlane);
	Pl = tangent_eye_pos(fTexelCoords + float2(-g_OutResolution.z, 0), tangentPlane);
	Pt = tangent_eye_pos(fTexelCoords + float2(0, g_OutResolution.w), tangentPlane);
	Pb = tangent_eye_pos(fTexelCoords + float2(0, -g_OutResolution.w), tangentPlane);

	// Screen-aligned basis for the tangent plane
	float3 dPdu = min_diff(P, Pr, Pl);
	float3 dPdv = min_diff(P, Pt, Pb) * (g_OutResolution.y * g_OutResolution.z);

	// (cos(alpha),sin(alpha),jitter)
	float3 rand = float3(1, 0, 0.5); //g_RanomDirTex.Load(int3(texelCoords.x & 63, texelCoords.y & 63, 0)).xyz;

	float ao = 0;
	float d;
	float alpha = 2.0f * M_PI / g_NumDir;

	for (d = 0; d < g_NumDir; d++) 
	{
		float angle = alpha * d;
		float2 dir = float2(cos(angle), sin(angle));
		float2 deltaUV = rotate_direction(dir, rand.xy) * step_size.xy;
		ao += AccumulatedHorizonOcclusion(deltaUV, fTexelCoords, P, numSteps, rand.z, dPdu, dPdv);
	}

	return 1.0 - ao / g_NumDir * g_Contrast;

	
	
	float depth = tex_depth.Load(texelCoords); //depth
	
	float4 result = g_BackgroundColor;
	[branch] if (depth < 0.999999f)
	{
		float4 diffuseColor = tex1.Load(texelCoords);
		result = diffuseColor * g_AmbientLight;
	}
		
	return float4(result.xyz, 0.0f);
}

