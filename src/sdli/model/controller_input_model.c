#include "model.h"

#include <sdli/app.h>
#include <sdli/util.h>

#include <SDL3/SDL.h>
#include <stc/cstr.h>

/*
 * Key press and axis motion handling is based on the SDL gamepad mapper code:
 * https://github.com/libsdl-org/SDL/test/testcontroller.c
 */

//
// macros & constants
//

#define RETURN_IF_NOT_CONTROLLER(ID)                   \
  do {                                                 \
    if (g_controller_input_model.controller != (ID)) { \
      return;                                          \
    }                                                  \
  } while (0)

//
// private types
//

#define i_no_clone
#define i_no_emplace
#define i_type vec_u8
#define i_key uint8_t
#include <stc/vec.h>

typedef struct AxisState {
  bool moving;
  int last_value;
  int starting_value;
  int farthest_value;
} AxisState;

#define i_no_clone
#define i_no_emplace
#define i_type vec_axis_state
#define i_key AxisState
#include <stc/vec.h>

typedef struct ControllerInputModel {
  ControllerId controller;
  ControllerInputEventListener listener;
  SDL_Joystick* joystick;
  /* button id -> button state (last frame) */
  vec_u8 last_button_state;
  /* hat id -> hat state (last frame) */
  vec_u8 last_hat_state;
  /* axis id -> axis state */
  vec_axis_state axis_state;
  SDL_GamepadBinding axis_binding;
} ControllerInputModel;

//
// private function declarations
//

static void OnJoystickInputEvents(int event_type,
                                  void* event_data,
                                  void* user_data);
static void OnGamepadInputEvents(int event_type,
                                 void* event_data,
                                 void* user_data);
static void OnBeforeEvents(int event_type, void* event_data, void* user_data);
static void OnAfterEvents(int event_type, void* event_data, void* user_data);
static void OnJoystickAxisMotion(int event_type,
                                 void* event_data,
                                 void* user_data);
static int StandardizeAxisValue(int value);
static cstr ToAxisValue(const char* prefix,
                        const SDL_GamepadBinding* binding,
                        bool inverted);
static cstr GamepadBinding_ToString(const SDL_GamepadBinding* binding);
static bool HasOnlyOneBitSet(uint32_t value);
static bool GetExclusiveRelease(SDL_GamepadBinding* binding);

//
// global state
//

static ControllerInputModel g_controller_input_model = {0};

//
// public function implementation
//

void ControllerInputModel_Init(void)
{
  SLog("%s", FN_NAME);

  ControllerInputModel* M = &g_controller_input_model;

  M->last_button_state = vec_u8_with_capacity(32);
  M->last_hat_state = vec_u8_with_capacity(32);
  M->axis_state = vec_axis_state_with_capacity(32);

  SLog("%s: success", FN_NAME);
}

void ControllerInputModel_Drop(void)
{
  SLog("%s", FN_NAME);

  ControllerInputModel* M = &g_controller_input_model;

  ControllerInputModel_Disable();

  vec_u8_drop(&M->last_button_state);
  vec_u8_drop(&M->last_hat_state);
  vec_axis_state_drop(&M->axis_state);
}

void ControllerInputModel_Enable(ControllerId controller_id,
                                 ControllerApi api,
                                 ControllerInputEventListener listener)
{
  ControllerInputModel_Disable();

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

  ControllerInputModel* M = &g_controller_input_model;

  M->controller = controller_id;
  M->listener = listener;
}

void ControllerInputModel_EnableMapper(ControllerId controller_id,
                                       ControllerInputEventListener listener)
{
  ControllerInputModel_Disable();

  // TODO: check not enabled
  SDL_Joystick* joystick = SDL_OpenJoystick(controller_id);

  if (!joystick) {
    SLogCallError("SDL_JoystickOpen");
    return;
  }

  ControllerInputModel* M = &g_controller_input_model;

  M->controller = controller_id;
  M->joystick = joystick;
  M->listener = listener;

  const int button_count = SDL_GetNumJoystickButtons(joystick);
  const int hat_count = SDL_GetNumJoystickHats(joystick);
  const int axis_count = SDL_GetNumJoystickAxes(joystick);

  vec_u8_resize(&M->last_button_state, button_count, 0);
  vec_u8_resize(&M->last_hat_state, hat_count, 0);
  vec_axis_state_resize(&M->axis_state, axis_count, (AxisState){0});

  App_AddEventListener(EVT_BEFORE_PROCESS_EVENTS, &OnBeforeEvents, NULL);
  App_AddEventListener(EVT_AFTER_PROCESS_EVENTS, &OnAfterEvents, NULL);
  App_AddEventListener(SDL_EVENT_JOYSTICK_AXIS_MOTION, &OnJoystickAxisMotion,
                       NULL);
}

