#include "screen.h"

#include <sdli/model/model.h>
#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

#include <stc/cstr.h>

//
// private types
//

typedef struct HatDisplayInfo {
  const char* icon;
  ControllerPovHatMask mask;
} HatDisplayInfo;

//
// constants
//

#define NID_CONTROLLER_NAME "cevt:cname"
#define NID_API_BOX "cevt:api"
#define NID_API_TOGGLE "cevt:api-toggle"
#define NID_JOYSTICK_API "cevt:joy"
#define NID_GAMEPAD_API "cevt:gpad"
#define NID_GAMEPAD_LEFT_JOYSTICK_VALUE "cevt:gla"
#define NID_GAMEPAD_RIGHT_JOYSTICK_VALUE "cevt:gra"
#define NID_GAMEPAD_LEFT_TRIGGER_VALUE "cevt:glt"
#define NID_GAMEPAD_RIGHT_TRIGGER_VALUE "cevt:grt"
#define NID_GAMEPAD_LEFT_X_VALUE "cevt:glx"
#define NID_GAMEPAD_LEFT_Y_VALUE "cevt:gly"
#define NID_GAMEPAD_RIGHT_X_VALUE "cevt:grx"
#define NID_GAMEPAD_RIGHT_Y_VALUE "cevt:gry"

#define CLS_EV_SCREEN "ev-screen"
#define CLS_EV_HEADER "ev-header"
#define CLS_EV_BODY "ev-body"
#define CLS_EV_CONTROLLER_NAME "ev-ctrl-name"
#define CLS_EVJ_BOX "evj-box"
#define CLS_EVJ_BUTTON "evj-btn"
#define CLS_EVJ_BUTTON_DOWN "evj-btn-down"
#define CLS_EVJ_BUTTON_TEXT "evj-btn-text"
#define CLS_EVJ_BUTTON_TEXT_UNMAPPED "evj-btn-text-u"
#define CLS_EVJ_BUTTON_TEXT_PRESS "evj-btn-text-press"
#define CLS_EVJ_GROUP "evj-group"
#define CLS_EVJ_AXIS "evj-axis"
#define CLS_EVJ_AXIS_KEY "evj-axis-key"
#define CLS_EVJ_AXIS_VALUE "evj-axis-val"
#define CLS_EVG_BUTTON_GRID "evg-btn-grid"
#define CLS_EVG_BUTTON_ROW "evg-btn-row"
#define CLS_EVG_BUTTON "evg-btn"
#define CLS_EVG_BUTTON_PLACEHOLDER "evg-btn-placeholder"
#define CLS_EVG_BUTTON_DOWN "evg-btn-down"
#define CLS_EVG_BUTTON_UNMAPPED "evg-btn-u"
#define CLS_EVG_AXIS_GROUP "evg-axis-group"
#define CLS_EVG_AXIS_BOX "evg-axis-box"
#define CLS_EVG_AXIS_SPACER "evg-axis-spacer"
#define CLS_EVG_AXIS_KV_ITEM_FIRST "evg-axis-kv-first"
#define CLS_EVG_AXIS_KV_ITEM "evg-axis-kv"
#define CLS_EVG_AXIS_KEY "evg-axis-key"
#define CLS_EVG_AXIS_VALUE "evg-axis-val"
#define CLS_DPAD "dpad"
#define CLS_DPAD_DOWN "dpad-down"
#define CLS_DPAD_ROW "dpad-row"
#define CLS_DPAD_ICON "dpad-icon"
#define CLS_DPAD_ICON_DOWN "dpad-icon-down"
#define CLS_DPAD_BLOCK "dpad-block"

#define JOYSTICK_BUTTON_ID_FMT "jbtn:%i"
#define JOYSTICK_HAT_ID_FMT "jhat:%i"
#define JOYSTICK_HAT_DIRECTION_ID_FMT "jhat:%ix%i"
#define JOYSTICK_AXIS_ID_FMT "jaxis:%i"

#define GAMEPAD_BUTTON_ID_FMT "gbtn:%i"

#define JOYSTICK_BUTTON_DISPLAY_FMT "b%i"
#define JOYSTICK_HAT_DISPLAY_FMT "h%i"
#define JOYSTICK_AXIS_DISPLAY_FMT "a%i"

#define JOYSTICK_API_DATA ((void*)(intptr_t)CONTROLLER_API_JOYSTICK)
#define GAMEPAD_API_DATA ((void*)(intptr_t)CONTROLLER_API_GAMEPAD)

