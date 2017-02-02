#include "PCH.hpp"
#include "nfCore/Engine.hpp"
#include "nfCore/Renderer/HighLevelRenderer.hpp"
#include "nfCore/Components/TransformComponent.hpp"
#include "nfCore/Components/ComponentMesh.hpp"
#include "nfCore/Components/ComponentBody.hpp"
#include "nfCore/Components/ComponentLight.hpp"


using namespace NFE;
using namespace NFE::Renderer;
using namespace NFE::Scene;
using namespace NFE::Resource;
using namespace NFE::Common;

class nfEngineTest : public testing::Test
{
protected:
    // prepare test case environment - initialize the engine
    static void SetUpTestCase()
    {
        mEngine = Engine::GetInstance();
        ASSERT_NE(nullptr, mEngine);

        mRenderer = mEngine->GetRenderer();
        ASSERT_NE(nullptr, mRenderer);

        mResourcesManager = mEngine->GetResManager();
        ASSERT_NE(nullptr, mResourcesManager);
    }

    // prepare test case environment - release the engine
    static void TearDownTestCase()
    {
        Engine::Release();
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
    mEngine->DeleteScene(scene);
    mEngine->DeleteScene(scene); // second try should fail
    mEngine->DeleteScene(nullptr); // pass invalid pointer
}

// ResManager::GetResource weird arguments test
TEST_F(nfEngineTest, GetResourceWeirdArguments)
{
    ResourceBase* resource = nullptr;

    resource = mResourcesManager->GetResource(NULL, ResourceType::Unknown, true);
    EXPECT_EQ(nullptr, resource);

    resource = mResourcesManager->GetResource(NULL, ResourceType::Unknown, false);
    EXPECT_EQ(nullptr, resource);

    resource = mResourcesManager->GetResource(NULL, ResourceType::Texture, true);
    EXPECT_EQ(nullptr, resource);

    resource = mResourcesManager->GetResource("blah", ResourceType::Unknown, true);
    EXPECT_EQ(nullptr, resource);
}