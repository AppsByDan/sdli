#include "screen.h"

#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

//
// private function declarations
//

static void OnNavigatorEvent(NavigatorEvent* event);
static void BackButtonOnClick(VNode* node, VEvent* event);

//
// public function implementation
//

VNode* ControllerEventsScreen(void)
{
  // clang-format off
  VNode* screen = Box({
    .id = SCREENID_CONTROLLER_EVENTS,
    .sclass = CLS_CENTER_XY,
    Children(
      Button("Back", NULL, &BackButtonOnClick),
    ),
  });
  // clang-format on

  return Navigable_Init(screen, &OnNavigatorEvent);
}

//
// private function implementation
//

static void OnNavigatorEvent(NavigatorEvent* event)
{
  UNUSED(event);
}

static void BackButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(node, event);
  // TODO: this should be a back navigation, not a hard goto
  ScreenNavigator_Goto(SCREENID_HOME);
}
