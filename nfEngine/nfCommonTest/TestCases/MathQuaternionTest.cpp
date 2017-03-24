#include "PCH.hpp"
#include "nfCommon/Math/Quaternion.hpp"

using namespace NFE::Math;

namespace {

const Vector vecA = Vector(1.0f, 1.0f, 1.0f, 1.0f);
const Vector vecB = Vector(1.0f, 2.0f, 3.0f, 4.0f);
const Vector vecC = Vector(2.0f, 3.0f, 4.0f, 5.0f);
const Vector vecD = Vector(1.0f, 4.0f, 9.0f, 16.0f);
const Vector vecE = Vector(4.0f, 3.0f, 2.0f, 1.0f);

} // namespace

TEST(MathQuaternion, Transform)
{
    // TODO
}