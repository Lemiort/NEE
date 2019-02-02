#ifndef SCALEABLEOBJECT_H
#define SCALEABLEOBJECT_H
#include <math_3d.h>

class ScaleableObject
{
    public:
        ScaleableObject();
        virtual ~ScaleableObject();
        void SetScale(float x, float y, float z);
        void SetScale(Vector3f v);
        Vector3f GetScale();
    protected:
        float scale[3];
    private:
};

#endif // SCALEABLEOBJECT_H
