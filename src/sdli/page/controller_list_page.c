#include "page.h"

#include <sdli/model/model.h>
#include <sdli/strings.h>
#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

#define NID_CONTROLLER_LIST "clp:clist"

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
    .id = PAGEID_CONTROLLER_LIST,
    .sclass = CLS_PAGE,
    Children(
      Text({.content.text = STR(SID_CONTROLLERS), .sclass = CLS_PAGE_H1}),
      Box({
        .id = NID_CONTROLLER_LIST,
        .sclass= CLS_SCROLLABLE_LIST,
      }),
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
  if (event->type == NAVIGATOR_EVENT_ENTER) {
    VNode* list = v_get_node_by_id(NID_CONTROLLER_LIST);

    v_node_remove_children(list);

    int controller_count = 0;
    ControllerId* controller_ids =
        ControllerListModel_SortControllers(&controller_count);

    for (int i = 0; i < controller_count; ++i) {
      // TODO: create a proper controller list item widget
      ControllerId id = controller_ids[i];
      const char* name = Controller_GetName(id);
      const char* guid = Controller_GetGUID(id);
      VNode* list_item =
          Box({.sclass = CLS_LIST, Children(Text({.sclass = CLS_TEXT}))});

      v_node_set_text_fmt(v_node_first_child(list_item), "%i :: %s :: %s",
                          (int)id, name, guid);

      v_node_append_child(list, list_item);
    }
  }
}
