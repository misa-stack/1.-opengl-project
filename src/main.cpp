#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "utils.h"

using namespace std;

GLFWwindow * glfw_window = NULL;
bool keep_running = true;

void glfw_error_callback(int error, const char * description)
{
	cout << "GLFW error " << error << ": " << description << endl;
}

void glfw_key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		keep_running = false;
	}
}

void glfw_mouse_click_callback(GLFWwindow * window, int button, int action, int mods)
{

}

void glfw_mouse_move_callback(GLFWwindow * window, double xpos, double ypos)
{

}

bool createWindow(int width, int height, string title, bool vsync, bool core = true)
{
	if (glfw_window) return false;
	if (!glfwInit()) return false;
	
	glfwSetErrorCallback(glfw_error_callback);

	if(core)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}

	// create window using GLFW
	if (!(glfw_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL)))
	{
		glfwTerminate();
		return false;
	}
	
	cout << "GLFW " << glfwGetVersionString() << endl;
	
	glfwSetKeyCallback(glfw_window, glfw_key_callback);
	glfwSetMouseButtonCallback(glfw_window, glfw_mouse_click_callback);
	glfwSetCursorPosCallback(glfw_window, glfw_mouse_move_callback);
	
	glfwMakeContextCurrent(glfw_window);
	
	if (vsync) glfwSwapInterval(1); // <-- VSYNC on
	
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Shading language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	
	// load OpenGL functions using GLEW
	if (glewInit() != GLEW_OK)
	{
		glfwDestroyWindow(glfw_window);
		glfw_window = NULL;
		glfwTerminate();
	}
	
	cout << "GLEW" << endl;
	
	return true;
}

void init()
{
	/* Barva pozadí */
	glClearColor(0.1, 0.1, 0.15, 1);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1280, 0, 720, -1, 1);
}

void bod(int x, int y, int z)
{
	glVertex2i(x*200/(500-z) +640, y*200/(500-z) +360);
}
/*
 *
 * x=x *200/(400-hloubka)
 * y=y *200(400-hloubka)
 *
 *
 *
 *
*/


void render(double dt)
{
	//glBegin(GL_LINE_STRIP);

	glBegin(GL_LINE_STRIP);
	int r = 200;
	int x = 640;
	int y = 360;
	for(int z = 0;z <r;z++)
	{
		glColor3ub(255, 0, 0);
		glVertex2i(x+z, y);
		glVertex2i(x+z, y-sqrt(r*r-z*z));
		glVertex2i(x-z, y);
		glVertex2i(x-z, y-sqrt(r*r-z*z));
		glVertex2i(x-z, y);
		glVertex2i(x-z, y+sqrt(r*r-z*z));
		glVertex2i(x+z, y);
		glVertex2i(x+z, y+sqrt(r*r-z*z));
		/*
	    cara(x+z,y,x+z,y-sqrt(r*r-z*z));
	    cara(x-z,y,x-z,y-sqrt(r*r-z*z));
	    cara(x-z,y,x-z,y+sqrt(r*r-z*z));
	    cara(x+z,y,x+z,y+sqrt(r*r-z*z));
	    */
	}

	glEnd();


	glBegin(GL_LINE_STRIP);
	glColor3ub(0, 0, 0);

	bod(100,100,100);
	bod(-100,100,100);
	bod(-100,-100,100);
	bod(100,-100,100);
	bod(100,100,100);
	bod(100,100,-100);
	bod(-100,100,-100);
	bod(-100,-100,-100);
	bod(100,-100,-100);
	bod(100,100,-100);
	glEnd();
	glBegin(GL_LINE_STRIP);
	bod(-100,100,100);
	bod(-100,100,-100);
	bod(-100,-100,-100);
	bod(-100,-100,100);
	bod(100,-100,100);
	bod(100,-100,-100);


	glEnd();


}

int main()
{
	if(!createWindow(1280, 720, "OpenGL", true, false)) return -1;
	
	init();
	
	/* Hlavní smyčka */
	double delta = 0;
	double old_time = glfwGetTime();
	double total_time = 0;
	int ticks = 0;
	double max_delta = 0;
	while (keep_running && !glfwWindowShouldClose(glfw_window))
	{
		// process input
		glfwPollEvents();
		
		// render frame
		glClearColor(255,255,255,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render(delta);
		glfwSwapBuffers(glfw_window);
		
		// measure how long it took
		double new_time = glfwGetTime();
		delta = new_time - old_time;
		old_time = new_time;
		
		ticks++;
		total_time += delta;
		
		if(delta > max_delta)
			max_delta = delta;
		
		// write out FPS
		if(total_time > 3.0)
		{
			double fps = ticks / total_time;
			cout << "FPS: " << fps << " (" << (total_time / ticks * 100) << " ms), max " << (max_delta * 100) << " ms" << endl;
			
			total_time = 0;
			ticks = 0;
			max_delta = 0;
		}
	}
	
	glfwDestroyWindow(glfw_window);
	glfw_window = NULL;
	
	glfwTerminate();
	
	cout << "Finished." << endl;
	return 0;
}
