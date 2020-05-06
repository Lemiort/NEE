#ifndef INCLUDE_SHADERFUNCTIONS_H
#define INCLUDE_SHADERFUNCTIONS_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>

#include "EngineCommon.h"
GLuint MakeVertexShader(const std::string& source);
GLuint MakeFragmentShader(const std::string& source);
GLuint MakeGeometryShader(const std::string& source);
GLuint MakeShaderProgram(GLuint vertexShaderID, GLuint fragmentShaderID);
GLuint MakeShaderProgram(GLuint vertexShaderID, GLuint geometryShaderID,
                         GLuint fragmentShaderID);

#endif  // INCLUDE_SHADERFUNCTIONS_H
