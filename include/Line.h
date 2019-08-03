#ifndef LINE_H_INCLUDED
#define LINE_H_INCLUDED
#include "Assistant.h"
#include "Camera.h"
#include "Shader.h"
#include "ShaderFunctions.h"
#include "math_3d.h"
#define BUFFER_OFFSET(i) ((char*)NULL + (i))

class Line {
public:
    /*Vector3f p1;
    Vector3f p2;
    Vector3f c1;*/
    //	GLuint shaderProgramID;
    GLuint PixelColorID, PointSizeID;
    GLuint gWorldID;
    GLuint positionID;
    GLuint VBO;
    float col[3];
    float pos[6];
    Line(Vector3f pos1, Vector3f pos2, Vector3f color);
    Line(Vector3f pos1, Vector3f pos2, Vector3f color, GLuint shader);
    ~Line();
    void Render(Camera* pGameCamera);

private:
    shared_ptr<Shader> shaderProgram;
};
#endif  // LINE_H_INCLUDED
