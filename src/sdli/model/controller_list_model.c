#include <sdli/model/model.h>

#include <sdli/app.h>
#include <sdli/util.h>

#include <SDL3/SDL.h>
#include <stc/cstr.h>

//
// macros & constants
//

#define ReturnGamepadValue(ID, FN, DEFAULT)                              \
  do {                                                                   \
    const Controller* controller = GetController(ID);                    \
    return (controller && controller->gamepad) ? FN(controller->gamepad) \
                                               : DEFAULT;                \
  } while (0)

#define ReturnJoystickValue(ID, FN, DEFAULT)                               \
  do {                                                                     \
    const Controller* controller = GetController(ID);                      \
    return (controller && controller->joystick) ? FN(controller->joystick) \
                                                : DEFAULT;                 \
  } while (0)

static const char* JOYSTICK_PROPERTY_NAMES[] = {
    // clang-format off
    SDL_PROP_JOYSTICK_CAP_MONO_LED_BOOLEAN,
    SDL_PROP_JOYSTICK_CAP_RGB_LED_BOOLEAN,
    SDL_PROP_JOYSTICK_CAP_PLAYER_LED_BOOLEAN,
    SDL_PROP_JOYSTICK_CAP_RUMBLE_BOOLEAN,
    SDL_PROP_JOYSTICK_CAP_TRIGGER_RUMBLE_BOOLEAN,
    // clang-format on
};

//
// private types
//

typedef struct Controller {
  ControllerId id;

  cstr gamepad_name;
  cstr joystick_name;
  char guid[33];

  SDL_PowerState power_state;
  int battery_level;

  SDL_GamepadBinding** gamepad_bindings;
  int gamepad_bindings_count;

  SDL_Joystick* joystick;
  SDL_Gamepad* gamepad;

  ControllerProperty properties[c_arraylen(JOYSTICK_PROPERTY_NAMES)];
} Controller;

static void Controller_drop(Controller* controller);

#define i_implement
#define i_no_clone
#define i_no_emplace
#define i_key ControllerId
#define i_valclass Controller
#define i_type controller_map
#include <stc/hmap.h>

#define i_no_clone
#define i_no_emplace
#define i_type vec_controller_id
#define i_key ControllerId
#include <stc/vec.h>

#define i_key ControllerId
#define i_type u32
#include <stc/sort.h>

typedef struct ControllerListModel {
  controller_map controllers;
  vec_controller_id sorted_controller_ids;
} ControllerListModel;

//
// private function declarations
//

static Controller* GetController(ControllerId id);
static bool AddController(SDL_JoystickID id);
static void UpdatePowerInfo(Controller* controller);
static void UpdateGamepadById(SDL_JoystickID id);
static void UpdateGamepad(Controller* controller);
static void RemoveGamepadById(ControllerId id);
static void ClearGamepad(Controller* controller);
static void AppEventHandler(int event_type, void* event_data, void* user_data);
static void UpdateGUID(Controller* controller);
static void UpdateJoystickName(Controller* controller);
static void UpdateProperties(Controller* controller);

//
// global state
//

static ControllerListModel g_controller_list_model = {0};

//
// public function implementation
//

void ControllerListModel_Init(void)
{
  g_controller_list_model = (ControllerListModel){
      .controllers = controller_map_init(),
  };

  int count = 0;
  SDL_JoystickID* joysticks = SDL_GetJoysticks(&count);

  for (int i = 0; i < count; ++i) {
    // TODO: print warning if AddController fails
    AddController(joysticks[i]);
  }

  SDL_free(joysticks);

  App_AddEventListener(SDL_EVENT_JOYSTICK_ADDED, &AppEventHandler, NULL);
  App_AddEventListener(SDL_EVENT_JOYSTICK_REMOVED, &AppEventHandler, NULL);
  App_AddEventListener(SDL_EVENT_GAMEPAD_ADDED, &AppEventHandler, NULL);
  App_AddEventListener(SDL_EVENT_GAMEPAD_REMOVED, &AppEventHandler, NULL);
  App_AddEventListener(SDL_EVENT_GAMEPAD_REMAPPED, &AppEventHandler, NULL);
  App_AddEventListener(SDL_EVENT_JOYSTICK_BATTERY_UPDATED, &AppEventHandler,
                       NULL);
  App_AddEventListener(SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED, &AppEventHandler,
                       NULL);
}

