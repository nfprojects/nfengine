#pragma once

// TODO: this paths must be changed
#include "../nfEngine/nfCore/Engine.hpp"
#include "../nfEngine/nfCore/Scene/EntityManager.hpp"
#include "../nfEngine/nfCore/Systems/TransformSystem.hpp"
#include "../nfEngine/nfCore/Renderer/HighLevelRenderer.hpp"
#include "../nfEngine/nfCore/Renderer/View.hpp"
#include "../nfEngine/nfCore/Renderer/GuiRenderer.hpp"

#include "../nfEngine/nfCore/Components/TransformComponent.hpp"
#include "../nfEngine/nfCore/Components/BodyComponent.hpp"
#include "../nfEngine/nfCore/Components/LightComponent.hpp"
#include "../nfEngine/nfCore/Components/CameraComponent.hpp"
#include "../nfEngine/nfCore/Components/MeshComponent.hpp"

#include "nfCommon/Math/Random.hpp"
#include "nfCommon/InputStream.hpp"
#include "nfCommon/OutputStream.hpp"
#include "nfCommon/Timer.hpp"
#include "nfCommon/Logger.hpp"

extern NFE::Engine* gEngine;