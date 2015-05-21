/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of renderer module base class
 */

#pragma once

#include "RendererResources.hpp"

namespace NFE {
namespace Renderer {

template<typename T>
class RendererModule
{
private:
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

    /// disable other constructors and assignment operators
    RendererModule(const RendererModule&) = delete;
    RendererModule(RendererModule&&) = delete;
    RendererModule& operator=(const RendererModule&) = delete;
    RendererModule& operator=(RendererModule&&) = delete;

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

    // only Init() can create
    RendererModule()
    {
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
