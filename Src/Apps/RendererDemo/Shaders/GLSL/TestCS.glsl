// simple compute shader

layout (rgba8, set = 0, binding = 1) uniform image2D gOutputTexture;

layout (std140, row_major, set = 0, binding = 0) uniform cbuf
{
    uvec4 OutputResolution;
    vec4 ResolutionInverse;
} gParams;


layout (local_size_x = THREADS_X, local_size_y = THREADS_Y, local_size_z = 1) in;

void main()
{
    // check if texel is inside the render target
    if (any(bvec2(gl_GlobalInvocationID.x >= gParams.OutputResolution.x,
                  gl_GlobalInvocationID.y >= gParams.OutputResolution.y)))
        return;

    vec2 result = vec2(gl_GlobalInvocationID.xy) * gParams.ResolutionInverse.xy;
    imageStore(gOutputTexture, ivec2(gl_GlobalInvocationID.xy), vec4(result.x, result.y, 0.0f, 1.0f));
}
