#include "cube.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "texture.hpp"
#include "sphere.h"

// shade mode
enum eShadeMode { NO_LIGHT, GOURAUD, PHONG, NUM_LIGHT_MODE };

GLuint vPosition;
GLuint vNormal;
GLuint vTexCoord;
GLuint program;

glm::mat4 projectMat;
glm::mat4 viewMat;
glm::mat4 modelMat = glm::mat4(1.0f);

int shadeMode = NO_LIGHT;
int isRotate = false;
int isTexture = false;

GLuint projectMatrixID;
GLuint viewMatrixID;
GLuint modelMatrixID;
GLuint shadeModeID;
GLuint textureModeID;
GLuint Texture;
GLuint TextureID;
GLuint FaceTexture;
GLuint BodyTexture;
GLuint FrontBodyTexture;

float rotAngle = 0.0f;
int isDrawingCar = true;

GLuint pvmMatrixID;

float upperLeftArmAngle = 0.0f;
float lowerLeftArmAngle = 0.0f;
float upperLeftLegAngle = 0.0f;
float lowerLeftLegAngle = 0.0f;
float upperRightArmAngle = 0.0f;
float lowerRightArmAngle = 0.0f;
float upperRightLegAngle = 0.0f;
float lowerRightLegAngle = 0.0f;
float jump = 1.0f;
float interpolation = 0.0f;
int currentPosition = 0;

typedef glm::vec4  point4;
typedef glm::vec4  normal4;
typedef glm::vec2 texture2;

int view = 0;
const int NumVertices = 36;

point4 points[NumVertices];
normal4 normals[NumVertices];
texture2 textCoords[NumVertices];

texture2 textcoord[4] = {
        texture2(0.0,1.0),
        texture2(0.0,0.0),
        texture2(1.0,0.0),
        texture2(1.0,1.0)
};

point4 vertices[8] = {
   point4(-0.5, -0.5, 0.5, 1.0),
   point4(-0.5, 0.5, 0.5, 1.0),
   point4(0.5, 0.5, 0.5, 1.0),
   point4(0.5, -0.5, 0.5, 1.0),
   point4(-0.5, -0.5, -0.5, 1.0),
   point4(-0.5, 0.5, -0.5, 1.0),
   point4(0.5, 0.5, -0.5, 1.0),
   point4(0.5, -0.5, -0.5, 1.0)
};


Sphere sphere(50, 50);
//----------------------------------------------------------------------------

int Index = 0;
void
quad(int a, int b, int c, int d)
{
    glm::vec3 edge1 = vertices[b] - vertices[a];
    glm::vec3 edge2 = vertices[c] - vertices[a];
    glm::vec3 normalv = glm::cross(edge1, edge2);

    //direction
    glm::vec4 normal = glm::vec4(glm::normalize(normalv), 1.0f);

    normals[Index] = normal; points[Index] = vertices[a]; textCoords[Index] = textcoord[3]; Index++;
    normals[Index] = normal; points[Index] = vertices[b]; textCoords[Index] = textcoord[0]; Index++;
    normals[Index] = normal; points[Index] = vertices[c]; textCoords[Index] = textcoord[1]; Index++;
    normals[Index] = normal; points[Index] = vertices[a]; textCoords[Index] = textcoord[3]; Index++;
    normals[Index] = normal; points[Index] = vertices[c]; textCoords[Index] = textcoord[1]; Index++;
    normals[Index] = normal; points[Index] = vertices[d]; textCoords[Index] = textcoord[2]; Index++;
}

//----------------------------------------------------------------------------

void
colorcube()
{
    quad(1, 0, 3, 2);
    quad(2, 3, 7, 6);
    quad(3, 0, 4, 7);
    quad(6, 5, 1, 2);
    quad(4, 5, 6, 7);
    quad(5, 4, 0, 1);
}

//----------------------------------------------------------------------------

