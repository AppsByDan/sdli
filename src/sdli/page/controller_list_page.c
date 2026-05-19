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
static void InfoButtonOnClick(VNode* node, VEvent* event);
static void EventsButtonOnClick(VNode* node, VEvent* event);
static void ConfigureButtonOnClick(VNode* node, VEvent* event);

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
      // clang-format off
      VNode* list_item = Box({
        .sclass = CLS_LIST,
        Children(
          Text({.sclass = CLS_TEXT}),
          Button("Info", (void*)(uintptr_t)id, &InfoButtonOnClick),
          Button("Events", (void*)(uintptr_t)id, &EventsButtonOnClick),
          Button("Configure", (void*)(uintptr_t)id, &ConfigureButtonOnClick),
        )
      });
      // clang-format on
      VNode* text = v_node_first_child(list_item);

      vs_set_direction(v_node_style(list_item), V_DIRECTION_ROW);
      vs_set_gap(v_node_style(list_item), THEME_SP_SM);

      vs_set_width(v_node_style(text), V_GROW());
      v_node_set_text_fmt(text, "%i :: %s :: %s", (int)id, name, guid);

      v_node_append_child(list, list_item);
    }
  }
}

static void InfoButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(node, event);
}

static void EventsButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(node, event);
}

static void ConfigureButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(node, event);
}
