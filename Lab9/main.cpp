#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>
#include "SkyBox.hpp"


int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

glm::mat4 mercModel;
glm::mat4 mercNormalMatrix;

glm::mat4 ymercModel;
glm::mat4 ymercNormalMatrix;

glm::mat4 heliModel;
glm::mat4 heliNormalMatrix;

glm::mat4 propModel;
glm::mat4 propNormalMatrix;

glm::mat4 rainModel;
glm::mat4 rainNormalMatrix;

glm::mat4 droneModel;
glm::mat4 droneNormalMatrix;

GLuint foginitLoc;

gps::Camera myCamera(
				glm::vec3(-2.187659f, 4.413680f, 25.673971f),
				glm::vec3(0.159823f, 0.027050f, -0.986775f),
				glm::vec3(0.0f, 1.0f, 0.0f));

float cameraSpeed = 0.30f;
bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

gps::Model3D nanosuit;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D heli;
gps::Model3D prop;
gps::Model3D drone;
gps::Model3D yMerc;

gps::SkyBox mySkyBox;

gps::Shader skyboxShader;
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

short int mercVar;
float mercHeight;
GLfloat mercAngle;

short int somethingHeli;
float heliHeight;
GLfloat heliAngle;
GLfloat propAngle;


int pointinit;
glm::vec3 pointLightPosition; 
GLuint pointLightDirection;

float moveForward;
float moveBackward;

bool foginit = false;

GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
	//toggle fog
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		foginit = !foginit;
	}
}
bool mouse = true;

float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch;
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (mouse)
	{
		lastX = xpos;
		lastY = ypos;
		mouse = false;
	}

	float xoffset = lastX - xpos;
	float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
}
bool wireframe = false;
int cam = 0;

