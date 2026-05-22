#include "screen.h"

#include <sdli/model/model.h>
#include <sdli/strings.h>
#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

//
// private types
//

typedef struct HatDisplayInfo {
  const char* icon;
  uint8_t mask;
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
#define CLS_EV_BUTTON_ROW "ev-btn-row"
#define CLS_EVJ_BOX "evj-box"
#define CLS_EVJ_BUTTON "evj-btn"
#define CLS_EVJ_BUTTON_DOWN "evj-btn-down"
#define CLS_EVJ_BUTTON_TEXT "evj-btn-text"
#define CLS_EVJ_BUTTON_TEXT_PRESS "evj-btn-text-press"
#define CLS_EVJ_GROUP "evj-group"
#define CLS_EVJ_HAT "evj-hat"
#define CLS_EVJ_HAT_DOWN "evj-hat-down"
#define CLS_EVJ_HAT_ICON "evj-hat-icon"
#define CLS_EVJ_HAT_ICON_DOWN "evj-hat-icon-down"
#define CLS_EVG_BUTTON "evg-btn"
#define CLS_EVG_BUTTON_PLACEHOLDER "evg-btn-placeholder"
#define CLS_EVG_BUTTON_DOWN "evg-btn-down"
#define CLS_EVG_AXIS_GROUP "evg-axis-group"
#define CLS_EVG_AXIS_BOX "evg-axis-box"
#define CLS_EVG_AXIS_SPACER "evg-axis-spacer"
#define CLS_EVG_AXIS_KV_ITEM_FIRST "evg-axis-kv-first"
#define CLS_EVG_AXIS_KV_ITEM "evg-axis-kv"
#define CLS_EVG_AXIS_KEY "evg-axis-key"
#define CLS_EVG_AXIS_VALUE "evg-axis-val"

static const HatDisplayInfo HAT_DISPLAY_INFO[] = {
    {ICON_UP, POV_HAT_MASK_UP},
    {ICON_DOWN, POV_HAT_MASK_DOWN},
    {ICON_LEFT, POV_HAT_MASK_LEFT},
    {ICON_RIGHT, POV_HAT_MASK_RIGHT},
};

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
static const int BUTTON_GRID_SIZE = (int)c_arraylen(BUTTON_GRID);
// clang-format on

//
// private function declarations
//

static VNode* JoystickEvents(ControllerId controller_id, bool visible);
static VNode* GamepadEvents(ControllerId controller_id, bool visible);
static void OnJoystickControllerInputEvent(ControllerInputEvent* event);
static void OnGamepadControllerInputEvent(ControllerInputEvent* event);
static void OnNavigatorEvent(NavigatorEvent* event);
static void BackButtonOnClick(VNode* node, VEvent* event);
static void ToggleApiButtonOnClick(VNode* node, VEvent* event);
static void SetControllerApi(VNode* toggle, ControllerApi new_api);
static void SetHatIconClass(VNode* hat_icon, uint8_t hat_value, uint8_t mask);
static void UpdateGamepadAxisValue(int sgk_axis, float value);
static void UpdateGamepadButtonValue(int sgk_button, bool pressed);
static void UpdateJoystickAxisValue(int axis_id, float value);
static void UpdateJoystickButtonValue(int button_id, bool pressed);
static void UpdateJoystickHatValue(int hat_id, uint8_t hat_value);
static void StyleSheet(void);

//
// public function implementation
//

VNode* ControllerEventsScreen(void)
{
  StyleSheet();

  void* api_button_data = (void*)(intptr_t)CONTROLLER_API_JOYSTICK;

  // clang-format off
  VNode* screen = Box({
    .id = SCREENID_CONTROLLER_EVENTS,
    .sclass = CLS_EV_SCREEN,
    Children(
      Box({
        .sclass = CLS_EV_HEADER,
        Children(
          Text({.id = NID_CONTROLLER_NAME, .sclass = CLS_EV_CONTROLLER_NAME}),
          ButtonWithId(NID_API_TOGGLE, "Joystick API", api_button_data, &ToggleApiButtonOnClick),
          Button("Back", NULL, &BackButtonOnClick)
        )
      }),
      Box({.id = NID_API_BOX, .sclass = CLS_EV_BODY}),
    )
  });
  // clang-format on

  return Navigable_Init(screen, &OnNavigatorEvent);
}

//
// private function implementation
//

static void OnJoystickControllerInputEvent(ControllerInputEvent* event)
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

static void OnGamepadControllerInputEvent(ControllerInputEvent* event)
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
    const uint32_t controller_id = ControllerListModel_GetSelectedController();

    BindString(NID_CONTROLLER_NAME, Controller_GetName(controller_id));

    VNode* api_box = v_get_node_by_id(NID_API_BOX);

    v_node_remove_children(api_box);
    v_node_append_child(api_box, JoystickEvents(controller_id, false));
    v_node_append_child(api_box, GamepadEvents(controller_id, false));

    SetControllerApi(v_get_node_by_id(NID_API_TOGGLE), CONTROLLER_API_JOYSTICK);
  }
}

