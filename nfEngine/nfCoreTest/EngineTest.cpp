#include "PCH.hpp"
#include "Engine.hpp"
#include "Renderer/HighLevelRenderer.hpp"

#include "Components/TransformComponent.hpp"
#include "Components/MeshComponent.hpp"
#include "Components/BodyComponent.hpp"
#include "Components/LightComponent.hpp"

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
        EXPECT_NO_THROW(mEngine = Engine::Initialize());
        ASSERT_NE(nullptr, mEngine);

        mRenderer = mEngine->GetRenderer();
        ASSERT_NE(nullptr, mRenderer);

        mResourcesManager = mEngine->GetResManager();
        ASSERT_NE(nullptr, mResourcesManager);
    }

    // preapre test case enviroment - release the engine
    static void TearDownTestCase()
    {
        ASSERT_EQ(true, Engine::Release());
        ASSERT_EQ(false, Engine::Release()); // second try should fail
    }

    static Engine* mEngine;
    static ResManager* mResourcesManager;
    static HighLevelRenderer* mRenderer;
};

/// static members definitions
Engine* nfEngineTest::mEngine = nullptr;
ResManager* nfEngineTest::mResourcesManager = nullptr;
HighLevelRenderer* nfEngineTest::mRenderer = nullptr;


// check if calling Init second time will return a proper value
TEST_F(nfEngineTest, DoubleInit)
{
    Engine* newEngine = mEngine->Initialize();
    ASSERT_EQ(mEngine, newEngine);

    HighLevelRenderer* newRendererPtr = mEngine->GetRenderer();
    EXPECT_EQ(mRenderer, newRendererPtr) << "Renderer pointer should not change";

    ResManager* newResourcesManagerPtr = mEngine->GetResManager();
    EXPECT_EQ(mResourcesManager, newResourcesManagerPtr) <<
            "Resources Manager pointer should not change";
}

// basic scene creation / removal
TEST_F(nfEngineTest, SceneManagement)
{
    SceneManager* scene;

    scene = mEngine->CreateScene();
    ASSERT_NE(nullptr, scene);
    EXPECT_EQ(true, mEngine->DeleteScene(scene));
    EXPECT_EQ(false, mEngine->DeleteScene(scene)); // second try should fail
    EXPECT_EQ(false, mEngine->DeleteScene(nullptr)); // pass invalid poiner
}

// ResManager::GetResource weird arguments test
TEST_F(nfEngineTest, GetResourceWeirdArguments)
{
    ResourceBase* resource = nullptr;

    EXPECT_NO_THROW(resource = mResourcesManager->GetResource(NULL, ResourceType::Unknown, true));
    EXPECT_EQ(nullptr, resource);

    EXPECT_NO_THROW(resource = mResourcesManager->GetResource(NULL, ResourceType::Unknown, false));
    EXPECT_EQ(nullptr, resource);

    EXPECT_NO_THROW(resource = mResourcesManager->GetResource(NULL, ResourceType::Texture, true));
    EXPECT_EQ(nullptr, resource);

    EXPECT_NO_THROW(resource = mResourcesManager->GetResource("blah", ResourceType::Unknown, true));
    EXPECT_EQ(nullptr, resource);
}