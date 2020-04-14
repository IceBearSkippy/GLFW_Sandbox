#pragma once
#include <string>
#include <iostream>
#include <fstream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

using namespace std;

namespace Utilities {

    string readShaderSource(const char* filePath) {
        string content;
        ifstream fileStream(filePath, ios::in);
        string line = "";
        while (!fileStream.eof()) {
            getline(fileStream, line);
            content.append(line + "\n");
        }
        fileStream.close();
        return content;
    }

    void printShaderLog(GLuint shader) {
        int len = 0;
        int chWrittn = 0;
        char* log;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        if (len > 0) {
            log = (char*)malloc(len);
            glGetShaderInfoLog(shader, len, &chWrittn, log);
            cout << "Shader Info Log: " << log << endl;
            free(log);
        }
    }

    void printProgramLog(int prog) {
        int len = 0;
        int chWrittn = 0;
        char* log;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
        if (len > 0) {
            log = (char*)malloc(len);
            glGetProgramInfoLog(prog, len, &chWrittn, log);
            cout << "Program Info Log: " << log << endl;
            free(log);
        }
    }

    bool checkOpenGLError() {
        bool foundError = false;
        int glErr = glGetError();
        while (glErr != GL_NO_ERROR) {
            cout << "glError: " << glErr << endl;
            foundError = true;
            glErr = glGetError();
        }
        return foundError;
    }

    // outline for reading shaders, could overloading for tesselation/geomtery shaders
    GLuint createShaderProgram(const char *vp, const char *fp) {
        GLint vertCompiled;
        GLint fragCompiled;
        GLint linked;

        string vertShaderStr = readShaderSource(vp);
        string fragShaderStr = readShaderSource(fp);

        const char* vertShaderSrc = vertShaderStr.c_str();
        const char* fragShaderSrc = fragShaderStr.c_str();

        GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
        GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(vShader, 1, &vertShaderSrc, NULL);
        glShaderSource(fShader, 1, &fragShaderSrc, NULL);
        glCompileShader(vShader);
        //check for errors after compiling each shader
        checkOpenGLError();
        glGetShaderiv(vShader, GL_COMPILE_STATUS, &vertCompiled);
        if (vertCompiled != 1) {
            cout << "ERROR:::::::Vertex compilation failed" << endl;
            printShaderLog(vShader);
        }

        glCompileShader(fShader);
        glGetShaderiv(fShader, GL_COMPILE_STATUS, &fragCompiled);
        if (fragCompiled != 1) {
            cout << "ERROR:::::::Fragment compilation failed" << endl;
            printShaderLog(fShader);
        }
        GLuint vfProgram = glCreateProgram();

        glAttachShader(vfProgram, vShader);
        glAttachShader(vfProgram, fShader);

        glLinkProgram(vfProgram);
        checkOpenGLError();
        glGetProgramiv(vfProgram, GL_LINK_STATUS, &linked);
        if (linked != 1) {
            cout << "ERROR:::::::Linking failed" << endl;
            printShaderLog(vfProgram);
        }
        return vfProgram;
    }

    
}