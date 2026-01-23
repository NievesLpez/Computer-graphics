/*  Base code by Alun Evans 2016 LaSalle (aevanss@salleurl.edu) modified by: Conrado Ruiz, Ferran Ruiz 2024*/

// student name: Lucia Perez y Nieves Yashuang Lopez

//include some standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

//include OpenGL libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//include some custom code files
#include "glfunctions.h"	//include all OpenGL stuff
#include "Shader.h"			// class to compile shaders
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 


//Adding sources 
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


using namespace std;
using namespace glm;

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

const char teapot[] = "assets/teapot.obj";
vector <tinyobj::shape_t> shapeTeapot;

const char bunny[] = "assets/bunny.obj";
vector <tinyobj::shape_t> shapeBunny;

//SKYBOX
const char sphere[] = "assets/sphere.obj";
vector <tinyobj::shape_t> shapes_sky;



//global variables to help us do things
int g_ViewportWidth = 512; int g_ViewportHeight = 512; // Default window size, in pixels
double mouse_x, mouse_y;	//variables storing mouse position
const vec3 g_backgroundColor(0.2f, 0.2f, 0.2f); // background colour - a GLM 3-component vector

//global var transf
mat4 modelTeapot = mat4(1.0f);
float teapot_angulo = 0.0f;
float teapot_angulo02 = 0.0f;
vec3 teapotPos = vec3(0.0f, 0.0f, -2.0f);
mat4 modelBunny = mat4(1.0f);

float bunny_angulo = 0.0f;
float bunny_radio = 0.7f;


bool isFPSCamera = false; // Empezamos en modo ORBITAL para probar
bool isPerspective = true;

// Orbital Camera variables
float orbitalRadius = 1.5f;
float orbitalAngleH = 45.0f;   // Horizontal angle
float orbitalAngleV = 20.0f;  // Vertical angle
vec3 orbitalTarget = vec3(0.0f, 0.0f, -2.0f);

//camera variables 
vec3 cameraPos = vec3(0.0f, 0.0f, 3.0f);
vec3 cameraCenter = vec3(0.0f, 0.0f, -2.0f);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);

float cameraSpeed = 0.1f;	//camara fps

//camera mouse movement variables
float horMov = -120.0;
float vertMov = 0.0f;
float lastMouseX = 256.0f;
float lastMouseY = 256.0f;

vec3 color1 = vec3(0.9, 0.9, 0.2);
vec3 color2 = vec3(0.68, 0.88, 0.91);

//light global var
vec3 g_light_dir(10, 10, 10);

GLuint g_simpleShader = 0;	//shader identifier

//teapot vao
GLuint teapotVao = 0;			//vao
GLuint teapotNumTriangles = 0;	//  Number of triangles we are painting.

//bunny vao
GLuint bunnyVao = 0;			//vao
GLuint bunnyNumTriangles = 0;	//  Number of triangles we are painting.

//texture global variable 
GLuint texture_id;
GLuint texture_id2;

//SKYBOX shader var
GLuint g_simpleShader_sky = 0;		// skybox shader identifier
GLuint g_Vao_sky = 0;				// skybox vao
GLuint g_NumTriangles_sky = 0;		// number of triangles of the skybox
GLuint texture_id_sky;				// global texture id

// Update Orbital Camera position -> new
void updateOrbitalCamera() {
	// Coordenadas esféricas: x = r·cos(v)·sin(h), y = r·sin(v), z = r·cos(v)·cos(h)
	float radH = glm::radians(orbitalAngleH);
	float radV = glm::radians(orbitalAngleV);

	cameraPos.x = orbitalTarget.x + orbitalRadius * cos(radV) * sin(radH);
	cameraPos.y = orbitalTarget.y + orbitalRadius * sin(radV);
	cameraPos.z = orbitalTarget.z + orbitalRadius * cos(radV) * cos(radH);

	cameraCenter = orbitalTarget;
	cameraUp = vec3(0.0f, 1.0f, 0.0f);
}

