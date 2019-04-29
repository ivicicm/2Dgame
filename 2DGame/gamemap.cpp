#include "gamemap.h"
#include <algorithm>
#include <memory>

using namespace std;

GameMenu::GameMenu(TextureSupport& ts, StateState state) : ProgramState(ts) {
	int center = textureS_.getWdith() / 2;
	std::string topText;
	SDL_Color topColor;
	if (state == ProgramState::ProgramStart) {
		topText = "move:wasd, shoot:mouse left, reload:mouse right";
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
	Texture* tmedium = createText("medium", mouseAway);
	Texture* tmedium2 = createText("medium", mouseOver);
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
		if (mouseOver(easy, x, y) && event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT  ) {
			next_ = ProgramState::StartEasy;
		}
		else if (mouseOver(medium, x, y) && event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
			next_ = ProgramState::StartMedium;
		}
		else if (mouseOver(hard, x, y) && event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
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

GameMap::GameMap(TextureSupport& ts, StateState state) : ProgramState(ts) {
	difficulty = state;
	auto playerp = make_unique<Player>(textureS_.getWdith() / 2, textureS_.getHeight() - 200, textureS_, *this);
	player = playerp.get();
	addPO(move(playerp),false);

	SDL_Color wallColor = { 60,60,60,255 };
	int thickness = 30;
	addIS(unique_ptr<Wall>(new Wall(0, 0, textureS_, thickness, textureS_.getHeight(), wallColor)),true);
	addIS(unique_ptr<Wall>(new Wall(0, 0, textureS_, textureS_.getWdith(), thickness, wallColor)), true);
	addIS(unique_ptr<Wall>(new Wall(textureS_.getWdith() - thickness, 0, textureS_, thickness, textureS_.getHeight(), wallColor)), true);
	addIS(unique_ptr<Wall>(new Wall(0, textureS_.getHeight() - thickness, textureS_, textureS_.getWdith(), thickness, wallColor)), true);
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
}

void GameMap::moveObjects(Uint32 miliseconds)
{
	for (auto && mo : movingObjects) {
		if (mo->vx != 0 || mo->vy != 0) {
			int dx = mo->vx*miliseconds/1000;
			int dy = mo->vy*miliseconds/1000;

			mo->x_ += dx;
			mo->y_ += dy;

			for (auto && co : solidObjects) {
				if (mo->cantCollideWith() != co && resolveCollision(mo, co, dx, dy)) {
					break;
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
	// return moving object in direction of the collision
	if (collisionType == Collider::Horizontal || collisionType == Collider::Diagonal) m->x_ -= dx;
	if (collisionType == Collider::Vertical || collisionType == Collider::Diagonal) m->y_ -= dy;
	// distribute velocities
	return true;
}

void GameMap::updatePlayerFromInupt() {
	if (player->physicalMovement == true) return;
	int keysDown = 0;
	for (size_t i = 0; i < 4; i++)
	{
		if (wasdDown[i] == true) ++keysDown;
	}
	float addVelocity = 0;
	if (keysDown == 1) addVelocity = player->getSpeed();
	else if (keysDown == 2) addVelocity = player->getDiagSpeed();

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
	for (auto&& o : floorObjects) {
		o->update(miliseconds);
	}
	for (auto&& o : basicObjects) {
		o->update(miliseconds);
	}
	return StateState::Running;
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

void GameMap::render() {
	for (auto&& o: floorObjects) {
		o->render();
	}
	for (auto&& o : basicObjects) {
		o->render();
	}
}
