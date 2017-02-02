/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of physics scene interface.
 */

#pragma once

#include "../Core.hpp"
#include "PhysicsProxies.hpp"


namespace NFE {
namespace Physics {


/**
 * Physics scene interface.
 *
 * @note Extracting the interface from the actual implementation hides
 * underlying physics library (Bullet in this case), so there is no symbols pollution.
 * It could be also possible to implement other "physics backends" in the future.
 */
class NFE_ALIGN(16) IPhysicsScene
{
public:
    virtual ~IPhysicsScene() { }

    /**
     * Simulate the physics scene.
     */
    virtual void Update(float timeDelta) = 0;

    /**
     * Create a new physics body proxy.
     */
    virtual ProxyID CreateBodyProxy(const BodyProxyInfo& info) = 0;

    /**
     * Update existing physics body proxy. All the properties will be overridden.
     * @return  False if properties are corrupted.
     * @note    Callback functions are ignored.
     */
    virtual bool UpdateBodyProxy(const ProxyID proxyID, const BodyProxyInfo& info) = 0;

    /**
     * Delete existing physics body proxy. Proxy ID must be valid.
     */
    virtual void DeleteBodyProxy(const ProxyID proxyID) = 0;
};


} // namespace Physics
} // namespace NFE
