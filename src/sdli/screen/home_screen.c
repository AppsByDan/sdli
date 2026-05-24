#include <sdli/screen/screen.h>

#include <sdli/model/model.h>
#include <sdli/page/page.h>
#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

//
// private types
//

typedef struct HomeButtonData {
  StringID string_id;
  const char* icon_text;
  const char* page_id;
} HomeButtonData;

//
// constants
//

#define CLS_HOME_LEFT_COLUMN "home-lcol"
#define CLS_HOME_RIGHT_COLUMN "home-rcol"
#define CLS_HOME_BUTTON "hb-box"
#define CLS_HOME_BUTTON_HOVER "hb-box-hover"
#define CLS_HOME_BUTTON_TEXT "hb-text"
#define CLS_HOME_BUTTON_TEXT_HOVER "hb-text-hover"
#define CLS_HOME_BUTTON_ICON_BOX "hb-icon-box"
#define CLS_HOME_BUTTON_ICON "hb-icon"
#define CLS_HOME_BUTTON_ICON_HOVER "hb-icon-hover"

static const HomeButtonData HOME_BUTTONS[] = {
    {SID_CONTROLLERS, ICON_GAMEPAD, PAGEID_CONTROLLER_LIST},
    {SID_SYSTEM, ICON_COG_ALT, PAGEID_SYSTEM},
};

//
// private function declarations
//

static void StyleSheet(void);
static VNode* HomeButton(const HomeButtonData* data);
static void HomeButton_OnClick(VNode* node, VEvent* event);
static void HomeButton_OnMouseOver(VNode* node, VEvent* event);
static void OnNavigatorEvent(NavigatorEvent* event);

//
// public function implementation
//

VNode* HomeScreen(void)
{
  StyleSheet();

  VNode* left_column;
  VNode* right_column;
  // clang-format off
  VNode* home_screen = Box({
    .id = SCREENID_HOME,
    .sclass = CLS_FILL,
    Children(
      left_column = Box({
        .sclass = CLS_HOME_LEFT_COLUMN,
      }),
      right_column = Box({
        .sclass = CLS_HOME_RIGHT_COLUMN,
      })
    )
  });
  // clang-format on

  PageNavigator_Init(right_column);

  for (size_t i = 0; i < c_arraylen(HOME_BUTTONS); ++i) {
    v_node_append_child(left_column, HomeButton(&HOME_BUTTONS[i]));
  }

  return Navigable_Init(home_screen, &OnNavigatorEvent);
}

static void OnNavigatorEvent(NavigatorEvent* event)
{
  if (event->type == NAVIGATOR_EVENT_ENTER) {
    State_ClearController();
    PageNavigator_Goto(PAGEID_CONTROLLER_LIST);
  }
}

static void StyleSheet(void)
{
  float max_width = 0;

  vss_with(S, CLS_HOME_LEFT_COLUMN)
  {
    vs_set_direction(S, V_DIRECTION_COLUMN);
    vs_set_background(S, THEME_BACKGROUND_1);
    vs_set_width(S, V_FIT());
    vs_set_height(S, V_GROW());
  }

  vss_extend(S, CLS_HOME_RIGHT_COLUMN, CLS_FILL)
  {
    vs_set_direction(S, V_DIRECTION_COLUMN);
    vs_set_padding(S, THEME_SP_SM, THEME_SP_SM, THEME_SP_SM, 0);
  }

  vss_with(S, CLS_HOME_BUTTON)
  {
    // TODO: the sizing is not working!
    vs_set_width(S, V_GROW());
    vs_set_padding(S, THEME_SP_SM, THEME_SP_LG, THEME_SP_SM, THEME_SP_LG);
    vs_set_gap(S, THEME_SP_SM);
    vs_set_background(S, THEME_BACKGROUND_1);
  }

  vss_extend(S, CLS_HOME_BUTTON_HOVER, CLS_HOME_BUTTON)
  {
    vs_set_background(S, THEME_TEXT_COLOR);
  }

  vss_extend(S, CLS_HOME_BUTTON_ICON, CLS_ICON)
  {
    for (size_t i = 0; i < c_arraylen(HOME_BUTTONS); ++i) {
      float w = v_style_measure_text_w(S, HOME_BUTTONS[i].icon_text);

      if (w > max_width) {
        max_width = w;
      }
    }
  }

  vss_extend(S, CLS_HOME_BUTTON_ICON_HOVER, CLS_HOME_BUTTON_ICON)
  {
    vs_set_color(S, THEME_BACKGROUND_1);
  }

  vss_with(S, CLS_HOME_BUTTON_ICON_BOX)
  {
    vs_set_width(S, V_FIXED(max_width));
    vs_set_xalign(S, V_ALIGN_X_CENTER);
  }

  vss_extend(S, CLS_HOME_BUTTON_TEXT, CLS_TEXT) {}

  vss_extend(S, CLS_HOME_BUTTON_TEXT_HOVER, CLS_HOME_BUTTON_TEXT)
  {
    vs_set_color(S, THEME_BACKGROUND_1);
  }
}

static VNode* HomeButton(const HomeButtonData* data)
{
  const char* text = GetString(LOCALE_EN_US, data->string_id);
  const char* icon_text = data->icon_text;
  const char* page_id = data->page_id;

  // clang-format off
  return Box({
    .sclass = CLS_HOME_BUTTON,
    .data = (void*)page_id,
    .on_click = &HomeButton_OnClick,
    .on_mouse_enter = &HomeButton_OnMouseOver,
    .on_mouse_leave = &HomeButton_OnMouseOver,
    Children(
      Box({
        .sclass = CLS_HOME_BUTTON_ICON_BOX,
        Children(
          Text({.sclass = CLS_HOME_BUTTON_ICON, .content.text = icon_text})
        )
      }),
      Text({.sclass = CLS_HOME_BUTTON_TEXT, .content.text = text})
    )
  });
  // clang-format on
}

static void HomeButton_OnClick(VNode* node, VEvent* event)
{
  UNUSED(event);
  const char* page_id = v_node_data(node);
  assert(page_id);

  PageNavigator_Goto(page_id);
}

static void HomeButton_OnMouseOver(VNode* node, VEvent* event)
{
  VNode* icon_box = v_node_child_at(node, 0);
  VNode* label = v_node_child_at(node, 1);

  switch (event->type) {
    case V_EVENT_MOUSE_ENTER:
      v_node_style_assign_class(node, CLS_HOME_BUTTON_HOVER);
      v_node_style_assign_class(v_node_first_child(icon_box),
                                CLS_HOME_BUTTON_ICON_HOVER);
      v_node_style_assign_class(label, CLS_HOME_BUTTON_TEXT_HOVER);
      break;
    case V_EVENT_MOUSE_LEAVE:
      v_node_style_assign_class(node, CLS_HOME_BUTTON);
      v_node_style_assign_class(v_node_first_child(icon_box),
                                CLS_HOME_BUTTON_ICON);
      v_node_style_assign_class(label, CLS_HOME_BUTTON_TEXT);
      break;
    default:
      break;
  }
}
