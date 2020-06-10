#pragma once

#include "InputStream.hpp"
#include "../../FileSystem/File.hpp"


namespace NFE {
namespace Common {

/**
 * Implementation of InputStream - file reader.
 */
class NFCOMMON_API FileInputStream : public InputStream
{
    NFE_MAKE_NONCOPYABLE(FileInputStream)
    NFE_MAKE_NONMOVEABLE(FileInputStream)

private:
    File mFile;

public:
    FileInputStream(const String& path);
    ~FileInputStream();

    uint64 GetSize() override;
    bool Seek(uint64 position) override;
    size_t Read(void* buffer, size_t num) override;
};

} // namespace Common
} // namespace NFE