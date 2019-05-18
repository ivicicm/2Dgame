#include "gamemap.h"
#include "staticfunctions.h"
#include <algorithm>
#include <memory>

#include <iostream>

using namespace std;

GameMenu::GameMenu(TextureSupport& ts, StateState state) : ProgramState(ts) {
	int center = textureS_.getWidth() / 2;

	std::string topText;
	SDL_Color topColor;
	if (state == ProgramState::ProgramStart) {
		topText = "Controls: move=wasd, shoot=mouse left";
		topColor = { 255,255,0,0 };
	}
	else if (state == ProgramState::Lost) {
		topText = "YOU DIED";
		topColor = { 255,0,0,0 };
	}
	else if (state == ProgramState::Won) {
		Texture textureTop(textureS_, "VICTORY ACHIEVED", { 255,255,0,0 });
		topText = "VICTORY ACHIEVED";
		topColor = { 255,255,0,0 };
	}
	Texture* topTexture = createText(topText, topColor);
	basicObjects.push_back(std::make_unique<GameObject>(topTexture, center - topTexture->getWidth() / 2, 170));

	Texture* tdifficulty = createText("choose difficulty:", { 255,255,150,0 });
	basicObjects.push_back(std::make_unique<GameObject>(tdifficulty, center - tdifficulty->getWidth() / 2, 350));

	SDL_Color mouseOver = { 200,0,0,0 };
	SDL_Color mouseAway = { 255,255,150,0 };

	Texture* teasy = createText("easy", mouseAway);
	Texture* teasy2 = createText("easy", mouseOver);
	auto p = std::make_unique<Clickable>(teasy, teasy2, center - teasy->getWidth() / 2, 420);
	easy = p.get();
	basicObjects.push_back(std::move(p));
	Texture* tmedium = createText("normal", mouseAway);
	Texture* tmedium2 = createText("normal", mouseOver);
	p = std::make_unique<Clickable>(tmedium, tmedium2, center - tmedium->getWidth() / 2, 470);
	medium = p.get();
	basicObjects.push_back(std::move(p));
	Texture* thard = createText("hard", mouseAway);
	Texture* thard2 = createText("hard", mouseOver);
	p = std::make_unique<Clickable>(thard, thard2, center - thard->getWidth() / 2, 520);
	hard = p.get();
	basicObjects.push_back(std::move(p));
}

bool mouseOver(Clickable* c, int x, int y) {
	bool isOver = c->x_ <= x && c->y_ <= y && c->x_ + c->width >= x && c->y_ + c->height >= y;
	c->mouseOver = isOver;
	return isOver;
}

void GameMenu::handleEvent(SDL_Event& event)
{
	if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
		int x, y;
		SDL_GetMouseState(&x, &y);
		if (mouseOver(easy, x, y) &&  event.button.button == SDL_BUTTON_LEFT  ) {
			next_ = ProgramState::StartEasy;
		}
		else if (mouseOver(medium, x, y) &&  event.button.button == SDL_BUTTON_LEFT) {
			next_ = ProgramState::StartMedium;
		}
		else if (mouseOver(hard, x, y) &&  event.button.button == SDL_BUTTON_LEFT) {
			next_ = ProgramState::StartHard;
		}
    }
}

void ProgramState::render() {
	for (auto && i : basicObjects) {
		i->render();
	}
}

Texture * ProgramState::createText(std::string text, SDL_Color color)
{
	auto topTexture = std::make_unique<Texture>(textureS_, text, color);
	Texture* toReturn = topTexture.get();
	generatedTextures.push_back(std::move(topTexture));
	return toReturn;
}

GameMap::GameMap(TextureSupport& ts, StateState state) : ProgramState(ts), distribution(0,100), generator(SDL_GetTicks()) {

	difficulty = state;
	auto playerp = make_unique<Player>(textureS_.getWidth() / 2, textureS_.getHeight() - 150, textureS_, *this);
	player = playerp.get();
	addPO(move(playerp),false);
	interfaceObjects.push_back(make_unique<PlayerStats>(textureS_,player));

	SDL_Color wallColor = { 60,60,60,255 };
	int thickness = 30;
	addIS(unique_ptr<Wall>(new Wall(0, 0, textureS_, thickness, textureS_.getHeight(), wallColor)),true);
	addIS(unique_ptr<Wall>(new Wall(0, 0, textureS_, textureS_.getWidth(), thickness, wallColor)), true);
	addIS(unique_ptr<Wall>(new Wall(textureS_.getWidth() - thickness, 0, textureS_, thickness, textureS_.getHeight(), wallColor)), true);
	addIS(unique_ptr<Wall>(new Wall(0, textureS_.getHeight() - thickness, textureS_, textureS_.getWidth(), thickness, wallColor)), true);

	Drone::DroneType type;
	switch (state)
	{
	case ProgramState::StartEasy:
		type = Drone::Slow;
		break;
	case ProgramState::StartMedium:
		type = Drone::Standard;
		break;
	case ProgramState::StartHard:
	default:
		type = Drone::Fast;
		break;
	}
	enemyCount = 3;
	int y = 100;
	int enemySpace = textureS_.getWidth() / (enemyCount + 1);
	for (int i = 1; i <= enemyCount; i++)
	{
		addPO(unique_ptr<Drone>(new Drone(textureS_, i*enemySpace, y, type, *this)), false);
	}

	int boxCount = 15;
	// randomly placing boxex everywhere
	for (int i = 0; i < boxCount; i++)
	{
		while (true) {
			int x = randomNumberTo100() * textureS_.getWidth() / 100;
			int y = randomNumberTo100() * textureS_.getHeight() / 100;
			Box* box = new Box(textureS_, x, y, Box::getRandomBox(randomNumberTo100()));
			unique_ptr<Collider> c = box->getCollider();
			if (getCollided(c.get(), nullptr).empty()) {
				addPO(unique_ptr<Box>(box), false);
				break;
			}
		}
	}
}

