#pragma once
#include "programgraphics.h"
#include "gameobject.h"
#include <SDL.h>
#include <vector>
#include <algorithm>
#include <random>

class GameObject;
class Clickable;
class Movable;
class Solid;
class PhysicalObject;
class ImmovableSolid;
class Player;
class Box;

// represents a state with things on the screen and interacts with them
class ProgramState {
public:
	enum StateState { ProgramStart, Running, StartEasy, StartMedium, StartHard, Won, Lost };

	ProgramState(TextureSupport& ts) : textureS_(ts) { textureS_.setCamera(0, 0); }
	virtual ~ProgramState() {};
	virtual void handleEvent(SDL_Event& event) = 0;
	virtual StateState update(Uint32 miliseconds) = 0;
	virtual void render();
	TextureSupport& textureS_;
protected:
	Texture* createText(std::string text, SDL_Color color);
	std::vector<std::unique_ptr<GameObject>> basicObjects;
	std::vector <std::unique_ptr<Texture>> generatedTextures;
};

class GameMenu : public ProgramState {
public:
	GameMenu(TextureSupport& ts, StateState state);
	virtual void handleEvent(SDL_Event& event);
	virtual StateState update(Uint32 miliseconds) { return next_; }
private:
	Clickable *easy, *medium, *hard;
	StateState next_ = Running;
};

class GameMap : public ProgramState {
public:
	GameMap(TextureSupport& ts, StateState state);
	virtual void handleEvent(SDL_Event& event);
	virtual StateState update(Uint32 miliseconds);
	void destroyObject(GameObject* o);
	virtual void render();
	void addPO(std::unique_ptr<PhysicalObject> po, bool floor);
	void addM(std::unique_ptr<Movable> po, bool floor);
	void addIS(std::unique_ptr<ImmovableSolid> po, bool floor);
	void addGO(std::unique_ptr<GameObject> po, bool floor);
	int randomNumberTo100() { return distribution(generator); }
	int playerx();
	int playery();
	std::vector<Solid*> getCollided(Collider * c, Solid * notThis);
	int enemyCount;
private:
	void moveObjects(Uint32 miliseconds);
	bool resolveCollision(Movable * m, Solid * s, int dx, int dy); //returns true if objects collide
	void updatePlayerFromInupt();
	std::vector<std::unique_ptr<GameObject>> floorObjects;
	std::vector<std::unique_ptr<GameObject>> interfaceObjects;
	std::vector<Movable*> movingObjects;
	std::vector<Solid*> solidObjects;
	Player* player;
	StateState difficulty;
	bool wasdDown[4] = { false,false,false,false }; // wasd in the order of the letters
	std::mt19937 generator; // generating random numbers
	std::uniform_int_distribution<> distribution;
};
