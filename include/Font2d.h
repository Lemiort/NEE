#ifndef FONT2D_H
#define FONT2D_H
#include <Text_2D.h>
#include <Mesh.h>
#include <map>

using namespace std;

class Character2d: public MaterialObject, public PlaceableObject
{
private:
    Mesh mesh;

    //название шрифта
    string fontName;
    //ссылка на файл
    string fileName;

    //отношение размеров картинки к экрану
    float kx,ky;

    //соотношение сторон экрана
    float aratio;


    //размер пикселя в uv
    float pkx,pky;
    //размер шрифта в пикселях
    int fontHeight;
    //размеры текстуры шрифта
    uint32_t imageWidth, imageHeight;
    //информация о шрифте, вернее обо всех буквах
    map<unsigned int,FontCharacter> fontInfo;

    //ширина символа в UV-координатах
    float realWidth;
    //высота символа в UV-координатах
    float realHeight;

    //какой-то коэффициент масштабирования
    //TODO подписать более подробно
    float dx;
    //смещение по х на текстуре в UV
    float xOffset;
    //смещение по y на текстуре в UV
    float yOffset;

    FontCharacter temp;
protected:
    //информация о кернинге
    map<uint32_t,float> kerningInfo;

    //TODO длина символа в текстурных координатах??
    Vector2f characterLength;

    //текущий символ
    unsigned int currentCharacter;
public:
    Character2d();


    float GetAspectRatio();
    //установка отношения + корректировка коээффициентов
    //машстабирования текстуры
    void SetAspectRatio(int _width, int _height);

    //возвращает высоту шрифта(в пикселях)
    int GetFontHeight();

    //возвращает  ширину в экранных координатах
    //текущего символа
	float GetWidth(unsigned int c);

	//возвращает высоту в экранных координатах
	//текущего символа
	float GetHeight(unsigned int c);

	//возвращает ширину пробела в экранных координатах
	float GetSpaceWidth();

    //устанавливает текущий символ
	void SetCharacter(unsigned int c);

    //инициализируем материалом и моделью quad2x2front.ho3d
    //и шрифтом
    bool Init(shared_ptr<Material> _mat, string _fileName);
    ~Character2d();
    void Render(Camera* cam);

    //TODO возвращает длину текущего символа в экрнных координатах??
    Vector2f GetLastCharacterLength();
};
#endif // FONT2D_H
