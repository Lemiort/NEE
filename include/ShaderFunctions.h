#ifndef SHADERFUNCTIONS_H_INCLUDED
#define SHADERFUNCTIONS_H_INCLUDED
#include <EngineCommon.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
char* ReadFile(const char* filename);
GLuint MakeVertexShader(const char* source);
GLuint MakeFragmentShader(const char* source);
GLuint MakeGeometryShader(const char* source);
GLuint MakeShaderProgram(GLuint vertexShaderID, GLuint fragmentShaderID);
GLuint MakeShaderProgram(GLuint vertexShaderID, GLuint geometryShaderID,
                         GLuint fragmentShaderID);
using namespace std;
#endif  // SHADERFUNCTIONS_H_INCLUDED
