#include "vuid_sdl3.h"

#include <SDL3/SDL.h>
#include <vuid.h>

static uint32_t v_sdl3_map_modifier(int raw_modifiers);
static VKey v_sdl3_map_key(SDL_Keycode keycode, SDL_Scancode scancode);
static VMouseButton v_sdl3_map_mouse_button(int button);
static bool v_sdl3_upload_texture(SDL_Texture* texture,
                                  const VImageBuffer* pixels);
static bool v_sdl3_check_update_rect(const VTextureInfo* info);
static SDL_Texture* v_sdl3_create_texture(SDL_Renderer* renderer,
                                          const VTextureInfo* info);
static const SDL_FColor* v_sdl3_to_color_array(VSDL3RenderData* render_data,
                                               const VVertex* vertices,
                                               uint32_t vertex_count);

bool v_sdl3_init(SDL_Renderer* renderer) {
  // TODO: use allocator from vuid
  VSDL3RenderData* render_data = SDL_calloc(1, sizeof(VSDL3RenderData));

  if (!render_data) {
    return false;
  }

  render_data->renderer = renderer;

  VRenderData* rm = v_get_render_data();

  rm->integration_data = render_data;

  return true;
}

void v_sdl3_shutdown(void) {
  VRenderData* rm = v_get_render_data();

  for (uint32_t i = 0; i < rm->texture_count; i++) {
    VTextureInfo* info = &rm->textures[i];
    if (info->integration_data) {
      SDL_DestroyTexture((SDL_Texture*)info->integration_data);
      info->integration_data = NULL;
    }
    info->state = V_TEXTURE_DESTROYED;
  }

  VSDL3RenderData* render_data = rm->integration_data;

  if (render_data) {
    SDL_free(render_data->color_buffer);
    SDL_free(render_data);
    rm->integration_data = NULL;
  }
}

void v_sdl3_process_event(SDL_Event* event) {
  VInputEvent ie = {0};
  VSDL3RenderData* render_data = v_get_render_data()->integration_data;
  SDL_Renderer* renderer = render_data->renderer;

  SDL_ConvertEventToRenderCoordinates(renderer, (SDL_Event*)event);

  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      VMouseButton btn = v_sdl3_map_mouse_button(event->button.button);
      if (btn != V_MOUSE_BUTTON_INVALID) {
        ie.type = V_INPUT_EVENT_MOUSE_BUTTON;
        ie.u.mouse_button.button = btn;
        ie.u.mouse_button.x = event->button.x;
        ie.u.mouse_button.y = event->button.y;
        ie.u.mouse_button.clicks = event->button.clicks;
        ie.u.mouse_button.down = (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN);
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION:
      ie.type = V_INPUT_EVENT_MOUSE_MOVE;
      ie.u.mouse_move.x = event->motion.x;
      ie.u.mouse_move.y = event->motion.y;
      ie.u.mouse_move.relative_x = event->motion.xrel;
      ie.u.mouse_move.relative_y = event->motion.yrel;
      break;
    case SDL_EVENT_MOUSE_WHEEL:
      ie.type = V_INPUT_EVENT_MOUSE_WHEEL;
      ie.u.mouse_wheel.x = event->wheel.x;
      ie.u.mouse_wheel.y = event->wheel.y;
      ie.u.mouse_wheel.mouse_x = event->wheel.mouse_x;
      ie.u.mouse_wheel.mouse_y = event->wheel.mouse_y;
      ie.u.mouse_wheel.direction =
          (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0f : 1.0f);
      break;
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
      VKey key = v_sdl3_map_key(event->key.key, event->key.scancode);

      if (key != V_KEY_INVALID) {
        ie.type = V_INPUT_EVENT_KEY;
        ie.u.key.key = key;
        ie.u.key.modifiers = v_sdl3_map_modifier(event->key.mod);
        ie.u.key.repeat_count = event->key.repeat ? 1 : 0;
        ie.u.key.down = (event->type == SDL_EVENT_KEY_DOWN);
      }
      break;
    }
    default:
      return;
  }

  v_add_input_event(&ie);
}

