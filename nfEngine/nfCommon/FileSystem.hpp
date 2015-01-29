/**
 * @file   FileSystem.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Filesystem utilities declarations.
 */

#pragma once
#include "nfCommon.hpp"

namespace NFE {
namespace Common {

class NFCOMMON_API FileSystem
{
public:
    typedef std::function<void(const std::string& path)> DirChangeNotifyCallback;

    typedef std::function<bool(const std::string& path, bool isDirectory)> DirListCallback;

    /**
     * Set current working directory.
     */
    static bool ChangeDirectory(const std::string& path);

    /**
     * Create a directory.
     */
    static bool CreateDir(const std::string& path);

    /**
     * Remove a file or a directory.
     */
    static bool Remove(const std::string& path, bool recursive = false);

    /**
     * Copy a file or a directory.
     */
    static bool Copy(const std::string& srcPath, const std::string& destPath,
                     bool overwrite = false);

    /**
     * Move a file or a directory to another location.
     */
    static bool Move(const std::string& srcPath, const std::string& destPath);

    /**
     * List files and directories in a specified path.
     * TODO: callbacks
     */
    static bool List(const std::string& path, DirListCallback callback);
};

} // namespace Common
} // namespace NFE