void GameMap::handleEvent(SDL_Event & event)
{
	if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
		bool down = event.type == SDL_KEYDOWN;
		switch (event.key.keysym.sym)
		{
		case SDLK_w: wasdDown[0] = down; break;
		case SDLK_a: wasdDown[1] = down; break;
		case SDLK_s: wasdDown[2] = down; break;
		case SDLK_d: wasdDown[3] = down; break;
		}
	}
	if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
		player->shoot();
}

float sign(float x) {
	if (x > 0) return 1;
	else if (x < 0) return -1;
	else return 0;
}

void GameMap::moveObjects(Uint32 miliseconds)
{
	for (auto && mo : movingObjects) {
		if (mo->vx != 0 || mo->vy != 0) {
			int dx = (int)(mo->vx*miliseconds/1000);
			int dy = (int)(mo->vy*miliseconds/1000);
			if (mo->physicalMovement && mo->drag != 0) {
				// drag causes physical to slow down
				float angle = atan(abs(mo->vy / mo->vx));
				float newvx = abs(mo->vx) - mo->drag*cos(angle)*miliseconds / 1000;
				if (newvx < 0) newvx = 0;
				float newvy = abs(mo->vy) - mo->drag*sin(angle)*miliseconds / 1000;
				if (newvy < 0) newvy = 0;
				mo->vx = sign(mo->vx)*newvx;
				mo->vy = sign(mo->vy)*newvy;
			}
			mo->x_ += dx;
			mo->y_ += dy;

			for (auto && co : solidObjects) {
				if (mo->cantCollideWith() != co) {
					resolveCollision(mo, co, dx, dy);
				}
			}
		}
	}
}

bool GameMap::resolveCollision(Movable* m, Solid* s, int dx, int dy) {
	auto mCollider =  m->getCollider();
	auto sCollider = s->getCollider();
	Collider::CollisionType collisionType = mCollider->collideInto(*sCollider);
	if (collisionType == Collider::NoCollision) return false;
	s->takeDamage(m->damageInCollision);
	// checking if the collision is caused only by horizontal or vertical movement
	if (collisionType == Collider::NonDiagonal || m->bouncing) {
		m->x_ -= dx;
		bool noVerticalCollision = (m->getCollider()->collideInto(*sCollider) == Collider::NoCollision);
		m->x_ += dx;

		m->y_ -= dy;
		bool noHorizontalCollision = (m->getCollider()->collideInto(*sCollider) == Collider::NoCollision);
		m->y_ += dy;
		if (noHorizontalCollision && !noVerticalCollision) collisionType = Collider::Vertical;
		else if (noVerticalCollision && !noHorizontalCollision) collisionType = Collider::Horizontal;
		else collisionType = Collider::Vertical;
	}
	// resolving collision
	if (collisionType == Collider::Horizontal || collisionType == Collider::Diagonal) {
		m->x_ -= dx;
		if (m->physicalMovement == true) {
			// transfer of velocity to the other object
			Movable *p = dynamic_cast<Movable*>(s);
			if (p != nullptr) {
				// conservation of momentum
				p->vx += m->vx*m->mass / p->mass /(m->bouncing?2:1);
			}
			if(!m->bouncing) m->vx = 0;
			else m->vx = -m->vx / 2;
		}
	}
	if (collisionType == Collider::Vertical || collisionType == Collider::Diagonal) {
		m->y_ -= dy;
		if (m->physicalMovement == true) {
			Movable *p = dynamic_cast<Movable*>(s);
			if (p != nullptr) {
				p->vy += m->vy*m->mass / p->mass / (m->bouncing ? 2 : 1);
			}
			if (!m->bouncing) m->vy = 0;
			else m->vy = -m->vy / 2;
		}
	}
	return true;
}

vector<Solid*> GameMap::getCollided(Collider* c, Solid* notThis) {
	vector<Solid*> collided;
	for (auto && so : solidObjects) {
		auto soCollider = so->getCollider();
		if (notThis != so && c->collideInto(*soCollider) != Collider::NoCollision) {
			collided.push_back(so);
		}
	}
	return collided;
}

