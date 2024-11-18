//
// Display a color cube
//
// Colors are assigned to each vertex and then the rasterizer interpolates
//   those colors across the triangles.  We us an orthographic projection
//   as the default projetion.

#include "cube.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "texture.hpp"
#include "sphere.h"

enum eShadeMode { NO_LIGHT, GOURAUD, PHONG, NUM_LIGHT_MODE };

GLint vPosition;
GLint vNormal;
GLint vTexCoord;
GLint program;

glm::mat4 projectMat;
glm::mat4 viewMat;
glm::mat4 modelMat = glm::mat4(1.0f);


int shadeMode = NO_LIGHT;
int isRotate = false;
int isTexture = false;

float rightUpperArmAngle = 0.0f, 
rightLowerArmAngle = 0.0f,
rightUpperLegAngle = 0.0f,
rightLowerLegAngle = 0.0f,
leftUpperArmAngle = 0.0f,
leftLowerArmAngle = 0.0f,
leftUpperLegAngle = 0.0f,
leftLowerLegAngle = 0.0f;

GLuint pvmMatrixID;

GLuint projectMatrixID;
GLuint viewMatrixID;
GLuint modelMatrixID;
GLuint shadeModeID;
GLuint textureModeID;
GLuint Texture;
GLuint TextureID;
GLuint FaceTexture;

float rotAngle = 0.0f;

typedef glm::vec4  point4;
typedef glm::vec4  normal4;
typedef glm::vec2 texture2;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
normal4 normals[NumVertices];
texture2 textCoords[NumVertices];


texture2 textcoord[4] = {
	texture2(1.0,1.0),
	texture2(1.0,0.0),
	texture2(0.0,0.0),
	texture2(0.0,1.0)
};

// Vertices of a unit cube centered at origin, sides aligned with axes
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

#define NUM_OF_KEYFRAMES 4

float keyFrameAngle[NUM_OF_KEYFRAMES][4] = {
	// { upper arm, lower arm, upper leg, lower leg }
	{ glm::radians(60.0f), glm::radians(-90.0f), glm::radians(30.0f), glm::radians(75.0f)}, // 0
	{ glm::radians(0.0f), glm::radians(-80.0f), glm::radians(0.0f), glm::radians(90.0f) }, // 1
	{ glm::radians(-15.0f), glm::radians(-100.0f), glm::radians(-15.0f), glm::radians(15.0f) }, // 2
	{ glm::radians(60.0f), glm::radians(-80.0f), glm::radians(45.0f), glm::radians(0.0f) }, // 3
};

