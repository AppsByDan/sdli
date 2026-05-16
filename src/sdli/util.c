#include <sdli/util.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <stc/common.h>

//
// private function declarations
//

static Navigable* FindNavigable(Navigator* navigator, const char* id);
static bool NodeIdEqual(VNode* node, const char* id);

//
// public function implementation
//

void Navigator_Init(Navigator* navigator, const char* container_id)
{
  *navigator = (Navigator){
      .container_id = container_id,
      .navigables_size = 0,
  };
}

void Navigator_Drop(Navigator* navigator)
{
  *navigator = (Navigator){0};
}

void Navigator_Register(Navigator* navigator,
                        const char* id,
                        NavigableCreate create,
                        NavigableEnter enter,
                        NavigableLeave leave)
{
  assert(id);
  assert(create);

  if (navigator->navigables_size >= c_arraylen(navigator->navigables) ||
      FindNavigable(navigator, id) != NULL) {
    abort();
  }

  navigator->navigables[navigator->navigables_size++] = (Navigable){
      .id = id,
      .create = create,
      .enter = enter,
      .leave = leave,
  };
}

void Navigator_Push(Navigator* navigator, const char* from, const char* to)
{
  VNode* container = v_get_node_by_id(navigator->container_id);
  assert(container);
  Navigable* to_nav = FindNavigable(navigator, to);
  assert(to_nav);
  VNode* top = v_node_last_child(container);

  if (top != NULL && !NodeIdEqual(top, from)) {
    return;
  }

  VNode* new_node = to_nav->create(to_nav->id);

  if (to_nav->enter) {
    to_nav->enter(new_node, NAV_FORWARD);
  }

  v_node_set_visible(top, false);
  v_node_append_child(container, new_node);
}

void Navigator_Pop(Navigator* navigator, const char* from)
{
  VNode* container = v_get_node_by_id(navigator->container_id);
  assert(container);
  VNode* top = v_node_last_child(container);

  if (top == NULL || !NodeIdEqual(top, from)) {
    return;
  }

  Navigable* from_nav = FindNavigable(navigator, from);
  assert(from_nav);

  if (from_nav->leave) {
    from_nav->leave(top, NAV_BACKWARD);
  }

  v_node_remove_child(container, top);

  VNode* new_top = v_node_last_child(container);

  if (new_top) {
    v_node_set_visible(new_top, true);
  }
}

//
// private function implementation
//

static Navigable* FindNavigable(Navigator* navigator, const char* id)
{
  for (size_t i = 0; i < navigator->navigables_size; i++) {
    if (strcmp(navigator->navigables[i].id, id) == 0) {
      return &navigator->navigables[i];
    }
  }
  return NULL;
}

static bool NodeIdEqual(VNode* node, const char* id)
{
  return strcmp(v_node_id(node), id) == 0;
}
