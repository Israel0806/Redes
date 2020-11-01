#include <glad/glad.h>
#include <GLFW\glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model.h"
#include "shader.h"
#include "camera.h"

#include <iostream>


using namespace std;
using namespace glm;

string mainPath = "D:/UCSP/Semestres/Semestre_VIII/Redes/Segunda_parte/Juego1/src/gradius/";

int canGoDown = 0;
GLFWwindow *window;
unsigned int VBO, VAO;
int screenWidth = 800, screenHeight = 600;

Shader *shader;
mat4 model, view, projection, shipModelMat;

// timing
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// camera settings
bool firstMouse = true;
float lastX = screenWidth / 2, lastY = screenHeight / 2;
float life;

Camera camera (vec3 (0.0f, 0.0f, 15.0f));
Model *shipModel, *mountain1;

vec3 bottomRockPos[]{
	vec3 (-8.f, 0.f, 0.f),
	vec3 (-5.f, 0.f, 0.f),
	vec3 (-2.f, 0.f, 0.f),
	vec3 (1.f, 0.f, 0.f),
	vec3 (4.f, 0.f, 0.f),
	vec3 (7.f, 0.f, 0.f),
	vec3 (10.f, 0.f, 0.f),
	vec3 (13.f, 0.f, 0.f)
};

float bottomRockRotation[]{
	0,
	0,
	30,
	15,
	0,
	90,
	90,
	270
};

int setOpenGLSettings ();
void framebuffer_size_callback (GLFWwindow *window, int w, int h);
void cursor_position_callback (GLFWwindow *window, double xpos, double ypos);
void processInput (GLFWwindow *window, int key, int scancode, int action, int mods);
void mouseButtonCallback (GLFWwindow *window, int button, int action, int mods);
void scrollCallback (GLFWwindow *window, double xOffset, double yOffset);

void bufferConfig (float *vertices, int size) {
	glGenVertexArrays (1, &VAO);
	glGenBuffers (1, &VBO);
	glBindVertexArray (VAO);

	glBindBuffer (GL_ARRAY_BUFFER, VBO);
	glBufferData (GL_ARRAY_BUFFER, size * sizeof (vertices[0]), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof (float), (void *)0);
	glEnableVertexAttribArray (0);
}

bool CheckCollision (vec4 AABB1, vec4 AABB2) // AABB - AABB collision
{
	// collision x-axis?
	bool collisionX = (AABB1.y > AABB2.x) && (AABB1.x < AABB2.y);
	// collision y-axis?
	bool collisionY = (AABB1.w < AABB2.z) && (AABB1.z > AABB2.w);
	// collision only if on both axes
	return collisionX && collisionY;
}

void reduceLife () {
	life -= 5;
	vec4 tmp = shipModelMat * vec4 (1.0f);
	shipModelMat = translate (mat4 (1.0f), vec3 (tmp.x - 0.5f, -2.0f, 1.f));
	if ( life < 0 ) life = 0;
	cout << "The ship has creashed into an asteroid\n";
	cout << "Life left: " << life << "\n";
}

void drawBaseRocks (vec2 offset, mat4 offsetShipModel) {
	for ( int i = 0; i < (sizeof (bottomRockRotation) / sizeof (bottomRockRotation[0])); ++i ) {
		mat4 offsetMountainModel = mat4 (1.0f);
		offsetMountainModel = translate (offsetMountainModel, vec3 (bottomRockPos[i].x + offset.x, bottomRockPos[i].y + offset.y, bottomRockPos[i].z)); // translate it down so it's at the center of the scene
		offsetMountainModel = rotate (offsetMountainModel, radians (bottomRockRotation[i]), vec3 (0.f, 0.f, 1.f));
		offsetMountainModel = scale (offsetMountainModel, vec3 (3.f, 2.f, 3.f));

		if ( CheckCollision (shipModel->AABBWithOffset (projection * view * offsetShipModel), mountain1->AABBWithOffset (projection * view * offsetMountainModel)) ) {
			vec4 tmp = shipModel->AABBWithOffset (projection * view * offsetShipModel);
			cout << "AABBShipppp: " << tmp.x << " " << tmp.y << " " << tmp.z << " " << tmp.w << "\n";
			vec4 tmp2 = mountain1->AABBWithOffset (projection * view * offsetMountainModel);
			cout << "AABBMontain: " << tmp2.x << " " << tmp2.y << " " << tmp2.z << " " << tmp2.w << "\n\n";

			reduceLife ();
		}

		shader->setMat4 ("model", offsetMountainModel);
		mountain1->Draw (shader);
	}
}

