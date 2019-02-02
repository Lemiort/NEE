#include "Font.h"

#include <fstream>
#include <sstream>

namespace {
const std::string kFontsPath = "Fonts";

enum class ParsingStep { kTextureName, kFontName, kCharacters, kKerning };
}  // namespace

void Font::AddCharacter(const std::string& line) {
    std::stringstream sstr(line);
    FontCharacter character;

    // read code
    sstr >> character.code;
    sstr >> character.x_pos;
    sstr >> character.y_pos;
    sstr >> character.figure_width;
    sstr >> character.figure_height;
    sstr >> character.x_offset;
    sstr >> character.y_offset;
    sstr >> character.character_width;
    sstr >> character.character_height;
    font_map_[character.code] = character;
}

void Font::AddKerning(const std::string& line) {
    std::stringstream sstr(line);

    // parse
    int code1, code2;
    sstr >> code1 >> code2;
    float f1;
    sstr >> f1;
    kerning_map_[{code1, code2}] = f1;
}

std::string Font::ParseImageFilename(const std::string& line) {
    std::stringstream sstr(line);
    std::string temp;
    sstr >> temp;  // parse "textures:"
    sstr >> temp;  // parse filename
    return temp;
}

void Font::ParseName(const std::string& line) {
    std::stringstream sstr(line);
    sstr >> name_;
    std::string size;
    sstr >> size;
    auto size_substr = std::string(size, 0, size.length() - 2);
    size_ = std::stoi(size_substr);
}

void Font::Init(std::string font_filename) {
    std::ifstream fin(font_filename, std::ios::in);
    std::string in_str;
    std::string image_filename;

    ParsingStep step = ParsingStep::kTextureName;

    bool kerning = false;
    bool data = false;
    while (std::getline(fin, in_str)) {
        switch (step) {
            case ParsingStep::kTextureName: {
                image_filename = ParseImageFilename(in_str);
                // std::filesystem::path image_path =
                //     std::filesystem::u8path(image_filename);
                // image_filename =
                //     image_path.parent_path().string() + image_filename;
                image_filename = kFontsPath + "/" + image_filename;
                step = ParsingStep::kFontName;
            } break;
            case ParsingStep::kFontName: {
                ParseName(in_str);
                step = ParsingStep::kCharacters;
            } break;
            case ParsingStep::kCharacters: {
                if (in_str.find("kerning pairs:") >= 0) {
                    step = ParsingStep::kKerning;
                } else {
                    AddCharacter(in_str);
                }
            } break;
            case ParsingStep::kKerning: {
                AddKerning(in_str);
            } break;
        }
    }

    // //������ ��������
    // texBufferID = SOIL_load_OGL_texture(
    //     (char*)imgFilename.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
    //     SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB |
    //         SOIL_FLAG_COMPRESS_TO_DXT);
    // // ������ �������� ���������� ���� 0
    // glActiveTexture(GL_TEXTURE0);
    // // ��������� �������� �� �������� ���������� ����
    // glBindTexture(GL_TEXTURE_2D, texBufferID);
    // texSamplerID = shaderProgram->GetUniformLocation("texSampler");

    // //�������� ������� �����������
    // FILE* imageFile = fopen(imgFilename.c_str(), "rb");
    // char buffer2[4];

    // //��������� ������������
    // fread(&buffer2, 1, 4, imageFile);
    // // printf("\nBuffer 0 %x",buffer);
    // // printf("\nBuffer 0 %x %x %x
    // // %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    // fread(&buffer2, 1, 4, imageFile);
    // // printf("\nBuffer 1 %x %x %x
    // // %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    // fread(&buffer2, 1, 4, imageFile);
    // // printf("\nIHDR chank");
    // // printf("\nBuffer 2 %x %x %x
    // // %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    // fread(&buffer2, 1, 4, imageFile);
    // // printf("\nBuffer 3 %x %x %x
    // // %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    // fread(&buffer2, 1, 4, imageFile);
    // // printf("\nBuffer 4 %x %x %x
    // // %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);

    // //������� ������ ������
    // imageWidth = ((uint32_t)buffer2[3] << 0) | ((uint32_t)buffer2[2] << 8) |
    //              ((uint32_t)buffer2[1] << 16) | ((uint32_t)buffer2[0] << 24);
    // printf("\n width=%d", imageWidth);

    // //������� ������
    // fread(&buffer2, 1, 4, imageFile);
    // // printf("\nBuffer 5 %x %x %x
    // // %x",buffer2[0],buffer2[1],buffer2[2],buffer2[3]);
    // imageHeight = ((uint32_t)buffer2[3] << 0) | ((uint32_t)buffer2[2] << 8) |
    //               ((uint32_t)buffer2[1] << 16) | ((uint32_t)buffer2[0] <<
    //               24);
    // printf("\n height=%d", imageHeight);

    // //��������� ������� � ������������� ����������
    // pkx = 1.0f / (float)imageWidth;
    // pky = 1.0f / (float)imageHeight;

    // fclose(imageFile);

    // return true;
}

Font::~Font() {}