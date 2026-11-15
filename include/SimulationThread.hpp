#ifndef SIMULATION_THREAD_HPP_INCLUDED
#define SIMULATION_THREAD_HPP_INCLUDED

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <exception>
#include <functional>
#include <mutex>
#include <stop_token>
#include <thread>

#include "FixedStepClock.hpp"

// One fixed step handed to the simulation callback.
struct SimulationStep {
    // 1-based index of the step being run; consecutive across the thread's
    // lifetime, including single steps taken while paused.
    std::uint64_t tick{};
    double stepSeconds{};

    // Simulation time at the end of this step; publish snapshots with it.
    [[nodiscard]] double EndSeconds() const noexcept {
        return static_cast<double>(tick) * stepSeconds;
    }
};

// Thread-safe view of the simulation clock for the render thread.
struct SimulationTiming {
    // Latest completed step.
    std::uint64_t tick{};
    double stepSeconds{};
    // Simulation time estimated for the moment GetTiming was called. Never
    // more than one step past the latest completed step, so rendering at
    // estimatedSeconds - stepSeconds always falls between published steps.
    double estimatedSeconds{};
    // State the worker has applied, which may trail the latest request.
    bool paused{false};
    double timeScale{1.0};
};

// Runs a simulation callback on a worker thread at a fixed rate, paced by
// FixedStepClock. The callback sees only the step index and duration, so the
// simulation is independent of frame rate and render-thread stalls. Render
// state crosses threads through snapshots (see SnapshotBuffer), not shared
// mutable objects.
//
// The worker starts in the constructor and stops in the destructor or Stop.
// Anything the callback touches must outlive this object; declare it before
// the SimulationThread member so it is destroyed after the thread is joined.
class SimulationThread {
public:
    using StepFunction = std::function<void(const SimulationStep&)>;

    // Throws std::invalid_argument for invalid clock settings or an empty
    // step function.
    SimulationThread(FixedStepClockSettings settings, StepFunction step);
    ~SimulationThread();
    SimulationThread(const SimulationThread&) = delete;
    SimulationThread& operator=(const SimulationThread&) = delete;
    SimulationThread(SimulationThread&&) = delete;
    SimulationThread& operator=(SimulationThread&&) = delete;

    // Commands are thread-safe and wake the worker; it applies them before
    // its next step.
    void SetPaused(bool paused);
    // Requested pause state, which the worker may not have applied yet.
    [[nodiscard]] bool IsPaused() const;
    // Queues one step to run while paused. Ignored when not paused.
    void RequestSingleStep();
    // Zero freezes the simulation; negative or non-finite values are ignored.
    void SetTimeScale(double scale);

    [[nodiscard]] SimulationTiming GetTiming() const;

    // Non-null once the step function has thrown. The worker stops at that
    // point; the exception is kept for the owner to inspect or rethrow.
    [[nodiscard]] std::exception_ptr GetFailure() const;

    // Requests the worker to stop and joins it. Safe to call more than once.
    void Stop();

private:
    using SteadyClock = std::chrono::steady_clock;

    void Run(const std::stop_token& stopToken);
    void ApplyCommands(SteadyClock::time_point& lastAdvance);

    StepFunction step_;
    FixedStepClock clock_;

    mutable std::mutex mutex_;
    std::condition_variable_any wake_;
    // Requested state, guarded by mutex_.
    bool requestedPaused_{false};
    double requestedTimeScale_{1.0};
    std::uint32_t pendingSingleSteps_{};
    bool commandsPending_{false};
    // Published state, guarded by mutex_.
    SimulationTiming timing_;
    SteadyClock::time_point timingTakenAt_;
    std::exception_ptr failure_;

    // Declared last so it is joined before the members above are destroyed.
    std::jthread worker_;
};

#endif  // SIMULATION_THREAD_HPP_INCLUDED
