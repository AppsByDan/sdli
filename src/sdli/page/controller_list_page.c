#include <sdli/page/page.h>

#include <sdli/strings.h>
#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

//
// private function declarations
//

static void OnNavigatorEvent(NavigatorEvent* event);

//
// public function implementation
//

VNode* ControllerListPage(void)
{
  // clang-format off
  VNode* page = Box({
    .sclass = CLS_CENTER_XY,
    Children(
      Text({
        .sclass = CLS_TEXT,
        .content.text = STR(SID_CONTROLLERS)
      })
    )
  });
  // clang-format on

  return Navigable_Init(page, &OnNavigatorEvent);
}

//
// private function implementation
//

static void OnNavigatorEvent(NavigatorEvent* event)
{
  UNUSED(event);
}
