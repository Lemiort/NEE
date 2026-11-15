#ifndef FIXED_STEP_CLOCK_HPP_INCLUDED
#define FIXED_STEP_CLOCK_HPP_INCLUDED

#include <cstdint>

// Result of feeding one frame of real time into a FixedStepClock.
struct ClockAdvance {
    // Number of fixed steps the caller must run this frame, in order.
    std::uint32_t steps{};
    // Fraction of a step left in the accumulator, in [0, 1). Render state as
    // lerp(previous, current, alpha) to hide the step granularity.
    double alpha{};
    // True when more time arrived than maxStepsPerAdvance could consume and
    // the excess was discarded, i.e. the simulation is running slower than
    // requested.
    bool droppedTime{false};
};

struct FixedStepClockSettings {
    static constexpr double kDefaultStepsPerSecond = 60.0;
    static constexpr std::uint32_t kDefaultMaxStepsPerAdvance = 8;
    static constexpr double kDefaultMaxFrameSeconds = 0.25;

    // Duration of one fixed step in seconds of scaled time.
    double stepSeconds{1.0 / kDefaultStepsPerSecond};
    // Upper bound on steps per Advance call, guarding against the "spiral of
    // death" where slow updates cause ever more updates per frame.
    std::uint32_t maxStepsPerAdvance{kDefaultMaxStepsPerAdvance};
    // Real frame time is clamped to this before scaling, so a debugger break,
    // window drag or minimized window does not produce a burst of steps.
    double maxFrameSeconds{kDefaultMaxFrameSeconds};
};

// Converts variable real frame times into a deterministic sequence of fixed
// steps. Simulation code only ever sees GetStepSeconds() and the tick index,
// so its results do not depend on frame rate. The clock is pure CPU state and
// does not read any time source itself; callers pass the elapsed real time.
class FixedStepClock {
public:
    // Throws std::invalid_argument when a setting is not positive and finite.
    explicit FixedStepClock(FixedStepClockSettings settings = {});

    // Accumulates `realSeconds` (negative or non-finite values count as zero)
    // multiplied by the time scale, and reports how many steps to run. The
    // reported steps are counted as completed ticks immediately.
    ClockAdvance Advance(double realSeconds) noexcept;

    // While paused, Advance reports no steps and keeps alpha unchanged.
    void SetPaused(bool paused) noexcept;
    [[nodiscard]] bool IsPaused() const noexcept;

    // Requests exactly one step on the next Advance call while paused, for
    // single-stepping a simulation. Ignored when not paused.
    void RequestSingleStep() noexcept;

    // Multiplier from real time to simulation time. Zero is allowed and
    // freezes the clock; negative or non-finite values are ignored.
    void SetTimeScale(double scale) noexcept;
    [[nodiscard]] double GetTimeScale() const noexcept;

    [[nodiscard]] double GetStepSeconds() const noexcept;
    [[nodiscard]] std::uint64_t GetTickCount() const noexcept;
    // Simulation time at the latest tick, computed from the tick count so it
    // does not drift with accumulated floating-point error.
    [[nodiscard]] double GetSimulationSeconds() const noexcept;

    // Returns to tick zero with an empty accumulator; keeps settings, pause
    // state and time scale.
    void Reset() noexcept;

private:
    FixedStepClockSettings settings_;
    double accumulator_{};
    double timeScale_{1.0};
    std::uint64_t ticks_{};
    bool paused_{false};
    bool singleStepRequested_{false};
};

#endif  // FIXED_STEP_CLOCK_HPP_INCLUDED
