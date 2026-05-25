#include "screen.h"

#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

#define NID_SCREEN_NAVIGATOR "screen-nav"

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
      .id = NID_SCREEN_NAVIGATOR,
      .sclass = CLS_FILL,
  });

  return Navigator_Init(screen_navigator, &CreateScreen);
}

void ScreenNavigator_Goto(const char* id)
{
  VNode* screen_navigator = v_get_node_by_id(NID_SCREEN_NAVIGATOR);
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
