#include "SimulationThread.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <stop_token>
#include <thread>
#include <utility>

#include "FixedStepClock.hpp"

SimulationThread::SimulationThread(FixedStepClockSettings settings,
                                   StepFunction step)
    : step_(std::move(step)), clock_(settings) {
    if (!step_) {
        throw std::invalid_argument("SimulationThread requires a step function");
    }
    timing_.stepSeconds = clock_.GetStepSeconds();
    timingTakenAt_ = SteadyClock::now();
    worker_ = std::jthread(
        [this](const std::stop_token& stopToken) { Run(stopToken); });
}

SimulationThread::~SimulationThread() { Stop(); }

void SimulationThread::SetPaused(bool paused) {
    {
        const std::scoped_lock lock(mutex_);
        requestedPaused_ = paused;
        commandsPending_ = true;
    }
    wake_.notify_all();
}

bool SimulationThread::IsPaused() const {
    const std::scoped_lock lock(mutex_);
    return requestedPaused_;
}

void SimulationThread::RequestSingleStep() {
    {
        const std::scoped_lock lock(mutex_);
        if (!requestedPaused_) {
            return;
        }
        if (pendingSingleSteps_ < UINT32_MAX) {
            ++pendingSingleSteps_;
        }
        commandsPending_ = true;
    }
    wake_.notify_all();
}

void SimulationThread::SetTimeScale(double scale) {
    if (!std::isfinite(scale) || scale < 0.0) {
        return;
    }
    {
        const std::scoped_lock lock(mutex_);
        requestedTimeScale_ = scale;
        commandsPending_ = true;
    }
    wake_.notify_all();
}

SimulationTiming SimulationThread::GetTiming() const {
    const auto now = SteadyClock::now();
    const std::scoped_lock lock(mutex_);
    SimulationTiming timing = timing_;
    if (!timing.paused && timing.timeScale > 0.0 && failure_ == nullptr) {
        const double elapsed =
            std::chrono::duration<double>(now - timingTakenAt_).count();
        timing.estimatedSeconds += std::max(elapsed, 0.0) * timing.timeScale;
    }
    // A worker that falls behind must not let renderers run ahead of the
    // steps that have actually been published.
    const double nextStepSeconds =
        static_cast<double>(timing.tick + 1) * timing.stepSeconds;
    timing.estimatedSeconds = std::min(timing.estimatedSeconds, nextStepSeconds);
    return timing;
}

std::exception_ptr SimulationThread::GetFailure() const {
    const std::scoped_lock lock(mutex_);
    return failure_;
}

void SimulationThread::Stop() {
    if (worker_.joinable()) {
        worker_.request_stop();
        worker_.join();
    }
}

void SimulationThread::ApplyCommands(SteadyClock::time_point& lastAdvance) {
    if (!commandsPending_) {
        return;
    }
    const bool wasPaused = clock_.IsPaused();
    clock_.SetPaused(requestedPaused_);
    clock_.SetTimeScale(requestedTimeScale_);
    if (wasPaused && !clock_.IsPaused()) {
        // Time spent paused must not turn into a burst of catch-up steps.
        lastAdvance = SteadyClock::now();
    }
    if (clock_.IsPaused()) {
        if (pendingSingleSteps_ > 0) {
            clock_.RequestSingleStep();
            --pendingSingleSteps_;
        }
    } else {
        pendingSingleSteps_ = 0;
    }
    // The clock takes one single step per advance; keep the rest queued.
    commandsPending_ = pendingSingleSteps_ > 0;
}

void SimulationThread::Run(const std::stop_token& stopToken) {
    auto lastAdvance = SteadyClock::now();
    while (!stopToken.stop_requested()) {
        {
            const std::scoped_lock lock(mutex_);
            ApplyCommands(lastAdvance);
        }

        const auto now = SteadyClock::now();
        const double elapsed =
            std::chrono::duration<double>(now - lastAdvance).count();
        lastAdvance = now;
        const std::uint64_t ticksBefore = clock_.GetTickCount();
        const ClockAdvance advance = clock_.Advance(elapsed);
        const double stepSeconds = clock_.GetStepSeconds();

        // On stop or failure, publish only the steps that actually ran.
        const auto finish = [&](std::uint64_t completedTick,
                                std::exception_ptr failure) {
            const std::scoped_lock lock(mutex_);
            failure_ = std::move(failure);
            timing_.tick = completedTick;
            timing_.estimatedSeconds =
                static_cast<double>(completedTick) * stepSeconds;
        };
        for (std::uint32_t index = 0; index < advance.steps; ++index) {
            const std::uint64_t tick = ticksBefore + index + 1;
            if (stopToken.stop_requested()) {
                finish(tick - 1, nullptr);
                return;
            }
            try {
                step_(SimulationStep{.tick = tick, .stepSeconds = stepSeconds});
            } catch (...) {
                finish(tick - 1, std::current_exception());
                return;
            }
        }

        std::unique_lock lock(mutex_);
        timing_.tick = clock_.GetTickCount();
        timing_.estimatedSeconds =
            (static_cast<double>(timing_.tick) + advance.alpha) * stepSeconds;
        timing_.paused = clock_.IsPaused();
        timing_.timeScale = clock_.GetTimeScale();
        timingTakenAt_ = now;

        if (commandsPending_) {
            continue;
        }
        const auto hasCommands = [this] { return commandsPending_; };
        if (clock_.IsPaused() || clock_.GetTimeScale() <= 0.0) {
            wake_.wait(lock, stopToken, hasCommands);
        } else {
            // Wake at least every maxFrameSeconds: the clock clamps longer
            // gaps as stalls, so one long sleep at a slow time scale would
            // lose most of its time and run the simulation slower than asked.
            const double secondsUntilNextStep =
                std::min((1.0 - advance.alpha) * stepSeconds /
                             clock_.GetTimeScale(),
                         clock_.GetMaxFrameSeconds());
            const auto deadline =
                now + std::chrono::duration_cast<SteadyClock::duration>(
                          std::chrono::duration<double>(secondsUntilNextStep));
            wake_.wait_until(lock, stopToken, deadline, hasCommands);
        }
    }
}
