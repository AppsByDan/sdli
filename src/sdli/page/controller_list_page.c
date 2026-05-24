#include "page.h"

#include <sdli/app.h>
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

static VNode* ControllerListItem(ControllerId id);
static void OnNavigatorEvent(NavigatorEvent* event);
static void OnControllerChangeEvent(const ControllerChangeEvent* event);
static void InfoButtonOnClick(VNode* node, VEvent* event);
static void EventsButtonOnClick(VNode* node, VEvent* event);
static void ConfigureButtonOnClick(VNode* node, VEvent* event);
static void RemoveMappingButtonOnClick(VNode* node, VEvent* event);
static void CopyMappingButtonOnClick(VNode* node, VEvent* event);
static void ReloadMappingsButtonOnClick(VNode* node, VEvent* event);
static void LoadMappingsFromClipboardButtonOnClick(VNode* node, VEvent* event);
static void ExportMappingsToClipboardButtonOnClick(VNode* node, VEvent* event);
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

static VNode* ControllerListItem(ControllerId id)
{
  // TODO: create a proper controller list item widget

  void* button_data = (void*)(uintptr_t)id;
  // clang-format off
  VNode* list_item = Box({
    .sclass = CLS_LIST,
    Children(
      Text({.sclass = CLS_TEXT}),
      // TODO: localize
      Button("Events", button_data, &EventsButtonOnClick),
      Button("Configure", button_data, &ConfigureButtonOnClick),
      Button("I", button_data, &InfoButtonOnClick)
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

  if (Controller_HasMapping(id)) {
    v_node_append_child(list_item,
                        Button("RM", button_data, &RemoveMappingButtonOnClick));
    v_node_append_child(list_item,
                        Button("CM", button_data, &CopyMappingButtonOnClick));
  }

  return list_item;
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
    // TODO: the button row is not final design

    // clang-format off
    v_node_append_child(list, Box({
      .sclass= CLS_BUTTON_ROW,
      Children(
        Button("Reload Mappings", NULL, &ReloadMappingsButtonOnClick),
        Button("Load Mappings (Clipboard)", NULL, &LoadMappingsFromClipboardButtonOnClick),
        Button("Export Mappings (Clipboard)", NULL, &ExportMappingsToClipboardButtonOnClick)
      )
    }));
    // clang-format on

    for (int i = 0; i < controller_count; ++i) {
      v_node_append_child(list, ControllerListItem(controller_ids[i]));
    }

    ControllerListModel_AddChangeEventListener(&OnControllerChangeEvent);
  }
}

static void OnControllerChangeEvent(const ControllerChangeEvent* event)
{
  VNode* list = v_get_node_by_id(NID_CONTROLLER_LIST);

  switch (event->change) {
    case CONTROLLER_CHANGE_ADDED:
      v_node_append_child(list, ControllerListItem(event->id));
      break;
    case CONTROLLER_CHANGE_REMOVED: {
      v_node_remove_child(list,
                          v_get_node_by_id_fmt(CONTROLLER_ID_FMT, event->id));
      break;
    }
    case CONTROLLER_CHANGE_INFO: {
      VNode* old_list_item = v_get_node_by_id_fmt(CONTROLLER_ID_FMT, event->id);

      if (old_list_item) {
        v_node_replace_child(list, ControllerListItem(event->id),
                             old_list_item);
      }
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

static void RemoveMappingButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(event);
  Controller_RemoveMapping((ControllerId)(uintptr_t)v_node_data(node));
}

static void CopyMappingButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(event);
  const char* mapping_str =
      Controller_GetMappingString((ControllerId)(uintptr_t)v_node_data(node));

  if (*mapping_str) {
    App_CopyToClipboard(mapping_str);
  }
}

static void ReloadMappingsButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(node, event);
  ControllerListModel_ReloadMappings();
}

static void LoadMappingsFromClipboardButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(node, event);
  ControllerListModel_LoadMappingsFromClipboard();
}

static void ExportMappingsToClipboardButtonOnClick(VNode* node, VEvent* event)
{
  UNUSED(node, event);
  ControllerListModel_ExportMappingsToClipboard();
}
