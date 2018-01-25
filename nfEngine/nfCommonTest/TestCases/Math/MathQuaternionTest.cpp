#include "PCH.hpp"
#include "nfCommon/Math/Quaternion.hpp"
#include "nfCommon/Math/Random.hpp"


using namespace NFE::Math;

namespace {

// test rotation (axis = [1.23, -2.45, 4.43], angle = 74 degrees)
const Vector testAxis = Vector(1.23f, -2.45f, 4.43f);
const float testAngle = DegToRad(74.0f);

const Vector testVector0 = Vector(2.4f, -0.12f, 47.0f);

// calculated online
const Vector transformedX = Vector(0.316016f, 0.736977f, 0.597493f);        // (1,0,0) transformed with test rotation
const Vector transformedY = Vector(-0.897835f, 0.435841f, -0.06272f);       // (0,1,0) transformed with test rotation
const Vector transformedZ = Vector(-0.306636f, -0.51663f, 0.799417f);       // (0,0,1) transformed with test rotation
const Vector transformed0 = Vector(-13.545702f, -22.565163f, 39.014123f);   // testVector0 transformed with test rotation

} // namespace


TEST(MathQuaternion, Equal)
{
    const Quaternion q00 = Quaternion::FromAxisAndAngle(Vector(1.0f, 0.0f, 0.0f), Constants::pi<float> / 2.0f);
    const Quaternion q01 = Quaternion::FromAxisAndAngle(Vector(-1.0f, 0.0f, 0.0f), -Constants::pi<float> / 2.0f);

    const Quaternion q10 = Quaternion::FromAxisAndAngle(testAxis.Normalized3(), testAngle);
    const Quaternion q11 = Quaternion::FromAxisAndAngle(-testAxis.Normalized3(), -testAngle);

    EXPECT_TRUE(Quaternion::AlmostEqual(q00, q01, 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(q00, q00, 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(q10, q10, 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(q11, q11, 0.00001f));

    EXPECT_FALSE(Quaternion::AlmostEqual(q00, q10, 0.00001f));
    EXPECT_FALSE(Quaternion::AlmostEqual(q00, q11, 0.00001f));
    EXPECT_FALSE(Quaternion::AlmostEqual(q01, q10, 0.00001f));
    EXPECT_FALSE(Quaternion::AlmostEqual(q01, q11, 0.00001f));
}

TEST(MathQuaternion, RotationAxisX)
{
    const Quaternion q = Quaternion::RotationX(Constants::pi<float> / 2.0f);

    const Vector tx = q.TransformVector(Vector(1.0f, 0.0f, 0.0f));
    const Vector ty = q.TransformVector(Vector(0.0f, 1.0f, 0.0f));
    const Vector tz = q.TransformVector(Vector(0.0f, 0.0f, 1.0f));
    const Vector t0 = q.TransformVector(testVector0);

    EXPECT_TRUE(Vector::AlmostEqual(tx, Vector(1.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(Vector::AlmostEqual(ty, Vector(0.0f, 0.0f, 1.0f)));
    EXPECT_TRUE(Vector::AlmostEqual(tz, Vector(0.0f, -1.0f, 0.0f)));
    EXPECT_TRUE(Vector::AlmostEqual(t0, Vector(2.4f, -47.0f, -0.12f), 0.00001f));
}

TEST(MathQuaternion, RotationAxisY)
{
    const Quaternion q = Quaternion::RotationY(Constants::pi<float> / 2.0f);

    const Vector tx = q.TransformVector(Vector(1.0f, 0.0f, 0.0f));
    const Vector ty = q.TransformVector(Vector(0.0f, 1.0f, 0.0f));
    const Vector tz = q.TransformVector(Vector(0.0f, 0.0f, 1.0f));
    const Vector t0 = q.TransformVector(testVector0);

    EXPECT_TRUE(Vector::AlmostEqual(tx, Vector(0.0f, 0.0f, -1.0f)));
    EXPECT_TRUE(Vector::AlmostEqual(ty, Vector(0.0f, 1.0f, 0.0f)));
    EXPECT_TRUE(Vector::AlmostEqual(tz, Vector(1.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(Vector::AlmostEqual(t0, Vector(47.0f, -0.12f, -2.4f), 0.00001f));
}

TEST(MathQuaternion, RotationAxisZ)
{
    const Quaternion q = Quaternion::RotationZ(Constants::pi<float> / 2.0f);

    const Vector tx = q.TransformVector(Vector(1.0f, 0.0f, 0.0f));
    const Vector ty = q.TransformVector(Vector(0.0f, 1.0f, 0.0f));
    const Vector tz = q.TransformVector(Vector(0.0f, 0.0f, 1.0f));
    const Vector t0 = q.TransformVector(testVector0);

    EXPECT_TRUE(Vector::AlmostEqual(tx, Vector(0.0f, 1.0f, 0.0f)));
    EXPECT_TRUE(Vector::AlmostEqual(ty, Vector(-1.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(Vector::AlmostEqual(tz, Vector(0.0f, 0.0f, 1.0f)));
    EXPECT_TRUE(Vector::AlmostEqual(t0, Vector(0.12f, 2.4f, 47.0f), 0.00001f));
}

TEST(MathQuaternion, RotationAxis)
{
    const Vector axis = testAxis.Normalized3();
    const Quaternion q = Quaternion::FromAxisAndAngle(axis, testAngle);

    const Vector tx = q.TransformVector(Vector(1.0f, 0.0f, 0.0f));
    const Vector ty = q.TransformVector(Vector(0.0f, 1.0f, 0.0f));
    const Vector tz = q.TransformVector(Vector(0.0f, 0.0f, 1.0f));
    const Vector t0 = q.TransformVector(testVector0);

    EXPECT_TRUE(Vector::AlmostEqual(tx, transformedX, 0.00001f));
    EXPECT_TRUE(Vector::AlmostEqual(ty, transformedY, 0.00001f));
    EXPECT_TRUE(Vector::AlmostEqual(tz, transformedZ, 0.00001f));
    EXPECT_TRUE(Vector::AlmostEqual(t0, transformed0, 0.00001f));
}

TEST(MathQuaternion, ToMatrix)
{
    const Vector axis = testAxis.Normalized3();
    const Quaternion q = Quaternion::FromAxisAndAngle(axis, testAngle);

    Matrix m = q.ToMatrix();
    m.r[3] = Vector(); // zero 4th row
    ASSERT_TRUE(Vector::AlmostEqual(m.GetRow(0), transformedX, 0.00001f));
    ASSERT_TRUE(Vector::AlmostEqual(m.GetRow(1), transformedY, 0.00001f));
    ASSERT_TRUE(Vector::AlmostEqual(m.GetRow(2), transformedZ, 0.00001f));

    const Vector t0 = m.LinearCombination3(testVector0);
    EXPECT_TRUE(Vector::AlmostEqual(t0, transformed0, 0.00001f));
}

TEST(MathQuaternion, FromMatrix)
{
    const Vector axis = testAxis.Normalized3();
    const Quaternion q = Quaternion::FromAxisAndAngle(axis, testAngle);

    const Matrix m = q.ToMatrix();
    const Quaternion q2 = Quaternion::FromMatrix(m);

    ASSERT_TRUE(Quaternion::AlmostEqual(q, q2, 0.00001f));

    const Vector tx = q2.TransformVector(Vector(1.0f, 0.0f, 0.0f));
    const Vector ty = q2.TransformVector(Vector(0.0f, 1.0f, 0.0f));
    const Vector tz = q2.TransformVector(Vector(0.0f, 0.0f, 1.0f));
    const Vector t0 = q2.TransformVector(testVector0);

    EXPECT_TRUE(Vector::AlmostEqual(tx, transformedX, 0.00001f));
    EXPECT_TRUE(Vector::AlmostEqual(ty, transformedY, 0.00001f));
    EXPECT_TRUE(Vector::AlmostEqual(tz, transformedZ, 0.00001f));
    EXPECT_TRUE(Vector::AlmostEqual(t0, transformed0, 0.00001f));
}

TEST(MathQuaternion, ToAxis)
{
    const Vector axis = testAxis.Normalized3();
    const Quaternion q = Quaternion::FromAxisAndAngle(axis, testAngle);

    float angle2;
    Vector axis2;
    q.ToAxis(axis2, angle2);

    if (Vector::Dot3(axis, axis2) > 0.0f)
    {
        ASSERT_TRUE(Vector::AlmostEqual(axis, axis2, 0.00001f));
        ASSERT_FLOAT_EQ(testAngle, angle2);
    }
    else
    {
        ASSERT_TRUE(Vector::AlmostEqual(axis, -axis2, 0.00001f));
        ASSERT_FLOAT_EQ(testAngle, -angle2);
    }
}

TEST(MathQuaternion, ToAxis_Identity)
{
    float angle;
    Vector axis;
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
    EXPECT_TRUE(Quaternion::AlmostEqual(xRot * xRot, xRot2, 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(yRot * yRot, yRot2, 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(zRot * zRot, zRot2, 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(xRevRot * xRot, Quaternion::Identity(), 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(yRevRot * yRot, Quaternion::Identity(), 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(zRevRot * zRot, Quaternion::Identity(), 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(xRot2 * xRot2, Quaternion::Identity(), 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(yRot2 * yRot2, Quaternion::Identity(), 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(zRot2 * zRot2, Quaternion::Identity(), 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(xRot * xRot2, xRevRot, 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(yRot * yRot2, yRevRot, 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(zRot * zRot2, zRevRot, 0.00001f));

    // simple double 90 degree rotations
    EXPECT_TRUE(Quaternion::AlmostEqual(yRot * xRot, xRot * zRevRot, 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(xRot * yRot, yRot * zRot, 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(zRot * yRot, yRot * xRevRot, 0.00001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(xRot * zRot, yRevRot * xRot, 0.00001f));
}

TEST(MathQuaternion, MultiplyRandomInverse)
{
    const size_t iterations = 100;
    Random random;

    for (size_t i = 0; i < iterations; ++i)
    {
        const Quaternion q = Quaternion::FromAxisAndAngle(Vector(random.GetPointOnSphere()), random.GetFloat() * Constants::pi<float>);
        const Quaternion qInv = q.Inverted();

        EXPECT_TRUE(Quaternion::AlmostEqual(qInv * q, Quaternion::Identity(), 0.00001f));
        EXPECT_TRUE(Quaternion::AlmostEqual(q * qInv, Quaternion::Identity(), 0.00001f));
    }
}

TEST(MathQuaternion, FromEulerAngles)
{
    const float angle = DegToRad(10.0f);
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromAngles(angle, 0.0f, 0.0f), Quaternion::RotationX(angle), 0.00001f));
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromAngles(0.0f, angle, 0.0f), Quaternion::RotationY(angle), 0.00001f));
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromAngles(0.0f, 0.0f, angle), Quaternion::RotationZ(angle), 0.00001f));
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromAngles(-angle, 0.0f, 0.0f), Quaternion::RotationX(-angle), 0.00001f));
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromAngles(0.0f, -angle, 0.0f), Quaternion::RotationY(-angle), 0.00001f));
    ASSERT_TRUE(Quaternion::AlmostEqual(Quaternion::FromAngles(0.0f, 0.0f, -angle), Quaternion::RotationZ(-angle), 0.00001f));

    const float yawAngle = DegToRad(10.0f);
    const float pitchAngle = DegToRad(20.0f);
    const float rollAngle = DegToRad(34.0f);

    const Quaternion pitchRot = Quaternion::RotationX(pitchAngle);
    const Quaternion yawRot = Quaternion::RotationY(yawAngle);
    const Quaternion rollRot = Quaternion::RotationZ(rollAngle);

    EXPECT_TRUE(Quaternion::AlmostEqual(Quaternion::FromAngles(pitchAngle, yawAngle, 0.0f), yawRot * pitchRot, 0.001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(Quaternion::FromAngles(pitchAngle, 0.0f, rollAngle), pitchRot * rollRot, 0.001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(Quaternion::FromAngles(0.0f, yawAngle, rollAngle), yawRot * rollRot, 0.001f));
    EXPECT_TRUE(Quaternion::AlmostEqual(Quaternion::FromAngles(pitchAngle, yawAngle, rollAngle), yawRot * pitchRot * rollRot, 0.001f));
}

TEST(MathQuaternion, ToEulerAngles)
{
    const float angle = DegToRad(10.0f);

    float pitch, yaw, roll;

    Quaternion::RotationX(angle).ToAngles(pitch, yaw, roll);
    EXPECT_FLOAT_EQ(angle, pitch);
    EXPECT_FLOAT_EQ(0.0f, yaw);
    EXPECT_FLOAT_EQ(0.0f, roll);

    Quaternion::RotationY(angle).ToAngles(pitch, yaw, roll);
    EXPECT_FLOAT_EQ(0.0f, pitch);
    EXPECT_FLOAT_EQ(angle, yaw);
    EXPECT_FLOAT_EQ(0.0f, roll);

    Quaternion::RotationZ(angle).ToAngles(pitch, yaw, roll);
    EXPECT_FLOAT_EQ(0.0f, pitch);
    EXPECT_FLOAT_EQ(0.0f, yaw);
    EXPECT_FLOAT_EQ(angle, roll);


    const float yawAngle = DegToRad(10.0f);
    const float pitchAngle = DegToRad(20.0f);
    const float rollAngle = DegToRad(34.0f);
    const Quaternion q = Quaternion::FromAngles(pitchAngle, yawAngle, rollAngle);
    q.ToAngles(pitch, yaw, roll);

    EXPECT_FLOAT_EQ(pitchAngle, pitch);
    EXPECT_FLOAT_EQ(yawAngle, yaw);
    EXPECT_FLOAT_EQ(rollAngle, roll);
}