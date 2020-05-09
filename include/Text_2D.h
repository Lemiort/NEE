#ifndef INCLUDE_TEXT_2D_H
#define INCLUDE_TEXT_2D_H

#include <GL/glew.h>
#include <SOIL2/SOIL2.h>

#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <queue>
#include <string>

#include "PlaceableObject.h"
#include "RenderableObject.h"
#include "Shader.h"
#include "ShaderFunctions.h"

struct FontCharacter {
    //    X pos   Y pos   Width   Height   Xoffset  Yoffset  Orig W   Orig H
    unsigned int x_position, y_position, width, height, origW, origH;
    int xOffset, yOffset;
    FontCharacter(unsigned u1 = 0, unsigned u2 = 0, unsigned u3 = 0,
                  unsigned u4 = 0, int i1 = 0, int i2 = 0, unsigned u5 = 0,
                  unsigned u6 = 0) {
        x_position = u1;
        y_position = u2;
        width = u3;
        height = u4;
        origW = u5;
        origH = u6;
        xOffset = i1;
        yOffset = i2;
    }
};
struct KerningPairs {
    unsigned int code1;
    unsigned int code2;
    KerningPairs(unsigned k1, unsigned int k2) {
        code1 = k1;
        code2 = k2;
    }
};

class Font2d : public RenderableObject, public PlaceableObject {
private:
    GLuint vertexes_id, positions_id;
    GLuint uv_id, suvID;
    GLuint sizeID;
    GLuint texSamplerID, texBufferID;
    GLuint colorID;
    glm::vec4 color;
    std::string filename;
    unsigned int* indicies;
    float aspect_ratio;

    /**
     * @brief image to screen scaling
     */
    float kx, ky;

    /**
     * @brief pixel size in uv
     */
    float pkx, pky;
    int fontHeight;
    uint32_t imageWidth, imageHeight;
    std::string fontName;
    std::map<unsigned int, FontCharacter> fontInfo;

    float realWidth;
    float realHeight;
    float dx;
    float xOffset;
    float yOffset;
    FontCharacter temp;

protected:
    glm::vec2 characterLength;
    unsigned int character;
    std::map<uint32_t, float> kerningInfo;

public:
    Font2d();
    ~Font2d();
    bool Init(std::string filename, std::shared_ptr<Shader> _sh);
    int GetFontHeight();
    float GetWidth(unsigned int c);
    float GetHeight(unsigned int c);
    float GetSpaceWidth();
    void SetAspectRatio(int width, int height);
    void SetAspectRatio(float);
    void SetCharacter(unsigned int c);

    glm::vec2 GetLastCharacterLength();
    void Render(const Camera& cam) override;
    friend class FontLine2d;
};

/**
 * @brief Line of the font
 *
 */
class FontLine2d : public RenderableObject, public PlaceableObject {
private:
    Font2d character;
    std::shared_ptr<Shader> shaderProgram;
    float prevX, prevY;
    float aspect_ratio;
    float spaceWidth;
    unsigned int prevChar;
    std::string text;

public:
    FontLine2d();
    ~FontLine2d();
    bool Init(std::string filename, std::shared_ptr<Shader> _sh);
    void SetAspectRatio(int w, int h);
    void SetText(std::string _text);
    // void Render(string text,float x, float y, float size);
    void Render(const Camera& cam) override;
};

#endif  // INCLUDE_TEXT_2D_H
