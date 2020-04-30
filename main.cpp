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
#include "Camera.h"

using namespace std;

#define numVAOs 1
#define numVBOs 10

GLuint renderingProgram, shadowProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

Sphere mySphere(48);
Torus myTorus(0.5f, 0.2f, 48);
GLuint brickTexture, whiteTexture;
GLuint mvLoc, projLoc, nLoc, sLoc;
int width, height;
float aspect;

GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mAmbLoc, mDiffLoc, mSpecLoc, mShiLoc;
glm::mat4 pMat, vMat, mMat, mvMat, invTrMat;
stack<glm::mat4> mvStack;
glm::vec3 currentLightPos, lightPosV; //light position as Vec3f in both model and view space
float lightPos[3];  // light position as float array

//camera 
Camera camera(glm::vec3(-2.0f, 1.0f, 0.0f));
Camera lightEye(glm::vec3(-2.0f, 1.0f, 0.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

//white light properties -- this is global for the scene
float globalAmbient[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
float lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

//variables used in display() for transfering light to shaders
float curAmb[4], curDif[4], curSpe[4], matAmb[4], matDif[4], matSpe[4];
float curShi, matShi;

//shadow-related variables
int screenSizeX, screenSizeY;
GLuint shadowTex, shadowBuffer;
glm::mat4 lightVmatrix, lightPmatrix, shadowMVP1, shadowMVP2, b;

//function prototypes
void setupVertices(void);
void setupShadowBuffers(GLFWwindow* window);
void init(GLFWwindow* window);
void display(GLFWwindow* window, double currentTime);
void displayPreShadow(double currentTime);
void displayPostShadow(double currentTime);
void window_reshape_callback(GLFWwindow* window, int newWidth, int newHeight);
int main(void);
void setupLightAndMaterials(glm::mat4 vMatrix, float* matAmb, float* matDif, float* matSpe, float shi);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
int bindProceduralObject(vector<int> ind, vector<glm::vec3> vert, vector<glm::vec2> tex, vector<glm::vec3> norm, int numVertices, int vboIndex);

void setupVertices(void) {
    //create vao and vbos needed
    glGenVertexArrays(1, vao); // create 1 vao
    glBindVertexArray(vao[0]); // bind one vao (make as active)
    glGenBuffers(numVBOs, vbo); // create vbo to corresponding vao -- can segment vbo array to corresponding vao allocation


    vector<int> ind = mySphere.getIndices();
    vector<glm::vec3> vert = mySphere.getVertices();
    vector<glm::vec2> tex = mySphere.getTexCoords();
    vector<glm::vec3> norm = mySphere.getNormals();
    int numVertices = mySphere.getNumVertices();
    int vboIndex = 0;
    int vaoIndex = 0;
    // this only uses 4 vbos
    vboIndex = bindProceduralObject(ind, vert, tex, norm, numVertices, vboIndex);
    

    ind = myTorus.getIndices();
    vert = myTorus.getVertices();
    tex = myTorus.getTexCoords();
    norm = myTorus.getNormals();
    numVertices = myTorus.getNumVertices();
    // this only uses 4 vbos
    vboIndex = bindProceduralObject(ind, vert, tex, norm, numVertices, vboIndex);
    
}

void setupShadowBuffers(GLFWwindow* window) {
    glfwGetFramebufferSize(window, &width, &height);
    screenSizeX = width;
    screenSizeY = height;

    //create the custom frame buffer
    glGenFramebuffers(1, &shadowBuffer);

    // create the shadow texture and configure it to hold depth information
    glGenTextures(1, &shadowTex);
    glBindTexture(GL_TEXTURE_2D, shadowTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, screenSizeX, screenSizeY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

}
int bindProceduralObject(vector<int> ind, vector<glm::vec3> vert, vector<glm::vec2> tex, vector<glm::vec3> norm, int numVertices, int vboIndex) {
    vector<float> pvalues; //vertex positions
    vector<float> tvalues; //texture coordinates
    vector<float> nvalues; //normal vectors

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

    glBindBuffer(GL_ARRAY_BUFFER, vbo[vboIndex++]);
    glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[vboIndex++]);
    glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 4, &tvalues[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[vboIndex++]);
    glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[vboIndex++]); // indices
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * 4, &ind[0], GL_STATIC_DRAW);

    return vboIndex;
}

void init(GLFWwindow* window) {

    renderingProgram = Utils::createShaderProgram("./res/shaders/lighting.vert", "./res/shaders/lighting.frag");
    shadowProgram = Utils::createShaderProgram("./res/shaders/shadow.vert", "./res/shaders/shadow.frag");
    setupVertices();
    setupShadowBuffers(window);
    b = glm::mat4(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f
    );
    glfwGetFramebufferSize(window, &width, &height);
    aspect = (float)width / (float)height;
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // 1.0672 radians = 60 degrees
    currentLightPos = lightEye.GetPosition(); //<-- if you want a static position light position
    brickTexture = Utils::loadTexture("./res/images/brick1.jpg");
    whiteTexture = Utils::loadTexture("./res/images/white.jpg");
}


void display(GLFWwindow* window, double currentTime) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    //currentLightPos = camera.GetPosition(); // this simulates our camera as a "light" source
    // set up view and perspective matrix from the light point of view for displayPreShadow
    // lightVmatrix = glm::lookAt(currentLightPos, origin, up);
    lightVmatrix = glm::lookAt(currentLightPos, glm::vec3(1.0f, 1.0f, 1.0f), lightEye.GetUp()); // might need to double check this
    lightPmatrix = glm::perspective(Utils::toRadians(60.0f), aspect, 0.1f, 1000.0f);

    // make the custom frame buffer current, and associate it with the shadow texture
    glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTex, 0);

    //disable drawing color, but enable the depth computation
    glDrawBuffer(GL_NONE);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);
    displayPreShadow(currentTime); // first pass

    glDisable(GL_POLYGON_OFFSET_FILL);
    //restore the default display buffer and re-enable drawing
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE1); // binding 1 in shader
    glBindTexture(GL_TEXTURE_2D, shadowTex);
    glDrawBuffer(GL_FRONT);    // re-enables drawing colors

    displayPostShadow(currentTime); // second pass
}