void v_sdl3_render(void) {
  VRenderData* rm = v_get_render_data();
  VSDL3RenderData* integration = rm->integration_data;
  SDL_Renderer* renderer = integration->renderer;

  for (uint32_t i = 0; i < rm->texture_count; i++) {
    VTextureInfo* info = &rm->textures[i];

    switch (info->state) {
      case V_TEXTURE_CREATING: {
        if (info->integration_data) {
          SDL_DestroyTexture(info->integration_data);
          info->integration_data = NULL;
        }

        SDL_Texture* tex = v_sdl3_check_update_rect(info)
                               ? v_sdl3_create_texture(renderer, info)
                               : NULL;

        if (tex) {
          if (v_sdl3_upload_texture(tex, &info->pixels)) {
            info->integration_data = tex;
            info->state = V_TEXTURE_READY;
          } else {
            SDL_DestroyTexture(tex);
          }
        }
        break;
      }
      case V_TEXTURE_UPDATING: {
        SDL_Texture* tex = info->integration_data;

        if (tex && v_sdl3_check_update_rect(info)) {
          if (v_sdl3_upload_texture(tex, &info->pixels)) {
            info->state = V_TEXTURE_READY;
          }
        }
        break;
      }
      case V_TEXTURE_DESTROYING:
        if (info->integration_data) {
          SDL_DestroyTexture(info->integration_data);
          info->integration_data = NULL;
        }
        break;
      default:
        break;
    }
  }

  for (uint32_t i = 0; i < rm->command_count; i++) {
    const VCommand* cmd = &rm->commands[i];
    switch (cmd->type) {
      case V_COMMAND_SET_CLIP: {
        if (cmd->u.set_clip.clear) {
          SDL_SetRenderClipRect(renderer, NULL);
        } else {
          SDL_Rect clip_rect = {
              .x = cmd->u.set_clip.x,
              .y = cmd->u.set_clip.y,
              .w = cmd->u.set_clip.w,
              .h = cmd->u.set_clip.h,
          };
          SDL_SetRenderClipRect(renderer, &clip_rect);
        }
        break;
      }
      case V_COMMAND_RENDER: {
        const VVertex* start_vertex =
            &rm->vertices[cmd->u.render.vertex_offset];
        const int* indices =
            (const int*)&rm->indices[cmd->u.render.index_offset];
        const int stride = (int)sizeof(VVertex);
        const SDL_FColor* color = v_sdl3_to_color_array(
            integration, start_vertex, cmd->u.render.vertex_count);
        SDL_Texture* texture = NULL;

        if (cmd->u.render.texture_id != 0) {
          for (uint32_t i = 0; i < rm->texture_count; i++) {
            if (rm->textures[i].id == cmd->u.render.texture_id) {
              texture = rm->textures[i].integration_data;
              break;
            }
          }
        }

        SDL_RenderGeometryRaw(
            renderer,                    // renderer
            texture,                     // texture (NULL for solid rect)
            &start_vertex->x,            // xy pointer
            stride,                      // xy stride
            color,                       // color pointer
            (int)sizeof(SDL_FColor),     // color stride
            &start_vertex->u,            // uv pointer
            stride,                      // uv stride
            cmd->u.render.vertex_count,  // num_vertices
            indices,                     // indices pointer
            cmd->u.render.index_count,   // num_indices
            (int)sizeof(rm->indices[0])  // size of each index (4 bytes)
        );
        break;
      }
      default:
        break;
    }
  }
}

bool v_sdl3_image_loader(VImageLoaderOp op,
                         VImageBuffer* buffer,
                         const void* file_data,
                         size_t file_size) {
  if (op == V_IMAGE_LOADER_OP_FREE) {
    if (buffer->idata) {
      SDL_DestroySurface(buffer->idata);
    }
  } else if (op == V_IMAGE_LOADER_OP_LOAD) {
    SDL_IOStream* file_io = SDL_IOFromConstMem(file_data, file_size);

    if (!file_io) {
      SDL_Log("v_sdl3_image_loader: SDL_IOFromConstMem failed: %s\n",
              SDL_GetError());
      return false;
    }

    SDL_Surface* surface = SDL_LoadBMP_IO(file_io, true);

    if (!surface) {
      SDL_Log("v_sdl3_image_loader: SDL_LoadBMP_IO failed: %s\n",
              SDL_GetError());
      return false;
    }

    if (surface->format != SDL_PIXELFORMAT_RGBA32) {
      SDL_Surface* converted_surface =
          SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
      SDL_DestroySurface(surface);
      surface = converted_surface;

      if (!surface) {
        SDL_Log("v_sdl3_image_loader: SDL_ConvertSurfaceFormat failed: %s\n",
                SDL_GetError());
        return false;
      }
    }

    *buffer = (VImageBuffer){
        .bytes = (const uint8_t*)surface->pixels,
        .format = V_PIXEL_FORMAT_RGBA8,
        .width = surface->w,
        .height = surface->h,
        .pitch = surface->pitch,
        .idata = surface,
    };
  }

  return true;
}

