#include "PCH.hpp"
#include "Engine.hpp"
#include "Entity.hpp"
#include "Renderer/Renderer.hpp"

using namespace NFE;
using namespace NFE::Renderer;
using namespace NFE::Scene;
using namespace NFE::Resource;
using namespace NFE::Common;

class nfEngineTest : public testing::Test
{
protected:
    // preapre test case enviroment - initialize the engine
    static void SetUpTestCase()
    {
        Result ret;
        EXPECT_NO_THROW(ret = EngineInit());
        ASSERT_EQ(Result::OK, ret);

        pRenderer = EngineGetRenderer();
        ASSERT_NE(nullptr, pRenderer);

        pResourcesManager = EngineGetResManager();
        ASSERT_NE(nullptr, pResourcesManager);
    }

    // preapre test case enviroment - release the engine
    static void TearDownTestCase()
    {
        Result ret;
        EXPECT_NO_THROW(ret = EngineRelease());
        ASSERT_EQ(Result::OK, ret);

        EXPECT_NO_THROW(ret = EngineRelease());
        ASSERT_EQ(Result::AlreadyFree, ret);
    }

    static ResManager* pResourcesManager;
    static HighLevelRenderer* pRenderer;
};

/// static members definitions
ResManager* nfEngineTest::pResourcesManager = nullptr;
HighLevelRenderer* nfEngineTest::pRenderer = nullptr;


// check if calling Init second time will return a proper value
TEST_F(nfEngineTest, DoubleInit)
{
    Result ret = EngineInit();
    ASSERT_EQ(Result::AlreadyInit, ret);

    HighLevelRenderer* newRendererPtr = EngineGetRenderer();
    EXPECT_EQ(pRenderer, newRendererPtr) << "Renderer pointer should not change";

    ResManager* newResourcesManagerPtr = EngineGetResManager();
    EXPECT_EQ(pResourcesManager, newResourcesManagerPtr) <<
            "Resources Manager pointer should not change";
}

// basic scene creation / removal
TEST_F(nfEngineTest, SceneManagement)
{
    SceneManager* pScene;
    Result ret;

    EXPECT_NO_THROW(pScene = EngineCreateScene());
    ASSERT_NE(nullptr, pScene);

    EXPECT_NO_THROW(ret = EngineDeleteScene(pScene));
    EXPECT_EQ(Result::OK, ret);

    EXPECT_NO_THROW(ret = EngineDeleteScene(pScene));
    EXPECT_EQ(Result::AlreadyFree, ret);

    EXPECT_NO_THROW(ret = EngineDeleteScene(NULL));
    EXPECT_EQ(Result::AlreadyFree, ret);
}

// basic entity creation / removal
TEST_F(nfEngineTest, EntityManagement)
{
    SceneManager* pScene;
    Entity* pEntity;
    Result ret;

    EXPECT_NO_THROW(pScene = EngineCreateScene());
    ASSERT_NE(nullptr, pScene);

    EXPECT_NO_THROW(pEntity = pScene->CreateEntity());
    ASSERT_NE(nullptr, pEntity);

    EXPECT_NO_THROW(ret = pScene->DeleteEntity(pEntity));
    EXPECT_EQ(Result::OK, ret);

    EXPECT_NO_THROW(ret = pScene->DeleteEntity(pEntity));
    EXPECT_EQ(Result::Error, ret);

    EXPECT_NO_THROW(ret = pScene->DeleteEntity(NULL));
    EXPECT_EQ(Result::Error, ret);

    EXPECT_NO_THROW(ret = EngineDeleteScene(pScene));
    EXPECT_EQ(Result::OK, ret);
}


// ResManager::GetResource weird arguments test
TEST_F(nfEngineTest, GetResourceWeirdArguments)
{
    ResourceBase* pResource = nullptr;

    EXPECT_NO_THROW(pResource = pResourcesManager->GetResource(NULL, ResourceType::Unknown, true));
    EXPECT_EQ(nullptr, pResource);

    EXPECT_NO_THROW(pResource = pResourcesManager->GetResource(NULL, ResourceType::Unknown, false));
    EXPECT_EQ(nullptr, pResource);

    EXPECT_NO_THROW(pResource = pResourcesManager->GetResource(NULL, ResourceType::Texture, true));
    EXPECT_EQ(nullptr, pResource);

    EXPECT_NO_THROW(pResource = pResourcesManager->GetResource("blah", ResourceType::Unknown, true));
    EXPECT_EQ(nullptr, pResource);
}