static void BackButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(node, event);
  // TODO: this should be a back navigation, not a hard goto
  ScreenNavigator_Goto(SCREENID_HOME);
}

static void ToggleApiButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(event);
  const ControllerApi api = (ControllerApi)(intptr_t)v_node_data(node);
  ControllerApi new_api;

  if (api == CONTROLLER_API_GAMEPAD) {
    new_api = CONTROLLER_API_JOYSTICK;
  } else if (api == CONTROLLER_API_JOYSTICK) {
    new_api = CONTROLLER_API_GAMEPAD;
  } else {
    return;
  }

  SetControllerApi(node, new_api);
}

static void SetControllerApi(VNode* toggle, ControllerApi new_api)
{
  ControllerId controller_id = ControllerListModel_GetSelectedController();
  ControllerInputEventHandler event_handler = NULL;
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
  ControllerListModel_EnableControllerInputEvents(new_api, event_handler);
}

static VNode* JoystickEvents(ControllerId controller_id, bool visible)
{
  const int button_count = Controller_GetButtonCount(controller_id);
  const int hat_count = Controller_GetHatCount(controller_id);
  const int axis_count = Controller_GetAxisCount(controller_id);
  VNode* box = Box({.id = NID_JOYSTICK_API, .sclass = CLS_EVJ_BOX});

  v_node_set_visible(box, visible);

  if (button_count > 0) {
    VNode* group = Box({.sclass = CLS_EVJ_GROUP});

    for (int i = 0; i < button_count; i++) {
      VNode* button_text;
      VNode* button = Box({
          .sclass = CLS_EVJ_BUTTON,
          .data = (void*)(uintptr_t)i,
          v_children(button_text = Text({.sclass = CLS_EVJ_BUTTON_TEXT})),
      });

      v_node_set_id_fmt(button, "jb:%i", i);
      v_node_set_text_fmt(button_text, "b%i", i);

      v_node_append_child(group, button);
    }

    v_node_append_child(box, group);
  }

  if (hat_count > 0) {
    VNode* group = Box({.sclass = CLS_EVJ_GROUP});

    for (int i = 0; i < hat_count; i++) {
      VNode* hat_name;
      VNode* hat = Box({
          .sclass = CLS_EVJ_HAT,
          .data = (void*)(uintptr_t)i,
          Children(hat_name = Text({.sclass = CLS_EVJ_BUTTON_TEXT})),
      });

      v_node_set_text_fmt(hat_name, "h%i", i);
      v_node_set_id_fmt(hat, "jhat:%i", i);

      for (size_t h = 0; h < c_arraylen(HAT_DISPLAY_INFO); h++) {
        VNode* hat_icon = Text({
            .sclass = CLS_EVJ_HAT_ICON,
            .content.text = HAT_DISPLAY_INFO[h].icon,
        });
        v_node_append_child(hat, hat_icon);
      }

      v_node_append_child(group, hat);
    }

    v_node_append_child(box, group);
  }

  if (axis_count > 0) {
    VNode* group = Box({.sclass = CLS_EVJ_GROUP});

    for (int i = 0; i < axis_count; i++) {
      VNode* axis_name;
      VNode* axis_value;
      // clang-format off
      VNode* axis = Box({
          .data = (void*)(uintptr_t)i,
          .sclass = CLS_EVJ_HAT,
          Children(
            axis_name = Text({.sclass = CLS_EVJ_BUTTON_TEXT}),
            axis_value = Text({.sclass = CLS_EVJ_BUTTON_TEXT})
          ),
      });
      // clang-format on

      v_node_set_text_fmt(axis_name, "a%i", i);
      v_node_set_id_fmt(axis_value, "jaxis:%i", i);

      v_node_append_child(group, axis);
    }

    v_node_append_child(box, group);
  }

  return box;
}

