#pragma once

#include "RayPacket.h"


namespace NFE {
namespace RT {


// Ray stream - generator of ray packets
// Push incoherent rays, pops coherent ray packets
class RayStream
{
public:
    static constexpr uint32 MaxRays = 1024 * 1024;

    RayStream();
    ~RayStream();

    // push a new ray to the stream
    void PushRay(const Math::Ray& ray, const Math::Vec4f& weight, const ImageLocationInfo& imageLocation);

    // Convert collected rays into ray packets.
    // This will flush all the pushed rays and generate list of fresh ray packets
    void Sort();

    // Pop generated packet
    // If there's no packets pending the function returns false
    // Note: ray stream keeps the packet ownership
    bool PopPacket(RayPacket& outPacket);

private:

    struct PendingRay
    {
        Math::Vec4f rayWeight;
        Math::Vec3f rayOrigin;
        Math::Vec3f rayDir;
        ImageLocationInfo imageLocation;
    };

    uint32 mNumRays;
    PendingRay mRays[MaxRays];
};


} // namespace RT
} // namespace NFE
