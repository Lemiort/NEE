#ifndef MATERIALOBJECT_H
#define MATERIALOBJECT_H

#include <memory>

#include "Material.hpp"
#include "RenderableObject.hpp"

// renderable object, using materials
class MaterialObject : public RenderableObject {
public:
    MaterialObject();
    ~MaterialObject() override;
    MaterialObject(MaterialObject&&) noexcept = default;
    MaterialObject& operator=(MaterialObject&&) noexcept = default;
    virtual void SetMaterial(std::shared_ptr<Material> _mat);
    virtual std::shared_ptr<Material> GetMaterial();

protected:
    std::shared_ptr<Material> mat;

private:
};

#endif  // MATERIALOBJECT_H