static const HatDisplayInfo HAT_DISPLAY_INFO[] = {
    {ICON_UP, POV_HAT_MASK_UP},
    {ICON_DOWN, POV_HAT_MASK_DOWN},
    {ICON_LEFT, POV_HAT_MASK_LEFT},
    {ICON_RIGHT, POV_HAT_MASK_RIGHT},
};
static const size_t HAT_DISPLAY_INFO_SIZE = u_arraylen(HAT_DISPLAY_INFO);

// clang-format off
static const StandardGamepadKey BUTTON_GRID[STANDARD_GAMEPAD_BUTTON_COUNT + 2] = {
    SGK_BUTTON_SOUTH,
    SGK_BUTTON_EAST,
    SGK_BUTTON_WEST,
    SGK_BUTTON_NORTH,
    SGK_BUTTON_DPAD_UP,
    SGK_BUTTON_DPAD_DOWN,
    SGK_BUTTON_DPAD_LEFT,
    SGK_BUTTON_DPAD_RIGHT,
    SGK_BUTTON_LEFT_SHOULDER,
    SGK_BUTTON_GUIDE,
    SGK_BUTTON_BACK,
    SGK_BUTTON_RIGHT_SHOULDER,
    SGK_BUTTON_LEFT_PADDLE1,
    SGK_BUTTON_START,
    SGK_BUTTON_TOUCHPAD,
    SGK_BUTTON_RIGHT_PADDLE1,
    SGK_BUTTON_LEFT_PADDLE2,
    SGK_BUTTON_MISC1,
    SGK_BUTTON_MISC2,
    SGK_BUTTON_RIGHT_PADDLE2,
    SGK_BUTTON_LEFT_STICK,
    SGK_BUTTON_MISC3,
    SGK_BUTTON_MISC4,
    SGK_BUTTON_RIGHT_STICK,
    SGK_INVALID,
    SGK_BUTTON_MISC5,
    SGK_BUTTON_MISC6,
    SGK_INVALID,
};
static const int BUTTON_GRID_ROW = 4;
static const int BUTTON_GRID_SIZE = (int)u_arraylen(BUTTON_GRID);
// clang-format on

//
// private function declarations
//

static void JoystickEvents(NN_CALLABLE,
                           ControllerId controller_id,
                           bool visible);
static void GamepadEvents(NN_CALLABLE,
                          ControllerId controller_id,
                          bool visible);
static void DirectionPadBlock(NN_CALLABLE,
                              int hat_id,
                              ControllerPovHatMask direction);
static void DirectionPad(NN_CALLABLE, int hat_id);
static void JoystickButton(NN_CALLABLE, int button_id);
static VNode* JoystickButton_GetText(VNode* button);
static void JoystickAxis(NN_CALLABLE, int axis_id);
static void OnJoystickControllerInputEvent(const ControllerInputEvent* event);
static void OnGamepadControllerInputEvent(const ControllerInputEvent* event);
static void OnNavigatorEvent(NavigatorEvent* event);
static void SetControllerApi(VNode* toggle, ControllerApi new_api);
static void UpdateGamepadAxisValue(int sgk_axis, float value);
static void UpdateGamepadButtonValue(int sgk_button, bool pressed);
static void UpdateJoystickAxisValue(int axis_id, float value);
static void UpdateJoystickButtonValue(int button_id, bool pressed);
static void UpdateJoystickHatValue(int hat_id, uint8_t hat_value);
static VNode* DirectionPadBlock_GetText(VNode* block);
static void StyleSheet(void);

//
// private node event handlers
//

// clang-format off
OnClickInline(Back, {
  // TODO: this should be a back navigation, not a hard goto
  ScreenNavigator_Goto(SCREENID_HOME);
})

OnClickInline(SetApi, {
  SetControllerApi(node, (ControllerApi)(intptr_t)v_node_data(node));
})

// clang-format on

//
// public function implementation
//

