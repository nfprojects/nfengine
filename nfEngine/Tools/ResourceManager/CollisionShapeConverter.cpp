/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Collision shape converter implementation
 */

#include "PCH.hpp"
#include "MeshConverter.hpp"

#include "nfCommon/Logger.hpp"
#include "nfCommon/FileSystem.hpp"

#include "model_obj/model_obj.h"

namespace NFE {

using namespace Math;

namespace {

// replace backslashes with forward slashes
inline std::string FixTexturePath(std::string str)
{
    std::replace(str.begin(), str.end(), '\\', '/');
    return str;
}

char FloatToChar(float x)
{
    x *= 127.0f;
    x += 0.5f;

    if (x <= -128.0f) return -128;
    if (x >= 127.0f) return 127;

    return (char)x;
}

} // namespace

  // convert *.obj to *.nfcm
void ConvertFileName(const char* pInput, char* pOutput)
{
    strcpy(pOutput, pInput);

    int len = strlen(pOutput);
    for (int i = len - 1; i > 0; i--)
    {
        if (pOutput[i] == '.')
            break;

        pOutput[i] = 0;
    }

    strcat(pOutput, "nfcs");
}

bool Convert(const char* pFilePath)
{
    ModelOBJ m_Model;
    FILE* pOutput = 0;
    static char pOutPath[MAX_PATH];

    if (!m_Model.import(pFilePath, true))
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

    // write header
    ShapeHeader header;
    header.type = SHAPE_TYPE_TRI_MESH;
    fwrite(&header, sizeof(ShapeHeader), 1, pOutput);

    ShapeTraingleMesh triMeshInfo;
    triMeshInfo.trianglesCount = m_Model.getNumberOfTriangles();
    triMeshInfo.verticesCount = m_Model.getNumberOfVertices();
    fwrite(&triMeshInfo, sizeof(ShapeTraingleMesh), 1, pOutput);

    // write vertices
    const ModelOBJ::Vertex* pModelVertices = m_Model.getVertexBuffer();
    TriMeshVertex vertex;
    for (int i = 0; i < m_Model.getNumberOfVertices(); i++)
    {
        vertex.posX = pModelVertices[i].position[0];
        vertex.posY = pModelVertices[i].position[1];
        vertex.posZ = -pModelVertices[i].position[2];
        fwrite(&vertex, sizeof(vertex), 1, pOutput);
    }


    // write triangles
    const int* pModelIndicies = m_Model.getIndexBuffer();
    for (int i = 0; i < m_Model.getNumberOfMeshes(); i++)
    {
        ModelOBJ::Mesh srcMesh = m_Model.getMesh(i);

        TriMeshTriangle tri;
        tri.materialID = 0;
        for (int j = 0; j < srcMesh.triangleCount; j++)
        {
            tri.indices[0] = pModelIndicies[srcMesh.startIndex + 3 * j];
            tri.indices[1] = pModelIndicies[srcMesh.startIndex + 3 * j + 1];
            tri.indices[2] = pModelIndicies[srcMesh.startIndex + 3 * j + 2];

            fwrite(&tri, sizeof(TriMeshTriangle), 1, pOutput);
        }
    }

    fclose(pOutput);
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

} // namespace NFE
