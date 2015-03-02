//MOTION BLUR - PIXEL SHADER

Texture2D<float4> g_Source : register(t0);
Texture2D<float2> g_EdgeBuffer : register(t1);
SamplerState g_LinearSampler : register(s2);




struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
};

//---------------------------------------------------------------

float4 main(VS_OUTPUT In) : SV_TARGET0
{
	int3 texelCoords = int3((int2)In.Pos.xy, 0);
	float2 offset = g_EdgeBuffer.Load(texelCoords);

	

	// Check geometry buffer for an edge cutting through the pixel.
	[flatten]
	if (min(abs(offset.x), abs(offset.y)) >= 0.5)
	{
		// If no edge was found we look in neighboring pixels' geometry information. This is necessary because
		// relevant geometry information may only be available on one side of an edge, such as on silhouette edges,
		// where a background pixel adjacent to the edge will have the background's geometry information, and not
		// the foreground's geometric edge that we need to antialias against. Doing this step covers up gaps in the
		// geometry information.

		offset = 0.5f;

		// We only need to check the component on neighbor samples that point towards us
		float offset_x0 = g_EdgeBuffer.Load(texelCoords + int3(-1,  0, 0)).x;
		float offset_x1 = g_EdgeBuffer.Load(texelCoords + int3( 1,  0, 0)).x;
		float offset_y0 = g_EdgeBuffer.Load(texelCoords + int3( 0, -1, 0)).y;
		float offset_y1 = g_EdgeBuffer.Load(texelCoords + int3( 0,  1, 0)).y;

		// Check range of neighbor pixels' distance and use if edge cuts this pixel.
		if (abs(offset_x0 - 0.75f) < 0.25f) offset = float2(offset_x0 - 1.0f, 0.5f); // Left  x-offset [ 0.5 ..  1.0] cuts this pixel
		if (abs(offset_x1 + 0.75f) < 0.25f) offset = float2(offset_x1 + 1.0f, 0.5f); // Right x-offset [-1.0 .. -0.5] cuts this pixel
		if (abs(offset_y0 - 0.75f) < 0.25f) offset = float2(0.5f, offset_y0 - 1.0f); // Up    y-offset [ 0.5 ..  1.0] cuts this pixel
		if (abs(offset_y1 + 0.75f) < 0.25f) offset = float2(0.5f, offset_y1 + 1.0f); // Down  y-offset [-1.0 .. -0.5] cuts this pixel
	}

	// Convert distance to texture coordinate shift
	float2 off = (offset >= float2(0, 0))? float2(0.5f, 0.5f) : float2(-0.5f, -0.5f);
	offset = off - offset;

	// Blend pixel with neighbor pixel using texture filtering and shifting the coordinate appropriately.
	float2 PixelSize = float2(1.0f/1920.0, 1.0/1080.0);

	return g_Source.Sample(g_LinearSampler, (In.Pos + offset) * PixelSize);
}

