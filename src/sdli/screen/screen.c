#include "screen.h"

#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

//
// external declarations
//

extern VNode* HomeScreen(void);

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

//
// private function implementation
//

static VNode* CreateScreen(const char* id)
{
  if (StringEq(id, SCREENID_HOME)) {
    return HomeScreen();
  }

  return NULL;
}
