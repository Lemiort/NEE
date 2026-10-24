#include <gtest/gtest.h>

#include <type_traits>

#include "CubemapTexture.hpp"
#include "GBuffer.hpp"
#include "RandomTexture.hpp"
#include "Shader.hpp"
#include "ShadowMapFBO.hpp"

namespace {
// Trait checks require no OpenGL context or GPU-resource construction.
template <typename Resource>
void ExpectUniqueOwnership() {
    EXPECT_FALSE(std::is_copy_constructible_v<Resource>);
    EXPECT_FALSE(std::is_copy_assignable_v<Resource>);
    // An rvalue must not fall back to an implicit copy of the GL handles.
    EXPECT_FALSE(std::is_move_constructible_v<Resource>);
    EXPECT_FALSE(std::is_move_assignable_v<Resource>);
}

TEST(GpuResourceOwnership, ShaderCannotDuplicateHandles) {
    ExpectUniqueOwnership<Shader>();
}

TEST(GpuResourceOwnership, GBufferCannotDuplicateHandles) {
    ExpectUniqueOwnership<GBuffer>();
}

TEST(GpuResourceOwnership, CubemapCannotDuplicateHandles) {
    ExpectUniqueOwnership<CubemapTexture>();
}

TEST(GpuResourceOwnership, ShadowMapCannotDuplicateHandles) {
    ExpectUniqueOwnership<ShadowMapFBO>();
}

TEST(GpuResourceOwnership, RandomTextureCannotDuplicateHandles) {
    ExpectUniqueOwnership<RandomTexture>();
}
}  // namespace
