#include "Material.hpp"

#include <glad/gl.h>

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "Shader.hpp"
#include "Texture.hpp"
#include "Util.hpp"
#include "spdlog/spdlog.h"

namespace {
std::shared_ptr<Texture2D> LoadRequiredTexture(const std::string& path) {
    auto texture = std::make_shared<Texture2D>();
    if (!texture->Load(path)) {
        spdlog::error("Unable to load required material texture '{}'", path);
        throw std::runtime_error("Unable to load required material texture: " +
                                 path);
    }
    return texture;
}
}  // namespace

Material::Material(gsl_lite::not_null<std::shared_ptr<Shader>> shader)
    : shaderProgram(std::move(shader)) {
    colorMap = LoadRequiredTexture("textures/checker.tga");
    normalMap = LoadRequiredTexture("textures/normal_map.tga");
    specularMap = LoadRequiredTexture("textures/specular.tga");
    shadowMap = LoadRequiredTexture("textures/white.png");

    shaderProgram->Link();
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);
    texturesID.resize(max_texture_units);
    abstractSamplersID.resize(max_texture_units, -1);
    const GLuint program = shaderProgram->GetShaderProgramId();
    colTexID = shaderProgram->GetUniformLocation("colTexSampler");
    normSamplerID = shaderProgram->GetUniformLocation("normTexSampler");
    specSamplerID = shaderProgram->GetUniformLocation("specTexSampler");
    shadowSamplerID = shaderProgram->GetUniformLocation("shadowTexSampler");
    glProgramUniform1i(program, colTexID, 0);
    glProgramUniform1i(program, normSamplerID, 1);
    glProgramUniform1i(program, specSamplerID, 2);
    glProgramUniform1i(program, shadowSamplerID, 3);
    opacityID = shaderProgram->GetUniformLocation("opacity");
    for (GLint unit = 4; unit < max_texture_units; ++unit) {
        auto& location = abstractSamplersID.at(static_cast<std::size_t>(unit));
        location = shaderProgram->GetUniformLocation("gSampler" +
                                                     ConvertToString(unit));
        glProgramUniform1i(program, location, unit);
    }
}

Material::~Material() = default;

void Material::BindTextures() const {
    const std::array textures{
        colorMap->GetTextureID(), normalMap->GetTextureID(),
        specularMap->GetTextureID(), shadowMap->GetTextureID()};
    glBindTextures(0, static_cast<GLsizei>(textures.size()), textures.data());
    if (max_texture_units > 4)
        glBindTextures(4, max_texture_units - 4, texturesID.data() + 4);
}

void Material::Use(float drawOpacity) const {
    shaderProgram->Use();
    BindTextures();
    glProgramUniform1f(shaderProgram->GetShaderProgramId(), opacityID,
                       drawOpacity);
}

void Material::UseWithShader(Shader& shader, float drawOpacity) const {
    shader.Use();
    BindTextures();
    const GLuint program = shader.GetShaderProgramId();
    glProgramUniform1i(program, shader.GetUniformLocation("colTexSampler"), 0);
    glProgramUniform1i(program, shader.GetUniformLocation("normTexSampler"), 1);
    glProgramUniform1i(program, shader.GetUniformLocation("specTexSampler"), 2);
    glProgramUniform1i(program, shader.GetUniformLocation("shadowTexSampler"),
                       3);
    glProgramUniform1f(program, shader.GetUniformLocation("opacity"),
                       drawOpacity);
    for (GLint unit = 4; unit < max_texture_units; ++unit) {
        if (texturesID.at(static_cast<std::size_t>(unit)) == 0) continue;
        glProgramUniform1i(
            program,
            shader.GetUniformLocation("gSampler" + ConvertToString(unit)),
            unit);
    }
}

void Material::SetColorTexture(std::shared_ptr<Texture2D> _colorMap) {
    colorMap = std::move(_colorMap);
}

void Material::SetNormalTexture(std::shared_ptr<Texture2D> _normalMap) {
    normalMap = std::move(_normalMap);
}

void Material::SetSpecularTexture(std::shared_ptr<Texture2D> _specularMap) {
    specularMap = std::move(_specularMap);
}

void Material::SetShadowTexture(std::shared_ptr<Texture2D> _shadowMap) {
    shadowMap = std::move(_shadowMap);
}

void Material::SetTexture(const std::shared_ptr<Texture2D>& _map, GLuint num) {
    if (num <= 3 || num >= texturesID.size()) {
        spdlog::error("Texture unit {} is outside the available range", num);
        return;
    }
    texturesID.at(num) = _map->GetTextureID();
}

void Material::SetTexture(GLuint _map, GLuint num) {
    if (num <= 3 || num >= texturesID.size()) {
        spdlog::error("Texture unit {} is outside the available range", num);
        return;
    }
    texturesID.at(num) = _map;
}
