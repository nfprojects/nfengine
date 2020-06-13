#include "PCH.hpp"
#include "FileInputStream.hpp"


namespace NFE {
namespace Common {

FileInputStream::FileInputStream(const StringView& path)
{
    mFile.Open(path, AccessMode::Read, false);
}

FileInputStream::~FileInputStream()
{
    mFile.Close();
}

uint64 FileInputStream::GetSize()
{
    return mFile.GetSize();
}

bool FileInputStream::Seek(int64 offset, SeekMode mode)
{
    return mFile.Seek(offset, mode);
}

size_t FileInputStream::Read(void* buffer, size_t num)
{
    return mFile.Read(buffer, num);
}

} // namespace Common
} // namespace NFE
