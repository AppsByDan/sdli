#include "screen.h"

#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

//
// external declarations
//

extern VNode* HomeScreen(void);
extern VNode* ControllerEventsScreen(void);
extern VNode* ControllerConfigScreen(void);

//
// private function declarations
//

static VNode* CreateScreen(const char* id);

//
// public function implementation
//

VNode* ScreenNavigator(void)
{
  VNode* screen_navigator = Box({
      .id = "screen_navigator",
      .sclass = CLS_FILL,
  });

  v_node_append_child(v_root(), screen_navigator);

  return Navigator_Init(screen_navigator, &CreateScreen);
}

void ScreenNavigator_Goto(const char* id)
{
  VNode* screen_navigator = v_get_node_by_id("screen_navigator");
  assert(screen_navigator);

  Navigator_Goto(screen_navigator, id);
}

//
// private function implementation
//

static VNode* CreateScreen(const char* id)
{
  if (StringEq(id, SCREENID_HOME)) {
    return HomeScreen();
  } else if (StringEq(id, SCREENID_CONTROLLER_EVENTS)) {
    return ControllerEventsScreen();
  } else if (StringEq(id, SCREENID_CONTROLLER_CONFIG)) {
    return ControllerConfigScreen();
  }

  return NULL;
}
