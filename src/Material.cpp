#include "Material.h"

Material::Material() {
    // ctor

    colorMap = std::make_shared<Texture2D>();
    auto tempMap = colorMap;
    tempMap->Load("Textures/checker.tga");

    normalMap = std::make_shared<Texture2D>();
    normalMap->Load("Textures/normal_map.tga");

    specularMap = std::make_shared<Texture2D>();
    specularMap->Load("Textures/specular.tga");

    shadowMap = std::make_shared<Texture2D>();
    shadowMap->Load("Textures/white.png");

    abstractMap = std::make_shared<AbstractTexture>();
}

bool Material::Init(std::shared_ptr<Shader> _sh) {
    if (_sh == nullptr) return false;
    shaderProgram = _sh;
    shaderProgram->Use();

    colorMap->Bind(GL_TEXTURE0);
    std::string colName("colTexSampler");
    colTexID = shaderProgram->GetUniformLocation(colName.c_str());
    glUniform1i(colTexID, 0);

    normalMap->Bind(GL_TEXTURE1);
    normSamplerID = shaderProgram->GetUniformLocation("normTexSampler");
    glUniform1i(normSamplerID, 1);

    specularMap->Bind(GL_TEXTURE2);
    specSamplerID = shaderProgram->GetUniformLocation("specTexSampler");
    glUniform1i(specSamplerID, 2);

    shadowMap->Bind(GL_TEXTURE3);
    shadowSamplerID = shaderProgram->GetUniformLocation("shadowTexSampler");
    glUniform1i(shadowSamplerID, 3);
    return true;
}

void Material::Use() {
    shaderProgram->Use();
    colorMap->Bind(GL_TEXTURE0);
    glUniform1i(colTexID, 0);

    normalMap->Bind(GL_TEXTURE1);
    glUniform1i(normSamplerID, 1);

    specularMap->Bind(GL_TEXTURE2);
    glUniform1i(specSamplerID, 2);

    shadowMap->Bind(GL_TEXTURE3);
    glUniform1i(shadowSamplerID, 3);

    for (const auto sampler : abstractSamplersID) {
        const auto& num = sampler.first;
        const auto& uniform_text = sampler.second;
        if (texturesID[num] != 0) {
            abstractMap->SetTexture(texturesID[num]);
            abstractMap->Bind(GL_TEXTURE0 + num);

            glUniform1i(uniform_text, num);
        }
    }
}

void Material::SetColorTexture(std::shared_ptr<Texture2D> _colorMap) {
    colorMap = _colorMap;
}

void Material::SetNormalTexture(std::shared_ptr<Texture2D> _normalMap) {
    normalMap = _normalMap;
}

void Material::SetSpecularTexture(std::shared_ptr<Texture2D> _specularMap) {
    specularMap = _specularMap;
}

void Material::SetShadowTexture(std::shared_ptr<Texture2D> _shadowMap) {
    shadowMap = _shadowMap;
}

void Material::SetTexture(std::shared_ptr<Texture2D> map, TextureNumber num) {
    if (num <= 3) return;

    texturesID[num] = map->GetTextureID();

    shaderProgram->Use();
    std::string abstractSamplerName("gSampler");
    abstractSamplersID[num] = shaderProgram->GetUniformLocation(
        (abstractSamplerName + std::to_string(num)).c_str());
}

void Material::SetTexture(GLuint map, GLuint num) {
    if (num <= 3) return;

    texturesID[num] = map;

    shaderProgram->Use();
    std::string abstractSamplerName("gSampler");
    abstractSamplersID[num] = shaderProgram->GetUniformLocation(
        (abstractSamplerName + std::to_string(num)).c_str());
}
