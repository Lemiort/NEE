#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.hpp"
#include "FontMetrics.hpp"
#include "RenderQueue.hpp"
#include "Text2D.hpp"
#include "TextLayout.hpp"
#include "Transform.hpp"
#include "Transformable.hpp"

namespace {
constexpr float kTolerance = 1e-4F;

glm::vec3 rotate(float x, float y, float z, const glm::vec3& direction) {
    Transform transform;
    transform.rotationDegrees = glm::vec3(x, y, z);
    return {transform.ModelMatrix() * glm::vec4(direction, 0.0F)};
}
}  // namespace

TEST(MathMigration, DefaultVectorsAreZeroInitialized) {
    const glm::vec3 value;
    EXPECT_EQ(value.x, 0.0F);
    EXPECT_EQ(value.y, 0.0F);
    EXPECT_EQ(value.z, 0.0F);
}

TEST(ObjectTransform, UsesRightHandedEulerAxes) {
    const glm::vec3 rightAfterYaw = rotate(0, 90, 0, glm::vec3(1, 0, 0));
    const glm::vec3 upAfterPitch = rotate(90, 0, 0, glm::vec3(0, 1, 0));
    const glm::vec3 upAfterRoll = rotate(0, 0, 90, glm::vec3(1, 0, 0));
    const glm::vec3 upAfterTwoRotations = rotate(90, 90, 0, glm::vec3(0, 1, 0));

    EXPECT_NEAR(rightAfterYaw.z, -1.0F, kTolerance);
    EXPECT_NEAR(upAfterPitch.z, 1.0F, kTolerance);
    EXPECT_NEAR(upAfterRoll.y, 1.0F, kTolerance);
    EXPECT_NEAR(upAfterTwoRotations.x, 1.0F, kTolerance);
}

TEST(ObjectTransform, AppliesScaleThenRotationThenTranslation) {
    const Transform model{.position = glm::vec3(5, 6, 7),
                          .rotationDegrees = glm::vec3(0, 90, 0),
                          .scale = glm::vec3(2, 3, 4)};

    const glm::vec4 transformed = model.ModelMatrix() * glm::vec4(1, 0, 0, 1);
    EXPECT_NEAR(transformed.x, 5.0F, kTolerance);
    EXPECT_NEAR(transformed.y, 6.0F, kTolerance);
    EXPECT_NEAR(transformed.z, 5.0F, kTolerance);
}

TEST(ObjectTransform, RotationMatrixOmitsTranslationAndScale) {
    const Transform transform{.position = glm::vec3(5, 6, 7),
                              .rotationDegrees = glm::vec3(0, 90, 0),
                              .scale = glm::vec3(2, 3, 4)};
    const glm::vec4 rotated =
        transform.RotationMatrix() * glm::vec4(1, 0, 0, 1);
    EXPECT_NEAR(rotated.x, 0.0F, kTolerance);
    EXPECT_NEAR(rotated.y, 0.0F, kTolerance);
    EXPECT_NEAR(rotated.z, -1.0F, kTolerance);
    EXPECT_NEAR(rotated.w, 1.0F, kTolerance);
}

TEST(ObjectTransform, TransformableKeepsSpatialStateTogether) {
    Transformable object;
    object.SetPosition(1.0F, 2.0F, 3.0F);
    object.SetRotation(glm::vec3(10.0F, 20.0F, 30.0F));
    object.SetScale(2.0F, 3.0F, 4.0F);

    EXPECT_EQ(object.GetTransform().position, glm::vec3(1.0F, 2.0F, 3.0F));
    EXPECT_EQ(object.GetTransform().rotationDegrees,
              glm::vec3(10.0F, 20.0F, 30.0F));
    EXPECT_EQ(object.GetTransform().scale, glm::vec3(2.0F, 3.0F, 4.0F));
}

TEST(ObjectTransform, CanBeUsedThroughTransformInterface) {
    Transformable object;
    ITransformable& transformable = object;
    transformable.SetTransform(Transform{.position = glm::vec3(4.0F)});

    EXPECT_EQ(transformable.GetTransform().position, glm::vec3(4.0F));
    EXPECT_EQ(transformable.GetTransform().scale, glm::vec3(1.0F));
}

TEST(TextLayout, KeepsPositionAndFontSizeAsDistinctProperties) {
    const TextLayout layout{.position = glm::vec2(-0.5F, 0.25F),
                            .fontSizePixels = 36.0F};

    EXPECT_EQ(layout.position, glm::vec2(-0.5F, 0.25F));
    EXPECT_EQ(layout.fontSizePixels, 36.0F);
}

TEST(FontMetrics, StoresGlyphsAndDefaultsMissingKerningToZero) {
    FontMetrics metrics;
    metrics.SetFont("Example", 32);
    metrics.SetAtlasSize(256, 128);
    metrics.AddCharacter(u'A', FontCharacter{.width = 12, .height = 20});
    metrics.AddKerning(u'A', u'V', -1.5F);

    EXPECT_EQ(metrics.GetFontName(), "Example");
    EXPECT_EQ(metrics.GetFontHeight(), 32U);
    EXPECT_EQ(metrics.GetAtlasWidth(), 256U);
    EXPECT_EQ(metrics.GetCharacter(u'A').width, 12U);
    EXPECT_FLOAT_EQ(metrics.GetKerning(u'A', u'V'), -1.5F);
    EXPECT_FLOAT_EQ(metrics.GetKerning(u'A', u'A'), 0.0F);
}

