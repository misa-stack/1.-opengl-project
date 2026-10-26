#ifndef CAMERA_H
#define CAMERA_H

struct Vec3
{
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
};

// Our own camera math. This module has no OpenGL, GLFW or GLM dependency.
struct Camera
{
	static constexpr double canvasWidth = 1280.0;
	static constexpr double canvasHeight = 720.0;
	static constexpr double focalLength = 200.0;
	static constexpr double nearPlane = 1.0;
	static constexpr double farPlane = 5000.0;

	Vec3 position{0.0, 0.0, 500.0};
	double yaw = 0.0;
	double pitch = 0.0;

	void reset();
	void look(double mouseDeltaX, double mouseDeltaY);
	void move(double rightInput, double forwardInput, double upInput, double dt);
	Vec3 toView(Vec3 worldPoint) const;
	bool projectLine(Vec3 first, Vec3 second, Vec3 &projectedFirst, Vec3 &projectedSecond) const;
};

#endif
