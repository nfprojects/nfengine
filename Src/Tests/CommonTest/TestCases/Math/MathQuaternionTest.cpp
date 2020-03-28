#include "PCH.hpp"
#include "Engine/Common/Math/Quaternion.hpp"
#include "Engine/Common/Math/Random.hpp"
#include "Engine/Common/Math/SamplingHelpers.hpp"


using namespace NFE::Math;

namespace {

// test rotation (axis = [1.23, -2.45, 4.43], angle = 74 degrees)
const Vec4f testAxis = Vec4f(1.23f, -2.45f, 4.43f);
const float testAngle = DegToRad(74.0f);

const Vec4f testVector0 = Vec4f(2.4f, -0.12f, 47.0f);

// calculated online
const Vec4f transformedX = Vec4f(0.316016f, 0.736977f, 0.597493f);        // (1,0,0) transformed with test rotation
const Vec4f transformedY = Vec4f(-0.897835f, 0.435841f, -0.06272f);       // (0,1,0) transformed with test rotation
const Vec4f transformedZ = Vec4f(-0.306636f, -0.51663f, 0.799417f);       // (0,0,1) transformed with test rotation
const Vec4f transformed0 = Vec4f(-13.545702f, -22.565163f, 39.014123f);   // testVector0 transformed with test rotation

const float maxError = 0.0001f;

} // namespace


