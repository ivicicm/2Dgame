#include "collider.h"
#include "math.h"

int min(int x, int y) {
	if (x >= y) return x;
	else return y;
}

Collider::CollisionType Rectangle::collideWith(Rectangle& c)
{
	if (x1_ <= c.x2_ && c.x1_ <= x2_ && y1_ <= c.y2_ && c.y1_ <= y2_) {
		int verticalDistance = min(abs(y1_ - c.y2_), abs(y2_ - c.y1_));
		int horizontalDistance = min(abs(x1_ - c.x2_), abs(x2_ - c.x1_));
		if (verticalDistance <= horizontalDistance) return Vertical;
		else return Horizontal;
	}
	else {
		return NoCollision;
	}
}
