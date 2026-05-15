#ifndef VUID_SDL3_H
#define VUID_SDL3_H

#include <SDL3/SDL.h>
#include <vuid.h>

static inline bool v_sdl3_handle_event(SDL_Event* event, SDL_Renderer* renderer)
{
  float x;
  float y;

  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
      if (SDL_RenderCoordinatesFromWindow(renderer, event->button.x,
                                          event->button.y, &x, &y)) {
        const VMouseButtonData data = {
            .button = v_itx_map_mouse_button(event->button.button),
            .x = x,
            .y = y,
            .clicks = event->button.clicks,
            .down = (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN),
        };
        v_inject_mouse_button(&data);
      }
      break;
    case SDL_EVENT_MOUSE_WHEEL:
      if (SDL_RenderCoordinatesFromWindow(renderer, event->wheel.mouse_x,
                                          event->wheel.mouse_y, &x, &y)) {
        const VMouseWheelData data = {
            .x = event->wheel.x,
            .y = event->wheel.y,
            .mouse_x = x,
            .mouse_y = y,
            .direction =
                (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0f
                                                                  : 1.0f),
        };
        v_inject_mouse_wheel(&data);
      }
      break;
    case SDL_EVENT_MOUSE_MOTION:
      if (SDL_RenderCoordinatesFromWindow(renderer, event->motion.x,
                                          event->motion.y, &x, &y)) {
        const VMouseMoveData data = {
            .x = x,
            .y = y,
            .relative_x = event->motion.xrel,
            .relative_y = event->motion.yrel,
        };
        v_inject_mouse_move(&data);
      }
      break;
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
      VKey key;
      uint32_t modifiers;

      if (v_itx_map_key(event->key.key, event->key.scancode, event->key.mod,
                        &key, &modifiers)) {
        v_inject_key(key, modifiers, event->key.repeat,
                     (event->type == SDL_EVENT_KEY_DOWN));
      }
      break;
    }
    default:
      return false;
  }

  return true;
}

#endif  // VUID_SDL3_H
