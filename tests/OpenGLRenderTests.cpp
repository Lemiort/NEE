#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <string>
#include <vector>

#include "CubemapTexture.hpp"
#include "GBuffer.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "MeshInstanceRenderer.hpp"
#include "ModelLoader.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

namespace {
constexpr int kSize = 128;
using Image = std::vector<std::uint8_t>;

std::filesystem::path Asset(const std::string& path) {
    return std::filesystem::path(NEE_TEST_ASSET_ROOT) / path;
}

std::filesystem::path WriteTga(const std::string& name, int channels) {
    const auto directory = std::filesystem::path(NEE_TEST_ARTIFACT_ROOT);
    std::filesystem::create_directories(directory);
    const auto path = directory / name;
    constexpr int width = 3;
    constexpr int height = 2;
    std::vector<std::uint8_t> bytes(18 + width * height * channels, 0);
    bytes[2] = channels <= 2 ? 3 : 2;
    bytes[12] = width;
    bytes[14] = height;
    bytes[16] = static_cast<std::uint8_t>(channels * 8);
    bytes[17] = 0x20;
    for (std::size_t i = 18; i < bytes.size(); ++i) {
        bytes[i] = static_cast<std::uint8_t>((i * 17) % 251);
    }
    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
    return path;
}

std::shared_ptr<Shader> LoadShader(const std::string& vertex,
                                   const std::string& fragment,
                                   const std::string& geometry = {}) {
    auto shader = std::make_shared<Shader>();
    shader->AddShader(ReadFile(Asset(vertex)), ShaderType::Vertex);
    if (!fragment.empty())
        shader->AddShader(ReadFile(Asset(fragment)), ShaderType::Fragment);
    if (!geometry.empty())
        shader->AddShader(ReadFile(Asset(geometry)), ShaderType::Geometry);
    shader->Link();
    return shader;
}

std::shared_ptr<Texture2D> SolidTexture(
    const std::array<std::uint8_t, 4>& color) {
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 color.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return std::make_shared<Texture2D>(texture);
}

class OpenGLRender : public testing::Test {
protected:
    void SetUp() override {
        glfwSetErrorCallback([](int code, const char* message) {
            ADD_FAILURE() << "GLFW " << code << ": " << message;
        });
        ASSERT_TRUE(glfwInit())
            << "A display is required; use xvfb-run on Linux.";
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 0);
        window_ = glfwCreateWindow(kSize, kSize, "NEE rendering tests", nullptr,
                                   nullptr);
        ASSERT_NE(window_, nullptr);
        glfwMakeContextCurrent(window_);
        ASSERT_NE(gladLoadGL(glfwGetProcAddress), 0);
        glViewport(0, 0, kSize, kSize);
        glDisable(GL_DITHER);
        glDisable(GL_MULTISAMPLE);
        ASSERT_EQ(glGetError(), GL_NO_ERROR);
    }

    void TearDown() override {
        // Test-local resources are destroyed before the context.
        if (window_ != nullptr) glfwDestroyWindow(window_);
        glfwTerminate();
    }

private:
    GLFWwindow* window_{};
};

TEST_F(OpenGLRender, AssetLoadersReadRepositoryTexturesAndModels) {
    for (const auto* path : {"textures/checker.tga", "textures/normal_map.tga",
                             "textures/specular.tga", "textures/white.png",
                             "textures/monster_hellknight.png"}) {
        SCOPED_TRACE(path);
        Texture2D texture;
        ASSERT_TRUE(texture.Load(Asset(path).string()));
        EXPECT_TRUE(glIsTexture(texture.GetTextureID()));
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
    }
    CubemapTexture cubemap(Asset("textures").string(), "sp3right.tga",
                           "sp3left.tga", "sp3top.tga", "sp3bot.tga",
                           "sp3front.tga", "sp3back.tga");
    ASSERT_TRUE(cubemap.Load());
    cubemap.Bind(GL_TEXTURE0);
    GLint previousCubemap = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &previousCubemap);
    ASSERT_TRUE(cubemap.Load());
    cubemap.Bind(GL_TEXTURE0);
    GLint replacementCubemap = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &replacementCubemap);
    EXPECT_NE(replacementCubemap, previousCubemap);
    EXPECT_FALSE(glIsTexture(static_cast<GLuint>(previousCubemap)));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
    for (const auto* path : {"models/normal_geosphere.obj",
                             "models/normal_plane.obj", "models/teapot.obj"}) {
        SCOPED_TRACE(path);
        ModelData model;
        ASSERT_TRUE(LoadModel(Asset(path).string(), model));
        EXPECT_FALSE(model.vertices.empty());
        EXPECT_FALSE(model.indices.empty());
    }
}

