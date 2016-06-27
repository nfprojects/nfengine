cbuffer Global : register(b0)
{
    float4x4 gCameraMatrix;
    float4x4 gViewMatrix;
    float4x4 gProjMatrix;
    float4 gViewportResInv;
    float4 gScreenScale;
};

// Binding slot 1: g-buffer
Texture2D<float> gDepthTex : register(t0);
Texture2D<float4> gGBufferTex0 : register(t1);
Texture2D<float4> gGBufferTex1 : register(t2);
Texture2D<float4> gGBufferTex2 : register(t3);
Texture2D<float4> gGBufferTex3 : register(t4);

struct VertexShaderInput
{
    float3 pos : POSITION;
};

struct VertexShaderOutput
{
    float4 pos : SV_POSITION;
    float4 viewPos : POSITION;
};

static float gMaxDepth = 10000.0f;
static float gInfinityDist = 0.999999f;