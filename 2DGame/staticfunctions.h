#pragma once
#include <math.h>

class Geometry {
public:
	static float const pi;

	// returns angle in degrees starting from the top
	static float angleOfVector(int x, int y) {
		float rotation = atan((float)x / (float)y) * 180 / pi;
		if (y < 0) rotation += 180;
		return rotation;
	}
};
