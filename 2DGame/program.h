#pragma once
#include "programgraphics.h"
#include "gamemap.h"
#include <SDL.h>
#include <memory>

class Program {
public:
	Program();

	~Program();
	
	void run();

private:
	void renderProgramState();
	static const int windowWidth = 1280;
	static const int windowHeight = 720;
	SDL_Window* window_;
	SDL_Renderer* renderer_;
	TextureSupport textureS_;
	std::unique_ptr<ProgramState> pstate_;
};