TEST_F(OpenGLRender, TextureUploadsHandleAllChannelCountsAndReloadSafely) {
    const std::array formats{GL_RED, GL_RG, GL_RGB, GL_RGBA};
    Texture2D texture;
    GLuint previousTexture = 0;
    for (int channels = 1; channels <= 4; ++channels) {
        SCOPED_TRACE(channels);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
        ASSERT_TRUE(texture.Load(
            WriteTga("channels-" + std::to_string(channels) + ".tga", channels)
                .string()));
        if (previousTexture != 0) EXPECT_FALSE(glIsTexture(previousTexture));
        previousTexture = texture.GetTextureID();
        GLint internalFormat = 0;
        GLint unpackAlignment = 0;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT,
                                 &internalFormat);
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpackAlignment);
        EXPECT_EQ(internalFormat, formats.at(channels - 1));
        EXPECT_EQ(unpackAlignment, 8);
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
    }

    const GLuint retainedTexture = texture.GetTextureID();
    EXPECT_FALSE(texture.Load(Asset("textures/missing-image.png").string()));
    EXPECT_EQ(texture.GetTextureID(), retainedTexture);
    EXPECT_TRUE(glIsTexture(retainedTexture));
}

TEST_F(OpenGLRender, ProductionShaderProgramsLinkWithoutGlErrors) {
    const std::array pairs{
        std::pair{"vertexShader.vsh", "fragmentShader.fsh"},
        std::pair{"DSGeometryPass.vsh", "DSGeometryPass.fsh"},
        std::pair{"DSPointLight.vsh", "DSPointLight.fsh"},
        std::pair{"DSSpotLight.vsh", "DSSpotLight.fsh"},
        std::pair{"DSDirectionalLight.vsh", "DSDirectionalLight.fsh"},
        std::pair{"text2d.vsh", "text2d.fsh"},
        std::pair{"lightVS.vsh", "lightFS.fsh"},
        std::pair{"skybox.vsh", "skybox.fsh"}};
    for (const auto& [vertex, fragment] : pairs) {
        SCOPED_TRACE(vertex);
        const auto shader = LoadShader(std::string("shaders/") + vertex,
                                       std::string("shaders/") + fragment);
        GLint linked = GL_FALSE;
        glGetProgramiv(shader->GetShaderProgramId(), GL_LINK_STATUS, &linked);
        EXPECT_EQ(linked, GL_TRUE);
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
    }
    const auto billboard =
        LoadShader("shaders/billboard.vsh", "shaders/billboard.fsh",
                   "shaders/billboard.gsh");
    const auto stencil = LoadShader("shaders/DSStencilPass.vsh", "");
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

// Offscreen target avoids default framebuffer formats, MSAA, and window
// visibility.
class RenderTarget {
public:
    RenderTarget() {
        glGenFramebuffers(1, &framebuffer_);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
        glGenTextures(1, &color_);
        glBindTexture(GL_TEXTURE_2D, color_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, color_, 0);
        glGenRenderbuffers(1, &depth_);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize,
                              kSize);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, depth_);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
    }
    ~RenderTarget() {
        glDeleteFramebuffers(1, &framebuffer_);
        glDeleteTextures(1, &color_);
        glDeleteRenderbuffers(1, &depth_);
    }
    RenderTarget(const RenderTarget&) = delete;
    RenderTarget& operator=(const RenderTarget&) = delete;

    Image Read() const {
        Image image(kSize * kSize * 4);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, kSize, kSize, GL_RGBA, GL_UNSIGNED_BYTE,
                     image.data());
        return image;
    }

private:
    GLuint framebuffer_{};
    GLuint color_{};
    GLuint depth_{};
};

void SaveImage(const std::string& name, const Image& image) {
    const auto directory = std::filesystem::path(NEE_TEST_ARTIFACT_ROOT);
    std::filesystem::create_directories(directory);
    std::ofstream file(directory / (name + ".ppm"), std::ios::binary);
    file << "P6\n" << kSize << ' ' << kSize << "\n255\n";
    for (int y = kSize - 1; y >= 0; --y)
        for (int x = 0; x < kSize; ++x)
            file.write(reinterpret_cast<const char*>(image.data() +
                                                     (y * kSize + x) * 4),
                       3);
}

