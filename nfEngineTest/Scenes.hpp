#pragma once

#include "../nfEngine/nfCore/SceneManager.hpp"

extern NFE::Scene::SceneManager* gScene;

void InitScene(int sceneId);
int GetScenesNum();