void
init()
{
    colorcube();

    GLuint vao[1];
    glGenVertexArrays(1, vao);
    glBindVertexArray(vao[0]);

    GLuint buffer[1];
    glGenBuffers(1, buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);

    // sphere
    int vertSize = sizeof(sphere.verts[0]) * sphere.verts.size();
    int normalSize = sizeof(sphere.normals[0]) * sphere.normals.size();
    int texSize = sizeof(sphere.texCoords[0]) * sphere.texCoords.size();

    //cube
    int cvertSize = sizeof(points) * 16;
    int cnormalSize = sizeof(normals) * 16;
    int ctexSize = sizeof(textCoords) * 8;


    glBufferData(GL_ARRAY_BUFFER, cvertSize + cnormalSize + ctexSize + vertSize + normalSize + texSize,
        NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, cvertSize, points);
    glBufferSubData(GL_ARRAY_BUFFER, cvertSize, cnormalSize, normals);
    glBufferSubData(GL_ARRAY_BUFFER, cvertSize + cnormalSize, ctexSize, textCoords);
    glBufferSubData(GL_ARRAY_BUFFER, cvertSize + cnormalSize + ctexSize, vertSize, sphere.verts.data());
    glBufferSubData(GL_ARRAY_BUFFER, cvertSize + cnormalSize + ctexSize + vertSize, normalSize, sphere.normals.data());
    glBufferSubData(GL_ARRAY_BUFFER, cvertSize + cnormalSize + ctexSize + vertSize + normalSize, texSize, sphere.texCoords.data());


    // Load shaders and use the resulting shader program
    program = InitShader("src/vshader.glsl", "src/fshader.glsl");
    glUseProgram(program);

    // set up vertex arrays
    vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);

    vNormal = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(vNormal);

    vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);

    projectMatrixID = glGetUniformLocation(program, "mProject");
    projectMat = glm::perspective(glm::radians(65.0f), 1.0f, 0.1f, 100.0f);
    glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);

    viewMatrixID = glGetUniformLocation(program, "mView");
    viewMat = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);

    modelMatrixID = glGetUniformLocation(program, "mModel");
    modelMat = glm::mat4(1.0f);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);

    shadeModeID = glGetUniformLocation(program, "shadeMode");
    glUniform1i(shadeModeID, shadeMode);

    textureModeID = glGetUniformLocation(program, "isTexture");
    glUniform1i(textureModeID, isTexture);
    


    // Load the texture using any two methods
    FaceTexture = loadBMP_custom("face.bmp");
    BodyTexture = loadBMP_custom("body.bmp");
    FrontBodyTexture = loadBMP_custom("frontbody.bmp");

    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID = glGetUniformLocation(program, "sphereTexture");

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);

    // Set our "myTextureSampler" sampler to use Texture Unit 0
    glUniform1i(TextureID, 0);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

//----------------------------------------------------------------------------