void processMovement()
{
	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);		
	}
	if (pressedKeys[GLFW_KEY_K]) {
		cam = 1;
	}
	if (pressedKeys[GLFW_KEY_6]) {
		cam = 0;
	}
	if (cam == 1)
	{
		angleY += 1.0f;
	}
	//toggle wireframe
	if (pressedKeys[GLFW_KEY_V]) {
		
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (pressedKeys[GLFW_KEY_P]) {

		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	if (pressedKeys[GLFW_KEY_B]) {

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (pressedKeys[GLFW_KEY_F]) {
		myCustomShader.useShaderProgram();
		if (!foginit) {
			foginitLoc = glGetUniformLocation(myCustomShader.shaderProgram, "foginit");
			glUniform1i(foginitLoc, foginit);
		}
		else {
			foginitLoc = glGetUniformLocation(myCustomShader.shaderProgram, "foginit");
			glUniform1i(foginitLoc, foginit);
		}
	}
	if (pressedKeys[GLFW_KEY_G]) {
		myCustomShader.useShaderProgram();
		pointinit = 1;
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointinit"), pointinit);
	}
	if (pressedKeys[GLFW_KEY_H]) {
		myCustomShader.useShaderProgram();
		pointinit = 0;
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointinit"), pointinit);
	}
	if (pressedKeys[GLFW_KEY_1]) {
		moveForward += 0.5;
	}
	if (pressedKeys[GLFW_KEY_2]) {
		moveForward -= 0.5;
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	nanosuit.LoadModel("objects/model/cars.obj");
	ground.LoadModel("objects/merc/pozitie.obj");
	yMerc.LoadModel("objects/merc/yellowMerc.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	lightCube.LoadModel("objects/cube/cube.obj");
	heli.LoadModel("objects/Helicopter/heli.obj");
	prop.LoadModel("objects/Helicopter/fly.obj");
	drone.LoadModel("objects/Helicopter/drone1.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();

	std::vector<const GLchar*> faces;
	faces.push_back("skybox/cloudtop_rt.tga");
	faces.push_back("skybox/cloudtop_lf.tga");
	faces.push_back("skybox/cloudtop_up.tga");
	faces.push_back("skybox/cloudtop_dn.tga");
	faces.push_back("skybox/cloudtop_bk.tga");
	faces.push_back("skybox/cloudtop_ft.tga");
	mySkyBox.Load(faces);
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();

	depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
	depthMapShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); 
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	pointLightPosition = glm::vec3(4.8f, 1.77f, -50.75f); 
	pointLightDirection = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightPosition");
	glUniform3fv(pointLightDirection, 1, glm::value_ptr(pointLightPosition));

}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	//generate FBO ID 
	glGenFramebuffers(1, &shadowMapFBO);
	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE); 
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	const GLfloat near_plane = 0.1f, far_plane = 30.0f;

	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * 5.0f * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightProjection = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}


void drawObjects(gps::Shader shader, bool depthPass) {
		
	shader.useShaderProgram();
	
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	nanosuit.Draw(shader);	


	mercVar = mercVar + 2.5;
	mercHeight = 0.1f + 0.3 * glm::sin(1 * 3.1415 * 0.003 * mercVar);
	mercAngle++;
	mercModel = glm::translate(glm::mat4(1.0f), glm::vec3(0, mercHeight, 0));
	//mercModel = glm::rotate(mercModel, glm::radians(0.5f * cubeAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(mercModel));
	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		mercNormalMatrix = glm::mat3(glm::inverseTranspose(mercModel));
		glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(mercNormalMatrix));
	}
	ground.Draw(shader);


	heliAngle = heliAngle + 0.4;
	float angle = 1.0f * heliAngle;
	float radius = 50.0f;
	//heliModel = glm::rotate(glm::mat4(1.0f), glm::radians(-angle), glm::vec3(0.0f, 1.0f, 0.0f));
	//heliModel = glm::translate(heliModel, glm::vec3(radius, 0.0f, 0.0f));
	//glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(heliModel));
	// do not send the normal matrix if we are rendering in the depth map
	//if (!depthPass) {
	//	heliNormalMatrix = glm::mat3(glm::inverseTranspose(heliModel));
	//	glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(heliNormalMatrix));
	//}
	heli.Draw(shader);


	propAngle = propAngle + 2.0f;
	propModel = glm::rotate(glm::mat4(1.0f), glm::radians(-angle), glm::vec3(0.0f, 1.0f, 0.0f));
	//propModel = glm::translate(propModel, glm::vec3(1.0f, 1.0f, 1.0f));
	//propModel = glm::translate(propModel, glm::vec3(radius, 0.0f, 0.0f));
	//propModel = glm::rotate(glm::mat4(1.0f), glm::radians(-propAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	//propModel = glm::translate(propModel, glm::vec3(radius, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(propModel));
	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		propNormalMatrix = glm::mat3(glm::inverseTranspose(propModel));
		glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(propNormalMatrix));
	}
	prop.Draw(shader);


	droneModel = glm::scale(droneModel, glm::vec3(0.75, 0.75, 0.75));
	droneModel = glm::rotate(glm::mat4(1.0f), glm::radians(-angle), glm::vec3(0.0f, 1.0f, 0.0f));
	droneModel = glm::translate(droneModel, glm::vec3(radius, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(droneModel));
	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		droneNormalMatrix = glm::mat3(glm::inverseTranspose(droneModel));
		glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(droneNormalMatrix));
	}
	drone.Draw(shader);


	ymercModel = glm::translate(glm::mat4(1.0f), glm::vec3(-moveForward, 0, 0));
	ymercModel = glm::translate(ymercModel, glm::vec3(0, mercHeight, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(ymercModel));
	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		ymercNormalMatrix = glm::mat3(glm::inverseTranspose(ymercModel));
		glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(ymercNormalMatrix));
	}
	yMerc.Draw(shader);
}

void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	//render scene -> drawObjects
	drawObjects(depthMapShader, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key
	

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		mySkyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();		

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}
	//to access these values make sure to set them to public in the Camera.hpp
	printf("Camera Position : X: %f Y: %f Z: %f\n", myCamera.cameraPosition.x, myCamera.cameraPosition.y, myCamera.cameraPosition.z);
	printf("Camera Target : X: %f Y: %f Z: %f\n", myCamera.cameraFrontDirection.x, myCamera.cameraFrontDirection.y, myCamera.cameraFrontDirection.z);
	cleanup();

	return 0;
}
