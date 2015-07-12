// g-buffer
Texture2D<float4> gGBufferTex0 : register(t0);
Texture2D<float4> gGBufferTex1 : register(t1);
Texture2D<float4> gGBufferTex2 : register(t2);
Texture2D<float4> gGBufferTex3 : register(t3);
Texture2D<float> gDepthTex : register(t4);

cbuffer gParams : register (b0)
{
    float4 gAmbientLight;
    float4 gBackgroundColor;
};

struct VertexShaderOutput
{
    float4 Pos : SV_POSITION;
};

static float gInfinityDist = 0.999999f;

float4 main(VertexShaderOutput input) : SV_TARGET0
{
    int3 texelCoords = int3((int2)input.Pos.xy, 0);
    float depth = gDepthTex.Load(texelCoords); //depth

    float4 result = gBackgroundColor;
    if (depth < gInfinityDist)
    {
        float4 diffuseColor = gGBufferTex1.Load(texelCoords);
        result = diffuseColor * gAmbientLight;
        result += gGBufferTex2.Load(texelCoords); // emission color
    }
        
    return float4(result.xyz, 1.0f);
}