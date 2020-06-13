#include "PCH.hpp"
#include "FileOutputStream.hpp"


namespace NFE {
namespace Common {

FileOutputStream::FileOutputStream(const StringView& fileName)
{
    mFile.Open(fileName, AccessMode::Write, true);
}

size_t FileOutputStream::Write(const void* buffer, size_t num)
{
    return mFile.Write(buffer, num);
}

uint64 FileOutputStream::GetPosition() const
{
    return mFile.GetPos();
}

bool FileOutputStream::Seek(int64 offset, SeekMode mode)
{
    return mFile.Seek(offset, mode);
}

} // namespace Common
} // namespace NFE
