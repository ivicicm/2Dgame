#pragma once
#include "programgraphics.h"
#include "collider.h"
#include <math.h>
#include "gamemap.h"

class GameMap;

class GameObject {
public:
	GameObject() {}
	GameObject(Texture* texture, float x, float y) {
		x_ = x; y_ = y;
		texture_ = texture;
	}
	virtual bool update(Uint32 miliseconds) { return true; } // returns false if the object wants to be destroyed
	virtual ~GameObject() {}
	virtual void render() { texture_->render((int)x_, (int)y_); }
	float x_, y_;
protected:
	Texture* texture_;
};

// ProgramState has to check if the mouse is over it and set mouseOver
// also needs to handle the clicking
class Clickable : public GameObject {
public:
	Clickable() {}
	Clickable(Texture* texture, Texture* mouseOverTexture,float x, float y) {
		x_ = x; y_ = y;
		texture_ = texture;
		mouseOverTexture_ = mouseOverTexture;
		width = texture->getWidth();
		height = texture->getHeight();
	}
	void render() { 
		if(mouseOver)  mouseOverTexture_->render((int)x_, (int)y_);
		else texture_->render((int)x_, (int)y_);
	}
	int width, height;
	bool mouseOver = false;
private:
	Texture* mouseOverTexture_;
};

class Movable : public GameObject {
public:
	// velocity
	float vx = 0;
	float vy = 0;
	float mass = 10;
	float drag = 10; // drag - how much velocity is lost in 1 second
	bool physicalMovement = false; // can make other objects move
	virtual std::unique_ptr<Collider> getCollider() = 0;
	virtual Solid* cantCollideWith() = 0;
};

class ImmovableSolid : public GameObject, public Solid {
public:
	virtual std::unique_ptr<Collider> getCollider() {
		return std::make_unique<Rectangle>(x_, y_, texture_->getWidth(), texture_->getHeight());
	}
};

class PhysicalObject : public Movable, public  Solid {
	virtual std::unique_ptr<Collider> getCollider() {
		return std::make_unique<Rectangle>(x_, y_, texture_->getWidth(), texture_->getHeight());
	}
	virtual Solid* cantCollideWith() { return this; }
};

class Projectile : public Movable {

};

class Wall : public ImmovableSolid {
public:
	Wall(float x, float y, TextureSupport& ts, int width, int height, SDL_Color color) : ts_(ts) {
		x_ = x;
		y_ = y;
		width_ = width; height_ = height; color_ = color;
	}
	virtual std::unique_ptr<Collider> getCollider() {
		return std::make_unique<Rectangle>(x_, y_, width_, height_);
	}
	virtual void render() { 
		SDL_Rect rect = { x_, y_, width_, height_ };
		SDL_SetRenderDrawColor(ts_.getRenderer(), color_.r, color_.g, color_.b, color_.a);
		SDL_RenderFillRect(ts_.getRenderer(), &rect);
	}
private:
	int width_;
	int height_;
	SDL_Color color_;
	TextureSupport& ts_;
};

class Player : public PhysicalObject {
public:
	Player(float x, float y, TextureSupport& ts, GameMap& gameMap) : gameMap_(gameMap) {
		x_ = x;
		y_ = y;
		texture_ = ts.getTexture("player.png", 2);
		diagSpeed_ = sqrt(speed_*speed_ / 2);
	}
	void render() {
		texture_->render((int)x_, (int)y_, turnLeft?1:0);
	}
	float getSpeed() { return speed_; }
	float getDiagSpeed() { return diagSpeed_; }
	bool turnLeft = true;
private:
	float speed_ = 300;
	float diagSpeed_;
	GameMap& gameMap_;
};