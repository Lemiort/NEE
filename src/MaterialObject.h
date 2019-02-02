#ifndef MATERIALOBJECT_H
#define MATERIALOBJECT_H

#include <Material.h>
#include <RenderableObject.h>

// renderable object, using materials
class MaterialObject : public RenderableObject {
public:
    MaterialObject();
    virtual ~MaterialObject();
    virtual void SetMaterial(std::shared_ptr<Material> _mat);
    virtual std::shared_ptr<Material> GetMaterial();

protected:
    std::shared_ptr<Material> mat;

private:
};

#endif  // MATERIALOBJECT_H
