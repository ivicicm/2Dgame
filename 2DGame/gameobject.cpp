#include "gameobject.h"
#include "math.h"
#include "staticfunctions.h"
#include <memory>
#include <iostream>

using namespace std;

void Movable::setVelocity(int angle, int startDistance, float speed)
{
	float ycoef = -cos(angle*M_PI/180);
	float xcoef = sin(angle*M_PI/180);
	x_ += startDistance * xcoef;
	y_ += startDistance * ycoef;
	vy = ycoef*speed;
	vx = xcoef*speed;

}

void Player::render() {
	texture_->render(x_, y_, turnLeft ? 1 : 0);
	weaponTexture->render(x_ - weaponTexture->getWidth() / 2 + width / 2,
		y_ + height*weaponRelativeHeight - weaponTexture->getHeight() / 2, rotation > 0 ? 0 : 1, true, rotation);
}

void Player::shoot()
{
	if (timeLeftReloading <= 0) {
		timeLeftReloading = reloadTime;
		Projectile* projectile = new Projectile(this, gameMap_.textureS_, 800, x_ + width / 2, 
			y_ + height * weaponRelativeHeight, rotation, weaponTexture->getHeight() * 1 / 2);
		gameMap_.addM(unique_ptr<Projectile>(projectile),false);
	}
}

bool Player::update(Uint32 miliseconds)
{
	stopPhysicalMovement();
	if (timeLeftReloading > 0) timeLeftReloading -= miliseconds;
	if (!stillLives()) throw playerDeathExc();
	return true;
}

Drone::Drone(TextureSupport & ts, int x, int y, DroneType type, GameMap & map) : map_(map) {
	x_ = x;
	y_ = y;
	texture_ = ts.getTexture("drone.png");
	type_ = type;
	switch (type)
	{
	case Drone::Slow:
		speed = 150;
		projectileSpeed = 250;
		break;
	case Drone::Standard:
		speed = 300;
		projectileSpeed = 500;
		break;
	case Drone::Fast:
		speed = 400;
		projectileSpeed = 700;
		break;
	default:
		break;
	}
}

bool Drone::update(Uint32 miliseconds) {
	if (stopPhysicalMovement()) timeToNextAction = 200;
	if (!physicalMovement) {
		if (timeToNextAction > 0) timeToNextAction -= miliseconds;
		else {
			// starting new action
			vx = 0; vy = 0;
			int r = map_.randomNumberTo100();
			if (r < 25) timeToNextAction = shoot();
			else timeToNextAction = move();
		}
	}
	return stillLives();
}

void Drone::testament(GameMap & map)
{
	map.enemyCount--;
}

int Drone::shoot()
{
	int x = map_.playerx() - x_;
	int y = -map_.playery() + y_;
	int angle = Geometry::angleOfVector(x, y);
	// shoots randomly near the player
	angle += (map_.randomNumberTo100() - 50) / 5;

	Projectile* projectile = new Projectile(this, map_.textureS_, projectileSpeed, x_ + texture_->getWidth() / 2,
		y_ + texture_->getHeight() / 2, angle,0,false);
	map_.addM(unique_ptr<Projectile>(projectile), true);
	return 500;
}

int Drone::move()
{
	// generate random distance
	int from = 100;
	int to = 300;
	int distance = map_.randomNumberTo100()*(to-from)/100 + from;
	// generate random direction
	float radAngle = map_.randomNumberTo100() / (float)100 * 2 * M_PI;
	int dx = (int)(cos(radAngle)*distance);
	int dy = (int)(sin(radAngle)*distance);
	int centerx = x_ + texture_->getWidth() / 2;
	int centery = y_ + texture_->getHeight() / 2;
	Line line(centerx, centery, centerx + dx, centery + dy);
	// makes 2 lines in direction of the movement to check if no objects are in front of it
	int fromCenter = texture_->getWidth() / 2;
	Line topLine = line.moveSideways(fromCenter);
	Line botLine = line.moveSideways(-fromCenter);
	if (map_.getCollided(&topLine, this).empty() && map_.getCollided(&botLine, this).empty()) {
		vx = cos(radAngle)*speed;
		vy = sin(radAngle)*speed;
		return distance *1000 / speed; // returns time
	}
	else return 0;
}

Box::Box(TextureSupport & ts, int x, int y, BoxType type) {
	x_ = x;
	y_ = y;
	type_ = type;
	string texture;
	switch (type)
	{
	case Box::Standard:
		texture = "package.png";
		break;
	case Box::Fragile:
		texture = "fragilepackage.png";
		health = 1;
		break;
	case Box::Bomb:
		texture = "bomb.png";
		break;
	case Box::BallBox:
		texture = "ballpackage.png";
		break;
	default:
		texture = "package.png";
		break;
	}
	texture_ = ts.getTexture(texture);
}

void Box::testament(GameMap & map) {
	if (type_ == Bomb) {
		map.addM(std::make_unique<Explosion>(map.textureS_, x_, y_),false);
	} else if (type_ == BallBox)
		map.addPO(std::make_unique<Ball>(map.textureS_, x_, y_,map), false);
}

bool Ball::update(Uint32 miliseconds) {
	// texture rotation
	float speed = sqrt( vx * vx + vy * vy);
	rotation += (speed * miliseconds / 1000) / circumference * 360 * (vx>0?1:-1);
	// making the ball follow the player
	int x = map_.playerx() - x_;
	int y = map_.playery() - y_;
	float angle = Geometry::angleOfVector(x, y)/180*M_PI ;
	vx += sin(angle)*playerAttraction*miliseconds/1000;
	vy += cos(angle)*playerAttraction*miliseconds/1000;
	// ball is indestructible
	return true;
}

void PlayerStats::render() {
	int healthBarSpace = 2;
	int relativeHeight = 20;
	SDL_SetRenderDrawColor(ts_.getRenderer(), 255, 255, 255, 0);

	int x = player_->x_ + player_->getWidth() / 2 - width / 2;
	int y = player_->y_ - relativeHeight;
	for (int i = 0; i < player_->getHealth(); i++)
	{
		SDL_Rect rect = { x+ i*healthBarWidth, y, healthBarWidth - healthBarSpace, 5 };
		SDL_RenderFillRect(ts_.getRenderer(), &rect);
	}
	SDL_SetRenderDrawColor(ts_.getRenderer(), 255, 0, 0, 0);
	SDL_Rect rect = { x, y+7, (int)(player_->getReloadProgression()*width), 5 };
	SDL_RenderFillRect(ts_.getRenderer(), &rect);
}

Box::BoxType Box::getRandomBox(int randomto100)
{
	int vote = randomto100;
	if (vote < 40) return Box::Standard;
	else if (vote < 65) return Box::Fragile;
	else if (vote < 90) return Box::Bomb;
	else return Box::BallBox;
}