#include "stdafx.hpp"
#include "Multishader.hpp"
#include "Renderer.hpp"
#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Render {

/*
class XShaderIncluder : public ID3DInclude
{
public:
    HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
    {
        char path[260];
        strcpy(path, "..\\Data\\Shaders\\");
        strcat(path, pFileName);
    }

    HRESULT Close(LPCVOID pData)
    {
    }
};
*/

void WriteD3DXBufferToFile(ID3DBlob* pBuffer, std::string& FileName)
{
    FILE* pFile = 0;
    fopen_s(&pFile, FileName.c_str(), "w");
    if (pFile)
    {
        fwrite(pBuffer->GetBufferPointer(), pBuffer->GetBufferSize(), 1, pFile);
        fclose(pFile);
    }
}

void WriteD3DXBufferToFile(ID3DBlob* pBuffer, const char* pFileName)
{
    FILE* pFile = 0;
    fopen_s(&pFile, pFileName, "w");
    if (pFile)
    {
        fwrite(pBuffer->GetBufferPointer(), pBuffer->GetBufferSize(), 1, pFile);
        fclose(pFile);
    }
}

void WriteStringToFile(const char* pString, const char* pFileName)
{
    FILE* pFile = 0;
    fopen_s(&pFile, pFileName, "w");
    if (pFile)
    {
        fwrite(pString, strlen(pString), 1, pFile);
        fclose(pFile);
    }
}

Multishader::Multishader()
{
    mType = ShaderType::Unknown;
    mCurrHash = 0;
    mRenderer = nullptr;
}

Multishader::~Multishader()
{
    shaders.clear();
    macros.clear();
}

ShaderType Multishader::GetType() const
{
    return mType;
}

bool Multishader::Init(RendererD3D11* pRenderer, ShaderType type, const char* shaderName)
{
    Common::Timer timer;
    timer.Start();

    mType = type;
    mShaderName = shaderName;
    mRenderer = pRenderer;

    std::string CompileLog;

    bool ErrorOccurred = false;
    uint32 loopIterations[X_MULTISHADER_MAX_MACROS];

    if (macros.size() > 0)
    {
        int AllCombinations = 1;
        uint32 LoopsCount = (uint32)macros.size();

        for (uint32 i = 0; i < LoopsCount; i++)
        {
            loopIterations[i] = 0;
            AllCombinations *= macros[i].maxValue + 1;
        }

        for (int i = 0; i < AllCombinations; i++)
        {
            //SubshaderInfo* pSubshader = new SubshaderInfo;
            //shaders.Add(pSubshader);

            if (!LoadSubshader(loopIterations, CompileLog))
                ErrorOccurred = true;

            (loopIterations[0])++;
            for (uint32 i = 0; i < LoopsCount - 1; i++)
            {
                if (loopIterations[i] > macros[i].maxValue)
                {
                    loopIterations[i] = 0;
                    (loopIterations[i + 1])++;
                }
            }
        }
    }
    else
    {
        ErrorOccurred |= !LoadSubshader(0, CompileLog);
    }

    std::string FileName = "Logs\\ShadersCompilation\\" + mShaderName + ".txt";
    WriteStringToFile(CompileLog.c_str(), FileName.c_str());


    if (ErrorOccurred)
    {
        LOG_ERROR("Failed to load '%s'. See 'Logs\\ShadersCompilation\\%s.txt' for more info.",
                  mShaderName.c_str(), mShaderName.c_str());
    }
    else
    {
        LOG_SUCCESS("Shader '%s' loaded in %.3lf sec.", mShaderName.c_str(), timer.Stop());
    }

    return true;
}

