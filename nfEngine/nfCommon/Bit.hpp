/**
 * @file   Bit.hpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  File with defined bitwise operations
 */

#pragma once

#define SWAP_ENDIANNESS(x) do {                                     \
        x = ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >>  8) |   \
            ((x & 0x0000FF00) << 8)  | ((x & 0x000000FF) << 24);    \
    } while (0)

