#ifndef FONT2D_H
#define FONT2D_H
#include <map>

#include "Mesh.hpp"
#include "Text2D.hpp"

using namespace std;

class Character2d : public MaterialObject, public PlaceableObject {
private:
    Mesh mesh;

    // font name
    string fontName;
    // file reference
    string fileName;

    // image size ratio to screen
    float kx, ky;

    // screen aspect ratio
    float aratio;

    // pixel size in UV
    float pkx, pky;
    // font size in pixels
    int fontHeight;
    // font texture dimensions
    uint32_t imageWidth, imageHeight;
    // font information, more accurately about all letters
    map<unsigned int, FontCharacter> fontInfo;

    // width in UV coordinates
    float realWidth;
    // height in UV coordinates
    float realHeight;

    // some scaling factor
    //  TODO sign more detailed
    float dx;
    // x offset on the texture in UV
    float xOffset;
    // y offset on the texture in UV
    float yOffset;

    FontCharacter temp;

protected:
    // kerning information
    map<uint32_t, float> kerningInfo;

    // TODO length of character in texture coordinates??
    Vector2f characterLength;

    // current character
    unsigned int currentCharacter;

public:
    Character2d();

    float GetAspectRatio();
    // setting the ratio + adjusting coefficients
    // texture scaling
    void SetAspectRatio(int _width, int _height);

    // returns font height (in pixels)
    int GetFontHeight();

    // returns width in screen coordinates
    // of current character
    float GetWidth(unsigned int c);

    // returns height in screen coordinates
    // of current character
    float GetHeight(unsigned int c);

    // returns space width in screen coordinates
    float GetSpaceWidth();

    // sets the current character
    void SetCharacter(unsigned int c);

    // initialize with material and model quad2x2front.ho3d
    // and font
    bool Init(shared_ptr<Material> _mat, string _fileName);
    ~Character2d();
    void Render(const Camera& cam);

    // TODO returns length of current character in screen coordinates??
    Vector2f GetLastCharacterLength();
};
#endif  // FONT2D_H
