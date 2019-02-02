#ifndef LINE_H
#define LINE_H

#include <GL/glew.h>

class Line {
public:
    /*Vector3f p1;
    Vector3f p2;
    Vector3f c1;*/
    // GLuint shaderProgramID;
    // GLuint PixelColorID, PointSizeID;
    // GLuint gWorldID;
    // GLuint positionID;
    // float col[3];
    float pos[6];
    Line() {
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), nullptr,
                     GL_STATIC_DRAW);

        pos[0] = 0;
        pos[1] = 0;
        pos[2] = 0;
        pos[3] = 1;
        pos[4] = 1;
        pos[5] = 1;
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), pos);
    }
    ~Line() {}
    // void Render(Camera* pGameCamera, int width, int height);
    void Render() {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        // glUniformMatrix4fv(gWorldID, 1, GL_TRUE, (const
        // GLfloat*)TM.GetTSRVC()); glUniform1f(PointSizeID, 1.5);
        // glUniform4f(PixelColorID, col[0], col[1], col[2], 1.0);
        // glEnableVertexAttribArray(positionID);
        glDrawArrays(GL_LINES, 0, 2);
        // glDisableVertexAttribArray(positionID);
    }

private:
    GLuint VBO, IBO;
};
#endif /* LINE_H */
