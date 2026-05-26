#include "screen.h"

#include <sdli/model/model.h>
#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

#include <stc/cstr.h>

//
// macros & constants
//

#define NID_PLAN "ccfg:plan"
#define NID_CONTROLLER_NAME "ccfg:ctrl-name"

#define CLS_PLAN "plan"
#define CLS_PLAN_H1 "plan-h1"
#define CLS_PLAN_TEXT "plan-text"
#define CLS_PLAN_TEXT_HIGHLIGHT "plan-text-hl"
#define CLS_CONFIG_GROUP "config-group"
#define CLS_CONFIG_CURSOR "config-cursor"
#define CLS_CONFIG_CURSOR_NAV "cursor-nav"
#define CLS_SCREEN "ccfg-screen"
#define CLS_SCREEN_MENU "ccfg-screen-menu"
#define CLS_SCREEN_MENU_NAME "ccfg-screen-menu-name"

#define CCBindingArray_Init(...)                                      \
  {                                                                   \
      .items = {__VA_ARGS__},                                         \
      .size = sizeof((CCBinding[]){__VA_ARGS__}) / sizeof(CCBinding), \
  }

//
// private types
//

typedef struct CCBinding {
  StandardGamepadKey key;
  cstr value;
} CCBinding;

typedef struct CCBindingArray {
  CCBinding items[8];
  size_t size;
} CCBindingArray;

typedef struct CCBindingGroup {
  const char* name;
  CCBindingArray bindings;
} CCBindingGroup;

static void CCBindingGroup_drop(CCBindingGroup* group);

#define i_no_clone
#define i_no_emplace
#define i_keyclass CCBindingGroup
#define i_type vec_binding_group
#include <stc/vec.h>

typedef struct CCScreenState {
  size_t group_index;
  size_t binding_index;
  vec_binding_group groups;
} CCScreenState;

//
// private function declarations
//

static void UpdateConfigGroup(size_t selected_index);
static void UpdateSelectedConfig(size_t group_index,
                                 size_t old_config_index,
                                 size_t new_config_index);
static void OnNavigatorEvent(NavigatorEvent* event);
static void OnControllerInputEvent(const ControllerInputEvent* event);
static void BackButtonOnClick(VNode* node, VEvent* event);
static void FinishButtonOnClick(VNode* node, VEvent* event);
static void NextButtonOnClick(VNode* node, VEvent* event);
static void NextGroupButtonOnClick(VNode* node, VEvent* event);
static void PreviousButtonOnClick(VNode* node, VEvent* event);
static void PreviousGroupButtonOnClick(VNode* node, VEvent* event);
static CCScreenState* GetScreenState(void);
static void ControllerConfigScreenState_Drop(void* state);
static void StandardGamepadConfig(CCScreenState* state);
static void StyleSheet(void);

//
// public function implementation
//

VNode* ControllerConfigScreen(void)
{
  StyleSheet();

  // clang-format off
  VNode* screen =  Box({
    .id = SCREENID_CONTROLLER_CONFIG,
    .sclass = CLS_SCREEN,
    Children(
      Box({
        .sclass = CLS_SCREEN_MENU,
        Children(
          Text({.id = NID_CONTROLLER_NAME, .sclass = CLS_SCREEN_MENU_NAME}),
          Button("Back", NULL, &BackButtonOnClick)
        )
      }),
      Box({
        .sclass = CLS_FILL,
        Children(
          Box({
            .id = NID_PLAN,
            .sclass = CLS_CENTER_XY,
          }),
          Box({
            .sclass = CLS_CENTER_XY,
            Children(
              Box({
                .sclass = CLS_CONFIG_CURSOR,
                Children(
                  Box({
                    .sclass = CLS_CONFIG_CURSOR_NAV,
                    Children(
                      Button("<<", NULL, &PreviousGroupButtonOnClick),
                      Button("<", NULL, &PreviousButtonOnClick),
                      Button(">", NULL, &NextButtonOnClick),
                      Button(">>", NULL, &NextGroupButtonOnClick),
                    )
                  }),
                  Button("Finish", NULL, &FinishButtonOnClick)
                )
              })
            )
          })
        )
      })
    )
  });
  // clang-format on

  return Navigable_Init(screen, &OnNavigatorEvent);
}

//
// private function implementation
//

static void UpdateConfigGroup(size_t selected_index)
{
  CCScreenState* state = GetScreenState();
  CCBindingGroup* group = &state->groups.data[state->group_index];
  VNode* group_box = Box(
      {.sclass = CLS_CONFIG_GROUP,
       Children(Text({.content.text = group->name, .sclass = CLS_PLAN_H1}))});

  for (size_t i = 0; i < group->bindings.size; i++) {
    CCBinding* config = &group->bindings.items[i];
    VNode* text = Text({
        .content.text = StandardGamepadKey_ToString(config->key),
        .sclass = selected_index == i ? CLS_PLAN_TEXT_HIGHLIGHT : CLS_PLAN_TEXT,
    });
    v_node_set_id_fmt(text, "ccfg:sgk%i-%i", (int)state->group_index,
                      (int)config->key);
    v_node_append_child(group_box, text);
  }

  VNode* plan = v_get_node_by_id(NID_PLAN);

  v_node_remove_children(plan);
  v_node_append_child(plan, group_box);
}

