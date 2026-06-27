#include "page.h"

#include <sdli/app.h>
#include <sdli/model/model.h>
#include <sdli/screen/screen.h>
#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

#include <stc/cstr.h>

#define NID_CONTROLLER_LIST "clp:clist"
#define CONTROLLER_ID_FMT "ctrl:%" PRIu32

//
// private function declarations
//

static void ControllerListItem(NN_CALLABLE, ControllerId id);
static void OnNavigatorEvent(NavigatorEvent* event);
static void OnControllerChangeEvent(const ControllerChangeEvent* event);
static void OnFileDialogResult(int event_type,
                               void* event_data,
                               void* user_data);
static void ReloadMappingsOverlay(void);
static void LoadMappingsOverlay(void);
static void ExportMappingsOverlay(void);
static void WaitOverlay(const char* title);
static void LoadMappingsResultsOverlay(int count);
static void ExportMappingsResultOverlay(bool success);
static ControllerId GetControllerId(VNode* node);

//
// private node event handlers
//

// clang-format off
OnClickInline(Info, {
  State_SelectController(GetControllerId(node));
  PageNavigator_Goto(PAGEID_CONTROLLER_INFO);
})

OnClickInline(Events, {
  State_SelectController(GetControllerId(node));
  ScreenNavigator_Goto(SCREENID_CONTROLLER_EVENTS);
})

OnClickInline(Configure, {
  State_SelectController(GetControllerId(node));
  ScreenNavigator_Goto(SCREENID_CONTROLLER_CONFIG);
})

OnClickInline(Rumble, {
  Controller_Rumble(GetControllerId(node));
})

OnClickInline(RemoveMapping, {
  Controller_RemoveMapping(GetControllerId(node));
})

OnClickInline(CopyMapping, {
  SystemModel_CopyToClipboard(
      Controller_GetMappingString(GetControllerId(node)));
})

OnClickInline(ReloadMappings, {
  ReloadMappingsOverlay();
})

OnClickInline(LoadMappings, {
  LoadMappingsOverlay();
})

OnClickInline(ExportMappings, {
  ExportMappingsOverlay();
})

OnClickInline(ReloadMappingsOverlayOk, {
  ControllerListModel_ReloadMappings();
  Overlay_Dismiss();
})

OnClickInline(LoadMappingsFromClipboard, {
  int count = ControllerListModel_LoadMappingsFromClipboard();
  Overlay_Dismiss();
  LoadMappingsResultsOverlay(count);
})

OnClickInline(LoadMappingsFromFile, {
  App_ShowOpenFileDialog();
  Overlay_Dismiss();
  WaitOverlay("Load Mappings");
})

OnClickInline(ExportMappingsToClipboard, {
  ControllerListModel_ExportMappingsToClipboard();
  Overlay_Dismiss();
})