void ControllerListModel_Drop(void)
{
  controller_map_drop(&g_controller_list_model.controllers);
  g_controller_list_model = (ControllerListModel){0};
}

ControllerId* ControllerListModel_SortControllers(int* out_count)
{
  vec_controller_id* ids = &g_controller_list_model.sorted_controller_ids;
  controller_map* controllers = &g_controller_list_model.controllers;

  vec_controller_id_clear(ids);

  c_foreach(it, controller_map, *controllers)
  {
    vec_controller_id_push_back(ids, it.ref->first);
  }

  u32_sort(ids->data, ids->size);
  *out_count = (int)ids->size;

  return ids->data;
}

const char* Controller_GetName(ControllerId id)
{
  Controller* controller = GetController(id);

  if (controller) {
    if (!cstr_is_empty(&controller->gamepad_name)) {
      return cstr_str(&controller->gamepad_name);
    } else if (!cstr_is_empty(&controller->joystick_name)) {
      return cstr_str(&controller->joystick_name);
    }
  }

  return "";
}

const char* Controller_GetGUID(ControllerId id)
{
  Controller* controller = GetController(id);
  return controller ? controller->guid : "";
}

const char* Controller_GetPath(ControllerId id)
{
  ReturnJoystickValue(id, SDL_GetJoystickPath, "");
}

const char* Controller_GetJoystickType(ControllerId id)
{
  switch (SDL_GetJoystickTypeForID(id)) {
    case SDL_JOYSTICK_TYPE_GAMEPAD:
      return "GAMEPAD";
    case SDL_JOYSTICK_TYPE_WHEEL:
      return "WHEEL";
    case SDL_JOYSTICK_TYPE_ARCADE_STICK:
      return "ARCADE_STICK";
    case SDL_JOYSTICK_TYPE_FLIGHT_STICK:
      return "FLIGHT_STICK";
    case SDL_JOYSTICK_TYPE_DANCE_PAD:
      return "DANCE_PAD";
    case SDL_JOYSTICK_TYPE_GUITAR:
      return "GUITAR";
    case SDL_JOYSTICK_TYPE_DRUM_KIT:
      return "DRUM_KIT";
    case SDL_JOYSTICK_TYPE_UNKNOWN:
    default:
      return "UNKNOWN";
  }
}

const char* Controller_GetGamepadType(ControllerId id)
{
  switch (SDL_GetGamepadTypeForID(id)) {
    case SDL_GAMEPAD_TYPE_STANDARD:
      return "STANDARD";
    case SDL_GAMEPAD_TYPE_XBOX360:
      return "XBOX360";
    case SDL_GAMEPAD_TYPE_XBOXONE:
      return "XBOXONE";
    case SDL_GAMEPAD_TYPE_PS3:
      return "PS3";
    case SDL_GAMEPAD_TYPE_PS4:
      return "PS4";
    case SDL_GAMEPAD_TYPE_PS5:
      return "PS5";
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
      return "NINTENDO_SWITCH_PRO";
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
      return "NINTENDO_SWITCH_JOYCON_LEFT";
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
      return "NINTENDO_SWITCH_JOYCON_RIGHT";
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
      return "NINTENDO_SWITCH_JOYCON_PAIR";
    case SDL_GAMEPAD_TYPE_UNKNOWN:
    default:
      return "UNKNOWN";
  }
}