VNode* ControllerEventsScreen(void)
{
  StyleSheet();

  NN_BUILD_NEW(screen)
  {
    NN_BOX({.id = SCREENID_CONTROLLER_EVENTS, .sclass = CLS_EV_SCREEN})
    {
      NN_BOX({.sclass = CLS_EV_HEADER})
      {
        NN_TEXT({
            .id = NID_CONTROLLER_NAME,
            .sclass = CLS_EV_CONTROLLER_NAME,
        });

        static const SelectItem apis[] = {
            {.label = "Joystick API", .data = JOYSTICK_API_DATA},
            {.label = "Gamepad API", .data = GAMEPAD_API_DATA},
        };

        NN_CALL(Select, apis, u_arraylen(apis), JOYSTICK_API_DATA, &SetApi_OnClick);
        NN_CALL(Button, "Back", NULL, &Back_OnClick);
      }
      NN_BOX({.id = NID_API_BOX, .sclass = CLS_EV_BODY});
    }
  }

  return Navigable_Init(screen, &OnNavigatorEvent);
}

//
// private function implementation
//

static void OnJoystickControllerInputEvent(const ControllerInputEvent* event)
{
  switch (event->type) {
    case CONTROLLER_INPUT_BUTTON: {
      UpdateJoystickButtonValue(event->u.button.id, event->u.button.pressed);
      break;
    }
    case CONTROLLER_INPUT_HAT: {
      UpdateJoystickHatValue(event->u.hat.id, event->u.hat.value);
      break;
    }
    case CONTROLLER_INPUT_AXIS: {
      UpdateJoystickAxisValue(event->u.axis.id, event->u.axis.value);
      break;
    }
    default:
      break;
  }
}

static void OnGamepadControllerInputEvent(const ControllerInputEvent* event)
{
  switch (event->type) {
    case CONTROLLER_INPUT_BUTTON: {
      UpdateGamepadButtonValue(event->u.button.id, event->u.button.pressed);
      break;
    }
    case CONTROLLER_INPUT_AXIS: {
      UpdateGamepadAxisValue(event->u.axis.id, event->u.axis.value);
      break;
    }
    default:
      break;
  }
}

static void OnNavigatorEvent(NavigatorEvent* event)
{
  if (event->type == NAVIGATOR_EVENT_ENTER) {
    const ControllerId controller_id = State_GetSelectedController();
    VNode* api_box = v_get_node_by_id(NID_API_BOX);

    BindString(NID_CONTROLLER_NAME, Controller_GetName(controller_id));

    v_node_remove_children(api_box);

    NN_BUILD_APPEND(api_box)
    {
      NN_CALL(JoystickEvents, controller_id, false);
      NN_CALL(GamepadEvents, controller_id, false);
    }

    SetControllerApi(v_get_node_by_id(NID_API_TOGGLE), CONTROLLER_API_JOYSTICK);
  }
}

static void SetControllerApi(VNode* toggle, ControllerApi new_api)
{
  ControllerId controller_id = State_GetSelectedController();
  ControllerInputEventListener event_handler = NULL;
  bool joystick_visible = false;
  bool gamepad_visible = false;
  const char* label = "";
  StandardGamepadKey sgk_axis;

  if (new_api == CONTROLLER_API_JOYSTICK) {
    label = "Joystick API";
    joystick_visible = true;
    event_handler = &OnJoystickControllerInputEvent;

    // use the current state of the controller for initial values in case the
    // user is holding down a button or axis

    int count = Controller_GetAxisCount(controller_id);

    for (int i = 0; i < count; i++) {
      UpdateJoystickAxisValue(
          i, Controller_GetJoystickAxisValue(controller_id, i));
    }

    count = Controller_GetHatCount(controller_id);

    for (int i = 0; i < count; i++) {
      UpdateJoystickHatValue(i,
                             Controller_GetJoystickHatValue(controller_id, i));
    }

    count = Controller_GetButtonCount(controller_id);

    for (int i = 0; i < count; i++) {
      UpdateJoystickButtonValue(
          i, Controller_GetJoystickButtonValue(controller_id, i));
    }

  } else if (new_api == CONTROLLER_API_GAMEPAD) {
    label = "Gamepad API";
    gamepad_visible = true;
    event_handler = &OnGamepadControllerInputEvent;

    // use the current state of the controller for initial values in case the
    // user is holding down a button or axis

    for (int i = 0; i < STANDARD_GAMEPAD_BUTTON_COUNT; i++) {
      UpdateGamepadButtonValue(i, Controller_GetButtonValue(controller_id, i));
    }

    for (int i = 0; i < STANDARD_GAMEPAD_KEY_AXIS_OFFSET; i++) {
      // TODO: check mapping
      sgk_axis = (StandardGamepadKey)(i + STANDARD_GAMEPAD_KEY_AXIS_OFFSET);
      UpdateGamepadAxisValue(sgk_axis,
                             Controller_GetAxisValue(controller_id, sgk_axis));
    }
  } else {
    return;
  }

  Button_SetLabel(toggle, label);
  v_node_set_visible(v_get_node_by_id(NID_GAMEPAD_API), gamepad_visible);
  v_node_set_visible(v_get_node_by_id(NID_JOYSTICK_API), joystick_visible);
  v_node_set_data(toggle, (void*)(intptr_t)new_api);
  ControllerInputModel_Enable(controller_id, new_api, event_handler);
}

