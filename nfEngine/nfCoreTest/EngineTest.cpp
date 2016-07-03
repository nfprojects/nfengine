#include "PCH.hpp"
#include "nfCore/Engine.hpp"
#include "nfCore/Renderer/HighLevelRenderer.hpp"
#include "nfCore/Components/TransformComponent.hpp"
#include "nfCore/Components/MeshComponent.hpp"
#include "nfCore/Components/BodyComponent.hpp"
#include "nfCore/Components/LightComponent.hpp"


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
        EXPECT_NO_THROW(mEngine = Engine::GetInstance());
        ASSERT_NE(nullptr, mEngine);

        mRenderer = mEngine->GetRenderer();
        ASSERT_NE(nullptr, mRenderer);

        mResourcesManager = mEngine->GetResManager();
        ASSERT_NE(nullptr, mResourcesManager);
    }

    // preapre test case enviroment - release the engine
    static void TearDownTestCase()
    {
        EXPECT_NO_THROW(Engine::Release());
    }

    static Engine* mEngine;
    static ResManager* mResourcesManager;
    static HighLevelRenderer* mRenderer;
};

/// static members definitions
Engine* nfEngineTest::mEngine = nullptr;
ResManager* nfEngineTest::mResourcesManager = nullptr;
HighLevelRenderer* nfEngineTest::mRenderer = nullptr;


// check if calling GetInstance second time will return a proper value
TEST_F(nfEngineTest, GetInstance)
{
    Engine* newEngine = mEngine->GetInstance();
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
    EXPECT_NO_THROW(mEngine->DeleteScene(scene));
    EXPECT_NO_THROW(mEngine->DeleteScene(scene)); // second try should fail
    EXPECT_NO_THROW(mEngine->DeleteScene(nullptr)); // pass invalid poiner
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