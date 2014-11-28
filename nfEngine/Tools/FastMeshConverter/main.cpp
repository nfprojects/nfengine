#include "stdafx.hpp"
#include "model_obj.h"

#define MAT_NAME_MAX_LENGTH (120)

struct XVertex
{
    float pos[3];
    float texCoord[2];

    char normal[4];
    char tangent[4];
};

struct SubMesh
{
    int indexOffset;
    int triangleCount;
    char materialName[MAT_NAME_MAX_LENGTH];
};

// replace backslashes with forward slashes
inline std::string FixTexturePath(std::string str)
{
    std::replace(str.begin(), str.end(), '\\', '/');
    return str;
}

void ConvertFileName(const char* pInput, char* pOutput)
{
    strcpy(pOutput, pInput);

    size_t len = strlen(pOutput);
    for (size_t i = len - 1; i > 0; i--)
    {
        if (pOutput[i] == '.')
            break;

        pOutput[i] = 0;
    }

    strcat(pOutput, "nfm");
}

void ExtractFileDir(const char* pFilePath, char* pDir)
{
    strcpy(pDir, pFilePath);

    size_t len = strlen(pFilePath);
    for (size_t i = len - 1; i > 0; i--)
    {
        if (pDir[i] == '\\')
            break;

        pDir[i] = 0;
    }
}


#define ISNAN(x)  ((*(UINT*)&(x) & 0x7F800000) == 0x7F800000 && (*(UINT*)&(x) & 0x7FFFFF) != 0)
#define ISINF(x)  ((*(UINT*)&(x) & 0x7FFFFFFF) == 0x7F800000)

/*
bool IsNaN(float a)
{
    union
    {
        float f;
        UINT u;
    } num;
    num.f = a;

    return ((num.u & 0x7F800000) == 0x7F800000) && ((num.u & 0x7FFFFF) != 0);
}
*/

char FloatToChar(float x)
{
    x *= 127.0f;
    x += 0.5f;

    if (x <= -128.0f) return -128;
    if (x >= 127.0f) return 127;

    return (char)x;
}

bool FileExists(char filename[])
{
    WIN32_FIND_DATAA FileFindData;
    HANDLE handle = FindFirstFileA(filename, &FileFindData);
    if (handle != INVALID_HANDLE_VALUE)
    {
        FindClose(handle);
        return true;
    }
    else return false;
}

