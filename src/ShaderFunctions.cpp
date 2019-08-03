#include "ShaderFunctions.h"

char* ReadFile(const char* filename) {
    FILE* fp;
    // fopen_s(&fp, filename, "r");
    fp = fopen(filename, "r");
    if (fp == NULL) {
        // MessageBox( NULL, "Cannot open shader file!", "ERROR",
        //    MB_OK | MB_ICONEXCLAMATION )
        printf("Cant open shader file!");
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    long file_length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* contents = new char[file_length + 1];
    for (int i = 0; i < file_length + 1; i++) {
        contents[i] = 0;
    }

    fread(contents, 1, file_length, fp);
    contents[file_length] = '\0';
    fclose(fp);
    return contents;
}

GLuint MakeVertexShader(const char* source) {
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    if (vertexShaderID == 0) {
        fprintf(stderr, "\nError creating vertex shader\n");
        exit(1);
    }
    glShaderSource(vertexShaderID, 1, (const GLchar**)&source, NULL);
    glCompileShader(vertexShaderID);
    return vertexShaderID;
}

GLuint MakeFragmentShader(const char* source) {
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    if (fragmentShaderID == 0) {
        fprintf(stderr, "Error creating fragment shader\n");
        exit(1);
    }
    glShaderSource(fragmentShaderID, 1, (const GLchar**)&source, NULL);
    glCompileShader(fragmentShaderID);
    return fragmentShaderID;
}

GLuint MakeGeometryShader(const char* source) {
    GLuint geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
    if (geometryShaderID == 0) {
        fprintf(stderr, "Error creating fragment shader\n");
        exit(1);
    }
    glShaderSource(geometryShaderID, 1, (const GLchar**)&source, NULL);
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
    //��������

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
    //��������

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