const char* Controller_GetConnectionType(ControllerId id)
{
  Controller* controller = GetController(id);
  SDL_JoystickConnectionState connection_state;

  if (controller) {
    connection_state = SDL_GetJoystickConnectionState(controller->joystick);
  } else {
    connection_state = SDL_JOYSTICK_CONNECTION_UNKNOWN;
  }

  switch (connection_state) {
    case SDL_JOYSTICK_CONNECTION_WIRED:
      return "WIRED";
    case SDL_JOYSTICK_CONNECTION_WIRELESS:
      return "WIRELESS";
    case SDL_JOYSTICK_CONNECTION_UNKNOWN:
    default:
      return "UNKNOWN";
  }
}
const char* Controller_GetPowerState(ControllerId id)
{
  Controller* controller = GetController(id);
  SDL_PowerState power_state;

  if (controller) {
    power_state = controller->power_state;
  } else {
    power_state = SDL_POWERSTATE_UNKNOWN;
  }

  switch (power_state) {
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

int Controller_GetBatteryLevel(ControllerId id)
{
  Controller* controller = GetController(id);
  return controller ? controller->battery_level : -1;
}

int Controller_GetProduct(ControllerId id)
{
  ReturnJoystickValue(id, SDL_GetJoystickProduct, 0);
}

int Controller_GetProductVersion(ControllerId id)
{
  ReturnJoystickValue(id, SDL_GetJoystickProductVersion, 0);
}

int Controller_GetFirmwareVersion(ControllerId id)
{
  ReturnJoystickValue(id, SDL_GetJoystickFirmwareVersion, 0);
}

int Controller_GetVendor(ControllerId id)
{
  ReturnJoystickValue(id, SDL_GetJoystickVendor, 0);
}

const char* Controller_GetSerial(ControllerId id)
{
  ReturnJoystickValue(id, SDL_GetJoystickSerial, "");
}

bool Controller_IsHaptic(ControllerId id)
{
  ReturnJoystickValue(id, SDL_IsJoystickHaptic, false);
}

int Controller_GetButtonCount(ControllerId id)
{
  ReturnJoystickValue(id, SDL_GetNumJoystickButtons, 0);
}

int Controller_GetAxisCount(ControllerId id)
{
  ReturnJoystickValue(id, SDL_GetNumJoystickAxes, 0);
}

int Controller_GetHatCount(ControllerId id)
{
  ReturnJoystickValue(id, SDL_GetNumJoystickHats, 0);
}
int Controller_GetBallCount(ControllerId id)
{
  ReturnJoystickValue(id, SDL_GetNumJoystickBalls, 0);
}

int Controller_GetTouchpadCount(ControllerId id)
{
  ReturnGamepadValue(id, SDL_GetNumGamepadTouchpads, 0);
}

uint64_t Controller_GetSteamHandle(ControllerId id)
{
  ReturnGamepadValue(id, SDL_GetGamepadSteamHandle, 0);
}

const ControllerProperty* Controller_GetProperties(ControllerId id,
                                                   int* out_count)
{
  Controller* controller = GetController(id);

  if (!controller) {
    *out_count = 0;
    return NULL;
  }

  *out_count = (int)c_arraylen(controller->properties);
  return &controller->properties[0];
}

const char** Controller_GetPropertyNames(int* out_count)
{
  *out_count = (int)c_arraylen(JOYSTICK_PROPERTY_NAMES);
  return &JOYSTICK_PROPERTY_NAMES[0];
}

//
// private function implementation
//

void Controller_drop(Controller* controller)
{
  ClearGamepad(controller);

  cstr_drop(&controller->joystick_name);
  cstr_drop(&controller->gamepad_name);

  if (controller->joystick) {
    SDL_CloseJoystick(controller->joystick);
  }
}

static Controller* GetController(ControllerId id)
{
  controller_map_value* value =
      controller_map_get_mut(&g_controller_list_model.controllers, id);
  return value ? &value->second : NULL;
}

static bool AddController(SDL_JoystickID id)
{
  if (controller_map_contains(&g_controller_list_model.controllers, id)) {
    return false;
  }

  SDL_Joystick* joystick = SDL_OpenJoystick(id);

  if (!joystick) {
    return false;
  }

  Controller controller = {
      .id = id,
      .joystick = joystick,
  };

  UpdateJoystickName(&controller);
  UpdateGUID(&controller);
  UpdateProperties(&controller);
  UpdatePowerInfo(&controller);
  UpdateGamepad(&controller);

  // note controller drop'd on failed insert
  controller_map_result result = controller_map_insert(
      &g_controller_list_model.controllers, id, controller);

  return result.inserted;
}

static void UpdatePowerInfo(Controller* controller)
{
  if (!controller) {
    return;
  }

  int battery_level;

  SDL_PowerState power_state =
      SDL_GetJoystickPowerInfo(controller->joystick, &battery_level);

  if (power_state == SDL_POWERSTATE_ERROR ||
      power_state == SDL_POWERSTATE_UNKNOWN ||
      power_state == SDL_POWERSTATE_NO_BATTERY) {
    battery_level = -1;
  }

  controller->power_state = power_state;
  controller->battery_level = battery_level;
}

static void UpdateGamepadById(SDL_JoystickID id)
{
  UpdateGamepad(GetController(id));
}

static void UpdateGamepad(Controller* controller)
{
  if (!controller) {
    return;
  }

  SDL_Gamepad* gamepad = controller->gamepad;

  if (gamepad == NULL && SDL_IsGamepad(controller->id)) {
    gamepad = SDL_OpenGamepad(controller->id);
    controller->gamepad = gamepad;
  }

  if (gamepad == NULL) {
    return;
  }

  SDL_free(controller->gamepad_bindings);
  controller->gamepad_bindings =
      SDL_GetGamepadBindings(gamepad, &controller->gamepad_bindings_count);

  if (controller->gamepad_bindings_count == 0) {
    ClearGamepad(controller);
  }

  const char* gamepad_name = SDL_GetGamepadName(gamepad);

  cstr_assign(&controller->gamepad_name, EnsureString(gamepad_name, ""));

  // TODO: probably add csv mappings to the controller
}

static void RemoveGamepadById(ControllerId id)
{
  controller_map_erase(&g_controller_list_model.controllers, id);
}

static void ClearGamepad(Controller* controller)
{
  if (controller->gamepad) {
    SDL_CloseGamepad(controller->gamepad);
    controller->gamepad = NULL;
  }

  SDL_free(controller->gamepad_bindings);
  controller->gamepad_bindings = NULL;
  controller->gamepad_bindings_count = 0;
}

static void AppEventHandler(int event_type, void* event_data, void* user_data)
{
  UNUSED(user_data);
  SDL_Event* sdl_event = event_data;

  switch (event_type) {
    case SDL_EVENT_JOYSTICK_ADDED: {
      ControllerId id = sdl_event->jdevice.which;

      if (!controller_map_contains(&g_controller_list_model.controllers, id)) {
        AddController(id);
        // TODO: validate result
        // TODO: notify model listener of change
      }
      break;
    }
    case SDL_EVENT_JOYSTICK_REMOVED:
      controller_map_erase(&g_controller_list_model.controllers,
                           sdl_event->jdevice.which);
      // TODO: notify model listener of change
      break;
    case SDL_EVENT_GAMEPAD_REMOVED:
      RemoveGamepadById(sdl_event->gdevice.which);
      // TODO: notify model listener of change
      break;
    case SDL_EVENT_GAMEPAD_ADDED:
    case SDL_EVENT_GAMEPAD_REMAPPED: {
      UpdateGamepadById(sdl_event->gdevice.which);
      // TODO: notify model listener of change
      break;
    }
    case SDL_EVENT_JOYSTICK_BATTERY_UPDATED:
      UpdatePowerInfo(GetController(sdl_event->jdevice.which));
      // TODO: notify model listener of change
      break;
    case SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED:
      // TODO: notify model listener of change
      break;
    default:
      break;
  }
}

static void UpdateGUID(Controller* controller)
{
  if (!controller) {
    return;
  }

  SDL_GUID guid = SDL_GetJoystickGUID(controller->joystick);

  SDL_GUIDToString(guid, &controller->guid[0], (int)sizeof(controller->guid));
}

static void UpdateJoystickName(Controller* controller)
{
  if (!controller) {
    return;
  }

  cstr_assign(&controller->joystick_name,
              EnsureString(SDL_GetJoystickName(controller->joystick), ""));
}

static void UpdateProperties(Controller* controller)
{
  if (!controller) {
    return;
  }

  SDL_PropertiesID joystick_properties =
      SDL_GetJoystickProperties(controller->joystick);
  int property_count = (int)c_arraylen(controller->properties);

  for (int i = 0; i < property_count; ++i) {
    controller->properties[i].value = SDL_GetBooleanProperty(
        joystick_properties, controller->properties[i].name, false);
  }
}
