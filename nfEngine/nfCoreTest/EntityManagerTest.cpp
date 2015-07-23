#include "PCH.hpp"
#include "Engine.hpp"
#include "Renderer/HighLevelRenderer.hpp"
#include "Systems/TransformSystem.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/MeshComponent.hpp"
#include "Components/BodyComponent.hpp"
#include "Components/LightComponent.hpp"

using namespace NFE;
using namespace NFE::Renderer;
using namespace NFE::Scene;
using namespace NFE::Resource;
using namespace NFE::Common;

class EntityManagerTest : public testing::Test
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

    static HighLevelRenderer* pRenderer;
};

/// static members definitions
HighLevelRenderer* EntityManagerTest::pRenderer = nullptr;


// basic entity creation / removal
TEST_F(EntityManagerTest, Basic)
{
    std::vector<EntityID> entities;
    EntityManager em;

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
}

// attaching and iterating through components
TEST_F(EntityManagerTest, EntityComponents)
{
    std::set<EntityID> entitiesA, entitiesB, entitiesC, entitiesAC, entitiesBC;
    std::set<EntityID> tmpEntites;
    EntityManager em;

    const int entityNum = 3000;

    // attach TransformComponent to components in group A
    for (int i = 0; i < entityNum; ++i)
    {
        EntityID entity = em.CreateEntity();
        entitiesA.insert(entity);
        entitiesAC.insert(entity);
        ASSERT_TRUE(em.AddComponent(entity, TransformComponent()));
    }

    // attach TransformComponent and BodyComponent to components in group A
    for (int i = 0; i < entityNum; ++i)
    {
        EntityID entity = em.CreateEntity();
        entitiesB.insert(entity);
        entitiesBC.insert(entity);
        ASSERT_TRUE(em.AddComponent(entity, MeshComponent()));
    }

    // attach BodyComponent to components in group A
    for (int i = 0; i < entityNum; ++i)
    {
        EntityID entity = em.CreateEntity();
        entitiesC.insert(entity);
        entitiesAC.insert(entity);
        entitiesBC.insert(entity);
        ASSERT_TRUE(em.AddComponent(entity, TransformComponent()));
        ASSERT_TRUE(em.AddComponent(entity, MeshComponent()));
    }

    int transformComponentId = TransformComponent::GetID();
    int bodyComponentId = MeshComponent::GetID();
    ASSERT_NE(transformComponentId, bodyComponentId);

    // verify entities in group A
    tmpEntites.clear();
    em.ForEach<TransformComponent>(
        [&](EntityID entity, TransformComponent*)
        {
            tmpEntites.insert(entity);
        }
    );
    ASSERT_TRUE(tmpEntites == entitiesAC);


    // verify entities in group B
    tmpEntites.clear();
    em.ForEach<MeshComponent>(
        [&](EntityID entity, MeshComponent*)
        {
            tmpEntites.insert(entity);
        }
    );
    ASSERT_TRUE(tmpEntites == entitiesBC);


    // verify entities in group C
    tmpEntites.clear();
    em.ForEach<TransformComponent, MeshComponent>(
        [&](EntityID entity, TransformComponent*, MeshComponent*)
        {
            tmpEntites.insert(entity);
        }
    );
    ASSERT_TRUE(tmpEntites == entitiesC);


    // we don't have entities with LightComponents
    tmpEntites.clear();
    em.ForEach<LightComponent, MeshComponent>(
        [&](EntityID entity, LightComponent*, MeshComponent*)
        {
            tmpEntites.insert(entity);
        }
    );
    ASSERT_TRUE(tmpEntites.empty());

    // we don't have entities with LightComponents
    tmpEntites.clear();
    em.ForEach<LightComponent, TransformComponent>(
        [&](EntityID entity, LightComponent*, TransformComponent*)
        {
            tmpEntites.insert(entity);
        }
    );
    ASSERT_TRUE(tmpEntites.empty());

    // we don't have entities with LightComponents
    tmpEntites.clear();
    em.ForEach<LightComponent>(
        [&](EntityID entity, LightComponent*)
        {
            tmpEntites.insert(entity);
        }
    );
    ASSERT_TRUE(tmpEntites.empty());
}

// attaching child entities
TEST_F(EntityManagerTest, EntityChildren)
{
    TransformComponent* transform;
    SceneManager* scene;
    EXPECT_NO_THROW(scene = EngineCreateScene());
    ASSERT_NE(nullptr, scene);
    EntityManager* em = scene->GetEntityManager();
    ASSERT_TRUE(em != nullptr);
    TransformSystem* ts = scene->GetTransformSystem();
    ASSERT_TRUE(ts != nullptr);

    /// create 3 entities
    EntityID entityA = em->CreateEntity();
    ASSERT_TRUE(em->AddComponent(entityA, TransformComponent()));
    EntityID entityB = em->CreateEntity();
    ASSERT_TRUE(em->AddComponent(entityB, TransformComponent()));
    EntityID entityC = em->CreateEntity();
    ASSERT_TRUE(em->AddComponent(entityC, TransformComponent()));

    /// can't set parent to self
    ASSERT_FALSE(ts->SetParent(entityA, entityA));
    ASSERT_FALSE(ts->SetParent(entityB, entityB));
    ASSERT_FALSE(ts->SetParent(entityC, entityC));

    ASSERT_TRUE(ts->SetParent(entityB, entityA));
    ASSERT_TRUE(ts->SetParent(entityC, entityB));
    // C can't be parent of A, because A is parent of B, which is parent of C
    ASSERT_FALSE(ts->SetParent(entityA, entityC));

    /// set root entity global position
    transform = em->GetComponent<TransformComponent>(entityA);
    ASSERT_TRUE(transform != nullptr);
    transform->SetPosition(Vector(1.0f, 0.0f, 0.0f));

    /// set child entities local positions
    transform = em->GetComponent<TransformComponent>(entityB);
    ASSERT_NE(nullptr, transform);
    transform->SetLocalPosition(Vector(0.0f, 2.0f, 0.0f));
    transform = em->GetComponent<TransformComponent>(entityC);
    ASSERT_NE(nullptr, transform);
    transform->SetLocalPosition(Vector(0.0f, 0.0f, 3.0f));

    scene->Update(0.0f);

    transform = em->GetComponent<TransformComponent>(entityB);
    ASSERT_NE(nullptr, transform);
    EXPECT_TRUE(VectorAbs(Vector(1.0f, 2.0f, 0.0f) - transform->GetPosition())
                < VectorSplat(NFE_MATH_EPSILON));

    transform = em->GetComponent<TransformComponent>(entityC);
    ASSERT_NE(nullptr, transform);
    EXPECT_TRUE(VectorAbs(Vector(1.0f, 2.0f, 3.0f) - transform->GetPosition())
                < VectorSplat(NFE_MATH_EPSILON));

    /// detach children from parents
    ASSERT_TRUE(ts->SetParent(entityA, gInvalidEntityID));
    ASSERT_TRUE(ts->SetParent(entityB, gInvalidEntityID));
    ASSERT_TRUE(ts->SetParent(entityC, gInvalidEntityID));
}