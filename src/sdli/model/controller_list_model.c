#include <sdli/model/model.h>

#include <sdli/app.h>
#include <sdli/util.h>

#include <SDL3/SDL.h>
#include <stc/cstr.h>
#include <stc/zsview.h>
#include <stc/csview.h>

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

// these values work ok on an Xbox One S controller
// (maybe make this configurable later)
#define JOYSTICK_DEADZONE (SDL_JOYSTICK_AXIS_MAX / 32)
#define JOYSTICK_RUMBLE_LOW_FREQUENCY (0xFFFF / 2)
#define JOYSTICK_RUMBLE_HIGH_FREQUENCY (0xFFFF / 2)
#define JOYSTICK_RUMBLE_DURATION_MS (300)

static const char* JOYSTICK_PROPERTY_NAMES[] = {
    // clang-format off
    SDL_PROP_JOYSTICK_CAP_MONO_LED_BOOLEAN,
    SDL_PROP_JOYSTICK_CAP_RGB_LED_BOOLEAN,
    SDL_PROP_JOYSTICK_CAP_PLAYER_LED_BOOLEAN,
    SDL_PROP_JOYSTICK_CAP_RUMBLE_BOOLEAN,
    SDL_PROP_JOYSTICK_CAP_TRIGGER_RUMBLE_BOOLEAN,
    // clang-format on
};

static zsview GAMEPAD_MAPPING_NAMES[] = {
    // clang-format off
    zv_init("guid"),
    zv_init("name"),
    // buttons
    zv_init("a"),
    zv_init("b"),
    zv_init("x"),
    zv_init("y"),
    zv_init("back"),
    zv_init("guide"),
    zv_init("start"),
    zv_init("leftstick"),
    zv_init("rightstick"),
    zv_init("leftshoulder"),
    zv_init("rightshoulder"),
    zv_init("dpup"),
    zv_init("dpdown"),
    zv_init("dpleft"),
    zv_init("dpright"),
    zv_init("misc1"),
    zv_init("paddle1"),
    zv_init("paddle2"),
    zv_init("paddle3"),
    zv_init("paddle4"),
    zv_init("touchpad"),
    zv_init("misc2"),
    zv_init("misc3"),
    zv_init("misc4"),
    zv_init("misc5"),
    zv_init("misc6"),
    // axes
    zv_init("leftx"),
    zv_init("lefty"),
    zv_init("rightx"),
    zv_init("righty"),
    zv_init("lefttrigger"),
    zv_init("righttrigger"),
    // other
    zv_init("crc"),
    zv_init("type"),
    zv_init("face"),
    zv_init("platform"),
    zv_init("hint"),
    zv_init("sdk>="),
    zv_init("sdk<="),
    // clang-format on
};

//
// private types
//

typedef struct GamepadMapping {
  cstr value;
  int name_index;
} GamepadMapping;

static void GamepadMapping_drop(GamepadMapping* mapping);

#define i_no_clone
#define i_no_emplace
#define i_keyclass GamepadMapping
#define i_type gamepad_mappings
#include <stc/vec.h>

typedef struct Controller {
  ControllerId id;

  cstr gamepad_name;
  cstr joystick_name;
  char guid[33];

  SDL_PowerState power_state;
  int battery_level;

  // TODO: this may not be necessary..
  SDL_GamepadBinding** gamepad_bindings;
  int gamepad_bindings_count;

  SDL_Joystick* joystick;
  SDL_Gamepad* gamepad;

  bool properties[u_arraylen(JOYSTICK_PROPERTY_NAMES)];
  char* gamepad_mapping_csv;

  gamepad_mappings mappings;

  bool has_rumble;
} Controller;

static void Controller_drop(Controller* controller);

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
static void ParseGamepadMapping(Controller* controller);
static void OnAppEvent(int event_type, void* event_data, void* user_data);
static void UpdateGUID(Controller* controller);
static void UpdateJoystickName(Controller* controller);
static void UpdateProperties(Controller* controller);
static void DispatchControllerChangeEvent(ControllerId id,
                                          ControllerChange change);
static bool ReadString(csview* src, csview* out, int ch);
static bool FindMappingNameIndex(csview name, int* out_index);
static GamepadMapping GamepadMapping_Init(csview value, int name_index);
static cstr ControllerMappingsToString(void);

//
// global state
//

static ControllerListModel g_controller_list_model = {0};

//
// public function implementation
//

