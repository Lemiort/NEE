#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED
#include <glad/gl.h>

#include <array>
#include <string>

/**
 * Load an image file using stb_image and create an OpenGL 2D texture.
 * @param path Path to the image file.
 * @return GLuint texture ID or 0 on failure.
 */
GLuint loadTexture(const std::string& path);
/**
 * Load a cubemap from six image files.
 * The order of the paths must match the enum values in `types[]`:
 *  POSITIVE_X, NEGATIVE_X, POSITIVE_Y, NEGATIVE_Y, POSITIVE_Z, NEGATIVE_Z
 */
GLuint loadCubeMap(const std::array<std::string, 6>& faces);

class Texture {
protected:
    GLuint texBufferID{};
    bool del;
    static std::string ConvertInternalFormatToString(GLenum format);

public:
    [[nodiscard]] GLuint GetTextureID() const;
    explicit Texture(bool _del = true);
    virtual ~Texture();
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;
    // virtual ~Texture(bool del=true);
    std::string GetParameters() const;
    virtual void Bind(GLenum TextureUnit);
};

class Texture2D : public Texture {
protected:
public:
    // abstract texture constructor
    explicit Texture2D(GLuint texBufferID, bool _del = true);

    // default constructor, deletes the texture after use
    Texture2D() = default;

    ~Texture2D() override;
    Texture2D(Texture2D&&) noexcept = default;
    Texture2D& operator=(Texture2D&&) noexcept = default;
    virtual bool Load(const std::string& path);
};

class AbstractTexture : public Texture {
public:
    AbstractTexture() { texBufferID = 0; }
    ~AbstractTexture() override = default;

    void SetTexture(GLuint _texID);

protected:
};

#endif  // TEXTURE_H_INCLUDED
