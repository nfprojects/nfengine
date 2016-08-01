/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of renderer module base class
 */

#pragma once

#include "RendererResources.hpp"
#include "../Engine.hpp"

namespace NFE {
namespace Renderer {

template<typename T>
class CORE_API RendererModule
{
    NFE_MAKE_NONCOPYABLE(RendererModule)
    NFE_MAKE_NONMOVEABLE(RendererModule)

    friend class HighLevelRenderer;

    // instance pointer
    static std::unique_ptr<T> mPtr;

    static void Init()
    {
        mPtr.reset(new T);
    }

    static void Release()
    {
        mPtr.reset();
    }

public:
    void Enter(RenderContext *context)
    {
        OnEnter(context);
    }

    void Leave(RenderContext *context)
    {
        OnLeave(context);
    }

    NFE_INLINE static T* Get()
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

    virtual void OnEnter(RenderContext *context)
    {
        // nothing to do by default
    }

    virtual void OnLeave(RenderContext *context)
    {
        // nothing to do by default
    }

    virtual ~RendererModule()
    {
    }
};

} // namespace Renderer
} // namespace NFE