// ------------------------------------------------------------------------------------------
// This function manually creates a square geometry (defined in the array vertices[])
// ------------------------------------------------------------------------------------------
void load()
{
	//**********************
	// CODE TO SET GEOMETRY
	//**********************
	bool ret;

	//SKYBOX
	bool ret_sky = tinyobj::LoadObj(shapes_sky, sphere);

	if (ret_sky)
		cout << "OBJ File: " << sphere << " sucessfully loaded\n";
	else
		cout << "OBJ File:" << sphere << " cannot be found or is not a valid OBJ\n";

	//Teapot
	ret = tinyobj::LoadObj(shapeTeapot, teapot);

	if (ret)
		cout << "OBJ File: " << teapot << " sucessfully loaded\n";
	else
		cout << "OBJ File:" << teapot << " cannot be found or is not a valid OBJ\n";


	//Bunny
	ret = tinyobj::LoadObj(shapeBunny, bunny);

	if (ret)
		cout << "OBJ File: " << bunny << " sucessfully loaded\n";
	else
		cout << "OBJ File:" << bunny << " cannot be found or is not a valid OBJ\n";


	//the positions of the corner of the square, each a 3-component vector, x, y, z
	const GLfloat vertices[] =
	{ 0.5f, -0.5f, 0.0f,		// vertex 0
		0.5f,  0.5f, 0.0f,		// vertex 1
	   -0.5f,  0.5f, 0.0f,		// vertex 2
	   -0.5f, -0.5f, 0.0f };	// vertex 3

	//we assign a colour to each corner (each colour is RGB)
	const GLfloat colors[] =
	{ 1.0f, 0.0f, 0.0f,			// color of vertex 1 (Red)
		0.f, 1.0f, 0.0f,		// color of vertex 1 (Green)
		0.0f, 0.0f, 1.0f,		// color of vertex 1 (Blue)
		1.0f, 1.0f, 0.0f };		// color of vertex 1 (Yellow)

	// The index buffer references the vertices we paint, in order
	//here we have two triangles
	const GLuint indices[] =
	{ 0, 1, 2,					// triangle 0
		0, 2, 3 };				// triangle 1


	//**********************
	// CODE TO LOAD EVERYTHING INTO MEMORY
	//**********************

	//load the main shader
	Shader simpleShader("src/shader.vert", "src/shader.frag");
	g_simpleShader = simpleShader.program;

	//Load the SKYBOX shader
	Shader simpleShader_sky("src/shader_sky.vert", "src/shader_sky.frag");
	g_simpleShader_sky = simpleShader_sky.program;


	//create vertex buffer for positions, colors, and indices, and bind them to shader

	// Create the VAO where we store all geometry (stored in g_Vao)
	//SKYBOX VAO

	g_Vao_sky = gl_createAndBindVAO();

	//Skybox
	gl_createAndBindAttribute(&(shapes_sky[0].mesh.positions[0]),
		shapes_sky[0].mesh.positions.size() * sizeof(float), g_simpleShader_sky,
		"a_vertex", 3);

	gl_createIndexBuffer(&(shapes_sky[0].mesh.indices[0]),
		shapes_sky[0].mesh.indices.size() * sizeof(unsigned int));

	gl_createAndBindAttribute(&(shapes_sky[0].mesh.texcoords[0]),
		shapes_sky[0].mesh.texcoords.size() * sizeof(GLfloat), g_simpleShader_sky,
		"a_uv", 2);

	gl_unbindVAO();

	g_NumTriangles_sky = shapes_sky[0].mesh.indices.size() / 3;


	//Teapot VAO
	teapotVao = gl_createAndBindVAO();

	//Teapot
	gl_createAndBindAttribute(&(shapeTeapot[0].mesh.positions[0]),
		sizeof(float) * shapeTeapot[0].mesh.positions.size(),
		g_simpleShader, "a_vertex", 3);



	gl_createAndBindAttribute(
		&(shapeTeapot[0].mesh.texcoords[0]),
		shapeTeapot[0].mesh.texcoords.size() * sizeof(GLfloat),
		g_simpleShader,
		"a_uv", 2
	);

	gl_createAndBindAttribute(&(shapeTeapot[0].mesh.normals[0]),
		shapeTeapot[0].mesh.normals.size() * sizeof(float),
		g_simpleShader, "a_normal", 3);


	gl_createIndexBuffer(&(shapeTeapot[0].mesh.indices[0]), sizeof(unsigned int) * shapeTeapot[0].mesh.indices.size());

	//unbind everything
	gl_unbindVAO();

	//store number of triangles (use in draw())

	teapotNumTriangles = shapeTeapot[0].mesh.indices.size() / 3;

	//--------------------------------------------
		//create vertex buffer for positions, colors, and indices, and bind them to shader
	bunnyVao = gl_createAndBindVAO();

	gl_createAndBindAttribute(&(shapeBunny[0].mesh.positions[0]),
		sizeof(float) * shapeBunny[0].mesh.positions.size(),
		g_simpleShader, "a_vertex", 3);


	if (!shapeBunny[0].mesh.texcoords.empty()) {
		gl_createAndBindAttribute(
			&(shapeBunny[0].mesh.texcoords[0]),
			shapeBunny[0].mesh.texcoords.size() * sizeof(GLfloat),
			g_simpleShader,
			"a_uv", 2
		);
	}

	gl_createAndBindAttribute(&(shapeBunny[0].mesh.normals[0]),
		shapeBunny[0].mesh.normals.size() * sizeof(float),
		g_simpleShader, "a_normal", 3);

	gl_createIndexBuffer(&(shapeBunny[0].mesh.indices[0]),
		sizeof(unsigned int) * shapeBunny[0].mesh.indices.size());

	//unbind everything

	gl_unbindVAO();

	//store number of triangles (use in draw())

	bunnyNumTriangles = shapeBunny[0].mesh.indices.size() / 3;


	//load both images with textures 

	char path_sky[] = "textures/sunset.png";
	int width, height, numChannels;

	stbi_set_flip_vertically_on_load(true);		// Remove if texture is flipped.
	unsigned char* pixels_sky = stbi_load(path_sky, &width, &height, &numChannels, 0);

	glGenTextures(1, &texture_id_sky);
	glBindTexture(GL_TEXTURE_2D, texture_id_sky);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,		// target
		0,							// level = 0 base, no mipmap
		GL_RGB,						// how the data will be stored(Grayscale, RGB, RGBA)
		width,						// width of the image
		height,						// height of the image
		0,							//border
		GL_RGBA,					// format of original data
		GL_UNSIGNED_BYTE,			// type of data
		pixels_sky);



	char path[] = "textures/arco_baseColor.jpg";


	stbi_set_flip_vertically_on_load(true);
	unsigned char* pixels = stbi_load(path, &width, &height, &numChannels, 0);

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGB,
		width,
		height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		pixels);


	char path2[] = "textures/mushroom_baseColor.jpg";

	pixels = stbi_load(path2, &width, &height, &numChannels, 0);

	glGenTextures(1, &texture_id2);
	glBindTexture(GL_TEXTURE_2D, texture_id2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGB,
		width,
		height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		pixels);

}