static void JoystickEvents(NN_CALLABLE,
                           ControllerId controller_id,
                           bool visible)
{
  const int button_count = Controller_GetButtonCount(controller_id);
  const int hat_count = Controller_GetHatCount(controller_id);
  const int axis_count = Controller_GetAxisCount(controller_id);

  NN_BOX({.id = NID_JOYSTICK_API, .sclass = CLS_EVJ_BOX, .hidden = !visible})
  {
    if (button_count > 0) {
      NN_BOX({.sclass = CLS_EVJ_GROUP})
      {
        for (int i = 0; i < button_count; i++) {
          NN_CALL(JoystickButton, i);
        }
      }
    }

    if (hat_count > 0) {
      NN_BOX({.sclass = CLS_EVJ_GROUP})
      {
        for (int i = 0; i < hat_count; i++) {
          NN_CALL(DirectionPad, i);
        }
      }
    }

    if (axis_count > 0) {
      NN_BOX({.sclass = CLS_EVJ_GROUP})
      {
        for (int i = 0; i < axis_count; i++) {
          NN_CALL(JoystickAxis, i);
        }
      }
    }
  }
}

static void GamepadEvents(NN_CALLABLE, ControllerId controller_id, bool visible)
{
  UNUSED(controller_id);
  const int cols = BUTTON_GRID_ROW;
  const int rows = (BUTTON_GRID_SIZE + cols - 1) / cols;

  // TODO: should be a static assert
  assert(BUTTON_GRID_SIZE % cols == 0);

  NN_BOX({.id = NID_GAMEPAD_API,
          .sclass = CLS_EVG_BUTTON_GRID,
          .hidden = !visible})
  {
    for (int r = 0; r < rows; r++) {
      NN_BOX({.sclass = CLS_EVG_BUTTON_ROW})
      {
        for (int c = 0; c < cols; c++) {
          const int idx = r * cols + c;
          const StandardGamepadKey sgk = BUTTON_GRID[idx];

          NN_BOX({0})
          {
            if (sgk != SGK_INVALID) {
              const bool has_mapping =
                  Controller_HasMappingForKey(controller_id, sgk);
              const char* text_class = has_mapping
                                           ? CLS_EVJ_BUTTON_TEXT
                                           : CLS_EVJ_BUTTON_TEXT_UNMAPPED;
              const char* button_class =
                  has_mapping ? CLS_EVG_BUTTON : CLS_EVG_BUTTON_UNMAPPED;

              v_node_set_id_fmt(NN_SELF(), GAMEPAD_BUTTON_ID_FMT, (int)sgk);
              v_node_style_assign_class(NN_SELF(), button_class);
              v_node_set_data(NN_SELF(), (void*)(intptr_t)has_mapping);

              NN_TEXT({
                  .sclass = text_class,
                  .text = StandardGamepadKey_ToString(sgk),
              });
            } else {
              v_node_style_assign_class(NN_SELF(), CLS_EVG_BUTTON_PLACEHOLDER);
            }
          }
        }
      }
    }

    NN_BOX({.sclass = CLS_EVG_AXIS_GROUP})
    {
      NN_BOX({.sclass = CLS_EVG_AXIS_BOX})
      {
        NN_BOX({.sclass = CLS_EVG_AXIS_KV_ITEM_FIRST})
        {
          NN_TEXT({.sclass = CLS_EVG_AXIS_KEY, .text = STR(SID_LEFT_TRIGGER)});
          NN_TEXT({.id = NID_GAMEPAD_LEFT_TRIGGER_VALUE,
                   .sclass = CLS_EVG_AXIS_VALUE});
        }
        NN_BOX({.sclass = CLS_EVG_AXIS_KV_ITEM})
        {
          NN_TEXT({.sclass = CLS_EVG_AXIS_KEY, .text = STR(SID_LEFT_JOYSTICK)});
          NN_TEXT(
              {.id = NID_GAMEPAD_LEFT_X_VALUE, .sclass = CLS_EVG_AXIS_VALUE});
          NN_TEXT(
              {.id = NID_GAMEPAD_LEFT_Y_VALUE, .sclass = CLS_EVG_AXIS_VALUE});
        }
      }

      NN_BOX({
          .sclass = CLS_EVG_AXIS_SPACER,
      });

      NN_BOX({.sclass = CLS_EVG_AXIS_BOX})
      {
        NN_BOX({.sclass = CLS_EVG_AXIS_KV_ITEM_FIRST})
        {
          NN_TEXT({.sclass = CLS_EVG_AXIS_KEY, .text = STR(SID_RIGHT_TRIGGER)});
          NN_TEXT({.id = NID_GAMEPAD_RIGHT_TRIGGER_VALUE,
                   .sclass = CLS_EVG_AXIS_VALUE});
        }
        NN_BOX({.sclass = CLS_EVG_AXIS_KV_ITEM})
        {
          NN_TEXT(
              {.sclass = CLS_EVG_AXIS_KEY, .text = STR(SID_RIGHT_JOYSTICK)});
          NN_TEXT(
              {.id = NID_GAMEPAD_RIGHT_X_VALUE, .sclass = CLS_EVG_AXIS_VALUE});
          NN_TEXT(
              {.id = NID_GAMEPAD_RIGHT_Y_VALUE, .sclass = CLS_EVG_AXIS_VALUE});
        }
      }
    }
  }
}

