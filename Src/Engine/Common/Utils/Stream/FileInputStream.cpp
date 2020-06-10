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

bool FileInputStream::Seek(uint64 position)
{
    return mFile.Seek(position, SeekMode::Begin);
}

size_t FileInputStream::Read(void* buffer, size_t num)
{
    return mFile.Read(buffer, num);
}

} // namespace Common
} // namespace NFE
