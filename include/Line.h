#ifndef INCLUDE_LINE_H
#define INCLUDE_LINE_H
#include <memory>

#include "Assistant.h"
#include "Camera.h"
#include "Shader.h"
#include "ShaderFunctions.h"

using std::shared_ptr;

class Line {
public:
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
    std::shared_ptr<Shader> shaderProgram;
};
#endif  // INCLUDE_LINE_H
