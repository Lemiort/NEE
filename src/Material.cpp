#include "Material.hpp"

#include <glad/gl.h>

#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "Shader.hpp"
#include "Texture.hpp"
#include "Util.hpp"

Material::Material() {
    // ctor

    colorMap = make_shared<Texture2D>();
    auto tempMap = colorMap;
    tempMap->Load("Textures/checker.tga");

    normalMap = make_shared<Texture2D>();
    normalMap->Load("Textures/normal_map.tga");

    specularMap = make_shared<Texture2D>();
    specularMap->Load("Textures/specular.tga");

    shadowMap = make_shared<Texture2D>();
    shadowMap->Load("Textures/white.png");

    // abstract texture
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);
    texturesID.resize(max_texture_units);
    abstractSamplersID.resize(max_texture_units);

    abstractMap = make_shared<AbstractTexture>();

    for (int i = 0; i < max_texture_units; i++) {
        abstractSamplersID[i] = 0;
        texturesID[i] = 0;
    }
    // std::cout<<"\n Max texture units is "<<max_texture_units;
}

Material::~Material() = default;

bool Material::Init(const shared_ptr<Shader>& _sh) {
  if (_sh == nullptr) {
    return false;
  }
  shaderProgram = _sh;
  shaderProgram->Use();

  // set default value

  // load into shader

  // activate texture unit 0
  colorMap->Bind(GL_TEXTURE0);
  std::string const colName("colTexSampler");
  colTexID = shaderProgram->GetUniformLocation(colName.c_str());
  std::cout << "\n color sampler num is " << colTexID;
  // tell shader to use as texture 0
  glUniform1i(colTexID, 0);

  normalMap->Bind(GL_TEXTURE1);
  normSamplerID = shaderProgram->GetUniformLocation("normTexSampler");
  // tell shader to use as texture #1
  glUniform1i(normSamplerID, 1);

  specularMap->Bind(GL_TEXTURE2);
  specSamplerID = shaderProgram->GetUniformLocation("specTexSampler");
  // tell shader to use texture unit 2 for specular map
  glUniform1i(specSamplerID, 2);

  shadowMap->Bind(GL_TEXTURE3);
  shadowSamplerID = shaderProgram->GetUniformLocation("shadowTexSampler");
  // tell shader to use texture unit 3 for shadow map
  glUniform1i(shadowSamplerID, 3);

  std::string const abstractSamplerName("gSampler");
  for (GLint i = 4; i < max_texture_units; i++) {
    abstractSamplersID[i] = shaderProgram->GetUniformLocation(
        (abstractSamplerName + ConvertToString(i)).c_str());
    // std::cout<<"\nSampler name is
    // "<<(abstractSamplerName+ConvertToString(i)).c_str();
  }

  return true;
}

void Material::Use() {
    shaderProgram->Use();
    colorMap->Bind(GL_TEXTURE0);
    glUniform1i(colTexID,
                0);  // tell shader to use texture unit 0

    normalMap->Bind(GL_TEXTURE1);
    // Assign texture to active texture unit
    glUniform1i(normSamplerID,
                1);  // tell shader to use texture unit 1

    specularMap->Bind(GL_TEXTURE2);
    // assign texture to active texture unit
    glUniform1i(specSamplerID,
                2);  // tell shader to use texture unit 2

    shadowMap->Bind(GL_TEXTURE3);
    // assign texture to active texture unit
    glUniform1i(shadowSamplerID,
                3);  // tell shader to use texture unit 3

    // starting from 4th, are custom texture units
    for (GLint i = 4; i < max_texture_units; i++) {
        if (texturesID[i] != 0) {
            abstractMap->SetTexture(texturesID[i]);
            abstractMap->Bind(GL_TEXTURE0 + i);
            // std::cout<<"\n\ntex set as "<<texturesID[i]<<" render at unit num
            // "<<i<<"\n sampler num is "<<abstractSamplersID[i]; std::cout<<"\n
            // color sampler num is "<<colTexID; std::cout<<"\n norm sampler num
            // is "<<normSamplerID;

            // assign texture to active texture unit
            glUniform1i(abstractSamplersID[i],
                        i);  // tell shader to use texture i
        }
    }
}

void Material::SetColorTexture(shared_ptr<Texture2D> _colorMap) {
  colorMap = std::move(_colorMap);
}

void Material::SetNormalTexture(shared_ptr<Texture2D> _normalMap) {
  normalMap = std::move(_normalMap);
}

void Material::SetSpecularTexture(shared_ptr<Texture2D> _specularMap) {
  specularMap = std::move(_specularMap);
}

void Material::SetShadowTexture(shared_ptr<Texture2D> _shadowMap) {
  shadowMap = std::move(_shadowMap);
}

void Material::SetTexture(const shared_ptr<Texture2D>& _map, GLuint num) {
  if (num <= 3) {
    return;
  }
  texturesID[num] = _map->GetTextureID();
}

void Material::SetTexture(GLuint _map, GLuint num) {
  if (num <= 3) {
    return;
  }
    texturesID[num] = _map;
}
