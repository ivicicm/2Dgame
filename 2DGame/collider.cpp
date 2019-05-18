#include "collider.h"
#include "math.h"
#include <iostream>

int min(int x, int y) {
	if (x <= y) return x;
	else return y;
}

Collider::CollisionType Rectangle::collideWith(Rectangle& c)
{
	if (x1_ <= c.x2_ && c.x1_ <= x2_ && y1_ <= c.y2_ && c.y1_ <= y2_) {
		return NonDiagonal;
	}
	else {
		return NoCollision;
	}
}

Collider::CollisionType Rectangle::collideWith(Circle& c)
{
	// checking if the centre of the circle is closer than r from rectangle in max norm
	if (x1_ <= c.x_ + c.r_ && x2_ >= c.x_ - c.r_ && y1_ <= c.y_ + c.r_ && y2_ >= c.y_ - c.r_) {
		int cornerx, cornery;
		if (c.x_ <= x1_) {
			cornerx = x1_;
		}
		else if (c.x_ >= x2_) {
			cornerx = x2_;
		}
		else return Diagonal;

		if (c.y_ <= y1_) {
			cornery = y1_;
		}
		else if (c.y_ >= y2_) {
			cornery = y2_;
		}
		else return Diagonal;
		// now the circle is in one of 4 squares near the edges of the rectangle
		if ((cornerx - c.x_)*(cornerx - c.x_) + (cornery - c.y_)*(cornery - c.y_) <= c.r_*c.r_) return Diagonal;
		else return NoCollision;

	}
	else return NoCollision;
}

Collider::CollisionType Rectangle::collideWith(Line & c)
{
	Line top = Line(x1_, y1_, x2_, y1_);
	Line down = Line(x1_, y2_, x2_, y2_);
	Line left = Line(x1_, y1_, x1_, y2_);
	Line right = Line(x2_, y1_, x2_, y2_);
	if(c.collideWith(top) == NoCollision && c.collideWith(down) == NoCollision 
		&& c.collideWith(left) == NoCollision && c.collideWith(right) == NoCollision
		&& // checking if the line isn't inside of the rectangle
		!(x1_ < c.x1_ && c.x1_ < x2_ && y1_ < c.y1_ && c.y1_ < y2_)) return NoCollision;
	else return Diagonal;
}

Collider::CollisionType Circle::collideWith(Circle & c)
{
	if ((c.x_ - x_)*(c.x_ - x_) + (c.y_ - y_)*(c.y_ - y_) <= (c.r_ + r_)*(c.r_ + r_)) return Diagonal;
	else return NoCollision;
}

Collider::CollisionType Line::collideWith(Line & c)
{
	int dx = x2_ - x1_;
	int dy = y2_ - y1_;
	int dxc = c.x2_ - c.x1_;
	int dyc = c.y2_ - c.y1_;
	// solving equation v1 + t * (v2 - v1) = cv1 + s * (cv2 - cv1) where v1 = (x1,y1)
	// t and s have to be in [0,1]
	// we have one equation for x, one for y, now we subtract multiplication of the first from the second to remove coefficient t
	int rightsideX = c.x1_ - x1_;
	int rightsideY = c.y1_ - y1_;
	float xEquationMulitplier = (float)dy / dx;
	float sCoefficient = xEquationMulitplier * dxc - dyc; 
	float sRightSide = rightsideY - (xEquationMulitplier * rightsideX);
	// singular matrix, lines have the same direction
	if (sCoefficient == 0) {
		return NoCollision; // intersection occurs when the lines are over each other, but we will take it as no collision
	}
	float s = sRightSide / sCoefficient;
	if (s < 0 || s > 1) return NoCollision;
	float t = (rightsideX + s * dxc) / dx;
	if (t < 0 || t > 1) return NoCollision;
	
	else return Diagonal;
}

Line Line::moveSideways(int distance)
{
	float vx = x2_ - x1_;
	float vy = y2_ - y1_;
	// normalize
	float norm = sqrt(vx * vx + vy * vy);
	vx = vx / norm;
	vy = vy / norm;
	// vectors of movement
	int ux =(int)( -vy * distance);
	int uy = (int)(vx * distance);
	return Line(x1_ + ux, y1_ + uy, x2_ + ux, y2_ + uy);
}
