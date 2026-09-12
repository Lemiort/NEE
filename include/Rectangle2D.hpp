#ifndef RECTANGLE2D_H
#define RECTANGLE2D_H
#include "PlaceableObject.hpp"
#include "RenderableObject.hpp"

class Rectangle2D : public RenderableObject, public PlaceableObject {
public:
    Rectangle2D();
    ~Rectangle2D() override;

protected:
private:
};

#endif  // RECTANGLE2D_H