static void UpdateSelectedConfig(size_t group_index,
                                 size_t old_config_index,
                                 size_t new_config_index)
{
  CCScreenState* state = GetScreenState();
  CCBindingArray* bindings = &state->groups.data[group_index].bindings;
  VNode* old_binding =
      v_get_node_by_id_fmt("ccfg:sgk%i-%i", (int)group_index,
                           (int)bindings->items[old_config_index].key);
  VNode* new_binding =
      v_get_node_by_id_fmt("ccfg:sgk%i-%i", (int)group_index,
                           (int)bindings->items[new_config_index].key);

  v_node_style_assign_class(old_binding, CLS_PLAN_TEXT);
  v_node_style_assign_class(new_binding, CLS_PLAN_TEXT_HIGHLIGHT);
}

static void OnNavigatorEvent(NavigatorEvent* event)
{
  if (event->type == NAVIGATOR_EVENT_ENTER) {
    CCScreenState* state = GetScreenState();
    const ControllerId controller_id = State_GetSelectedController();

    // TODO: this needs to be cleaned up..

    if (state == NULL) {
      state = calloc(1, sizeof(CCScreenState));
      State_SetData(SCREENID_CONTROLLER_CONFIG, state,
                    &ControllerConfigScreenState_Drop);
    } else {
      state->group_index = 0;
      state->binding_index = 0;
    }

    StandardGamepadConfig(state);
    UpdateConfigGroup(state->binding_index);
    ControllerInputModel_EnableMapper(controller_id, &OnControllerInputEvent);
    BindString(NID_CONTROLLER_NAME, Controller_GetName(controller_id));
  } else if (event->type == NAVIGATOR_EVENT_LEAVE) {
    ControllerInputModel_Disable();
  }
}

void OnControllerInputEvent(const ControllerInputEvent* event)
{
  if (event->type != CONTROLLER_INPUT_BINDING) {
    return;
  }

  CCScreenState* state = GetScreenState();

  state->groups.data[state->group_index]
      .bindings.items[state->binding_index]
      .value = cstr_from(event->u.binding.value);

  // move to next config, use onclick event..
  NextButtonOnClick(NULL, NULL);
}

static void BackButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(node, event);
  // TODO: this should be a back navigation, not a hard goto
  ScreenNavigator_Goto(SCREENID_HOME);
}

static void DismissButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(node, event);
  Overlay_Dismiss();
}

static void FinishButtonOnClick(VNode* node, VEvent* event)
{
  // TODO: save config
  BackButtonOnClick(node, event);
}

static void PreviousButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(node, event);
  CCScreenState* state = GetScreenState();

  if (state->binding_index == 0) {
    if (state->group_index > 0) {
      state->group_index--;
      state->binding_index =
          state->groups.data[state->group_index].bindings.size - 1;
      UpdateConfigGroup(state->binding_index);
    }
  } else {
    const size_t old_index = state->binding_index;

    state->binding_index--;
    UpdateSelectedConfig(state->group_index, old_index, state->binding_index);
  }
}

static void PreviousGroupButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(node, event);
  CCScreenState* state = GetScreenState();

  if (state->group_index > 0) {
    state->group_index--;
    state->binding_index =
        state->groups.data[state->group_index].bindings.size - 1;
    UpdateConfigGroup(state->binding_index);
  }
}

static void NextButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(node, event);
  CCScreenState* state = GetScreenState();
  CCBindingGroup* group = &state->groups.data[state->group_index];

  if (state->binding_index < group->bindings.size - 1) {
    const size_t old_index = state->binding_index;

    state->binding_index++;
    UpdateSelectedConfig(state->group_index, old_index, state->binding_index);
  } else {
    if (state->group_index < state->groups.size - 1) {
      state->group_index++;
      state->binding_index = 0;
      UpdateConfigGroup(state->binding_index);
    }
  }
}

static void NextGroupButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(node, event);
  CCScreenState* state = GetScreenState();

  if (state->group_index < state->groups.size - 1) {
    state->group_index++;
    state->binding_index = 0;
    UpdateConfigGroup(state->binding_index);
  }
}

static CCScreenState* GetScreenState(void)
{
  return State_GetData(SCREENID_CONTROLLER_CONFIG);
}

static void ControllerConfigScreenState_Drop(void* state)
{
  CCScreenState* cc_state = state;

  vec_binding_group_drop(&cc_state->groups);
  free(state);
}

static void CCBindingGroup_drop(CCBindingGroup* group)
{
  cstr_drop(&group->name);
}

