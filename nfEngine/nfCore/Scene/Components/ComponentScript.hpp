/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of script component class.
 */

#pragma once

#include "../../Core.hpp"
#include "Component.hpp"
#include "../Events/Event.hpp"

#include <memory>


// TODO executing scripts from script resource
// scripting engine integration (Lua, Python, C#, etc.) will be required


namespace NFE {
namespace Scene {

/**
 * Base class for native (C++) scripts.
 * This is temporary - it's easier to hardcode scripts in C++ for now.
 */
class CORE_API NativeScript
{
public:
    NFE_INLINE ~NativeScript() { }

    virtual void OnEvent(const Event& event) = 0;

    Entity* GetEntity() const;
};

using NativeScriptPtr = std::unique_ptr<NativeScript>;


/**
 * A component allowing for adding logic to an entity.
 */
class CORE_API ScriptComponent : public IComponent
{
public:
    ScriptComponent();
    ~ScriptComponent();

    /**
     * Set a new native script object.
     */
    bool SetNativeScript(NativeScriptPtr script);

private:
    NativeScriptPtr mNativeScript;

    // TODO event filtering
};

} // namespace Scene
} // namespace NFE