// ------------------------------------------------------------------------------------------
// This function actually draws to screen and called non-stop, in a loop
// ------------------------------------------------------------------------------------------
void draw()
{
	//clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glDisable(GL_DEPTH_TEST);

	// activate shader
	glUseProgram(g_simpleShader_sky);
	

	float tiempo = glfwGetTime(); // para actualizar los angulos del teapot cuando rote
	teapot_angulo = tiempo;
	teapot_angulo02 = tiempo * 10.0f;


	mat4 projection_matrix;
	if (isPerspective) {
		projection_matrix = glm::perspective(
			90.0f,     // Field of view (P04 slide 6)
			1.0f,      // Aspect ratio
			0.1f,      // Near plane
			50.0f      // Far plane
		);
	}
	else {
		// Orthographic projection 
		projection_matrix = glm::ortho(
			-3.0f, 3.0f,   // left, right
			-3.0f, 3.0f,   // bottom, top
			0.1f, 50.0f    // near, far
		);
	}


	// VIEW MATRIX 
	mat4 view_matrix;
	if (isFPSCamera) {
		// FPS Camera 
		view_matrix = glm::lookAt(cameraPos, cameraCenter, cameraUp);
	}
	else {
		// Orbital Camera
		updateOrbitalCamera();
		view_matrix = glm::lookAt(cameraPos, cameraCenter, cameraUp);
	}


	//SKYBOX
	//texture
	GLuint u_texture_sky = glGetUniformLocation(g_simpleShader_sky,
		"u_texture");
	glUniform1i(u_texture_sky, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id_sky);


	GLuint model_loc_sky = glGetUniformLocation(g_simpleShader_sky,
		"u_model");
	GLuint view_loc_sky = glGetUniformLocation(g_simpleShader_sky,
		"u_view");
	GLuint projection_loc_sky = glGetUniformLocation(g_simpleShader_sky,
		"u_projection");
	//set MVP
	mat4 model_matrix_sky = translate(mat4(1.0f), cameraPos); // where the camera is


	//send all values to shader
	glUniformMatrix4fv(model_loc_sky, 1, GL_FALSE, glm::value_ptr(model_matrix_sky));
	glUniformMatrix4fv(view_loc_sky, 1, GL_FALSE, glm::value_ptr(view_matrix));
	glUniformMatrix4fv(projection_loc_sky, 1, GL_FALSE, glm::value_ptr(projection_matrix));


	gl_bindVAO(g_Vao_sky);
	glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles_sky, GL_UNSIGNED_INT, 0);

	//OTHER OBJECTS
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);

	glUseProgram(g_simpleShader);

	GLuint projection_loc = glGetUniformLocation(g_simpleShader, "u_projection");
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

	GLuint model_loc = glGetUniformLocation(g_simpleShader, "u_model"); //get mat

	//camera 
	GLuint view_loc = glGetUniformLocation(g_simpleShader, "u_view");
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));

	//teapot
	gl_bindVAO(teapotVao);

	//texture
	GLuint u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
	glUniform1i(u_texture, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);


	mat4 model1 = translate(mat4(1.0f), vec3(teapotPos.x, teapotPos.y, teapotPos.z));

	//mat4 model1 = translate(view_matrix, vec3 (cameraCenter));
	mat4 model3 = glm::scale(mat4(1.0f), vec3(0.8, 0.8, 0.8));
	mat4 modelTeapotRot = glm::rotate(mat4(1.0f), teapot_angulo, vec3(0.0, 1.0, 0.0));
	mat4 modelTeapotRotUp = glm::rotate(mat4(1.0f), teapot_angulo02, vec3(0.0, 1.0, 0.0));

	mat4 modelTeapotFinal = (model1 * modelTeapotRot * modelTeapotRotUp * model3);

	glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(modelTeapotFinal));

	glDrawElements(GL_TRIANGLES, 3 * teapotNumTriangles, GL_UNSIGNED_INT, 0);

	//bunny
	gl_bindVAO(bunnyVao);

	//texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id2);


	//position
	float angulo = tiempo * 40.0f;	//controle velocity of rotation

	//circular position
	mat4 model2 = model1 * rotate(mat4(1.0f), angulo, vec3(0.0f, 0.4f, 0.0f)) * translate(mat4(1.0f), vec3(1.0f, 0.0f, 0.0f));
	mat4 model4 = glm::scale(mat4(1.0f), vec3(1.3, 1.3, 1.3));


	mat4 modelBunnyFinal = model2 * model4;

	glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(modelBunnyFinal));

	//Lights and textures
	GLuint light_loc = glGetUniformLocation(g_simpleShader, "u_light_dir");
	glUniform3f(light_loc, g_light_dir.x, g_light_dir.y, g_light_dir.z);

	GLuint cam_pos_loc = glGetUniformLocation(g_simpleShader, "u_cam_pos");
	// this is the camera position, eye/cameraPos
	glUniform3f(cam_pos_loc, cameraPos.x, cameraPos.y, cameraPos.z);

	GLuint ambient_loc = glGetUniformLocation(g_simpleShader, "u_ambient");
	glUniform3f(ambient_loc, 0.1, 0.1, 0.1); // grey shadows

	GLuint diffuse_loc = glGetUniformLocation(g_simpleShader, "u_diffuse");
	glUniform3f(diffuse_loc, 1.0, 1.0, 1.0); // white light

	GLuint specular_loc = glGetUniformLocation(g_simpleShader, "u_specular");
	glUniform3f(specular_loc, 1.0, 1.0, 1.0);

	GLuint shininess_loc = glGetUniformLocation(g_simpleShader, "u_shininess");
	glUniform1f(shininess_loc, 20.0);

	glDrawElements(GL_TRIANGLES, 3 * bunnyNumTriangles, GL_UNSIGNED_INT, 0);


}

