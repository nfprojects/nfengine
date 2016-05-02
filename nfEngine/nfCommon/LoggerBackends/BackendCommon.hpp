/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Common logger backends definitions
 */

#pragma once

#define NFE_MAX_LOG_MESSAGE_LENGTH 1024

/// snprintf was not implemented until Visual Studio 2015
#if defined(WIN32) && defined(_MSC_VER)
#if _MSC_VER <= 1800
#define snprintf sprintf_s
#endif
#endif