static bool v_sdl3_upload_texture(SDL_Texture* texture,
                                  const VImageBuffer* pixels) {
  Uint32* dest_pixels;
  int dest_pitch;

  if (!SDL_LockTexture(texture, NULL, (void**)&dest_pixels, &dest_pitch)) {
    SDL_Log("v_sdl3_upload_texture: SDL_LockTexture failed: %s\n",
            SDL_GetError());
    return false;
  }

  bool result = true;
  SDL_PixelFormat texture_format = SDL_GetNumberProperty(
      SDL_GetTextureProperties(texture), SDL_PROP_TEXTURE_FORMAT_NUMBER,
      SDL_PIXELFORMAT_UNKNOWN);

  if (pixels->format == V_PIXEL_FORMAT_A8) {
    const uint32_t pitch_u32 = (uint32_t)(dest_pitch / 4);
    const SDL_PixelFormatDetails* texture_format_details =
        SDL_GetPixelFormatDetails(texture_format);

    uint32_t src_row_bytes = pixels->pitch > 0 ? pixels->pitch : pixels->width;
    for (uint32_t y = 0; y < pixels->height; y++) {
      const uint8_t* src_row = pixels->bytes + y * src_row_bytes;
      Uint32* dst_row = dest_pixels + y * pitch_u32;
      for (uint32_t x = 0; x < pixels->width; x++) {
        dst_row[x] = SDL_MapRGBA(texture_format_details, NULL, 255, 255, 255,
                                 src_row[x]);
      }
    }
  } else if (pixels->format == V_PIXEL_FORMAT_RGBA8) {
    SDL_ConvertPixels((int)pixels->width, (int)pixels->height,
                      SDL_PIXELFORMAT_RGBA32, pixels->bytes, (int)pixels->pitch,
                      texture_format, dest_pixels, dest_pitch);
  } else {
    result = false;
  }

  SDL_UnlockTexture(texture);
  return result;
}

static bool v_sdl3_check_update_rect(const VTextureInfo* info) {
  if (!info->pixels.bytes) {
    return false;
  }

  if (info->pixels.format != V_PIXEL_FORMAT_A8 &&
      info->pixels.format != V_PIXEL_FORMAT_RGBA8) {
    return false;
  }

  if (info->pixels.width == 0 || info->pixels.height == 0) {
    return false;
  }

  return info->pixels.width == info->width &&
         info->pixels.height == info->height;
}

static SDL_Texture* v_sdl3_create_texture(SDL_Renderer* renderer,
                                          const VTextureInfo* info) {
  SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                       SDL_TEXTUREACCESS_STREAMING,
                                       (int)info->width, (int)info->height);

  if (tex) {
    // TODO: this should be a state change with render commands
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
  } else {
    // TODO: use vuid to log
    SDL_Log("v_sdl3_render: SDL_CreateTexture failed: %s\n", SDL_GetError());
  }

  return tex;
}

static const SDL_FColor* v_sdl3_to_color_array(VSDL3RenderData* render_data,
                                               const VVertex* vertices,
                                               uint32_t vertex_count) {
  if (render_data->color_buffer_capacity < vertex_count) {
    const uint32_t new_capacity = render_data->color_buffer_capacity == 0
                                      ? 256
                                      : render_data->color_buffer_capacity * 2;
    // TODO: use allocator from vuid
    render_data->color_buffer = SDL_realloc(render_data->color_buffer,
                                            new_capacity * sizeof(SDL_FColor));
    render_data->color_buffer_capacity = new_capacity;
  }

  for (uint32_t i = 0; i < vertex_count; i++) {
    render_data->color_buffer[i].r = vertices[i].r / 255.0f;
    render_data->color_buffer[i].g = vertices[i].g / 255.0f;
    render_data->color_buffer[i].b = vertices[i].b / 255.0f;
    render_data->color_buffer[i].a = vertices[i].a / 255.0f;
  }

  return render_data->color_buffer;
}

static VMouseButton v_sdl3_map_mouse_button(int button) {
  switch (button) {
    case SDL_BUTTON_LEFT:
      return V_MOUSE_BUTTON_LEFT;
    case SDL_BUTTON_RIGHT:
      return V_MOUSE_BUTTON_RIGHT;
    case SDL_BUTTON_MIDDLE:
      return V_MOUSE_BUTTON_MIDDLE;
    default:
      return V_MOUSE_BUTTON_INVALID;
  }
}

