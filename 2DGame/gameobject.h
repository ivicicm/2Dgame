#pragma once

class GameObject {
	float x, y;
};

class Projectile : GameObject {
	float momentum;
};

class SolidObject : GameObject {
public:

private:
	float width, height;
};

class MovingObject : SolidObject {
public:

private:
	float velocityX, velocityY;
	float drag;
	float mass;
};