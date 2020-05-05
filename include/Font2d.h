#ifndef INCLUDE_FONT2D_H_
#define INCLUDE_FONT2D_H_
#include <Mesh.h>
#include <Text_2D.h>

#include <map>
#include <memory>
#include <string>

class Character2d : public MaterialObject, public PlaceableObject {
private:
    Mesh mesh;

    // название шрифта
    std::string fontName;
    // ссылка на файл
    std::string fileName;

    // отношение размеров картинки к экрану
    float kx, ky;

    // соотношение сторон экрана
    float aratio;

    // размер пикселя в uv
    float pkx, pky;
    // размер шрифта в пикселях
    int fontHeight;
    // размеры текстуры шрифта
    uint32_t imageWidth, imageHeight;
    // информация о шрифте, вернее обо всех буквах
    std::map<unsigned int, FontCharacter> fontInfo;

    // ширина символа в UV-координатах
    float realWidth;
    // высота символа в UV-координатах
    float realHeight;

    // какой-то коэффициент масштабирования
    // TODO подписать более подробно
    float dx;
    // смещение по х на текстуре в UV
    float xOffset;
    // смещение по y на текстуре в UV
    float yOffset;

    FontCharacter temp;

protected:
    // информация о кернинге
    std::map<uint32_t, float> kerningInfo;

    // TODO длина символа в текстурных координатах??
    glm::vec2 characterLength;

    // текущий символ
    unsigned int currentCharacter;

public:
    Character2d();

    float GetAspectRatio();
    // установка отношения + корректировка коээффициентов
    // машстабирования текстуры
    void SetAspectRatio(int _width, int _height);

    // возвращает высоту шрифта(в пикселях)
    int GetFontHeight();

    // возвращает  ширину в экранных координатах
    // текущего символа
    float GetWidth(unsigned int c);

    // возвращает высоту в экранных координатах
    // текущего символа
    float GetHeight(unsigned int c);

    // возвращает ширину пробела в экранных координатах
    float GetSpaceWidth();

    // устанавливает текущий символ
    void SetCharacter(unsigned int c);

    // инициализируем материалом и моделью quad2x2front.ho3d
    // и шрифтом
    bool Init(std::shared_ptr<Material> _mat, std::string _fileName);
    ~Character2d();
    void Render(Camera* cam);

    // TODO возвращает длину текущего символа в экрнных координатах??
    glm::vec2 GetLastCharacterLength();
};
#endif  // INCLUDE_FONT2D_H_
