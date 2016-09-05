// simple compute shader

RWTexture2D<float4> gOutputTexture : register(u0);

cbuffer gParams : register (b0)
{
    uint4 OutputResolution;
    float4 ResolutionInverse;
};

[numthreads(THREADS_X, THREADS_Y, 1)]
void main(uint3 threadIDInGroup : SV_GroupThreadID,
          uint3 groupID : SV_GroupID,
          uint3 dispatchThreadID : SV_DispatchThreadID,
          uint groupIndex : SV_GroupIndex)
{
    // check if texel is inside the render target
    if (any(dispatchThreadID.xy >= OutputResolution.xy))
        return;

    float2 result = (float2)dispatchThreadID * ResolutionInverse.xy;
    gOutputTexture[dispatchThreadID.xy] = float4(result.xy, 0.0f, 1.0f);
}
