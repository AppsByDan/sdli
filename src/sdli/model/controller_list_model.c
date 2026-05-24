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

#define JOYSTICK_DEADZONE (SDL_JOYSTICK_AXIS_MAX / 32)

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
  char* gamepad_mapping_csv;
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
  ControllerId selected_controller;
  ControllerInputEventListener input_event_listener;
  ControllerChangeEventListener change_listeners[8];
} ControllerListModel;

//
// private function declarations
//

static Controller* GetController(ControllerId id);
static bool AddController(SDL_JoystickID id);
static void UpdatePowerInfo(Controller* controller);
static void UpdateGamepadById(SDL_JoystickID id);
static void UpdateGamepad(Controller* controller);
static void ClearGamepad(Controller* controller);
static void OnAppEvent(int event_type, void* event_data, void* user_data);
static void OnJoystickInputEvents(int event_type,
                                  void* event_data,
                                  void* user_data);
static void OnGamepadInputEvents(int event_type,
                                 void* event_data,
                                 void* user_data);
static void UpdateGUID(Controller* controller);
static void UpdateJoystickName(Controller* controller);
static void UpdateProperties(Controller* controller);
static float JoystickAxisToFloat(Sint16 value);
static void DispatchControllerChangeEvent(ControllerId id,
                                          ControllerChange change);

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

  App_AddEventListener(SDL_EVENT_JOYSTICK_ADDED, &OnAppEvent, NULL);
  App_AddEventListener(SDL_EVENT_JOYSTICK_REMOVED, &OnAppEvent, NULL);
  App_AddEventListener(SDL_EVENT_GAMEPAD_ADDED, &OnAppEvent, NULL);
  App_AddEventListener(SDL_EVENT_GAMEPAD_REMOVED, &OnAppEvent, NULL);
  App_AddEventListener(SDL_EVENT_GAMEPAD_REMAPPED, &OnAppEvent, NULL);
  App_AddEventListener(SDL_EVENT_JOYSTICK_BATTERY_UPDATED, &OnAppEvent, NULL);
  App_AddEventListener(SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED, &OnAppEvent,
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

ControllerId ControllerListModel_GetSelectedController(void)
{
  return g_controller_list_model.selected_controller;
}

void ControllerListModel_SelectController(ControllerId id)
{
  g_controller_list_model.selected_controller = id;
}

void ControllerListModel_EnableControllerInputEvents(
    ControllerApi api,
    ControllerInputEventListener listener)
{
  ControllerListModel_DisableControllerEvents();

  if (api == CONTROLLER_API_JOYSTICK) {
    App_AddEventListener(SDL_EVENT_JOYSTICK_BUTTON_DOWN, &OnJoystickInputEvents,
                         NULL);
    App_AddEventListener(SDL_EVENT_JOYSTICK_BUTTON_UP, &OnJoystickInputEvents,
                         NULL);
    App_AddEventListener(SDL_EVENT_JOYSTICK_AXIS_MOTION, &OnJoystickInputEvents,
                         NULL);
    App_AddEventListener(SDL_EVENT_JOYSTICK_HAT_MOTION, &OnJoystickInputEvents,
                         NULL);
  } else if (api == CONTROLLER_API_GAMEPAD) {
    App_AddEventListener(SDL_EVENT_GAMEPAD_BUTTON_DOWN, &OnGamepadInputEvents,
                         NULL);
    App_AddEventListener(SDL_EVENT_GAMEPAD_BUTTON_UP, &OnGamepadInputEvents,
                         NULL);
    App_AddEventListener(SDL_EVENT_GAMEPAD_AXIS_MOTION, &OnGamepadInputEvents,
                         NULL);
  } else {
    return;
  }

  g_controller_list_model.input_event_listener = listener;
}

void ControllerListModel_DisableControllerEvents(void)
{
  App_RemoveEventListener(SDL_EVENT_JOYSTICK_BUTTON_DOWN,
                          &OnJoystickInputEvents);
  App_RemoveEventListener(SDL_EVENT_JOYSTICK_BUTTON_UP, &OnJoystickInputEvents);
  App_RemoveEventListener(SDL_EVENT_JOYSTICK_AXIS_MOTION,
                          &OnJoystickInputEvents);
  App_RemoveEventListener(SDL_EVENT_GAMEPAD_BUTTON_DOWN, &OnGamepadInputEvents);
  App_RemoveEventListener(SDL_EVENT_GAMEPAD_BUTTON_UP, &OnGamepadInputEvents);
  App_RemoveEventListener(SDL_EVENT_GAMEPAD_AXIS_MOTION, &OnGamepadInputEvents);
}

void ControllerListModel_AddChangeEventListener(
    ControllerChangeEventListener listener)
{
  const size_t size = c_arraylen(g_controller_list_model.change_listeners);

  for (size_t i = 0; i < size; i++) {
    if (g_controller_list_model.change_listeners[i] == listener) {
      return;  // already added
    }
  }

  for (size_t i = 0; i < size; i++) {
    if (g_controller_list_model.change_listeners[i] == NULL) {
      g_controller_list_model.change_listeners[i] = listener;
      return;
    }
  }

  // TODO: unreachable
  abort();
}

void ControllerListModel_RemoveChangeEventListener(
    ControllerChangeEventListener listener)
{
  const size_t size = c_arraylen(g_controller_list_model.change_listeners);

  for (size_t i = 0; i < size; i++) {
    if (g_controller_list_model.change_listeners[i] == listener) {
      g_controller_list_model.change_listeners[i] = NULL;
      return;
    }
  }
}

void ControllerListModel_ReloadMappings(void)
{
  SDL_ReloadGamepadMappings();
}

int ControllerListModel_LoadMappingsFromClipboard(void)
{
  char* mappings = SDL_GetClipboardText();

  if (!mappings) {
    return -1;
  }

  SDL_IOStream* src = SDL_IOFromConstMem(mappings, SDL_strlen(mappings));

  if (!src) {
    SDL_free(mappings);
    return -1;
  }

  const int result = SDL_AddGamepadMappingsFromIO(src, true);

  SDL_free(mappings);

  return result;
}

void ControllerListModel_ExportMappingsToClipboard(void)
{
  cstr builder = cstr_init();

  c_foreach(it, controller_map, g_controller_list_model.controllers)
  {
    const char* mapping = Controller_GetMappingString(it.ref->first);

    if (*mapping) {
      cstr_append(&builder, mapping);
      cstr_append(&builder, "\n");
    }
  }

  SDL_SetClipboardText(cstr_str(&builder));
  cstr_drop(&builder);
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

const char* Controller_GetJoystickName(ControllerId id)
{
  Controller* controller = GetController(id);
  return controller ? cstr_str(&controller->joystick_name) : "";
}

const char* Controller_GetGamepadName(ControllerId id)
{
  Controller* controller = GetController(id);
  return controller ? cstr_str(&controller->gamepad_name) : "";
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

float Controller_GetJoystickAxisValue(ControllerId id, int axis)
{
  const Controller* controller = GetController(id);
  Sint16 value = 0;

  if (controller) {
    value = SDL_GetJoystickAxis(controller->joystick, axis);
  }

  return JoystickAxisToFloat(value);
}

uint8_t Controller_GetJoystickHatValue(ControllerId id, int hat)
{
  const Controller* controller = GetController(id);
  uint8_t value = 0;

  if (controller) {
    value = SDL_GetJoystickHat(controller->joystick, hat);
  }

  return value;
}

bool Controller_GetJoystickButtonValue(ControllerId id, int button)
{
  const Controller* controller = GetController(id);
  uint8_t value = 0;

  if (controller) {
    value = SDL_GetJoystickButton(controller->joystick, button);
  }

  return value != 0;
}

float Controller_GetAxisValue(ControllerId id, StandardGamepadKey axis)
{
  Controller* controller = GetController(id);

  if (!controller || axis < STANDARD_GAMEPAD_KEY_AXIS_OFFSET ||
      axis >= STANDARD_GAMEPAD_KEY_AXIS_OFFSET + STANDARD_GAMEPAD_AXIS_COUNT) {
    return 0;
  }

  return JoystickAxisToFloat(SDL_GetGamepadAxis(
      controller->gamepad, axis - STANDARD_GAMEPAD_KEY_AXIS_OFFSET));
}

bool Controller_GetButtonValue(ControllerId id, StandardGamepadKey button)
{
  Controller* controller = GetController(id);

  if (!controller || button < 0 || button >= STANDARD_GAMEPAD_KEY_AXIS_OFFSET) {
    return false;
  }

  return SDL_GetGamepadButton(controller->gamepad, (SDL_GamepadButton)button) !=
         0;
}

const char* Controller_GetMappingString(ControllerId id)
{
  Controller* controller = GetController(id);

  // If no bindings exist, SDL can return a mapping string with name, guid and
  // platform, which is not useful for this app.
  if (controller && controller->gamepad_bindings_count > 0) {
    if (controller->gamepad_mapping_csv) {
      return controller->gamepad_mapping_csv;
    }

    if (controller->gamepad) {
      controller->gamepad_mapping_csv =
          SDL_GetGamepadMapping(controller->gamepad);
      return EnsureString(controller->gamepad_mapping_csv, "");
    }
  }

  return "";
}

bool Controller_HasMapping(ControllerId id)
{
  Controller* controller = GetController(id);

  return controller && controller->gamepad_bindings &&
         controller->gamepad_bindings_count > 0;
}

bool Controller_HasMappingForKey(ControllerId id, StandardGamepadKey key)
{
  // TODO: handle axis keys
  if (key >= STANDARD_GAMEPAD_BUTTON_COUNT) {
    return false;
  }

  Controller* controller = GetController(id);

  if (!controller) {
    return false;
  }

  // linear search is fine since the number of bindings is small
  // (typically 20-30)
  for (int i = 0; i < controller->gamepad_bindings_count; i++) {
    SDL_GamepadBinding* binding = controller->gamepad_bindings[i];

    if (binding->output_type == SDL_GAMEPAD_BINDTYPE_BUTTON &&
        (int)binding->output.button == (int)key) {
      return true;
    }
  }

  return false;
}

void Controller_RemoveMapping(ControllerId id)
{
  // this call will generate a remap event that will update the controller
  SDL_SetGamepadMapping(id, NULL);
}

const char* StandardGamepadKey_ToString(StandardGamepadKey key)
{
  switch (key) {
    case SGK_BUTTON_SOUTH:
      return "SOUTH";
    case SGK_BUTTON_EAST:
      return "EAST";
    case SGK_BUTTON_WEST:
      return "WEST";
    case SGK_BUTTON_NORTH:
      return "NORTH";
    case SGK_BUTTON_BACK:
      return "BACK";
    case SGK_BUTTON_GUIDE:
      return "GUIDE";
    case SGK_BUTTON_START:
      return "START";
    case SGK_BUTTON_LEFT_STICK:
      return "LEFT_STICK";
    case SGK_BUTTON_RIGHT_STICK:
      return "RIGHT_STICK";
    case SGK_BUTTON_LEFT_SHOULDER:
      return "LEFT_SHOULDER";
    case SGK_BUTTON_RIGHT_SHOULDER:
      return "RIGHT_SHOULDER";
    case SGK_BUTTON_DPAD_UP:
      return "DPAD_UP";
    case SGK_BUTTON_DPAD_DOWN:
      return "DPAD_DOWN";
    case SGK_BUTTON_DPAD_LEFT:
      return "DPAD_LEFT";
    case SGK_BUTTON_DPAD_RIGHT:
      return "DPAD_RIGHT";
    case SGK_BUTTON_MISC1:
      return "MISC1";
    case SGK_BUTTON_RIGHT_PADDLE1:
      return "RIGHT_PADDLE1";
    case SGK_BUTTON_LEFT_PADDLE1:
      return "LEFT_PADDLE1";
    case SGK_BUTTON_RIGHT_PADDLE2:
      return "RIGHT_PADDLE2";
    case SGK_BUTTON_LEFT_PADDLE2:
      return "LEFT_PADDLE2";
    case SGK_BUTTON_TOUCHPAD:
      return "TOUCHPAD";
    case SGK_BUTTON_MISC2:
      return "MISC2";
    case SGK_BUTTON_MISC3:
      return "MISC3";
    case SGK_BUTTON_MISC4:
      return "MISC4";
    case SGK_BUTTON_MISC5:
      return "MISC5";
    case SGK_BUTTON_MISC6:
      return "MISC6";
    case SGK_AXIS_LEFTX:
      return "AXIS_LEFTX";
    case SGK_AXIS_LEFTY:
      return "AXIS_LEFTY";
    case SGK_AXIS_RIGHTX:
      return "AXIS_RIGHTX";
    case SGK_AXIS_RIGHTY:
      return "AXIS_RIGHTY";
    case SGK_AXIS_LEFT_TRIGGER:
      return "AXIS_LEFT_TRIGGER";
    case SGK_AXIS_RIGHT_TRIGGER:
      return "AXIS_RIGHT_TRIGGER";
    default:
      return "UNKNOWN";
  }
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
    ClearGamepad(controller);
    return;
  }

  SDL_free(controller->gamepad_bindings);
  controller->gamepad_bindings =
      SDL_GetGamepadBindings(gamepad, &controller->gamepad_bindings_count);

  if (controller->gamepad_bindings_count == 0) {
    ClearGamepad(controller);
    return;
  }

  const char* gamepad_name = SDL_GetGamepadName(gamepad);

  cstr_assign(&controller->gamepad_name, EnsureString(gamepad_name, ""));

  // mapping string is lazily loaded
  SDL_free(controller->gamepad_mapping_csv);
  controller->gamepad_mapping_csv = NULL;
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

  cstr_clear(&controller->gamepad_name);

  SDL_free(controller->gamepad_mapping_csv);
  controller->gamepad_mapping_csv = NULL;
}

static void OnAppEvent(int event_type, void* event_data, void* user_data)
{
  UNUSED(user_data);
  SDL_Event* sdl_event = event_data;

  switch (event_type) {
    case SDL_EVENT_JOYSTICK_ADDED: {
      ControllerId id = sdl_event->jdevice.which;

      if (!controller_map_contains(&g_controller_list_model.controllers, id)) {
        AddController(id);
        DispatchControllerChangeEvent(id, CONTROLLER_CHANGE_ADDED);
      }
      break;
    }
    case SDL_EVENT_JOYSTICK_REMOVED:
      DispatchControllerChangeEvent(sdl_event->jdevice.which,
                                    CONTROLLER_CHANGE_REMOVED);
      controller_map_erase(&g_controller_list_model.controllers,
                           sdl_event->jdevice.which);
      break;
    case SDL_EVENT_GAMEPAD_REMOVED: {
      Controller* controller = GetController(sdl_event->gdevice.which);

      if (controller) {
        // Mapping API, like SDL_ReloadGamepadMappings, will generate
        // GAMEPAD_REMOVED events. It appears that remove means that the Gamepad
        // instance is no longer valid. ClearGamepad will destroy the Gamepad
        // instance.
        ClearGamepad(controller);
        // XXX: open the gamepad for this device to trigger a GAMEPAD_ADDED
        // event. UpdateGamepad will work here, but that will generate a
        // GAMEPAD_ADDED and call UpdateGamepad on the next frame. Unclear
        // if this is a bug in SDL or just a quirk of the API.
        SDL_Gamepad* gamepad = SDL_OpenGamepad(controller->id);
        if (gamepad) {
          SDL_CloseGamepad(gamepad);
        }
      }
      break;
    }
    case SDL_EVENT_GAMEPAD_ADDED:
    case SDL_EVENT_GAMEPAD_REMAPPED: {
      UpdateGamepadById(sdl_event->gdevice.which);
      DispatchControllerChangeEvent(sdl_event->gdevice.which,
                                    CONTROLLER_CHANGE_INFO);
      break;
    }
    case SDL_EVENT_JOYSTICK_BATTERY_UPDATED:
      UpdatePowerInfo(GetController(sdl_event->jdevice.which));
      DispatchControllerChangeEvent(sdl_event->jdevice.which,
                                    CONTROLLER_CHANGE_POWER);
      break;
    case SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED:
      DispatchControllerChangeEvent(sdl_event->gdevice.which,
                                    CONTROLLER_CHANGE_STEAM_HANDLE);
      break;
    default:
      break;
  }
}

static void OnJoystickInputEvents(int event_type,
                                  void* event_data,
                                  void* user_data)
{
  UNUSED(user_data);
  SDL_Event* sdl_event = event_data;
  ControllerInputEvent input_event = {
      .api = CONTROLLER_API_JOYSTICK,
  };

  // TODO: filter by controller id

  switch (event_type) {
    case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
    case SDL_EVENT_JOYSTICK_BUTTON_UP:
      if (g_controller_list_model.selected_controller !=
          sdl_event->jbutton.which) {
        return;
      }
      input_event.type = CONTROLLER_INPUT_BUTTON;
      input_event.u.button.id = sdl_event->jbutton.button;
      input_event.u.button.pressed =
          (event_type == SDL_EVENT_JOYSTICK_BUTTON_DOWN);

      break;
    case SDL_EVENT_JOYSTICK_AXIS_MOTION:
      if (g_controller_list_model.selected_controller !=
          sdl_event->jaxis.which) {
        return;
      }
      input_event.type = CONTROLLER_INPUT_AXIS;
      input_event.u.axis.id = sdl_event->jaxis.axis;
      input_event.u.axis.value = JoystickAxisToFloat(sdl_event->jaxis.value);
      break;
    case SDL_EVENT_JOYSTICK_HAT_MOTION:
      if (g_controller_list_model.selected_controller !=
          sdl_event->jhat.which) {
        return;
      }
      input_event.type = CONTROLLER_INPUT_HAT;
      input_event.u.hat.id = sdl_event->jhat.hat;
      input_event.u.hat.value = sdl_event->jhat.value;
      break;
    default:
      return;
  }

  g_controller_list_model.input_event_listener(&input_event);
}

static void OnGamepadInputEvents(int event_type,
                                 void* event_data,
                                 void* user_data)
{
  UNUSED(user_data);
  SDL_Event* sdl_event = event_data;
  ControllerInputEvent input_event = {
      .api = CONTROLLER_API_GAMEPAD,
  };

  switch (event_type) {
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
      if (g_controller_list_model.selected_controller !=
          sdl_event->gbutton.which) {
        return;
      }
      input_event.type = CONTROLLER_INPUT_BUTTON;
      input_event.u.button.id = sdl_event->gbutton.button;
      input_event.u.button.pressed =
          (event_type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
      break;
    case SDL_EVENT_GAMEPAD_AXIS_MOTION:
      if (g_controller_list_model.selected_controller !=
          sdl_event->gaxis.which) {
        return;
      }
      input_event.type = CONTROLLER_INPUT_AXIS;
      input_event.u.axis.id =
          sdl_event->gaxis.axis + STANDARD_GAMEPAD_KEY_AXIS_OFFSET;
      input_event.u.axis.value = JoystickAxisToFloat(sdl_event->gaxis.value);
      break;
    default:
      return;
  }

  g_controller_list_model.input_event_listener(&input_event);
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
    controller->properties[i].name = JOYSTICK_PROPERTY_NAMES[i];
    controller->properties[i].value = SDL_GetBooleanProperty(
        joystick_properties, controller->properties[i].name, false);
  }
}

static float JoystickAxisToFloat(Sint16 value)
{
  if (value < JOYSTICK_DEADZONE && value > -JOYSTICK_DEADZONE) {
    return 0;
  }

  return value >= 0 ? (float)value / (float)SDL_JOYSTICK_AXIS_MAX
                    : ((float)value / (float)SDL_JOYSTICK_AXIS_MIN) * -1.0f;
}

static void DispatchControllerChangeEvent(ControllerId id,
                                          ControllerChange change)
{
  const size_t size = c_arraylen(g_controller_list_model.change_listeners);

  if (size == 0) {
    return;
  }

  ControllerChangeEvent event = {
      .id = id,
      .change = change,
  };

  for (size_t i = 0; i < size; i++) {
    ControllerChangeEventListener listener =
        g_controller_list_model.change_listeners[i];

    if (listener) {
      listener(&event);
    }
  }
}