// ResManager::GetResource weird arguments test
TEST_F(nfEngineTest, EntityComponents)
{
    SceneManager* pScene;
    Entity* pEntity;
    Result ret;
    Component* pComponent;

    EXPECT_NO_THROW(pScene = EngineCreateScene());
    ASSERT_NE(nullptr, pScene);

    EXPECT_NO_THROW(pEntity = pScene->CreateEntity());
    ASSERT_NE(nullptr, pEntity);


    EXPECT_NO_THROW(pComponent = new MeshComponent (pEntity));
    ASSERT_NE(nullptr, pComponent);

    EXPECT_NO_THROW(pComponent = new LightComponent (pEntity));
    ASSERT_NE(nullptr, pComponent);

    // doubling MeshComponent should raise an error
    EXPECT_NO_THROW(pComponent = new MeshComponent (pEntity));
    ASSERT_EQ(nullptr, pComponent);

    EXPECT_NO_THROW(ret = pScene->DeleteEntity(pEntity));
    EXPECT_EQ(Result::OK, ret);

    EXPECT_NO_THROW(ret = EngineDeleteScene(pScene));
    EXPECT_EQ(Result::OK, ret);
}

// ResManager::GetResource weird arguments test
TEST_F(nfEngineTest, EntityAttachment)
{
    SceneManager* pScene;
    Entity* pEntityA;
    Entity* pEntityB;
    Result ret;

    EXPECT_NO_THROW(pScene = EngineCreateScene());
    ASSERT_NE(nullptr, pScene);

    EXPECT_NO_THROW(pEntityA = pScene->CreateEntity());
    ASSERT_NE(nullptr, pEntityA);

    EXPECT_NO_THROW(pEntityB = pScene->CreateEntity());
    ASSERT_NE(nullptr, pEntityB);

    // verify entity A default position
    EXPECT_FLOAT_EQ(0.0f, pEntityA->GetPosition().f[0]);
    EXPECT_FLOAT_EQ(0.0f, pEntityA->GetPosition().f[1]);
    EXPECT_FLOAT_EQ(0.0f, pEntityA->GetPosition().f[2]);

    // verify entity B default position
    EXPECT_FLOAT_EQ(0.0f, pEntityB->GetPosition().f[0]);
    EXPECT_FLOAT_EQ(0.0f, pEntityB->GetPosition().f[1]);
    EXPECT_FLOAT_EQ(0.0f, pEntityB->GetPosition().f[2]);

    // attach B to A
    EXPECT_NO_THROW(pEntityA->SetPosition(Math::Vector(1.0f, 2.0f, 3.0f)));
    EXPECT_NO_THROW(ret = pEntityA->Attach(pEntityB));
    EXPECT_EQ(Result::OK, ret);

    // verify entity B position (should not change)
    EXPECT_FLOAT_EQ(0.0f, pEntityB->GetPosition().f[0]);
    EXPECT_FLOAT_EQ(0.0f, pEntityB->GetPosition().f[1]);
    EXPECT_FLOAT_EQ(0.0f, pEntityB->GetPosition().f[2]);

    // move at (0,0,0) relative to parent, so (1,2,3) in global space
    EXPECT_NO_THROW(pEntityB->SetLocalPosition(Math::Vector()));

    // verify entity B position
    EXPECT_FLOAT_EQ(1.0f, pEntityB->GetPosition().f[0]);
    EXPECT_FLOAT_EQ(2.0f, pEntityB->GetPosition().f[1]);
    EXPECT_FLOAT_EQ(3.0f, pEntityB->GetPosition().f[2]);

    EXPECT_NO_THROW(ret = pScene->DeleteEntity(pEntityA));
    EXPECT_EQ(Result::OK, ret);

    EXPECT_NO_THROW(ret = pScene->DeleteEntity(pEntityB));
    EXPECT_EQ(Result::OK, ret);

    EXPECT_NO_THROW(ret = EngineDeleteScene(pScene));
    EXPECT_EQ(Result::OK, ret);
}