static void DirectionPadBlock(NN_CALLABLE,
                              int hat_id,
                              ControllerPovHatMask direction)
{
  const char* text = "";

  for (size_t i = 0; i < HAT_DISPLAY_INFO_SIZE; i++) {
    if (HAT_DISPLAY_INFO[i].mask == direction) {
      text = HAT_DISPLAY_INFO[i].icon;
      break;
    }
  }

  NN_BOX({.sclass = CLS_DPAD_BLOCK})
  {
    v_node_set_id_fmt(NN_SELF(), JOYSTICK_HAT_DIRECTION_ID_FMT, hat_id,
                      (int)direction);
    NN_TEXT({.text = text, .sclass = CLS_DPAD_ICON});
  }
}

static void DirectionPad(NN_CALLABLE, int hat_id)
{
  NN_BOX({.sclass = CLS_DPAD})
  {
    v_node_set_id_fmt(NN_SELF(), JOYSTICK_HAT_ID_FMT, hat_id);
    NN_BOX({.sclass = CLS_DPAD_ROW})
    {
      NN_CALL(DirectionPadBlock, hat_id, POV_HAT_MASK_UP);
    }
    NN_BOX({.sclass = CLS_DPAD_ROW})
    {
      NN_CALL(DirectionPadBlock, hat_id, POV_HAT_MASK_LEFT);
      NN_BOX({.sclass = CLS_DPAD_BLOCK})
      {
        NN_TEXT({.sclass = CLS_TEXT})
        {
          v_node_set_text_fmt(NN_SELF(), JOYSTICK_HAT_DISPLAY_FMT, hat_id);
        }
      }
      NN_CALL(DirectionPadBlock, hat_id, POV_HAT_MASK_RIGHT);
    }

    NN_BOX({.sclass = CLS_DPAD_ROW})
    {
      NN_CALL(DirectionPadBlock, hat_id, POV_HAT_MASK_DOWN);
    }
  }
}

static void JoystickButton(NN_CALLABLE, int button_id)
{
  NN_BOX({.sclass = CLS_EVJ_BUTTON, .data = (void*)(uintptr_t)button_id})
  {
    v_node_set_id_fmt(NN_SELF(), JOYSTICK_BUTTON_ID_FMT, button_id);
    NN_TEXT({.sclass = CLS_EVJ_BUTTON_TEXT})
    {
      v_node_set_text_fmt(NN_SELF(), JOYSTICK_BUTTON_DISPLAY_FMT, button_id);
    }
  }
}

static VNode* JoystickButton_GetText(VNode* button)
{
  return v_node_first_child(button);
}

static void JoystickAxis(NN_CALLABLE, int axis_id)
{
  NN_BOX({.sclass = CLS_EVJ_AXIS})
  {
    NN_TEXT({.sclass = CLS_EVJ_AXIS_KEY})
    {
      v_node_set_text_fmt(NN_SELF(), JOYSTICK_AXIS_DISPLAY_FMT, axis_id);
    }
    NN_TEXT({.sclass = CLS_EVJ_AXIS_VALUE})
    {
      v_node_set_id_fmt(NN_SELF(), JOYSTICK_AXIS_ID_FMT, axis_id);
    }
  }
}

