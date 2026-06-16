#ifndef VUID_SDL3_H
#define VUID_SDL3_H

#include <SDL3/SDL.h>
#include <vuid.h>

typedef struct VSDL3RenderData {
  SDL_Renderer* renderer;
  SDL_FColor* color_buffer;
  size_t color_buffer_capacity;
} VSDL3RenderData;

bool v_sdl3_init(SDL_Renderer* renderer);
void v_sdl3_shutdown(void);

void v_sdl3_process_event(SDL_Event* event);
void v_sdl3_render(void);

bool v_sdl3_image_loader(VImageLoaderOp op,
                         VImageBuffer* buffer,
                         const void* file_data,
                         size_t file_size);

#endif /* VUID_SDL3_H */
