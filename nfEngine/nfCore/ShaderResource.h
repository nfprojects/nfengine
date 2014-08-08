#pragma once
#include "Core.h"
#include "RendererContext.h"

class IShaderResource
{
protected:
    bool                m_isValid;
    bool                m_updateOnBind;
    ShaderType          m_shaderType;
    IRenderContext*  m_lastContext;

public:
    IShaderResource()
    {
        m_isValid = false;
        m_updateOnBind = false;
        m_shaderType = UNKNOWN_SHADER;
        m_lastContext = nullptr;
    }

    virtual void Update() = 0;
    virtual void Bind(IRenderContext* pContext) = 0;
    virtual void Unbind() = 0;
};

class ShaderCBuffer : public IShaderResource
{
    void*           m_ptr;
    size_t          m_size;
    ID3D11Buffer*    m_buffer;
    uint32          m_register;
public:
    bool Init(uint32 reg, ShaderType shaderType, void* ptr, size_t size, bool updateOnBind = true);
    void Release();

    void Update();
    void Bind(IRenderContext* pContext);
    void Unbind();
};