void displayPreShadow(double currentTime) {
    glUseProgram(shadowProgram);
    sLoc = glGetUniformLocation(shadowProgram, "shadowMVP");

    //drawing sphere
    mMat = Utils::buildScale(0.1f, 0.1f, 0.1f);
    mMat *= Utils::buildTranslate(-5.0f, 1.0f, 3.0f);
    shadowMVP1 = lightPmatrix * lightVmatrix * mMat;
    glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));
    // we only need to set up sphere vertices buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
    glDrawElements(GL_TRIANGLES, mySphere.getNumIndices(), GL_UNSIGNED_INT, 0);

    // drawing torus one

    mMat = Utils::buildTranslate(-0.5f, -0.5f, 0.0f);
    mMat *= Utils::buildRotateZ(-0.7f);

    // we are drawing from the light's point of view, so we use the light's P and V matrices
    shadowMVP1 = lightPmatrix * lightVmatrix * mMat;
    glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

    // we only need to set up torus vertices buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    // vbo[7] contains indices. We're just drawing that
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[7]);
    glDrawElements(GL_TRIANGLES, myTorus.getNumIndices(), GL_UNSIGNED_INT, 0);


    // drawing torus two
    mMat = Utils::buildTranslate(1.0f, -0.5f, 0.0f);
    shadowMVP1 = lightPmatrix * lightVmatrix * mMat;
    glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[7]);
    glDrawElements(GL_TRIANGLES, myTorus.getNumIndices(), GL_UNSIGNED_INT, 0);
}
void displayPostShadow(double currentTime) {
    glUseProgram(renderingProgram);

    // get the uniform variables for MV and projection matrices
    mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
    projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
    nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");
    sLoc = glGetUniformLocation(renderingProgram, "shadowMVP");

    //vMat = Utils::buildCameraLocation(cameraVec, cameraRotU, cameraRotV, cameraRotN);
    vMat = camera.GetViewMatrix();

    // operations for object in scene build into stack
    mvStack.push(vMat);
    mvStack.push(mvStack.top());
    mMat = Utils::buildScale(0.1f, 0.1f, 0.1f);
    mMat *= Utils::buildTranslate(-5.0f, 1.0f, 3.0f);
    shadowMVP2 = b * lightPmatrix * lightVmatrix * mMat;
    mvStack.top() *= mMat;

    invTrMat = glm::transpose(glm::inverse(mvStack.top()));

    setupLightAndMaterials(vMat, Utils::bronzeAmbient(), Utils::bronzeDiffuse(), Utils::bronzeSpecular(), Utils::bronzeShininess());
    //glBindVertexArray(vao[0]); // bind whatever vao first
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
    glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // the 2 specifies "coordinates" in vbo. 2 is needed for textures
    glEnableVertexAttribArray(1);

    //specify texture
    glActiveTexture(GL_TEXTURE0); // layout (binding = 0) uniform sampler2D samp ---- could send incremently (GL_TEXTURE0 + 1)
    glBindTexture(GL_TEXTURE_2D, whiteTexture);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);


    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
    glDrawElements(GL_TRIANGLES, mySphere.getNumIndices(), GL_UNSIGNED_INT, 0);
    mvStack.pop();

    // -------------------------- building draw matrix for torus 1-----------------
    mvStack.push(mvStack.top());
    mMat = Utils::buildTranslate(-0.5f, -0.5f, 0.0f);
    mMat *= Utils::buildRotateZ(-0.7f);
    shadowMVP2 = b * lightPmatrix * lightVmatrix * mMat;
    mvStack.top() *= mMat;

    invTrMat = glm::transpose(glm::inverse(mvStack.top()));

    // this handles material and lighting uniforms only
    setupLightAndMaterials(vMat, Utils::silverAmbient(), Utils::silverDiffuse(), Utils::silverSpecular(), Utils::silverShininess());

    
    //glBindVertexArray(vao[1]); // bind whatever vao first
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
    glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    //bind the related texture immediately after
    glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    //specify texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, brickTexture);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[7]);
    glDrawElements(GL_TRIANGLES, myTorus.getNumIndices(), GL_UNSIGNED_INT, 0);

    // -------------------------- building draw matrix for torus 2---------------------
    mvStack.pop();

    mvStack.push(mvStack.top());
    mMat = Utils::buildTranslate(1.0f, -0.5f, 0.0f);
    shadowMVP2 = b * lightPmatrix * lightVmatrix * mMat;
    mvStack.top() *= mMat;

    invTrMat = glm::transpose(glm::inverse(mvStack.top()));

    // this handles material and lighting uniforms only
    setupLightAndMaterials(vMat, Utils::goldAmbient(), Utils::goldDiffuse(), Utils::goldSpecular(), Utils::goldShininess());
    

    //glBindVertexArray(vao[1]); // bind whatever vao first
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
    glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    //bind the related texture immediately after
    glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    //specify texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, brickTexture);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[7]);
    glDrawElements(GL_TRIANGLES, myTorus.getNumIndices(), GL_UNSIGNED_INT, 0);


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

    // mouse and keys configurations
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glewInit() != GLEW_OK) {
        exit(EXIT_FAILURE);
    }
    glfwSwapInterval(1);
    glfwSetWindowSizeCallback(window, window_reshape_callback);

    init(window); // glew must be initialized before init

    while (!glfwWindowShouldClose(window)) { // game loop
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();
        display(window, currentFrame);
        glfwSwapBuffers(window); // GLFW windows are double-buffered by default
        
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void setupLightAndMaterials(glm::mat4 vMatrix, float* matAmb, float* matDif, float* matSpe, float matShi) {
    // this allows material to be specified -- could change to struct
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

void DoMovement() {
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }

    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }

    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }

    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            keys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            keys[key] = false;
        }
    }
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos) {
    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

    lastX = xPos;
    lastY = yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}