static VKey v_sdl3_map_key(SDL_Keycode keycode, SDL_Scancode scancode) {
#define CASE_MAP_KEY(SDL_KEY, V_KEY) \
  case SDL_KEY:                      \
    return V_KEY
#define FALLTHROUGH() \
  default:            \
    break

  switch (scancode) {
    CASE_MAP_KEY(SDL_SCANCODE_KP_0, V_KEY_KEYPAD_0);
    CASE_MAP_KEY(SDL_SCANCODE_KP_1, V_KEY_KEYPAD_1);
    CASE_MAP_KEY(SDL_SCANCODE_KP_2, V_KEY_KEYPAD_2);
    CASE_MAP_KEY(SDL_SCANCODE_KP_3, V_KEY_KEYPAD_3);
    CASE_MAP_KEY(SDL_SCANCODE_KP_4, V_KEY_KEYPAD_4);
    CASE_MAP_KEY(SDL_SCANCODE_KP_5, V_KEY_KEYPAD_5);
    CASE_MAP_KEY(SDL_SCANCODE_KP_6, V_KEY_KEYPAD_6);
    CASE_MAP_KEY(SDL_SCANCODE_KP_7, V_KEY_KEYPAD_7);
    CASE_MAP_KEY(SDL_SCANCODE_KP_8, V_KEY_KEYPAD_8);
    CASE_MAP_KEY(SDL_SCANCODE_KP_9, V_KEY_KEYPAD_9);
    CASE_MAP_KEY(SDL_SCANCODE_KP_PERIOD, V_KEY_KEYPAD_DECIMAL);
    CASE_MAP_KEY(SDL_SCANCODE_KP_DIVIDE, V_KEY_KEYPAD_DIVIDE);
    CASE_MAP_KEY(SDL_SCANCODE_KP_MULTIPLY, V_KEY_KEYPAD_MULTIPLY);
    CASE_MAP_KEY(SDL_SCANCODE_KP_MINUS, V_KEY_KEYPAD_SUBTRACT);
    CASE_MAP_KEY(SDL_SCANCODE_KP_PLUS, V_KEY_KEYPAD_ADD);
    CASE_MAP_KEY(SDL_SCANCODE_KP_ENTER, V_KEY_KEYPAD_ENTER);
    CASE_MAP_KEY(SDL_SCANCODE_KP_EQUALS, V_KEY_KEYPAD_EQUAL);
    FALLTHROUGH();
  }

  switch (keycode) {
    CASE_MAP_KEY(SDLK_TAB, V_KEY_TAB);
    CASE_MAP_KEY(SDLK_LEFT, V_KEY_LEFT_ARROW);
    CASE_MAP_KEY(SDLK_RIGHT, V_KEY_RIGHT_ARROW);
    CASE_MAP_KEY(SDLK_UP, V_KEY_UP_ARROW);
    CASE_MAP_KEY(SDLK_DOWN, V_KEY_DOWN_ARROW);
    CASE_MAP_KEY(SDLK_PAGEUP, V_KEY_PAGE_UP);
    CASE_MAP_KEY(SDLK_PAGEDOWN, V_KEY_PAGE_DOWN);
    CASE_MAP_KEY(SDLK_HOME, V_KEY_HOME);
    CASE_MAP_KEY(SDLK_END, V_KEY_END);
    CASE_MAP_KEY(SDLK_INSERT, V_KEY_INSERT);
    CASE_MAP_KEY(SDLK_DELETE, V_KEY_DELETE);
    CASE_MAP_KEY(SDLK_BACKSPACE, V_KEY_BACKSPACE);
    CASE_MAP_KEY(SDLK_SPACE, V_KEY_SPACE);
    CASE_MAP_KEY(SDLK_RETURN, V_KEY_ENTER);
    CASE_MAP_KEY(SDLK_ESCAPE, V_KEY_ESCAPE);
    CASE_MAP_KEY(SDLK_COMMA, V_KEY_COMMA);
    CASE_MAP_KEY(SDLK_PERIOD, V_KEY_PERIOD);
    CASE_MAP_KEY(SDLK_SEMICOLON, V_KEY_SEMICOLON);
    CASE_MAP_KEY(SDLK_CAPSLOCK, V_KEY_CAPS_LOCK);
    CASE_MAP_KEY(SDLK_SCROLLLOCK, V_KEY_SCROLL_LOCK);
    CASE_MAP_KEY(SDLK_NUMLOCKCLEAR, V_KEY_NUM_LOCK);
    CASE_MAP_KEY(SDLK_PRINTSCREEN, V_KEY_PRINT_SCREEN);
    CASE_MAP_KEY(SDLK_PAUSE, V_KEY_PAUSE);
    CASE_MAP_KEY(SDLK_LCTRL, V_KEY_LEFT_CTRL);
    CASE_MAP_KEY(SDLK_LSHIFT, V_KEY_LEFT_SHIFT);
    CASE_MAP_KEY(SDLK_LALT, V_KEY_LEFT_ALT);
    CASE_MAP_KEY(SDLK_LGUI, V_KEY_LEFT_SUPER);
    CASE_MAP_KEY(SDLK_RCTRL, V_KEY_RIGHT_CTRL);
    CASE_MAP_KEY(SDLK_RSHIFT, V_KEY_RIGHT_SHIFT);
    CASE_MAP_KEY(SDLK_RALT, V_KEY_RIGHT_ALT);
    CASE_MAP_KEY(SDLK_RGUI, V_KEY_RIGHT_SUPER);
    CASE_MAP_KEY(SDLK_APPLICATION, V_KEY_MENU);
    CASE_MAP_KEY(SDLK_0, V_KEY_0);
    CASE_MAP_KEY(SDLK_1, V_KEY_1);
    CASE_MAP_KEY(SDLK_2, V_KEY_2);
    CASE_MAP_KEY(SDLK_3, V_KEY_3);
    CASE_MAP_KEY(SDLK_4, V_KEY_4);
    CASE_MAP_KEY(SDLK_5, V_KEY_5);
    CASE_MAP_KEY(SDLK_6, V_KEY_6);
    CASE_MAP_KEY(SDLK_7, V_KEY_7);
    CASE_MAP_KEY(SDLK_8, V_KEY_8);
    CASE_MAP_KEY(SDLK_9, V_KEY_9);
    CASE_MAP_KEY(SDLK_A, V_KEY_A);
    CASE_MAP_KEY(SDLK_B, V_KEY_B);
    CASE_MAP_KEY(SDLK_C, V_KEY_C);
    CASE_MAP_KEY(SDLK_D, V_KEY_D);
    CASE_MAP_KEY(SDLK_E, V_KEY_E);
    CASE_MAP_KEY(SDLK_F, V_KEY_F);
    CASE_MAP_KEY(SDLK_G, V_KEY_G);
    CASE_MAP_KEY(SDLK_H, V_KEY_H);
    CASE_MAP_KEY(SDLK_I, V_KEY_I);
    CASE_MAP_KEY(SDLK_J, V_KEY_J);
    CASE_MAP_KEY(SDLK_K, V_KEY_K);
    CASE_MAP_KEY(SDLK_L, V_KEY_L);
    CASE_MAP_KEY(SDLK_M, V_KEY_M);
    CASE_MAP_KEY(SDLK_N, V_KEY_N);
    CASE_MAP_KEY(SDLK_O, V_KEY_O);
    CASE_MAP_KEY(SDLK_P, V_KEY_P);
    CASE_MAP_KEY(SDLK_Q, V_KEY_Q);
    CASE_MAP_KEY(SDLK_R, V_KEY_R);
    CASE_MAP_KEY(SDLK_S, V_KEY_S);
    CASE_MAP_KEY(SDLK_T, V_KEY_T);
    CASE_MAP_KEY(SDLK_U, V_KEY_U);
    CASE_MAP_KEY(SDLK_V, V_KEY_V);
    CASE_MAP_KEY(SDLK_W, V_KEY_W);
    CASE_MAP_KEY(SDLK_X, V_KEY_X);
    CASE_MAP_KEY(SDLK_Y, V_KEY_Y);
    CASE_MAP_KEY(SDLK_Z, V_KEY_Z);
    CASE_MAP_KEY(SDLK_F1, V_KEY_F1);
    CASE_MAP_KEY(SDLK_F2, V_KEY_F2);
    CASE_MAP_KEY(SDLK_F3, V_KEY_F3);
    CASE_MAP_KEY(SDLK_F4, V_KEY_F4);
    CASE_MAP_KEY(SDLK_F5, V_KEY_F5);
    CASE_MAP_KEY(SDLK_F6, V_KEY_F6);
    CASE_MAP_KEY(SDLK_F7, V_KEY_F7);
    CASE_MAP_KEY(SDLK_F8, V_KEY_F8);
    CASE_MAP_KEY(SDLK_F9, V_KEY_F9);
    CASE_MAP_KEY(SDLK_F10, V_KEY_F10);
    CASE_MAP_KEY(SDLK_F11, V_KEY_F11);
    CASE_MAP_KEY(SDLK_F12, V_KEY_F12);
    CASE_MAP_KEY(SDLK_F13, V_KEY_F13);
    CASE_MAP_KEY(SDLK_F14, V_KEY_F14);
    CASE_MAP_KEY(SDLK_F15, V_KEY_F15);
    CASE_MAP_KEY(SDLK_F16, V_KEY_F16);
    CASE_MAP_KEY(SDLK_F17, V_KEY_F17);
    CASE_MAP_KEY(SDLK_F18, V_KEY_F18);
    CASE_MAP_KEY(SDLK_F19, V_KEY_F19);
    CASE_MAP_KEY(SDLK_F20, V_KEY_F20);
    CASE_MAP_KEY(SDLK_F21, V_KEY_F21);
    CASE_MAP_KEY(SDLK_F22, V_KEY_F22);
    CASE_MAP_KEY(SDLK_F23, V_KEY_F23);
    CASE_MAP_KEY(SDLK_F24, V_KEY_F24);
    CASE_MAP_KEY(SDLK_AC_BACK, V_KEY_APP_BACK);
    CASE_MAP_KEY(SDLK_AC_FORWARD, V_KEY_APP_FORWARD);
    FALLTHROUGH();
  }

  // Fallback to scancode
  switch (scancode) {
    CASE_MAP_KEY(SDL_SCANCODE_GRAVE, V_KEY_GRAVE_ACCENT);
    CASE_MAP_KEY(SDL_SCANCODE_MINUS, V_KEY_MINUS);
    CASE_MAP_KEY(SDL_SCANCODE_EQUALS, V_KEY_EQUAL);
    CASE_MAP_KEY(SDL_SCANCODE_LEFTBRACKET, V_KEY_LEFT_BRACKET);
    CASE_MAP_KEY(SDL_SCANCODE_RIGHTBRACKET, V_KEY_RIGHT_BRACKET);
    CASE_MAP_KEY(SDL_SCANCODE_NONUSBACKSLASH, V_KEY_OEM_102);
    CASE_MAP_KEY(SDL_SCANCODE_BACKSLASH, V_KEY_BACKSLASH);
    CASE_MAP_KEY(SDL_SCANCODE_SEMICOLON, V_KEY_SEMICOLON);
    CASE_MAP_KEY(SDL_SCANCODE_APOSTROPHE, V_KEY_APOSTROPHE);
    CASE_MAP_KEY(SDL_SCANCODE_COMMA, V_KEY_COMMA);
    CASE_MAP_KEY(SDL_SCANCODE_PERIOD, V_KEY_PERIOD);
    CASE_MAP_KEY(SDL_SCANCODE_SLASH, V_KEY_SLASH);
    FALLTHROUGH();
  }

  return V_KEY_INVALID;

#undef CASE_MAP_KEY
#undef FALLTHROUGH
}

