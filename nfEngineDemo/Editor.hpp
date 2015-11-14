#pragma once

#include "../nfEngine/nfCore/Engine.hpp"
#include "../nfEngine/nfCore/Scene/EntityManager.hpp"

extern NFE::Scene::EntityManager* gEntityManager;
extern NFE::Scene::EntityID gSelectedEntity;

void Demo_InitEditorBar();