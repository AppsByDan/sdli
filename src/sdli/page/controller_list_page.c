#include "page.h"

#include <sdli/model/model.h>
#include <sdli/screen/screen.h>
#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

#define NID_CONTROLLER_LIST "clp:clist"
#define CONTROLLER_ID_FMT "ctrl:%" PRIu32

//
// private function declarations
//

static void ControllerListItem(NN_CALLABLE, ControllerId id);
static void OnNavigatorEvent(NavigatorEvent* event);
static void OnControllerChangeEvent(const ControllerChangeEvent* event);
static void InfoButtonOnClick(VNode* node, VNodeEvent* event);
static void EventsButtonOnClick(VNode* node, VNodeEvent* event);
static void ConfigureButtonOnClick(VNode* node, VNodeEvent* event);
static void RemoveMappingButtonOnClick(VNode* node, VNodeEvent* event);
static void CopyMappingButtonOnClick(VNode* node, VNodeEvent* event);
static void RumbleButtonOnClick(VNode* node, VNodeEvent* event);
static void ReloadMappingsButtonOnClick(VNode* node, VNodeEvent* event);
static void LoadMappingsFromClipboardButtonOnClick(VNode* node,
                                                   VNodeEvent* event);
static void ExportMappingsToClipboardButtonOnClick(VNode* node,
                                                   VNodeEvent* event);
static ControllerId GetControllerId(VNode* node);

//
// public function implementation
//

VNode* ControllerListPage(void)
{
  NN_BUILD_NEW(page)
  {
    NN_BOX({.id = PAGEID_CONTROLLER_LIST, .sclass = CLS_PAGE})
    {
      NN_TEXT({.text = STR(SID_CONTROLLERS), .sclass = CLS_PAGE_H1});
      NN_BOX({
          .id = NID_CONTROLLER_LIST,
          .sclass = CLS_SCROLLABLE_LIST,
      });
    }
  }

  return Navigable_Init(page, &OnNavigatorEvent);
}

//
// private function implementation
//

static void ControllerListItem(NN_CALLABLE, ControllerId id)
{
  // TODO: create a proper controller list item widget

  NN_BOX({.sclass = CLS_LIST})
  {
    VNode* list_item = NN_SELF();
    VStyle* list_item_style = v_node_style(list_item);

    v_node_set_id_fmt(list_item, CONTROLLER_ID_FMT, id);
    vs_set_direction(list_item_style, V_DIRECTION_ROW);
    vs_set_gap(list_item_style, THEME_SP_SM);

    NN_TEXT({.sclass = CLS_TEXT})
    {
      VNode* text = NN_SELF();
      VStyle* text_style = v_node_style(text);

      vs_set_width(text_style, V_GROW());
      v_node_set_text_fmt(text, "%i :: %s :: %s", (int)id,
                          Controller_GetName(id), Controller_GetGUID(id));
    }

    void* button_data = (void*)(uintptr_t)id;

    // TODO: localize
    NN_CALL(Button, "Events", button_data, &EventsButtonOnClick);
    NN_CALL(Button, "Configure", button_data, &ConfigureButtonOnClick);
    NN_CALL(Button, "I", button_data, &InfoButtonOnClick);

    if (Controller_HasRumble(id)) {
      NN_CALL(Button, "R", button_data, &RumbleButtonOnClick);
    }

    if (Controller_HasMapping(id)) {
      NN_CALL(Button, "RM", button_data, &RemoveMappingButtonOnClick);
      NN_CALL(Button, "CM", button_data, &CopyMappingButtonOnClick);
    }
  }
}

