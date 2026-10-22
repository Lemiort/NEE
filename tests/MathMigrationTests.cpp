#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

#include <glm/gtc/type_ptr.hpp>

#include "Camera.hpp"
#include "Transform.hpp"

namespace {
constexpr float kTolerance = 1e-4f;

glm::vec3 rotate(float x, float y, float z, const glm::vec3& direction) {
    Transform transform;
    transform.rotationDegrees = glm::vec3(x, y, z);
    return glm::vec3(transform.ModelMatrix() * glm::vec4(direction, 0.0f));
}
}  // namespace

TEST(MathMigration, DefaultVectorsAreZeroInitialized) {
    const glm::vec3 value;
    EXPECT_EQ(value.x, 0.0f);
    EXPECT_EQ(value.y, 0.0f);
    EXPECT_EQ(value.z, 0.0f);
}

TEST(ObjectTransform, UsesRightHandedEulerAxes) {
    const glm::vec3 rightAfterYaw = rotate(0, 90, 0, glm::vec3(1, 0, 0));
    const glm::vec3 upAfterPitch = rotate(90, 0, 0, glm::vec3(0, 1, 0));
    const glm::vec3 upAfterRoll = rotate(0, 0, 90, glm::vec3(1, 0, 0));
    const glm::vec3 upAfterTwoRotations = rotate(90, 90, 0, glm::vec3(0, 1, 0));

    EXPECT_NEAR(rightAfterYaw.z, -1.0f, kTolerance);
    EXPECT_NEAR(upAfterPitch.z, 1.0f, kTolerance);
    EXPECT_NEAR(upAfterRoll.y, 1.0f, kTolerance);
    EXPECT_NEAR(upAfterTwoRotations.x, 1.0f, kTolerance);
}

TEST(ObjectTransform, AppliesScaleThenRotationThenTranslation) {
    const Transform model{glm::vec3(5, 6, 7), glm::vec3(0, 90, 0),
                          glm::vec3(2, 3, 4)};

    const glm::vec4 transformed = model.ModelMatrix() * glm::vec4(1, 0, 0, 1);
    EXPECT_NEAR(transformed.x, 5.0f, kTolerance);
    EXPECT_NEAR(transformed.y, 6.0f, kTolerance);
    EXPECT_NEAR(transformed.z, 5.0f, kTolerance);
}

TEST(ObjectTransform, RotationMatrixOmitsTranslationAndScale) {
    const Transform transform{glm::vec3(5, 6, 7), glm::vec3(0, 90, 0),
                              glm::vec3(2, 3, 4)};
    const glm::vec4 rotated =
        transform.RotationMatrix() * glm::vec4(1, 0, 0, 1);
    EXPECT_NEAR(rotated.x, 0.0f, kTolerance);
    EXPECT_NEAR(rotated.y, 0.0f, kTolerance);
    EXPECT_NEAR(rotated.z, -1.0f, kTolerance);
    EXPECT_NEAR(rotated.w, 1.0f, kTolerance);
}

TEST(CameraControls, KeyboardMovesAlongStandardAxes) {
    Camera camera(800, 600, 60, 1, 100);
    EXPECT_NEAR(camera.GetTarget().z, -1.0f, kTolerance);

    camera.OnKeyboard(GLFW_KEY_A);
    EXPECT_NEAR(camera.GetPos().x, -0.1f, kTolerance);
    camera.OnKeyboard(GLFW_KEY_D);
    EXPECT_NEAR(camera.GetPos().x, 0.0f, kTolerance);
    camera.OnKeyboard(GLFW_KEY_W);
    EXPECT_NEAR(camera.GetPos().z, -0.1f, kTolerance);
}

TEST(CameraControls, MouseTurnsRightAndDown) {
    Camera camera(800, 600, 60, 1, 100);
    camera.OnMouse(420, 300);
    EXPECT_GT(camera.GetTarget().x, 0.0f);
    camera.OnMouse(420, 320);
    EXPECT_LT(camera.GetTarget().y, 0.0f);
}

TEST(CameraControls, UnchangedMousePositionPreservesInitialHeading) {
    for (const glm::vec3 forward : {glm::vec3(1, 0, 0), glm::vec3(0, 0, 1),
                                    glm::vec3(-1, 0, 0), glm::vec3(0, 0, -1)}) {
        SCOPED_TRACE(::testing::Message() << "forward: " << forward.x << ", "
                                          << forward.y << ", " << forward.z);
        Camera camera(800, 600, 60, 1, 100, glm::vec3(0), forward,
                      glm::vec3(0, 1, 0));
        camera.OnMouse(400, 300);
        EXPECT_LT(glm::distance(camera.GetTarget(), forward), kTolerance);
    }
}

TEST(CameraProjection, MapsNearAndFarPlanesToOpenGLDepth) {
    Camera camera(800, 600, 60, 1, 100);
    const glm::mat4 projectionView = camera.GetViewProjectionMatrix();
    const glm::vec4 nearClip = projectionView * glm::vec4(0, 0, -1, 1);
    const glm::vec4 farClip = projectionView * glm::vec4(0, 0, -100, 1);
    EXPECT_NEAR(nearClip.z / nearClip.w, -1.0f, kTolerance);
    EXPECT_NEAR(farClip.z / farClip.w, 1.0f, kTolerance);
}

TEST(CameraProjection, ViewMatrixUsesPositionAndForwardDirection) {
    Camera camera(800, 600, 60, 1, 100, glm::vec3(2, 3, 4), glm::vec3(0, 0, -1),
                  glm::vec3(0, 1, 0));
    const glm::mat4 view = camera.GetViewMatrix();
    const glm::vec4 eye = view * glm::vec4(2, 3, 4, 1);
    const glm::vec4 ahead = view * glm::vec4(2, 3, 3, 1);
    EXPECT_NEAR(eye.x, 0.0f, kTolerance);
    EXPECT_NEAR(eye.y, 0.0f, kTolerance);
    EXPECT_NEAR(eye.z, 0.0f, kTolerance);
    EXPECT_NEAR(ahead.z, -1.0f, kTolerance);
}

TEST(CameraProjection, WorldRightAppearsOnScreenRight) {
    Camera camera(800, 600, 60, 1, 100);

    const glm::vec4 rightClip =
        camera.GetViewProjectionMatrix() * glm::vec4(1, 0, -3, 1);
    EXPECT_GT(rightClip.x / rightClip.w, 0.0f);
}

TEST(MathMigration, GlmMatricesExposeColumnMajorData) {
    const glm::mat4 matrix(1.0f);
    EXPECT_EQ(glm::value_ptr(matrix), &matrix[0][0]);
}
