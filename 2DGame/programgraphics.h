#pragma once
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <map>
#include <memory>

class Texture;

class TextureSupport {
public:
	TextureSupport() {
	}
	TextureSupport(const TextureSupport&) = delete;
	TextureSupport& operator=(const TextureSupport&) = delete;
	void initialize(SDL_Renderer* renderer,int width,int height) {
		width_ = width;
		height_ = height;
		renderer_ = renderer;
		cameraX_ = 0;
		cameraY_ = 0;
		boldFont = TTF_OpenFont("assets/theboldfont.ttf", 45);
	}
	// for rendering relative to camera position
	void setCamera(int x, int y) {
		cameraX_ = x;
		cameraY_ = y;
	}
	int getCameraX() { return cameraX_; }
	int getCameraY() { return cameraY_; }
	// returns screen width
	int getWidth() { return width_; }
	int getHeight() { return height_; }
	SDL_Renderer* getRenderer() { return renderer_; }
	TTF_Font* getFont() { return boldFont; }
	// returns stored texture or loads new from address "assets/"name
	// inRow - how many pictures are next to eachother in the texture
	Texture* getTexture(std::string name, int inRow = 1, SDL_Color transparent = { 255,0,0,0 });
private:
	SDL_Renderer* renderer_ = nullptr;
	int cameraX_;
	int cameraY_;
	int width_;
	int height_;
	std::map<std::string, std::unique_ptr<Texture>> textures_;
	std::map<std::string, std::unique_ptr<Texture>> textTextures_;
	// only supports one font
	TTF_Font* boldFont;
};

class Texture {
public:
	// transprent - what color to ignore when loading the texture
	Texture(TextureSupport& ts, std::string path, int inRow = 1, SDL_Color transparent = { 0,255,0,0 });

	// create texture from text
	Texture(TextureSupport & ts, std::string text, SDL_Color color);

	~Texture() {
		SDL_DestroyTexture(texture_);
	}
	int getWidth() { return width_; }
	int getHeight() { return height_; }
	// order - order of image in a row
	void render(int x, int y, int order = 0, bool relativeToCamera = true, double angle = 0, SDL_Point* center = nullptr);
private:
	SDL_Texture* texture_;
	TextureSupport& textureS_;
	int width_, height_;
	int inRow_; // how many images are in a row next to each other - for example different animation frames
};


