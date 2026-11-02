#include "Texture.hpp"

#include <glad/gl.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <ios>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>

#include "spdlog/spdlog.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace {
class UnpackAlignmentGuard {
public:
    UnpackAlignmentGuard() {
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment_);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }
    ~UnpackAlignmentGuard() {
        glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment_);
    }

private:
    GLint previousAlignment_{};
};

GLenum ImageFormat(int channels) {
    switch (channels) {
        case 1:
            return GL_RED;
        case 2:
            return GL_RG;
        case 3:
            return GL_RGB;
        case 4:
            return GL_RGBA;
        default:
            return 0;
    }
}
GLenum ImageStorageFormat(int channels) {
    constexpr std::array<GLenum, 4> formats{GL_R8, GL_RG8, GL_RGB8, GL_RGBA8};
    return formats.at(static_cast<std::size_t>(channels - 1));
}
}  // namespace

/**
 * Load an image file using stb_image and create an OpenGL 2D texture.
 * The function generates mipmaps, flips the image vertically (OpenGL
 * expects bottom-left origin). Storage uses sized, uncompressed formats.
 *
 * @param path Path to the image file.
 * @return GLuint texture ID or 0 on failure.
 */
GLuint loadTexture(const std::string& path) {
    int width{};
    int height{};
    int channels{};
    // stbi_set_flip_vertically_on_load flips vertically if requested; we
    // want Y inverted
    stbi_set_flip_vertically_on_load(1);
    auto* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (data == nullptr) {
        spdlog::error("Failed to load texture {}: {}", path.c_str(),
                      stbi_failure_reason());

        return 0;
    }

    const GLenum format = ImageFormat(channels);
    if (format == 0) {
        spdlog::error("Unsupported channel count {} in texture {}", channels,
                      path);
        stbi_image_free(data);
        return 0;
    }

    const UnpackAlignmentGuard unpackAlignment;
    GLuint texID = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &texID);
    const auto levels = static_cast<GLsizei>(
        std::bit_width(static_cast<unsigned int>(std::max(width, height))));
    glTextureStorage2D(texID, levels, ImageStorageFormat(channels), width,
                       height);
    glTextureSubImage2D(texID, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE,
                        data);
    glGenerateTextureMipmap(texID);
    glTextureParameteri(texID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(texID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return texID;
}

// ---------------------------------------------------------------------
// Cubemap loader – uses the same stb_image routine as 2‑D textures.
// The function creates a new texture object, uploads six faces and
// returns its ID.  It also sets common parameters (linear filtering,
// clamp‑to‑edge).
GLuint loadCubeMap(const std::array<std::string, 6>& faces) {
    const UnpackAlignmentGuard unpackAlignment;
    GLuint tex = 0;
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &tex);
    int faceSize = 0;

    for (size_t i = 0; i < faces.size(); ++i) {
        int w{};
        int h{};
        int ch{};
        auto* data = stbi_load(faces.at(i).c_str(), &w, &h, &ch, 0);
        if (data == nullptr) {
            glDeleteTextures(1, &tex);
            return 0;  // failure
        }

        const GLenum format = ImageFormat(ch);
        if (format == 0) {
            stbi_image_free(data);
            glDeleteTextures(1, &tex);
            return 0;
        }
        if (w != h || (i != 0 && w != faceSize)) {
            spdlog::error("Cubemap faces must be square and equally sized: {}",
                          faces.at(i));
            stbi_image_free(data);
            glDeleteTextures(1, &tex);
            return 0;
        }
        if (i == 0) {
            faceSize = w;
            glTextureStorage2D(tex, 1, GL_RGBA8, w, h);
        }
        // DSA addresses cubemap faces as layers of a 3D subimage upload.
        glTextureSubImage3D(tex, 0, 0, 0, static_cast<GLint>(i), w, h, 1,
                            format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    // common parameters
    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return tex;
}

Texture::Texture(bool _del) : del(_del) {}

Texture::Texture(Texture&& other) noexcept
    : texBufferID(std::exchange(other.texBufferID, 0)), del(other.del) {
    other.del = false;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        if (del) {
            glDeleteTextures(1, &texBufferID);
        }
        texBufferID = std::exchange(other.texBufferID, 0);
        del = other.del;
        other.del = false;
    }
    return *this;
}

Texture::~Texture() {
    if (del) {
        glDeleteTextures(1, &texBufferID);
        texBufferID = 0;
    }
}

GLuint Texture::GetTextureID() const { return texBufferID; }

///////////////////////////////////////////////////////////////////////////////
// convert OpenGL internal format enum to string
///////////////////////////////////////////////////////////////////////////////
std::string Texture::ConvertInternalFormatToString(GLenum format) {
    std::string formatName;

    switch (format) {
        case GL_STENCIL_INDEX:  // 0x1901
            formatName = "GL_STENCIL_INDEX";
            break;
        case GL_DEPTH_COMPONENT:  // 0x1902
            formatName = "GL_DEPTH_COMPONENT";
            break;
        case GL_ALPHA:  // 0x1906
            formatName = "GL_ALPHA";
            break;
        case GL_R8:
            return "GL_R8";
        case GL_RG8:
            return "GL_RG8";
        case GL_DEPTH32F_STENCIL8:
            return "GL_DEPTH32F_STENCIL8";
        case GL_RGB:  // 0x1907
            formatName = "GL_RGB";
            break;
        case GL_RGBA:  // 0x1908
            formatName = "GL_RGBA";
            break;
        case GL_R3_G3_B2:  // 0x2A10
            formatName = "GL_R3_G3_B2";
            break;
        case GL_RGB4:  // 0x804F
            formatName = "GL_RGB4";
            break;
        case GL_RGB5:  // 0x8050
            formatName = "GL_RGB5";
            break;
        case GL_RGB8:  // 0x8051
            formatName = "GL_RGB8";
            break;
        case GL_RGB10:  // 0x8052
            formatName = "GL_RGB10";
            break;
        case GL_RGB12:  // 0x8053
            formatName = "GL_RGB12";
            break;
        case GL_RGB16:  // 0x8054
            formatName = "GL_RGB16";
            break;
        case GL_RGBA2:  // 0x8055
            formatName = "GL_RGBA2";
            break;
        case GL_RGBA4:  // 0x8056
            formatName = "GL_RGBA4";
            break;
        case GL_RGB5_A1:  // 0x8057
            formatName = "GL_RGB5_A1";
            break;
        case GL_RGBA8:  // 0x8058
            formatName = "GL_RGBA8";
            break;
        case GL_RGB10_A2:  // 0x8059
            formatName = "GL_RGB10_A2";
            break;
        case GL_RGBA12:  // 0x805A
            formatName = "GL_RGBA12";
            break;
        case GL_RGBA16:  // 0x805B
            formatName = "GL_RGBA16";
            break;
        case GL_DEPTH_COMPONENT16:  // 0x81A5
            formatName = "GL_DEPTH_COMPONENT16";
            break;
        case GL_DEPTH_COMPONENT24:  // 0x81A6
            formatName = "GL_DEPTH_COMPONENT24";
            break;
        case GL_DEPTH_COMPONENT32:  // 0x81A7
            formatName = "GL_DEPTH_COMPONENT32";
            break;
        case GL_DEPTH_STENCIL:  // 0x84F9
            formatName = "GL_DEPTH_STENCIL";
            break;
        case GL_RGBA32F:  // 0x8814
            formatName = "GL_RGBA32F";
            break;
        case GL_RGB32F:  // 0x8815
            formatName = "GL_RGB32F";
            break;
        case GL_RGBA16F:  // 0x881A
            formatName = "GL_RGBA16F";
            break;
        case GL_RGB16F:  // 0x881B
            formatName = "GL_RGB16F";
            break;
        case GL_DEPTH24_STENCIL8:  // 0x88F0
            formatName = "GL_DEPTH24_STENCIL8";
            break;
        default:
            std::stringstream ss;
            ss << "Unknown Format(0x" << std::hex << format << ")" << std::ends;
            formatName = ss.str();
    }

    return formatName;
}

///////////////////////////////////////////////////////////////////////////////
// Query the texture directly without changing texture-unit bindings.
///////////////////////////////////////////////////////////////////////////////
std::string Texture::GetParameters() const {
    GLuint const id = texBufferID;
    if (glIsTexture(id) == GL_FALSE) {
        return "Not texture object";
    }

    int width = 0;
    int height = 0;
    int format = 0;
    std::string formatName;
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_WIDTH,
                                 &width);  // get texture width
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_HEIGHT,
                                 &height);  // get texture height
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_INTERNAL_FORMAT,
                                 &format);  // get texture internal format

    formatName = ConvertInternalFormatToString(format);

    std::stringstream ss;
    ss << width << "x" << height << ", " << formatName;
    return ss.str();
}

void Texture::Bind(GLenum TextureUnit) const {
    glBindTextureUnit(TextureUnit - GL_TEXTURE0, texBufferID);
}

Texture2D::Texture2D(GLuint _texBufferID, bool _del) {
    texBufferID = _texBufferID;
    del = _del;
}
Texture2D::~Texture2D() = default;

bool Texture2D::Load(const std::string& path) {
    const GLuint replacement = loadTexture(path);
    if (replacement == 0) return false;
    if (del && texBufferID != 0) glDeleteTextures(1, &texBufferID);
    texBufferID = replacement;
    del = true;
    return true;
}
