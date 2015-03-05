/**
 * @file    RendererD3D11.hpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Direct3D 11 rendering backend.
 */

#pragma once

#include "../RendererInterface/Device.hpp"
#include "Common.hpp"


#ifdef RENDERERD3D11_EXPORTS
#define RENDERER_API __declspec(dllexport)
#else
#define RENDERER_API __declspec(dllimport)
#endif

namespace NFE {
namespace Renderer {

class Device;

class VertexLayout : public IVertexLayout
{
    friend class CommandBuffer;
    D3DPtr<ID3D11InputLayout> mIL;

public:
    VertexLayout();
    bool Init(const VertexLayoutDesc& desc);
};

class Buffer : public IBuffer
{
    friend class CommandBuffer;

    size_t mSize;
    BufferType mType;
    BufferAccess mAccess;
    D3DPtr<ID3D11Buffer> mBuffer;

public:
    Buffer();
    bool Init(const BufferDesc& desc);
    void Write(size_t offset, size_t size, const void* data);
    void Read(size_t offset, size_t size, void* data);
};

class Sampler : public ISampler
{
public:
};

class BlendState : public IBlendState
{
    friend class CommandBuffer;
    D3DPtr<ID3D11BlendState> mBS;
public:
    bool Init(const BlendStateDesc& desc);
};

class RasterizerState : public IRasterizerState
{
    friend class CommandBuffer;
    D3DPtr<ID3D11RasterizerState> mRS;
public:
    bool Init(const RasterizerStateDesc& desc);
};

class DepthState : public IDepthState
{
    friend class CommandBuffer;
    D3DPtr<ID3D11DepthStencilState> mDS;
public:
    bool Init(const DepthStateDesc& desc);
};

class Texture : virtual public ITexture
{
    friend class RenderTarget;

    int mWidth;
    int mHeight;
    D3DPtr<ID3D11Texture2D> mTexture;
    D3DPtr<ID3D11ShaderResourceView> mSRV;

public:
};

class RenderTarget : public IRenderTarget, public Texture
{
    friend class CommandBuffer;

    HANDLE mWindow;
    D3DPtr<ID3D11RenderTargetView> mRTV;
    D3DPtr<IDXGISwapChain> mSwapChain;

public:
    int Resize(int newWidth, int newHeight);
    int Present();

    int InitSwapChain(int width, int height, HWND window);
};

class Shader : public IShader
{
    ShaderType mType;
    D3DPtr<ID3DBlob> mBytecode;

    union
    {
        ID3D11VertexShader* mVS;
        ID3D11GeometryShader* mGS;
        ID3D11HullShader* mHS;
        ID3D11DomainShader* mDS;
        ID3D11PixelShader* mPS;
        void* mGeneric;
    };

public:
    Shader();
    ~Shader();
    bool Init(const ShaderDesc& desc);
    void* GetShaderObject() const;
    ID3DBlob* GetBytecode() const;
};

class ShaderProgram : public IShaderProgram
{
    ShaderProgramDesc mDesc;
public:
    ShaderProgram(const ShaderProgramDesc& desc);
    const ShaderProgramDesc& GetDesc() const;
};

class CommandBuffer : public ICommandBuffer
{
    friend class Device;

    PrimitiveType mCurrentPrimitiveType;
    RenderTarget* mCurrentRenderTargets[MAX_RENDER_TARGETS];
    D3DPtr<ID3D11DeviceContext> mContext;
    
    ShaderProgramDesc mBoundShaders;

public:
    CommandBuffer(ID3D11DeviceContext* deviceContext);
    ~CommandBuffer();

    /// Shader resources setup methods

    void SetVertexLayout(IVertexLayout* vertexLayout);
    void SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets);
    void SetIndexBuffer(IBuffer* indexBuffer);
    void SetSamplers(ISampler** samplers, int num, ShaderType target);
    void SetTextures(ITexture** textures, int num, ShaderType target);
    void SetConstantBuffers(IBuffer** constantBuffers, int num, ShaderType target);
    void SetRenderTargets(IRenderTarget** renderTargets, int num);
    void SetShaderProgram(IShaderProgram* shaderProgram);
    void SetBlendState(IBlendState* state);
    void SetRasterizerState(IRasterizerState* state);
    void SetDepthState(IDepthState* state);
    void SetViewport(float left, float width, float top, float height,
                     float minDepth, float maxDepth);

    /// "Executive" methods

    void CopyTexture(ITexture* src, ITexture* dest);
    void Clear(const float* color);
    void Draw(PrimitiveType type, int vertexNum,
              int instancesNum = 1, int indexOffset = 0,
              int vertexOffset = 0, int instanceOffset = 0);

    void Execute(ICommandBuffer* commandBuffer, bool saveState);
};

class Device : public IDevice
{
    friend class RenderTarget;

    D3DPtr<ID3D11Device> mDevice;
    std::unique_ptr<CommandBuffer> mDefaultCommandBuffer;
    D3DPtr<IDXGIFactory> mDXGIFactory;
    D3D_FEATURE_LEVEL mFeatureLevel;

    /// resources tracking
    std::set<std::unique_ptr<RenderTarget>> mRenderTargets;
    std::set<std::unique_ptr<Shader>> mShaders;

public:
    Device();
    ~Device();

    ID3D11Device* Get() const;

    /// Resources creation functions

    IVertexLayout* CreateVertexLayout(const VertexLayoutDesc& desc);
    IBuffer* CreateBuffer(const BufferDesc& desc);
    ITexture* CreateTexture(const TextureDesc& desc);
    IRenderTarget* CreateRenderTarget(const RenderTargetDesc& desc);
    IBlendState* CreateBlendState(const BlendStateDesc& desc);
    IDepthState* CreateDepthState(const DepthStateDesc& desc);
    IRasterizerState* CreateRasterizerState(const RasterizerStateDesc& desc);
    IShader* CreateShader(const ShaderDesc& desc);
    IShaderProgram* CreateShaderProgram(const ShaderProgramDesc& desc);

    ICommandBuffer* GetDefaultCommandBuffer();
    void Execute(ICommandBuffer* commandBuffer, bool saveState);
};

// TODO: temporary, this must be replaced with nfCommon::Logger
void Log(const char* str);

int GetElementFormatSize(ElementFormat format);
DXGI_FORMAT TranslateElementFormat(ElementFormat format, int size);
D3D11_COMPARISON_FUNC TranslateComparisonFunc(CompareFunc func);

extern std::unique_ptr<Device> gDevice;

// export Device creation function
extern "C" RENDERER_API IDevice* Init();
extern "C" RENDERER_API void Release();

} // namespace Renderer
} // namespace NFE
