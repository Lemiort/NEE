#ifndef RENDER_STATISTICS_HPP_INCLUDED
#define RENDER_STATISTICS_HPP_INCLUDED

#include <cstddef>

// What a pipeline drew of the opaque meshes and batches in its last frame.
struct RenderStatistics {
    // Per-frame mesh commands drawn, and skipped as outside the view.
    std::size_t meshesDrawn{};
    std::size_t meshesCulled{};
    // Mesh batches drawn and skipped, and the instances drawn in them.
    std::size_t batchesDrawn{};
    std::size_t batchesCulled{};
    std::size_t batchInstancesDrawn{};
    // Draw calls for all of the above.
    std::size_t drawCalls{};

    RenderStatistics& operator+=(const RenderStatistics& other) noexcept {
        meshesDrawn += other.meshesDrawn;
        meshesCulled += other.meshesCulled;
        batchesDrawn += other.batchesDrawn;
        batchesCulled += other.batchesCulled;
        batchInstancesDrawn += other.batchInstancesDrawn;
        drawCalls += other.drawCalls;
        return *this;
    }
};

#endif  // RENDER_STATISTICS_HPP_INCLUDED
