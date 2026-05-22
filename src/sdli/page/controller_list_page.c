#include "page.h"

#include <sdli/model/model.h>
#include <sdli/screen/screen.h>
#include <sdli/strings.h>
#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

#define NID_CONTROLLER_LIST "clp:clist"
#define CONTROLLER_ID_FMT "ctrl:%" PRIu32

//
// private function declarations
//

static void AddControllerListItem(VNode* parent, ControllerId id);
static void OnNavigatorEvent(NavigatorEvent* event);
static void OnControllerChangeEvent(const ControllerChangeEvent* event);
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

static void AddControllerListItem(VNode* parent, ControllerId id)
{
  // TODO: create a proper controller list item widget

  // clang-format off
  VNode* list_item = Box({
    .sclass = CLS_LIST,
    Children(
      Text({.sclass = CLS_TEXT}),
      // TODO: localize
      Button("Info", (void*)(uintptr_t)id, &InfoButtonOnClick),
      Button("Events", (void*)(uintptr_t)id, &EventsButtonOnClick),
      Button("Configure", (void*)(uintptr_t)id, &ConfigureButtonOnClick),
    )
  });
  // clang-format on

  v_node_set_id_fmt(list_item, CONTROLLER_ID_FMT, id);

  VNode* text = v_node_first_child(list_item);

  vs_set_direction(v_node_style(list_item), V_DIRECTION_ROW);
  vs_set_gap(v_node_style(list_item), THEME_SP_SM);

  vs_set_width(v_node_style(text), V_GROW());
  v_node_set_text_fmt(text, "%i :: %s :: %s", (int)id, Controller_GetName(id),
                      Controller_GetGUID(id));

  v_node_append_child(parent, list_item);
}

static void OnNavigatorEvent(NavigatorEvent* event)
{
  // TODO: fill the list once and rely on change events to update if visible or
  // invisible
  if (event->type == NAVIGATOR_EVENT_ENTER) {
    VNode* list = v_get_node_by_id(NID_CONTROLLER_LIST);

    v_node_remove_children(list);

    int controller_count = 0;
    ControllerId* controller_ids =
        ControllerListModel_SortControllers(&controller_count);

    // TODO: show message for no controllers

    for (int i = 0; i < controller_count; ++i) {
      AddControllerListItem(list, controller_ids[i]);
    }

    ControllerListModel_AddChangeEventListener(&OnControllerChangeEvent);
  }
}

static void OnControllerChangeEvent(const ControllerChangeEvent* event)
{
  VNode* list = v_get_node_by_id(NID_CONTROLLER_LIST);

  switch (event->change) {
    case CONTROLLER_CHANGE_ADDED:
      AddControllerListItem(list, event->id);
      break;
    case CONTROLLER_CHANGE_REMOVED: {
      VNode* cell = v_get_node_by_id_fmt(CONTROLLER_ID_FMT, event->id);

      v_node_remove_child(list, cell);
      break;
    }
    case CONTROLLER_CHANGE_INFO: {
      // TODO: update controller list item
      break;
    }
    // TODO: case CONTROLLER_CHANGE_POWER:
    //       case CONTROLLER_CHANGE_STEAM_HANDLE:
    default:
      break;
  }
}

static void InfoButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(event);
  ControllerListModel_SelectController(
      (ControllerId)(uintptr_t)v_node_data(node));
  PageNavigator_Goto(PAGEID_CONTROLLER_INFO);
}

static void EventsButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(event);
  ControllerListModel_SelectController(
      (ControllerId)(uintptr_t)v_node_data(node));
  ScreenNavigator_Goto(SCREENID_CONTROLLER_EVENTS);
}

static void ConfigureButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(event);
  ControllerListModel_SelectController(
      (ControllerId)(uintptr_t)v_node_data(node));
  ScreenNavigator_Goto(SCREENID_CONTROLLER_CONFIG);
}
