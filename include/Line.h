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
    /*glm::vec3 p1;
    glm::vec3 p2;
    glm::vec3 c1;*/
    //	GLuint shaderProgramID;
    GLuint PixelColorID, PointSizeID;
    GLuint gWorldID;
    GLuint positionID;
    GLuint VBO;
    float col[3];
    float pos[6];
    Line(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color);
    Line(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color, GLuint shader);
    ~Line();
    void Render(Camera* pGameCamera);

private:
    shared_ptr<Shader> shaderProgram;
};
#endif  // LINE_H_INCLUDED
