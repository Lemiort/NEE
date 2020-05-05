#ifndef INCLUDE_SHADERFUNCTIONS_H
#define INCLUDE_SHADERFUNCTIONS_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>

#include "EngineCommon.h"
char* ReadFile(const char* filename);
GLuint MakeVertexShader(const char* source);
GLuint MakeFragmentShader(const char* source);
GLuint MakeGeometryShader(const char* source);
GLuint MakeShaderProgram(GLuint vertexShaderID, GLuint fragmentShaderID);
GLuint MakeShaderProgram(GLuint vertexShaderID, GLuint geometryShaderID,
                         GLuint fragmentShaderID);

#endif  // INCLUDE_SHADERFUNCTIONS_H