void ControllerListModel_Init(void)
{
  SLog("%s", FN_NAME);

  g_controller_list_model = (ControllerListModel){
      .controllers = controller_map_init(),
  };

  int count = 0;
  SDL_JoystickID* joysticks = SDL_GetJoysticks(&count);

  for (int i = 0; i < count; ++i) {
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

  SLog("%s: success", FN_NAME);
}

void ControllerListModel_Drop(void)
{
  SLog("%s", FN_NAME);

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

void ControllerListModel_AddChangeEventListener(
    ControllerChangeEventListener listener)
{
  const size_t size = u_arraylen(g_controller_list_model.change_listeners);

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
  const size_t size = u_arraylen(g_controller_list_model.change_listeners);

  for (size_t i = 0; i < size; i++) {
    if (g_controller_list_model.change_listeners[i] == listener) {
      g_controller_list_model.change_listeners[i] = NULL;
      return;
    }
  }
}

void ControllerListModel_ReloadMappings(void)
{
  if (!SDL_ReloadGamepadMappings()) {
    SLogCallError("SDL_ReloadGamepadMappings");
  }
}

int ControllerListModel_LoadMappingsFromClipboard(void)
{
  char* mappings = SDL_GetClipboardText();

  if (!mappings) {
    return -1;
  }

  SDL_IOStream* src = SDL_IOFromConstMem(mappings, SDL_strlen(mappings));

  if (!src) {
    SLogCallError("SDL_IOFromConstMem");
    SDL_free(mappings);
    return -1;
  }

  const int result = SDL_AddGamepadMappingsFromIO(src, true);

  if (!result) {
    SLogCallError("SDL_AddGamepadMappingsFromIO");
  }

  SDL_free(mappings);

  return result;
}

int ControllerListModel_LoadMappingsFromFile(const char* filename)
{
  // TODO: this can return -1
  // TODO: log sdl error?
  return SDL_AddGamepadMappingsFromIO(SDL_IOFromFile(filename, "r"), true);
}

void ControllerListModel_ExportMappingsToClipboard(void)
{
  cstr mappings = ControllerMappingsToString();
  SystemModel_CopyToClipboard(cstr_str(&mappings));
  cstr_drop(&mappings);
}

bool ControllerListModel_ExportMappingsToFile(const char* filename)
{
  cstr mappings = ControllerMappingsToString();
  bool result =
      SDL_SaveFile(filename, cstr_str(&mappings), cstr_size(&mappings));

  cstr_drop(&mappings);
  return result;
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

bool Controller_GetPropertyValue(ControllerId id, int property_index)
{
  Controller* controller = GetController(id);

  return controller && property_index >= 0 &&
         property_index < (int)u_arraylen(JOYSTICK_PROPERTY_NAMES) &&
         controller->properties[property_index];
}

int Controller_GetPropertyCount(void)
{
  return (int)u_arraylen(JOYSTICK_PROPERTY_NAMES);
}

const char* Controller_GetPropertyName(int property_index)
{
  return (property_index >= 0 &&
          property_index < (int)u_arraylen(JOYSTICK_PROPERTY_NAMES))
             ? JOYSTICK_PROPERTY_NAMES[property_index]
             : "";
}

const char* Controller_GetBindingName(ControllerId id, int binding_index)
{
  Controller* controller = GetController(id);

  if (controller && binding_index >= 0 &&
      binding_index < (int)gamepad_mappings_size(&controller->mappings)) {
    return GAMEPAD_MAPPING_NAMES[controller->mappings.data[binding_index]
                                     .name_index]
        .str;
  }

  return "";
}

const char* Controller_GetBindingValue(ControllerId id, int binding_index)
{
  Controller* controller = GetController(id);

  if (controller && binding_index >= 0 &&
      binding_index < (int)gamepad_mappings_size(&controller->mappings)) {
    return cstr_str(&controller->mappings.data[binding_index].value);
  }

  return "";
}

int Controller_GetBindingCount(ControllerId id)
{
  Controller* controller = GetController(id);
  return controller ? (int)gamepad_mappings_size(&controller->mappings) : 0;
}

float Controller_GetJoystickAxisValue(ControllerId id, int axis)
{
  const Controller* controller = GetController(id);
  Sint16 value = 0;

  if (controller) {
    value = SDL_GetJoystickAxis(controller->joystick, axis);
  }

  return Controller_JoystickAxisToFloat(value);
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

  return Controller_JoystickAxisToFloat(SDL_GetGamepadAxis(
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

  // If no bindings exist, SDL can return a mapping string with name, guid, etc,
  // which is not useful for this app.

  return (controller && controller->gamepad_bindings_count > 0 &&
          controller->gamepad_mapping_csv)
             ? controller->gamepad_mapping_csv
             : "";
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

bool Controller_HasRumble(ControllerId id)
{
  Controller* controller = GetController(id);
  return controller && controller->has_rumble;
}

void Controller_RemoveMapping(ControllerId id)
{
  // this call will generate a remap event that will update the controller
  if (!SDL_SetGamepadMapping(id, NULL)) {
    SLogCallErrorWithU64("SDL_SetGamepadMapping[Remove]", (uint64_t)id);
  }
}

void Controller_Rumble(ControllerId id)
{
  Controller* controller = GetController(id);

  if (controller && controller->has_rumble) {
    if (!SDL_RumbleJoystick(controller->joystick, 0xFFFF / 2, 0xFFFF / 2,
                            300)) {
      SLogCallErrorWithU64("SDL_RumbleJoystick", (uint64_t)controller->id);
    }
  }
}

float Controller_JoystickAxisToFloat(int16_t value)
{
  if (value < JOYSTICK_DEADZONE && value > -JOYSTICK_DEADZONE) {
    return 0;
  }

  return value >= 0 ? (float)value / (float)SDL_JOYSTICK_AXIS_MAX
                    : ((float)value / (float)SDL_JOYSTICK_AXIS_MIN) * -1.0f;
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
  SLog("%s(%" PRIu32 ")", FN_NAME, controller->id);

  ClearGamepad(controller);

  cstr_drop(&controller->joystick_name);
  cstr_drop(&controller->gamepad_name);
  gamepad_mappings_drop(&controller->mappings);

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
    SLog("%s(%" PRIu32 "): already exists", FN_NAME, id);
    return false;
  }

  SLog("%s(%" PRIu32 ")", FN_NAME, id);

  SDL_Joystick* joystick = SDL_OpenJoystick(id);

  if (!joystick) {
    SLogCallError("SDL_OpenJoystick");
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

  SLog("%s(%" PRIu32 "): %s", FN_NAME, id,
       result.inserted ? "success" : "failed");

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

    if (!gamepad) {
      SLogCallErrorWithU64("SDL_OpenGamepad", (uint64_t)controller->id);
    }

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

  SDL_free(controller->gamepad_mapping_csv);
  controller->gamepad_mapping_csv = SDL_GetGamepadMapping(gamepad);

  ParseGamepadMapping(controller);
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

  gamepad_mappings_clear(&controller->mappings);
}

static bool ReadString(csview* src, csview* out, int ch)
{
  for (isize i = 0; i < src->size; i++) {
    if (src->buf[i] == ch) {
      *out = *src;
      out->size = i;
      src->buf += i + 1;
      src->size -= i + 1;
      return true;
    }
  }

  return false;
}

static bool FindMappingNameIndex(csview name, int* out_index)
{
  for (size_t i = 0; i < u_arraylen(GAMEPAD_MAPPING_NAMES); i++) {
    if (csview_equals_sv(name, csview_with_n(GAMEPAD_MAPPING_NAMES[i].str,
                                             GAMEPAD_MAPPING_NAMES[i].size))) {
      *out_index = (int)i;
      return true;
    }
  }
  return false;
}

static void ParseGamepadMapping(Controller* controller)
{
  gamepad_mappings_clear(&controller->mappings);

  if (controller->gamepad_mapping_csv == NULL) {
    return;
  }

  // gamepad_mapping_csv comes from SDL, so we can assume it's well-formed.
  csview source = csview_from(controller->gamepad_mapping_csv);
  csview key = csview_init();
  csview value = csview_init();
  int name_index;
  bool result;

  result = ReadString(&source, &value, ',');

  if (!result) {
    goto error;
  }

  gamepad_mappings_push_back(&controller->mappings,
                             GamepadMapping_Init(value, 0));

  result = ReadString(&source, &value, ',');

  if (!result) {
    goto error;
  }

  gamepad_mappings_push_back(&controller->mappings,
                             GamepadMapping_Init(value, 1));

  while (ReadString(&source, &key, ':')) {
    if (!ReadString(&source, &value, ',')) {
      goto error;
    }

    if (!FindMappingNameIndex(key, &name_index)) {
      // TODO: log warning about unknown mapping name
      printf("Warning: unknown mapping name '%.*s'\n", (int)key.size, key.buf);
      continue;
    }

    gamepad_mappings_push_back(&controller->mappings,
                               GamepadMapping_Init(value, name_index));
  }

  return;

error:
  gamepad_mappings_clear(&controller->mappings);
}

static void OnAppEvent(int event_type, void* event_data, void* user_data)
{
  UNUSED(user_data);
  SLogEvent(event_data);

  SDL_Event* sdl_event = event_data;

  switch (event_type) {
    case SDL_EVENT_JOYSTICK_ADDED: {
      ControllerId id = sdl_event->jdevice.which;

      if (AddController(id)) {
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

  for (size_t i = 0; i < u_arraylen(controller->properties); i++) {
    controller->properties[i] = SDL_GetBooleanProperty(
        joystick_properties, JOYSTICK_PROPERTY_NAMES[i], false);
  }

  controller->has_rumble = SDL_GetBooleanProperty(
      joystick_properties, SDL_PROP_JOYSTICK_CAP_RUMBLE_BOOLEAN, false);
}

static void DispatchControllerChangeEvent(ControllerId id,
                                          ControllerChange change)
{
  const size_t size = u_arraylen(g_controller_list_model.change_listeners);

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

static GamepadMapping GamepadMapping_Init(csview value, int name_index)
{
  return (GamepadMapping){
      .value = cstr_from_sv(value),
      .name_index = name_index,
  };
}

static void GamepadMapping_drop(GamepadMapping* mapping)
{
  cstr_drop(&mapping->value);
}

static cstr ControllerMappingsToString(void)
{
  cstr builder = cstr_init();

  c_foreach(it, controller_map, g_controller_list_model.controllers)
  {
    const char* mapping = Controller_GetMappingString(it.ref->first);

    if (*mapping) {
      cstr_append(&builder, mapping);
      cstr_append(&builder, NEWLINE);
    }
  }

  return builder;
}
