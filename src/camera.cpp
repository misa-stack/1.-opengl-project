#include "camera.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr double pi = 3.14159265358979323846;

Vec3 project(Vec3 point)
{
	// Perspective division and depth mapping into OpenGL's [-1, 1] range.
	return {
		point.x * Camera::focalLength / (point.z * Camera::canvasWidth / 2.0),
		point.y * Camera::focalLength / (point.z * Camera::canvasHeight / 2.0),
		std::clamp((Camera::farPlane + Camera::nearPlane) / (Camera::farPlane - Camera::nearPlane)
			- 2.0 * Camera::farPlane * Camera::nearPlane
			/ ((Camera::farPlane - Camera::nearPlane) * point.z), -1.0, 1.0)
	};
}
}

void Camera::reset()
{
	*this = Camera{};
}

void Camera::look(double mouseDeltaX, double mouseDeltaY)
{
	constexpr double sensitivity = 0.0025;
	constexpr double pitchLimit = 89.0 * pi / 180.0;
	yaw = std::remainder(yaw + mouseDeltaX * sensitivity, 2.0 * pi);
	pitch = std::clamp(pitch - mouseDeltaY * sensitivity, -pitchLimit, pitchLimit);
}

void Camera::move(double rightInput, double forwardInput, double upInput, double dt)
{
	// Right and forward are unit vectors derived from yaw/pitch, without matrices.
	Vec3 direction{
		rightInput * std::cos(yaw) + forwardInput * std::sin(yaw) * std::cos(pitch),
		upInput + forwardInput * std::sin(pitch),
		rightInput * std::sin(yaw) - forwardInput * std::cos(yaw) * std::cos(pitch)
	};
	const double length = std::sqrt(direction.x * direction.x
		+ direction.y * direction.y + direction.z * direction.z);
	if (length == 0.0) return;

	const double step = 200.0 * dt / length;
	position.x += direction.x * step;
	position.y += direction.y * step;
	position.z += direction.z * step;
}

Vec3 Camera::toView(Vec3 worldPoint) const
{
	// Subtract the camera position, then measure along its right/up/forward axes.
	const double x = worldPoint.x - position.x;
	const double y = worldPoint.y - position.y;
	const double z = worldPoint.z - position.z;
	return {
		x * std::cos(yaw) + z * std::sin(yaw),
		-x * std::sin(yaw) * std::sin(pitch) + y * std::cos(pitch)
			+ z * std::cos(yaw) * std::sin(pitch),
		x * std::sin(yaw) * std::cos(pitch) + y * std::sin(pitch)
			- z * std::cos(yaw) * std::cos(pitch)
	};
}

bool Camera::projectLine(Vec3 first, Vec3 second, Vec3 &projectedFirst, Vec3 &projectedSecond) const
{
	first = toView(first);
	second = toView(second);
	const double horizontalSlope = canvasWidth / (2.0 * focalLength);
	const double verticalSlope = canvasHeight / (2.0 * focalLength);

	// A nonnegative value means the endpoint is inside this clipping plane.
	const double firstDistances[] = {
		first.z - nearPlane, farPlane - first.z,
		first.x + horizontalSlope * first.z, horizontalSlope * first.z - first.x,
		first.y + verticalSlope * first.z, verticalSlope * first.z - first.y
	};
	const double secondDistances[] = {
		second.z - nearPlane, farPlane - second.z,
		second.x + horizontalSlope * second.z, horizontalSlope * second.z - second.x,
		second.y + verticalSlope * second.z, verticalSlope * second.z - second.y
	};

	double start = 0.0;
	double end = 1.0;
	for (int plane = 0; plane < 6; ++plane)
	{
		const double a = firstDistances[plane];
		const double b = secondDistances[plane];
		if (a < 0.0 && b < 0.0) return false;
		if (a < 0.0 || b < 0.0)
		{
			const double intersection = a / (a - b);
			if (a < 0.0) start = std::max(start, intersection);
			else end = std::min(end, intersection);
			if (start > end) return false;
		}
	}

	const Vec3 delta{second.x - first.x, second.y - first.y, second.z - first.z};
	// Clip in 3D before dividing by depth, including when a line crosses the camera.
	projectedFirst = project({first.x + delta.x * start, first.y + delta.y * start, first.z + delta.z * start});
	projectedSecond = project({first.x + delta.x * end, first.y + delta.y * end, first.z + delta.z * end});
	return true;
}
