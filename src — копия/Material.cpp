#include "Material.h"
#include <cstdlib>
#include <util.h>
#include <iostream>

Material::Material()
{
    //ctor
    ownColorMap = true;
    ownNormapMap = true;
    ownSpecularMap = true;
    ownShadowMap = true;

    Texture2D* tempMap;

    colorMap = new Texture2D;
    tempMap=colorMap;
    tempMap->Load( "Textures/checker.tga" );

    normalMap = new Texture2D;
    normalMap->Load( "Textures/normal_map.tga" );

    specularMap = new Texture2D;
    specularMap->Load( "Textures/specular.tga" );

    shadowMap = new Texture2D;
    shadowMap->Load("Textures/white.png");

    //абстрактная текстура
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,&max_texture_units);
    texturesID =  new GLuint[max_texture_units];
    abstractSamplersID = new GLuint[max_texture_units];

    abstractMap = new AbstractTexture;

    for(int i = 0; i < max_texture_units; i++)
    {
        abstractSamplersID[i] = 0;
        texturesID[i] = 0;
    }
    //std::cout<<"\n Max texture units is "<<max_texture_units;
}

Material::~Material()
{
    //dtor
    if( ownColorMap )
        delete colorMap;
    if( ownNormapMap )
        delete normalMap;
    if( ownSpecularMap )
        delete specularMap;
    if( ownShadowMap   )
        delete shadowMap;

    delete abstractMap;
    delete[] texturesID;
    delete[] abstractSamplersID;
}

bool Material::Init(Shader* _sh)
{
    if(_sh==0)
        return false;
    shaderProgram=_sh;
    shaderProgram->Use();

    //установка значение по умолчанию

    //загружем это в шейдер

    // делаем активным текстурный юнит 0
    colorMap->Bind(GL_TEXTURE0);
    std::string colName("colTexSampler");
    colTexID =	shaderProgram->GetUniformLocation(colName.c_str());
    std::cout<<"\n color sampler num is "<<colTexID;
    //говорим шейдеру, чтоб использовал в качестве текстуры №0
    glUniform1i(colTexID, 0);

    normalMap->Bind(GL_TEXTURE1);
    normSamplerID =	shaderProgram->GetUniformLocation("normTexSampler");
    //говорим шейдеру, чтоб использовал в качестве текстуры №1
    glUniform1i(normSamplerID, 1);

    specularMap->Bind(GL_TEXTURE2);
    specSamplerID =	shaderProgram->GetUniformLocation("specTexSampler");
    //говорим шейдеру, чтоб использовал в качестве текстуры №2
    glUniform1i(specSamplerID, 2);

    shadowMap->Bind(GL_TEXTURE3);
    shadowSamplerID =	shaderProgram->GetUniformLocation("shadowTexSampler");
    //говорим шейдеру, чтоб использовал в качестве текстуры №3
    glUniform1i(shadowSamplerID, 3);


    std::string abstractSamplerName("gSampler");
    for(GLint i = 4; i < max_texture_units; i++)
    {
            abstractSamplersID[i] =	shaderProgram->GetUniformLocation((abstractSamplerName+ConvertToString(i)).c_str());
            //std::cout<<"\nSampler name is "<<(abstractSamplerName+ConvertToString(i)).c_str();
    }


    return true;
}

void Material::Use()
{
    shaderProgram->Use();
    colorMap->Bind(GL_TEXTURE0);
    glUniform1i(colTexID,0);//говорим шейдеру, чтобы использовал в качестве текстуры 0

    normalMap->Bind(GL_TEXTURE1);
    // назначаем текстуру на активный текстурный юнит
    glUniform1i(normSamplerID, 1);//говорим шейдеру, чтобы использовал в качестве текстуры 1

    specularMap->Bind(GL_TEXTURE2);
    // назначаем текстуру на активный текстурный юнит
    glUniform1i(specSamplerID, 2);//говорим шейдеру, чтобы использовал в качестве текстуры 2

    shadowMap->Bind(GL_TEXTURE3);
    // назначаем текстуру на активный текстурный юнит
    glUniform1i(shadowSamplerID, 3);//говорим шейдеру, чтобы использовал в качестве текстуры 3

    //начиная с 4го, лежат кастомные текстурные юниты
    for(GLint i = 4; i < max_texture_units; i++)
    {
        if(texturesID[i] != 0)
        {
            abstractMap->SetTexture(texturesID[i]);
            abstractMap->Bind(GL_TEXTURE0+i);
            //std::cout<<"\n\ntex set as "<<texturesID[i]<<" render at unit num "<<i<<"\n sampler num is "<<abstractSamplersID[i];
            //std::cout<<"\n color sampler num is "<<colTexID;
            //std::cout<<"\n norm sampler num is "<<normSamplerID;



            // назначаем текстуру на активный текстурный юнит
            glUniform1i(abstractSamplersID[i], i);//говорим шейдеру, чтобы использовал в качестве текстуры i
        }
    }
}

void Material::SetColorTexture( Texture2D* _colorMap )
{
    if(_colorMap != NULL)
    {
        if(ownColorMap)
            delete colorMap;
        colorMap = _colorMap;
        ownColorMap = false;
    }
}


void Material::SetNormalTexture( Texture2D* _normalMap )
{
    if(_normalMap != NULL)
    {
        if(ownNormapMap)
            delete normalMap;
        normalMap = _normalMap;
        ownNormapMap = false;
    }
}

void Material::SetSpecularTexture( Texture2D* _specularMap )
{
    if(_specularMap != NULL )
    {
        if(ownSpecularMap)
            delete specularMap;
        specularMap = _specularMap;
        ownSpecularMap = false;
    }
}

void Material::SetShadowTexture( Texture2D* _shadowMap )
{
    if(_shadowMap != NULL )
    {
        if(ownShadowMap)
            delete shadowMap;
        shadowMap = _shadowMap;
        ownShadowMap = false;
    }
}

void Material::SetTexture(Texture2D* _map, GLuint num)
{
    if(num <=3)
        return;
    texturesID[num]=_map->GetTextureID();
}

void  Material::SetTexture(GLuint _map, GLuint num)
{
    if(num <= 3)
        return;
    texturesID[num]=_map;
}
