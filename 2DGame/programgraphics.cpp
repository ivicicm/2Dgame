#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <memory>
#include "programgraphics.h"


Texture* TextureSupport::getTexture(std::string name, int inRow, SDL_Color transparent) {
	if (textures_.count(name) == 0) {
		textures_[name] = std::make_unique<Texture>(*this, "assets/" + name,inRow,transparent);
	}

	return textures_.at(name).get();
}

// transparent - what color to ignore when loading the texture
Texture::Texture(TextureSupport& ts, std::string path, int inRow, SDL_Color transparent) : textureS_(ts) {
	SDL_Surface* surface = IMG_Load(path.c_str());
	SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, transparent.r, transparent.g, transparent.b));
	texture_ = SDL_CreateTextureFromSurface(textureS_.getRenderer(), surface);
	width_ = surface->w / inRow;
	height_ = surface->h;
	SDL_FreeSurface(surface);
	inRow_ = inRow;
}

// create texture from font 
Texture::Texture(TextureSupport& ts, std::string text, SDL_Color color) :textureS_(ts) {
	SDL_Surface* surface = TTF_RenderText_Blended(ts.getFont(), text.c_str(), color);
	texture_ = SDL_CreateTextureFromSurface(textureS_.getRenderer(), surface);
	width_ = surface->w;
	height_ = surface->h;
	SDL_FreeSurface(surface);
}

// order - order of image in a row
void Texture::render(int x, int y, int order, bool relativeToCamera,double angle, SDL_Point* center) {
	
	SDL_Rect to = { x,y,width_,height_ };
	if (relativeToCamera) {
		to.x = to.x - textureS_.getCX();
		to.y = to.y - textureS_.getCY();
	}
	SDL_Rect from = { width_*order,0,width_,height_ };
	SDL_RenderCopyEx(textureS_.getRenderer(), texture_, &from, &to, angle, center, SDL_FLIP_NONE);
}