static void UpdateGamepadAxisValue(int sgk_axis, float value)
{
  const char* id = NULL;
  const char* post_fix = "";

  switch (sgk_axis) {
    case SGK_AXIS_LEFT_TRIGGER:
      id = NID_GAMEPAD_LEFT_TRIGGER_VALUE;
      break;
    case SGK_AXIS_RIGHT_TRIGGER:
      id = NID_GAMEPAD_RIGHT_TRIGGER_VALUE;
      break;
    case SGK_AXIS_LEFTX:
      id = NID_GAMEPAD_LEFT_X_VALUE;
      post_fix = ", ";
      break;
    case SGK_AXIS_LEFTY:
      id = NID_GAMEPAD_LEFT_Y_VALUE;
      break;
    case SGK_AXIS_RIGHTX:
      id = NID_GAMEPAD_RIGHT_X_VALUE;
      post_fix = ", ";
      break;
    case SGK_AXIS_RIGHTY:
      id = NID_GAMEPAD_RIGHT_Y_VALUE;
      break;
    default:
      return;
  }

  v_node_set_text_fmt(v_get_node_by_id(id), "%0.3f%s", value, post_fix);
}

static void UpdateGamepadButtonValue(int button_id, bool pressed)
{
  VNode* node = v_get_node_by_id_fmt(GAMEPAD_BUTTON_ID_FMT, button_id);
  const bool has_mapping = (intptr_t)v_node_data(node) != 0;

  if (has_mapping) {
    v_node_style_assign_class(node,
                              pressed ? CLS_EVG_BUTTON_DOWN : CLS_EVG_BUTTON);
    v_node_style_assign_class(
        v_node_first_child(node),
        pressed ? CLS_EVJ_BUTTON_TEXT_PRESS : CLS_EVJ_BUTTON_TEXT);
  }
}

static void UpdateJoystickAxisValue(int axis_id, float value)
{
  v_node_set_text_fmt(v_get_node_by_id_fmt(JOYSTICK_AXIS_ID_FMT, axis_id),
                      "%0.3f", value);
}

static void UpdateJoystickHatValue(int hat_id, uint8_t hat_value)
{
  for (size_t i = 0; i < HAT_DISPLAY_INFO_SIZE; i++) {
    const int mask = (int)HAT_DISPLAY_INFO[i].mask;
    const bool pressed = (hat_value & mask) != 0;

    VNode* dpad_block =
        v_get_node_by_id_fmt(JOYSTICK_HAT_DIRECTION_ID_FMT, hat_id, mask);

    v_node_style_assign_class(DirectionPadBlock_GetText(dpad_block),
                              pressed ? CLS_DPAD_ICON_DOWN : CLS_DPAD_ICON);
  }

  // TODO: not sure i like this effect
  // const bool any_pressed = hat_value != 0;
  // VNode* dpad = v_get_node_by_id_fmt(JOYSTICK_HAT_ID_FMT, hat_id);

  // v_node_style_assign_class(dpad, any_pressed ? CLS_DPAD_DOWN : CLS_DPAD);
}

static void UpdateJoystickButtonValue(int button_id, bool pressed)
{
  VNode* button = v_get_node_by_id_fmt(JOYSTICK_BUTTON_ID_FMT, button_id);

  v_node_style_assign_class(button,
                            pressed ? CLS_EVJ_BUTTON_DOWN : CLS_EVJ_BUTTON);
  v_node_style_assign_class(
      JoystickButton_GetText(button),
      pressed ? CLS_EVJ_BUTTON_TEXT_PRESS : CLS_EVJ_BUTTON_TEXT);
}

static VNode* DirectionPadBlock_GetText(VNode* block)
{
  return v_node_first_child(block);
}

