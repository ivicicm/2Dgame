
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <memory>
#include "programgraphics.h"
#include "gamemap.h"
#include "program.h"

Program::Program() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	window_ = SDL_CreateWindow("2DGame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
	renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	textureS_.initialize(renderer_, windowWidth, windowHeight);
	// program starts with menu screen
	pstate_ = std::make_unique<GameMenu>(textureS_, ProgramState::ProgramStart);
}

Program::~Program() {
	TTF_CloseFont(textureS_.getFont());
	SDL_DestroyRenderer(renderer_);
	SDL_DestroyWindow(window_);
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}

void Program::run() {
	Uint32 time = 0;
	SDL_Event event;
	while (true) {
		// handle all queued events
		while (SDL_PollEvent(&event) != 0)
		{
			// closing the window
			if (event.type == SDL_QUIT)
			{
				return;
			}

			pstate_->handleEvent(event);
		}
		Uint32 newTime = SDL_GetTicks();
		ProgramState::StateState s = pstate_->update(newTime - time);
		time = newTime;
		
		switch (s)
		{
		case ProgramState::Running:
			renderProgramState();
			break;
		case ProgramState::StartEasy:
		case ProgramState::StartMedium:
		case ProgramState::StartHard:
			pstate_ = std::make_unique<GameMap>(textureS_, s);
			break;
		case ProgramState::Won:
		case ProgramState::Lost:
			pstate_ = std::make_unique<GameMenu>(textureS_, s);
		default:
			break;
		}
	}
}

void Program::renderProgramState(){
	// black background
 	SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer_);
	pstate_->render();
	SDL_RenderPresent(renderer_);
}
