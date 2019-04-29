#include<memory>

class Collider;
class Rectangle;
class Circle;
class Line;

class Solid {
public:
	virtual std::unique_ptr<Collider> getCollider() = 0;
};

class Collider {
public:
	enum CollisionType {Horizontal, Vertical, Diagonal, NoCollision};
	virtual CollisionType collideWith(Rectangle& c) { return NoCollision; }
	virtual CollisionType collideWith(Circle& c) { return NoCollision; }
	virtual CollisionType collideWith(Line& c) { return NoCollision; }
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
	virtual CollisionType collideInto(Collider& c) { return c.collideWith(*this);}
};

class Circle : public Collider {
	virtual CollisionType collideInto(Collider& c) { return c.collideWith(*this); }
};

class Line : public Collider {
	virtual CollisionType collideInto(Collider& c) { return c.collideWith(*this); }
}; 