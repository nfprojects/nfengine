#pragma once

#include "nfCore/Scene/SceneManager.hpp"

#define ENGINE_GET_TEXTURE(name) \
    static_cast<Texture*>(Engine::GetInstance()->GetResManager()->GetResource(name, ResourceType::Texture))

#define ENGINE_GET_MATERIAL(name) \
    static_cast<Material*>(Engine::GetInstance()->GetResManager()->GetResource(name, ResourceType::Material))

#define ENGINE_GET_MESH(name) \
    static_cast<Mesh*>(Engine::GetInstance()->GetResManager()->GetResource(name, ResourceType::Mesh))

#define ENGINE_GET_COLLISION_SHAPE(name) \
    static_cast<CollisionShape*>(Engine::GetInstance()->GetResManager()->GetResource(name, ResourceType::CollisionShape))

#define ENGINE_GET_GAME_OBJECT(name) \
    static_cast<CollisionShape*>(Engine::GetInstance()->GetResManager()->GetResource(name, ResourceType::GameObject))

NFE::Scene::SceneManager* InitScene(int sceneId);
int GetScenesNum();