OnClickInline(ExportMappingsToFile, {
  App_ShowSaveFileDialog();
  Overlay_Dismiss();
  WaitOverlay("Export Mappings");
})
// clang-format on

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

      vs_set_width(text_style, v_grow());
      v_node_set_text_fmt(text, "%i :: %s :: %s", (int)id,
                          Controller_GetName(id), Controller_GetGUID(id));
    }

    void* button_data = (void*)(uintptr_t)id;

    // TODO: localize
    NN_CALL(Button, "Events", button_data, &Events_OnClick);
    NN_CALL(Button, "Configure", button_data, &Configure_OnClick);
    NN_CALL(Button, "I", button_data, &Info_OnClick);

    if (Controller_HasRumble(id)) {
      NN_CALL(Button, "R", button_data, &Rumble_OnClick);
    }

    if (Controller_HasMapping(id)) {
      NN_CALL(Button, "RM", button_data, &RemoveMapping_OnClick);
      NN_CALL(Button, "CM", button_data, &CopyMapping_OnClick);
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
      // TODO: need at least 1 connected controller with mapping for export
      // TODO: reload and load dont make sense with no controllers connected
      NN_CALL(Button, "Reload mappings", NULL, &ReloadMappings_OnClick);
      NN_CALL(Button, "Load mappings", NULL, &LoadMappings_OnClick);
      NN_CALL(Button, "Export mappings", NULL, &ExportMappings_OnClick);
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
  App_AddEventListener(EVT_OPEN_FILE_DIALOG_RESULT, &OnFileDialogResult, NULL);
  App_AddEventListener(EVT_SAVE_FILE_DIALOG_RESULT, &OnFileDialogResult, NULL);
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

static void OnFileDialogResult(int event_type,
                               void* event_data,
                               void* user_data)
{
  UNUSED(event_type, user_data);
  Overlay_Dismiss();

  if (event_type == EVT_OPEN_FILE_DIALOG_RESULT) {
    int count = ControllerListModel_LoadMappingsFromFile(
        ((FileDialogResultEvent*)event_data)->filename);
    LoadMappingsResultsOverlay(count);
  } else if (event_type == EVT_SAVE_FILE_DIALOG_RESULT) {
    bool result = ControllerListModel_ExportMappingsToFile(
        ((FileDialogResultEvent*)event_data)->filename);

    ExportMappingsResultOverlay(result);
  }
}

static void ReloadMappingsOverlay(void)
{
  static const OverlayButton buttons[] = {
      {.label = "Reload", .on_click = &ReloadMappingsOverlayOk_OnClick},
      {.label = "Cancel", .on_click = &Overlay_Cancel},
  };

  Overlay_Show("Reload Mappings",
               "Reset the SDL Gamepad mapping database to its initial state. "
               "All controller configurations will be lost.",
               buttons, u_arraylen(buttons));
}

static void LoadMappingsOverlay(void)
{
  static const OverlayButton buttons[] = {
      {.label = "From file...", .on_click = &LoadMappingsFromFile_OnClick},
      {.label = "From clipboard",
       .on_click = &LoadMappingsFromClipboard_OnClick},
      {.label = "Cancel", .on_click = &Overlay_Cancel},
  };

  Overlay_Show("Load Mappings", "Description.", buttons, u_arraylen(buttons));
}

static void ExportMappingsOverlay(void)
{
  static const OverlayButton buttons[] = {
      {.label = "Save to file...", .on_click = &ExportMappingsToFile_OnClick},
      {.label = "Copy to clipboard",
       .on_click = &ExportMappingsToClipboard_OnClick},
      {.label = "Cancel", .on_click = &Overlay_Cancel},
  };

  Overlay_Show(
      "Export Mappings",
      "Export Gamepad mappings to a file or the clipboard in CSV format.",
      buttons, u_arraylen(buttons));
}

static void WaitOverlay(const char* title)
{
  // TODO: different type of overlay?
  Overlay_Show(title, "Waiting for system file dialog.", NULL, 0);
}

static void LoadMappingsResultsOverlay(int count)
{
  // TODO: error message, if possible? (SDL apis dont always return error
  // messages for loading mappings)
  cstr text = cstr_from_fmt("Mappings processed: %i", count);

  static const OverlayButton buttons[] = {
      {.label = "OK", .on_click = &Overlay_Cancel},
  };

  Overlay_Show("Load Mappings", cstr_str(&text), buttons, u_arraylen(buttons));
  cstr_drop(&text);
}

static void ExportMappingsResultOverlay(bool success)
{
  static const OverlayButton buttons[] = {
      {.label = "OK", .on_click = &Overlay_Cancel},
  };

  Overlay_Show("Export Mappings",
               success ? "Mappings exported successfully."
                       : "Failed to export mappings.",
               buttons, u_arraylen(buttons));
}

static ControllerId GetControllerId(VNode* node)
{
  return (ControllerId)(uintptr_t)v_node_data(node);
}
