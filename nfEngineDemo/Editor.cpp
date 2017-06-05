#include "PCH.hpp"
#include "Editor.hpp"
#include "Test.hpp"

#include <cstdint>

using namespace NFE;
using namespace NFE::Renderer;
using namespace NFE::Math;
using namespace NFE::Scene;
using namespace NFE::Resource;

EntityManager* gEntityManager = nullptr;
EntityID gSelectedEntity = static_cast<EntityID>(-1);

// TODO: reimplement Scene editor with ImGui