bool Multishader::LoadSubshader(UINT* pMacros, std::string& CompileLog)
{
    char TmpStrBuffer[1024];
    uint32 hash = 0;

    if ((macros.size() > 0) && pMacros)
    {
        for (uint32 i = 0; i < macros.size(); i++)
            hash |= pMacros[i] << macros[i].bits;
    }

    shaders[hash].hash = hash;

    std::string CacheFilePath;

    FILE* pFile = 0;
    errno_t FileError;


    ID3DBlob* pBufferErrors = 0;
    ID3DBlob* pshaderCode = 0;
    HRESULT HR;

    sprintf_s(TmpStrBuffer, 1024, "hash: %u\n", hash);
    CompileLog += TmpStrBuffer;


    //try to load VS from cache file
    std::string ShaderFilePath = "nfEngine/nfRenderer/Shaders/" + mShaderName + ".hlsl";


#ifdef _DEBUG
    sprintf_s(TmpStrBuffer, 1024, "ShaderCache_Debug\\%s_hash%u.cache", mShaderName.c_str(), hash);
#else
    sprintf_s(TmpStrBuffer, 1024, "ShaderCache\\%s_hash%u.cache", mShaderName.c_str(), hash);
#endif

    CacheFilePath = TmpStrBuffer;




    // get shader's source moddification date
    struct _stat ShaderFileStat;
    _stat(ShaderFilePath.c_str(), &ShaderFileStat);

    // get shader's cache moddification date
    struct _stat CacheFileStat;
    _stat(CacheFilePath.c_str(), &CacheFileStat);




    if (ShaderFileStat.st_mtime > CacheFileStat.st_mtime)
    {
        CompileLog += "Shader cache outdated. ";
    }
    else
    {
        FileError = fopen_s(&pFile, CacheFilePath.c_str(), "rb");

        if (FileError == 0 && pFile)
        {
            CompileLog += "Loading Shader bytecode from cache file... ";
            //Logger.AddLine(StrToChar("Loading VS from cache: "+m_ShaderName));

            char pSignature[5];
            unsigned int CodeSize;

            //read file signature
            fread(pSignature, 4, 1, pFile);
            pSignature[4] = 0;
            if (strcmp("CSHR", pSignature) != 0)
            {
                CompileLog += "[ ERROR - corrupted file ]\n";
            }
            else
            {
                fread(&CodeSize, sizeof(unsigned int), 1, pFile);

                char* pBuffer = (char*)malloc(CodeSize);
                fread(pBuffer, CodeSize, 1, pFile);
                fclose(pFile);

                //try to create shader loaded from cache
                shaders[hash].shaderCode.Load(pBuffer, CodeSize);
                free(pBuffer);

                bool shaderCreated = CreateSubshader(hash, shaders[hash].shaderCode);

                if (shaderCreated)
                {
                    CompileLog += "[ OK ]\n\n";
                    return true;
                }
                else
                {
                    //shader object cant be created, file may be corrupted, recompile shader
                    CompileLog += "[ ERROR ]\n";
                }
            }
        }
    }


    CompileLog += "Compiling Shader from source...\n";
    shaders[hash].shader = 0;




    FILE* pSrcFile = 0;
    if (fopen_s(&pSrcFile, ShaderFilePath.c_str(), "rb") != 0)
    {

        CompileLog += "[ ERROR: Can't open source file ]\n\n\n";
        return false;
    }

    fseek(pSrcFile, 0L, SEEK_END);
    UINT SrcFileSize = ftell(pSrcFile);
    fseek(pSrcFile, 0L, SEEK_SET);
    char* pSrcFileData = (char*)malloc(SrcFileSize);
    fread(pSrcFileData, SrcFileSize, 1, pSrcFile);
    fclose (pSrcFile);


    //Compilation flags
    DWORD Flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG //shaders debugging
    Flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    Flags |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
#endif

    char ProfileName[16];
    switch (mType)
    {
        case ShaderType::Vertex:
            strcpy(ProfileName, "vs_4_0");
            Flags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
            break;

        case ShaderType::Pixel:
            strcpy(ProfileName, "ps_4_1");
            Flags |= D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
            break;

        case ShaderType::Geometry:
            strcpy(ProfileName, "gs_4_0");
            Flags |= D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
            break;

        case ShaderType::Domain:
            strcpy(ProfileName, "ds_5_0");
            Flags |= D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
            break;

        case ShaderType::Hull:
            strcpy(ProfileName, "hs_5_0");
            Flags |= D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
            break;

        case ShaderType::Compute:
            if (mRenderer->featureLevel == D3D_FEATURE_LEVEL_11_0)
                strcpy(ProfileName, "cs_5_0");
            else
                strcpy(ProfileName, "cs_4_0");

            Flags |= D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
            break;
    }



    //build D3D macros table
    D3D_SHADER_MACRO* pD3DMacros = 0;
    char* pMacroDefinitions = 0;

    if (macros.size() > 0)
    {
        pD3DMacros = new D3D10_SHADER_MACRO [macros.size() + 2];
        pMacroDefinitions = new char [16 * macros.size()]; //16 chars per each macro def

        for (uint32 i = 0; i < macros.size(); i++)
        {
            sprintf(pMacroDefinitions + 16 * i, "%u", pMacros[i]);

            pD3DMacros[i].Name = macros[i].name;
            pD3DMacros[i].Definition = pMacroDefinitions + 16 * i;


            sprintf_s(TmpStrBuffer, 1024, "Macro '%s' defined to '%s'\n", macros[i].name,
                      pD3DMacros[i].Definition);
            CompileLog += TmpStrBuffer;
        }

        pD3DMacros[macros.size()].Name = "_SHADER";
        pD3DMacros[macros.size()].Definition = "";

        pD3DMacros[macros.size() + 1].Name = 0;
        pD3DMacros[macros.size() + 1].Definition = 0;
    }

    HR = D3DCompile(pSrcFileData, SrcFileSize, ShaderFilePath.c_str(), pD3DMacros,
                    D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", ProfileName, Flags, 0, &pshaderCode, &pBufferErrors);
    free(pSrcFileData);

    delete[] pD3DMacros;
    delete[] pMacroDefinitions;

    //log bledow
    if (pBufferErrors)
    {
        OutputDebugStringA((char*)pBufferErrors->GetBufferPointer());

        CompileLog += (char*)pBufferErrors->GetBufferPointer();
        CompileLog += '\n';
        D3D_SAFE_RELEASE(pBufferErrors);
    }

    //compilation failed
    if (FAILED(HR))
    {
        CompileLog += "[ D3DCompile ERROR ]\n\n\n";
        pshaderCode = 0;
        return false;
    }


    // disasemble shader and write to disk
    ID3DBlob* pDisassembly = 0;
    HR = D3DDisassemble(pshaderCode->GetBufferPointer(), pshaderCode->GetBufferSize(),
                        D3D_DISASM_ENABLE_COLOR_CODE | D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING,
                        0, &pDisassembly);

    if (SUCCEEDED(HR))
    {
        std::string DissasemblyPath = "..\\Logs\\DisassembledShaders\\" + mShaderName + "\\";
        CreateDirectoryA(DissasemblyPath.c_str(), 0);

        sprintf_s(TmpStrBuffer, 1024, "%shash%u.html", DissasemblyPath.c_str(), hash);
        std::string DissasemblyFile = TmpStrBuffer;

        WriteD3DXBufferToFile(pDisassembly, DissasemblyFile.c_str());
    }

    D3D_SAFE_RELEASE(pDisassembly);



    //write compiled shader to cache
    pFile = 0;
    fopen_s(&pFile, CacheFilePath.c_str(), "wb");
    if (pFile)
    {
        char* pSignature = "CSHR";
        uint32 CodeSize = (uint32)pshaderCode->GetBufferSize();

        fwrite(pSignature, 4, 1, pFile);
        fwrite(&CodeSize, sizeof(unsigned int), 1, pFile);
        fwrite(pshaderCode->GetBufferPointer(), CodeSize, 1, pFile);
        fclose(pFile);
    }

    D3D_SAFE_RELEASE(pBufferErrors);



    shaders[hash].shaderCode.Load(pshaderCode->GetBufferPointer(), pshaderCode->GetBufferSize());
    D3D_SAFE_RELEASE(pshaderCode);

    bool shaderCreated = CreateSubshader(hash, shaders[hash].shaderCode);

    if (!shaderCreated)
    {
        CompileLog += "[ ERROR ]\n\n";
        D3D_SAFE_RELEASE(pBufferErrors);
        return false;
    }

    CompileLog += "[ OK ]\n\n";
    return true;
}

// create successfully compiled shader
bool Multishader::CreateSubshader(UINT hash, Common::Buffer& buffer)
{
    HRESULT HR = 0;

    switch (mType)
    {
        case ShaderType::Vertex:
        {
            ID3D11VertexShader* pVS;
            HR = D3D_CHECK(mRenderer->D3DDevice->CreateVertexShader(buffer.GetData(), buffer.GetSize(), 0,
                           &pVS));
            shaders[hash].shader = pVS;
            break;
        }

        case ShaderType::Pixel:
        {
            ID3D11PixelShader* pPS;
            HR = D3D_CHECK(mRenderer->D3DDevice->CreatePixelShader(buffer.GetData(), buffer.GetSize(), 0,
                           &pPS));
            shaders[hash].shader = pPS;
            break;
        }

        case ShaderType::Geometry:
        {
            ID3D11GeometryShader* pGS;
            HR = D3D_CHECK(mRenderer->D3DDevice->CreateGeometryShader(buffer.GetData(), buffer.GetSize(),
                           0, &pGS));
            shaders[hash].shader = pGS;
            break;
        }

        case ShaderType::Domain:
        {
            ID3D11DomainShader* pDS;
            HR = D3D_CHECK(mRenderer->D3DDevice->CreateDomainShader(buffer.GetData(), buffer.GetSize(), 0,
                           &pDS));
            shaders[hash].shader = pDS;
            break;
        }

        case ShaderType::Hull:
        {
            ID3D11HullShader* pHS;
            HR = D3D_CHECK(mRenderer->D3DDevice->CreateHullShader(buffer.GetData(), buffer.GetSize(), 0,
                           &pHS));
            shaders[hash].shader = pHS;
            break;
        }

        case ShaderType::Compute:
        {
            ID3D11ComputeShader* pCS;
            HR = D3D_CHECK(mRenderer->D3DDevice->CreateComputeShader(buffer.GetData(), buffer.GetSize(), 0,
                           &pCS));
            shaders[hash].shader = pCS;
            break;
        }

        default:
        {
            shaders[hash].shader = 0;
            return false;
        }
    }

    return SUCCEEDED(HR);
}

// add macro to macros table
void Multishader::AddMacro(const char* pName, UINT MaxValue, UINT BitOffset)
{
    MultshaderMacro macro;
    macro.maxValue = MaxValue;
    macro.bits = BitOffset;
    strcpy(macro.name, pName);

    macros.push_back(macro);
}

// get buffer with compiled shader bytecode
Common::Buffer* Multishader::GetShaderBytecode(UINT* pMacroValues)
{
    UINT hash = 0;
    if (pMacroValues)
    {
        for (uint32 i = 0; i < macros.size(); i++)
            hash |= (pMacroValues[i] << macros[i].bits);
    }

    if (shaders.count(hash))
        return &(shaders[hash].shaderCode);

    return 0;
}


void Multishader::Release()
{
    std::map<UINT, SubshaderInfo>::iterator It;
    SubshaderInfo* shaderInfo;

    for (It = shaders.begin(); It != shaders.end(); It++)
    {
        shaderInfo = &(It->second);

        switch (mType)
        {
            case ShaderType::Vertex:
            {
                ID3D11VertexShader* pVS = (ID3D11VertexShader*)shaderInfo->shader;
                D3D_SAFE_RELEASE(pVS);
                break;
            }

            case ShaderType::Pixel:
            {
                ID3D11PixelShader* pVS = (ID3D11PixelShader*)shaderInfo->shader;
                D3D_SAFE_RELEASE(pVS);
                break;
            }

            case ShaderType::Geometry:
            {
                ID3D11GeometryShader* pVS = (ID3D11GeometryShader*)shaderInfo->shader;
                D3D_SAFE_RELEASE(pVS);
                break;
            }

            case ShaderType::Domain:
            {
                ID3D11DomainShader* pVS = (ID3D11DomainShader*)shaderInfo->shader;
                D3D_SAFE_RELEASE(pVS);
                break;
            }

            case ShaderType::Hull:
            {
                ID3D11HullShader* pVS = (ID3D11HullShader*)shaderInfo->shader;
                D3D_SAFE_RELEASE(pVS);
                break;
            }

            case ShaderType::Compute:
            {
                ID3D11ComputeShader* pVS = (ID3D11ComputeShader*)shaderInfo->shader;
                D3D_SAFE_RELEASE(pVS);
                break;
            }
        }
    }

    shaders.clear();
    macros.clear();
}

} // namespace Render
} // namespace NFE
