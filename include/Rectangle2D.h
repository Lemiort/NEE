#ifndef INCLUDE_RECTANGLE2D_H_
#define INCLUDE_RECTANGLE2D_H_
#include <PlaceableObject.h>
#include <RenderableObject.h>

class Rectangle2D : public RenderableObject, public PlaceableObject {
public:
    Rectangle2D();
    virtual ~Rectangle2D();

protected:
private:
};

#endif  // INCLUDE_RECTANGLE2D_H_
