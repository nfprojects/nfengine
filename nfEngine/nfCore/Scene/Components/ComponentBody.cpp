/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Body component definition.
*/

#include "PCH.hpp"
#include "ComponentBody.hpp"
#include "Resources/ResourcesManager.hpp"
#include "Resources/CollisionShape.hpp"

#include "nfCommon/Utils/InputStream.hpp"
#include "nfCommon/Utils/OutputStream.hpp"


namespace NFE {
namespace Scene {

using namespace Math;
using namespace Resource;

BodyComponent::BodyComponent()
    : mCollisionShape(nullptr)
    , mMass(0.0f)
{
}

BodyComponent::~BodyComponent()
{
    // TODO use "normal" resource pointer
    if (mCollisionShape != nullptr)
    {
        mCollisionShape->DelRef();
        mCollisionShape = nullptr;
    }
}

Box BodyComponent::GetBoundingBox() const
{
    // TODO
    return Box();
}

void BodyComponent::OnAttach()
{

}

void BodyComponent::OnDetach()
{

}

void BodyComponent::SetCollisionShape(CollisionShape* shape)
{
    // TODO use "normal" resource pointer

    if (mCollisionShape)
        mCollisionShape->DelRef();

    mCollisionShape = shape;
    if (mCollisionShape)
        mCollisionShape->AddRef();
}

Math::Vector BodyComponent::GetVelocity() const
{
    // TODO get velocity from physics engine
    return mVelocity;
}

Math::Vector BodyComponent::GetAngularVelocity() const
{
    // TODO get velocity from physics engine
    return mAngularVelocity;
}

void BodyComponent::SetVelocity(const Math::Vector& newVelocity)
{
    mVelocity = newVelocity;
    // TODO set velocity
}

void BodyComponent::SetAngularVelocity(const Math::Vector& newAngularVelocity)
{
    mAngularVelocity = newAngularVelocity;
    // TODO set velocity
}

void BodyComponent::SetMass(float mass)
{
    mMass = mass;
    // TODO set mass
}

float BodyComponent::GetMass() const
{
    return mMass;
}

} // namespace Scene
} // namespace NFE
