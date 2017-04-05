#include "PCH.hpp"
#include "nfCommon/Math/Transform.hpp"

using namespace NFE::Math;

namespace {

// some random values
const Vector TEST_TRANSLATION(1.0f, 2.0f, 3.0f);
const Vector TEST_TRANSLATION2(-4.1f, 5.74f, -1.52f);
const Quaternion TEST_ROTATION = Quaternion::FromAxisAndAngle(Vector(1.0f, -1.53f, 2.34f).Normalized3(), NFE_MATH_PI * 1.273f);
const Quaternion TEST_ROTATION2 = Quaternion::FromAxisAndAngle(Vector(-2.0f, 1.46f, 1.2f).Normalized3(), -NFE_MATH_PI * 0.235f);

} // namespace

TEST(MathTransform, TransformPoint)
{
    const Transform t(TEST_TRANSLATION, Quaternion::RotationY(NFE_MATH_PI / 2.0f));

    EXPECT_TRUE(Vector::AlmostEqual(Vector(1.0f, 2.0f, 3.0f), t.TransformPoint(Vector())));
    EXPECT_TRUE(Vector::AlmostEqual(Vector(1.0f, 2.0f, 2.0f), t.TransformPoint(Vector(1.0f, 0.0f, 0.0f))));
    EXPECT_TRUE(Vector::AlmostEqual(Vector(1.0f, 2.0f, 4.0f), t.TransformPoint(Vector(-1.0f, 0.0f, 0.0f))));
    EXPECT_TRUE(Vector::AlmostEqual(Vector(1.0f, 3.0f, 3.0f), t.TransformPoint(Vector(0.0f, 1.0f, 0.0f))));
    EXPECT_TRUE(Vector::AlmostEqual(Vector(1.0f, 1.0f, 3.0f), t.TransformPoint(Vector(0.0f, -1.0f, 0.0f))));
    EXPECT_TRUE(Vector::AlmostEqual(Vector(2.0f, 2.0f, 3.0f), t.TransformPoint(Vector(0.0f, 0.0f, 1.0f))));
    EXPECT_TRUE(Vector::AlmostEqual(Vector(0.0f, 2.0f, 3.0f), t.TransformPoint(Vector(0.0f, 0.0f, -1.0f))));
}

TEST(MathTransform, TransformVector)
{
    const Transform t(TEST_TRANSLATION, Quaternion::RotationY(NFE_MATH_PI / 2.0f));

    EXPECT_TRUE(Vector::AlmostEqual(Vector(0.0f, 0.0f, 0.0f), t.TransformVector(Vector())));
    EXPECT_TRUE(Vector::AlmostEqual(Vector(0.0f, 0.0f, -1.0f), t.TransformVector(Vector(1.0f, 0.0f, 0.0f))));
    EXPECT_TRUE(Vector::AlmostEqual(Vector(0.0f, 0.0f, 1.0f), t.TransformVector(Vector(-1.0f, 0.0f, 0.0f))));
    EXPECT_TRUE(Vector::AlmostEqual(Vector(0.0f, 1.0f, 0.0f), t.TransformVector(Vector(0.0f, 1.0f, 0.0f))));
    EXPECT_TRUE(Vector::AlmostEqual(Vector(0.0f, -1.0f, 0.0f), t.TransformVector(Vector(0.0f, -1.0f, 0.0f))));
    EXPECT_TRUE(Vector::AlmostEqual(Vector(1.0f, 0.0f, 0.0f), t.TransformVector(Vector(0.0f, 0.0f, 1.0f))));
    EXPECT_TRUE(Vector::AlmostEqual(Vector(-1.0f, 0.0f, 0.0f), t.TransformVector(Vector(0.0f, 0.0f, -1.0f))));
}

TEST(MathTransform, CompoundTransform)
{
    const Transform t0(TEST_TRANSLATION, TEST_ROTATION);
    const Transform t1(TEST_TRANSLATION2, TEST_ROTATION2);

    const Transform compoundTransform = t0 * t1;

    const Vector testVectors[5] =
    {
        Vector(),
        Vector(1.0f, 0.0f, 0.0f),
        Vector(0.0f, 1.0f, 0.0f),
        Vector(0.0f, 0.0f, 1.0f),
        Vector(-0.34f, 3.352f, 1.2542f),
    };

    for (size_t i = 0; i < 5; ++i)
    {
        const Vector p2 = compoundTransform.TransformPoint(testVectors[i]);
        const Vector p2ref = t0.TransformPoint(t1.TransformPoint(testVectors[i]));
        EXPECT_TRUE(Vector::AlmostEqual(p2ref, p2, 0.00001f)) << "i=" << i;
    }
}

TEST(MathTransform, Invert_TranslationOnly)
{
    const Transform t(TEST_TRANSLATION, Quaternion());
    const Transform inverted = t.Inverted();

    EXPECT_TRUE(Transform::AlmostEqual(Transform(), t * inverted, 0.00001f));
    EXPECT_TRUE(Transform::AlmostEqual(Transform(), inverted * t, 0.00001f));
}

TEST(MathTransform, Invert_RotationOnly)
{
    const Transform t(Vector(), TEST_ROTATION);
    const Transform inverted = t.Inverted();

    EXPECT_TRUE(Transform::AlmostEqual(Transform(), t * inverted, 0.00001f));
    EXPECT_TRUE(Transform::AlmostEqual(Transform(), inverted * t, 0.00001f));
}

TEST(MathTransform, Invert)
{
    const Transform t(TEST_TRANSLATION, TEST_ROTATION);
    const Transform inverted = t.Inverted();

    EXPECT_TRUE(Transform::AlmostEqual(Transform(), t * inverted, 0.00001f));
    EXPECT_TRUE(Transform::AlmostEqual(Transform(), inverted * t, 0.00001f));
}