#ifndef PLACEABLEOBJECT_H
#define PLACEABLEOBJECT_H

#include <Object.h>
#include <math_3d.h>

class PlaceableObject : public Object
{
    public:
        PlaceableObject();
        virtual ~PlaceableObject();
        float position[3];

        virtual void SetPosition(float x, float y, float z);
        virtual void SetPosition(Vector3f _pos);
    protected:
    private:
};

#endif // PLACEABLEOBJECT_H