TEST_F(OpenGLRender, DeferredSpotlightKeepsCameraPositionInWorldSpace) {
    constexpr auto fragment = R"(#version 330
        in vec3 fE;
        in vec3 fN;
        in vec3 Tangent;
        in vec2 UV;
        uniform float preserveAttributes;
        out vec4 color;
        void main() {
            vec3 attributes = fN + Tangent + vec3(UV, 0.0);
            color = vec4(fE / 10.0 + preserveAttributes * attributes, 1.0);
        })";
    auto shader = std::make_shared<Shader>();
    shader->AddShader(ReadFile(Asset("shaders/DSSpotLight.vsh")),
                      ShaderType::Vertex);
    shader->AddShader(fragment, ShaderType::Fragment);
    shader->Link();
    auto material = std::make_shared<Material>(gsl_lite::not_null{shader});
    Mesh mesh(gsl_lite::not_null{material},
              Asset("models/cube2x2x2.obj").string());

    RenderTarget target;
    ASSERT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
              GL_FRAMEBUFFER_COMPLETE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader->Use();
    const glm::mat4 projection =
        glm::orthoRH_NO(-2.0F, 2.0F, -2.0F, 2.0F, 0.1F, 10.0F);
    glUniformMatrix4fv(shader->GetUniformLocation("gVC"), 1, GL_FALSE,
                       glm::value_ptr(projection));
    glUniform3f(shader->GetUniformLocation("s_vCamPos"), 2.0F, 3.0F, 4.0F);
    glUniform1f(shader->GetUniformLocation("preserveAttributes"), 0.0F);
    mesh.Draw(Transform{.position = {0.0F, 0.0F, -2.0F}}, *material,
              MeshShaderBindings::FromShader(*shader));

    const Image image = target.Read();
    const std::size_t center =
        static_cast<std::size_t>((kSize / 2 * kSize + kSize / 2) * 4);
    EXPECT_NEAR(image.at(center), 51, 1);
    EXPECT_NEAR(image.at(center + 1), 77, 1);
    EXPECT_NEAR(image.at(center + 2), 102, 1);
    EXPECT_EQ(image.at(center + 3), 255);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(OpenGLRender, DirectionalLightFullscreenTriangleDrawsEachPixelOnce) {
    constexpr auto fragment = R"(#version 330
        out vec4 color;
        void main() {
            color = vec4(0.25, 0.0, 0.0, 1.0);
        })";
    auto shader = std::make_shared<Shader>();
    shader->AddShader(ReadFile(Asset("shaders/DSDirectionalLight.vsh")),
                      ShaderType::Vertex);
    shader->AddShader(fragment, ShaderType::Fragment);
    shader->Link();

    RenderTarget target;
    ASSERT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
              GL_FRAMEBUFFER_COMPLETE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    shader->Use();
    glUniform3f(shader->GetUniformLocation("s_vCamPos"), 0.0F, 0.0F, 0.0F);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    const Image image = target.Read();
    for (std::size_t index = 0; index < image.size(); index += 4) {
        EXPECT_NEAR(image[index], 64, 1);
        EXPECT_EQ(image[index + 1], 0);
        EXPECT_EQ(image[index + 2], 0);
        EXPECT_EQ(image[index + 3], 255);
    }
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(OpenGLRender, InstancedAndIndividualTransformsProduceMatchingImages) {
    // Both production vertex paths must produce the same geometry. The
    // diagnostic fragment consumes every geometry attribute without lighting
    // differences.
    constexpr auto fragment = R"(#version 330
        in vec3 fN;
        in vec3 Tangent;
        in vec2 UV;
        out vec4 color;
        void main() {
            color = vec4(vec3(0.15) + 0.2 * abs(normalize(fN)) +
                         0.2 * abs(normalize(Tangent)) + vec3(0.2 * UV, 0.0), 1.0);
        })";
    std::vector<Image> pipelineImages;
    for (const auto* vertex :
         {"shaders/vertexShader.vsh", "shaders/DSGeometryPass.vsh"}) {
        SCOPED_TRACE(vertex);
        auto shader = std::make_shared<Shader>();
        shader->AddShader(ReadFile(Asset(vertex)), ShaderType::Vertex);
        shader->AddShader(fragment, ShaderType::Fragment);
        shader->Link();
        auto material = std::make_shared<Material>(gsl_lite::not_null{shader});
        Mesh mesh(gsl_lite::not_null{material},
                  Asset("models/cube2x2x2.obj").string());
        const auto bindings = MeshShaderBindings::FromShader(*shader);
        ASSERT_GE(bindings.instanceWorld, 0);
        ASSERT_GE(bindings.instanceRotation, 0);
        ASSERT_GE(bindings.instanced, 0);
        RenderQueue queue;
        queue.SubmitOpaqueMesh(
            mesh,
            Transform{.position = {-0.8F, -0.2F, -3.0F},
                      .rotationDegrees = {15.0F, 30.0F, 10.0F},
                      .scale = {0.35F, 0.45F, 0.3F}},
            material);
        queue.SubmitOpaqueMesh(
            mesh,
            Transform{.position = {0.8F, 0.3F, -4.0F},
                      .rotationDegrees = {-10.0F, -20.0F, 15.0F},
                      .scale = {0.4F, 0.3F, 0.45F}},
            material);
        RenderTarget target;
        ASSERT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                  GL_FRAMEBUFFER_COMPLETE);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glClearColor(0, 0, 0, 1);
        shader->Use();
        const auto projection =
            glm::orthoRH_NO(-2.0F, 2.0F, -2.0F, 2.0F, 0.1F, 10.0F);
        glUniformMatrix4fv(shader->GetUniformLocation("gVC"), 1, GL_FALSE,
                           glm::value_ptr(projection));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (const auto& command : queue.GetOpaqueMeshes())
            mesh.Draw(command.transform, *material, bindings);
        const Image individual = target.Read();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        MeshInstanceRenderer renderer;
        renderer.Render(queue.GetOpaqueMeshes(), bindings);
        const Image instanced = target.Read();
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
        std::size_t coveredLeft = 0, coveredRight = 0;
        int largestDifference = 0;
        for (std::size_t pixel = 0; pixel < individual.size(); pixel += 4) {
            if (individual[pixel] + individual[pixel + 1] +
                    individual[pixel + 2] >
                0) {
                if ((pixel / 4) % kSize < kSize / 2)
                    ++coveredLeft;
                else
                    ++coveredRight;
            }
            for (std::size_t channel = 0; channel < 3; ++channel)
                largestDifference =
                    std::max(largestDifference,
                             std::abs(int(individual[pixel + channel]) -
                                      int(instanced[pixel + channel])));
        }
        EXPECT_GT(coveredLeft, 100U) << "Left instance must be visible.";
        EXPECT_GT(coveredRight, 100U) << "Right instance must be visible.";
        EXPECT_LE(largestDifference, 1);
        if (largestDifference > 1 || coveredLeft <= 100 ||
            coveredRight <= 100) {
            const auto name = std::filesystem::path(vertex).stem().string();
            SaveImage(name + "-individual", individual);
            SaveImage(name + "-instanced", instanced);
        }
        pipelineImages.push_back(instanced);
    }
    // Geometry consistency across the two production vertex shaders,
    // independent of the known differences between their lighting passes (#22,
    // #24, #26).
    int pipelineDifference = 0;
    for (std::size_t index = 0; index < pipelineImages[0].size(); ++index) {
        pipelineDifference = std::max(pipelineDifference,
                                      std::abs(int(pipelineImages[0][index]) -
                                               int(pipelineImages[1][index])));
    }
    EXPECT_LE(pipelineDifference, 1);
    if (pipelineDifference > 1) {
        SaveImage("forward-geometry", pipelineImages[0]);
        SaveImage("deferred-geometry", pipelineImages[1]);
    }
}

