#include "MaterialObject.hpp"

#include <memory>
#include <utility>

#include "Material.hpp"

MaterialObject::MaterialObject() {
    // ctor
}

MaterialObject::~MaterialObject() {
    // dtor
}

shared_ptr<Material> MaterialObject::GetMaterial() { return mat; }

void MaterialObject::SetMaterial(shared_ptr<Material> _mat) {
  mat = std::move(_mat);
  this->SetShader(mat->GetShader());
  // shaderProgram = mat->GetShader();
}