//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices
int Index = 0;
void
quad(int a, int b, int c, int d)
{
	glm::vec3 edge1 = vertices[b] - vertices[a];
	glm::vec3 edge2 = vertices[c] - vertices[a];
	glm::vec3 normalVector = glm::cross(edge1, edge2);

	glm::vec4 normal = glm::vec4(glm::normalize(normalVector), 1.0f);

	normals[Index] = normal; points[Index] = vertices[a]; textCoords[Index] = textcoord[3];
	Index++;

	normals[Index] = normal; points[Index] = vertices[b]; textCoords[Index] = textcoord[0];
	Index++;

	normals[Index] = normal; points[Index] = vertices[c]; textCoords[Index] = textcoord[1];
	Index++;

	normals[Index] = normal; points[Index] = vertices[a]; textCoords[Index] = textcoord[3];
	Index++;

	normals[Index] = normal; points[Index] = vertices[c]; textCoords[Index] = textcoord[1];
	Index++;

	normals[Index] = normal; points[Index] = vertices[d]; textCoords[Index] = textcoord[2];
	Index++;
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
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

// OpenGL initialization
void
init()
{
	colorcube();

	// Create a vertex array object
	GLuint vao[1];
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);

	// Create and initialize a buffer object
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

	//---------------------------------------
	// 
	// Load the texture
	Texture = loadBMP_custom("textureFile/face.bmp");
	FaceTexture = loadBMP_custom("textureFile/brick.bmp");

	// Get a handle for our "myTextureSampler" uniform
	TextureID = glGetUniformLocation(program, "sphereTexture");

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	// Set our "myTextureSampler" sampler to use Texture Unit 0
	glUniform1i(TextureID, 0);
	//---------------------------------------

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

//----------------------------------------------------------------------------

#define armLegNum 4

void drawCubeMan(glm::mat4 worldMat) {

	int cvertSize = sizeof(points) * 16;
	int cnormalSize = sizeof(normals) * 16;
	int ctexSize = sizeof(textCoords) * 8;

	int vertSize = sizeof(sphere.verts[0]) * sphere.verts.size();
	int normalSize = sizeof(sphere.normals[0]) * sphere.normals.size();
	int texSize = sizeof(sphere.texCoords[0]) * sphere.texCoords.size();

	glm::mat4 bodyCenterMat, modelMat, pvmMat;
	
	glm::vec3 upperPos[armLegNum];

	upperPos[0] = glm::vec3(-0.25, 0.3, 0); // right arm
	upperPos[1] = glm::vec3(0.25, 0.3, 0); // left arm
	upperPos[2] = glm::vec3(-0.1, -0.7, 0); // right leg
	upperPos[3] = glm::vec3(0.1, -0.7, 0); // left leg

	// 몸통 (Body)
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(cvertSize));
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(cvertSize + cnormalSize));

	bodyCenterMat = glm::translate(worldMat, glm::vec3(0, 0, 0));
	modelMat = glm::scale(bodyCenterMat, glm::vec3(0.4, 1.0, 0.2));
	pvmMat = projectMat * viewMat * modelMat;
	
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
	glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
	glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);

	glBindTexture(GL_TEXTURE_2D, Texture);	// Bind Our Texture
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	
	
	// 팔, 다리들
	int isRight, isArm;
	for (int i = 0; i < armLegNum; i++) {
		// 짝수: 오른쪽, 홀수: 왼쪽 그리기
		if (i % 2 == 0) isRight = 1;
		else isRight = -1;

		// 0~1: 팔, 2~3: 다리 그리기
		if (i < 2) isArm = 1;
		else isArm = -1;

		// 짝수: 오른쪽, 홀수: 왼쪽 팔다리 각도 설정
		float upperArmAngle = isRight==1 ? rightUpperArmAngle : leftUpperArmAngle;
		float lowerArmAngle = isRight==1 ? rightLowerArmAngle : leftLowerArmAngle;
		float upperLegAngle = isRight==1 ? rightUpperLegAngle : leftUpperLegAngle;
		float lowerLegAngle = isRight==1 ? rightLowerLegAngle : leftLowerLegAngle;

		if (isArm == 1) {	// 팔
			// 윗 팔
			modelMat = glm::translate(bodyCenterMat, upperPos[i]); //P*V*C*T*S*v
			modelMat = glm::rotate(modelMat, upperArmAngle, glm::vec3(1, 0, 0));
			glm::mat4 upperArmMat = modelMat;
			//modelMat == glm::translate(modelMat, glm::vec3(0, -0.14, 0));
			modelMat = glm::scale(modelMat, glm::vec3(0.1, 0.4, 0.1));

			pvmMat = projectMat * viewMat * modelMat;

			glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
			glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
			glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
			glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);

			glDrawArrays(GL_TRIANGLES, 0, NumVertices);
			
			// 아래 팔
			modelMat = glm::translate(upperArmMat, glm::vec3(0, -0.25, 0.15));
			modelMat = glm::rotate(modelMat, lowerArmAngle, glm::vec3(1, 0, 0));
			modelMat = glm::scale(modelMat, glm::vec3(0.1, 0.4, 0.1));
			pvmMat = projectMat * viewMat * modelMat;

			glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
			glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
			glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
			glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);

			glDrawArrays(GL_TRIANGLES, 0, NumVertices);
		}
		else {	// 다리
			// 윗 다리
			modelMat = glm::translate(bodyCenterMat, upperPos[i]); //P*V*C*T*S*v
			modelMat = glm::rotate(modelMat, upperLegAngle, glm::vec3(1, 0, 0));
			glm::mat4 upperLegMat = modelMat;
			//modelMat = glm::translate(modelMat, glm::vec3(0, -0.2, 0));
			modelMat = glm::scale(modelMat, glm::vec3(0.1, 0.4, 0.1));

			pvmMat = projectMat * viewMat * modelMat;
			glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
			glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
			glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
			glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);
			
			glDrawArrays(GL_TRIANGLES, 0, NumVertices);

			// 아래 다리
			modelMat = glm::translate(upperLegMat, glm::vec3(0, -0.27, -0.05));
			modelMat = glm::rotate(modelMat, lowerLegAngle, glm::vec3(1, 0, 0));
			modelMat = glm::scale(modelMat, glm::vec3(0.1, 0.4, 0.1));
			pvmMat = projectMat * viewMat * modelMat;

			glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
			glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
			glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
			glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);

			glDrawArrays(GL_TRIANGLES, 0, NumVertices);	
		}
	}

	// 머리 (Head) - 몸통 위에 위치
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(cvertSize + cnormalSize + ctexSize));
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(cvertSize + cnormalSize + ctexSize + vertSize));
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(cvertSize + cnormalSize + ctexSize + vertSize + normalSize));

	modelMat = glm::translate(bodyCenterMat, glm::vec3(0, 0.7, 0));	// P*V*C*T*S*v
	modelMat = glm::scale(modelMat, glm::vec3(0.2, 0.2, 0.2));

	pvmMat = projectMat * viewMat * modelMat;

	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
	glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
	glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);

	glBindTexture(GL_TEXTURE_2D, FaceTexture);	// Bind Face Texture

	glDrawArrays(GL_TRIANGLES, NumVertices, sphere.verts.size());
}



