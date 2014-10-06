#pragma once

// TODO: this paths must be changed
#include "../nfEngine/nfCore/Engine.h"
#include "../nfEngine/nfCore/SceneEvent.h"
#include "../nfEngine/nfCore/SceneSegment.h"
#include "../nfEngine/nfCore/CollisionShape.h"
#include "../nfEngine/nfCore/Renderer.h"
#include "../nfEngine/nfCore/View.h"
#include "../nfEngine/nfCore/BVH.h"

#include "../nfEngine/nfCommon/Math/Random.h"
#include "../nfEngine/nfCommon/InputStream.h"
#include "../nfEngine/nfCommon/OutputStream.h"
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

    uint32 indicies[] = {0, 1, 2,  1, 3, 2};

    CustomMeshSubMesh submeshes[2];
    submeshes[0].m_IndexOffset = 0;
    submeshes[0].m_TrianglesCount = 1;
    submeshes[0].pMaterial = 0; //ENGINE_GET_MATERIAL("floor");

    submeshes[1].m_IndexOffset = 3;
    submeshes[1].m_TrianglesCount = 1;
    submeshes[1].pMaterial = ENGINE_GET_MATERIAL("bricks");


    Mesh* pCustomMesh = ENGINE_GET_MESH("floor_mesh");
    pCustomMesh->Create(verticies, 4,  indicies, 6,  submeshes, 2);
    //pCustomMesh->AddRef();
}
*/


class MainCameraView : public NFE::Render::View
{
public:

    MainCameraView();
    void OnPostRender(NFE::Render::IRenderContext* pCtx, NFE::Render::IGuiRenderer* pGuiRenderer);
};