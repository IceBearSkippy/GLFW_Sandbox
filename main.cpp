// Std. Includes
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stack>

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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
// Other Libs -- SOIL
#include "SOIL2.h"

#include "Utils.h"
#include "Sphere.h"
#include "Torus.h"

using namespace std;

#define numVAOs 1
#define numVBOs 4

//allocate variables used in display function
glm::vec3 cameraVec, cameraRotU, cameraRotV, cameraRotN;
GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

Sphere mySphere(48);
Torus myTorus(0.5f, 0.2f, 48);

GLuint brickTexture;
GLuint mvLoc, projLoc;
int width, height;
float aspect;
glm::mat4 pMat, vMat, mMat, mvMat;
stack<glm::mat4> mvStack;

//function prototypes
void setupVertices(void);
void init(GLFWwindow* window);
void display(GLFWwindow* window, double currentTime);
void window_reshape_callback(GLFWwindow* window, int newWidth, int newHeight);
int main(void);

void setupVertices(void) {

    vector<int> ind = myTorus.getIndices();
    vector<glm::vec3> vert = myTorus.getVertices();
    vector<glm::vec2> tex = myTorus.getTexCoords();
    vector<glm::vec3> norm = myTorus.getNormals();

    vector<float> pvalues; //vertex positions
    vector<float> tvalues; //texture coordinates
    vector<float> nvalues; //normal vectors

    int numVertices = myTorus.getNumVertices();
    for (int i = 0; i < numVertices; i++) {
        pvalues.push_back(vert[i].x);
        pvalues.push_back(vert[i].y);
        pvalues.push_back(vert[i].z);

        tvalues.push_back(tex[i].s);
        tvalues.push_back(tex[i].t);

        nvalues.push_back(norm[i].x);
        nvalues.push_back(norm[i].y);
        nvalues.push_back(norm[i].z);
    }

    glGenVertexArrays(1, vao);
    glBindVertexArray(vao[0]);
    glGenBuffers(numVBOs, vbo); // four vbos are created

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 4, &tvalues[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]); // indices
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * 4, &ind[0], GL_STATIC_DRAW);

}

void init(GLFWwindow* window) {
    renderingProgram = Utils::createShaderProgram("./res/shaders/practice.vert", "./res/shaders/practice.frag");
    cameraVec = glm::vec3(0.0f, 0.0f, -3.0f);
    // N is the lookat vector (try staring at an object with it)
    cameraRotU = normalize(glm::vec3(1.0f, 0.0f, 0.0f));
    cameraRotV = normalize(glm::vec3(0.0f, 1.0f, 0.0f));
    cameraRotN = normalize(glm::vec3(0.0f, 0.0f, 1.0f));

    setupVertices();
    glfwGetFramebufferSize(window, &width, &height);
    aspect = (float)width / (float)height;
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // 1.0672 radians = 60 degrees
    brickTexture = Utils::loadTexture("./res/images/brick1.jpg");
}

void display(GLFWwindow* window, double currentTime) {
    glClear(GL_DEPTH_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(renderingProgram);

    // get the uniform variables for MV and projection matrices
    mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
    projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");

    vMat = Utils::buildCameraLocation(cameraVec, cameraRotU, cameraRotV, cameraRotN);

    //planetary system transform create and draw
    mvStack.push(vMat);

    mvStack.push(mvStack.top());
    mvStack.top() *= Utils::buildTranslate(0.0f, 0.0f, 0.0f);
    mvStack.push(mvStack.top());
    mvStack.top() *= Utils::buildRotateY((float)currentTime);
    mvStack.top() *= Utils::buildRotateX((float)currentTime/2);
    mvStack.top() *= Utils::buildTranslate(0.0f, sin((float)currentTime), 0.0f);

    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

    glEnable(GL_CULL_FACE);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    //bind the related texture immediately after
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    //specify texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, brickTexture);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glFrontFace(GL_CCW);
    glDrawElements(GL_TRIANGLES, myTorus.getNumIndices(), GL_UNSIGNED_INT, 0);
    //glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());
    
   
    

    //remove the moon scale/rotation/position, planet position, sun_position
    // and view matrices from stack
    //mvStack.pop(); mvStack.pop(); mvStack.pop(); mvStack.pop();
    while (!mvStack.empty()) {
        mvStack.pop();
    }
}

void window_reshape_callback(GLFWwindow* window, int newWidth, int newHeight) {
    aspect = (float)newWidth / (float)newHeight;
    glViewport(0, 0, newWidth, newHeight);
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
}

int main(void) {
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    // for mac -- Catalina stuck at 4.1
    // would need to change shader versions as well
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    //you need these two for mac
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(600, 600, "Chapter 6", NULL, NULL);
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        exit(EXIT_FAILURE);
    }
    glfwSwapInterval(1);

    glfwSetWindowSizeCallback(window, window_reshape_callback);

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