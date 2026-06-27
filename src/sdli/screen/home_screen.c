#include <sdli/screen/screen.h>

#include <sdli/node_notation.h>
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
static const size_t HOME_BUTTON_SIZE = u_arraylen(HOME_BUTTONS);

//
// private function declarations
//

static void StyleSheet(void);
static void HomeButton(NN_CALLABLE, const HomeButtonData* data);
static void OnNavigatorEvent(NavigatorEvent* event);

//
// private node event handlers
//

// clang-format off
OnClickInline(HomeButton, {
  PageNavigator_Goto((const char*)v_node_data(node));
})

OnMouseOverInline(HomeButton, {
  const char* button_class;
  const char* icon_class;
  const char* text_class;

  if (event->type == V_NODE_EVENT_MOUSE_ENTER) {
    button_class = CLS_HOME_BUTTON_HOVER;
    icon_class = CLS_HOME_BUTTON_ICON_HOVER;
    text_class = CLS_HOME_BUTTON_TEXT_HOVER;
  } else if (event->type == V_NODE_EVENT_MOUSE_LEAVE) {
    button_class = CLS_HOME_BUTTON;
    icon_class = CLS_HOME_BUTTON_ICON;
    text_class = CLS_HOME_BUTTON_TEXT;
  } else {
    return;
  }

  VNode* icon_box = v_node_child_at(node, 0);
  VNode* label = v_node_child_at(node, 1);

  v_node_style_assign_class(node, button_class);
  v_node_style_assign_class(v_node_first_child(icon_box),
                            icon_class);
  v_node_style_assign_class(label, text_class);
})
// clang-format on

//
// public function implementation
//

VNode* HomeScreen(void)
{
  StyleSheet();

  NN_BUILD_NEW(home_screen)
  {
    NN_BOX({.id = SCREENID_HOME, .sclass = CLS_FILL})
    {
      NN_BOX({.sclass = CLS_HOME_LEFT_COLUMN})
      {
        for (size_t i = 0; i < HOME_BUTTON_SIZE; ++i) {
          NN_CALL(HomeButton, &HOME_BUTTONS[i]);
        }
      }
      NN_BOX({.sclass = CLS_HOME_RIGHT_COLUMN})
      {
        PageNavigator_Init(NN_SELF());
      }
    }
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

static void HomeButton(NN_CALLABLE, const HomeButtonData* data)
{
  const char* text = GetString(LOCALE_EN_US, data->string_id);
  const char* icon_text = data->icon_text;
  const char* page_id = data->page_id;

  NN_BOX({
      .sclass = CLS_HOME_BUTTON,
      .data = (void*)page_id,
      .on_click = &HomeButton_OnClick,
      .on_mouse_enter = &HomeButton_OnMouseOver,
      .on_mouse_leave = &HomeButton_OnMouseOver,
  })
  {
    NN_BOX({.sclass = CLS_HOME_BUTTON_ICON_BOX})
    {
      NN_TEXT({.sclass = CLS_HOME_BUTTON_ICON, .text = icon_text});
    }
    NN_TEXT({.sclass = CLS_HOME_BUTTON_TEXT, .text = text});
  }
}

static void StyleSheet(void)
{
  float max_width = 0;

  vss_with(S, CLS_HOME_LEFT_COLUMN)
  {
    vs_set_direction(S, V_DIRECTION_COLUMN);
    vs_set_background(S, THEME_BACKGROUND_1);
    vs_set_width(S, v_fit());
    vs_set_height(S, v_grow());
  }

  vss_extend(S, CLS_HOME_RIGHT_COLUMN, CLS_FILL)
  {
    vs_set_direction(S, V_DIRECTION_COLUMN);
    vs_set_padding(S, THEME_SP_SM, THEME_SP_SM, THEME_SP_SM, v_px(0));
  }

  vss_with(S, CLS_HOME_BUTTON)
  {
    // TODO: the sizing is not working!
    vs_set_width(S, v_grow());
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
    for (size_t i = 0; i < HOME_BUTTON_SIZE; ++i) {
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
    vs_set_width(S, v_px(max_width));
    vs_set_xalign(S, V_ALIGN_X_CENTER);
  }

  vss_extend(S, CLS_HOME_BUTTON_TEXT, CLS_TEXT) {}

  vss_extend(S, CLS_HOME_BUTTON_TEXT_HOVER, CLS_HOME_BUTTON_TEXT)
  {
    vs_set_color(S, THEME_BACKGROUND_1);
  }
}
