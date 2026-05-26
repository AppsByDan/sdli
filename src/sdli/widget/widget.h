#ifndef SDLI_WIDGET_H
#define SDLI_WIDGET_H

#include <sdli/controller-fwd.h>
#include <sdli/strings.h>

#include <vuid.h>

//
// macros
//

/* Get the string for the current locale. */
#define STR(SID) GetString(State_GetLocale(), SID)

//
// types
//

typedef enum NavigatorEventType {
  NAVIGATOR_EVENT_ENTER = 1,
  NAVIGATOR_EVENT_LEAVE = 2,
} NavigatorEventType;

typedef enum Direction {
  DIRECTION_FORWARD,
  DIRECTION_BACKWARD,
} Direction;

typedef struct NavigatorEvent {
  VNode* navigator;
  VNode* navigable;
  NavigatorEventType type;
  Direction direction;
} NavigatorEvent;

typedef VNode* (*CreateNavigableFn)(const char* id);
typedef void (*OnNavigatorEventFn)(NavigatorEvent* event);

//
// public functions
//

/*
 * Navigator is a container and controller for Navigable nodes. It functions
 * like a tab control or screen router.
 *
 * Each Navigable can receive enter and leave events to control a tab, page or
 * screen lifecycle.
 *
 * The Navigator uses a Navigable factory function to create Navigable nodes.
 * The factory function takes a string id that must be the node id of the
 * created Navigable. The user will use this id to navigate to Navigables.
 *
 * The Navigator caches Navigables by leaving them as children and setting
 * visibility to false. When transitioning, if the Navigable already exists, it
 * is shown. If not, the factory function will create the new Navigable.
 */

/* Initialize an existing node as a Navigator. */
VNode* Navigator_Init(VNode* node, CreateNavigableFn create_fn);
/* Jump to a new Navigable. If Navigator is at this Navigable, do nothing. */
void Navigator_Goto(VNode* navigator, const char* to);
/* Initialize an existing node as a Navigable. */
VNode* Navigable_Init(VNode* node, OnNavigatorEventFn on_event);

/*
 * KeyValueListItem is for displaying an informational key/value pair in a
 * spreadsheet-like list.
 *
 * The constructors take a node id for the value node so that users can easily
 * use the Bind* functions to set the value.
 */

VNode* KeyValueListItem(const char* key_name, const char* value_id);
VNode* KeyValueListItemLast(const char* key_name, const char* value_id);
VNode* KeyValueListItem_GetValue(VNode* kv_list_item);

/*
 * Button is the basic text label button for the UI. It reacts to hover and
 * click actions.
 */

VNode* Button(const char* label, void* data, VEventListener on_click);
VNode* ButtonWithId(const char* id,
                    const char* label,
                    void* data,
                    VEventListener on_click);
void Button_SetLabel(VNode* node, const char* text);

/*
 * Overlay is a modal or non-modal dialog box. The app has an overlay layer
 * attached to root above the screen layer. One overlay can be shown at a time
 * (no stacking).
 */

/* Create a new overlay layer for attachment to the root. */
VNode* OverlayLayer(void);
/* Show an overlay. The style of the overlay is defined by the caller. */
void Overlay_Show(VNode* overlay, bool modal);
/* Dismiss the currently shown overlay. */
void Overlay_Dismiss(void);

/*
 * State is global UI information that is needed between pages and screens, like
 * the current locale. State is distinct from a model, as models are
 * representing SDL or platform state. Technically, State is not in the scene
 * graph, but it is in "widget" for convenience.
 *
 * SelectedController is the currently selected or focused controller id.
 *
 * Locale is the current locale for UI strings, en-US is the default.
 */

void State_Init(void);
void State_Drop(void);

ControllerId State_GetSelectedController(void);
void State_SelectController(ControllerId id);
void State_ClearController(void);

Locale State_GetLocale(void);
void State_SetLocale(Locale locale);

void State_SetData(const char* key, void* data, void (*free_data)(void*));
void* State_GetData(const char* key);

#endif  // SDLI_WIDGET_H
