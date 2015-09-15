#pragma once

#include "../nfEngine/nfCore/Engine.hpp"
#include "../nfEngine/nfCore/SceneEvent.hpp"
#include "../nfEngine/nfCore/EntityManager.hpp"

extern NFE::Scene::EntityManager* gEntityManager;
extern NFE::Scene::EntityID gSelectedEntity;

void Demo_InitEditorBar();