#pragma once

#include "nfCore/Engine.hpp"
#include "nfCore/Scene/Systems/EntitySystem.hpp"
#include "nfCore/Renderer/HighLevelRenderer.hpp"
#include "nfCore/Renderer/View.hpp"
#include "nfCore/Renderer/GuiRenderer.hpp"

#include "nfCommon/Math/Random.hpp"
#include "nfCommon/System/Timer.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/Containers/UniquePtr.hpp"

namespace NFE {

class GameWindow;

extern Engine* gEngine;
extern Common::UniquePtr<Renderer::Font> gFont;
extern float gDeltaTime;

void SceneDeleter(Scene::SceneManager* scene);
GameWindow* AddWindow(GameWindow* parent = nullptr);

} // namespace NFE
