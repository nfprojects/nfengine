#pragma once

#include "../nfEngine/nfCore/SceneManager.hpp"

#define ENGINE_GET_TEXTURE(name) \
    static_cast<Texture*>(Engine::GetInstance()->GetResManager()->GetResource(name, ResourceType::Texture))

#define ENGINE_GET_MATERIAL(name) \
    static_cast<Material*>(Engine::GetInstance()->GetResManager()->GetResource(name, ResourceType::Material))

#define ENGINE_GET_MESH(name) \
    static_cast<Mesh*>(Engine::GetInstance()->GetResManager()->GetResource(name, ResourceType::Mesh))

#define ENGINE_GET_COLLISION_SHAPE(name) \
    static_cast<CollisionShape*>(Engine::GetInstance()->GetResManager()->GetResource(name, ResourceType::CollisionShape))

NFE::Scene::SceneManager* InitScene(int sceneId);
int GetScenesNum();
