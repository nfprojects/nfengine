/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of Vulkan function acquisition macros
 */

#pragma once

/**
 * Acquire a function pointer from Vulkan library.
 *
 * The macro uses nfCommon Library object to retrieve a pointer to specified function.
 * All such functions should be retrieved only once per program lifetime.
 */
#ifndef VK_GET_LIBPROC
#define VK_GET_LIBPROC(lib, x) do { \
    if (!lib.GetSymbol(#x, x)) \
        LOG_FATAL("Failed to retrieve Library function " #x); \
} while(0)
#endif

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
        LOG_ERROR("Unable to retrieve Instance function " #x); \
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
        LOG_ERROR("Unable to retrieve Device function " #x); \
        allExtensionsAvailable = false; \
    } \
} while(0)
#endif
