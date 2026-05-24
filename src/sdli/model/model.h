#ifndef SDLI_MODEL_H
#define SDLI_MODEL_H

/*
 * The models contained in this file model SDL, mostly at the subsystem level.
 * The models abstract SDL data and events in a way that is easier for the UI to
 * consume.
 *
 * No SDL types should be exposed in model interfaces. The original intent was
 * to support SDL2 at a later point, but in practice the abstraction has been
 * useful for caching and keeping SDL API quirks away from the UI.
 *
 * Models are effectively singletons, stored as global data, to avoid passing
 * model pointers around the UI.
 */

#include <stdbool.h>
#include <stdint.h>

#include <sdli/controller-fwd.h>

//
// types
//

#define STANDARD_GAMEPAD_KEY_AXIS_OFFSET (26)
#define STANDARD_GAMEPAD_BUTTON_COUNT (26)
#define STANDARD_GAMEPAD_AXIS_COUNT (6)
#define STANDARD_GAMEPAD_KEY_COUNT \
  (STANDARD_GAMEPAD_BUTTON_COUNT + STANDARD_GAMEPAD_AXIS_COUNT)

/*
 * StandardGamepadKey is a mapping to SDL_GameControllerButton and
 * SDL_GameControllerAxis. SDL_GameControllerButton is 1:1 and
 * SDL_GameControllerAxis is 1:1 with an offset.
 */
typedef enum StandardGamepadKey {
  SGK_INVALID = -1,
  SGK_BUTTON_SOUTH = 0,
  SGK_BUTTON_EAST = 1,
  SGK_BUTTON_WEST = 2,
  SGK_BUTTON_NORTH = 3,
  SGK_BUTTON_BACK = 4,
  SGK_BUTTON_GUIDE = 5,
  SGK_BUTTON_START = 6,
  SGK_BUTTON_LEFT_STICK = 7,
  SGK_BUTTON_RIGHT_STICK = 8,
  SGK_BUTTON_LEFT_SHOULDER = 9,
  SGK_BUTTON_RIGHT_SHOULDER = 10,
  SGK_BUTTON_DPAD_UP = 11,
  SGK_BUTTON_DPAD_DOWN = 12,
  SGK_BUTTON_DPAD_LEFT = 13,
  SGK_BUTTON_DPAD_RIGHT = 14,
  SGK_BUTTON_MISC1 = 15,
  SGK_BUTTON_RIGHT_PADDLE1 = 16,
  SGK_BUTTON_LEFT_PADDLE1 = 17,
  SGK_BUTTON_RIGHT_PADDLE2 = 18,
  SGK_BUTTON_LEFT_PADDLE2 = 19,
  SGK_BUTTON_TOUCHPAD = 20,
  SGK_BUTTON_MISC2 = 21,
  SGK_BUTTON_MISC3 = 22,
  SGK_BUTTON_MISC4 = 23,
  SGK_BUTTON_MISC5 = 24,
  SGK_BUTTON_MISC6 = 25,

  SGK_AXIS_LEFTX = 0 + STANDARD_GAMEPAD_KEY_AXIS_OFFSET,
  SGK_AXIS_LEFTY = 1 + STANDARD_GAMEPAD_KEY_AXIS_OFFSET,
  SGK_AXIS_RIGHTX = 2 + STANDARD_GAMEPAD_KEY_AXIS_OFFSET,
  SGK_AXIS_RIGHTY = 3 + STANDARD_GAMEPAD_KEY_AXIS_OFFSET,
  SGK_AXIS_LEFT_TRIGGER = 4 + STANDARD_GAMEPAD_KEY_AXIS_OFFSET,
  SGK_AXIS_RIGHT_TRIGGER = 5 + STANDARD_GAMEPAD_KEY_AXIS_OFFSET,
} StandardGamepadKey;

/* POV hat direction masks. This is a 1:1 mapping to SDL_HAT_* values. */
typedef enum ControllerPovHatMask {
  // SDL_HAT_* values
  POV_HAT_MASK_CENTERED = 0x00u,
  POV_HAT_MASK_UP = 0x01u,
  POV_HAT_MASK_RIGHT = 0x02u,
  POV_HAT_MASK_DOWN = 0x04u,
  POV_HAT_MASK_LEFT = 0x08u,
} ControllerPovHatMask;

/* Controller API types for event dispatch. */
typedef enum ControllerApi {
  CONTROLLER_API_JOYSTICK_NONE,
  CONTROLLER_API_JOYSTICK,
  CONTROLLER_API_GAMEPAD,
} ControllerApi;

/* Controller input tag. */
typedef enum ControllerInputType {
  CONTROLLER_INPUT_BUTTON,
  CONTROLLER_INPUT_AXIS,
  CONTROLLER_INPUT_HAT,
} ControllerInputType;

