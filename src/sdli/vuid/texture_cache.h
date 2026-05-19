#ifndef SDLI_VUID_TEXTURE_CACHE_H
#define SDLI_VUID_TEXTURE_CACHE_H

#include <SDL3/SDL.h>

//
// types
//

typedef struct TextureCache TextureCache;

//
// public function declarations
//

void TextureCache_Init(SDL_Renderer* renderer);
void TextureCache_Drop(void);
SDL_Texture* TextureCache_Get(uint16_t border_radius, uint16_t border_width);

#endif  // SDLI_VUID_TEXTURE_CACHE_H
