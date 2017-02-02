#pragma once

// TODO: this paths must be changed
#include "../nfEngine/nfCore/Engine.hpp"
#include "../nfEngine/nfCore/Systems/GameObjectSystem.hpp"
#include "../nfEngine/nfCore/Systems/EntitySystem.hpp"
#include "../nfEngine/nfCore/Renderer/HighLevelRenderer.hpp"
#include "../nfEngine/nfCore/Renderer/View.hpp"
#include "../nfEngine/nfCore/Renderer/GuiRenderer.hpp"

#include "../nfEngine/nfCore/Components/ComponentBody.hpp"
#include "../nfEngine/nfCore/Components/ComponentLight.hpp"
#include "../nfEngine/nfCore/Components/ComponentCamera.hpp"
#include "../nfEngine/nfCore/Components/ComponentMesh.hpp"

#include "nfCommon/Math/Random.hpp"
#include "nfCommon/InputStream.hpp"
#include "nfCommon/OutputStream.hpp"
#include "nfCommon/Timer.hpp"
#include "nfCommon/Logger.hpp"

extern NFE::Engine* gEngine;