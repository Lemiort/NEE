#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <stdexcept>

#include "FixedStepClock.hpp"

namespace {
// Quarter-second steps are exact in binary floating point, which keeps the
// expected values in these tests free of rounding noise.
constexpr double kQuarter = 0.25;
constexpr double kTolerance = 1e-9;

FixedStepClock MakeQuarterClock(std::uint32_t maxSteps = 8,
                                double maxFrameSeconds = 10.0) {
    return FixedStepClock(
        FixedStepClockSettings{.stepSeconds = kQuarter,
                               .maxStepsPerAdvance = maxSteps,
                               .maxFrameSeconds = maxFrameSeconds});
}

std::uint64_t TicksAfter(int frames, double frameSeconds) {
    FixedStepClock clock;
    for (int frame = 0; frame < frames; ++frame) {
        clock.Advance(frameSeconds);
    }
    return clock.GetTickCount();
}
}  // namespace

TEST(FixedStepClock, DefaultsToSixtyStepsPerSecond) {
    const FixedStepClock clock;
    EXPECT_DOUBLE_EQ(clock.GetStepSeconds(), 1.0 / 60.0);
    EXPECT_EQ(clock.GetTickCount(), 0U);
    EXPECT_DOUBLE_EQ(clock.GetTimeScale(), 1.0);
    EXPECT_FALSE(clock.IsPaused());
}

TEST(FixedStepClock, RejectsInvalidSettings) {
    constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();
    constexpr double kInfinity = std::numeric_limits<double>::infinity();
    for (const double step : {0.0, -1.0, kNaN, kInfinity}) {
        EXPECT_THROW(FixedStepClock(FixedStepClockSettings{.stepSeconds = step}),
                     std::invalid_argument)
            << "step " << step;
    }
    EXPECT_THROW(
        FixedStepClock(FixedStepClockSettings{.maxStepsPerAdvance = 0}),
        std::invalid_argument);
    for (const double maxFrame : {0.0, -1.0, kNaN, kInfinity}) {
        EXPECT_THROW(
            FixedStepClock(FixedStepClockSettings{.maxFrameSeconds = maxFrame}),
            std::invalid_argument)
            << "max frame " << maxFrame;
    }
}

TEST(FixedStepClock, AccumulatesPartialFramesIntoSteps) {
    FixedStepClock clock = MakeQuarterClock();

    ClockAdvance advance = clock.Advance(0.125);
    EXPECT_EQ(advance.steps, 0U);
    EXPECT_NEAR(advance.alpha, 0.5, kTolerance);

    advance = clock.Advance(0.1875);
    EXPECT_EQ(advance.steps, 1U);
    EXPECT_NEAR(advance.alpha, 0.25, kTolerance);
    EXPECT_FALSE(advance.droppedTime);
    EXPECT_EQ(clock.GetTickCount(), 1U);
    EXPECT_DOUBLE_EQ(clock.GetSimulationSeconds(), kQuarter);
}

TEST(FixedStepClock, RunsSeveralStepsForALongFrame) {
    FixedStepClock clock = MakeQuarterClock();
    const ClockAdvance advance = clock.Advance(0.75);
    EXPECT_EQ(advance.steps, 3U);
    EXPECT_NEAR(advance.alpha, 0.0, kTolerance);
    EXPECT_EQ(clock.GetTickCount(), 3U);
}

TEST(FixedStepClock, DropsTimeBeyondTheStepLimit) {
    FixedStepClock clock = MakeQuarterClock(2);
    const ClockAdvance advance = clock.Advance(1.125);
    EXPECT_EQ(advance.steps, 2U);
    EXPECT_TRUE(advance.droppedTime);
    // 1.125 - 2 * 0.25 = 0.625 left; only the fractional 0.125 is kept.
    EXPECT_NEAR(advance.alpha, 0.5, kTolerance);
    EXPECT_EQ(clock.GetTickCount(), 2U);

    const ClockAdvance next = clock.Advance(0.0);
    EXPECT_EQ(next.steps, 0U);
    EXPECT_FALSE(next.droppedTime);
}

TEST(FixedStepClock, ClampsLongRealFramesBeforeStepping) {
    FixedStepClock clock = MakeQuarterClock(8, 0.5);
    const ClockAdvance advance = clock.Advance(30.0);
    EXPECT_EQ(advance.steps, 2U);
    EXPECT_FALSE(advance.droppedTime);
}