static uint32_t v_sdl3_map_modifier(int raw_modifiers) {
#define MAP_MODIFIER(sdl_mod, v_mod) \
  if (raw_modifiers & sdl_mod)       \
    modifiers |= v_mod;

  uint32_t modifiers = 0;

  MAP_MODIFIER(SDL_KMOD_LSHIFT, V_KMOD_LSHIFT);
  MAP_MODIFIER(SDL_KMOD_RSHIFT, V_KMOD_RSHIFT);
  MAP_MODIFIER(SDL_KMOD_LCTRL, V_KMOD_LCTRL);
  MAP_MODIFIER(SDL_KMOD_RCTRL, V_KMOD_RCTRL);
  MAP_MODIFIER(SDL_KMOD_LALT, V_KMOD_LALT);
  MAP_MODIFIER(SDL_KMOD_RALT, V_KMOD_RALT);
  MAP_MODIFIER(SDL_KMOD_LGUI, V_KMOD_LGUI);
  MAP_MODIFIER(SDL_KMOD_RGUI, V_KMOD_RGUI);
  MAP_MODIFIER(SDL_KMOD_NUM, V_KMOD_NUM);
  MAP_MODIFIER(SDL_KMOD_CAPS, V_KMOD_CAPS);
  MAP_MODIFIER(SDL_KMOD_MODE, V_KMOD_MODE);
  MAP_MODIFIER(SDL_KMOD_SCROLL, V_KMOD_SCROLL);

  return modifiers;

#undef MAP_MODIFIER
}
