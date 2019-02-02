#include "Material.h"
#include <util.h>
#include <cstdlib>
#include <iostream>

Material::Material() {
    // ctor

    colorMap = std::make_shared<Texture2D>();
    auto tempMap = colorMap;
    tempMap->Load("textures/checker.tga");

    normalMap = std::make_shared<Texture2D>();
    normalMap->Load("textures/normal_map.tga");

    specularMap = std::make_shared<Texture2D>();
    specularMap->Load("textures/specular.tga");

    shadowMap = std::make_shared<Texture2D>();
    shadowMap->Load("textures/white.png");

    //����������� ��������
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);
    texturesID = new GLuint[max_texture_units];
    abstractSamplersID = new GLuint[max_texture_units];

    abstractMap = std::make_shared<AbstractTexture>();

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

bool Material::Init(std::shared_ptr<Shader> _sh) {
    if (_sh == nullptr) return false;
    shaderProgram = _sh;
    shaderProgram->Use();

    //��������� �������� �� ���������

    //�������� ��� � ������

    // ������ �������� ���������� ���� 0
    colorMap->Bind(GL_TEXTURE0);
    std::string colName("colTexSampler");
    colTexID = shaderProgram->GetUniformLocation(colName.c_str());
    std::cout << "\n color sampler num is " << colTexID;
    //������� �������, ���� ����������� � �������� �������� �0
    glUniform1i(colTexID, 0);

    normalMap->Bind(GL_TEXTURE1);
    normSamplerID = shaderProgram->GetUniformLocation("normTexSampler");
    //������� �������, ���� ����������� � �������� �������� �1
    glUniform1i(normSamplerID, 1);

    specularMap->Bind(GL_TEXTURE2);
    specSamplerID = shaderProgram->GetUniformLocation("specTexSampler");
    //������� �������, ���� ����������� � �������� �������� �2
    glUniform1i(specSamplerID, 2);

    shadowMap->Bind(GL_TEXTURE3);
    shadowSamplerID = shaderProgram->GetUniformLocation("shadowTexSampler");
    //������� �������, ���� ����������� � �������� �������� �3
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
                0);  //������� �������, ����� ����������� � �������� �������� 0

    normalMap->Bind(GL_TEXTURE1);
    // ��������� �������� �� �������� ���������� ����
    glUniform1i(normSamplerID,
                1);  //������� �������, ����� ����������� � �������� �������� 1

    specularMap->Bind(GL_TEXTURE2);
    // ��������� �������� �� �������� ���������� ����
    glUniform1i(specSamplerID,
                2);  //������� �������, ����� ����������� � �������� �������� 2

    shadowMap->Bind(GL_TEXTURE3);
    // ��������� �������� �� �������� ���������� ����
    glUniform1i(shadowSamplerID,
                3);  //������� �������, ����� ����������� � �������� �������� 3

    //������� � 4��, ����� ��������� ���������� �����
    for (GLint i = 4; i < max_texture_units; i++) {
        if (texturesID[i] != 0) {
            abstractMap->SetTexture(texturesID[i]);
            abstractMap->Bind(GL_TEXTURE0 + i);
            // std::cout<<"\n\ntex set as "<<texturesID[i]<<" render at unit num
            // "<<i<<"\n sampler num is "<<abstractSamplersID[i]; std::cout<<"\n
            // color sampler num is "<<colTexID; std::cout<<"\n norm sampler num
            // is "<<normSamplerID;

            // ��������� �������� �� �������� ���������� ����
            glUniform1i(
                abstractSamplersID[i],
                i);  //������� �������, ����� ����������� � �������� �������� i
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

void Material::SetTexture(std::shared_ptr<Texture2D> _map, GLuint num) {
    if (num <= 3) return;
    texturesID[num] = _map->GetTextureID();
}

void Material::SetTexture(GLuint _map, GLuint num) {
    if (num <= 3) return;
    texturesID[num] = _map;
}
