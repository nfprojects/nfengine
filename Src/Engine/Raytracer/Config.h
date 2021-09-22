#pragma once

#ifndef NFE_CONFIGURATION_FINAL

// enables runtime counting of ray-triangle and ray-box intersection tests
#define NFE_ENABLE_INTERSECTION_COUNTERS

// enables code for collecting path tracing debug data
#define NFE_ENABLE_PATH_DEBUGGING

#endif // NFE_CONFIGURATION_FINAL

// enables spectral rendering via Monte Carlo wavelength sampling
// NOTE: this slows down everything significantly
// #define NFE_ENABLE_SPECTRAL_RENDERING