TEST(FixedStepClock, TreatsInvalidFrameTimesAsZero) {
    FixedStepClock clock = MakeQuarterClock();
    for (const double frame : {-1.0, std::numeric_limits<double>::quiet_NaN(),
                               std::numeric_limits<double>::infinity()}) {
        const ClockAdvance advance = clock.Advance(frame);
        EXPECT_EQ(advance.steps, 0U) << "frame " << frame;
        EXPECT_NEAR(advance.alpha, 0.0, kTolerance) << "frame " << frame;
    }
    EXPECT_EQ(clock.GetTickCount(), 0U);
}

TEST(FixedStepClock, TimeScaleChangesStepsPerRealSecond) {
    FixedStepClock clock = MakeQuarterClock();
    clock.SetTimeScale(2.0);
    EXPECT_EQ(clock.Advance(kQuarter).steps, 2U);

    clock.SetTimeScale(0.0);
    EXPECT_EQ(clock.Advance(1.0).steps, 0U);

    clock.SetTimeScale(-1.0);
    clock.SetTimeScale(std::numeric_limits<double>::quiet_NaN());
    EXPECT_DOUBLE_EQ(clock.GetTimeScale(), 0.0);
}

TEST(FixedStepClock, PauseFreezesStepsAndAlpha) {
    FixedStepClock clock = MakeQuarterClock();
    clock.Advance(0.125);
    clock.SetPaused(true);
    EXPECT_TRUE(clock.IsPaused());

    const ClockAdvance paused = clock.Advance(5.0);
    EXPECT_EQ(paused.steps, 0U);
    EXPECT_NEAR(paused.alpha, 0.5, kTolerance);

    clock.SetPaused(false);
    const ClockAdvance resumed = clock.Advance(0.125);
    EXPECT_EQ(resumed.steps, 1U);
    EXPECT_NEAR(resumed.alpha, 0.0, kTolerance);
}

TEST(FixedStepClock, SingleStepRunsOnceWhilePaused) {
    FixedStepClock clock = MakeQuarterClock();
    clock.RequestSingleStep();  // Ignored while running.
    EXPECT_EQ(clock.Advance(0.0).steps, 0U);

    clock.SetPaused(true);
    clock.RequestSingleStep();
    EXPECT_EQ(clock.Advance(0.0).steps, 1U);
    EXPECT_EQ(clock.Advance(0.0).steps, 0U);
    EXPECT_EQ(clock.GetTickCount(), 1U);

    clock.RequestSingleStep();
    clock.SetPaused(false);  // Resuming discards a pending single step.
    EXPECT_EQ(clock.Advance(0.0).steps, 0U);
}

TEST(FixedStepClock, StepCountDoesNotDependOnFrameRate) {
    // Ten seconds of frames at each rate.
    const std::uint64_t at30 = TicksAfter(300, 1.0 / 30.0);
    const std::uint64_t at60 = TicksAfter(600, 1.0 / 60.0);
    const std::uint64_t at144 = TicksAfter(1440, 1.0 / 144.0);
    // Accumulated rounding may leave the final step a hair short.
    EXPECT_NEAR(static_cast<double>(at30), 600.0, 1.0);
    EXPECT_NEAR(static_cast<double>(at60), 600.0, 1.0);
    EXPECT_NEAR(static_cast<double>(at144), 600.0, 1.0);
}

TEST(FixedStepClock, SimulationTimeIsDerivedFromTicks) {
    FixedStepClock clock;
    for (int frame = 0; frame < 36000; ++frame) {
        clock.Advance(1.0 / 60.0);
    }
    EXPECT_DOUBLE_EQ(clock.GetSimulationSeconds(),
                     static_cast<double>(clock.GetTickCount()) / 60.0);
}

TEST(FixedStepClock, ResetKeepsPauseAndScale) {
    FixedStepClock clock = MakeQuarterClock();
    clock.SetTimeScale(3.0);
    clock.Advance(1.0);
    clock.SetPaused(true);
    clock.Reset();
    EXPECT_EQ(clock.GetTickCount(), 0U);
    EXPECT_TRUE(clock.IsPaused());
    EXPECT_DOUBLE_EQ(clock.GetTimeScale(), 3.0);
    EXPECT_NEAR(clock.Advance(0.0).alpha, 0.0, kTolerance);
}
