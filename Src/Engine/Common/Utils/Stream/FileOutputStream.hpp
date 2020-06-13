#pragma once

#include "OutputStream.hpp"
#include "../../FileSystem/File.hpp"


namespace NFE {
namespace Common {

/**
 * Implementation of OutputStream - file writer.
 */
class NFCOMMON_API FileOutputStream : public OutputStream
{
    NFE_MAKE_NONCOPYABLE(FileOutputStream)
    NFE_MAKE_NONMOVEABLE(FileOutputStream)

private:
    File mFile;

public:
    FileOutputStream(const StringView& fileName);

    virtual size_t Write(const void* buffer, size_t num) override;
    virtual bool Seek(int64 offset, SeekMode mode) override;
    virtual uint64 GetPosition() const override;
};

} // namespace Common
} // namespace NFE
