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
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) return false;

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
	GLenum glew_result = glewInit();
#if defined(GLEW_ERROR_NO_GLX_DISPLAY) && defined(GLFW_PLATFORM_WAYLAND)
	// Wayland uses EGL; GLEW has loaded OpenGL even when its GLX setup is unavailable.
	if (glew_result == GLEW_ERROR_NO_GLX_DISPLAY && glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
		glew_result = GLEW_OK;
#endif
	if (glew_result != GLEW_OK)
	{
		cerr << "GLEW initialization failed: " << glewGetErrorString(glew_result) << endl;
		glfwDestroyWindow(glfw_window);
		glfw_window = NULL;
		glfwTerminate();
		return false;
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

double angleX = 0.0;
double angleY = 0.0;
double angleZ = 0.0;
double positionX = 0.0;
double positionY = 0.0;
double scale = 1.0;

void bod(double x, double y, double z)
{

	x *= scale;
	y *= scale;
	z *= scale;


	const double yAfterX = y * cos(angleX) - z * sin(angleX);
	const double zAfterX = y * sin(angleX) + z * cos(angleX);


	const double xAfterY = x * cos(angleY) + zAfterX * sin(angleY);
	const double zAfterY = -x * sin(angleY) + zAfterX * cos(angleY);


	const double rotatedX = xAfterY * cos(angleZ) - yAfterX * sin(angleZ);
	const double rotatedY = xAfterY * sin(angleZ) + yAfterX * cos(angleZ);


	const double translatedX = rotatedX + positionX;
	const double translatedY = rotatedY + positionY;

	glVertex2d(translatedX * 200 / (500 - zAfterY) + 640, translatedY * 200 / (500 - zAfterY) + 360);
}
void cara(int x1, int y1, int z1, int x2, int y2, int z2)
{
	bod(x1, y1, z1);
	bod(x2, y2, z2);
}

void render(double dt)
{
	constexpr double pi = 3.14159265358979323846;
	const double rotationStep = 60.0 * pi / 180.0 * dt;

	const int directionX = (glfwGetKey(glfw_window, GLFW_KEY_UP) == GLFW_PRESS)
		- (glfwGetKey(glfw_window, GLFW_KEY_DOWN) == GLFW_PRESS);
	const int directionY = (glfwGetKey(glfw_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		- (glfwGetKey(glfw_window, GLFW_KEY_LEFT) == GLFW_PRESS);
	const int directionZ = (glfwGetKey(glfw_window, GLFW_KEY_E) == GLFW_PRESS)
		- (glfwGetKey(glfw_window, GLFW_KEY_Q) == GLFW_PRESS);

	angleX = fmod(angleX + directionX * rotationStep, 2.0 * pi);
	angleY = fmod(angleY + directionY * rotationStep, 2.0 * pi);
	angleZ = fmod(angleZ + directionZ * rotationStep, 2.0 * pi);


	const double movementStep = 200.0 * dt;
	const int movementX = (glfwGetKey(glfw_window, GLFW_KEY_D) == GLFW_PRESS)
		- (glfwGetKey(glfw_window, GLFW_KEY_A) == GLFW_PRESS);
	const int movementY = (glfwGetKey(glfw_window, GLFW_KEY_W) == GLFW_PRESS)
		- (glfwGetKey(glfw_window, GLFW_KEY_S) == GLFW_PRESS);
	positionX += movementX * movementStep;
	positionY += movementY * movementStep;


	if (glfwGetKey(glfw_window, GLFW_KEY_1) == GLFW_PRESS)
	{
		scale += 0.01;
	}
	else if (glfwGetKey(glfw_window, GLFW_KEY_2) == GLFW_PRESS)
	{
		scale -= 0.01;
	}






	if (glfwGetKey(glfw_window, GLFW_KEY_R) == GLFW_PRESS)
	{
		angleX = 0.0;
		angleY = 0.0;
		angleZ = 0.0;
		positionX = 0.0;
		positionY = 0.0;
		scale = 1.0;
	}

	int width, height;
	glfwGetFramebufferSize(glfw_window, &width, &height);
	glViewport((width - 1280) / 2, (height - 720) / 2, 1280, 720);

	glColor3ub(0, 0, 0);
	glBegin(GL_LINES);

	// Predni stena: x je sirka, y vyska, z hloubka.
	cara(-250, -200, 175, 250, -200, 175);
	cara(250, -200, 175, 250, 100, 175);
	cara(250, 100, 175, -250, 100, 175);
	cara(-250, 100, 175, -250, -200, 175);

	// Zadni stena.
	cara(-250, -200, -175, 250, -200, -175);
	cara(250, -200, -175, 250, 100, -175);
	cara(250, 100, -175, -250, 100, -175);
	cara(-250, 100, -175, -250, -200, -175);

	// Spojeni predni a zadni steny.
	cara(-250, -200, 175, -250, -200, -175);
	cara(250, -200, 175, 250, -200, -175);
	cara(250, 100, 175, 250, 100, -175);
	cara(-250, 100, 175, -250, 100, -175);

	// Strecha: dva trojuhelniky spojene hrebenem.
	cara(-250, 100, 175, 0, 250, 175);
	cara(0, 250, 175, 250, 100, 175);
	cara(-250, 100, -175, 0, 250, -175);
	cara(0, 250, -175, 250, 100, -175);
	cara(0, 250, 175, 0, 250, -175);

	// Dvere na predni stene; spodni hrana uz je soucast steny.
	cara(-40, -200, 175, -40, -40, 175);
	cara(-40, -40, 175, 40, -40, 175);
	cara(40, -40, 175, 40, -200, 175);

	// Leve okno.
	cara(-190, -35, 175, -100, -35, 175);
	cara(-100, -35, 175, -100, 45, 175);
	cara(-100, 45, 175, -190, 45, 175);
	cara(-190, 45, 175, -190, -35, 175);

	// Prave okno.
	cara(100, -35, 175, 190, -35, 175);
	cara(190, -35, 175, 190, 45, 175);
	cara(190, 45, 175, 100, 45, 175);
	cara(100, 45, 175, 100, -35, 175);

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