void display(void)
{
	glm::mat4 worldMat, pvmMat;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	worldMat = glm::rotate(glm::mat4(1.0f), rotAngle, glm::vec3(1.0f, 1.0f, 0.0f));

	drawCubeMan(worldMat);

	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void idle()
{
	static int prevTime = glutGet(GLUT_ELAPSED_TIME);
	int currTime = glutGet(GLUT_ELAPSED_TIME);

	float deltaTime = (currTime - prevTime) / 1000.0f;

	prevTime = currTime;
	

	float cycleDuration = 2.0f;
	float elapsedTime = fmod(currTime / 1000.0f, cycleDuration);
	float keyFrameProgress = elapsedTime / cycleDuration;
	int currentKeyFrame = static_cast<int>(keyFrameProgress * NUM_OF_KEYFRAMES);
	int nextKeyFrame = (currentKeyFrame + 1) % NUM_OF_KEYFRAMES;

	float t = (keyFrameProgress * NUM_OF_KEYFRAMES) - currentKeyFrame;

	rightUpperArmAngle = glm::mix(keyFrameAngle[currentKeyFrame][0], keyFrameAngle[nextKeyFrame][0], t);
	rightLowerArmAngle = glm::mix(keyFrameAngle[currentKeyFrame][1], keyFrameAngle[nextKeyFrame][1], t);
	rightUpperLegAngle = glm::mix(keyFrameAngle[currentKeyFrame][2], keyFrameAngle[nextKeyFrame][2], t);
	rightLowerLegAngle = glm::mix(keyFrameAngle[currentKeyFrame][3], keyFrameAngle[nextKeyFrame][3], t);

	currentKeyFrame = (currentKeyFrame + 2) % NUM_OF_KEYFRAMES;
	nextKeyFrame = (nextKeyFrame + 2) % NUM_OF_KEYFRAMES;

	leftUpperArmAngle = glm::mix(keyFrameAngle[currentKeyFrame][0], keyFrameAngle[nextKeyFrame][0], t);
	leftLowerArmAngle = glm::mix(keyFrameAngle[currentKeyFrame][1], keyFrameAngle[nextKeyFrame][1], t);
	leftUpperLegAngle = glm::mix(keyFrameAngle[currentKeyFrame][2], keyFrameAngle[nextKeyFrame][2], t);
	leftLowerLegAngle = glm::mix(keyFrameAngle[currentKeyFrame][3], keyFrameAngle[nextKeyFrame][3], t);

	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{

	switch (key) {
	case '1': // Side view
		viewMat = glm::lookAt(glm::vec3(3, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		break;
	case '2': // Over-the-shoulder view
		viewMat = glm::lookAt(glm::vec3(0.5, 1, -2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		break;
	case '3': // Front view
		viewMat = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		break;
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
	case 'q': case 'Q': case 033:  // 'q' or ESC
		exit(EXIT_SUCCESS);
		break;
	}

	glutPostRedisplay();

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
	glutCreateWindow("Texture Cube Man");

	glewInit();

	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(resize);
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}