TEST_F(OpenGLRender, PassShaderPreservesPerInstanceSurfaceMaterials) {
    constexpr auto surfaceVertex = R"(#version 330
        in vec3 s_vPosition;
        void main() { gl_Position = vec4(s_vPosition, 1.0); })";
    constexpr auto surfaceFragment = R"(#version 330
        out vec4 color;
        void main() { color = vec4(1.0); })";
    auto surfaceShader = std::make_shared<Shader>();
    surfaceShader->AddShader(surfaceVertex, ShaderType::Vertex);
    surfaceShader->AddShader(surfaceFragment, ShaderType::Fragment);
    surfaceShader->Link();

    constexpr auto passFragment = R"(#version 330
        in vec3 fN;
        in vec3 Tangent;
        in vec2 UV;
        uniform float attributeWeight;
        uniform sampler2D colTexSampler;
        uniform sampler2D normTexSampler;
        uniform sampler2D specTexSampler;
        out vec4 color;
        void main() {
            vec3 surface = vec3(texture(colTexSampler, UV).r,
                                texture(normTexSampler, UV).g,
                                texture(specTexSampler, UV).b);
            color = vec4(surface + attributeWeight * (fN + Tangent), 1.0);
        })";
    auto passShader = std::make_shared<Shader>();
    passShader->AddShader(ReadFile(Asset("shaders/DSGeometryPass.vsh")),
                          ShaderType::Vertex);
    passShader->AddShader(passFragment, ShaderType::Fragment);
    passShader->Link();

    auto leftMaterial =
        std::make_shared<Material>(gsl_lite::not_null{surfaceShader});
    auto rightMaterial =
        std::make_shared<Material>(gsl_lite::not_null{surfaceShader});
    const auto leftTexture = SolidTexture({51, 76, 102, 255});
    const auto rightTexture = SolidTexture({204, 178, 153, 255});
    for (const auto& [material, texture] :
         {std::pair{leftMaterial, leftTexture},
          std::pair{rightMaterial, rightTexture}}) {
        material->SetColorTexture(texture);
        material->SetNormalTexture(texture);
        material->SetSpecularTexture(texture);
    }

    Mesh mesh(gsl_lite::not_null{leftMaterial},
              Asset("models/cube2x2x2.obj").string());
    RenderQueue queue;
    for (const float y : {-0.5F, 0.5F}) {
        queue.SubmitOpaqueMesh(
            mesh,
            Transform{.position = {-0.8F, y, -3.0F}, .scale = glm::vec3(0.35F)},
            leftMaterial);
        queue.SubmitOpaqueMesh(
            mesh,
            Transform{.position = {0.8F, y, -3.0F}, .scale = glm::vec3(0.35F)},
            rightMaterial);
    }

    RenderTarget target;
    ASSERT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
              GL_FRAMEBUFFER_COMPLETE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    passShader->Use();
    const glm::mat4 projection =
        glm::orthoRH_NO(-2.0F, 2.0F, -2.0F, 2.0F, 0.1F, 10.0F);
    glUniformMatrix4fv(passShader->GetUniformLocation("gVC"), 1, GL_FALSE,
                       glm::value_ptr(projection));
    glUniform1f(passShader->GetUniformLocation("attributeWeight"), 0.0F);
    MeshInstanceRenderer renderer;
    renderer.Render(queue.GetOpaqueMeshes(),
                    MeshShaderBindings::FromShader(*passShader),
                    passShader.get());

    const Image image = target.Read();
    const auto pixel = [&image](int x, int y, int channel) {
        return image.at(
            static_cast<std::size_t>((y * kSize + x) * 4 + channel));
    };
    for (const int y : {48, 80}) {
        EXPECT_NEAR(pixel(38, y, 0), 51, 1);
        EXPECT_NEAR(pixel(38, y, 1), 76, 1);
        EXPECT_NEAR(pixel(38, y, 2), 102, 1);
        EXPECT_NEAR(pixel(90, y, 0), 204, 1);
        EXPECT_NEAR(pixel(90, y, 1), 178, 1);
        EXPECT_NEAR(pixel(90, y, 2), 153, 1);
    }
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(OpenGLRender, GBufferCreationAndResizeAreComplete) {
    GBuffer buffer(kSize, kSize);
    EXPECT_EQ(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER),
              GL_FRAMEBUFFER_COMPLETE);
    glBindTexture(GL_TEXTURE_2D, buffer.GetDepthTexture());
    GLint internalFormat = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT,
                             &internalFormat);
    EXPECT_EQ(internalFormat, GL_DEPTH32F_STENCIL8);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
    ASSERT_TRUE(buffer.Resize(64, 64));
    EXPECT_EQ(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER),
              GL_FRAMEBUFFER_COMPLETE);
    glBindTexture(GL_TEXTURE_2D, buffer.GetDepthTexture());
    GLint width = 0;
    GLint height = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    EXPECT_EQ(width, 64);
    EXPECT_EQ(height, 64);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    const GLuint previousDepthTexture = buffer.GetDepthTexture();
    EXPECT_FALSE(buffer.Resize(0, 64));
    EXPECT_EQ(buffer.GetDepthTexture(), previousDepthTexture);
    buffer.BindForWriting();
    EXPECT_EQ(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER),
              GL_FRAMEBUFFER_COMPLETE);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    for (GLsizei size : {32, 96, 48}) {
        ASSERT_TRUE(buffer.Resize(size, size));
        EXPECT_EQ(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER),
                  GL_FRAMEBUFFER_COMPLETE);
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
    }
}
}  // namespace