// ------------------------------------------------------------------------------------------
// This function is called every time you press a screen
// ------------------------------------------------------------------------------------------

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	//quit
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, 1);
	//reload
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		load();

	// Switch FPS/Orbital/Perspective 
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		isPerspective = !isPerspective;
	}
	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		isFPSCamera = false;
	}
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		isFPSCamera = true;
	}

	// ORBITAL CAMERA CONTROLS
	if (!isFPSCamera) {
		// Rotate horizontally (around Y axis)
		if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			orbitalAngleH -= 5.0f;
		}
		if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			orbitalAngleH += 5.0f;
		}

		// Rotate vertically
		if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			orbitalAngleV += 5.0f;
			if (orbitalAngleV > 80.0f) orbitalAngleV = 80.0f;
		}
		if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			orbitalAngleV -= 5.0f;
			if (orbitalAngleV < -80.0f) orbitalAngleV = -80.0f;
		}

		// Zoom with Page Up/Page Down
		if (key == GLFW_KEY_PAGE_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			orbitalRadius -= 0.5f;
			if (orbitalRadius < 1.0f) orbitalRadius = 1.0f;
		}
		if (key == GLFW_KEY_PAGE_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			orbitalRadius += 0.5f;
			if (orbitalRadius > 20.0f) orbitalRadius = 20.0f;
		}
	}

	// FPS CAMERA CONTROLS

	else {
		// Calculate Forward vector 
		// F = normalize(center - eye)
		vec3 F = glm::normalize(cameraCenter - cameraPos);

		// Calculate Side vector 
		// S = F × up
		vec3 S = glm::normalize(glm::cross(F, cameraUp));

		// Adelante
		if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			cameraPos += F * cameraSpeed;
			cameraCenter += F * cameraSpeed;
		}
		// Atrás
		if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			cameraPos -= F * cameraSpeed;
			cameraCenter -= F * cameraSpeed;
		}
		// Izquierda
		if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			cameraPos -= S * cameraSpeed;
			cameraCenter -= S * cameraSpeed;
		}
		// Derecha
		if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			cameraPos += S * cameraSpeed;
			cameraCenter += S * cameraSpeed;
		}
	}
}

