#include "PCH.h"
#include "..\\Common\model_obj.h"

#define SHAPE_TYPE_TRI_MESH (1)

struct ShapeHeader
{
    UINT type;
    float translation[3];
    float orientation[3][3];

    ShapeHeader()
    {
        type = 0;
        translation[0] = translation[1] = translation[2] = 0.0f;

        orientation[0][0] = 1.0f;
        orientation[0][1] = 0.0f;
        orientation[0][2] = 0.0f;
        orientation[1][0] = 0.0f;
        orientation[1][1] = 1.0f;
        orientation[1][2] = 0.0f;
        orientation[2][0] = 0.0f;
        orientation[2][1] = 0.0f;
        orientation[2][2] = 1.0f;
    }
};

struct ShapeTraingleMesh
{
    UINT verticesCount;
    UINT trianglesCount;
};

struct TriMeshVertex
{
    float posX;
    float posY;
    float posZ;
};

struct TriMeshTriangle
{
    UINT indices[3];
    UINT materialID;    // unused
};

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
            tri.indices[0] = pModelIndicies[srcMesh.startIndex + 3 * j    ];
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