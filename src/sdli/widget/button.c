#include "widget.h"

#include <sdli/style.h>
#include <sdli/util.h>

//
// private function declarations
//

static void OnMouseMove(VNode* node, VNodeEvent* event);

//
// public function implementation
//

void Button(NN_CALLABLE,
            const char* label,
            void* data,
            VNodeEventListener on_click)
{
  ButtonWithId(NN_STATE(), NULL, label, data, on_click);
}

void ButtonWithId(NN_CALLABLE,
                  const char* id,
                  const char* label,
                  void* data,
                  VNodeEventListener on_click)
{
  NN_BOX({
      .id = id,
      .data = data,
      .sclass = CLS_BUTTON,
      .on_mouse_enter = &OnMouseMove,
      .on_mouse_leave = &OnMouseMove,
      .on_click = on_click,
  })
  {
    NN_TEXT({.text = label, .sclass = CLS_BUTTON_TEXT});
  }
}

void Button_SetLabel(VNode* node, const char* text)
{
  v_node_set_text(v_node_child_at(node, 0), text);
}

//
// private function implementation
//

static void OnMouseMove(VNode* node, VNodeEvent* event)
{
  if (event->type == V_NODE_EVENT_MOUSE_ENTER) {
    v_node_style_assign_class(node, CLS_BUTTON_HOVER);
    v_node_style_assign_class(v_node_child_at(node, 0), CLS_BUTTON_TEXT_HOVER);
  } else if (event->type == V_NODE_EVENT_MOUSE_LEAVE) {
    v_node_style_assign_class(node, CLS_BUTTON);
    v_node_style_assign_class(v_node_child_at(node, 0), CLS_BUTTON_TEXT);
  }
}