static VNode* GamepadEvents(ControllerId controller_id, bool visible)
{
  UNUSED(controller_id);
  VNode* box = Box({.id = NID_GAMEPAD_API});

  v_node_set_visible(box, visible);

  VStyle* box_style = v_node_style(box);

  vs_set_direction(box_style, V_DIRECTION_COLUMN);
  vs_set_gap(box_style, THEME_SP_SM);

  const int cols = BUTTON_GRID_ROW;
  const int rows = (BUTTON_GRID_SIZE + cols - 1) / cols;

  for (int r = 0; r < rows; r++) {
    VNode* row = Box({.sclass = CLS_EV_BUTTON_ROW});

    for (int c = 0; c < cols; c++) {
      const int idx = r * cols + c;
      VNode* button = Box({0});

      if (idx < BUTTON_GRID_SIZE && BUTTON_GRID[idx] != SGK_INVALID) {
        VNode* button_text = Text({
            .sclass = CLS_EVJ_BUTTON_TEXT,
            .content.text = StandardGamepadKey_ToString(BUTTON_GRID[idx]),
        });

        v_node_set_id_fmt(button, "gb:%i", (int)BUTTON_GRID[idx]);
        v_node_style_assign_class(button, CLS_EVG_BUTTON);
        v_node_append_child(button, button_text);
      } else {
        v_node_style_assign_class(button, CLS_EVG_BUTTON_PLACEHOLDER);
      }

      v_node_append_child(row, button);
    }

    v_node_append_child(box, row);
  }

  // clang-format off
  VNode* gamepad_axis_group = Box({
    .sclass = CLS_EVG_AXIS_GROUP,
    Children(
      Box({
        .sclass = CLS_EVG_AXIS_BOX,
        Children(
          Box({
            .sclass = CLS_EVG_AXIS_KV_ITEM_FIRST,
            Children(
              Text({
                .sclass = CLS_EVG_AXIS_KEY,
                .content.text = "Left Trigger",
              }),
              Text({
                .id = NID_GAMEPAD_LEFT_TRIGGER_VALUE,
                .sclass = CLS_EVG_AXIS_VALUE,
              })
            )
          }),
          Box({
            .sclass = CLS_EVG_AXIS_KV_ITEM,
            Children(
              Text({.sclass = CLS_EVG_AXIS_KEY, .content.text = "Left Joystick"}),
              Text({.id = NID_GAMEPAD_LEFT_X_VALUE, .sclass = CLS_EVG_AXIS_VALUE}),
              Text({.id = NID_GAMEPAD_LEFT_Y_VALUE, .sclass = CLS_EVG_AXIS_VALUE})
            )
          })
        )
      }),
      Box({
        .sclass = CLS_EVG_AXIS_SPACER,
      }),
      Box({
        .sclass = CLS_EVG_AXIS_BOX,
        Children(
          Box({
            .sclass = CLS_EVG_AXIS_KV_ITEM_FIRST,
            Children(
              Text({
                .sclass = CLS_EVG_AXIS_KEY,
                .content.text = "Right Trigger",
              }),
              Text({
                .id = NID_GAMEPAD_RIGHT_TRIGGER_VALUE,
                .sclass = CLS_EVG_AXIS_VALUE,
              })
            )
          }),
          Box({
            .sclass = CLS_EVG_AXIS_KV_ITEM,
            Children(
              Text({.sclass = CLS_EVG_AXIS_KEY, .content.text = "Right Joystick"}),
              Text({.id = NID_GAMEPAD_RIGHT_X_VALUE, .sclass = CLS_EVG_AXIS_VALUE}),
              Text({.id = NID_GAMEPAD_RIGHT_Y_VALUE, .sclass = CLS_EVG_AXIS_VALUE})
            )
          })
        )
      }),
    ),
  });
  // clang-format on

  v_node_append_child(box, gamepad_axis_group);

  return box;
}

static void SetHatIconClass(VNode* hat_icon, uint8_t hat_value, uint8_t mask)
{
  v_node_style_assign_class(
      hat_icon, (hat_value & mask) ? CLS_EVJ_HAT_ICON_DOWN : CLS_EVJ_HAT_ICON);
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
  VNode* node = v_get_node_by_id_fmt("gb:%i", button_id);

  v_node_style_assign_class(node,
                            pressed ? CLS_EVG_BUTTON_DOWN : CLS_EVG_BUTTON);
  v_node_style_assign_class(
      v_node_first_child(node),
      pressed ? CLS_EVJ_BUTTON_TEXT_PRESS : CLS_EVJ_BUTTON_TEXT);
}

static void UpdateJoystickAxisValue(int axis_id, float value)
{
  v_node_set_text_fmt(v_get_node_by_id_fmt("jaxis:%i", axis_id), "%0.3f",
                      value);
}

