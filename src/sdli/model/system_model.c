#include "model.h"

#include <sdli/app.h>
#include <sdli/util.h>

#include <SDL3/SDL.h>
#include <stc/cstr.h>

//
// private types
//

typedef struct SystemModel {
  Sint64 last_power_info_update;
  cstr sdl_version;
  SDL_SystemTheme theme;
  SDL_PowerState power_state;
  int battery_level;
} SystemModel;

//
// private function declarations
//

static void OnSystemThemeChanged(int event_type,
                                 void* event_data,
                                 void* user_data);
static void OnBeforeEvents(int event_type, void* event_data, void* user_data);
static void UpdatePowerInfo(void);

//
// global state
//

static const Uint64 POWER_INFO_UPDATE_INTERVAL_MS = 60 * 1000;
static SystemModel g_system_model = {0};

//
// public function implementation
//

void SystemModel_Init(void)
{
  cstr_append_fmt(&g_system_model.sdl_version, "%i.%i.%i", SDL_MAJOR_VERSION,
                  SDL_MINOR_VERSION, SDL_MICRO_VERSION);

  g_system_model.theme = SDL_GetSystemTheme();
  UpdatePowerInfo();

  App_AddEventListener(SDL_EVENT_SYSTEM_THEME_CHANGED, OnSystemThemeChanged,
                       NULL);
  App_AddEventListener(EVT_BEFORE_PROCESS_EVENTS, OnBeforeEvents, NULL);
}

void SystemModel_Drop(void)
{
  cstr_drop(&g_system_model.sdl_version);
  g_system_model = (SystemModel){0};
}

const char* SystemModel_GetSdlVersion(void)
{
  return cstr_str(&g_system_model.sdl_version);
}

const char* SystemModel_GetSdlRevision(void)
{
  return EnsureString(SDL_GetRevision(), "");
}

const char* SystemModel_GetPlatformName(void)
{
  return EnsureString(SDL_GetPlatform(), "UNKNOWN");
}

const char* SystemModel_GetTheme(void)
{
  switch (g_system_model.theme) {
    case SDL_SYSTEM_THEME_DARK:
      return "DARK";
    case SDL_SYSTEM_THEME_LIGHT:
      return "LIGHT";
    default:
      return "UNKNOWN";
  }
}
const char* SystemModel_GetPowerState(void)
{
  switch (g_system_model.power_state) {
    case SDL_POWERSTATE_ERROR:
      return "ERROR";
    case SDL_POWERSTATE_ON_BATTERY:
      return "ON_BATTERY";
    case SDL_POWERSTATE_NO_BATTERY:
      return "NO_BATTERY";
    case SDL_POWERSTATE_CHARGING:
      return "CHARGING";
    case SDL_POWERSTATE_CHARGED:
      return "CHARGED";
    case SDL_POWERSTATE_UNKNOWN:
    default:
      return "UNKNOWN";
  }
}

int SystemModel_GetBatteryLevel(void)
{
  return g_system_model.battery_level;
}

int SystemModel_GetCpuCoreCount(void)
{
  return SDL_GetNumLogicalCPUCores();
}

int SystemModel_GetCpuCacheLineSize(void)
{
  return SDL_GetCPUCacheLineSize();
}

int SystemModel_GetRamMiB(void)
{
  return SDL_GetSystemRAM();
}

//
// private function implementation
//

static void OnSystemThemeChanged(int event_type,
                                 void* event_data,
                                 void* user_data)
{
  UNUSED(event_type, event_data, user_data);
  SDL_SystemTheme theme = SDL_GetSystemTheme();

  if (theme != g_system_model.theme) {
    g_system_model.theme = theme;
    // TODO: notify of theme change
  }
}

static void OnBeforeEvents(int event_type, void* event_data, void* user_data)
{
  UNUSED(event_type, event_data, user_data);
  Uint64 ticks = SDL_GetTicks();

  if (ticks - g_system_model.last_power_info_update >=
      POWER_INFO_UPDATE_INTERVAL_MS) {
    g_system_model.last_power_info_update = ticks;
    UpdatePowerInfo();
  }
}

static void UpdatePowerInfo(void)
{
  int battery_level = -1;
  SDL_PowerState power_state = SDL_GetPowerInfo(NULL, &battery_level);

  if (power_state == SDL_POWERSTATE_ERROR ||
      power_state == SDL_POWERSTATE_UNKNOWN ||
      power_state == SDL_POWERSTATE_NO_BATTERY) {
    battery_level = -1;
  }

  if (power_state != g_system_model.power_state ||
      battery_level != g_system_model.battery_level) {
    g_system_model.power_state = power_state;
    g_system_model.battery_level = battery_level;
    // TODO: notify of power info change
  }
}
