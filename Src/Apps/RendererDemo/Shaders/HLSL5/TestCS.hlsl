// simple compute shader

[[vk::binding(0, 0)]]
cbuffer gParams : register (b0)
{
    uint4 OutputResolution;
    float4 ResolutionInverse;
};

#if FILL_INDIRECT_ARG_BUFFER == 1

[[vk::binding(1, 0)]]
RWBuffer<uint> gOutputIndirectArgBuffer : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 threadIDInGroup : SV_GroupThreadID,
    uint3 groupID : SV_GroupID,
    uint3 dispatchThreadID : SV_DispatchThreadID,
    uint groupIndex : SV_GroupIndex)
{
    if (all(dispatchThreadID.xy == uint2(0, 0)))
    {
        gOutputIndirectArgBuffer[0] = (OutputResolution.x + THREAD_GROUP_SIZE - 1) / THREAD_GROUP_SIZE;
        gOutputIndirectArgBuffer[1] = (OutputResolution.y + THREAD_GROUP_SIZE - 1) / THREAD_GROUP_SIZE;
        gOutputIndirectArgBuffer[2] = 1;
    }
}

#else // FILL_INDIRECT_ARG_BUFFER == 0

[[vk::binding(1, 0)]]
RWTexture2D<unorm float4> gOutputTexture : register(u0);

[numthreads(THREAD_GROUP_SIZE, THREAD_GROUP_SIZE, 1)]
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

#endif // FILL_INDIRECT_ARG_BUFFER
