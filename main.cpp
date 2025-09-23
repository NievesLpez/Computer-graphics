/*  Base code by Alun Evans 2016 LaSalle (aevanss@salleurl.edu) modified by: Conrado Ruiz, Ferran Ruiz 2024*/

// student name: Nieves Yashuang Lopez, Lucia Perez

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

using namespace std;
using namespace glm;

//global variables to help us do things
int g_ViewportWidth = 512; int g_ViewportHeight = 512; // Default window size, in pixels
double mouse_x, mouse_y;	//variables storing mouse position
const vec3 g_backgroundColor(0.2f, 0.2f, 0.2f); // background colour - a GLM 3-component vector

GLuint g_simpleShader = 0;	//shader identifier
GLuint g_Vao = 0;			//vao
GLuint g_NumTriangles = 0;	 //  Number of triangles we are painting.

// ------------------------------------------------------------------------------------------
// This function manually creates a square geometry (defined in the array vertices[])
// ------------------------------------------------------------------------------------------
void load()
{
	//**********************
	// CODE TO SET GEOMETRY
	//**********************

	//the positions of the corner of the square, each a 3-component vector, x, y, z
	const GLfloat vertices[] = 
		
		{	
		//Right ear
			0.4f,  0.65f, 0.0f,		// vertex 0
			0.7f,  0.8f, 0.0f,		// vertex 1
		    0.9f,  0.5f, 0.0f,		// vertex 2
		    0.7f,  0.15f, 0.0f,		// vertex 3

		//Left ear
			-0.4f,  0.65f, 0.0f,	// vertex 4
			-0.7f,  0.8f, 0.0f,		// vertex 5
			-0.9f,  0.5f, 0.0f,		// vertex 6
			-0.7f,  0.15f, 0.0f,	// vertex 7

		//Right Upper Head
			 0.0f,  0.7f, 0.0f,		// vertex 8
			 0.4f,  0.65f, 0.0f,	// vertex 9
			 0.2f,  0.2f, 0.0f,		// vertex 10
			 0.0f,  0.2f, 0.0f,		// vertex 11

		//Left Upper Head
			 0.0f,  0.7f, 0.0f,		// vertex 12
			-0.4f,  0.65f, 0.0f,	// vertex 13
			-0.2f,  0.2f, 0.0f,		// vertex 14
			 0.0f,  0.2f, 0.0f, 	// vertex 15

		//Right lateral face
			 0.2f, 0.2f, 0.0f,		// vertex 16
			 0.4f,  0.65f, 0.0f,	// vertex 17
			 0.8f,  -0.02f, 0.0f,	// vertex 18
			 0.65f,  -0.5f, 0.0f,	// vertex 19

		//Left lateral face
			  -0.2f, 0.2f, 0.0f,	// vertex 20
			  -0.4f,  0.65f, 0.0f,	// vertex 21
			  -0.8f,  -0.02f, 0.0f,	// vertex 22
			  -0.65f,  -0.5f, 0.0f,	// vertex 23

		//Right middle face
			  0.0f,  0.2f, 0.0f,	// vertex 24
			  0.2f,  0.2f, 0.0f,	// vertex 25
			  0.22f,  -0.25f, 0.0f,	// vertex 26
			  0.0f, -0.25f, 0.0f,	// vertex 27

		 //Left middle face
			  0.0f,  0.2f, 0.0f,	// vertex 28
			  -0.2f,  0.2f, 0.0f,	// vertex 29
			  -0.22f,  -0.25f, 0.0f,// vertex 30
			  0.0f, -0.25f, 0.0f,	// vertex 31

		//Right eye
			  0.22f,  -0.25f, 0.0f,	// vertex 32
			  0.2f,  0.2f, 0.0f,	// vertex 33
			  0.55f, -0.35f, 0.0f,	// vertex 34
			  0.32f,  -0.4f, 0.0f,	// vertex 35

		//Left eye
			  -0.22f,  -0.25f, 0.0f,// vertex 36
			  -0.2f,  0.2f, 0.0f,	// vertex 37
			  -0.55f, -0.35f, 0.0f,	// vertex 38
			  -0.32f,  -0.4f, 0.0f,	// vertex 39

		};

	//we assign a colour to each corner (each colour is RGB)
	const GLfloat colors[] = 

		{
		//Right ear 
		0.4f, 0.25f, 0.1f,      //Vertex 0 
		0.35f, 0.2f, 0.08f,     //Vertex 1 
		0.3f, 0.15f, 0.06f,     //Vertex 2 
		0.45f, 0.28f, 0.12f,    //Vertex 3 

		//Left ear 
		0.4f, 0.25f, 0.1f,      //Vertex 4 
		0.35f, 0.2f, 0.08f,     //Vertex 5 
		0.3f, 0.15f, 0.06f,     //Vertex 6 
		0.45f, 0.28f, 0.12f,    //Vertex 7 

		//Right Upper Head
		0.6f, 0.4f, 0.2f,       //Vertex 8
		0.65f, 0.45f, 0.25f,    //Vertex 9 
		0.55f, 0.35f, 0.18f,    //Vertex 10 
		0.58f, 0.38f, 0.19f,    //Vertex 11 

		//Left Upper Head 
		0.6f, 0.4f, 0.2f,       //Vertex 12 
		0.65f, 0.45f, 0.25f,    //Vertex 13 
		0.55f, 0.35f, 0.18f,    //Vertex 14 
		0.58f, 0.38f, 0.19f,    //Vertex 15 

		//Right lateral face 
		0.52f, 0.32f, 0.15f,    //Vertex 16 
		0.58f, 0.38f, 0.19f,    //Vertex 17
		0.48f, 0.28f, 0.12f,    //Vertex 18 
		0.45f, 0.25f, 0.1f,     //Vertex 19 

		//Left lateral face 
		0.52f, 0.32f, 0.15f,    //Vertex 20
		0.58f, 0.38f, 0.19f,    //Vertex 21 
		0.48f, 0.28f, 0.12f,    //Vertex 22 
		0.45f, 0.25f, 0.1f,     //Vertex 23 

		//Right middle face
		0.62f, 0.42f, 0.22f,    //Vertex 24 
		0.6f, 0.4f, 0.2f,       //Vertex 25 
		0.58f, 0.38f, 0.19f,    //Vertex 26
		0.6f, 0.4f, 0.2f,       //Vertex 27 

		//Left middle face 
		0.62f, 0.42f, 0.22f,    //Vertex 28 
		0.6f, 0.4f, 0.2f,       //Vertex 29 
		0.58f, 0.38f, 0.19f,    //Vertex 30 
		0.6f, 0.4f, 0.2f,        //Vertex 31

		//Right eye
		0.0f, 0.0f, 0.0f,		//Vertex 28 
		0.0f, 0.0f, 0.0f,       //Vertex 29 
		0.0f, 0.0f, 0.0f,    	//Vertex 30
		0.0f, 0.0f, 0.0f,        //Vertex 31 

		//Leftt eye
		0.0f, 0.0f, 0.0f,		//Vertex 28 
		0.0f, 0.0f, 0.0f,       //Vertex 29 
		0.0f, 0.0f, 0.0f,    	//Vertex 30 
		0.0f, 0.0f, 0.0f        //Vertex 31 
		};

	// The index buffer references the vertices we paint, in order
	//here we have two triangles
	const GLuint indices[] = 
		{	0, 1, 2,				// triangle 0
			0, 2, 3,				// triangle 1

			4, 5, 6,				// triangle 2
			4, 6, 7,				// triangle 3

			8, 9, 10,				// triangle 4
			8, 10, 11,				// triangle 5

			12, 13, 14,				// triangle 6
			12, 14, 15,				// triangle 7
	
			16, 17, 18,				// triangle 8
			16, 18, 19,				// triangle 9
	
			20, 21, 22,				// triangle 10
			20, 22, 23,				// triangle 11
	
			24, 25, 26,				// triangle 12
			24, 26, 27,				// triangle 13
	
			28, 29, 30,				// triangle 14
			28, 30, 31,				// triangle 15
			
			32, 33, 34,				// triangle 16
			32, 34, 35,				// triangle 17

			36, 37, 38,				// triangle 18
			36, 38, 39				// triangle 19
	
		};				


 	//**********************
	// CODE TO LOAD EVERYTHING INTO MEMORY
	//**********************

	//load the shader
	Shader simpleShader("src/shader.vert", "src/shader.frag");
	g_simpleShader = simpleShader.program;

	// Create the VAO where we store all geometry (stored in g_Vao)
	g_Vao = gl_createAndBindVAO();
	std::cout << "vao: " << g_Vao;

	//create vertex buffer for positions, colors, and indices, and bind them to shader
	gl_createAndBindAttribute(vertices, sizeof(vertices), g_simpleShader, "a_vertex", 3);
	gl_createAndBindAttribute(colors, sizeof(colors), g_simpleShader, "a_color", 3);
	gl_createIndexBuffer(indices, sizeof(indices));

	//unbind everything
	gl_unbindVAO();

	//store number of triangles (use in draw())
	g_NumTriangles = sizeof(indices) / (sizeof(GLuint) * 3);
}

// ------------------------------------------------------------------------------------------
// This function actually draws to screen and called non-stop, in a loop
// ------------------------------------------------------------------------------------------
void draw()
{
	//clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// activate shader
	glUseProgram(g_simpleShader);

	//bind the geometry
	gl_bindVAO(g_Vao);

	// Draw to screen
	glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles, GL_UNSIGNED_INT, 0);

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
}

// ------------------------------------------------------------------------------------------
// This function is called every time you click the mouse
// ------------------------------------------------------------------------------------------
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        cout << "Left mouse down at" << mouse_x << ", " << mouse_y << endl;
    }
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
	if (!window) {glfwTerminate();	return -1;}
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();

	//input callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
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