TEST(TextModel, StoresPresentationDataWithoutRenderingDependencies) {
    FontLine2d line;
    line.SetText("status");
    line.SetPosition(-0.75F, 0.5F);
    line.SetFontSize(24.0F);
    line.SetColor(glm::vec4(0.25F, 0.5F, 0.75F, 1.0F));

    EXPECT_EQ(line.GetText(), "status");
    EXPECT_EQ(line.GetPosition(), glm::vec2(-0.75F, 0.5F));
    EXPECT_EQ(line.GetFontSize(), 24.0F);
    EXPECT_EQ(line.GetColor(), glm::vec4(0.25F, 0.5F, 0.75F, 1.0F));
}

TEST(RenderQueue, SnapshotsTextAtSubmissionTime) {
    RenderQueue queue;
    FontLine2d line;
    line.SetText("first");
    line.SetPosition(-1.0F, 0.5F);
    queue.SubmitText(line);

    line.SetText("second");
    line.SetPosition(0.0F, 0.0F);
    queue.SubmitText(line);

    ASSERT_EQ(queue.GetText().size(), 2U);
    EXPECT_EQ(queue.GetText()[0].text.GetText(), "first");
    EXPECT_EQ(queue.GetText()[0].text.GetPosition(), glm::vec2(-1.0F, 0.5F));
    EXPECT_EQ(queue.GetText()[1].text.GetText(), "second");
}

TEST(RenderQueue, KeepsDebugDataIndependentFromDebugObjects) {
    RenderQueue queue;
    FontLine2d text;
    text.SetText("overlay");
    queue.SubmitText(text);
    queue.SubmitDebugLine({.start = glm::vec3(1.0F, 2.0F, 3.0F),
                           .end = glm::vec3(4.0F, 5.0F, 6.0F),
                           .color = glm::vec3(1.0F, 0.0F, 0.0F)});

    ASSERT_EQ(queue.GetDebugLines().size(), 1U);
    EXPECT_EQ(queue.GetDebugLines().front().end, glm::vec3(4.0F, 5.0F, 6.0F));
    queue.ClearDebugLines();
    EXPECT_TRUE(queue.GetDebugLines().empty());
    EXPECT_EQ(queue.GetText().size(), 1U);
    queue.Clear();
    EXPECT_TRUE(queue.GetDebugLines().empty());
    EXPECT_TRUE(queue.GetText().empty());
}

TEST(CameraControls, KeyboardMovesAlongStandardAxes) {
    Camera camera(800, 600, 60, 1, 100);
    EXPECT_NEAR(camera.GetTarget().z, -1.0F, kTolerance);

    camera.OnKeyboard(GLFW_KEY_A);
    EXPECT_NEAR(camera.GetPos().x, -0.1F, kTolerance);
    camera.OnKeyboard(GLFW_KEY_D);
    EXPECT_NEAR(camera.GetPos().x, 0.0F, kTolerance);
    camera.OnKeyboard(GLFW_KEY_W);
    EXPECT_NEAR(camera.GetPos().z, -0.1F, kTolerance);
}

TEST(CameraControls, MouseTurnsRightAndDown) {
    Camera camera(800, 600, 60, 1, 100);
    camera.OnMouse(420, 300);
    EXPECT_GT(camera.GetTarget().x, 0.0F);
    camera.OnMouse(420, 320);
    EXPECT_LT(camera.GetTarget().y, 0.0F);
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
    Camera const camera(800, 600, 60, 1, 100);
    const glm::mat4 projectionView = camera.GetViewProjectionMatrix();
    const glm::vec4 nearClip = projectionView * glm::vec4(0, 0, -1, 1);
    const glm::vec4 farClip = projectionView * glm::vec4(0, 0, -100, 1);
    EXPECT_NEAR(nearClip.z / nearClip.w, -1.0F, kTolerance);
    EXPECT_NEAR(farClip.z / farClip.w, 1.0F, kTolerance);
}

TEST(CameraProjection, ViewMatrixUsesPositionAndForwardDirection) {
    Camera const camera(800, 600, 60, 1, 100, glm::vec3(2, 3, 4),
                        glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    const glm::mat4 view = camera.GetViewMatrix();
    const glm::vec4 eye = view * glm::vec4(2, 3, 4, 1);
    const glm::vec4 ahead = view * glm::vec4(2, 3, 3, 1);
    EXPECT_NEAR(eye.x, 0.0F, kTolerance);
    EXPECT_NEAR(eye.y, 0.0F, kTolerance);
    EXPECT_NEAR(eye.z, 0.0F, kTolerance);
    EXPECT_NEAR(ahead.z, -1.0F, kTolerance);
}

TEST(CameraProjection, WorldRightAppearsOnScreenRight) {
    Camera const camera(800, 600, 60, 1, 100);

    const glm::vec4 rightClip =
        camera.GetViewProjectionMatrix() * glm::vec4(1, 0, -3, 1);
    EXPECT_GT(rightClip.x / rightClip.w, 0.0F);
}

TEST(MathMigration, GlmMatricesExposeColumnMajorData) {
    const glm::mat4 matrix(1.0F);
    // Both indices are constant and within GLM's fixed-size matrix.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    EXPECT_EQ(glm::value_ptr(matrix), &matrix[0][0]);
}
