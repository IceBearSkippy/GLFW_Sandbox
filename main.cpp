// Std. Includes
#include <string>
#include <iostream>
#include <fstream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
//#include "Shader.h"
//#include "Camera.h"
//#include "Model.h"

// GLM Mathemtics
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>

// Other Libs -- SOIL
//#include "SOIL2.h"
#include "Utilities.h"

using namespace std;

#define numVAOs 1

GLuint renderingProgram;
GLuint vao[numVAOs];

float x = 0.0f;
float inc = 0.01f;

void init(GLFWwindow* window) {
    renderingProgram = Utilities::createShaderProgram("./res/shaders/practice.vert", "./res/shaders/practice.frag");
    glGenVertexArrays(numVAOs, vao);
    glBindVertexArray(vao[0]);
}

void display(GLFWwindow* window, double currentTime) {
    glClear(GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT); // this references the color buffer that contains the pixels as they are rendered
 
    glUseProgram(renderingProgram);
    
    x += inc;
    if (x > 1.0f) {
        inc = -0.01f;
    }
    if (x < -1.0f) {
        inc = 0.01f;
    }
    GLuint offsetLoc = glGetUniformLocation(renderingProgram, "offset");
    glProgramUniform1f(renderingProgram, offsetLoc, x); // gett the variable and setting it to x
    
    //glPointSize(30.0f);
    glDrawArrays(GL_TRIANGLES, 0, 3);

}
int main(void) {
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
 
    GLFWwindow* window = glfwCreateWindow(600, 600, "Chapter2 - program1", NULL, NULL);
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        exit(EXIT_FAILURE);
    }
    glfwSwapInterval(1);

    init(window); // glew must be initialized before init
    
    while (!glfwWindowShouldClose(window)) {
        display(window, glfwGetTime());
        glfwSwapBuffers(window); // GLFW windows are double-buffered by default
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}