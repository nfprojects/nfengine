#pragma once

#include "nfCore/Engine.hpp"
#include "nfCore/Scene/GameObjectInstance.hpp"
#include "nfCore/Scene/Systems/GameObjectSystem.hpp"
#include "nfCore/Scene/Systems/EntitySystem.hpp"
#include "nfCore/Renderer/HighLevelRenderer.hpp"
#include "nfCore/Renderer/View.hpp"
#include "nfCore/Renderer/GuiRenderer.hpp"

#include "nfCore/Scene/Components/ComponentBody.hpp"
#include "nfCore/Scene/Components/ComponentLight.hpp"
#include "nfCore/Scene/Components/ComponentCamera.hpp"
#include "nfCore/Scene/Components/ComponentMesh.hpp"
#include "nfCore/Scene/Components/ComponentInput.hpp"

#include "nfCommon/Math/Random.hpp"
#include "nfCommon/InputStream.hpp"
#include "nfCommon/OutputStream.hpp"
#include "nfCommon/Timer.hpp"
#include "nfCommon/Logger.hpp"

namespace NFE {

class GameWindow;

extern Engine* gEngine;
extern std::unique_ptr<Renderer::Font> gFont;
extern float gDeltaTime;

void SceneDeleter(Scene::SceneManager* scene);
GameWindow* AddWindow(GameWindow* parent = nullptr);

} // namespace NFE
