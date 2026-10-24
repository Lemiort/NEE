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
class GpuResourceOwnership : public testing::Test {};

using ResourceTypes = testing::Types<Shader, GBuffer, CubemapTexture,
                                     ShadowMapFBO, RandomTexture>;
TYPED_TEST_SUITE(GpuResourceOwnership, ResourceTypes);

TYPED_TEST(GpuResourceOwnership, CannotDuplicateOwnershipByCopyOrMove) {
    EXPECT_FALSE(std::is_copy_constructible_v<TypeParam>);
    EXPECT_FALSE(std::is_copy_assignable_v<TypeParam>);
    // An rvalue must not fall back to an implicit copy of the GL handles.
    EXPECT_FALSE(std::is_move_constructible_v<TypeParam>);
    EXPECT_FALSE(std::is_move_assignable_v<TypeParam>);
}
}  // namespace
