#ifndef FONT2D_H
#define FONT2D_H
#include <Mesh.h>
#include <Text_2D.h>
#include <map>


using namespace std;

class Character2d : public MaterialObject, public PlaceableObject {
private:
    Mesh mesh;

    //�������� ������
    string fontName;
    //������ �� ����
    string fileName;

    //��������� �������� �������� � ������
    float kx, ky;

    //����������� ������ ������
    float aratio;

    //������ ������� � uv
    float pkx, pky;
    //������ ������ � ��������
    int fontHeight;
    //������� �������� ������
    uint32_t imageWidth, imageHeight;
    //���������� � ������, ������ ��� ���� ������
    map<unsigned int, FontCharacter> fontInfo;

    //������ ������� � UV-�����������
    float realWidth;
    //������ ������� � UV-�����������
    float realHeight;

    //�����-�� ����������� ���������������
    // TODO ��������� ����� ��������
    float dx;
    //�������� �� � �� �������� � UV
    float xOffset;
    //�������� �� y �� �������� � UV
    float yOffset;

    FontCharacter temp;

protected:
    //���������� � ��������
    map<uint32_t, float> kerningInfo;

    // TODO ����� ������� � ���������� �����������??
    glm::vec2 characterLength;

    //������� ������
    unsigned int currentCharacter;

public:
    Character2d();

    float GetAspectRatio();
    //��������� ��������� + ������������� ��������������
    //��������������� ��������
    void SetAspectRatio(int _width, int _height);

    //���������� ������ ������(� ��������)
    int GetFontHeight();

    //����������  ������ � �������� �����������
    //�������� �������
    float GetWidth(unsigned int c);

    //���������� ������ � �������� �����������
    //�������� �������
    float GetHeight(unsigned int c);

    //���������� ������ ������� � �������� �����������
    float GetSpaceWidth();

    //������������� ������� ������
    void SetCharacter(unsigned int c);

    //�������������� ���������� � ������� quad2x2front.ho3d
    //� �������
    bool Init(shared_ptr<Material> _mat, string _fileName);
    ~Character2d();
    void Render(Camera* cam);

    // TODO ���������� ����� �������� ������� � ������� �����������??
    glm::vec2 GetLastCharacterLength();
};
#endif  // FONT2D_H