void drawPerson(glm::mat4 personMat)
{
    int cvertSize = sizeof(points) * 16;
    int cnormalSize = sizeof(normals) * 16;
    int ctexSize = sizeof(textCoords) * 8;

    int vertSize = sizeof(sphere.verts[0]) * sphere.verts.size();
    int normalSize = sizeof(sphere.normals[0]) * sphere.normals.size();
    int texSize = sizeof(sphere.texCoords[0]) * sphere.texCoords.size();
    glm::mat4 modelMat, pvmMat, bodyMat;

    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));
    glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(cvertSize));
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(cvertSize + cnormalSize));
    // body
    bodyMat = glm::translate(personMat, glm::vec3(0, jump, 0));
    modelMat = glm::scale(bodyMat, glm::vec3(0.5, 1, 0.3));
    pvmMat = projectMat * viewMat * modelMat;

    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
    glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
    glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);
    glBindTexture(GL_TEXTURE_2D, BodyTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDrawArrays(GL_TRIANGLES, 7, 8);
    glDrawArrays(GL_TRIANGLES, 9, 11);
    glBindTexture(GL_TEXTURE_2D, FrontBodyTexture);
    glDrawArrays(GL_TRIANGLES, 12, 36);

    // left upper arm
    modelMat = glm::translate(bodyMat, glm::vec3(0.35, 0.29, 0));
    modelMat = glm::rotate(modelMat, upperLeftArmAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::translate(modelMat, glm::vec3(0, -0.14, 0));
    modelMat = glm::scale(modelMat, glm::vec3(0.2, 0.4, 0.15));
    pvmMat = projectMat * viewMat * modelMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
    glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
    glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);
    glBindTexture(GL_TEXTURE_2D, BodyTexture);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // right upper arm
    modelMat = glm::translate(bodyMat, glm::vec3(-0.35, 0.29, 0));
    modelMat = glm::rotate(modelMat, upperRightArmAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::translate(modelMat, glm::vec3(0, -0.14, 0));
    modelMat = glm::scale(modelMat, glm::vec3(0.2, 0.4, 0.15));
    pvmMat = projectMat * viewMat * modelMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
    glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
    glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);
    glBindTexture(GL_TEXTURE_2D, BodyTexture);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // left lower arm
    modelMat = glm::translate(bodyMat, glm::vec3(0.35, 0.29, 0));
    modelMat = glm::rotate(modelMat, upperLeftArmAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::translate(modelMat, glm::vec3(0, -0.3, 0));

    modelMat = glm::rotate(modelMat, lowerLeftArmAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::translate(modelMat, glm::vec3(0, -0.2, 0));

    modelMat = glm::scale(modelMat, glm::vec3(0.19, 0.4, 0.14));
    pvmMat = projectMat * viewMat * modelMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
    glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
    glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);
    glBindTexture(GL_TEXTURE_2D, BodyTexture);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // right lower arm
    modelMat = glm::translate(bodyMat, glm::vec3(-0.35, 0.29, 0));
    modelMat = glm::rotate(modelMat, upperRightArmAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::translate(modelMat, glm::vec3(0, -0.3, 0));

    modelMat = glm::rotate(modelMat, lowerRightArmAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::translate(modelMat, glm::vec3(0, -0.2, 0));

    modelMat = glm::scale(modelMat, glm::vec3(0.19, 0.4, 0.14));
    pvmMat = projectMat * viewMat * modelMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
    glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
    glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);
    glBindTexture(GL_TEXTURE_2D, BodyTexture);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // left upper leg
    modelMat = glm::translate(bodyMat, glm::vec3(0.14, -0.45, 0));
    modelMat = glm::rotate(modelMat, upperLeftLegAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::translate(modelMat, glm::vec3(0, -0.2, 0));
    modelMat = glm::scale(modelMat, glm::vec3(0.2, 0.4, 0.15));
    pvmMat = projectMat * viewMat * modelMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
    glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
    glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);
    glBindTexture(GL_TEXTURE_2D, BodyTexture);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // right upper leg
    modelMat = glm::translate(bodyMat, glm::vec3(-0.14, -0.45, 0));
    modelMat = glm::rotate(modelMat, upperRightLegAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::translate(modelMat, glm::vec3(0, -0.2, 0));
    modelMat = glm::scale(modelMat, glm::vec3(0.2, 0.4, 0.15));
    pvmMat = projectMat * viewMat * modelMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
    glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
    glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);
    glBindTexture(GL_TEXTURE_2D, BodyTexture);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // left lower leg
    modelMat = glm::translate(bodyMat, glm::vec3(0.14, -0.45, 0));
    modelMat = glm::rotate(modelMat, upperLeftLegAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::translate(modelMat, glm::vec3(0, -0.4, 0));

    modelMat = glm::rotate(modelMat, lowerLeftLegAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::translate(modelMat, glm::vec3(0, -0.2, 0));

    modelMat = glm::scale(modelMat, glm::vec3(0.19, 0.6, 0.14));
    pvmMat = projectMat * viewMat * modelMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
    glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
    glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);
    glBindTexture(GL_TEXTURE_2D, BodyTexture);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    // right lower leg
    modelMat = glm::translate(bodyMat, glm::vec3(-0.14, -0.45, 0));
    modelMat = glm::rotate(modelMat, upperRightLegAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::translate(modelMat, glm::vec3(0, -0.4, 0));

    modelMat = glm::rotate(modelMat, lowerRightLegAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::translate(modelMat, glm::vec3(0, -0.2, 0));

    modelMat = glm::scale(modelMat, glm::vec3(0.19, 0.6, 0.14));
    pvmMat = projectMat * viewMat * modelMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
    glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
    glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);
    glBindTexture(GL_TEXTURE_2D, BodyTexture);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(cvertSize + cnormalSize + ctexSize));
    glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(cvertSize + cnormalSize + ctexSize + vertSize));
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(cvertSize + cnormalSize + ctexSize + vertSize + normalSize));

    // head
    modelMat = glm::translate(bodyMat, glm::vec3(0, 0.6, 0));
    modelMat = glm::scale(modelMat, glm::vec3(0.2, 0.2, 0.2));
    pvmMat = projectMat * viewMat * modelMat;
    glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
    glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
    glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);
    glBindTexture(GL_TEXTURE_2D, FaceTexture);
    glDrawArrays(GL_TRIANGLES, NumVertices, sphere.verts.size());
}