void GameMap::updatePlayerFromInupt() {
	// weapon rotation
	int x, y;
	SDL_GetMouseState(&x, &y);
	x = x + textureS_.getCameraX() - player->x_ - player->getWidth()/2;
	y = - y - textureS_.getCameraY() + player->y_ + player->getHeight()/2;
	player->rotation = (int)Geometry::angleOfVector(x,y);


	// player movement
	if (player->physicalMovement == true) return;
	int keysDown = 0;
	for (size_t i = 0; i < 4; i++)
	{
		if (wasdDown[i] == true) ++keysDown;
	}
	float addVelocity = 0;
	if (keysDown == 1) addVelocity = (float)player->getSpeed();
	else if (keysDown == 2) addVelocity = (float)player->getDiagSpeed();

	if (wasdDown[0] == true) player->vy = -addVelocity;
	else if (wasdDown[2] == true) player->vy = addVelocity;
	else player->vy = 0;

	if (wasdDown[1] == true) {
		player->vx = -addVelocity; player->turnLeft = true;
	}
	else if (wasdDown[3] == true) {
		player->vx = addVelocity; player->turnLeft = false;
	}
	else player->vx = 0;
}

ProgramState::StateState GameMap::update(Uint32 miliseconds)
{
	updatePlayerFromInupt();
	moveObjects(miliseconds);
	vector<GameObject*> toDestroy;
	try {
		for (auto&& o : floorObjects) {
			if (!(o->update(miliseconds)))  toDestroy.push_back(o.get());
		}
		for (auto&& o : basicObjects) {
			if (!(o->update(miliseconds))) toDestroy.push_back(o.get());;
		}
		for (auto&& o : interfaceObjects) {
			if (!(o->update(miliseconds))) toDestroy.push_back(o.get());;
		}
		for (auto&& o : toDestroy) destroyObject(o);
	}
	catch (playerDeathExc&) { 
		SDL_Delay(500);
		return StateState::Lost; 
	}
	if (enemyCount <= 0) {
		SDL_Delay(500);
		return StateState::Won;
	}
	else return StateState::Running;
}

void GameMap::destroyObject(GameObject* o) {
	o->testament(*this);

	Movable* p1 = dynamic_cast<Movable*>(o);
	if (p1 != nullptr) movingObjects.erase(remove(movingObjects.begin(),movingObjects.end(),p1));

	ImmovableSolid* p2 = dynamic_cast<ImmovableSolid*>(o);
	if(p2 != nullptr )
		solidObjects.erase(remove(solidObjects.begin(), solidObjects.end(), dynamic_cast<Solid*>(p2)));
	else {
		PhysicalObject* p3 = dynamic_cast<PhysicalObject*>(o);
		if(p3 != nullptr)
		solidObjects.erase(remove(solidObjects.begin(), solidObjects.end(), dynamic_cast<Solid*>(p3)));
	}

	auto it = find_if(basicObjects.begin(), basicObjects.end(), [o](unique_ptr<GameObject>& x) {return x.get() == o; });
	if (it != basicObjects.end()) { 
		basicObjects.erase(it); return; 
	}
	it = find_if(floorObjects.begin(), floorObjects.end(), [o](unique_ptr<GameObject>& x) {return x.get() == o; });
	if (it != floorObjects.end()) {
		floorObjects.erase(it); return;
	}
	it = find_if(interfaceObjects.begin(), interfaceObjects.end(), [o](unique_ptr<GameObject>& x) {return x.get() == o; });
	if (it != interfaceObjects.end()) {
		interfaceObjects.erase(it);
		return;
	}
}

void GameMap::addPO(std::unique_ptr<PhysicalObject> po, bool floor)
{
	solidObjects.push_back(po.get());
	movingObjects.push_back(po.get());
	if (floor) floorObjects.push_back(move(po));
	else  basicObjects.push_back(move(po));
}
void GameMap::addIS(std::unique_ptr<ImmovableSolid> po, bool floor)
{
	solidObjects.push_back(po.get());
	if (floor) floorObjects.push_back(move(po));
	else  basicObjects.push_back(move(po));
}
void GameMap::addM(std::unique_ptr<Movable> po, bool floor)
{
	movingObjects.push_back(po.get());
	if (floor) floorObjects.push_back(move(po));
	else  basicObjects.push_back(move(po));
}
void GameMap::addGO(std::unique_ptr<GameObject> po, bool floor)
{
	if (floor) floorObjects.push_back(move(po));
	else  basicObjects.push_back(move(po));
}

int GameMap::playerx() { return player->x_ + player->getWidth() / 2; }

int GameMap::playery() { return player->y_ + player->getHeight() / 2; }

void GameMap::render() {
	for (auto&& o: floorObjects) {
		o->render();
	}
	for (auto&& o : basicObjects) {
		o->render();
	}
	for (auto&& o : interfaceObjects) {
		o->render();
	}
}
