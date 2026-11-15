#include "FixedStepClock.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace {
bool IsPositiveFinite(double value) noexcept {
    return std::isfinite(value) && value > 0.0;
}
}  // namespace

FixedStepClock::FixedStepClock(FixedStepClockSettings settings)
    : settings_(settings) {
    if (!IsPositiveFinite(settings_.stepSeconds)) {
        throw std::invalid_argument(
            "FixedStepClock step duration must be positive and finite");
    }
    if (settings_.maxStepsPerAdvance == 0) {
        throw std::invalid_argument(
            "FixedStepClock must allow at least one step per advance");
    }
    if (!IsPositiveFinite(settings_.maxFrameSeconds)) {
        throw std::invalid_argument(
            "FixedStepClock maximum frame time must be positive and finite");
    }
}

ClockAdvance FixedStepClock::Advance(double realSeconds) noexcept {
    ClockAdvance result;
    if (paused_) {
        if (singleStepRequested_) {
            singleStepRequested_ = false;
            result.steps = 1;
            ++ticks_;
        }
        result.alpha = accumulator_ / settings_.stepSeconds;
        return result;
    }

    const double frameSeconds =
        std::isfinite(realSeconds)
            ? std::clamp(realSeconds, 0.0, settings_.maxFrameSeconds)
            : 0.0;
    accumulator_ += frameSeconds * timeScale_;

    while (accumulator_ >= settings_.stepSeconds &&
           result.steps < settings_.maxStepsPerAdvance) {
        accumulator_ -= settings_.stepSeconds;
        ++result.steps;
    }
    if (accumulator_ >= settings_.stepSeconds) {
        // Keep only the fractional part so alpha stays meaningful.
        accumulator_ = std::fmod(accumulator_, settings_.stepSeconds);
        result.droppedTime = true;
    }
    ticks_ += result.steps;
    result.alpha =
        std::clamp(accumulator_ / settings_.stepSeconds, 0.0, 1.0);
    if (result.alpha >= 1.0) {
        // Rounding in fmod can leave a value a hair below the step size.
        result.alpha = std::nextafter(1.0, 0.0);
    }
    return result;
}

void FixedStepClock::SetPaused(bool paused) noexcept {
    paused_ = paused;
    if (!paused_) {
        singleStepRequested_ = false;
    }
}

bool FixedStepClock::IsPaused() const noexcept { return paused_; }

void FixedStepClock::RequestSingleStep() noexcept {
    if (paused_) {
        singleStepRequested_ = true;
    }
}

void FixedStepClock::SetTimeScale(double scale) noexcept {
    if (std::isfinite(scale) && scale >= 0.0) {
        timeScale_ = scale;
    }
}

double FixedStepClock::GetTimeScale() const noexcept { return timeScale_; }

double FixedStepClock::GetStepSeconds() const noexcept {
    return settings_.stepSeconds;
}

std::uint64_t FixedStepClock::GetTickCount() const noexcept { return ticks_; }

double FixedStepClock::GetSimulationSeconds() const noexcept {
    return static_cast<double>(ticks_) * settings_.stepSeconds;
}

void FixedStepClock::Reset() noexcept {
    accumulator_ = 0.0;
    ticks_ = 0;
    singleStepRequested_ = false;
}
