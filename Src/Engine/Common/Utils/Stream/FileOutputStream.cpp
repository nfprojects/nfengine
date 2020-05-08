#include "PCH.hpp"
#include "FileOutputStream.hpp"


namespace NFE {
namespace Common {

FileOutputStream::FileOutputStream(const String& fileName)
{
    mFile.Open(fileName, AccessMode::Write, true);
}

size_t FileOutputStream::Write(const void* buffer, size_t num)
{
    return mFile.Write(buffer, num);
}

bool FileOutputStream::Seek(uint64 offset)
{
    return mFile.Seek(offset, SeekMode::Begin);
}

} // namespace Common
} // namespace NFE
