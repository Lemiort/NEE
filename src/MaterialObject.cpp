#include "MaterialObject.h"

MaterialObject::MaterialObject()
{
    //ctor
}

MaterialObject::~MaterialObject()
{
    //dtor
}


Material* MaterialObject::GetMaterial()
{
    return mat;
}

void MaterialObject::SetMaterial(Material* _mat)
{
    mat = _mat;
    this->SetShader(mat->GetShader());
    //shaderProgram = mat->GetShader();
}
