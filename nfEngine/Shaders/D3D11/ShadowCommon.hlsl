struct VertexShaderOutput
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 screenPos : TEXCOORD1;
    float3 worldPos : TEXCOORD2;
};