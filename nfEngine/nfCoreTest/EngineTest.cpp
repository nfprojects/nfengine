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