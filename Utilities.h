#pragma once
#include <string>
#include <iostream>
#include <fstream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

namespace Utilities {
    
    //shader/error handler
    string readShaderSource(const char* filePath);
    void printShaderLog(GLuint shader);
    bool checkOpenGLError();
    GLuint createShaderProgram(const char* vp, const char* fp);

    //matrix functions
    mat4 buildCameraLocation(vec3 camera, vec3 uComp, vec3 vComp, vec3 nComp);
    mat4 buildRotateX(float rad);
    mat4 buildRotateY(float rad);
    mat4 buildRotateZ(float rad);
    mat4 buildTranslate(float x, float y, float z);
    mat4 buildScale(float x, float y, float z);


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
    GLuint createShaderProgram(const char* vp, const char* fp) {
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

    mat4 buildCameraLocation(vec3 camera, vec3 uComp, vec3 vComp, vec3 nComp) {
        mat4 rotMat = mat4(
            uComp.x, uComp.y, uComp.z, 0.0,
            vComp.x, vComp.y, vComp.z, 0.0,
            nComp.x, nComp.y, nComp.z, 0.0,
              0.0,     0.0,     0.0,   1.0
        );
        
        mat4 camMat = buildTranslate(camera.x, camera.y, camera.z);
        return camMat * rotMat;
    }

    // Transformation matrices
    mat4 buildTranslate(float x, float y, float z) {
        mat4 trans = mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            x, y, z, 1.0
        );
        return trans;
    }

    mat4 buildRotateX(float rad) {
        mat4 xrot = mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, cos(rad), -sin(rad), 0.0,
            0.0, sin(rad), cos(rad), 0.0,
            0.0, 0.0, 0.0, 1.0
        );
        return xrot;
    }

    mat4 buildRotateY(float rad) {
        mat4 yrot = mat4(
            cos(rad), 0.0, sin(rad), 0.0,
            0.0, 1.0, 0.0, 0.0,
            -sin(rad), 0.0, cos(rad), 0.0,
            0.0, 0.0, 0.0, 1.0
        );
        return yrot;
    }

    mat4 buildRotateZ(float rad) {
        mat4 zrot = mat4(
            cos(rad), -sin(rad), 0.0, 0.0,
            sin(rad), cos(rad), 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        );
        return zrot;
    }

    mat4 buildScale(float x, float y, float z) {
        mat4 scale = mat4(
            x, 0.0, 0.0, 0.0,
            0.0, y, 0.0, 0.0,
            0.0, 0.0, z, 0.0,
            0.0, 0.0, 0.0, 1.0
        );
        return scale;
    }
}