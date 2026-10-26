#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "camera.h"

using namespace std;

GLFWwindow * glfw_window = NULL;
bool keep_running = true;
Camera camera;
bool controlCamera = true;
bool mouseLooking = false;
double lastMouseX = 0.0;
double lastMouseY = 0.0;

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
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
	{
		controlCamera = !controlCamera;
		cout << "WASD controls: " << (controlCamera ? "camera" : "house") << endl;
	}
}

void glfw_mouse_click_callback(GLFWwindow * window, int button, int action, int mods)
{
	if (button != GLFW_MOUSE_BUTTON_RIGHT) return;
	mouseLooking = action == GLFW_PRESS;
	if (mouseLooking) glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
	glfwSetInputMode(window, GLFW_CURSOR, mouseLooking ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void glfw_mouse_move_callback(GLFWwindow * window, double xpos, double ypos)
{
	if (mouseLooking) camera.look(xpos - lastMouseX, ypos - lastMouseY);
	lastMouseX = xpos;
	lastMouseY = ypos;
}

void glfw_focus_callback(GLFWwindow * window, int focused)
{
	if (!focused)
	{
		mouseLooking = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
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
	glfwWindowHint(GLFW_DEPTH_BITS, 24);

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
	glfwSetWindowFocusCallback(glfw_window, glfw_focus_callback);
	
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
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// Camera::projectLine already computes the view, clipping and projection.
	// OpenGL receives normalized coordinates and only rasterizes our lines.

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}

double angleX = 0.0;
double angleY = 0.0;
double angleZ = 0.0;
double positionX = 0.0;
double positionY = 0.0;
double scale = 1.0;

Vec3 bod(double x, double y, double z)
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


	return {translatedX, translatedY, zAfterY};
}
void cara(int x1, int y1, int z1, int x2, int y2, int z2)
{
	Vec3 first, second;
	if (camera.projectLine(bod(x1, y1, z1), bod(x2, y2, z2), first, second))
	{
		glVertex3d(first.x, first.y, first.z);
		glVertex3d(second.x, second.y, second.z);
	}
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
	if (controlCamera)
	{
		const int vertical = (glfwGetKey(glfw_window, GLFW_KEY_SPACE) == GLFW_PRESS)
			- (glfwGetKey(glfw_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS
				|| glfwGetKey(glfw_window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
		camera.move(movementX, movementY, vertical, dt);
	}
	else
	{
		positionX += movementX * movementStep;
		positionY += movementY * movementStep;
	}


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
		camera.reset();
	}

	int width, height;
	glfwGetFramebufferSize(glfw_window, &width, &height);
	glViewport((width - 1280) / 2, (height - 720) / 2, 1280, 720);

	if (scale == 0.0) return;

	// Move the current world-space camera into the house's original coordinates:
	// undo translation, then Z/Y/X rotations, then scaling.
	const double relativeX = camera.position.x - positionX;
	const double relativeY = camera.position.y - positionY;
	const double cameraXAfterZ = relativeX * cos(angleZ) + relativeY * sin(angleZ);
	const double cameraYAfterZ = -relativeX * sin(angleZ) + relativeY * cos(angleZ);
	const double cameraZAfterY = cameraXAfterZ * sin(angleY) + camera.position.z * cos(angleY);
	const double cameraX = (cameraXAfterZ * cos(angleY) - camera.position.z * sin(angleY)) / scale;
	const double cameraY = (cameraYAfterZ * cos(angleX) + cameraZAfterY * sin(angleX)) / scale;
	const double cameraZ = (-cameraYAfterZ * sin(angleX) + cameraZAfterY * cos(angleX)) / scale;

	// This house is convex. An edge is visible when either adjacent face
	// points toward the camera; no filled polygons are needed.
	bool front = cameraZ > 175.0;
	bool back = cameraZ < -175.0;
	bool left = cameraX < -250.0;
	bool right = cameraX > 250.0;
	bool bottom = cameraY < -200.0;
	// Roof planes: y = 0.6*x + 250 and y = -0.6*x + 250.
	bool roofLeft = cameraY - 0.6 * cameraX > 250.0;
	bool roofRight = cameraY + 0.6 * cameraX > 250.0;

	// From inside the house, all surrounding faces can be seen.
	if (!front && !back && !left && !right && !bottom && !roofLeft && !roofRight)
	{
		front = back = left = right = bottom = roofLeft = roofRight = true;
	}

	glColor3ub(0, 0, 0);
	glBegin(GL_LINES);

	// Predni stena: x je sirka, y vyska, z hloubka.
	if (front || bottom) cara(-250, -200, 175, 250, -200, 175);
	if (front || right) cara(250, -200, 175, 250, 100, 175);
	if (front) cara(250, 100, 175, -250, 100, 175);
	if (front || left) cara(-250, 100, 175, -250, -200, 175);

	// Zadni stena.
	if (back || bottom) cara(-250, -200, -175, 250, -200, -175);
	if (back || right) cara(250, -200, -175, 250, 100, -175);
	if (back) cara(250, 100, -175, -250, 100, -175);
	if (back || left) cara(-250, 100, -175, -250, -200, -175);

	// Spojeni predni a zadni steny.
	if (left || bottom) cara(-250, -200, 175, -250, -200, -175);
	if (right || bottom) cara(250, -200, 175, 250, -200, -175);
	if (right || roofRight) cara(250, 100, 175, 250, 100, -175);
	if (left || roofLeft) cara(-250, 100, 175, -250, 100, -175);

	// Strecha: dva trojuhelniky spojene hrebenem.
	if (front || roofLeft) cara(-250, 100, 175, 0, 250, 175);
	if (front || roofRight) cara(0, 250, 175, 250, 100, 175);
	if (back || roofLeft) cara(-250, 100, -175, 0, 250, -175);
	if (back || roofRight) cara(0, 250, -175, 250, 100, -175);
	if (roofLeft || roofRight) cara(0, 250, 175, 0, 250, -175);

	// Dvere a okna jsou videt jen z predni strany.
	if (front)
	{
		cara(-40, -200, 175, -40, -40, 175);
		cara(-40, -40, 175, 40, -40, 175);
		cara(40, -40, 175, 40, -200, 175);

		cara(-190, -35, 175, -100, -35, 175);
		cara(-100, -35, 175, -100, 45, 175);
		cara(-100, 45, 175, -190, 45, 175);
		cara(-190, 45, 175, -190, -35, 175);

		cara(100, -35, 175, 190, -35, 175);
		cara(190, -35, 175, 190, 45, 175);
		cara(190, 45, 175, 100, 45, 175);
		cara(100, 45, 175, 100, -35, 175);
	}

	glEnd();
}

int main()
{
	if(!createWindow(1280, 720, "OpenGL", true, false)) return -1;
	
	init();
	cout << "WASD: camera | Space/Ctrl: up/down | hold right mouse: look | Tab: camera/house" << endl;
	cout << "Arrows + Q/E: rotate house | 1/2: scale house | R: reset house and camera | Esc: exit" << endl;
	
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
