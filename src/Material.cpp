#include "Material.h"
#include <util.h>
#include <cstdlib>
#include <iostream>

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

    //абстрактная текстура
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);
    texturesID = new GLuint[max_texture_units];
    abstractSamplersID = new GLuint[max_texture_units];

    abstractMap = make_shared<AbstractTexture>();

    for (int i = 0; i < max_texture_units; i++) {
        abstractSamplersID[i] = 0;
        texturesID[i] = 0;
    }
    // std::cout<<"\n Max texture units is "<<max_texture_units;
}

Material::~Material() {
    // dtor
    delete[] texturesID;
    delete[] abstractSamplersID;
}

bool Material::Init(shared_ptr<Shader> _sh) {
    if (_sh == false) return false;
    shaderProgram = _sh;
    shaderProgram->Use();

    //установка значение по умолчанию

    //загружем это в шейдер

    // делаем активным текстурный юнит 0
    colorMap->Bind(GL_TEXTURE0);
    std::string colName("colTexSampler");
    colTexID = shaderProgram->GetUniformLocation(colName.c_str());
    std::cout << "\n color sampler num is " << colTexID;
    //говорим шейдеру, чтоб использовал в качестве текстуры №0
    glUniform1i(colTexID, 0);

    normalMap->Bind(GL_TEXTURE1);
    normSamplerID = shaderProgram->GetUniformLocation("normTexSampler");
    //говорим шейдеру, чтоб использовал в качестве текстуры №1
    glUniform1i(normSamplerID, 1);

    specularMap->Bind(GL_TEXTURE2);
    specSamplerID = shaderProgram->GetUniformLocation("specTexSampler");
    //говорим шейдеру, чтоб использовал в качестве текстуры №2
    glUniform1i(specSamplerID, 2);

    shadowMap->Bind(GL_TEXTURE3);
    shadowSamplerID = shaderProgram->GetUniformLocation("shadowTexSampler");
    //говорим шейдеру, чтоб использовал в качестве текстуры №3
    glUniform1i(shadowSamplerID, 3);

    std::string abstractSamplerName("gSampler");
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
                0);  //говорим шейдеру, чтобы использовал в качестве текстуры 0

    normalMap->Bind(GL_TEXTURE1);
    // назначаем текстуру на активный текстурный юнит
    glUniform1i(normSamplerID,
                1);  //говорим шейдеру, чтобы использовал в качестве текстуры 1

    specularMap->Bind(GL_TEXTURE2);
    // назначаем текстуру на активный текстурный юнит
    glUniform1i(specSamplerID,
                2);  //говорим шейдеру, чтобы использовал в качестве текстуры 2

    shadowMap->Bind(GL_TEXTURE3);
    // назначаем текстуру на активный текстурный юнит
    glUniform1i(shadowSamplerID,
                3);  //говорим шейдеру, чтобы использовал в качестве текстуры 3

    //начиная с 4го, лежат кастомные текстурные юниты
    for (GLint i = 4; i < max_texture_units; i++) {
        if (texturesID[i] != 0) {
            abstractMap->SetTexture(texturesID[i]);
            abstractMap->Bind(GL_TEXTURE0 + i);
            // std::cout<<"\n\ntex set as "<<texturesID[i]<<" render at unit num
            // "<<i<<"\n sampler num is "<<abstractSamplersID[i]; std::cout<<"\n
            // color sampler num is "<<colTexID; std::cout<<"\n norm sampler num
            // is "<<normSamplerID;

            // назначаем текстуру на активный текстурный юнит
            glUniform1i(
                abstractSamplersID[i],
                i);  //говорим шейдеру, чтобы использовал в качестве текстуры i
        }
    }
}

void Material::SetColorTexture(shared_ptr<Texture2D> _colorMap) {
    colorMap = _colorMap;
}

void Material::SetNormalTexture(shared_ptr<Texture2D> _normalMap) {
    normalMap = _normalMap;
}

void Material::SetSpecularTexture(shared_ptr<Texture2D> _specularMap) {
    specularMap = _specularMap;
}

void Material::SetShadowTexture(shared_ptr<Texture2D> _shadowMap) {
    shadowMap = _shadowMap;
}

void Material::SetTexture(shared_ptr<Texture2D> _map, GLuint num) {
    if (num <= 3) return;
    texturesID[num] = _map->GetTextureID();
}

void Material::SetTexture(GLuint _map, GLuint num) {
    if (num <= 3) return;
    texturesID[num] = _map;
}
