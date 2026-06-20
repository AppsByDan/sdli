#ifndef VUID_SDL3_H
#define VUID_SDL3_H

#include <SDL3/SDL.h>
#include <vuid.h>

bool v_sdl3_init(void);
void v_sdl3_shutdown(void);

void v_sdl3_process_event(SDL_Event* event);

bool v_sdl3_image_loader(VImageLoaderOp op,
                         VImageBuffer* buffer,
                         const void* file_data,
                         size_t file_size);

#endif /* VUID_SDL3_H */
