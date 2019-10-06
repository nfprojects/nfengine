#include "PCH.h"
#include "RenderingParams.h"


NFE_BEGIN_DEFINE_ENUM(NFE::RT::TraversalMode)
    NFE_ENUM_OPTION(Single);
    NFE_ENUM_OPTION(Packet);
NFE_END_DEFINE_ENUM()


NFE_BEGIN_DEFINE_ENUM(NFE::RT::LightSamplingStrategy)
    NFE_ENUM_OPTION(Single);
    NFE_ENUM_OPTION(All);
NFE_END_DEFINE_ENUM()


NFE_BEGIN_DEFINE_CLASS(NFE::RT::AdaptiveRenderingSettings)
{
    NFE_CLASS_MEMBER(enable);
    NFE_CLASS_MEMBER(numInitialPasses).Min(1).Max(100);
    NFE_CLASS_MEMBER(minBlockSize).Min(1).Max(64);
    NFE_CLASS_MEMBER(maxBlockSize).Min(1).Max(64);
    NFE_CLASS_MEMBER(subdivisionTreshold).Min(1.0e-6f).Max(1.0f).LogScale(10.0f);
    NFE_CLASS_MEMBER(convergenceTreshold).Min(1.0e-6f).Max(1.0f).LogScale(10.0f);
}
NFE_END_DEFINE_CLASS()


NFE_BEGIN_DEFINE_CLASS(NFE::RT::SamplingParams)
{
    NFE_CLASS_MEMBER(dimensions).Min(1).Max(256);
    NFE_CLASS_MEMBER(useBlueNoiseDithering);
}
NFE_END_DEFINE_CLASS()


NFE_BEGIN_DEFINE_CLASS(NFE::RT::RenderingParams)
{
    NFE_CLASS_MEMBER(maxRayDepth).Min(0).Max(256);
    NFE_CLASS_MEMBER(tileSize).Min(4).Max(256);
    NFE_CLASS_MEMBER(antiAliasingSpread).Min(0.0f).Max(3.0f);
    NFE_CLASS_MEMBER(motionBlurStrength).Min(0.0f).Max(1.0f);
    NFE_CLASS_MEMBER(minRussianRouletteDepth).Min(0).Max(256);
    NFE_CLASS_MEMBER(traversalMode);
    NFE_CLASS_MEMBER(lightSamplingStrategy);
    NFE_CLASS_MEMBER(visualizeTimePerPixel);
    NFE_CLASS_MEMBER(samplingParams);
    NFE_CLASS_MEMBER(adaptiveSettings);
}
NFE_END_DEFINE_CLASS()
