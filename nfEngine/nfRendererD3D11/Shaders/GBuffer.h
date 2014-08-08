
#ifdef CORE_EXPORTS
X_ALIGN(16)
#endif
struct XGeometryPassGlobalCBuffer
{
    float4x4 ViewMatrix;
    float4x4 ProjMatrix;
    float4x4 ViewProjMatrix;
    float4x4 SecondaryViewProjMatrix;

    float4 CameraVelocity;
    float4 CameraAngularVelocity;

    float DeltaTime;
};