static void OnNavigatorEvent(NavigatorEvent* event)
{
  // TODO: fill the list once and rely on change events to update if visible or
  // invisible
  if (event->type != NAVIGATOR_EVENT_ENTER) {
    return;
  }

  VNode* list = v_get_node_by_id(NID_CONTROLLER_LIST);

  v_node_remove_children(list);

  NN_BUILD_APPEND(list)
  {
    NN_BOX({.sclass = CLS_BUTTON_ROW})
    {
      NN_CALL(Button, "Reload Mappings", NULL, &ReloadMappingsButtonOnClick);
      NN_CALL(Button, "Load Mappings (Clipboard)", NULL,
              &LoadMappingsFromClipboardButtonOnClick);
      NN_CALL(Button, "Export Mappings (Clipboard)", NULL,
              &ExportMappingsToClipboardButtonOnClick);
    }

    // TODO: show message for no controllers
    // TODO: the button row is not final design

    int controller_count = 0;
    ControllerId* controller_ids =
        ControllerListModel_SortControllers(&controller_count);

    for (int i = 0; i < controller_count; ++i) {
      NN_CALL(ControllerListItem, controller_ids[i]);
    }
  }

  ControllerListModel_AddChangeEventListener(&OnControllerChangeEvent);
}

static void OnControllerChangeEvent(const ControllerChangeEvent* event)
{
  VNode* list = v_get_node_by_id(NID_CONTROLLER_LIST);

  switch (event->change) {
    case CONTROLLER_CHANGE_ADDED:
      NN_BUILD_APPEND(list)
      {
        ControllerListItem(NN_STATE(), event->id);
      }
      break;
    case CONTROLLER_CHANGE_REMOVED: {
      v_node_remove_child(list,
                          v_get_node_by_id_fmt(CONTROLLER_ID_FMT, event->id));
      break;
    }
    case CONTROLLER_CHANGE_INFO: {
      VNode* old_list_item = v_get_node_by_id_fmt(CONTROLLER_ID_FMT, event->id);

      if (old_list_item) {
        NN_BUILD_NEW(new_list_item)
        {
          NN_CALL(ControllerListItem, event->id);
        }
        v_node_replace_child(list, new_list_item, old_list_item);
      }
      break;
    }
    // TODO: case CONTROLLER_CHANGE_POWER:
    //       case CONTROLLER_CHANGE_STEAM_HANDLE:
    default:
      break;
  }
}

static void InfoButtonOnClick(VNode* node, VNodeEvent* event)
{
  UNUSED(event);
  State_SelectController(GetControllerId(node));
  PageNavigator_Goto(PAGEID_CONTROLLER_INFO);
}

static void EventsButtonOnClick(VNode* node, VNodeEvent* event)
{
  UNUSED(event);
  State_SelectController(GetControllerId(node));
  ScreenNavigator_Goto(SCREENID_CONTROLLER_EVENTS);
}

static void ConfigureButtonOnClick(VNode* node, VNodeEvent* event)
{
  UNUSED(event);
  State_SelectController(GetControllerId(node));
  ScreenNavigator_Goto(SCREENID_CONTROLLER_CONFIG);
}

static void RemoveMappingButtonOnClick(VNode* node, VNodeEvent* event)
{
  UNUSED(event);
  Controller_RemoveMapping(GetControllerId(node));
}

static void CopyMappingButtonOnClick(VNode* node, VNodeEvent* event)
{
  UNUSED(event);
  SystemModel_CopyToClipboard(
      Controller_GetMappingString(GetControllerId(node)));
}

static void RumbleButtonOnClick(VNode* node, VNodeEvent* event)
{
  UNUSED(event);
  Controller_Rumble(GetControllerId(node));
}

static void ReloadMappingsButtonOnClick(VNode* node, VNodeEvent* event)
{
  UNUSED(node, event);
  ControllerListModel_ReloadMappings();
}

static void LoadMappingsFromClipboardButtonOnClick(VNode* node,
                                                   VNodeEvent* event)
{
  UNUSED(node, event);
  ControllerListModel_LoadMappingsFromClipboard();
}

static void ExportMappingsToClipboardButtonOnClick(VNode* node,
                                                   VNodeEvent* event)
{
  UNUSED(node, event);
  ControllerListModel_ExportMappingsToClipboard();
}

static ControllerId GetControllerId(VNode* node)
{
  return (ControllerId)(uintptr_t)v_node_data(node);
}