void display(void)
{
    glm::mat4 worldMat, pvmMat;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (view == 0) {
        viewMat = glm::lookAt(glm::vec3(2, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    }
    else if (view == 1) {
        viewMat = glm::lookAt(glm::vec3(0.5, 1, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    }
    else {
        viewMat = glm::lookAt(glm::vec3(0.5, 1, -2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    }
    //worldMat = glm::mat4(1.0f);
    worldMat = glm::rotate(glm::mat4(1.0f), rotAngle, glm::vec3(0.0f, 0.0f, 1.0f));

    drawPerson(worldMat);

    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void idle()
{
    static int prevTime = glutGet(GLUT_ELAPSED_TIME);
    int currTime = glutGet(GLUT_ELAPSED_TIME);
    float upperLeg[8] = { 45.0f, 30.0f, 0.0f, -30.0f, -50.0f, 30.0f, 80.0f, 80.0f };
    float lowerLeg[8] = { 25.0f, 0.0f, -10.0f, -90.0f, -100.0f, -110.0f, -45.0f, 0.0f };
    float upperArm[8] = { -45.0f, 0.0f, 30.0f, 30.0f, 0.0f, -5.0f, -50.0f, -90.0f };
    float lowerArm[8] = { 15.0f, 90.0f, 100.0f, 120.0f, 110.0f, 10.0f, 0.0f, 0.0f };
    float body[8] = { 0.0f, 0.05f, 0.1f, 0.05f, 0.0f, 0.05f, 0.1f, 0.05f };
    if (abs(currTime - prevTime) >= 0.1)
    {
        if (upperLeg[currentPosition] >= upperLeg[(currentPosition + 1) % 8]) {
            upperLeftLegAngle = glm::radians(upperLeg[currentPosition] - ((upperLeg[currentPosition] - upperLeg[(currentPosition + 1) % 8]) / 100 * interpolation));
        }
        else {
            upperLeftLegAngle = glm::radians(upperLeg[currentPosition] + (upperLeg[(currentPosition + 1) % 8] - upperLeg[currentPosition]) / 100 * interpolation);
        }
        if (lowerLeg[currentPosition] >= lowerLeg[(currentPosition + 1) % 8]) {
            lowerLeftLegAngle = glm::radians(lowerLeg[currentPosition] - ((lowerLeg[currentPosition] - lowerLeg[(currentPosition + 1) % 8]) / 100 * interpolation));
        }
        else {
            lowerLeftLegAngle = glm::radians(lowerLeg[currentPosition] + ((lowerLeg[(currentPosition + 1) % 8] - lowerLeg[currentPosition]) / 100 * interpolation));
        }
        if (upperArm[currentPosition] >= upperArm[(currentPosition + 1) % 8]) {
            upperLeftArmAngle = glm::radians(upperArm[currentPosition] - ((upperArm[currentPosition] - upperArm[(currentPosition + 1) % 8]) / 100 * interpolation));
        }
        else {
            upperLeftArmAngle = glm::radians(upperArm[currentPosition] + ((upperArm[(currentPosition + 1) % 8] - upperArm[currentPosition]) / 100 * interpolation));
        }
        if (lowerArm[currentPosition] >= lowerArm[(currentPosition + 1) % 8]) {
            lowerLeftArmAngle = glm::radians(lowerArm[currentPosition] - ((lowerArm[currentPosition] - lowerArm[(currentPosition + 1) % 8]) / 100 * interpolation));
        }
        else {
            lowerLeftArmAngle = glm::radians(lowerArm[currentPosition] + ((lowerArm[(currentPosition + 1) % 8] - lowerArm[currentPosition]) / 100 * interpolation));
        }


        if (upperLeg[(currentPosition + 4) % 8] >= upperLeg[(currentPosition + 5) % 8]) {
            upperRightLegAngle = glm::radians(upperLeg[(currentPosition + 4) % 8] - ((upperLeg[(currentPosition + 4) % 8] - upperLeg[(currentPosition + 5) % 8]) / 100 * interpolation));
        }
        else {
            upperRightLegAngle = glm::radians(upperLeg[(currentPosition + 4) % 8] + ((upperLeg[(currentPosition + 5) % 8] - upperLeg[(currentPosition + 4) % 8])) / 100 * interpolation);
        }
        if (lowerLeg[(currentPosition + 4) % 8] >= lowerLeg[(currentPosition + 5) % 8]) {
            lowerRightLegAngle = glm::radians(lowerLeg[(currentPosition + 4) % 8] - ((lowerLeg[(currentPosition + 4) % 8] - lowerLeg[(currentPosition + 5) % 8]) / 100 * interpolation));
        }
        else {
            lowerRightLegAngle = glm::radians(lowerLeg[(currentPosition + 4) % 8] + ((lowerLeg[(currentPosition + 5) % 8] - lowerLeg[(currentPosition + 4) % 8]) / 100 * interpolation));
        }
        if (upperArm[(currentPosition + 4) % 8] >= upperArm[(currentPosition + 5) % 8]) {
            upperRightArmAngle = glm::radians(upperArm[(currentPosition + 4) % 8] - ((upperArm[(currentPosition + 4) % 8] - upperArm[(currentPosition + 5) % 8]) / 100 * interpolation));
        }
        else {
            upperRightArmAngle = glm::radians(upperArm[(currentPosition + 4) % 8] + ((upperArm[(currentPosition + 5) % 8] - upperArm[(currentPosition + 4) % 8]) / 100 * interpolation));
        }
        if (lowerArm[(currentPosition + 4) % 8] >= lowerArm[(currentPosition + 5) % 8]) {
            lowerRightArmAngle = glm::radians(lowerArm[(currentPosition + 4) % 8] - ((lowerArm[(currentPosition + 4) % 8] - lowerArm[(currentPosition + 5) % 8]) / 100 * interpolation));
        }
        else {
            lowerRightArmAngle = glm::radians(lowerArm[(currentPosition + 4) % 8] + ((lowerArm[(currentPosition + 5) % 8] - lowerArm[(currentPosition + 4) % 8]) / 100 * interpolation));
        }
        if (body[(currentPosition) % 8] >= body[(currentPosition + 1) % 8]) {
            jump = body[(currentPosition) % 8] - ((body[(currentPosition) % 8] - body[(currentPosition + 1) % 8]) / 100 * interpolation);
        }
        else {
            jump = body[(currentPosition) % 8] + ((body[(currentPosition + 1) % 8] - body[(currentPosition) % 8]) / 100 * interpolation);
        }
        lowerLeftLegAngle -= upperLeftLegAngle;
        lowerRightLegAngle -= upperRightLegAngle;

        interpolation++;
        if (interpolation >= 100) {
            interpolation = 0;
            currentPosition = (currentPosition + 1) % 8;
        }
        prevTime = currTime;
        glutPostRedisplay();
    }
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'l': case 'L':
        shadeMode = (++shadeMode % NUM_LIGHT_MODE);
        glUniform1i(shadeModeID, shadeMode);
        glutPostRedisplay();
        break;
    case 't': case 'T':
        isTexture = !isTexture;
        glUniform1i(textureModeID, isTexture);
        glutPostRedisplay();
        break;
    case 'r': case 'R':
        isRotate = !isRotate;
        glutPostRedisplay();
        break;
    case '1':
        view = 0;
        break;
    case '2':
        view = 1;
        break;
    case '3':
        view = 2;
        break;
    case 033:  // Escape key
    case 'q': case 'Q':
        exit(EXIT_SUCCESS);
        break;
    }
}

//----------------------------------------------------------------------------

void resize(int w, int h)
{
    float ratio = (float)w / (float)h;
    glViewport(0, 0, w, h);

    projectMat = glm::perspective(glm::radians(65.0f), ratio, 0.1f, 100.0f);

    glutPostRedisplay();
}

//----------------------------------------------------------------------------

int
main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("Cubeman Running");

    glewInit();
    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(resize);
    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}

