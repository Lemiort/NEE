#ifndef MATERIALOBJECT_H
#define MATERIALOBJECT_H

#include <RenderableObject.h>
#include <Material.h>

class MaterialObject : public RenderableObject
{
    public:
        MaterialObject();
        virtual ~MaterialObject();
        virtual void SetMaterial(Material* _mat);
        virtual Material* GetMaterial();
    protected:
        Material* mat;
    private:
};

#endif // MATERIALOBJECT_H