void ControllerInputModel_Disable(void)
{
  // joystick input events
  App_RemoveEventListener(SDL_EVENT_JOYSTICK_BUTTON_DOWN,
                          &OnJoystickInputEvents);
  App_RemoveEventListener(SDL_EVENT_JOYSTICK_BUTTON_UP, &OnJoystickInputEvents);
  App_RemoveEventListener(SDL_EVENT_JOYSTICK_AXIS_MOTION,
                          &OnJoystickInputEvents);
  // gamepad input events
  App_RemoveEventListener(SDL_EVENT_GAMEPAD_BUTTON_DOWN, &OnGamepadInputEvents);
  App_RemoveEventListener(SDL_EVENT_GAMEPAD_BUTTON_UP, &OnGamepadInputEvents);
  App_RemoveEventListener(SDL_EVENT_GAMEPAD_AXIS_MOTION, &OnGamepadInputEvents);
  // mapper
  App_RemoveEventListener(EVT_BEFORE_PROCESS_EVENTS, &OnBeforeEvents);
  App_RemoveEventListener(EVT_AFTER_PROCESS_EVENTS, &OnAfterEvents);
  App_RemoveEventListener(SDL_EVENT_JOYSTICK_AXIS_MOTION,
                          &OnJoystickAxisMotion);

  ControllerInputModel* M = &g_controller_input_model;

  if (M->joystick) {
    SDL_CloseJoystick(M->joystick);
    M->joystick = NULL;
  }

  M->controller = 0;
  M->listener = NULL;
}

//
// private function implementation
//

static void OnJoystickInputEvents(int event_type,
                                  void* event_data,
                                  void* user_data)
{
  UNUSED(user_data);
  SDL_Event* sdl_event = event_data;
  ControllerInputEvent input_event = {
      .api = CONTROLLER_API_JOYSTICK,
  };

  switch (event_type) {
    case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
    case SDL_EVENT_JOYSTICK_BUTTON_UP:
      RETURN_IF_NOT_CONTROLLER(sdl_event->jbutton.which);
      input_event.type = CONTROLLER_INPUT_BUTTON;
      input_event.u.button.id = sdl_event->jbutton.button;
      input_event.u.button.pressed =
          (event_type == SDL_EVENT_JOYSTICK_BUTTON_DOWN);

      break;
    case SDL_EVENT_JOYSTICK_AXIS_MOTION:
      RETURN_IF_NOT_CONTROLLER(sdl_event->jaxis.which);
      input_event.type = CONTROLLER_INPUT_AXIS;
      input_event.u.axis.id = sdl_event->jaxis.axis;
      input_event.u.axis.value =
          Controller_JoystickAxisToFloat(sdl_event->jaxis.value);
      break;
    case SDL_EVENT_JOYSTICK_HAT_MOTION:
      RETURN_IF_NOT_CONTROLLER(sdl_event->jhat.which);
      input_event.type = CONTROLLER_INPUT_HAT;
      input_event.u.hat.id = sdl_event->jhat.hat;
      input_event.u.hat.value = sdl_event->jhat.value;
      break;
    default:
      return;
  }

  g_controller_input_model.listener(&input_event);
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
      RETURN_IF_NOT_CONTROLLER(sdl_event->gbutton.which);
      input_event.type = CONTROLLER_INPUT_BUTTON;
      input_event.u.button.id = sdl_event->gbutton.button;
      input_event.u.button.pressed =
          (event_type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
      break;
    case SDL_EVENT_GAMEPAD_AXIS_MOTION:
      RETURN_IF_NOT_CONTROLLER(sdl_event->gaxis.which);
      input_event.type = CONTROLLER_INPUT_AXIS;
      input_event.u.axis.id =
          sdl_event->gaxis.axis + STANDARD_GAMEPAD_KEY_AXIS_OFFSET;
      input_event.u.axis.value =
          Controller_JoystickAxisToFloat(sdl_event->gaxis.value);
      break;
    default:
      return;
  }

  g_controller_input_model.listener(&input_event);
}