static void UpdateJoystickHatValue(int hat_id, uint8_t hat_value)
{
  VNode* hat = v_get_node_by_id_fmt("jhat:%i", hat_id);

  for (size_t i = 0; i < c_arraylen(HAT_DISPLAY_INFO); i++) {
    SetHatIconClass(v_node_child_at(hat, i + 1), hat_value,
                    HAT_DISPLAY_INFO[i].mask);
  }
}

static void UpdateJoystickButtonValue(int button_id, bool pressed)
{
  VNode* node = v_get_node_by_id_fmt("jb:%i", button_id);

  v_node_style_assign_class(node,
                            pressed ? CLS_EVJ_BUTTON_DOWN : CLS_EVJ_BUTTON);
  v_node_style_assign_class(
      v_node_first_child(node),
      pressed ? CLS_EVJ_BUTTON_TEXT_PRESS : CLS_EVJ_BUTTON_TEXT);
}

static void StyleSheet(void)
{
  float max_width = 0;
  VStyle* text_style = vss_get_class(CLS_TEXT);

  for (int i = 0; i < STANDARD_GAMEPAD_BUTTON_COUNT; i++) {
    const float width = v_style_measure_text_w(
        text_style, StandardGamepadKey_ToString((StandardGamepadKey)i));

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
    vs_set_border_radius(S, 24);
    vs_set_border(S, 2, 2, 2, 2);
    vs_set_width(S, V_FIXED(48));
    vs_set_height(S, V_FIXED(48));
    vs_set_xalign(S, V_ALIGN_X_CENTER);
    vs_set_yalign(S, V_ALIGN_Y_CENTER);
  }

  vss_with(S, CLS_EV_BUTTON_ROW)
  {
    vs_set_gap(S, THEME_SP_2XS);
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

  vss_with(S, CLS_EVJ_GROUP)
  {
    vs_set_width(S, V_GROW());
    vs_set_xalign(S, V_ALIGN_X_CENTER);
    vs_set_direction(S, V_DIRECTION_ROW);
    vs_set_gap(S, THEME_SP_2XS);
  }

  vss_with(S, CLS_EVJ_HAT)
  {
    vs_set_border_color(S, THEME_TEXT_COLOR);
    vs_set_border_radius(S, 24);
    vs_set_border(S, 2, 2, 2, 2);
    vs_set_xalign(S, V_ALIGN_X_CENTER);
    vs_set_yalign(S, V_ALIGN_Y_CENTER);
    vs_set_talign(S, V_ALIGN_X_CENTER);
    vs_set_height(S, V_FIXED(48));
    vs_set_gap(S, THEME_SP_XS);
    vs_set_padding(S, 0, THEME_SP_MD, 0, THEME_SP_MD);
  }

  vss_extend(S, CLS_EVJ_HAT_DOWN, CLS_EVJ_HAT)
  {
    vs_set_border_color(S, THEME_APP_TITLE_COLOR);
  }

  vss_extend(S, CLS_EVJ_HAT_ICON, CLS_ICON) {}

  vss_extend(S, CLS_EVJ_HAT_ICON_DOWN, CLS_EVJ_HAT_ICON)
  {
    vs_set_color(S, THEME_APP_TITLE_COLOR);
  }

  vss_with(S, CLS_EVG_BUTTON)
  {
    vs_set_border_color(S, THEME_TEXT_COLOR);
    vs_set_border_radius(S, 24);
    vs_set_border(S, 2, 2, 2, 2);
    vs_set_width(S, V_FIXED((uint64_t)max_width + (20 * 2)));
    vs_set_height(S, V_FIXED(48));
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

  vss_with(S, CLS_EVG_AXIS_GROUP)
  {
    vs_set_padding(S, THEME_SP_MD, 0, 0, 0);
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
    vs_set_padding(S, THEME_SP_SM, 0, 0, 0);
    vs_set_direction(S, V_DIRECTION_ROW);
    vs_set_width(S, V_GROW());
  }

  vss_extend(S, CLS_EVG_AXIS_KV_ITEM_FIRST, CLS_EVG_AXIS_KV_ITEM)
  {
    vs_set_padding(S, 0, 0, THEME_SP_SM, 0);
    vs_set_border(S, 0, 0, 1, 0);
    vs_set_border_color(S, THEME_TEXT_COLOR);
  }

  vss_extend(S, CLS_EVG_AXIS_KEY, CLS_TEXT)
  {
    vs_set_width(S, V_GROW());
  }

  vss_extend(S, CLS_EVG_AXIS_VALUE, CLS_TEXT) {}
}
