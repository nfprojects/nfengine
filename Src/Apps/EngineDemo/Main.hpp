#pragma once

#include "Engine/Core/Scene/Systems/EntitySystem.hpp"
#include "Engine/Common/Math/Random.hpp"
#include "Engine/Common/System/Timer.hpp"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/Containers/UniquePtr.hpp"

namespace NFE {

class GameWindow;

extern Engine* gEngine;
extern float gDeltaTime;

void SceneDeleter(Scene::Scene* scene);
GameWindow* AddWindow(GameWindow* parent = nullptr);

} // namespace NFE
