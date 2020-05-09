#ifndef INCLUDE_FONT2D_H
#define INCLUDE_FONT2D_H
#include <map>
#include <memory>
#include <string>

#include "Mesh.h"
#include "Text_2D.h"

/**
 * @brief Font, that uses 3d object to rendering
 */
class Character2d : public MaterialObject, public PlaceableObject {
private:
    Mesh mesh;

    std::string fontName;

    std::string fileName;

    /** @brief scale of image */
    float kx, ky;

    /** @brief sapect ratio */
    float aspect_ratio;

    /** @brief size of pixel in uv coordinates */
    float pkx, pky;

    /** @brief size of font in pixels */
    int fontHeight;
    /** @brief size of font's texture */
    uint32_t imageWidth, imageHeight;

    /** @brief info about every font's character */
    std::map<unsigned int, FontCharacter> fontInfo;

    /** @brief Character width in UV space */
    float realWidth;

    /** @brief Character height in UV space */
    float realHeight;

    /**
     * @brief scaling coefficient
     *
     * @todo add detailed decr
     */
    float dx;

    /** @brief x offset in UV */
    float xOffset;

    /** @brief y offset in uv */
    float yOffset;

    FontCharacter temp;

protected:
    std::map<uint32_t, float> kerningInfo;
    /**
     * @brief character length in UV
     * @todo check if is true
     */
    glm::vec2 characterLength;

    uint32_t currentCharacter;

public:
    Character2d() = default;
    ~Character2d() override = default;

    float GetAspectRatio();

    /**
     * @brief Set the Aspect Ratio and recalculate scaling
     *
     * @param _width
     * @param _height
     */
    void SetAspectRatio(int _width, int _height);

    /**
     * @brief Get the Font Height
     *
     * @return int  height in pixels
     */
    int GetFontHeight();

    /**
     * @brief Get the Width
     *
     * @param c character
     * @return float width of character in screen-space
     */
    float GetWidth(unsigned int c);

    /**
     * @brief Get the Height
     *
     * @param c character
     * @return float height of character in scree-space
     */
    float GetHeight(unsigned int c);

    /**
     * @brief Get the Space Width
     *
     * @return float space width in screen-space
     */
    float GetSpaceWidth();

    /**
     * @brief Set the current Character
     *
     * @param c character
     */
    void SetCharacter(unsigned int c);

    /**
     * @brief Init with material and quad2x2front.ho3d model
     *
     * @param _mat
     * @param _fileName
     * @return true if succeed
     * @return false otherwise
     */
    bool Init(std::shared_ptr<Material> _mat, std::string _fileName);
    void Render(const Camera& cam) override;

    /**
     * @brief Get the Last Character size
     *
     * @return glm::vec2 size of characer in screen space
     */
    glm::vec2 GetLastCharacterLength();
};
#endif  // INCLUDE_FONT2D_H
