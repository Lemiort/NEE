#include "Texture.h"

Texture::Texture(bool _del)
{
    del=_del;
}

Texture::~Texture()
{
    if(del)
    {
        glDeleteTextures(1,&texBufferID);
        texBufferID=0;
    }
}

GLuint Texture::GetTextureID()
{
    return texBufferID;
}

///////////////////////////////////////////////////////////////////////////////
// convert OpenGL internal format enum to string
///////////////////////////////////////////////////////////////////////////////
std::string Texture::ConvertInternalFormatToString(GLenum format)
{
    std::string formatName;

    switch(format)
    {
    case GL_STENCIL_INDEX:      // 0x1901
        formatName = "GL_STENCIL_INDEX";
        break;
    case GL_DEPTH_COMPONENT:    // 0x1902
        formatName = "GL_DEPTH_COMPONENT";
        break;
    case GL_ALPHA:              // 0x1906
        formatName = "GL_ALPHA";
        break;
    case GL_RGB:                // 0x1907
        formatName = "GL_RGB";
        break;
    case GL_RGBA:               // 0x1908
        formatName = "GL_RGBA";
        break;
    case GL_LUMINANCE:          // 0x1909
        formatName = "GL_LUMINANCE";
        break;
    case GL_LUMINANCE_ALPHA:    // 0x190A
        formatName = "GL_LUMINANCE_ALPHA";
        break;
    case GL_R3_G3_B2:           // 0x2A10
        formatName = "GL_R3_G3_B2";
        break;
    case GL_ALPHA4:             // 0x803B
        formatName = "GL_ALPHA4";
        break;
    case GL_ALPHA8:             // 0x803C
        formatName = "GL_ALPHA8";
        break;
    case GL_ALPHA12:            // 0x803D
        formatName = "GL_ALPHA12";
        break;
    case GL_ALPHA16:            // 0x803E
        formatName = "GL_ALPHA16";
        break;
    case GL_LUMINANCE4:         // 0x803F
        formatName = "GL_LUMINANCE4";
        break;
    case GL_LUMINANCE8:         // 0x8040
        formatName = "GL_LUMINANCE8";
        break;
    case GL_LUMINANCE12:        // 0x8041
        formatName = "GL_LUMINANCE12";
        break;
    case GL_LUMINANCE16:        // 0x8042
        formatName = "GL_LUMINANCE16";
        break;
    case GL_LUMINANCE4_ALPHA4:  // 0x8043
        formatName = "GL_LUMINANCE4_ALPHA4";
        break;
    case GL_LUMINANCE6_ALPHA2:  // 0x8044
        formatName = "GL_LUMINANCE6_ALPHA2";
        break;
    case GL_LUMINANCE8_ALPHA8:  // 0x8045
        formatName = "GL_LUMINANCE8_ALPHA8";
        break;
    case GL_LUMINANCE12_ALPHA4: // 0x8046
        formatName = "GL_LUMINANCE12_ALPHA4";
        break;
    case GL_LUMINANCE12_ALPHA12:// 0x8047
        formatName = "GL_LUMINANCE12_ALPHA12";
        break;
    case GL_LUMINANCE16_ALPHA16:// 0x8048
        formatName = "GL_LUMINANCE16_ALPHA16";
        break;
    case GL_INTENSITY:          // 0x8049
        formatName = "GL_INTENSITY";
        break;
    case GL_INTENSITY4:         // 0x804A
        formatName = "GL_INTENSITY4";
        break;
    case GL_INTENSITY8:         // 0x804B
        formatName = "GL_INTENSITY8";
        break;
    case GL_INTENSITY12:        // 0x804C
        formatName = "GL_INTENSITY12";
        break;
    case GL_INTENSITY16:        // 0x804D
        formatName = "GL_INTENSITY16";
        break;
    case GL_RGB4:               // 0x804F
        formatName = "GL_RGB4";
        break;
    case GL_RGB5:               // 0x8050
        formatName = "GL_RGB5";
        break;
    case GL_RGB8:               // 0x8051
        formatName = "GL_RGB8";
        break;
    case GL_RGB10:              // 0x8052
        formatName = "GL_RGB10";
        break;
    case GL_RGB12:              // 0x8053
        formatName = "GL_RGB12";
        break;
    case GL_RGB16:              // 0x8054
        formatName = "GL_RGB16";
        break;
    case GL_RGBA2:              // 0x8055
        formatName = "GL_RGBA2";
        break;
    case GL_RGBA4:              // 0x8056
        formatName = "GL_RGBA4";
        break;
    case GL_RGB5_A1:            // 0x8057
        formatName = "GL_RGB5_A1";
        break;
    case GL_RGBA8:              // 0x8058
        formatName = "GL_RGBA8";
        break;
    case GL_RGB10_A2:           // 0x8059
        formatName = "GL_RGB10_A2";
        break;
    case GL_RGBA12:             // 0x805A
        formatName = "GL_RGBA12";
        break;
    case GL_RGBA16:             // 0x805B
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
    case GL_DEPTH_STENCIL:      // 0x84F9
        formatName = "GL_DEPTH_STENCIL";
        break;
    case GL_RGBA32F:            // 0x8814
        formatName = "GL_RGBA32F";
        break;
    case GL_RGB32F:             // 0x8815
        formatName = "GL_RGB32F";
        break;
    case GL_RGBA16F:            // 0x881A
        formatName = "GL_RGBA16F";
        break;
    case GL_RGB16F:             // 0x881B
        formatName = "GL_RGB16F";
        break;
    case GL_DEPTH24_STENCIL8:   // 0x88F0
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
// return texture parameters as string using glGetTexLevelParameteriv()
///////////////////////////////////////////////////////////////////////////////
std::string Texture::GetParameters()
{
    GLuint id=texBufferID;
    if(glIsTexture(id) == GL_FALSE)
        return "Not texture object";

    int width, height, format;
    std::string formatName;
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);            // get texture width
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);          // get texture height
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format); // get texture internal format
    glBindTexture(GL_TEXTURE_2D, 0);

    formatName = ConvertInternalFormatToString(format);

    std::stringstream ss;
    ss << width << "x" << height << ", " << formatName;
    return ss.str();
}


void Texture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_2D, texBufferID);
}

Texture2D::Texture2D(GLuint _texBufferID, bool _del)
{
    texBufferID=_texBufferID;
    del=_del;
}
Texture2D::~Texture2D()
{
    /*if(del && texBufferID!=0)
        glDeleteTextures(1,&texBufferID);*/
}

bool Texture2D::Load(const char* filename)
{
    texBufferID=SOIL_load_OGL_texture
	(
		filename,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(texBufferID)
        return true;
	else
        return false;
}


void AbstractTexture::SetTexture(GLuint _texID)
{
    texBufferID=_texID;
}
