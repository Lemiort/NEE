#include "MaterialObject.hpp"

#include <memory>
#include <utility>

#include "Material.hpp"

MaterialObject::MaterialObject() = default;

MaterialObject::~MaterialObject() = default;

std::shared_ptr<Material> MaterialObject::GetMaterial() { return mat; }

void MaterialObject::SetMaterial(std::shared_ptr<Material> _mat) {
    mat = std::move(_mat);
    this->SetShader(mat->GetShader());
    // shaderProgram = mat->GetShader();
}
