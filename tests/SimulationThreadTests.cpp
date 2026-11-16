#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

#include "FixedStepClock.hpp"
#include "SimulationThread.hpp"
#include "SnapshotBuffer.hpp"

using namespace std::chrono_literals;

namespace {
constexpr double kFastStepSeconds = 0.001;
constexpr auto kWaitTimeout = 5s;

FixedStepClockSettings FastSettings() {
    return FixedStepClockSettings{.stepSeconds = kFastStepSeconds};
}

// Polls a condition the worker thread makes true, failing after a timeout
// instead of hanging the test run.
template <typename Predicate>
bool WaitFor(Predicate predicate) {
    const auto deadline = std::chrono::steady_clock::now() + kWaitTimeout;
    while (!predicate()) {
        if (std::chrono::steady_clock::now() > deadline) {
            return false;
        }
        std::this_thread::sleep_for(1ms);
    }
    return true;
}
}  // namespace

TEST(SnapshotBuffer, IsEmptyUntilFirstPublish) {
    const SnapshotBuffer<int> buffer;
    EXPECT_FALSE(buffer.SampleAt(0.0).has_value());
}

TEST(SnapshotBuffer, SingleSnapshotIsBothEnds) {
    SnapshotBuffer<int> buffer;
    buffer.Publish(1.0, 7);
    const auto sample = buffer.SampleAt(0.5);
    ASSERT_TRUE(sample.has_value());
    EXPECT_EQ(sample->previous, sample->current);
    EXPECT_EQ(sample->current->value, 7);
    EXPECT_DOUBLE_EQ(sample->alpha, 1.0);
}

TEST(SnapshotBuffer, InterpolatesBetweenTheLatestTwo) {
    SnapshotBuffer<int> buffer;
    buffer.Publish(1.0, 10);
    buffer.Publish(2.0, 20);
    buffer.Publish(3.0, 30);

    const auto middle = buffer.SampleAt(2.25);
    ASSERT_TRUE(middle.has_value());
    EXPECT_EQ(middle->previous->value, 20);
    EXPECT_EQ(middle->current->value, 30);
    EXPECT_DOUBLE_EQ(middle->alpha, 0.25);

    EXPECT_DOUBLE_EQ(buffer.SampleAt(1.0)->alpha, 0.0);
    EXPECT_DOUBLE_EQ(buffer.SampleAt(9.0)->alpha, 1.0);
}

TEST(SnapshotBuffer, SampleKeepsSnapshotAliveAfterNewPublishes) {
    SnapshotBuffer<std::vector<int>> buffer;
    buffer.Publish(1.0, {1, 2, 3});
    const auto held = buffer.SampleAt(1.0);
    buffer.Publish(2.0, {4});
    buffer.Publish(3.0, {5});
    buffer.Clear();
    ASSERT_TRUE(held.has_value());
    EXPECT_EQ(held->current->value, (std::vector<int>{1, 2, 3}));
    EXPECT_FALSE(buffer.SampleAt(1.0).has_value());
}

TEST(SnapshotBuffer, ReaderSeesConsistentSnapshotsWhileWriterPublishes) {
    SnapshotBuffer<std::vector<int>> buffer;
    std::atomic<bool> done{false};
    std::thread writer([&] {
        for (int tick = 1; tick <= 20000; ++tick) {
            // Every element equals the tick, so a torn snapshot is visible.
            buffer.Publish(static_cast<double>(tick),
                           std::vector<int>(16, tick));
        }
        done = true;
    });
    bool consistent = true;
    while (!done) {
        if (const auto sample = buffer.SampleAt(0.0)) {
            for (const auto& entry : {sample->previous, sample->current}) {
                for (const int value : entry->value) {
                    consistent = consistent &&
                                 value == static_cast<int>(
                                              entry->simulationSeconds);
                }
            }
            consistent = consistent &&
                         sample->previous->simulationSeconds <=
                             sample->current->simulationSeconds;
        }
    }
    writer.join();
    EXPECT_TRUE(consistent);
}

TEST(SimulationThread, RejectsEmptyStepFunction) {
    EXPECT_THROW(SimulationThread(FastSettings(), {}), std::invalid_argument);
}

TEST(SimulationThread, RunsConsecutiveFixedSteps) {
    std::mutex mutex;
    std::vector<SimulationStep> steps;
    SimulationThread simulation(FastSettings(), [&](const SimulationStep& step) {
        const std::scoped_lock lock(mutex);
        steps.push_back(step);
    });
    ASSERT_TRUE(WaitFor([&] { return simulation.GetTiming().tick >= 20; }));
    simulation.Stop();

    const std::scoped_lock lock(mutex);
    ASSERT_GE(steps.size(), 20U);
    for (std::size_t index = 0; index < steps.size(); ++index) {
        EXPECT_EQ(steps[index].tick, index + 1);
        EXPECT_DOUBLE_EQ(steps[index].stepSeconds, kFastStepSeconds);
    }
    EXPECT_EQ(simulation.GetTiming().tick, steps.size());
}

