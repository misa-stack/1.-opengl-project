#include "camera.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
int checks = 0;

void expect(bool condition, const char *message)
{
	if (!condition)
	{
		std::cerr << "FAIL: " << message << '\n';
		std::exit(1);
	}
	++checks;
}

void near(double actual, double expected, const char *message)
{
	expect(std::isfinite(actual) && std::abs(actual - expected) < 1e-8, message);
}
}

int main()
{
	Camera camera;
	Vec3 point = camera.toView({100, 50, 25});
	near(point.x, 100, "default camera keeps horizontal coordinate");
	near(point.y, 50, "default camera keeps vertical coordinate");
	near(point.z, 475, "depth is distance in the viewing direction");

	camera.yaw = 1.5707963267948966;
	point = camera.toView({100, 0, 500});
	near(point.x, 0, "positive yaw looks toward positive X");
	near(point.z, 100, "yaw preserves distance along view direction");
	camera.reset();
	camera.pitch = 0.5235987755982989;
	point = camera.toView({0, 50, 413.3974596215561});
	near(point.y, 0, "positive pitch looks upward");
	near(point.z, 100, "pitch preserves distance along view direction");

	camera.reset();
	Vec3 first, second;
	expect(camera.projectLine({-100, 0, 0}, {100, 0, 0}, first, second), "center line is visible");
	near(first.x, -0.0625, "default projection preserves original framing");
	near(second.x, 0.0625, "default projection is symmetric");
	near(first.y, 0, "center line stays vertically centered");

	expect(camera.projectLine({0, 0, 499.5}, {0, 1, 498}, first, second), "line crossing near plane is clipped");
	near(first.z, -1, "near plane maps to normalized depth -1");
	near(first.y, 0.1851851851851852, "near clipping interpolates the endpoint");
	near(second.y, 0.2777777777777778, "visible endpoint remains unchanged");
	expect(!camera.projectLine({0, 0, 501}, {1, 0, 502}, first, second), "line entirely behind camera is rejected");
	expect(!camera.projectLine({0, 0, -5001}, {1, 0, -6000}, first, second), "line beyond far plane is rejected");
	expect(camera.projectLine({0, 0, -4000}, {0, 0, -6000}, first, second), "line crossing far plane is clipped");
	near(second.z, 1, "far plane maps to normalized depth 1");
	expect(camera.projectLine({-2000, 0, 0}, {0, 0, 0}, first, second), "line crossing left plane is clipped");
	near(first.x, -1, "left intersection reaches viewport boundary");
	expect(!camera.projectLine({2000, 0, 0}, {2500, 1, 0}, first, second), "line entirely right of view is rejected");
	expect(camera.projectLine({0, 1000, 0}, {0, 0, 0}, first, second), "line crossing top plane is clipped");
	near(first.y, 1, "top intersection reaches viewport boundary");
	expect(camera.projectLine({0, -1000, 0}, {0, 0, 0}, first, second), "line crossing bottom plane is clipped");
	near(first.y, -1, "bottom intersection reaches viewport boundary");
	expect(camera.projectLine({0, 0, 501}, {0, 0, 0}, first, second), "line crossing camera position stays finite");
	near(first.z, -1, "crossing camera position clips before division");

	camera.move(1, 0, 0, 0.5);
	near(camera.position.x, 100, "strafe speed is 200 units per second");
	near(camera.position.z, 500, "strafing does not move forward");
	camera.reset();
	camera.move(1, 1, 0, 0.5);
	near(camera.position.x, 70.71067811865476, "diagonal input is normalized");
	near(camera.position.z, 429.28932188134524, "diagonal forward motion is normalized");
	Camera manyFrames;
	for (int frame = 0; frame < 30; ++frame) manyFrames.move(1, 1, 0, 1.0 / 60.0);
	near(manyFrames.position.x, camera.position.x, "horizontal movement is frame-rate independent");
	near(manyFrames.position.z, camera.position.z, "forward movement is frame-rate independent");
	camera.reset();
	camera.move(0, 0, 1, 0.5);
	near(camera.position.y, 100, "vertical input moves along world Y");
	camera.reset();
	camera.yaw = 1.5707963267948966;
	camera.move(0, 1, 0, 0.5);
	near(camera.position.x, 100, "forward movement follows camera orientation");
	near(camera.position.z, 500, "forward movement at 90 degrees does not change Z");
	camera.move(1, 1, 1, 0);
	near(camera.position.x, 100, "zero delta does not move camera");

	camera.reset();
	camera.look(100, -100);
	expect(camera.yaw > 0 && camera.pitch > 0, "mouse right and up turn view right and up");
	camera.look(0, -1000000);
	expect(camera.pitch < 1.5707963267948966 && camera.pitch > 1.5, "pitch stops before the upward pole");
	camera.look(0, 1000000);
	expect(camera.pitch > -1.5707963267948966 && camera.pitch < -1.5, "pitch stops before the downward pole");
	camera.reset();
	near(camera.position.x, 0, "reset restores camera X");
	near(camera.position.y, 0, "reset restores camera Y");
	near(camera.position.z, 500, "reset restores camera Z");
	near(camera.yaw, 0, "reset restores yaw");
	near(camera.pitch, 0, "reset restores pitch");

	std::cout << "PASS: " << checks << " camera math checks\n";
}
