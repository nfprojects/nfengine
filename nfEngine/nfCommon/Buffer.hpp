/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Buffer class declaration.
 */

#pragma once
#include "nfCommon.hpp"

namespace NFE {
namespace Common {

/**
 * Simple static data buffer
 */
class NFCOMMON_API Buffer
{
private:
    size_t mSize;
    void* mData;

public:
    Buffer();
    Buffer(const Buffer& src);
    Buffer(Buffer&& other); // move constructor
    Buffer& operator= (const Buffer& src);
    ~Buffer();

    /**
     * Allocate memory
     * @param size Number of bytes to allocate
     */
    // TODO: return something
    void Create(size_t size);

    /**
     * Allocate memory and load data
     * @param pData Source pointer
     * @param size Number of bytes to allocate and copy
     */
    void Load(const void* data, size_t size);

    /**
     * Free memory
     */
    void Release();

    size_t GetSize() const;

    /**
     * @return NULL if not allocated
     */
    void* GetData() const;
};

} // namespace Common
} // namespace NFE
