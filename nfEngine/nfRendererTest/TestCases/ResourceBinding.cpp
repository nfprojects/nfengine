#include "../PCH.hpp"
#include "../RendererTest.hpp"
#include "../../nfCommon/Math/Math.hpp"

class ResourceBinding : public RendererTest
{
};

TEST_F(ResourceBinding, BindingSetEmpty)
{
    std::unique_ptr<IResourceBindingSet> bindingSet;

    // there is no point in creating empty binding set
    {
        ResourceBindingSetDesc bindingSetDesc(nullptr, 0, ShaderType::Vertex);
        bindingSet.reset(gRendererDevice->CreateResourceBindingSet(bindingSetDesc));
        EXPECT_TRUE(bindingSet.get() == nullptr);
    }
}

TEST_F(ResourceBinding, BindingSetSingle)
{
    std::unique_ptr<IResourceBindingSet> bindingSet;

    // simple binding with single cbuffer
    {
        ResourceBindingDesc binding(ShaderResourceType::CBuffer, 0);
        ResourceBindingSetDesc bindingSetDesc(&binding, 1, ShaderType::Vertex);
        bindingSet.reset(gRendererDevice->CreateResourceBindingSet(bindingSetDesc));
        EXPECT_TRUE(bindingSet.get() != nullptr);
    }

    // simple binding with single texture
    {
        ResourceBindingDesc binding(ShaderResourceType::Texture, 0);
        ResourceBindingSetDesc bindingSetDesc(&binding, 1, ShaderType::Vertex);
        bindingSet.reset(gRendererDevice->CreateResourceBindingSet(bindingSetDesc));
        EXPECT_TRUE(bindingSet.get() != nullptr);
    }
}

TEST_F(ResourceBinding, BindingSetSingleInvalid)
{
    std::unique_ptr<IResourceBindingSet> bindingSet;

    // you can not create binding for sampler - samplers are merged with binding layout
    {
        ResourceBindingDesc binding(ShaderResourceType::Sampler, 0);
        ResourceBindingSetDesc bindingSetDesc(&binding, 1, ShaderType::Vertex);
        bindingSet.reset(gRendererDevice->CreateResourceBindingSet(bindingSetDesc));
        EXPECT_TRUE(bindingSet.get() == nullptr);
    }

    // invalid shader resource type
    {
        ResourceBindingDesc binding(ShaderResourceType::Unknown, 0);
        ResourceBindingSetDesc bindingSetDesc(&binding, 1, ShaderType::Vertex);
        bindingSet.reset(gRendererDevice->CreateResourceBindingSet(bindingSetDesc));
        EXPECT_TRUE(bindingSet.get() == nullptr);
    }
}

TEST_F(ResourceBinding, BindingSetCompound)
{
    std::unique_ptr<IResourceBindingSet> bindingSet;

    {
        ResourceBindingDesc bindings[3] =
        {
            ResourceBindingDesc(ShaderResourceType::Texture, 123),
            ResourceBindingDesc(ShaderResourceType::CBuffer, 123),
            ResourceBindingDesc(ShaderResourceType::CBuffer, 321),
        };
        ResourceBindingSetDesc bindingSetDesc(bindings, 3, ShaderType::Pixel);
        bindingSet.reset(gRendererDevice->CreateResourceBindingSet(bindingSetDesc));
        EXPECT_TRUE(bindingSet.get() != nullptr);
    }
}

TEST_F(ResourceBinding, BindingSetCompoundInvalid)
{
    std::unique_ptr<IResourceBindingSet> bindingSet;

    // can't create binding set with two same binding slots

    {
        ResourceBindingDesc bindings[2] =
        {
            ResourceBindingDesc(ShaderResourceType::Texture, 123),
            ResourceBindingDesc(ShaderResourceType::Texture, 123),
        };
        ResourceBindingSetDesc bindingSetDesc(bindings, 2, ShaderType::Pixel);
        bindingSet.reset(gRendererDevice->CreateResourceBindingSet(bindingSetDesc));
        EXPECT_TRUE(bindingSet.get() == nullptr);
    }

    {
        ResourceBindingDesc bindings[2] =
        {
            ResourceBindingDesc(ShaderResourceType::CBuffer, 123),
            ResourceBindingDesc(ShaderResourceType::CBuffer, 123),
        };
        ResourceBindingSetDesc bindingSetDesc(bindings, 2, ShaderType::Pixel);
        bindingSet.reset(gRendererDevice->CreateResourceBindingSet(bindingSetDesc));
        EXPECT_TRUE(bindingSet.get() == nullptr);
    }
}


