#ifndef SNAPSHOT_BUFFER_HPP_INCLUDED
#define SNAPSHOT_BUFFER_HPP_INCLUDED

#include <algorithm>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

// Hands immutable snapshots from a simulation thread to a render thread.
// The writer publishes one snapshot per completed step; the reader samples the
// latest two and an interpolation factor for any render time. Published
// snapshots are shared, never copied, so readers keep a snapshot alive for as
// long as they hold it, while the writer moves on.
template <typename T>
class SnapshotBuffer {
public:
    struct Entry {
        // Simulation time the snapshot describes, normally the end of a step.
        double simulationSeconds{};
        T value;
    };

    struct Sample {
        std::shared_ptr<const Entry> previous;
        std::shared_ptr<const Entry> current;
        // Position of the sampled time between previous and current, in
        // [0, 1]. Render lerp(previous->value, current->value, alpha).
        double alpha{};
    };

    // Thread-safe. Snapshots must be published in increasing time order.
    void Publish(double simulationSeconds, T value) {
        auto entry = std::make_shared<const Entry>(
            Entry{.simulationSeconds = simulationSeconds,
                  .value = std::move(value)});
        const std::scoped_lock lock(mutex_);
        previous_ = std::move(current_);
        current_ = std::move(entry);
    }

    // Thread-safe. Returns nothing until the first snapshot is published.
    // With a single snapshot, previous and current are the same entry.
    [[nodiscard]] std::optional<Sample> SampleAt(double renderSeconds) const {
        std::shared_ptr<const Entry> previous;
        std::shared_ptr<const Entry> current;
        {
            const std::scoped_lock lock(mutex_);
            previous = previous_;
            current = current_;
        }
        if (current == nullptr) {
            return std::nullopt;
        }
        if (previous == nullptr) {
            previous = current;
        }
        const double span =
            current->simulationSeconds - previous->simulationSeconds;
        const double alpha =
            span > 0.0 ? std::clamp((renderSeconds -
                                     previous->simulationSeconds) /
                                        span,
                                    0.0, 1.0)
                       : 1.0;
        return Sample{.previous = std::move(previous),
                      .current = std::move(current),
                      .alpha = alpha};
    }

    // Thread-safe. Drops both snapshots.
    void Clear() {
        const std::scoped_lock lock(mutex_);
        previous_.reset();
        current_.reset();
    }

private:
    mutable std::mutex mutex_;
    std::shared_ptr<const Entry> previous_;
    std::shared_ptr<const Entry> current_;
};

#endif  // SNAPSHOT_BUFFER_HPP_INCLUDED
