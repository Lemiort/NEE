#ifndef RENDER_MODE_HPP_INCLUDED
#define RENDER_MODE_HPP_INCLUDED

#include <array>
#include <cstddef>
#include <string_view>

enum class RenderMode {
    DeferredShading,
    PositionBuffer,
    DiffuseBuffer,
    NormalBuffer,
    TextureCoordinateBuffer,
    ForwardRendering
};

struct RenderModeDescriptor {
    RenderMode mode;
    std::string_view description;
};

inline constexpr std::array kRenderModes{
    RenderModeDescriptor{RenderMode::DeferredShading, "Deferred shading"},
    RenderModeDescriptor{RenderMode::PositionBuffer, "Position buffer"},
    RenderModeDescriptor{RenderMode::DiffuseBuffer, "Diffuse buffer"},
    RenderModeDescriptor{RenderMode::NormalBuffer, "Normal buffer"},
    RenderModeDescriptor{RenderMode::TextureCoordinateBuffer,
                         "Texture-coordinate buffer"},
    RenderModeDescriptor{RenderMode::ForwardRendering, "Forward rendering"},
};

[[nodiscard]] constexpr std::string_view GetRenderModeDescription(
    RenderMode mode) {
    for (const auto& descriptor : kRenderModes) {
        if (descriptor.mode == mode) {
            return descriptor.description;
        }
    }
    return "Unknown rendering mode";
}

[[nodiscard]] constexpr RenderMode GetNextRenderMode(RenderMode mode) {
    for (std::size_t index = 0; index < kRenderModes.size(); ++index) {
        if (kRenderModes.at(index).mode == mode) {
            return kRenderModes.at((index + 1) % kRenderModes.size()).mode;
        }
    }
    return kRenderModes.front().mode;
}

#endif  // RENDER_MODE_HPP_INCLUDED
