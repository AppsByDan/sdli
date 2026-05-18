#include "widget.h"

#include <sdli/util.h>
#include <stdint.h>

//
// private function declarations
//

static void Dispatch(NavigatorEvent* event);
static VNode* GetCurrentNavigable(VNode* navigator);
static VNode* GetNavigable(VNode* navigator, const char* id);

//
// public function implementation
//

VNode* Navigator_Init(VNode* node, CreateNavigableFn create_fn)
{
  v_node_set_data(node, (void*)(uintptr_t)create_fn);
  return node;
}

void Navigator_Goto(VNode* navigator, const char* to)
{
  VNode* current_navigable = GetCurrentNavigable(navigator);

  if (current_navigable) {
    if (NodeIdEq(current_navigable, to)) {
      // TODO: re-enter?
      return;
    }

    NavigatorEvent leave_event = {
        .navigator = navigator,
        .navigable = current_navigable,
        .type = NAVIGATOR_EVENT_LEAVE,
        .direction = DIRECTION_FORWARD,
    };

    Dispatch(&leave_event);

    v_node_set_visible(current_navigable, false);
  }

  VNode* navigable = GetNavigable(navigator, to);

  NavigatorEvent event = {
      .navigator = navigator,
      .navigable = navigable,
      .type = NAVIGATOR_EVENT_ENTER,
      .direction = DIRECTION_FORWARD,
  };

  Dispatch(&event);
}

VNode* Navigable_Init(VNode* node, OnNavigatorEventFn on_event)
{
  v_node_set_data(node, (void*)(uintptr_t)on_event);
  return node;
}

static void Dispatch(NavigatorEvent* event)
{
  OnNavigatorEventFn on_event =
      (OnNavigatorEventFn)(uintptr_t)v_node_data(event->navigable);
  assert(on_event);

  on_event(event);
}

//
// private function implementation
//

static VNode* GetCurrentNavigable(VNode* navigator)
{
  v_foreach_child(navigator, child)
  {
    if (v_node_is_visible(child)) {
      return child;
    }
  }

  return NULL;
}

static VNode* GetNavigable(VNode* navigator, const char* id)
{
  v_foreach_child(navigator, child)
  {
    if (NodeIdEq(child, id)) {
      v_node_set_visible(child, true);
      return child;
    }
  }

  CreateNavigableFn create_fn =
      (CreateNavigableFn)(uintptr_t)v_node_data(navigator);
  assert(create_fn);

  VNode* navigable = create_fn(id);
  assert(navigable);

  v_node_append_child(navigator, navigable);

  return navigable;
}
