Texture2D<float4> gSourceTexture : register(t0);

cbuffer ToneMappingParams : register(b0)
{
    float4 gBufferInvRes;
    float4 gSeed;
    float4 gParams; // exposure, saturation, <unused>, <unused>
};

float rand(float2 coord)
{
    return frac(sin(dot(coord, float2(12.9898f, 78.233f))) * 43758.5453f);
}

float4 main(float4 pos : SV_POSITION) : SV_TARGET0
{
    int3 texelCoords = int3((int2)pos.xy, 0);
    float2 texelCoordsF = pos.xy * gBufferInvRes.xy;
    float3 color = gSourceTexture.Load(texelCoords).rgb;

    // apply saturation
    float saturationFactor = gParams.y;
    float gray = dot(color, float3(0.3f, 0.59f, 0.11f));
    color = lerp(gray.xxx, color, saturationFactor);

    // tone mapping
    float exposure = gParams.x;
    float3 result = float3(1.0f, 1.0f, 1.0f) - exp(-color * exposure);

    // apply gamma correction
    // TODO: custom gamma power
    result = sqrt(result);

    float noiseFactor = gParams.z;
    result += (rand(texelCoordsF + gSeed.xy).xxx - 0.5f) * noiseFactor;

    // TODO: dithering

    return float4(result, 1.0f);
}