/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Definition of Vulkan function acquisition macros
 */

#pragma once

/**
 * Acquire a function from VkDevice object.
 *
 * These functions must be acquired per-Instance. Since there will be only one Vulkan Instance,
 * each functon will probably be retrieved only once.
 */
#ifndef VK_GET_INSTANCEPROC
#define VK_GET_INSTANCEPROC(inst, x) do { \
    x = (PFN_##x)vkGetInstanceProcAddr(inst, #x); \
    if (!x) \
    { \
        NFE_LOG_ERROR("Unable to retrieve Instance function " #x); \
        allExtensionsAvailable = false; \
    } \
} while(0)
#endif

/**
 * Acquire a function from VkDevice object.
 *
 * These functions must be acquired per-Device. Each device must keep its own functions.
 */
#ifndef VK_GET_DEVICEPROC
#define VK_GET_DEVICEPROC(dev, x) do { \
    x = (PFN_##x)vkGetDeviceProcAddr(dev, #x); \
    if (!x) \
    { \
        NFE_LOG_ERROR("Unable to retrieve Device function " #x); \
        allExtensionsAvailable = false; \
    } \
} while(0)
#endif
