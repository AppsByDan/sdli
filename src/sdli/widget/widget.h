#ifndef SDLI_WIDGET_H
#define SDLI_WIDGET_H

#include <vuid.h>

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

VNode* Navigator_Init(VNode* node, CreateNavigableFn create_fn);
void Navigator_Goto(VNode* navigator, const char* to);

VNode* Navigable_Init(VNode* node, OnNavigatorEventFn on_event);

VNode* KeyValueListItem(const char* key_name, const char* value_id);
VNode* KeyValueListItemLast(const char* key_name, const char* value_id);

VNode* Button(const char* label, void* data, VEventListener on_click);
void Button_SetLabel(VNode* node, const char* text);

#endif  // SDLI_WIDGET_H
