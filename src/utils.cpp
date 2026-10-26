#include "utils.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>

void makeFrustum(double fovY, double aspectRatio, double front, double back)
{
	const double DEG2RAD = 3.14159265 / 180;
	
	double tangent = tan(fovY/2 * DEG2RAD);   // tangent of half fovY
	double height = front * tangent;          // half height of near plane
	double width = height * aspectRatio;      // half width of near plane
	
	// params: left, right, bottom, top, near, far
	glFrustum(-width, width, -height, height, front, back);
}