TEST(MathQuaternion, Equal)
{
    const Quaternion q00 = Quaternion::FromAxisAndAngle(Vec4f(1.0f, 0.0f, 0.0f), Constants::pi<float> / 2.0f);
    const Quaternion q01 = Quaternion::FromAxisAndAngle(Vec4f(-1.0f, 0.0f, 0.0f), -Constants::pi<float> / 2.0f);

    const Quaternion q10 = Quaternion::FromAxisAndAngle(testAxis.Normalized3(), testAngle);
    const Quaternion q11 = Quaternion::FromAxisAndAngle(-testAxis.Normalized3(), -testAngle);

    EXPECT_TRUE(Quaternion::AlmostEqual(q00, q01, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(q00, q00, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(q10, q10, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(q11, q11, maxError));

    EXPECT_FALSE(Quaternion::AlmostEqual(q00, q10, maxError));
    EXPECT_FALSE(Quaternion::AlmostEqual(q00, q11, maxError));
    EXPECT_FALSE(Quaternion::AlmostEqual(q01, q10, maxError));
    EXPECT_FALSE(Quaternion::AlmostEqual(q01, q11, maxError));
}

TEST(MathQuaternion, RotationAxisX)
{
    const Quaternion q = Quaternion::RotationX(Constants::pi<float> / 2.0f);

    const Vec4f tx = q.TransformVector(Vec4f(1.0f, 0.0f, 0.0f));
    const Vec4f ty = q.TransformVector(Vec4f(0.0f, 1.0f, 0.0f));
    const Vec4f tz = q.TransformVector(Vec4f(0.0f, 0.0f, 1.0f));
    const Vec4f t0 = q.TransformVector(testVector0);

    EXPECT_TRUE(Vec4f::AlmostEqual(tx, Vec4f(1.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(Vec4f::AlmostEqual(ty, Vec4f(0.0f, 0.0f, 1.0f)));
    EXPECT_TRUE(Vec4f::AlmostEqual(tz, Vec4f(0.0f, -1.0f, 0.0f)));
    EXPECT_TRUE(Vec4f::AlmostEqual(t0, Vec4f(2.4f, -47.0f, -0.12f), maxError));
}

TEST(MathQuaternion, RotationAxisY)
{
    const Quaternion q = Quaternion::RotationY(Constants::pi<float> / 2.0f);

    const Vec4f tx = q.TransformVector(Vec4f(1.0f, 0.0f, 0.0f));
    const Vec4f ty = q.TransformVector(Vec4f(0.0f, 1.0f, 0.0f));
    const Vec4f tz = q.TransformVector(Vec4f(0.0f, 0.0f, 1.0f));
    const Vec4f t0 = q.TransformVector(testVector0);

    EXPECT_TRUE(Vec4f::AlmostEqual(tx, Vec4f(0.0f, 0.0f, -1.0f)));
    EXPECT_TRUE(Vec4f::AlmostEqual(ty, Vec4f(0.0f, 1.0f, 0.0f)));
    EXPECT_TRUE(Vec4f::AlmostEqual(tz, Vec4f(1.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(Vec4f::AlmostEqual(t0, Vec4f(47.0f, -0.12f, -2.4f), maxError));
}

TEST(MathQuaternion, RotationAxisZ)
{
    const Quaternion q = Quaternion::RotationZ(Constants::pi<float> / 2.0f);

    const Vec4f tx = q.TransformVector(Vec4f(1.0f, 0.0f, 0.0f));
    const Vec4f ty = q.TransformVector(Vec4f(0.0f, 1.0f, 0.0f));
    const Vec4f tz = q.TransformVector(Vec4f(0.0f, 0.0f, 1.0f));
    const Vec4f t0 = q.TransformVector(testVector0);

    EXPECT_TRUE(Vec4f::AlmostEqual(tx, Vec4f(0.0f, 1.0f, 0.0f)));
    EXPECT_TRUE(Vec4f::AlmostEqual(ty, Vec4f(-1.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(Vec4f::AlmostEqual(tz, Vec4f(0.0f, 0.0f, 1.0f)));
    EXPECT_TRUE(Vec4f::AlmostEqual(t0, Vec4f(0.12f, 2.4f, 47.0f), maxError));
}

TEST(MathQuaternion, RotationAxis)
{
    const Vec4f axis = testAxis.Normalized3();
    const Quaternion q = Quaternion::FromAxisAndAngle(axis, testAngle);

    const Vec4f tx = q.TransformVector(Vec4f(1.0f, 0.0f, 0.0f));
    const Vec4f ty = q.TransformVector(Vec4f(0.0f, 1.0f, 0.0f));
    const Vec4f tz = q.TransformVector(Vec4f(0.0f, 0.0f, 1.0f));
    const Vec4f t0 = q.TransformVector(testVector0);

    EXPECT_TRUE(Vec4f::AlmostEqual(tx, transformedX, maxError));
    EXPECT_TRUE(Vec4f::AlmostEqual(ty, transformedY, maxError));
    EXPECT_TRUE(Vec4f::AlmostEqual(tz, transformedZ, maxError));
    EXPECT_TRUE(Vec4f::AlmostEqual(t0, transformed0, maxError));
}

TEST(MathQuaternion, ToMatrix)
{
    const Vec4f axis = testAxis.Normalized3();
    const Quaternion q = Quaternion::FromAxisAndAngle(axis, testAngle);

    Matrix4 m = q.ToMatrix();
    m.rows[3] = Vec4f::Zero();
    ASSERT_TRUE(Vec4f::AlmostEqual(m.GetRow(0), transformedX, maxError));
    ASSERT_TRUE(Vec4f::AlmostEqual(m.GetRow(1), transformedY, maxError));
    ASSERT_TRUE(Vec4f::AlmostEqual(m.GetRow(2), transformedZ, maxError));

    const Vec4f t0 = m.TransformPoint(testVector0);
    EXPECT_TRUE(Vec4f::AlmostEqual(t0, transformed0, maxError));
}

TEST(MathQuaternion, FromMatrix)
{
    const Vec4f axis = testAxis.Normalized3();
    const Quaternion q = Quaternion::FromAxisAndAngle(axis, testAngle);

    const Matrix4 m = q.ToMatrix();
    const Quaternion q2 = Quaternion::FromMatrix(m);

    ASSERT_TRUE(Quaternion::AlmostEqual(q, q2, maxError));

    const Vec4f tx = q2.TransformVector(Vec4f(1.0f, 0.0f, 0.0f));
    const Vec4f ty = q2.TransformVector(Vec4f(0.0f, 1.0f, 0.0f));
    const Vec4f tz = q2.TransformVector(Vec4f(0.0f, 0.0f, 1.0f));
    const Vec4f t0 = q2.TransformVector(testVector0);

    EXPECT_TRUE(Vec4f::AlmostEqual(tx, transformedX, maxError));
    EXPECT_TRUE(Vec4f::AlmostEqual(ty, transformedY, maxError));
    EXPECT_TRUE(Vec4f::AlmostEqual(tz, transformedZ, maxError));
    EXPECT_TRUE(Vec4f::AlmostEqual(t0, transformed0, maxError));
}

TEST(MathQuaternion, ToAxis)
{
    const Vec4f axis = testAxis.Normalized3();
    const Quaternion q = Quaternion::FromAxisAndAngle(axis, testAngle);

    float angle2;
    Vec4f axis2;
    q.ToAxis(axis2, angle2);

    if (Vec4f::Dot3(axis, axis2) > 0.0f)
    {
        ASSERT_TRUE(Vec4f::AlmostEqual(axis, axis2, maxError));
        ASSERT_FLOAT_EQ(testAngle, angle2);
    }
    else
    {
        ASSERT_TRUE(Vec4f::AlmostEqual(axis, -axis2, maxError));
        ASSERT_FLOAT_EQ(testAngle, -angle2);
    }
}

TEST(MathQuaternion, ToAxis_Identity)
{
    float angle;
    Vec4f axis;
    Quaternion::Identity().ToAxis(axis, angle);

    ASSERT_FLOAT_EQ(0.0f, angle);
}

TEST(MathQuaternion, MultiplyBasics)
{
    // test basic quaternion identities

    const Quaternion i = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
    const Quaternion j = Quaternion(0.0f, 1.0f, 0.0f, 0.0f);
    const Quaternion k = Quaternion(0.0f, 0.0f, 1.0f, 0.0f);
    const Quaternion one = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);

    const Quaternion iNeg = Quaternion(-1.0f, 0.0f, 0.0f, 0.0f);
    const Quaternion jNeg = Quaternion(0.0f, -1.0f, 0.0f, 0.0f);
    const Quaternion kNeg = Quaternion(0.0f, 0.0f, -1.0f, 0.0f);
    const Quaternion oneNeg = Quaternion(0.0f, 0.0f, 0.0f, -1.0f);

    EXPECT_TRUE(Quaternion::AlmostEqual(i * i, oneNeg));    // i^2 = -1
    EXPECT_TRUE(Quaternion::AlmostEqual(i * j, k));         // i * j = k
    EXPECT_TRUE(Quaternion::AlmostEqual(i * k, jNeg));      // i * k = -j
    EXPECT_TRUE(Quaternion::AlmostEqual(i * one, i));       // i * 1 = i

    EXPECT_TRUE(Quaternion::AlmostEqual(j * i, kNeg));      // j * i = -k
    EXPECT_TRUE(Quaternion::AlmostEqual(j * j, oneNeg));    // j^2 = -1
    EXPECT_TRUE(Quaternion::AlmostEqual(j * k, i));         // j * k = i
    EXPECT_TRUE(Quaternion::AlmostEqual(j * one, j));       // j * 1 = j

    EXPECT_TRUE(Quaternion::AlmostEqual(k * i, j));         // k * i = j
    EXPECT_TRUE(Quaternion::AlmostEqual(k * j, iNeg));      // k * j = -i
    EXPECT_TRUE(Quaternion::AlmostEqual(k * k, oneNeg));    // k^k = -1
    EXPECT_TRUE(Quaternion::AlmostEqual(k * one, k));       // k * 1 = k

    EXPECT_TRUE(Quaternion::AlmostEqual(one * i, i));       // 1 * i = i
    EXPECT_TRUE(Quaternion::AlmostEqual(one * j, j));       // 1 * j = j
    EXPECT_TRUE(Quaternion::AlmostEqual(one * k, k));       // 1 * k = k
    EXPECT_TRUE(Quaternion::AlmostEqual(one * one, one));   // 1 * 1 = 1
}

TEST(MathQuaternion, MultiplyRotations)
{
    EXPECT_TRUE(Quaternion::AlmostEqual(Quaternion::Identity() * Quaternion::Identity(), Quaternion::Identity()));

    // 90 degree rotations (CW)
    const Quaternion xRot = Quaternion::RotationX(Constants::pi<float> / 2.0f);
    const Quaternion yRot = Quaternion::RotationY(Constants::pi<float> / 2.0f);
    const Quaternion zRot = Quaternion::RotationZ(Constants::pi<float> / 2.0f);

    // 180 degree rotations
    const Quaternion xRevRot = Quaternion::RotationX(-Constants::pi<float> / 2.0f);
    const Quaternion yRevRot = Quaternion::RotationY(-Constants::pi<float> / 2.0f);
    const Quaternion zRevRot = Quaternion::RotationZ(-Constants::pi<float> / 2.0f);

    // 90 degree rotation (CCW)
    const Quaternion xRot2 = Quaternion::RotationX(Constants::pi<float>);
    const Quaternion yRot2 = Quaternion::RotationY(Constants::pi<float>);
    const Quaternion zRot2 = Quaternion::RotationZ(Constants::pi<float>);

    // rotation in the same plane
    EXPECT_TRUE(Quaternion::AlmostEqual(xRot * xRot, xRot2, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(yRot * yRot, yRot2, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(zRot * zRot, zRot2, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(xRevRot * xRot, Quaternion::Identity(), maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(yRevRot * yRot, Quaternion::Identity(), maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(zRevRot * zRot, Quaternion::Identity(), maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(xRot2 * xRot2, Quaternion::Identity(), maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(yRot2 * yRot2, Quaternion::Identity(), maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(zRot2 * zRot2, Quaternion::Identity(), maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(xRot * xRot2, xRevRot, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(yRot * yRot2, yRevRot, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(zRot * zRot2, zRevRot, maxError));

    // simple double 90 degree rotations
    EXPECT_TRUE(Quaternion::AlmostEqual(yRot * xRot, xRot * zRevRot, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(xRot * yRot, yRot * zRot, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(zRot * yRot, yRot * xRevRot, maxError));
    EXPECT_TRUE(Quaternion::AlmostEqual(xRot * zRot, yRevRot * xRot, maxError));
}

TEST(MathQuaternion, MultiplyRandomInverse)
{
    const size_t iterations = 100;
    Random random;

    for (size_t i = 0; i < iterations; ++i)
    {
        const Vec4f pointOnSphere = SamplingHelpers::GetSphere(random.GetVec2f());
        const Quaternion q = Quaternion::FromAxisAndAngle(pointOnSphere, random.GetFloat() * Constants::pi<float>);
        const Quaternion qInv = q.Inverted();

        EXPECT_TRUE(Quaternion::AlmostEqual(qInv * q, Quaternion::Identity(), maxError));
        EXPECT_TRUE(Quaternion::AlmostEqual(q * qInv, Quaternion::Identity(), maxError));
    }
}

TEST(MathQuaternion, FromEulerAngles)
{
    const float angle = DegToRad(10.0f);
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(angle, 0.0f, 0.0f), Quaternion::RotationX(angle), maxError));
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(0.0f, angle, 0.0f), Quaternion::RotationY(angle), maxError));
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(0.0f, 0.0f, angle), Quaternion::RotationZ(angle), maxError));
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(-angle, 0.0f, 0.0f), Quaternion::RotationX(-angle), maxError));
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(0.0f, -angle, 0.0f), Quaternion::RotationY(-angle), maxError));
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(0.0f, 0.0f, -angle), Quaternion::RotationZ(-angle), maxError));

    const float yawAngle = DegToRad(10.0f);
    const float pitchAngle = DegToRad(20.0f);
    const float rollAngle = DegToRad(34.0f);

    const Quaternion pitchRot = Quaternion::RotationX(pitchAngle);
    const Quaternion yawRot = Quaternion::RotationY(yawAngle);
    const Quaternion rollRot = Quaternion::RotationZ(rollAngle);

    EXPECT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(pitchAngle, yawAngle, 0.0f), yawRot * pitchRot, 0.001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(pitchAngle, 0.0f, rollAngle), pitchRot * rollRot, 0.001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(0.0f, yawAngle, rollAngle), yawRot * rollRot, 0.001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(Quaternion::FromEulerAngles(pitchAngle, yawAngle, rollAngle), yawRot * pitchRot * rollRot, 0.001f));
}

TEST(MathQuaternion, ToEulerAngles)
{
    const float angle = DegToRad(10.0f);

    Vec3f angles;

    angles = Quaternion::RotationX(angle).ToEulerAngles();
    EXPECT_NEAR(angle, angles.x, maxError);
    EXPECT_NEAR(0.0f, angles.y, maxError);
    EXPECT_NEAR(0.0f, angles.z, maxError);

    angles = Quaternion::RotationY(angle).ToEulerAngles();
    EXPECT_NEAR(0.0f, angles.x, maxError);
    EXPECT_NEAR(angle, angles.y, maxError);
    EXPECT_NEAR(0.0f, angles.z, maxError);

    angles = Quaternion::RotationZ(angle).ToEulerAngles();
    EXPECT_NEAR(0.0f, angles.x, maxError);
    EXPECT_NEAR(0.0f, angles.y, maxError);
    EXPECT_NEAR(angle, angles.z, maxError);


    const float yawAngle = DegToRad(10.0f);
    const float pitchAngle = DegToRad(20.0f);
    const float rollAngle = DegToRad(34.0f);
    const Quaternion q = Quaternion::FromEulerAngles(pitchAngle, yawAngle, rollAngle);
    angles = q.ToEulerAngles();

    EXPECT_NEAR(pitchAngle, angles.x, maxError);
    EXPECT_NEAR(yawAngle, angles.y, maxError);
    EXPECT_NEAR(rollAngle, angles.z, maxError);
}