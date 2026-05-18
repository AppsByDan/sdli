#include <sdli/page/page.h>

#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

static void OnNavigatorEvent(NavigatorEvent* event);

VNode* SystemPage(void)
{
  // clang-format off
  VNode* page = Box({
    .sclass = CLS_CENTER_XY,
    Children(
      Text({
        .sclass = CLS_TEXT,
        .content.text = "System"
      })
    )
  });
  // clang-format on

  return Navigable_Init(page, &OnNavigatorEvent);
}

static void OnNavigatorEvent(NavigatorEvent* event)
{
  UNUSED(event);
}