static void OnBeforeEvents(int event_type, void* event_data, void* user_data)
{
  UNUSED(event_type, event_data, user_data);
  ControllerInputModel* M = &g_controller_input_model;
  SDL_Joystick* joystick = M->joystick;

  if (!joystick) {
    return;
  }

  const isize button_count = M->last_button_state.size;

  for (int i = 0; i < button_count; i++) {
    M->last_button_state.data[i] = (uint8_t)SDL_GetJoystickButton(joystick, i);
  }

  const isize hat_count = M->last_hat_state.size;

  for (int i = 0; i < hat_count; i++) {
    M->last_hat_state.data[i] = SDL_GetJoystickHat(joystick, i);
  }

  M->axis_binding = (SDL_GamepadBinding){0};
}

static void OnAfterEvents(int event_type, void* event_data, void* user_data)
{
  UNUSED(event_type, event_data, user_data);
  ControllerInputModel* M = &g_controller_input_model;

  if (!M->joystick) {
    return;
  }

  SDL_GamepadBinding binding;
  cstr result = cstr_init();

  if (GetExclusiveRelease(&binding)) {
    if (M->axis_binding.input_type == SDL_GAMEPAD_BINDTYPE_NONE) {
      result = GamepadBinding_ToString(&binding);
    }
  } else if (M->axis_binding.input_type != SDL_GAMEPAD_BINDTYPE_NONE) {
    // TODO: this is better, but something is broken..
    // mAxisState[g_mapper.axis_binding.input.axis.axis] = {0};
    result = GamepadBinding_ToString(&M->axis_binding);
  } else {
    // TODO: ignore
  }

  if (!cstr_is_empty(&result)) {
    M->listener(&(ControllerInputEvent){
        .api = CONTROLLER_API_JOYSTICK,
        .type = CONTROLLER_INPUT_BINDING,
        .u.binding.value = cstr_str(&result),
    });
  }

  cstr_drop(&result);
}

static void OnJoystickAxisMotion(int event_type,
                                 void* event_data,
                                 void* user_data)
{
  UNUSED(event_type, event_data, user_data);

  ControllerInputModel* M = &g_controller_input_model;
  SDL_Joystick* joystick = M->joystick;

  if (!joystick) {
    return;
  }

  SDL_Event* event = event_data;
  const int axis_id = event->jaxis.axis;
  const int axis_value = event->jaxis.value;
  const int MAX_ALLOWED_JITTER =
      SDL_JOYSTICK_AXIS_MAX / 80; /* ShanWan PS3 gamepad needed 96 */

  AxisState* axis_state = &M->axis_state.data[axis_id];
  const int value = axis_value;
  int current_distance, farthest_distance;

  if (!axis_state->moving) {
    Sint16 initial_value;
    axis_state->moving =
        SDL_GetJoystickAxisInitialState(joystick, axis_id, &initial_value);
    axis_state->last_value = value;
    axis_state->starting_value = initial_value;
    axis_state->farthest_value = initial_value;
  } else if (SDL_abs(value - axis_state->last_value) <= MAX_ALLOWED_JITTER) {
    return;
  } else {
    axis_state->last_value = value;
  }
  current_distance = SDL_abs(value - axis_state->starting_value);
  farthest_distance =
      SDL_abs(axis_state->farthest_value - axis_state->starting_value);
  if (current_distance > farthest_distance) {
    axis_state->farthest_value = value;
    farthest_distance =
        SDL_abs(axis_state->farthest_value - axis_state->starting_value);
  }

  // If we've gone out far enough and started to come back, let's bind this axis
  if (farthest_distance >= 16000 && current_distance <= 10000) {
    const int axis_min = StandardizeAxisValue(axis_state->starting_value);
    const int axis_max = StandardizeAxisValue(axis_state->farthest_value);
    SDL_GamepadBinding* axis_binding = &M->axis_binding;

    if (axis_binding->input_type == SDL_GAMEPAD_BINDTYPE_NONE) {
      axis_binding->input_type = SDL_GAMEPAD_BINDTYPE_AXIS;
      axis_binding->input.axis.axis = axis_id;
      axis_binding->input.axis.axis_min = axis_min;
      axis_binding->input.axis.axis_max = axis_max;
      axis_binding->output_type = SDL_GAMEPAD_BINDTYPE_NONE;
      axis_binding->output.button = 0;
    } else {
      // TODO: ignore?
    }

    // TODO: reset, but need to think about it...
    // TODO: reset when enable binding event?
    axis_state->farthest_value = axis_state->starting_value;
  }
}

