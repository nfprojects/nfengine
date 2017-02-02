#include "PCH.hpp"
#include "nfCore/Engine.hpp"


using namespace NFE;
using namespace NFE::Math;
using namespace NFE::Renderer;
using namespace NFE::Scene;
using namespace NFE::Resource;
using namespace NFE::Common;

class EntityManagerTest : public testing::Test
{
protected:
    // prepare test case environment - initialize the engine
    static void SetUpTestCase()
    {
        mEngine = Engine::GetInstance();
        ASSERT_NE(nullptr, mEngine);

        mRenderer = mEngine->GetRenderer();
        ASSERT_NE(nullptr, mRenderer);
    }

    // prepare test case environment - release the engine
    static void TearDownTestCase()
    {
        Engine::Release();
    }

    static Engine* mEngine;
    static HighLevelRenderer* mRenderer;
};

/// static members definitions
Engine* EntityManagerTest::mEngine = nullptr;
HighLevelRenderer* EntityManagerTest::mRenderer = nullptr;


// TODO write tests...