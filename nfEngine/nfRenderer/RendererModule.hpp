/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of renderer module base class
 */

#pragma once

#include "RendererResources.hpp"


namespace NFE {
namespace Renderer {

template<typename ModuleType, typename ContextType>
class NFE_RENDERER_API RendererModule
{
    NFE_MAKE_NONCOPYABLE(RendererModule)
    NFE_MAKE_NONMOVEABLE(RendererModule)

    friend class HighLevelRenderer;

private:
    // instance pointer
    static std::unique_ptr<ModuleType> mPtr;

    static void Init()
    {
        mPtr.reset(new ModuleType);
    }

    static void Release()
    {
        mPtr.reset();
    }

public:
    virtual void OnEnter(ContextType* context)
    {
        // nothing to do by default
        NFE_UNUSED(context);
    }

    virtual void OnLeave(ContextType* context)
    {
        // nothing to do by default
        NFE_UNUSED(context);
    }

    NFE_INLINE static ModuleType* Get()
    {
        return mPtr.get();
    }

protected:

    HighLevelRenderer* mRenderer;

    // only Init() can create
    RendererModule()
    {
        mRenderer = Engine::GetInstance()->GetRenderer();
    }

    virtual ~RendererModule()
    {
    }
};

} // namespace Renderer
} // namespace NFE