static void StyleSheet(void)
{
  float max_width = 0;
  VStyle* text_style_class = vss_get_class(CLS_TEXT);

  for (int i = 0; i < STANDARD_GAMEPAD_BUTTON_COUNT; i++) {
    const float width = v_style_measure_text_w(
        text_style_class, StandardGamepadKey_ToString((StandardGamepadKey)i));

    if (width > max_width) {
      max_width = width;
    }
  }

  vss_extend(S, CLS_EV_SCREEN, CLS_FILL)
  {
    vs_set_direction(S, V_DIRECTION_COLUMN);
  }

  vss_with(S, CLS_EV_HEADER)
  {
    vs_set_direction(S, V_DIRECTION_ROW);
    vs_set_gap(S, THEME_SP_SM);
    vs_set_width(S, V_GROW());
  }

  vss_with(S, CLS_EV_BODY)
  {
    vs_set_xalign(S, V_ALIGN_X_CENTER);
    vs_set_yalign(S, V_ALIGN_Y_CENTER);
    vs_set_width(S, V_GROW());
    vs_set_height(S, V_GROW());
    vs_set_gap(S, THEME_SP_LG);
    vs_set_padding_top(S, THEME_SP_MD);
  }

  vss_extend(S, CLS_EV_CONTROLLER_NAME, CLS_TEXT)
  {
    vs_set_width(S, V_GROW());
  }

  vss_with(S, CLS_EVJ_BOX)
  {
    vs_set_direction(S, V_DIRECTION_COLUMN);
    vs_set_gap(S, THEME_SP_MD);
    vs_set_width(S, V_GROW());
  }

  vss_with(S, CLS_EVJ_BUTTON)
  {
    vs_set_border_color(S, THEME_TEXT_COLOR);
    vs_set_border_radius(S, 1000);
    vs_set_border_all(S, THEME_EVENT_BUTTON_BORDER);
    vs_set_width(S, V_FIXED(THEME_JOYSTICK_BUTTON_SIZE));
    vs_set_height(S, V_FIXED(THEME_JOYSTICK_BUTTON_SIZE));
    vs_set_xalign(S, V_ALIGN_X_CENTER);
    vs_set_yalign(S, V_ALIGN_Y_CENTER);
  }

  vss_with(S, CLS_EVG_BUTTON_GRID)
  {
    vs_set_direction(S, V_DIRECTION_COLUMN);
    vs_set_gap(S, THEME_SP_SM);
  }

  vss_with(S, CLS_EVG_BUTTON_ROW)
  {
    vs_set_gap(S, THEME_SP_SM);
  }

  vss_extend(S, CLS_EVJ_BUTTON_DOWN, CLS_EVJ_BUTTON)
  {
    vs_set_border_color(S, THEME_APP_TITLE_COLOR);
  }

  vss_extend(S, CLS_EVJ_BUTTON_TEXT, CLS_TEXT)
  {
    vs_set_talign(S, V_ALIGN_X_CENTER);
  }

  vss_extend(S, CLS_EVJ_BUTTON_TEXT_PRESS, CLS_EVJ_BUTTON_TEXT)
  {
    vs_set_color(S, THEME_APP_TITLE_COLOR);
  }

  vss_extend(S, CLS_EVJ_BUTTON_TEXT_UNMAPPED, CLS_EVJ_BUTTON_TEXT)
  {
    vs_set_color(S, THEME_UNMAPPED_COLOR);
  }

  vss_with(S, CLS_EVJ_GROUP)
  {
    vs_set_width(S, V_GROW());
    vs_set_xalign(S, V_ALIGN_X_CENTER);
    vs_set_direction(S, V_DIRECTION_ROW);
    vs_set_gap(S, THEME_SP_2XS);
  }

  vss_with(S, CLS_EVJ_AXIS)
  {
    vs_set_border_color(S, THEME_TEXT_COLOR);
    vs_set_border_radius(S, 1000);
    vs_set_border_all(S, THEME_EVENT_BUTTON_BORDER);
    vs_set_yalign(S, V_ALIGN_Y_CENTER);
    vs_set_height(S, V_FIXED(THEME_JOYSTICK_BUTTON_SIZE));
    vs_set_gap(S, THEME_SP_XS);
    vs_set_padding(S, 0, THEME_SP_MD, 0, THEME_SP_MD);
  }

  vss_extend(S, CLS_EVJ_AXIS_KEY, CLS_TEXT)
  {
    vs_set_width(
        S, V_FIXED(v_style_measure_text_w(text_style_class, "a88") + 1.f));
  }

  vss_extend(S, CLS_EVJ_AXIS_VALUE, CLS_TEXT)
  {
    vs_set_talign(S, V_ALIGN_X_RIGHT);
    vs_set_width(S,
                 V_FIXED(v_style_measure_text_w(text_style_class, "-8.888")));
  }

  vss_with(S, CLS_EVG_BUTTON)
  {
    const uint16_t half_height = THEME_GAMEPAD_BUTTON_HEIGHT / 2;

    vs_set_border_color(S, THEME_TEXT_COLOR);
    vs_set_border_radius(S, 1000);
    vs_set_border_all(S, THEME_EVENT_BUTTON_BORDER);
    vs_set_width(S, V_FIXED(max_width + (float)(half_height * 2)));
    vs_set_height(S, V_FIXED(THEME_GAMEPAD_BUTTON_HEIGHT));
    vs_set_xalign(S, V_ALIGN_X_CENTER);
    vs_set_yalign(S, V_ALIGN_Y_CENTER);
  }

  vss_extend(S, CLS_EVG_BUTTON_PLACEHOLDER, CLS_EVG_BUTTON)
  {
    vs_unset_border_color(S);
  }

  vss_extend(S, CLS_EVG_BUTTON_DOWN, CLS_EVG_BUTTON)
  {
    vs_set_border_color(S, THEME_APP_TITLE_COLOR);
  }

  vss_extend(S, CLS_EVG_BUTTON_UNMAPPED, CLS_EVG_BUTTON)
  {
    vs_set_border_color(S, THEME_UNMAPPED_COLOR);
  }

  vss_with(S, CLS_EVG_AXIS_GROUP)
  {
    vs_set_padding_top(S, THEME_SP_MD);
    vs_set_direction(S, V_DIRECTION_ROW);
    vs_set_width(S, V_GROW());
  }

  vss_with(S, CLS_EVG_AXIS_BOX)
  {
    vs_set_width(S, V_GROW());
    vs_set_direction(S, V_DIRECTION_COLUMN);
  }

  vss_with(S, CLS_EVG_AXIS_SPACER)
  {
    vs_set_width(S, V_FIXED(100));
    vs_set_height(S, V_FIXED(1));
  }

  vss_with(S, CLS_EVG_AXIS_KV_ITEM)
  {
    vs_set_padding_top(S, THEME_SP_SM);
    vs_set_direction(S, V_DIRECTION_ROW);
    vs_set_width(S, V_GROW());
  }

  vss_extend(S, CLS_EVG_AXIS_KV_ITEM_FIRST, CLS_EVG_AXIS_KV_ITEM)
  {
    vs_set_padding(S, 0, 0, THEME_SP_SM, 0);
    vs_set_border_bottom(S, THEME_BORDER);
    vs_set_border_color(S, THEME_TEXT_COLOR);
  }

  vss_extend(S, CLS_EVG_AXIS_KEY, CLS_TEXT)
  {
    vs_set_width(S, V_GROW());
  }

  vss_extend(S, CLS_EVG_AXIS_VALUE, CLS_TEXT) {}

  vss_with(S, CLS_DPAD)
  {
    vs_set_border_color(S, THEME_TEXT_COLOR);
    vs_set_border_radius(S, 1000);
    vs_set_border_all(S, THEME_EVENT_BUTTON_BORDER);

    vs_set_direction(S, V_DIRECTION_COLUMN);
    vs_set_width(S, V_FIT());
    vs_set_padding_all(S, THEME_SP_2XS);
  }

  vss_extend(S, CLS_DPAD_DOWN, CLS_DPAD)
  {
    vs_set_border_color(S, THEME_APP_TITLE_COLOR);
  }

  vss_with(S, CLS_DPAD_ROW)
  {
    vs_set_width(S, V_GROW());
    vs_set_xalign(S, V_ALIGN_X_CENTER);
  }

  vss_extend(S, CLS_DPAD_ICON, CLS_ICON)
  {
    vs_set_font_size(S, 24);
  }

  vss_extend(S, CLS_DPAD_ICON_DOWN, CLS_DPAD_ICON)
  {
    vs_set_color(S, THEME_APP_TITLE_COLOR);
  }

  vss_with(S, CLS_DPAD_BLOCK)
  {
    VStyle* dpad_icon_style = vss_get_class(CLS_DPAD_ICON);
    const float block_size =
        SDLI_MAX(v_style_measure_text_w(dpad_icon_style, ICON_UP),
                 v_style_measure_text_w(dpad_icon_style, ICON_RIGHT));

    vs_set_width(S, V_FIXED(block_size * 1.5f));
    vs_set_height(S, V_FIXED(block_size * 1.5f));
    vs_set_xalign(S, V_ALIGN_X_CENTER);
    vs_set_yalign(S, V_ALIGN_Y_CENTER);
  }
}
