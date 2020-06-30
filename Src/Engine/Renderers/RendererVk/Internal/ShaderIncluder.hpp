#pragma once

#include <Engine/Common/Containers/String.hpp>
#include <Engine/Common/Containers/StringView.hpp>
#include <Engine/Common/Containers/DynArray.hpp>
#include <Engine/Common/Containers/HashMap.hpp>
#include <Engine/Common/Containers/SharedPtr.hpp>


namespace NFE {
namespace Renderer {

class ShaderIncluder: public glslang::TShader::Includer
{
    struct IncludeEntry
    {
        Common::DynArray<char> data; // included file's contents
        Common::UniquePtr<IncludeResult> ptr;
    };

    using IncludeCollection = Common::HashMap<Common::String, IncludeEntry>;

    Common::String mLocalDir;
    Common::String mSystemDir;
    IncludeCollection mLocalSearchResults;
    IncludeCollection mSystemSearchResults;

    IncludeResult* StoreFailedInclusion(IncludeCollection& collection, const Common::String& header, const Common::String& error);
    IncludeResult* IncludeGeneric(IncludeCollection& collection, const Common::String& searchPath,
                                  const char* headerName, const char* includerName, size_t inclusionDepth);

public:
    ShaderIncluder(const Common::StringView& processedFile);
    ~ShaderIncluder() = default;

    // This is called to look for shaders in local paths - files included via #include "..."
    // In our case, looks in paths relative to processed file's directory.
    // This should only look in local files - glslang will call includeSystem() if this fails.
    IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override;

    // Look in system paths - in our case, check <NFE_ROOT_DIR>/Data/Shaders
    IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override;

    // Signals that the parser will no longer use the contents of the
    // specified IncludeResult.
    void releaseInclude(IncludeResult* result) override;
};

} // namespace Renderer
} // namespace NFE