TEST(SimulationThread, PauseStopsStepsAndSingleStepRunsOne) {
    std::atomic<std::uint64_t> ran{0};
    SimulationThread simulation(FastSettings(),
                                [&](const SimulationStep&) { ++ran; });
    ASSERT_TRUE(WaitFor([&] { return ran > 0; }));

    simulation.SetPaused(true);
    EXPECT_TRUE(simulation.IsPaused());
    ASSERT_TRUE(WaitFor([&] { return simulation.GetTiming().paused; }));
    const std::uint64_t pausedAt = ran;
    std::this_thread::sleep_for(50ms);
    EXPECT_EQ(ran, pausedAt);

    simulation.RequestSingleStep();
    simulation.RequestSingleStep();
    ASSERT_TRUE(WaitFor([&] { return ran == pausedAt + 2; }));
    std::this_thread::sleep_for(20ms);
    EXPECT_EQ(ran, pausedAt + 2);
    EXPECT_EQ(simulation.GetTiming().tick, pausedAt + 2);

    simulation.SetPaused(false);
    ASSERT_TRUE(WaitFor([&] { return ran > pausedAt + 2; }));
}

TEST(SimulationThread, ResumingDoesNotCatchUpOnPausedTime) {
    constexpr double kStepSeconds = 0.05;
    std::atomic<std::uint64_t> ran{0};
    SimulationThread simulation(
        FixedStepClockSettings{.stepSeconds = kStepSeconds},
        [&](const SimulationStep&) { ++ran; });
    simulation.SetPaused(true);
    ASSERT_TRUE(WaitFor([&] { return simulation.GetTiming().paused; }));
    std::this_thread::sleep_for(300ms);

    const std::uint64_t before = ran;
    simulation.SetPaused(false);
    std::this_thread::sleep_for(20ms);
    // Without resetting the time base the clock would run its catch-up
    // limit (several steps) immediately after resuming.
    EXPECT_LE(ran - before, 1U);
}

TEST(SimulationThread, LongStepsKeepTheScaledRateDespiteFrameClamp) {
    // One step per 0.5 s of real time, far longer than the frame clamp. If
    // the worker slept the whole half second, the clock would keep only
    // 0.05 s of each sleep and three steps would take about 8 s.
    constexpr double kStepSeconds = 2.0;
    constexpr double kTimeScale = 4.0;
    constexpr double kRealSecondsPerStep = kStepSeconds / kTimeScale;
    constexpr std::uint64_t kSteps = 3;
    std::atomic<std::uint64_t> ran{0};
    const auto start = std::chrono::steady_clock::now();
    SimulationThread simulation(
        FixedStepClockSettings{.stepSeconds = kStepSeconds,
                               .maxFrameSeconds = 0.05},
        [&](const SimulationStep&) { ++ran; });
    simulation.SetTimeScale(kTimeScale);
    ASSERT_TRUE(WaitFor([&] { return ran >= kSteps; }));
    const double elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - start)
            .count();
    const double expected = static_cast<double>(kSteps) * kRealSecondsPerStep;
    // The clock only counts real time, so it can never run early; allow a
    // generous margin for a loaded machine on the late side.
    EXPECT_GE(elapsed, expected - 0.05);
    EXPECT_LE(elapsed, expected + 0.75);
}

TEST(SimulationThread, ZeroTimeScaleFreezesSteps) {
    std::atomic<std::uint64_t> ran{0};
    SimulationThread simulation(FastSettings(),
                                [&](const SimulationStep&) { ++ran; });
    simulation.SetTimeScale(0.0);
    ASSERT_TRUE(
        WaitFor([&] { return simulation.GetTiming().timeScale == 0.0; }));
    const std::uint64_t frozenAt = ran;
    std::this_thread::sleep_for(50ms);
    EXPECT_EQ(ran, frozenAt);
}

TEST(SimulationThread, EstimatedTimeStaysWithinOneStepOfLatestTick) {
    SimulationThread simulation(FastSettings(), [](const SimulationStep&) {});
    for (int sample = 0; sample < 200; ++sample) {
        const SimulationTiming timing = simulation.GetTiming();
        const double latest =
            static_cast<double>(timing.tick) * timing.stepSeconds;
        EXPECT_GE(timing.estimatedSeconds, latest - 1e-12);
        EXPECT_LE(timing.estimatedSeconds,
                  latest + timing.stepSeconds + 1e-12);
        std::this_thread::sleep_for(100us);
    }
}

TEST(SimulationThread, StepExceptionStopsWorkerAndIsKept) {
    std::atomic<std::uint64_t> ran{0};
    SimulationThread simulation(FastSettings(), [&](const SimulationStep& step) {
        ++ran;
        if (step.tick == 3) {
            throw std::runtime_error("step failed");
        }
    });
    ASSERT_TRUE(WaitFor([&] { return simulation.GetFailure() != nullptr; }));
    std::this_thread::sleep_for(20ms);
    EXPECT_EQ(ran, 3U);
    EXPECT_EQ(simulation.GetTiming().tick, 2U);
    EXPECT_THROW(std::rethrow_exception(simulation.GetFailure()),
                 std::runtime_error);
}

TEST(SimulationThread, StopsPromptlyWhilePaused) {
    auto simulation = std::make_unique<SimulationThread>(
        FastSettings(), [](const SimulationStep&) {});
    simulation->SetPaused(true);
    ASSERT_TRUE(WaitFor([&] { return simulation->GetTiming().paused; }));
    const auto start = std::chrono::steady_clock::now();
    simulation.reset();
    EXPECT_LT(std::chrono::steady_clock::now() - start, 1s);
}
