#include <sdli/util.h>

#include <stdio.h>
#include <inttypes.h>

#include <SDL3/SDL.h>

#define CASE_EVENT_NAME(VAR, EVENT) \
  case EVENT:                       \
    VAR = #EVENT;                   \
    break

//
// public function implementation
//

void BindString(const char* id, const char* value)
{
  v_node_set_text(v_get_node_by_id(id), EnsureString(value, ""));
}

void BindInt(const char* id, int value)
{
  v_node_set_text_fmt(v_get_node_by_id(id), "%i", value);
}

void BindU32(const char* id, uint32_t val)
{
  v_node_set_text_fmt(v_get_node_by_id(id), "%" PRIu32, val);
}

void BindU64(const char* id, uint64_t val)
{
  v_node_set_text_fmt(v_get_node_by_id(id), "%" PRIu64, val);
}

void BindBool(const char* id, bool val)
{
  v_node_set_text(v_get_node_by_id(id), val ? "YES" : "NO");
}

void BindFloat(const char* id, float val, int digits)
{
  v_node_set_text_fmt(v_get_node_by_id(id), "%.*f", digits, val);
}

void SLog(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, fmt,
                  args);
  va_end(args);
}

const char* SGetError(void)
{
  return SDL_GetError();
}

void SLogCallError(const char* func_name)
{
  SLog("ERROR: %s(): %s", func_name, SDL_GetError());
}

void SLogCallErrorWithU64(const char* func_name, uint64_t value)
{
  SLog("ERROR: %s(%" PRIu64 "): %s", func_name, value, SDL_GetError());
}

void SLogEvent(void* sdl_event)
{
  SDL_Event* event = sdl_event;
  Uint32 event_type = event ? event->type : 0;
  const char* event_name;

  switch (event_type) {
    CASE_EVENT_NAME(event_name, SDL_EVENT_SYSTEM_THEME_CHANGED);
    CASE_EVENT_NAME(event_name, SDL_EVENT_JOYSTICK_ADDED);
    CASE_EVENT_NAME(event_name, SDL_EVENT_JOYSTICK_REMOVED);
    CASE_EVENT_NAME(event_name, SDL_EVENT_GAMEPAD_ADDED);
    CASE_EVENT_NAME(event_name, SDL_EVENT_GAMEPAD_REMOVED);
    CASE_EVENT_NAME(event_name, SDL_EVENT_GAMEPAD_REMAPPED);
    CASE_EVENT_NAME(event_name, SDL_EVENT_JOYSTICK_BATTERY_UPDATED);
    CASE_EVENT_NAME(event_name, SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED);
    default:
      event_name = "UNKNOWN_EVENT";
      break;
  }

  SLog("[EVENT]: %s", event_name);
}