bool Convert(const char* pFilePath)
{
    ModelOBJ model;
    FILE* pOutput = 0;
    SubMesh subMesh;
    static char pOutPath[MAX_PATH];

//open mesh
    if (!model.import(pFilePath))
    {
        printf("Failed to open file '%s'!\n", pFilePath);
        return false;
    }

    //open output file
    ConvertFileName(pFilePath, pOutPath);
    if (fopen_s(&pOutput, pOutPath, "wb") != 0)
    {
        printf("Could not open output file '%s'!\n", pOutPath);
        return false;
    }


    //write basic info
    int verticesCount = model.getNumberOfVertices();
    int indiciesCount = model.getNumberOfIndices();
    int subMeshesCount = model.getNumberOfMeshes();

    printf("Veritces count: %i\n", verticesCount);
    printf("Indicies count: %i\n", indiciesCount);
    printf("Submeshes count: %i\n", subMeshesCount);

    fwrite("nfm", 4, 1, pOutput); //signature
    fwrite(&verticesCount, sizeof(int), 1, pOutput);
    fwrite(&indiciesCount, sizeof(int), 1, pOutput);
    fwrite(&subMeshesCount, sizeof(int), 1, pOutput);



    //write verticies
    XVertex* pVertices = (XVertex*)malloc(verticesCount * sizeof(XVertex));
    const ModelOBJ::Vertex* pModelVertices = model.getVertexBuffer();
    for (int i = 0; i < verticesCount; i++)
    {
        //vertex position
        pVertices[i].pos[0] = pModelVertices[i].position[0];
        pVertices[i].pos[1] = pModelVertices[i].position[1];
        pVertices[i].pos[2] = -pModelVertices[i].position[2];

        //vertex texture coordinates
        pVertices[i].texCoord[0] = pModelVertices[i].texCoord[0];
        pVertices[i].texCoord[1] = pModelVertices[i].texCoord[1];

        //vertex normal
        if (ISNAN(pModelVertices[i].normal[0]) || ISNAN(pModelVertices[i].normal[1]) ||
                ISNAN(pModelVertices[i].normal[2]))
        {
            pVertices[i].normal[0] = 0;
            pVertices[i].normal[1] = 127;
            pVertices[i].normal[2] = 0;
        }
        else
        {
            pVertices[i].normal[0] = FloatToChar(pModelVertices[i].normal[0]);
            pVertices[i].normal[1] = FloatToChar(pModelVertices[i].normal[1]);
            pVertices[i].normal[2] = FloatToChar(-pModelVertices[i].normal[2]);
        }

        //vertex tangent
        if (ISNAN(pModelVertices[i].tangent[0]) || ISNAN(pModelVertices[i].tangent[1]) ||
                ISNAN(pModelVertices[i].tangent[2]))
        {
            pVertices[i].tangent[0] = 127;
            pVertices[i].tangent[1] = 0;
            pVertices[i].tangent[2] = 0;
        }
        else
        {
            pVertices[i].tangent[0] = FloatToChar(pModelVertices[i].tangent[0]);
            pVertices[i].tangent[1] = FloatToChar(pModelVertices[i].tangent[1]);
            pVertices[i].tangent[2] = FloatToChar(-pModelVertices[i].tangent[2]);
        }

        pVertices[i].normal[3] = 0;
        pVertices[i].tangent[3] = 0;
    }
    fwrite(pVertices, sizeof(XVertex), verticesCount, pOutput);
    free(pVertices);


    //write indicies
    fwrite(model.getIndexBuffer(), sizeof(int), indiciesCount, pOutput);



    //write submeshes
    UINT triCounter = 0;
    for (int i = 0; i < model.getNumberOfMeshes(); i++)
    {
        ModelOBJ::Mesh srcMesh = model.getMesh(i);
        subMesh.indexOffset = srcMesh.startIndex;
        subMesh.triangleCount = srcMesh.triangleCount;
        ZeroMemory(subMesh.materialName, MAT_NAME_MAX_LENGTH);
        strcpy(subMesh.materialName, srcMesh.pMaterial->name.c_str());

        fwrite(&subMesh, sizeof(SubMesh), 1, pOutput);
    }

    //close output file
    fclose(pOutput);


    //generate material files
    int matCount = model.getNumberOfMaterials();
    for (int i = 0; i < matCount; i++)
    {
        const ModelOBJ::Material& mat = model.getMaterial(i);

        char fileName[MAX_PATH];
        ExtractFileDir(pFilePath, fileName);
        strcat(fileName, "..\\Materials\\");
        strcat(fileName, mat.name.c_str());
        strcat(fileName, ".json");

        //check if material already exists
        if (FileExists(fileName))
        {
            printf("Material %s already exists. Skipping material file generation...\n", mat.name.c_str());
            continue;
        }

        /// TODO: This is temporary. Material file generation must be redesigned.
        FILE* pMatFile = fopen(fileName, "w");
        fprintf(pMatFile, "{\n\t\"Layers\" :\n\t[\n\t\t{\n");

        if (mat.colorMapFilename.length())
            fprintf(pMatFile, "\t\t\t\"DiffuseTexture\" : \"%s\"",
                    FixTexturePath(mat.colorMapFilename).c_str());

        if (mat.colorMapFilename.length() > 0 && mat.bumpMapFilename.length() > 0)
            fprintf(pMatFile, ",\n");
        else
            fprintf(pMatFile, "\n");

        if (mat.bumpMapFilename.length())
            fprintf(pMatFile, "\t\t\t\"NormalTexture\" : \"%s\"\n",
                    FixTexturePath(mat.bumpMapFilename).c_str());

        fprintf(pMatFile, "\t\t}\n\t]\n}\n");
        fclose(pMatFile);
    }

    return true;
}

int main(int argc, char* argv[])
{
    //check parameters
    if (argc < 2)
    {
        printf("You need to specify input file(s) in application parameters!\n");
        system("pause");
        return 0;
    }

    for (int i = 1; i < argc; i++)
        Convert(argv[i]);

    system("pause");
    return 0;
}