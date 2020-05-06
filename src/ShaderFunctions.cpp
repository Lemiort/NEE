#include "ShaderFunctions.h"

GLuint MakeVertexShader(const std::string& source) {
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    if (vertexShaderID == 0) {
        fprintf(stderr, "\nError creating vertex shader\n");
        exit(1);
    }
    const char* c_str{source.c_str()};
    glShaderSource(vertexShaderID, 1, static_cast<const GLchar**>(&c_str),
                   nullptr);
    glCompileShader(vertexShaderID);
    return vertexShaderID;
}

GLuint MakeFragmentShader(const std::string& source) {
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    if (fragmentShaderID == 0) {
        fprintf(stderr, "Error creating fragment shader\n");
        exit(1);
    }
    const char* c_str{source.c_str()};
    glShaderSource(fragmentShaderID, 1, static_cast<const GLchar**>(&c_str),
                   nullptr);
    glCompileShader(fragmentShaderID);
    return fragmentShaderID;
}

GLuint MakeGeometryShader(const std::string& source) {
    GLuint geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
    if (geometryShaderID == 0) {
        fprintf(stderr, "Error creating fragment shader\n");
        exit(1);
    }
    const char* c_str{source.c_str()};
    glShaderSource(geometryShaderID, 1, static_cast<const GLchar**>(&c_str),
                   nullptr);
    glCompileShader(geometryShaderID);
    return geometryShaderID;
}

GLuint MakeShaderProgram(GLuint vertexShaderID, GLuint fragmentShaderID) {
    FILE* flog;
    // fopen_s(&flog,"shaderbuild.log","w");
    flog = fopen("shaderbuild.log", "w");
    GLuint shaderID = glCreateProgram();
    if (shaderID == 0) {
        fprintf(stderr, "Error creating shader program\n");
        fprintf(flog, "Error creating shader program\n");
        fclose(flog);
        exit(1);
    }
    glAttachShader(shaderID, vertexShaderID);
    glAttachShader(shaderID, fragmentShaderID);
    glLinkProgram(shaderID);

    GLint Success = 0;
    GLchar ErrorLog[1024] = {0};
    glGetProgramiv(shaderID, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        fprintf(flog, "Error linking shader program: '%s'\n", ErrorLog);
        fclose(flog);
        exit(1);
    }

    glValidateProgram(shaderID);
    glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        fprintf(flog, "Invalid shader program: '%s'\n", ErrorLog);
        fclose(flog);
        exit(1);
    }
    fclose(flog);
    return shaderID;
}
GLuint MakeShaderProgram(GLuint vertexShaderID, GLuint geometryShaderID,
                         GLuint fragmentShaderID) {
    FILE* flog;
    // fopen_s(&flog,"shaderbuild.log","w");
    flog = fopen("shaderbuild.log", "w");
    GLuint shaderID = glCreateProgram();
    if (shaderID == 0) {
        fprintf(stderr, "Error creating shader program\n");
        fprintf(flog, "Error creating shader program\n");
        fclose(flog);
        exit(1);
    }
    glAttachShader(shaderID, vertexShaderID);
    glAttachShader(shaderID, fragmentShaderID);
    glAttachShader(shaderID, geometryShaderID);
    glLinkProgram(shaderID);

    GLint Success = 0;
    GLchar ErrorLog[1024] = {0};
    glGetProgramiv(shaderID, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        fprintf(flog, "Error linking shader program: '%s'\n", ErrorLog);
        fclose(flog);
        exit(1);
    }

    glValidateProgram(shaderID);
    glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        fprintf(flog, "Invalid shader program: '%s'\n", ErrorLog);
        fclose(flog);
        exit(1);
    }
    fclose(flog);
    return shaderID;
}
