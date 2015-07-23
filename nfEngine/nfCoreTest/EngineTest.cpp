#include "PCH.hpp"
#include "Engine.hpp"
#include "Entity.hpp"
#include "Renderer/HighLevelRenderer.hpp"

#include "Components/TransformComponent.hpp"
#include "Components/MeshComponent.hpp"
#include "Components/BodyComponent.hpp"

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
    SceneManager* scene;
    Result ret;
    std::vector<EntityID> entities;

    EXPECT_NO_THROW(scene = EngineCreateScene());
    ASSERT_NE(nullptr, scene);

    EntityManager& em = scene->GetEntityManager();

    const int entityNum = 10000;
    for (int i = 0; i < entityNum; ++i)
        entities.push_back(em.CreateEntity());

    for (int i = 0; i < entityNum / 2; ++i)
        ASSERT_TRUE(em.RemoveEntity(entities[i]));

    for (int i = 0; i < entityNum / 2; ++i)
        ASSERT_FALSE(em.RemoveEntity(entities[i]));

    for (int i = entityNum / 2; i < entityNum; ++i)
        ASSERT_TRUE(em.RemoveEntity(entities[i]));

    for (int i = entityNum / 2; i < entityNum; ++i)
        ASSERT_FALSE(em.RemoveEntity(entities[i]));

    EXPECT_NO_THROW(ret = EngineDeleteScene(scene));
    EXPECT_EQ(Result::OK, ret);
}

// basic entity creation / removal
TEST_F(nfEngineTest, EntityComponents)
{
    SceneManager* scene;
    Result ret;
    std::set<EntityID> entitiesA, entitiesB, entitiesC;
    std::set<EntityID> tmpEntites;

    EXPECT_NO_THROW(scene = EngineCreateScene());
    ASSERT_NE(nullptr, scene);

    EntityManager& em = scene->GetEntityManager();

    const int entityNum = 3000;

    // attach TransformComponent to components in group A
    for (int i = 0; i < entityNum; ++i)
    {
        EntityID entity = em.CreateEntity();
        entitiesA.insert(entity);
        ASSERT_TRUE(em.AddComponent<TransformComponent>(entity, TransformComponent()));
    }

    // attach TransformComponent and BodyComponent to components in group A
    for (int i = 0; i < entityNum; ++i)
    {
        EntityID entity = em.CreateEntity();
        entitiesB.insert(entity);
        ASSERT_TRUE(em.AddComponent<BodyComponent>(entity, BodyComponent()));
    }

    // attach BodyComponent to components in group A
    for (int i = 0; i < entityNum; ++i)
    {
        EntityID entity = em.CreateEntity();
        entitiesC.insert(entity);
        ASSERT_TRUE(em.AddComponent<TransformComponent>(entity, TransformComponent()));
        ASSERT_TRUE(em.AddComponent<BodyComponent>(entity, BodyComponent()));
    }

    int transformComponentId = Component::GetID<TransformComponent>();
    int bodyComponentId = Component::GetID<BodyComponent>();
    ASSERT_NE(transformComponentId, bodyComponentId);


    // verify entities in group A
    tmpEntites.clear();
    em.ForEach<TransformComponent>(
        [&](EntityID entity, TransformComponent&)
        {
            tmpEntites.insert(entity);
        }
    );
    ASSERT_TRUE(tmpEntites == entitiesA);

    // verify entities in group B
    tmpEntites.clear();
    em.ForEach<TransformComponent, BodyComponent>(
        [&](EntityID entity, TransformComponent&, BodyComponent&)
        {
            tmpEntites.insert(entity);
        }
    );
    ASSERT_TRUE(tmpEntites == entitiesB);

    // verify entities in group C
    tmpEntites.clear();
    em.ForEach<BodyComponent>(
        [&](EntityID entity, BodyComponent&)
        {
            tmpEntites.insert(entity);
        }
    );
    ASSERT_TRUE(tmpEntites == entitiesC);

    EXPECT_NO_THROW(ret = EngineDeleteScene(scene));
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