// ------------------------------------------------------------------------------------------
// This function is called every time you move the mouse
// ------------------------------------------------------------------------------------------

void mouse_movement_callback(GLFWwindow* window, double xpos, double ypos) {
	if (!isFPSCamera) return;


	//Calculate position
	float xoff = (xpos - lastMouseX) * 0.1f;
	float yoff = (lastMouseY - ypos) * 0.1f;

	//save pos
	lastMouseX = xpos;
	lastMouseY = ypos;

	horMov += xoff;
	vertMov += yoff;

	//Avoid gimbal lock
	if (vertMov > 89.0f) vertMov = 89.0f;
	if (vertMov < -89.0f) vertMov = -89.0f;

	vec3 direction;
	direction.x = cos(glm::radians(horMov)) * cos(glm::radians(vertMov));
	direction.y = sin(glm::radians(vertMov));
	direction.z = sin(glm::radians(horMov)) * cos(glm::radians(vertMov));

	cameraFront = normalize(direction);
	cameraCenter = cameraPos + glm::normalize(direction);
}

int main(void)
{
	//setup window and boring stuff, defined in glfunctions.cpp
	GLFWwindow* window;
	if (!glfwInit())return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(g_ViewportWidth, g_ViewportHeight, "Hello OpenGL!", NULL, NULL);
	if (!window) { glfwTerminate();	return -1; }
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();

	cout << "P = Pasar de la perspectiva a la ortografica \nF/O = Cambiar de FPS a Orbital\n" << endl;

	//input callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_movement_callback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

	glClearColor(g_backgroundColor.x, g_backgroundColor.y, g_backgroundColor.z, 1.0f);

	//load all the resources
	load();

	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{
		draw();

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Poll for and process events
		glfwPollEvents();

		//mouse position must be tracked constantly (callbacks do not give accurate delta)
		glfwGetCursorPos(window, &mouse_x, &mouse_y);
	}

	//terminate glfw and exit
	glfwTerminate();
	return 0;

}