static void StandardGamepadConfig(CCScreenState* state)
{
  vec_binding_group_clear(&state->groups);

  // clang-format off
  vec_binding_group_push_back(&state->groups, (CCBindingGroup){
    .name = "Face Buttons",
    .bindings = CCBindingArray_Init(
      {SGK_BUTTON_SOUTH},
      {SGK_BUTTON_EAST},
      {SGK_BUTTON_WEST},
      {SGK_BUTTON_NORTH}
    ),
  });

  vec_binding_group_push_back(&state->groups, (CCBindingGroup){
    .name = "Directional Pad",
    .bindings = CCBindingArray_Init(
      {SGK_BUTTON_DPAD_UP},
      {SGK_BUTTON_DPAD_DOWN},
      {SGK_BUTTON_DPAD_LEFT},
      {SGK_BUTTON_DPAD_RIGHT}
    ),
  });

  vec_binding_group_push_back(&state->groups, (CCBindingGroup){
    .name = "Secondary Buttons",
    .bindings = CCBindingArray_Init(
      {SGK_BUTTON_BACK},
      {SGK_BUTTON_GUIDE},
      {SGK_BUTTON_START},
      {SGK_BUTTON_TOUCHPAD},
      {SGK_BUTTON_LEFT_STICK},
      {SGK_BUTTON_RIGHT_STICK}
    ),
  });

  vec_binding_group_push_back(&state->groups, (CCBindingGroup)
  {
    .name = "Shoulder Buttons",
    .bindings = CCBindingArray_Init(
      {SGK_BUTTON_LEFT_SHOULDER},
      {SGK_BUTTON_LEFT_PADDLE1},
      {SGK_BUTTON_LEFT_PADDLE2},
      {SGK_BUTTON_RIGHT_SHOULDER},
      {SGK_BUTTON_RIGHT_PADDLE1},
      {SGK_BUTTON_RIGHT_PADDLE2}
    ),
  });

  vec_binding_group_push_back(&state->groups, (CCBindingGroup)
  {
    .name = "Miscellaneous Buttons",
    .bindings = CCBindingArray_Init(
      {SGK_BUTTON_MISC1},
      {SGK_BUTTON_MISC2},
      {SGK_BUTTON_MISC3},
      {SGK_BUTTON_MISC4},
      {SGK_BUTTON_MISC5},
      {SGK_BUTTON_MISC6}
    ),
  });

  vec_binding_group_push_back(&state->groups, (CCBindingGroup)
  {
    .name = "(Axis) Joysticks",
    .bindings = CCBindingArray_Init(
      {SGK_AXIS_LEFTX},
      {SGK_AXIS_LEFTY},
      {SGK_AXIS_RIGHTX},
      {SGK_AXIS_RIGHTY}
    ),
  });

  vec_binding_group_push_back(&state->groups, (CCBindingGroup)
  {
    .name = "(Axis) Triggers",
    .bindings = CCBindingArray_Init(
      {SGK_AXIS_LEFT_TRIGGER},
      {SGK_AXIS_RIGHT_TRIGGER}
    ),
  });
  // clang-format on
}

static void StyleSheet(void)
{
  vss_with(S, CLS_PLAN)
  {
    vs_set_direction(S, V_DIRECTION_ROW);
    vs_set_wrap(S, V_WRAP_WRAP);
  }

  vss_with(S, CLS_CONFIG_GROUP)
  {
    vs_set_direction(S, V_DIRECTION_COLUMN);
    vs_set_border(S, 0, THEME_BORDER, THEME_BORDER, THEME_BORDER);
    vs_set_border_color(S, THEME_BACKGROUND_1);
  }

  vss_extend(S, CLS_PLAN_H1, CLS_TEXT)
  {
    vs_set_font(S, FONT_BOLD);
    vs_set_font_size(S, THEME_TEXT_FONT_SIZE_LG);
    vs_set_background(S, THEME_BACKGROUND_1);
    vs_set_width(S, V_GROW());
    vs_set_padding_all(S, THEME_SP_2XS);
  }

  vss_extend(S, CLS_PLAN_TEXT, CLS_TEXT) {}

  vss_extend(S, CLS_PLAN_TEXT_HIGHLIGHT, CLS_PLAN_TEXT)
  {
    vs_set_color(S, THEME_APP_TITLE_COLOR);
  }

  vss_with(S, CLS_CONFIG_CURSOR)
  {
    vs_set_direction(S, V_DIRECTION_COLUMN);
    vs_set_border_all(S, THEME_BORDER);
    vs_set_border_color(S, THEME_BACKGROUND_1);
  }

  vss_with(S, CLS_CONFIG_CURSOR_NAV)
  {
    vs_set_direction(S, V_DIRECTION_ROW);
    vs_set_gap(S, THEME_SP_SM);
    vs_set_padding_all(S, THEME_SP_2XS);
  }

  vss_with(S, CLS_SCREEN_MENU)
  {
    vs_set_direction(S, V_DIRECTION_ROW);
    vs_set_gap(S, THEME_SP_SM);
    vs_set_width(S, V_GROW());
  }

  vss_extend(S, CLS_SCREEN_MENU_NAME, CLS_TEXT)
  {
    vs_set_width(S, V_GROW());
  }

  vss_extend(S, CLS_SCREEN, CLS_FILL)
  {
    vs_set_direction(S, V_DIRECTION_COLUMN);
  }
}