void displayWindow (GLFWwindow *window) {
	shipModelMat = mat4 (1.0f);
	while ( !glfwWindowShouldClose (window) ) {

		glClearColor (0.1f, 0.1f, 0.1f, 1.0f);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glBindVertexArray (VAO);

		/// for time stuff
		float time = glfwGetTime ();
		deltaTime = time - lastFrame;
		lastFrame = time;

		/// Object model
		shipModelMat = translate (shipModelMat, vec3 (0.1f, 0.f, 0.f));
		if ( !canGoDown )
			shipModelMat = translate (shipModelMat, vec3 (0.f, -0.05f, 0.f));
		else
			--canGoDown;


		if ( (shipModelMat * vec4 (1.0f)).x >= 160.f ) {
			cout << "Game has ended\n"
				<< "Game has Restarted\n";
			shipModelMat = mat4 (1.0f);
			camera.Position.x = 0.0f;
		}

		camera.Position = vec3 (camera.Position.x + 0.1f, camera.Position.y, camera.Position.z);

		view = camera.GetViewMatrix ();
		projection = perspective (radians (camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

		shader->use ();
		shader->setMat4 ("view", view);
		shader->setMat4 ("projection", projection);

		// model for space ship
		model = mat4 (1.0f);
		model = translate (shipModelMat, vec3 (-7.0f, 3.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = rotate (model, radians (90.f), vec3 (0.f, 1.f, 0.f));
		model = scale (model, vec3 (0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
		shader->setMat4 ("model", model);
		shipModel->Draw (shader);

		// draw bottom layer
		drawBaseRocks (vec2 (0.f, -7.f), model);
		drawBaseRocks (vec2 (20.f, -7.f), model);
		drawBaseRocks (vec2 (40.f, -7.f), model);
		drawBaseRocks (vec2 (60.f, -7.f), model);
		drawBaseRocks (vec2 (80.f, -7.f), model);
		drawBaseRocks (vec2 (100.f, -7.f), model);
		drawBaseRocks (vec2 (120.f, -7.f), model);
		drawBaseRocks (vec2 (140.f, -7.f), model);

		// draw upper layer
		drawBaseRocks (vec2 (0.f, 7.f), model);
		drawBaseRocks (vec2 (20.f, 7.f), model);
		drawBaseRocks (vec2 (40.f, 7.f), model);
		drawBaseRocks (vec2 (60.f, 7.f), model);
		drawBaseRocks (vec2 (80.f, 7.f), model);
		drawBaseRocks (vec2 (100.f, 7.f), model);
		drawBaseRocks (vec2 (120.f, 7.f), model);
		drawBaseRocks (vec2 (140.f, 7.f), model);

		glfwSwapBuffers (window); // swap buffers
		glfwPollEvents (); // checks if any events are triggered
	}
}


int main () {
	life = 100;
	if ( setOpenGLSettings () == -1 )
		return -1;
	shader = new Shader (mainPath + "shaders/shader.vs", mainPath + "shaders/shader.fs");
	string path = mainPath + "textures/ship/ship_destroyer.obj";
	shipModel = new Model (path);
	path = mainPath + "textures/asteroid/rock.obj";
	mountain1 = new Model (path);

	displayWindow (window);

	// ------------------------------------------------------------------------
	glDeleteVertexArrays (1, &VAO);
	glDeleteBuffers (1, &VBO);
	glfwTerminate ();
	delete shader, shipModel;
	return 0;
}

void framebuffer_size_callback (GLFWwindow *window, int w, int h) {
	glViewport (500, 500, w, h);
}

void cursor_position_callback (GLFWwindow *window, double xpos, double ypos) {
	//if ( firstMouse ) // initially set to true
	//{
	//	lastX = xpos;
	//	lastY = ypos;
	//	firstMouse = false;
	//}
	//float xoffset = xpos - lastX;
	//float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
	//lastX = xpos;
	//lastY = ypos;

	//camera.ProcessMouseMovement (xoffset, yoffset);
}

void processInput (GLFWwindow *window, int key, int scancode, int action, int mods) {
	float cameraSpeed = 5.f * deltaTime;
	if ( glfwGetKey (window, GLFW_KEY_ESCAPE) == GLFW_PRESS )
		glfwSetWindowShouldClose (window, true);
	if ( glfwGetKey (window, GLFW_KEY_W) == GLFW_PRESS ) {
		shipModelMat = translate (shipModelMat, vec3 (0.f, 0.3f, 0.f));
		canGoDown = 20;
	}
	if ( glfwGetKey (window, GLFW_KEY_S) == GLFW_PRESS )
		shipModelMat = translate (shipModelMat, vec3 (0.f, -0.2f, 0.f));
	if ( glfwGetKey (window, GLFW_KEY_A) == GLFW_PRESS )
		shipModelMat = translate (shipModelMat, vec3 (-0.2f, 0.f, 0.f));
	if ( glfwGetKey (window, GLFW_KEY_D) == GLFW_PRESS )
		shipModelMat = translate (shipModelMat, vec3 (0.2f, -0.f, 0.f));
}

void mouseButtonCallback (GLFWwindow *window, int button, int action, int mods) {
	if ( button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS )
		cout << "Right button pressed at " << lastX << "," << lastY << endl;

	if ( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS )
		cout << "Left button pressed at " << lastX << "," << lastY << endl;

}

void scrollCallback (GLFWwindow *window, double xOffset, double yOffset) {
	//camera.ProcessMouseScroll (yOffset);
}

int setOpenGLSettings () {
	if ( !glfwInit () )
		return -1;
	// para el manejo de la ventana
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3); // mayor and minor version for openGL 3
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
	// to use core_profile for access to subsets of features 
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	window = glfwCreateWindow (screenWidth, screenHeight, "LearnOpenGL", NULL, NULL);
	if ( !window ) {
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate (); // destroys remaining windows
		return -1;
	}

	// glfwSetCursorPosCallback (window, cursorPositionCallback);
	/*glfwSetCursorEnterCallback (window, cursorEnterCallback);*/
	glfwSetMouseButtonCallback (window, mouseButtonCallback);
	glfwSetScrollCallback (window, scrollCallback);
	glfwSetKeyCallback (window, processInput);
	glfwSetCursorPosCallback (window, cursor_position_callback);

	glfwSetInputMode (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent (window);
	glfwSetFramebufferSizeCallback (window, framebuffer_size_callback);

	/// gkad vaida que version esta permitida para el opengl actual
	// defines the correct function based on which OS we're compiling for
	if ( !gladLoadGLLoader ((GLADloadproc)glfwGetProcAddress) ) {
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}

	glEnable (GL_DEPTH_TEST);
	//glEnable (GL_STENCIL_TEST);
	//glStencilMask (0xFF); // each bit is written to the stencil buffer as is
	//glStencilMask (0x00); // each bit ends up as 0 in the stencil buffer (disabling writes)

}