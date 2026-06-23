#include "node_notation.h"

typedef VNode* NNStackItem;

#define i_no_clone
#define i_no_emplace
#define i_key NNStackItem
#define i_type nn_stack
#include <stc/vec.h>

static nn_stack g_nn_stack;

static bool PushNode(VNodeTag type, const NodeAttributes* attrs);

void NodeNotation_Init(void)
{
  g_nn_stack = nn_stack_init();
}

void NodeNotation_Drop(void)
{
  isize size = nn_stack_size(&g_nn_stack);

  for (isize i = 0; i < size; i++) {
    v_node_unref(g_nn_stack.data[i]);
  }

  nn_stack_drop(&g_nn_stack);
}

NNState NN__BeginNode(NNState token, VNodeTag tag, const NodeAttributes* attrs)
{
  if (token == NN_STATE_ERROR) {
    return token;
  }

  if (!PushNode(tag, attrs)) {
    return NN_STATE_ERROR;
  }

  return NN_STATE_INIT;
}

NNState NN__EndNode(NNState token)
{
  if (token == NN_STATE_ERROR) {
    return token;
  }

  isize size = nn_stack_size(&g_nn_stack);

  if (size < 2) {
    return NN_STATE_ERROR;
  }

  VNode* parent = g_nn_stack.data[size - 2];

  if (parent == NULL) {
    // this is the "await" marker. move the popped node into this slot.
    g_nn_stack.data[size - 2] = g_nn_stack.data[size - 1];
    nn_stack_pop(&g_nn_stack);
  } else {
    VNode* back = g_nn_stack.data[size - 1];
    assert(back != NULL);
    nn_stack_pop(&g_nn_stack);
    v_node_unref(back);
  }

  return NN_STATE_SUCCESS;
}

VNode* NN__Self(void)
{
  isize size = nn_stack_size(&g_nn_stack);
  assert(size != 0);

  if (size == 0) {
    return NULL;
  }

  // note: can be the await slot (NULL)
  return g_nn_stack.data[size - 1];
}

VNode* NN__Parent(void)
{
  isize size = nn_stack_size(&g_nn_stack);
  assert(size >= 2);

  if (size < 2) {
    return NULL;
  }

  return g_nn_stack.data[size - 2];
}

NNState NN__BeginAppend(VNode* parent)
{
  if (!parent) {
    return NN_STATE_ERROR;
  }

  if (!nn_stack_push(&g_nn_stack, parent)) {
    return NN_STATE_ERROR;
  }

  v_node_ref(parent);

  return NN_STATE_INIT;
}

NNState NN__EndAppend(NNState token)
{
  if (token == NN_STATE_ERROR) {
    return token;
  }

  const isize size = nn_stack_size(&g_nn_stack);

  if (size == 0) {
    return NN_STATE_ERROR;
  }

  VNode* node = g_nn_stack.data[size - 1];
  nn_stack_pop(&g_nn_stack);
  v_node_unref(node);

  return NN_STATE_SUCCESS;
}

NNState NN__BeginBuild(void)
{
  if (!nn_stack_push(&g_nn_stack, NULL)) {
    return NN_STATE_ERROR;
  }

  return NN_STATE_INIT;
}

NNState NN__EndBuild(NNState token, VNode** result)
{
  if (token == NN_STATE_ERROR) {
    return token;
  }

  const isize size = nn_stack_size(&g_nn_stack);
  assert(size > 0);

  if (size == 0) {
    return NN_STATE_ERROR;
  }

  VNode* node = g_nn_stack.data[size - 1];
  assert(node != NULL);

  // caller is now the proud owner of the node ref
  *result = node;
  nn_stack_pop(&g_nn_stack);

  return NN_STATE_SUCCESS;
}

static bool PushNode(VNodeTag type, const NodeAttributes* attrs)
{
  VNode* parent = NN__Self();
  VNode* node = v_node_new(type);
  if (!node) {
    return false;
  }

  if (parent) {
    v_node_ref(node);
    bool result = v_node_append_child(parent, node);
    if (!result) {
      v_node_unref(node);
      return false;
    }
  }

  if (attrs->id) {
    v_node_set_id(node, attrs->id);
  }

  if (attrs->sclass) {
    v_node_style_assign_class(node, attrs->sclass);
  }

  if (attrs->popover != V_POPOVER_NONE) {
    v_node_set_popover(node, attrs->popover);
  }

  if (attrs->data) {
    v_node_set_data(node, attrs->data);
  }

  if (attrs->src) {
    v_node_set_src(node, attrs->src);
  }

  if (attrs->text) {
    v_node_set_text(node, attrs->text);
  }

  if (attrs->hidden) {
    v_node_set_visible(node, !attrs->hidden);
  }

  if (attrs->on_click) {
    v_node_set_event_listener(node, V_NODE_EVENT_CLICK, attrs->on_click);
  }

  if (attrs->on_mouse_enter) {
    v_node_set_event_listener(node, V_NODE_EVENT_MOUSE_ENTER,
                              attrs->on_mouse_enter);
  }

  if (attrs->on_mouse_leave) {
    v_node_set_event_listener(node, V_NODE_EVENT_MOUSE_LEAVE,
                              attrs->on_mouse_leave);
  }

  if (!nn_stack_push_back(&g_nn_stack, node)) {
    v_node_unref(node);
    return false;
  }

  return true;
}
