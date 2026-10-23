#ifndef RECTANGLE2D_H
#define RECTANGLE2D_H
#include "RenderableObject.hpp"
#include "Transformable.hpp"

class Rectangle2D : public RenderableObject, public Transformable {
public:
    Rectangle2D();
    ~Rectangle2D() override;

protected:
private:
};

#endif  // RECTANGLE2D_H