/* Controller input event structure. */
typedef struct ControllerInputEvent {
  ControllerApi api;
  ControllerInputType type;
  union {
    struct {
      int id;
      bool pressed;
    } button;
    struct {
      int id;
      float value;
    } axis;
    struct {
      int id;
      uint8_t value;
    } hat;
  } u;
} ControllerInputEvent;

/* Describes how a controller has changed.*/
typedef enum ControllerChange {
  CONTROLLER_CHANGE_ADDED,
  CONTROLLER_CHANGE_REMOVED,
  CONTROLLER_CHANGE_INFO,
  CONTROLLER_CHANGE_POWER,
  CONTROLLER_CHANGE_STEAM_HANDLE,
} ControllerChange;

/* Controller change event structure. */
typedef struct ControllerChangeEvent {
  ControllerId id;
  ControllerChange change;
} ControllerChangeEvent;

typedef void (*ControllerInputEventListener)(const ControllerInputEvent*);
typedef void (*ControllerChangeEventListener)(const ControllerChangeEvent*);

/*
 * SystemModel exposes system and platform information from SDL.
 */

void SystemModel_Init(void);
void SystemModel_Drop(void);

const char* SystemModel_GetSdlVersion(void);
const char* SystemModel_GetSdlRevision(void);
const char* SystemModel_GetPlatformName(void);
const char* SystemModel_GetTheme(void);
const char* SystemModel_GetPowerState(void);
int SystemModel_GetBatteryLevel(void);
int SystemModel_GetCpuCoreCount(void);
int SystemModel_GetCpuCacheLineSize(void);
int SystemModel_GetRamMiB(void);

void SystemModel_CopyToClipboard(const char* text);

/*
 * ControllerListModel manages Controllers connected to the system.
 */

void ControllerListModel_Init(void);
void ControllerListModel_Drop(void);
ControllerId* ControllerListModel_SortControllers(int* out_count);
void ControllerListModel_SelectController(ControllerId id);
void ControllerListModel_EnableControllerInputEvents(
    ControllerId controller_id,
    ControllerApi api,
    ControllerInputEventListener listener);
void ControllerListModel_DisableControllerEvents(void);
void ControllerListModel_AddChangeEventListener(
    ControllerChangeEventListener listener);
void ControllerListModel_RemoveChangeEventListener(
    ControllerChangeEventListener listener);
void ControllerListModel_ReloadMappings(void);
int ControllerListModel_LoadMappingsFromClipboard(void);
void ControllerListModel_ExportMappingsToClipboard(void);

/*
 * Controller exposes SDL Joystick and Gamepad information about a specific
 * controller.
 */

const char* Controller_GetName(ControllerId id);
const char* Controller_GetJoystickName(ControllerId id);
const char* Controller_GetGamepadName(ControllerId id);
const char* Controller_GetGUID(ControllerId id);
const char* Controller_GetPath(ControllerId id);
const char* Controller_GetJoystickType(ControllerId id);
const char* Controller_GetGamepadType(ControllerId id);
const char* Controller_GetConnectionType(ControllerId id);
const char* Controller_GetPowerState(ControllerId id);
int Controller_GetBatteryLevel(ControllerId id);
int Controller_GetProduct(ControllerId id);
int Controller_GetProductVersion(ControllerId id);
int Controller_GetFirmwareVersion(ControllerId id);
int Controller_GetVendor(ControllerId id);
const char* Controller_GetSerial(ControllerId id);
bool Controller_IsHaptic(ControllerId id);
int Controller_GetButtonCount(ControllerId id);
int Controller_GetAxisCount(ControllerId id);
int Controller_GetHatCount(ControllerId id);
int Controller_GetBallCount(ControllerId id);
int Controller_GetTouchpadCount(ControllerId id);
uint64_t Controller_GetSteamHandle(ControllerId id);
bool Controller_GetPropertyValue(ControllerId id, int property_index);
float Controller_GetJoystickAxisValue(ControllerId id, int axis);
uint8_t Controller_GetJoystickHatValue(ControllerId id, int hat);
bool Controller_GetJoystickButtonValue(ControllerId id, int button);
float Controller_GetAxisValue(ControllerId id, StandardGamepadKey axis);
bool Controller_GetButtonValue(ControllerId id, StandardGamepadKey button);
const char* Controller_GetMappingString(ControllerId id);
bool Controller_HasMapping(ControllerId id);
bool Controller_HasMappingForKey(ControllerId id, StandardGamepadKey key);
bool Controller_HasRumble(ControllerId id);
int Controller_GetBindingCount(ControllerId id);
const char* Controller_GetBindingName(ControllerId id, int binding_index);
const char* Controller_GetBindingValue(ControllerId id, int binding_index);
void Controller_RemoveMapping(ControllerId id);
void Controller_Rumble(ControllerId id);

int Controller_GetPropertyCount(void);
const char* Controller_GetPropertyName(int property_index);

const char* StandardGamepadKey_ToString(StandardGamepadKey key);

#endif  // SDLI_MODEL_H
