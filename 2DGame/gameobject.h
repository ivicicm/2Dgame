#pragma once
#include "programgraphics.h"
#include "collider.h"
#include <math.h>
#include "gamemap.h"

class GameMap;

class GameObject {
public:
	GameObject() {}
	GameObject(Texture* texture, int x, int y) {
		x_ = x; y_ = y;
		texture_ = texture;
	}
	virtual bool update(Uint32 miliseconds) { return true; } // returns false if the object wants to be destroyed
	virtual void testament(GameMap& map) {} // is executed when the object is destroyed
	virtual ~GameObject() {}
	virtual void render() { texture_->render((int)x_, (int)y_); }
	int x_, y_;
protected:
	Texture* texture_;
};

// ProgramState has to check if the mouse is over it and set mouseOver
// also needs to handle the clicking
class Clickable : public GameObject {
public:
	Clickable() {}
	Clickable(Texture* texture, Texture* mouseOverTexture,int x, int y) {
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
	// velocity - pixels per second
	float vx = 0;
	float vy = 0;
	float mass = 10;
	float drag = 2000; // drag - how much velocity is lost in 1 second
	bool physicalMovement = false; // can make other objects move
	bool bouncing = false; // collision doesn't causes object to stop in the collision direction
	int damageInCollision = 0;
	virtual std::unique_ptr<Collider> getCollider() = 0;
	virtual Solid* cantCollideWith() {
		return nullptr;
	}
protected:
	// also moves the object in the angle direction
	void setVelocity(int angle, int startDistance,float speed);
};

class Explosion : public Movable {
public:
	Explosion(TextureSupport& ts, int x, int y) {
		texture_ = ts.getTexture("explosion.png");
		x_ = x - texture_->getWidth() / 2;
		y_ = y - texture_->getHeight() / 2;
		damageInCollision = 10;
		setVelocity(0, 0, 0.001f); // for starting collisions
	}
	virtual std::unique_ptr<Collider> getCollider() {
		return std::make_unique<Circle>(x_, y_, texture_->getWidth());
	}
	virtual bool update(Uint32 miliseconds) {
		milisecondsToLive -= miliseconds;
		return milisecondsToLive > 0;
	}
private:
	int milisecondsToLive = 700;
};

class ImmovableSolid : public GameObject, public Solid {
public:
	virtual std::unique_ptr<Collider> getCollider() {
		return std::make_unique<Rectangle>(x_, y_, texture_->getWidth(), texture_->getHeight());
	}
};

class PhysicalObject : public Movable, public  Solid {
public:
	virtual std::unique_ptr<Collider> getCollider() {
		return std::make_unique<Rectangle>(x_, y_, texture_->getWidth(), texture_->getHeight());
	}
	virtual Solid* cantCollideWith() { return this; }
	virtual void takeDamage(int damage) {
		health -= damage;
		physicalMovement = true;
	}
	virtual bool update(Uint32 miliseconds) {
		stopPhysicalMovement();
		return stillLives();
	}
protected:
	bool stopPhysicalMovement() {
		if (physicalMovement == true && vx == 0 && vy == 0) { physicalMovement = false; return true; }
		else return false;
	}
	bool stillLives() { return health > 0 || (health == 0 && physicalMovement); }
	int health = 3;
};

class Drone : public PhysicalObject {
public:
	enum DroneType {Slow, Standard, Fast};
	Drone(TextureSupport& ts, int x, int y, DroneType type, GameMap& map);
	virtual bool update(Uint32 miliseconds);
	virtual void testament(GameMap& map);
private:
	int shoot();
	int move();
	GameMap& map_;
	int timeToNextAction = 0;
	DroneType type_;
	float speed, projectileSpeed;
};

class Ball : public PhysicalObject {
public:
	Ball(TextureSupport& ts, int x, int y, GameMap& map) : map_(map) {
		texture_ = ts.getTexture("ball.png");
		x_ = x;
		y_ = y;
		drag = 0;
		mass = 20;
		physicalMovement = true;
		bouncing = true;
		damageInCollision = 1;
		circumference = (int)(M_PI * texture_->getWidth());
	}
	virtual bool update(Uint32 miliseconds);
	virtual void render() { texture_->render((int)x_, (int)y_,0,true,rotation); }
	std::unique_ptr<Collider> getCollider()
	{
		return std::make_unique<Circle>(x_, y_, texture_->getWidth());
	}
private:
	int rotation = 0;
	int circumference;
	GameMap& map_;
	int playerAttraction = 300;
};

class Box : public PhysicalObject {
public:
	enum BoxType { Standard, Fragile, Bomb, BallBox };

	Box(TextureSupport& ts, int x, int y, BoxType type);
	virtual void testament(GameMap& map);
	static Box::BoxType getRandomBox(int randomto100);
private:
	BoxType type_;
};

class Projectile : public Movable {
public:
	Projectile(Solid* shooter, TextureSupport& ts, float startSpeed, int x, int y, int angle, int startDistance = 0, bool fromPlayer = true) : shooter_(shooter) {
		physicalMovement = true;
		drag = 0;
		if (fromPlayer) texture_ = ts.getTexture("player_projectile.png");
		else texture_ = ts.getTexture("enemy_projectile.png");
		x_ = x - texture_->getWidth() / 2;
		y_ = y - texture_->getHeight() / 2;
		setVelocity(angle, startDistance, startSpeed);
		damageInCollision = 1;
	}
	std::unique_ptr<Collider> getCollider()
	{
		return std::make_unique<Circle>(x_, y_, texture_->getWidth());
	}
	Solid* cantCollideWith()
	{
		return shooter_;
	}
	// is destroyed after collison
	virtual bool update(Uint32 miliseconds) { return !(vx == 0 && vy == 0); }
protected:
	Solid* shooter_;
};

class Wall : public ImmovableSolid {
public:
	Wall(int x, int y, TextureSupport& ts, int width, int height, SDL_Color color) : ts_(ts) {
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
	Player(int x, int y, TextureSupport& ts, GameMap& gameMap) : gameMap_(gameMap) {
		x_ = x;
		y_ = y;
		texture_ = ts.getTexture("player.png", 2);
		weaponTexture = ts.getTexture("weapon.png", 2);
		diagSpeed_ = (int)sqrt(speed_*speed_ / 2);
		width = texture_->getWidth();
		height = texture_->getHeight();
	}
	int getHealth() { return health; }
	float getReloadProgression() { return (float)timeLeftReloading / reloadTime; }
	void render();
	void shoot();
	int getSpeed() { return speed_; }
	int getDiagSpeed() { return diagSpeed_; }
	bool turnLeft = true;
	int rotation = 90;
	int getWidth() { return width; }
	int getHeight() { return height; }
	virtual bool update(Uint32 miliseconds);
private:
	float weaponRelativeHeight = (float)5 / 9;
	int width, height;
	int speed_ = 300;
	int diagSpeed_;
	int reloadTime = 2000;
	int timeLeftReloading = 0;
	GameMap& gameMap_;
	Texture* weaponTexture;
};

class PlayerStats : public GameObject {
public:
	PlayerStats(TextureSupport& ts, Player* player) : ts_(ts) {
		player_ = player;
		x_ = 0; y_ = 0;
		healthBarWidth = width / player->getHealth();
	}
	void render();
private:
	int healthBarWidth;
	int width = 60;
	TextureSupport& ts_;
	Player* player_;
};

class playerDeathExc : public std::exception {};