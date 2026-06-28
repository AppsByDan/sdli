#include "widget.h"

#include <sdli/style.h>
#include <sdli/util.h>

//
// private function declarations
//

static void ButtonImpl(NN_CALLABLE,
                       const char* id,
                       const char* label,
                       void* data,
                       const char* button_class,
                       VNodeEventListener on_click,
                       VNodeEventListener on_mouse_move);
static void OnMouseOverImpl(VNode* node,
                            VNodeEvent* event,
                            const char* button_class,
                            const char* button_hover_class);

//
// private node event handlers
//

OnMouseOverInline(Button, {
  OnMouseOverImpl(node, event, CLS_BUTTON, CLS_BUTTON_HOVER);
})

OnMouseOverInline(ButtonStretch, {
  OnMouseOverImpl(node, event, CLS_BUTTON_STRETCH, CLS_BUTTON_STRETCH_HOVER);
})

OnMouseOverInline(MenuButton, {
  OnMouseOverImpl(node, event, CLS_MENU_BUTTON, CLS_MENU_BUTTON_HOVER);
})

//
// public function implementation
//

void Button(NN_CALLABLE,
            const char* label,
            void* data,
            VNodeEventListener on_click)
{
  NN_CALL(ButtonImpl, NULL, label, data, CLS_BUTTON, on_click,
          &Button_OnMouseOver);
}

void ButtonStretch(NN_CALLABLE,
                   const char* label,
                   void* data,
                   VNodeEventListener on_click)
{
  NN_CALL(ButtonImpl, NULL, label, data, CLS_BUTTON_STRETCH, on_click,
          &ButtonStretch_OnMouseOver);
}

void MenuButton(NN_CALLABLE,
                const char* label,
                void* data,
                VNodeEventListener on_click)
{
  NN_BOX({
      .sclass = CLS_MENU_BUTTON,
      .data = data,
      .on_mouse_enter = &MenuButton_OnMouseOver,
      .on_mouse_leave = &MenuButton_OnMouseOver,
      .on_click = on_click,
  })
  {
    NN_TEXT({.text = label, .sclass = CLS_BUTTON_TEXT});
  }
}

void Button_SetLabel(VNode* node, const char* text)
{
  v_node_set_text(v_node_first_child(node), text);
}

//
// private function implementation
//

static void ButtonImpl(NN_CALLABLE,
                       const char* id,
                       const char* label,
                       void* data,
                       const char* button_class,
                       VNodeEventListener on_click,
                       VNodeEventListener on_mouse_move)
{
  NN_BOX({
      .id = id,
      .data = data,
      .sclass = button_class,
      .on_mouse_enter = on_mouse_move,
      .on_mouse_leave = on_mouse_move,
      .on_click = on_click,
  })
  {
    NN_TEXT({.text = label, .sclass = CLS_BUTTON_TEXT});
  }
}

static void OnMouseOverImpl(VNode* node,
                            VNodeEvent* event,
                            const char* button_class,
                            const char* button_hover_class)
{
  VNode* label_node = v_node_first_child(node);

  if (event->type == V_NODE_EVENT_MOUSE_ENTER) {
    v_node_style_assign_class(node, button_hover_class);
    v_node_style_assign_class(label_node, CLS_BUTTON_TEXT_HOVER);
  } else if (event->type == V_NODE_EVENT_MOUSE_LEAVE) {
    v_node_style_assign_class(node, button_class);
    v_node_style_assign_class(label_node, CLS_BUTTON_TEXT);
  }
}
