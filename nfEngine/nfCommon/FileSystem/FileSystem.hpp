/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Filesystem utilities declarations.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../Containers/StringView.hpp"

#include <functional>


namespace NFE {
namespace Common {

enum class PathType
{
    Invalid,   // non-existing path
    File,
    Directory,
};

class NFCOMMON_API FileSystem
{
public:
    /*
     * Callback function called during iterating a directory.
     * @param path Path found relative to root directory
     * @param isDirectory Is @p path a directory
     * @return Return "true" to continue iteration or "false" to break it.
     * @see FileSystem::Iterate
     */
    typedef std::function<bool(const String& path, PathType type)> DirIterateCallback;

    /**
     * Get parent directory of a given path.
     */
    static StringView GetParentDir(const StringView path);

    /**
     * Extract extension from path (without dot).
     */
    static StringView GetExtension(const StringView path);

    /**
     * Get location of the executable.
     */
    static String GetExecutablePath();

    /**
     * Set current working directory.
     */
    static bool ChangeDirectory(const String& path);

    /**
     * Create an empty file if not exists.
     */
    static bool TouchFile(const String& path);

    /**
     * Check if a path is a file, directory or does not exist.
     */
    static PathType GetPathType(const String& path);

    /**
     * Create a directory.
     */
    static bool CreateDir(const String& path);

    /**
     * Create a directory. Won't generate error if the directory already exists.
     */
    static bool CreateDirIfNotExist(const String& path);

    /**
     * Remove a file or a directory.
     * @param path
     * @param recursive Removes all elements in a directory recursively
     */
    static bool Remove(const String& path, bool recursive = false);

    /**
     * Copy a file or a directory.
     * @param srcPath   Source path
     * @param destPath  Destination path
     * @param overwrite Overwrite destination path
     */
    static bool Copy(const String& srcPath, const String& destPath, bool overwrite = false);

    /**
     * Move a file or a directory to another location.
     * @param srcPath   Source path
     * @param destPath  Destination path
     */
    static bool Move(const String& srcPath, const String& destPath);

    /**
     * Iterate through files and directories in a specified path.
     * @param path
     * @param callback Callback function called for each found object
     */
    static bool Iterate(const String& path, const DirIterateCallback& callback);
};

} // namespace Common
} // namespace NFE
