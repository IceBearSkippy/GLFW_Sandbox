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
//#include "ImportedModel.h" <-- this is too basic

using namespace std;

#define numVAOs 1
#define numVBOs 4

//allocate variables used in display function
glm::vec3 cameraVec, cameraRotU, cameraRotV, cameraRotN;
GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

//Sphere mySphere(48);
Torus myTorus(0.5f, 0.2f, 48);
//ImportedModel myModel("./res/models/bleh.obj");
GLuint brickTexture;
GLuint mvLoc, projLoc, nLoc;
int width, height;
float aspect;

GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mAmbLoc, mDiffLoc, mSpecLoc, mShiLoc;
glm::mat4 pMat, vMat, mMat, mvMat, invTrMat;
stack<glm::mat4> mvStack;
glm::vec3 currentLightPos, lightPosV; //light position as Vec3f in both model and view space
float lightPos[3];  // light position as float array

//inital light location
glm::vec3 initialLightLoc = glm::vec3(5.0f, 2.0f, 2.0f);

//white light properties
float globalAmbient[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
float lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

//gold material properties
float* matAmb = Utils::goldAmbient();
float* matDif = Utils::goldDiffuse();
float* matSpe = Utils::goldSpecular();
float matShi = Utils::goldShininess();


//function prototypes
void setupVertices(void);
void init(GLFWwindow* window);
void display(GLFWwindow* window, double currentTime);
void window_reshape_callback(GLFWwindow* window, int newWidth, int newHeight);
int main(void);
void installLights(glm::mat4);

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
    //renderingProgram = Utils::createShaderProgram("./res/shaders/practice.vert", "./res/shaders/practice.frag");
    renderingProgram = Utils::createShaderProgram("./res/shaders/lighting.vert", "./res/shaders/lighting.frag");
    cameraVec = glm::vec3(0.0f, 0.0f, -4.0f);

    cameraRotU = normalize(glm::vec3(1.0f, 0.0f, 0.0f));
    cameraRotV = normalize(glm::vec3(0.0f, 1.0f, 0.0f));
    cameraRotN = normalize(glm::vec3(0.0f, 0.0f, 1.0f));

    setupVertices();
    glfwGetFramebufferSize(window, &width, &height);
    aspect = (float)width / (float)height;
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // 1.0672 radians = 60 degrees
    //brickTexture = Utils::loadTexture("./res/images/brick1.jpg");
}

void display(GLFWwindow* window, double currentTime) {
    glClear(GL_DEPTH_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(renderingProgram);

    // get the uniform variables for MV and projection matrices
    mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
    projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
    nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");

    vMat = Utils::buildCameraLocation(cameraVec, cameraRotU, cameraRotV, cameraRotN);

    //planetary system transform create and draw
    mvStack.push(vMat);

    mvStack.push(mvStack.top());
    mvStack.top() *= Utils::buildTranslate(0.0f, 0.0f, 0.0f);
    mvStack.push(mvStack.top());
    mvStack.top() *= Utils::buildRotateY((float)currentTime);
    mvStack.top() *= Utils::buildRotateX(-0.7f);
    mvStack.top() *= Utils::buildTranslate(0.0f, sin((float)currentTime), 0.0f);

    //set up lights based on the current light's position
    currentLightPos = glm::vec3(initialLightLoc.x, initialLightLoc.y, initialLightLoc.z);
    
    installLights(vMat); 

    //build the mv matrix
    //mvMat = vMat * mMat;

    //build the inverse-transpose of the MV matrix for transforming normal vectors
    invTrMat = glm::transpose(glm::inverse(mvStack.top()));

    // put mv, proj and inverse-transpose(normal) into normals
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));


    
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    //bind the related texture immediately after
    //glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //glEnableVertexAttribArray(1);

    //specify texture
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, brickTexture);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
    glDrawElements(GL_TRIANGLES, myTorus.getNumIndices(), GL_UNSIGNED_INT, 0);

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

void installLights(glm::mat4 vMatrix) {
    // Installing lights:
    //     vMat makes light relative to view
    //     mvStack.top() makes light relative to movement (shadow stays in same spots)

    //convert light's position to view space and save it in a float array
    lightPosV = glm::vec3(vMatrix * glm::vec4(currentLightPos, 1.0f));
    lightPos[0] = lightPosV.x;
    lightPos[1] = lightPosV.y;
    lightPos[2] = lightPosV.z;

    //get the location of the light/material fields in the shader
    globalAmbLoc = glGetUniformLocation(renderingProgram, "globalAmbient");
    ambLoc = glGetUniformLocation(renderingProgram, "light.ambient");
    diffLoc = glGetUniformLocation(renderingProgram, "light.diffuse");
    specLoc = glGetUniformLocation(renderingProgram, "light.specular");
    posLoc = glGetUniformLocation(renderingProgram, "light.position");
    mAmbLoc = glGetUniformLocation(renderingProgram, "material.ambient");
    mDiffLoc = glGetUniformLocation(renderingProgram, "material.diffuse");
    mSpecLoc = glGetUniformLocation(renderingProgram, "material.specular");
    mShiLoc = glGetUniformLocation(renderingProgram, "material.shininess");

    //set the uniform light/material values in the shader
    glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);
    glProgramUniform4fv(renderingProgram, ambLoc, 1, lightAmbient);
    glProgramUniform4fv(renderingProgram, diffLoc, 1, lightDiffuse);
    glProgramUniform4fv(renderingProgram, specLoc, 1, lightSpecular);
    glProgramUniform3fv(renderingProgram, posLoc, 1, lightPos);
    glProgramUniform4fv(renderingProgram, mAmbLoc, 1, matAmb);
    glProgramUniform4fv(renderingProgram, mDiffLoc, 1, matDif);
    glProgramUniform4fv(renderingProgram, mSpecLoc, 1, matSpe);
    glProgramUniform1f(renderingProgram, mShiLoc, matShi);

}