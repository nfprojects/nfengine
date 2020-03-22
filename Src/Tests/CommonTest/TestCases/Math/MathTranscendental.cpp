#include "PCH.hpp"
#include "Engine/Common/Math/Transcendental.hpp"
#include "Engine/Common/Math/Math.hpp"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/System/Timer.hpp"

using namespace NFE;

namespace {

using TranscendentalFloatFunc = float(*)(float);

struct TestRange
{
    enum class StepType
    {
        Increment,
        Multiply
    };

    float min;
    float max;
    float step;
    StepType type;

    TestRange(float min, float max, float step, StepType type)
        : min(min), max(max), step(step), type(type)
    { }
};

void TestTranscendental(const char* name, const TestRange& range,
                        const TranscendentalFloatFunc& func, const TranscendentalFloatFunc& ref,
                        const float maxAbsError, const float maxRelError)
{
    float measuredMaxAbsError = 0.0f;
    float measuredMaxRelError = 0.0f;

    for (float x = range.min; x < range.max;)
    {
        const float calculated = func(x);
        const float reference = ref(x);

        const float relError = Math::Abs((calculated - reference) / reference);
        const float absError = Math::Abs(calculated - reference);

        EXPECT_LE(relError, maxRelError) << name << " calc=" << calculated << " ref=" << reference << " x=" << x;
        EXPECT_LE(absError, maxAbsError) << name << " calc=" << calculated << " ref=" << reference << " x=" << x;

        measuredMaxAbsError = Math::Max(measuredMaxAbsError, absError);
        measuredMaxRelError = Math::Max(measuredMaxRelError, relError);

        if (range.type == TestRange::StepType::Increment)
            x += range.step;
        else
            x *= range.step;
    }

    NFE_LOG_INFO("Math::%s (float) max absolute error = %.3e, max relative error: %.3e",
                 name, measuredMaxAbsError, measuredMaxRelError);
}

} // namespace

//////////////////////////////////////////////////////////////////////////

TEST(Math, Sin)
{
    TestRange range(-10.0f, 10.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("Sin", range, Math::Sin, sinf, 1.0e-06f, 1.0f);
}

TEST(MathTest, Sin_4)
{
    const auto func = [] (float x) { return Math::Sin(Math::Vector4(x)).x; };
    const TestRange range(-10.0f, 10.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("Sin_4", range, func, sinf, 2.0e-06f, 1.0f);
}

TEST(MathTest, Sin_8)
{
    const auto func = [] (float x) { return Math::Sin(Math::Vector8(x))[0]; };
    const TestRange range(-10.0f, 10.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("Sin_8", range, func, sinf, 2.0e-06f, 1.0f);
}

TEST(Math, Cos)
{
    TestRange range(-10.0f, 10.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("Cos", range, Math::Cos, cosf, 1.0e-06f, 1.0f);
}

TEST(Math, ASin)
{
    TestRange range(-1.0f, 1.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("ASin", range, Math::ASin, asinf, 3.0e-07f, 1.0f);
}

TEST(Math, ACos)
{
    TestRange range(-1.0f, 1.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("ACos", range, Math::ACos, acosf, 3.0e-07f, 1.0f);
}

TEST(MathTest, FastACos)
{
    const TestRange range(-1.0f, 1.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("ACos", range, Math::FastACos, acosf, 7.0e-5f, 1.0f);
}

TEST(Math, ATan)
{
    TestRange range(-5.0f, 5.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("ATan", range, Math::ATan, atanf, 2.0e-07f, 1.0f);
}

TEST(Math, FastExp)
{
    TestRange range(-5.0f, 5.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("FastExp", range, Math::FastExp, expf, 1.0f, 2.0e-2f);
}

TEST(MathTest, FastExp_4)
{
    const auto func = [] (float x) { return Math::FastExp(Math::Vector4(x)).x; };
    const TestRange range(-40.0f, 5.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("FastExp_4", range, func, expf, 1.0f, 2.0e-2f);
}

TEST(Math, FastExp2)
{
    TestRange range(-5.0f, 5.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("FastExp2", range, Math::FastExp2, exp2f, 1.0f, 2.0e-2f);
}

TEST(MathTest, FastExp2_4)
{
    const auto func = [] (float x) { return Math::FastExp2(Math::Vector4(x)).x; };
    const TestRange range(-40.0f, 5.0f, 0.01f, TestRange::StepType::Increment);
    TestTranscendental("FastExp2_4", range, func, exp2f, 1.0f, 2.0e-2f);
}

TEST(Math, Log)
{
    TestRange range(0.0001f, 1.0e+30f, 1.5f, TestRange::StepType::Multiply);
    TestTranscendental("Log", range, Math::Log, logf, 1.0f, 3.0e-07f);
}

TEST(Math, FastLog)
{
    TestRange range(0.0001f, 1.0e+30f, 1.5f, TestRange::StepType::Multiply);
    TestTranscendental("FastLog", range, Math::FastLog, logf, 1.0f, 1.0e-4f);
}

TEST(Math, FastLog_4)
{
    const auto func = [] (float x) { return Math::FastLog(Math::Vector4(x)).x; };
    TestRange range(0.0001f, 1.0e+30f, 1.5f, TestRange::StepType::Multiply);
    TestTranscendental("FastLog_4", range, func, logf, 1.0f, 1.0e-4f);
}

TEST(Math, FastLog2)
{
    TestRange range(1.0e-30f, 1.0e+30f, 1.01f, TestRange::StepType::Multiply);
    TestTranscendental("FastLog2", range, Math::FastLog2, log2f, 0.01f, 0.1f);
}

TEST(Math, FastLog2_4)
{
    const auto func = [] (float x) { return Math::FastLog2(Math::Vector4(x)).x; };
    TestRange range(1.0e-30f, 1.0e+30f, 1.01f, TestRange::StepType::Multiply);
    TestTranscendental("FastLog2_4", range, func, log2f, 0.01f, 0.1f);
}