static cstr ToAxisValue(const char* prefix,
                        const SDL_GamepadBinding* binding,
                        bool inverted)
{
  return cstr_from_fmt("%sa%" PRIi32 "%s", prefix,
                       (int32_t)(binding->input.axis.axis),
                       inverted ? "~" : "");
}

static cstr GamepadBinding_ToString(const SDL_GamepadBinding* binding)
{
  cstr result = cstr_init();

  if (binding->input_type == SDL_GAMEPAD_BINDTYPE_BUTTON) {
    result = cstr_from_fmt("b%" PRIi32, (int32_t)(binding->input.button));
  } else if (binding->input_type == SDL_GAMEPAD_BINDTYPE_HAT) {
    result = cstr_from_fmt("h%" PRIi32 ".%" PRIi32,
                           (int32_t)(binding->input.hat.hat),
                           (int32_t)(binding->input.hat.hat_mask));
  } else if (binding->input_type == SDL_GAMEPAD_BINDTYPE_AXIS) {
    if (binding->input.axis.axis_min == 0) {
      if (binding->input.axis.axis_max == SDL_JOYSTICK_AXIS_MAX) {
        result = ToAxisValue("+", binding, false);
      } else {
        result = ToAxisValue("-", binding, false);
      }
    } else {
      result = ToAxisValue("", binding, false);
    }
  } else {
    if (binding->input.axis.axis_min == 0 &&
        binding->input.axis.axis_max == SDL_JOYSTICK_AXIS_MIN) {
      // The negative half axis
      result = ToAxisValue("-", binding, false);
    } else if (binding->input.axis.axis_min == 0 &&
               binding->input.axis.axis_max == SDL_JOYSTICK_AXIS_MAX) {
      // The positive half axis
      result = ToAxisValue("+", binding, false);
    } else {
      if (binding->input.axis.axis_min > binding->input.axis.axis_max) {
        // Invert the axis
        result = ToAxisValue("", binding, true);
      } else {
        result = ToAxisValue("", binding, false);
      }
    }
  }

  return result;
}

static int StandardizeAxisValue(int value)
{
  if (value > SDL_JOYSTICK_AXIS_MAX / 2) {
    return SDL_JOYSTICK_AXIS_MAX;
  } else if (value < SDL_JOYSTICK_AXIS_MIN / 2) {
    return SDL_JOYSTICK_AXIS_MIN;
  } else {
    return 0;
  }
}

static bool GetExclusiveRelease(SDL_GamepadBinding* binding)
{
  ControllerInputModel* M = &g_controller_input_model;
  SDL_Joystick* joystick = M->joystick;
  const int button_count = (int)M->last_button_state.size;
  int button = -1;

  // check that only one button has been released this frame.
  for (int i = 0; i < button_count; i++) {
    if (M->last_button_state.data[i]) {
      // if another button was down last frame, bail.
      if (button >= 0) {
        return false;
      }

      button = i;

      // if the button is still down this frame, bail.
      if (SDL_GetJoystickButton(joystick, i)) {
        return false;
      }
    }
  }

  const int hat_count = (int)M->last_hat_state.size;
  int hat = -1;
  uint8_t hat_dir = 0;

  // check that only one hat direction has been released this frame.
  for (int i = 0; i < hat_count; i++) {
    const uint8_t hat_state = M->last_hat_state.data[i];

    // if something is down last frame, bail.
    if (hat_state == SDL_HAT_CENTERED) {
      continue;
    }

    // do not consider multiple hat dirs. only one.
    if (HasOnlyOneBitSet((uint32_t)(hat_state))) {
      if (hat >= 0) {
        return false;
      }

      hat = i;
      hat_dir = hat_state;
    } else {
      return false;
    }

    // must be released this frame.
    if (SDL_GetJoystickHat(joystick, i) != SDL_HAT_CENTERED) {
      return false;
    }
  }

  // report the binding.
  if (hat >= 0) {
    if (button >= 0) {
      // hat + button were released this frame. bail.
      return false;
    }
    binding->input_type = SDL_GAMEPAD_BINDTYPE_HAT;
    binding->input.hat.hat = hat;
    binding->input.hat.hat_mask = (int)(hat_dir);
  } else if (button >= 0) {
    binding->input_type = SDL_GAMEPAD_BINDTYPE_BUTTON;
    binding->input.button = button;
  } else {
    return false;
  }

  return true;
}

static bool HasOnlyOneBitSet(uint32_t value)
{
  return (value != 0) && ((value & (value - 1)) == 0);
}