TEST_F(ResourceBinding, BindingLayout)
{
    std::unique_ptr<IResourceBindingSet> bindingSetA, bindingSetB, bindingSetC, bindingSetD;
    std::unique_ptr<IResourceBindingLayout> bindingLayout;

    // create binding sets

    ResourceBindingDesc cbufferBinding0(ShaderResourceType::CBuffer, 0);
    ResourceBindingDesc cbufferBinding1(ShaderResourceType::CBuffer, 1);

    ResourceBindingSetDesc bindingSetDescA(&cbufferBinding0, 1, ShaderType::Vertex);
    bindingSetA.reset(gRendererDevice->CreateResourceBindingSet(bindingSetDescA));
    ASSERT_TRUE(bindingSetA.get() != nullptr);

    ResourceBindingSetDesc bindingSetDescB(&cbufferBinding0, 1, ShaderType::Pixel);
    bindingSetB.reset(gRendererDevice->CreateResourceBindingSet(bindingSetDescB));
    ASSERT_TRUE(bindingSetB.get() != nullptr);

    ResourceBindingSetDesc bindingSetDescC(&cbufferBinding0, 1, ShaderType::All);
    bindingSetC.reset(gRendererDevice->CreateResourceBindingSet(bindingSetDescC));
    ASSERT_TRUE(bindingSetC.get() != nullptr);

    ResourceBindingSetDesc bindingSetDescD(&cbufferBinding1, 1, ShaderType::Vertex);
    bindingSetD.reset(gRendererDevice->CreateResourceBindingSet(bindingSetDescD));
    ASSERT_TRUE(bindingSetD.get() != nullptr);


    // empty binding layout is OK
    {
        ResourceBindingLayoutDesc bindingLayoutDesc(nullptr, 0);
        bindingLayout.reset(gRendererDevice->CreateResourceBindingLayout(bindingLayoutDesc));
        EXPECT_TRUE(bindingLayout.get() != nullptr);
    }

    // valid binding layout (independent cbuffers in VS and PS at slot 0)
    {
        IResourceBindingSet* bindingSets[] = { bindingSetA.get(), bindingSetB.get() };
        ResourceBindingLayoutDesc bindingLayoutDesc(bindingSets, 2);
        bindingLayout.reset(gRendererDevice->CreateResourceBindingLayout(bindingLayoutDesc));
        EXPECT_TRUE(bindingLayout.get() != nullptr);
    }

    // valid binding layout (independent cbuffers in VS at slots 0 and 1)
    {
        IResourceBindingSet* bindingSets[] = { bindingSetA.get(), bindingSetD.get() };
        ResourceBindingLayoutDesc bindingLayoutDesc(bindingSets, 2);
        bindingLayout.reset(gRendererDevice->CreateResourceBindingLayout(bindingLayoutDesc));
        EXPECT_TRUE(bindingLayout.get() != nullptr);
    }

    // invalid binding layout reuse of the same binding
    {
        IResourceBindingSet* bindingSets[] = { bindingSetA.get(), bindingSetA.get() };
        ResourceBindingLayoutDesc bindingLayoutDesc(bindingSets, 2);
        bindingLayout.reset(gRendererDevice->CreateResourceBindingLayout(bindingLayoutDesc));
        EXPECT_TRUE(bindingLayout.get() == nullptr);
    }

    // invalid binding layout - overlapping binding (cbuffer at slot 0)
    {
        IResourceBindingSet* bindingSets[] = { bindingSetA.get(), bindingSetC.get() };
        ResourceBindingLayoutDesc bindingLayoutDesc(bindingSets, 2);
        bindingLayout.reset(gRendererDevice->CreateResourceBindingLayout(bindingLayoutDesc));
        EXPECT_TRUE(bindingLayout.get() == nullptr);
    }

    // invalid binding layout - overlapping binding (cbuffer at slot 0)
    // (same as above, but different order)
    {
        IResourceBindingSet* bindingSets[] = { bindingSetC.get(), bindingSetA.get() };
        ResourceBindingLayoutDesc bindingLayoutDesc(bindingSets, 2);
        bindingLayout.reset(gRendererDevice->CreateResourceBindingLayout(bindingLayoutDesc));
        EXPECT_TRUE(bindingLayout.get() == nullptr);
    }
}