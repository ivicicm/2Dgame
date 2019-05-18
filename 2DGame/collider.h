#include<memory>

class Collider;
class Rectangle;
class Circle;
class Line;

class Solid {
public:
	virtual std::unique_ptr<Collider> getCollider() = 0;
	virtual void takeDamage(int damage) {};
};

class Collider {
public:
	enum CollisionType {Horizontal, Vertical, Diagonal, NonDiagonal, NoCollision};
	virtual CollisionType collideWith(Rectangle& c) = 0;
	virtual CollisionType collideWith(Circle& c) = 0;
	virtual CollisionType collideWith(Line& c) = 0;
	virtual CollisionType collideInto(Collider& c) = 0;
};

class Rectangle : public Collider {
public:
	Rectangle(int x,int y,int width,int height) {
		x1_ = x;
		y1_ = y;
		x2_ = x + width;
		y2_ = y + height;
	}
	int x1_, x2_, y1_, y2_;
	CollisionType collideWith(Rectangle& c);
	CollisionType collideWith(Circle& c);
	CollisionType collideWith(Line& c);
	virtual CollisionType collideInto(Collider& c) { return c.collideWith(*this);}
};

class Circle : public Collider {
public:
	Circle(int x, int y, int diameter) {
		r_ = diameter / 2;
		x_ = x + r_;
		y_ = y + r_;
	}
	int x_, y_, r_;
	CollisionType collideWith(Rectangle& c) { return c.collideWith(*this); }
	CollisionType collideWith(Circle& c);
	CollisionType collideWith(Line& c) { return NoCollision; } // this type of collision is undetected for now
	virtual CollisionType collideInto(Collider& c) { return c.collideWith(*this); }
};

class Line : public Collider {
public:
	Line(int x1, int y1, int x2, int y2) {
		x1_ = x1; y1_ = y1;
		x2_ = x2; y2_ = y2;
	}
	int x1_, x2_, y1_, y2_;
	CollisionType collideWith(Line& c);
	CollisionType collideWith(Circle& c) { return c.collideWith(*this); }
	CollisionType collideWith(Rectangle& c) { return c.collideWith(*this); }
	virtual CollisionType collideInto(Collider& c) { return c.collideWith(*this); }
	Line moveSideways(int distance);
}; 