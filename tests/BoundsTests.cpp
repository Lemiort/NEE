#include <gtest/gtest.h>

#include <cmath>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include "Bounds.hpp"

namespace {
// A camera at the origin looking down -z, 90 degrees wide, from 0.1 to 100.
Frustum LookDownMinusZ() {
    const glm::mat4 projection =
        glm::perspective(glm::radians(90.0F), 1.0F, 0.1F, 100.0F);
    const glm::mat4 view =
        glm::lookAt(glm::vec3(0.0F), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    return Frustum::FromViewProjection(projection * view);
}

Bounds Box(glm::vec3 center, float half) {
    return {.min = center - glm::vec3(half), .max = center + glm::vec3(half)};
}
}  // namespace

TEST(Bounds, StartsEmptyAndGrowsAroundPoints) {
    Bounds bounds;
    EXPECT_TRUE(bounds.IsEmpty());
    bounds.Grow(glm::vec3(1.0F, 2.0F, 3.0F));
    EXPECT_FALSE(bounds.IsEmpty());
    EXPECT_EQ(bounds.min, glm::vec3(1.0F, 2.0F, 3.0F));
    EXPECT_EQ(bounds.max, glm::vec3(1.0F, 2.0F, 3.0F));
    bounds.Grow(glm::vec3(-1.0F, 5.0F, 0.0F));
    EXPECT_EQ(bounds.min, glm::vec3(-1.0F, 2.0F, 0.0F));
    EXPECT_EQ(bounds.max, glm::vec3(1.0F, 5.0F, 3.0F));
    Bounds other;
    other.Grow(Bounds{});  // growing by an empty box changes nothing
    EXPECT_TRUE(other.IsEmpty());
    other.Grow(bounds);
    EXPECT_EQ(other.min, bounds.min);
    EXPECT_EQ(other.max, bounds.max);
}

TEST(Bounds, TransformedBoxesHoldEveryMovedCorner) {
    const Bounds unit = Box(glm::vec3(0.0F), 1.0F);
    const glm::mat4 moved =
        glm::translate(glm::mat4(1.0F), glm::vec3(5, 0, 0)) *
        glm::scale(glm::mat4(1.0F), glm::vec3(2, 1, 1));
    const Bounds result = unit.Transformed(moved);
    EXPECT_EQ(result.min, glm::vec3(3.0F, -1.0F, -1.0F));
    EXPECT_EQ(result.max, glm::vec3(7.0F, 1.0F, 1.0F));
    // Turned 45 degrees about y, the box grows to hold its corners.
    const Bounds turned = unit.Transformed(
        glm::rotate(glm::mat4(1.0F), glm::radians(45.0F), glm::vec3(0, 1, 0)));
    EXPECT_NEAR(turned.max.x, std::sqrt(2.0F), 1e-5F);
    EXPECT_NEAR(turned.max.y, 1.0F, 1e-5F);
    EXPECT_TRUE(Bounds{}.Transformed(moved).IsEmpty());
}

TEST(Frustum, KeepsBoxesInViewAndDropsTheRest) {
    const Frustum frustum = LookDownMinusZ();
    EXPECT_TRUE(frustum.Intersects(Box({0, 0, -10}, 1.0F)));     // ahead
    EXPECT_FALSE(frustum.Intersects(Box({0, 0, 10}, 1.0F)));     // behind
    EXPECT_FALSE(frustum.Intersects(Box({30, 0, -10}, 1.0F)));   // to the right
    EXPECT_FALSE(frustum.Intersects(Box({0, -30, -10}, 1.0F)));  // below
    EXPECT_FALSE(frustum.Intersects(Box({0, 0, -200}, 1.0F)));   // too far
    // Straddling the right edge: x = 10 is the edge at 10 units away.
    EXPECT_TRUE(frustum.Intersects(Box({10.5F, 0, -10}, 1.0F)));
    // Around the camera itself.
    EXPECT_TRUE(frustum.Intersects(Box({0, 0, 0}, 1.0F)));
    EXPECT_FALSE(frustum.Intersects(Bounds{}));
}
