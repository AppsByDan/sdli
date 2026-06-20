#ifndef VUID_SDL3_RENDERER_H
#define VUID_SDL3_RENDERER_H

#include <SDL3/SDL.h>
#include <vuid.h>

bool v_sdl3_renderer_init(SDL_Renderer* renderer);
void v_sdl3_renderer_shutdown(void);

void v_sdl3_renderer_render(void);

#endif  // VUID_SDL3_RENDERER_H
