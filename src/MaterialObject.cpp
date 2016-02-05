#include "MaterialObject.h"

MaterialObject::MaterialObject()
{
    //ctor
}

MaterialObject::~MaterialObject()
{
    //dtor
}


shared_ptr<Material> MaterialObject::GetMaterial()
{
    return mat;
}

void MaterialObject::SetMaterial(shared_ptr<Material> _mat)
{
    mat = _mat;
    this->SetShader(mat->GetShader());
    //shaderProgram = mat->GetShader();
}
