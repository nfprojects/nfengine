#pragma once

// TODO: this paths must be changed
#include "../nfEngine/nfCore/Engine.hpp"
#include "../nfEngine/nfCore/SceneEvent.hpp"
#include "../nfEngine/nfCore/SceneSegment.hpp"
#include "../nfEngine/nfCore/CollisionShape.hpp"
#include "../nfEngine/nfCore/Renderer.hpp"
#include "../nfEngine/nfCore/View.hpp"
#include "../nfEngine/nfCore/BVH.hpp"

#include "../nfEngine/nfCommon/Math/Random.hpp"
#include "../nfEngine/nfCommon/InputStream.hpp"
#include "../nfEngine/nfCommon/OutputStream.hpp"
#include "../nfEngine/nfCommon/Timer.hpp"


/*
    This function creates an example mesh.

void CreateCustomFloorMesh()
{
    CustomMeshVertex verticies[4];
    verticies[0].Position = Float3(-50.0f, 0.0f, -50.0f);
    verticies[1].Position = Float3(50.0f, 0.0f, -50.0f);
    verticies[2].Position = Float3(-50.0f, 0.0f, 50.0f);
    verticies[3].Position = Float3(50.0f, 0.0f, 50.0f);

    verticies[0].TexCoord = Float2(-20.0f, 20.0f);
    verticies[1].TexCoord = Float2(20.0f,  20.0f);
    verticies[2].TexCoord = Float2(-20.0f, -20.0f);
    verticies[3].TexCoord = Float2(20.0f, -20.0f);

    for (int i = 0; i<4; i++)
    {
        verticies[i].Normal = Float3(0.0f, 1.0f, 0.0f);
        verticies[i].Tangent = Float3(1.0f, 0.0f, 0.0f);
    }

    uint32 indices[] = {0, 1, 2,  1, 3, 2};

    CustomMeshSubMesh submeshes[2];
    submeshes[0].m_IndexOffset = 0;
    submeshes[0].m_TrianglesCount = 1;
    submeshes[0].pMaterial = 0; //ENGINE_GET_MATERIAL("floor");

    submeshes[1].m_IndexOffset = 3;
    submeshes[1].m_TrianglesCount = 1;
    submeshes[1].pMaterial = ENGINE_GET_MATERIAL("bricks");


    Mesh* pCustomMesh = ENGINE_GET_MESH("floor_mesh");
    pCustomMesh->Create(verticies, 4,  indices, 6,  submeshes, 2);
    //pCustomMesh->AddRef();
}
*/


class MainCameraView : public NFE::Renderer::View
{
public:

    MainCameraView();
    void OnPostRender(NFE::Renderer::RenderContext* pCtx, NFE::Renderer::GuiRenderer* pGuiRenderer);
};