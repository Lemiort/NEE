#ifndef RECTANGLE2D_H
#define RECTANGLE2D_H
#include <RenderableObject.h>
#include <PlaceableObject.h>

class Rectangle2D: public RenderableObject, public PlaceableObject
{
    public:
        Rectangle2D();
        virtual ~Rectangle2D();
    protected:
    private:


};

#endif // RECTANGLE2D_H
