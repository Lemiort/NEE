#ifndef PLACEABLEOBJECT_H
#define PLACEABLEOBJECT_H

#include <Object.h>


class PlaceableObject : public Object
{
    public:
        PlaceableObject();
        virtual ~PlaceableObject();
        float position[3];
    protected:
    private:
};

#endif // PLACEABLEOBJECT_H
