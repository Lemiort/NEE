#include <glad/gl.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "Material.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

// Isolate texture ownership from shader compilation and a window-system
// context. Material and Texture use their production implementations and mocked
// GL calls.
Shader::Shader() = default;
Shader::~Shader() = default;
void Shader::Use() {}
GLint Shader::GetUniformLocation(const std::string&) const { return 0; }

namespace {
std::vector<GLuint> deletedTextures;
std::vector<std::pair<GLenum, GLuint>> textureBindings;
GLenum activeUnit = GL_TEXTURE0;
GLuint nextTexture = 1;

void GLAD_API_PTR DeleteTextures(GLsizei count, const GLuint* textures) {
    for (GLsizei i = 0; i < count; ++i) {
        if (textures[i] != 0) deletedTextures.push_back(textures[i]);
    }
}

void GLAD_API_PTR BindTexture(GLenum, GLuint texture) {
    textureBindings.emplace_back(activeUnit, texture);
}

class MaterialTextureOwnership : public testing::Test {
protected:
    void SetUp() override {
        deletedTextures.clear();
        textureBindings.clear();
        nextTexture = 1;
        activeUnit = GL_TEXTURE0;
        glad_glDeleteTextures = DeleteTextures;
        glad_glBindTexture = BindTexture;
        glad_glActiveTexture = [](GLenum unit) { activeUnit = unit; };
        glad_glGetIntegerv = [](GLenum parameter, GLint* value) {
            *value = parameter == GL_UNPACK_ALIGNMENT ? 4 : 16;
        };
        glad_glPixelStorei = [](GLenum, GLint) {};
        glad_glGetUniformLocation = [](GLuint, const GLchar*) -> GLint {
            return 0;
        };
        glad_glUniform1i = [](GLint, GLint) {};
        glad_glUniform1f = [](GLint, GLfloat) {};
        glad_glGenTextures = [](GLsizei count, GLuint* textures) {
            for (GLsizei i = 0; i < count; ++i) textures[i] = nextTexture++;
        };
        glad_glTexImage2D = [](GLenum, GLint, GLint, GLsizei, GLsizei, GLint,
                               GLenum, GLenum, const void*) {};
        glad_glGenerateMipmap = [](GLenum) {};
        glad_glTexParameteri = [](GLenum, GLenum, GLint) {};
    }

    static bool WasDeleted(GLuint texture) {
        return std::ranges::find(deletedTextures, texture) !=
               deletedTextures.end();
    }
};

TEST_F(MaterialTextureOwnership, DestructionPreservesBorrowedAttachments) {
    constexpr GLuint positionTexture = 10001;
    constexpr GLuint normalTexture = 10002;
    {
        Material material(gsl_lite::not_null{std::make_shared<Shader>()});
        material.SetTexture(positionTexture, 4);
        material.SetTexture(normalTexture, 5);
        material.Use();
        EXPECT_NE(std::ranges::find(
                      textureBindings,
                      std::pair<GLenum, GLuint>{GL_TEXTURE4, positionTexture}),
                  textureBindings.end());
        EXPECT_NE(std::ranges::find(
                      textureBindings,
                      std::pair<GLenum, GLuint>{GL_TEXTURE5, normalTexture}),
                  textureBindings.end());
    }
    EXPECT_FALSE(WasDeleted(positionTexture));
    EXPECT_FALSE(WasDeleted(normalTexture));
}

TEST_F(MaterialTextureOwnership, RebindingPreservesOldAndNewAttachments) {
    constexpr GLuint oldTexture = 10001;
    constexpr GLuint newTexture = 10002;
    {
        Material material(gsl_lite::not_null{std::make_shared<Shader>()});
        material.SetTexture(oldTexture, 4);
        material.Use();
        material.SetTexture(newTexture, 4);
        material.Use();
    }
    EXPECT_FALSE(WasDeleted(oldTexture));
    EXPECT_FALSE(WasDeleted(newTexture));
}

TEST_F(MaterialTextureOwnership, BorrowedSharedTextureRemainsOwnedByCaller) {
    constexpr GLuint textureId = 10001;
    auto texture = std::make_shared<Texture2D>(textureId);
    {
        Material material(gsl_lite::not_null{std::make_shared<Shader>()});
        material.SetTexture(texture, 4);
        material.Use();
    }
    EXPECT_FALSE(WasDeleted(textureId));
    texture.reset();
    EXPECT_EQ(std::ranges::count(deletedTextures, textureId), 1);
}
}  // namespace
