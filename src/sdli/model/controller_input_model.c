#include "model.h"

#include <sdli/app.h>
#include <sdli/util.h>

#include <SDL3/SDL.h>

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

typedef struct ControllerInputModel {
  ControllerId controller;
  ControllerInputEventListener listener;
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
  SLog("%s: success", FN_NAME);
}

void ControllerInputModel_Drop(void)
{
  SLog("%s", FN_NAME);
  ControllerInputModel_Disable();
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

  g_controller_input_model.controller = controller_id;
  g_controller_input_model.listener = listener;
}

void ControllerInputModel_Disable(void)
{
  App_RemoveEventListener(SDL_EVENT_JOYSTICK_BUTTON_DOWN,
                          &OnJoystickInputEvents);
  App_RemoveEventListener(SDL_EVENT_JOYSTICK_BUTTON_UP, &OnJoystickInputEvents);
  App_RemoveEventListener(SDL_EVENT_JOYSTICK_AXIS_MOTION,
                          &OnJoystickInputEvents);
  App_RemoveEventListener(SDL_EVENT_GAMEPAD_BUTTON_DOWN, &OnGamepadInputEvents);
  App_RemoveEventListener(SDL_EVENT_GAMEPAD_BUTTON_UP, &OnGamepadInputEvents);
  App_RemoveEventListener(SDL_EVENT_GAMEPAD_AXIS_MOTION, &OnGamepadInputEvents);

  g_controller_input_model.controller = 0;
  g_controller_input_model.listener = NULL;
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
