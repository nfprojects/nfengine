#include "../PCH.hpp"
#include "../RendererTest.hpp"
#include "../../nfCommon/Math/Math.hpp"

class ResourceBinding : public RendererTest
{
};

TEST_F(ResourceBinding, BindingSet)
{
    ResourceBindingSetPtr bindingSet;

    // there is no point in creating empty binding set
    {
        ResourceBindingSetDesc bindingSetDesc(nullptr, 0, ShaderType::Vertex);
        bindingSet = gRendererDevice->CreateResourceBindingSet(bindingSetDesc);
        EXPECT_TRUE(bindingSet.get() == nullptr);
    }

    // simple binding with single cbuffer
    {
        ResourceBindingDesc binding(ShaderResourceType::CBuffer, 0);
        ResourceBindingSetDesc bindingSetDesc(&binding, 1, ShaderType::Vertex);
        bindingSet = gRendererDevice->CreateResourceBindingSet(bindingSetDesc);
        EXPECT_TRUE(bindingSet.get() != nullptr);
    }

    // simple binding with single texture
    {
        ResourceBindingDesc binding(ShaderResourceType::Texture, 0);
        ResourceBindingSetDesc bindingSetDesc(&binding, 1, ShaderType::Vertex);
        bindingSet = gRendererDevice->CreateResourceBindingSet(bindingSetDesc);
        EXPECT_TRUE(bindingSet.get() != nullptr);
    }

    // compound binding set
    {
        ResourceBindingDesc bindings[2];
        bindings[0] = ResourceBindingDesc(ShaderResourceType::Texture, 123);
        bindings[1] = ResourceBindingDesc(ShaderResourceType::CBuffer, 321);
        ResourceBindingSetDesc bindingSetDesc(bindings, 2, ShaderType::Pixel);
        bindingSet = gRendererDevice->CreateResourceBindingSet(bindingSetDesc);
        EXPECT_TRUE(bindingSet.get() != nullptr);
    }

    // you can not create binding for sampler - samplers are merged with textures
    {
        ResourceBindingDesc binding(ShaderResourceType::Sampler, 0);
        ResourceBindingSetDesc bindingSetDesc(&binding, 1, ShaderType::Vertex);
        bindingSet = gRendererDevice->CreateResourceBindingSet(bindingSetDesc);
        EXPECT_TRUE(bindingSet.get() == nullptr);
    }

    // invalid shader resource type
    {
        ResourceBindingDesc binding(ShaderResourceType::Unknown, 0);
        ResourceBindingSetDesc bindingSetDesc(&binding, 1, ShaderType::Vertex);
        bindingSet = gRendererDevice->CreateResourceBindingSet(bindingSetDesc);
        EXPECT_TRUE(bindingSet.get() == nullptr);
    }
}

TEST_F(ResourceBinding, BindingLayout)
{
    ResourceBindingSetPtr bindingSetA, bindingSetB, bindingSetC, bindingSetD;
    ResourceBindingLayoutPtr bindingLayout;

    // create binding sets

    ResourceBindingDesc cbufferBinding0(ShaderResourceType::CBuffer, 0);
    ResourceBindingDesc cbufferBinding1(ShaderResourceType::CBuffer, 1);

    ResourceBindingSetDesc bindingSetDescA(&cbufferBinding0, 1, ShaderType::Vertex);
    bindingSetA = gRendererDevice->CreateResourceBindingSet(bindingSetDescA);
    ASSERT_TRUE(bindingSetA.get() != nullptr);

    ResourceBindingSetDesc bindingSetDescB(&cbufferBinding0, 1, ShaderType::Pixel);
    bindingSetB = gRendererDevice->CreateResourceBindingSet(bindingSetDescB);
    ASSERT_TRUE(bindingSetB.get() != nullptr);

    ResourceBindingSetDesc bindingSetDescC(&cbufferBinding0, 1, ShaderType::All);
    bindingSetC = gRendererDevice->CreateResourceBindingSet(bindingSetDescC);
    ASSERT_TRUE(bindingSetC.get() != nullptr);

    ResourceBindingSetDesc bindingSetDescD(&cbufferBinding1, 1, ShaderType::Vertex);
    bindingSetD = gRendererDevice->CreateResourceBindingSet(bindingSetDescD);
    ASSERT_TRUE(bindingSetD.get() != nullptr);


    // empty binding layout is OK
    {
        ResourceBindingLayoutDesc bindingLayoutDesc(nullptr, 0);
        bindingLayout = gRendererDevice->CreateResourceBindingLayout(bindingLayoutDesc);
        EXPECT_TRUE(bindingLayout.get() != nullptr);
    }

    // valid binding layout (independent cbuffers in VS and PS at slot 0)
    {
        ResourceBindingSetPtr bindingSets[] = { bindingSetA, bindingSetB };
        ResourceBindingLayoutDesc bindingLayoutDesc(bindingSets, 2);
        bindingLayout = gRendererDevice->CreateResourceBindingLayout(bindingLayoutDesc);
        EXPECT_TRUE(bindingLayout.get() != nullptr);
    }

    // valid binding layout (independent cbuffers in VS at slots 0 and 1)
    {
        ResourceBindingSetPtr bindingSets[] = { bindingSetA, bindingSetD };
        ResourceBindingLayoutDesc bindingLayoutDesc(bindingSets, 2);
        bindingLayout = gRendererDevice->CreateResourceBindingLayout(bindingLayoutDesc);
        EXPECT_TRUE(bindingLayout.get() != nullptr);
    }

    // invalid binding layout reuse of the same binding
    {
        ResourceBindingSetPtr bindingSets[] = { bindingSetA, bindingSetA };
        ResourceBindingLayoutDesc bindingLayoutDesc(bindingSets, 2);
        bindingLayout = gRendererDevice->CreateResourceBindingLayout(bindingLayoutDesc);
        EXPECT_TRUE(bindingLayout.get() == nullptr);
    }

    // invalid binding layout - overlapping binding (cbuffer at slot 0)
    {
        ResourceBindingSetPtr bindingSets[] = { bindingSetA, bindingSetC };
        ResourceBindingLayoutDesc bindingLayoutDesc(bindingSets, 2);
        bindingLayout = gRendererDevice->CreateResourceBindingLayout(bindingLayoutDesc);
        EXPECT_TRUE(bindingLayout.get() == nullptr);
    }

    // invalid binding layout - overlapping binding (cbuffer at slot 0)
    // (same as above, but different order)
    {
        ResourceBindingSetPtr bindingSets[] = { bindingSetC, bindingSetA };
        ResourceBindingLayoutDesc bindingLayoutDesc(bindingSets, 2);
        bindingLayout = gRendererDevice->CreateResourceBindingLayout(bindingLayoutDesc);
        EXPECT_TRUE(bindingLayout.get() == nullptr);
    }
}