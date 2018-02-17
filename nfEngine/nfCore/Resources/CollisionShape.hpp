/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Collision shape resource declarations.
 */

#pragma once

#include "../Core.hpp"
#include "Resource.hpp"

#include "nfCommon/Math/Matrix4.hpp"


//predeclarations
class btCollisionShape;
class btCompoundShape;

namespace NFE {
namespace Resource {

NFE_ALIGN(16)
struct CompoundShapeChild
{
    btCollisionShape* pShape;
    Math::Matrix4 matrix;
};

class CORE_API CollisionShape : public ResourceBase
{
private:
    std::vector<CompoundShapeChild> mChildren;
    btCollisionShape* mShape;
    Math::Vector4 mLocalInertia;

    void Release();

public:
    CollisionShape();
    virtual ~CollisionShape();

    static CollisionShape* Allocate();
    static void Free(CollisionShape* ptr);

    NFE_INLINE const Math::Vector4& GetLocalInertia() const { return mLocalInertia; }
    NFE_INLINE btCollisionShape* GetShape() const { return mShape; }

    bool OnLoad();
    void OnUnload();

    // TODO: this function should operate on Common::InputStream
    bool LoadFromFile(const char* pPath);

    // Add box shape
    bool AddBox(const Math::Vector4& halfSize, const Math::Matrix4& matrix);

    // Add cylinder shape
    bool AddCylinder(float h, float r);
};

} // namespace Resource
} // namespace NFE
