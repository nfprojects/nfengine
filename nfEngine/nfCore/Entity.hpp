/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Scene entity declarations.
 */

#pragma once

#include "Core.hpp"
#include "Aligned.hpp"
#include "Components/TransformComponent.hpp"

namespace NFE {
namespace Scene {

#define ENTITY_FLAG_IS_ROOT (1 << 0)
#define ENTITY_FLAG_MOVED (1 << 1)


NFE_ALIGN16
class CORE_API Entity : public Util::Aligned
{
    friend class EntityManager;
	friend class SceneManager;
	friend class Segment;
    friend class PhysicsSystem;
    friend class RendererSystem;

	//disable unwanted methods
	Entity(const Entity&);
	Entity& operator=(const Entity&);

	/// TODO: These 3 pointer can be reducted to one:
	SceneManager* mScene;         // parent scene
	Segment* mSegment;    // parent scene segment

	// list of enabled components
	std::set<Component*> mComponents;

public:
    TransformComponent mTransform;

	Entity();
	virtual ~Entity();

	SceneManager* GetScene() const;
	Segment* GetSceneSegment() const;

	Result AddComponent(Component* pComponent);
	Result RemoveComponent(Component* pComponent);

	/**
	 * Remove and destroy all the entity components.
	 */
	void RemoveAllComponents();
};

} // namespace Scene
} // namespace NFE
