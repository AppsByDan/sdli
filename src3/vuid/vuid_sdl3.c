#include "vuid_sdl3.h"

#include <SDL3/SDL.h>
#include <vuid.h>

static uint32_t v_sdl3_map_modifier(int raw_modifiers);
static VKey v_sdl3_map_key(SDL_Keycode keycode, SDL_Scancode scancode);
static VMouseButton v_sdl3_map_mouse_button(int button);

bool v_sdl3_init(void) {
  return true;
}

void v_sdl3_shutdown(void) {}

void v_sdl3_process_event(SDL_Event* event) {
  VInputEvent ie = {0};

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
    case SDL_EVENT_WINDOW_MOUSE_LEAVE: {
      // send a mouse move that is outside of vuid's root
      // - SDL_GetMouseState() returns that last known position which is inside
      //   the root. Send a (-1, -1), as we 100% know it is outside the root.
      // - No need to SDL_EVENT_WINDOW_MOUSE_ENTER, as mouse move event will
      //   handle that.
      ie.type = V_INPUT_EVENT_MOUSE_MOVE;
      ie.u.mouse_move.x = -1;
      ie.u.mouse_move.y = -1;
      ie.u.mouse_move.relative_x = 0;
      ie.u.mouse_move.relative_y = 0;
      break;
    }
    default:
      return;
  }

  v_add_input_event(&ie);
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
