#ifndef MATERIALOBJECT_H
#define MATERIALOBJECT_H

#include <RenderableObject.h>
#include <Material.h>

//renderable object, using materials
class MaterialObject : public RenderableObject
{
    public:
        MaterialObject();
        virtual ~MaterialObject();
        virtual void SetMaterial(shared_ptr<Material> _mat);
        virtual shared_ptr<Material> GetMaterial();
    protected:
        shared_ptr<Material> mat;
    private:
};

#endif // MATERIALOBJECT_H
