#include "widget.h"

#include <sdli/style.h>

//
// macros & constants
//

#define SELECT_BUTTON_INDEX (0)
#define SELECT_POPOVER_INDEX (1)

#define SELECT_BUTTON_LABEL_INDEX (0)
#define SELECT_BUTTON_ICON_INDEX (1)

#define MENU_ITEM_ICON_INDEX (0)
#define MENU_ITEM_LABEL_INDEX (1)

//
// private function declarations
//

static bool SetCheckmark(VNode* select_button, void* selected_data);
static void OnMouseOverImpl(VNode* node,
                            VNodeEvent* event,
                            const char* normal_class,
                            const char* hover_class,
                            int icon_index,
                            int label_index);

//
// private node event handlers
//

OnClickInline(SelectButton, {
  v_node_show_popover(
      v_node_child_at(v_node_parent(node), SELECT_POPOVER_INDEX));
  v_node_event_stop_propagation(event);
})

OnMouseOverInline(SelectButton, {
  OnMouseOverImpl(node, event, CLS_BUTTON, CLS_BUTTON_HOVER,
                  SELECT_BUTTON_ICON_INDEX, SELECT_BUTTON_LABEL_INDEX);
})

OnClickInline(SelectItem, {
  VNode* menu = v_node_parent(node);
  VNode* select = v_node_parent(menu);

  v_node_hide_popover(menu);

  // onclick is the select's change event. bubble if something changed.
  if (!SetCheckmark(select, v_node_data(node))) {
    v_node_event_stop_propagation(event);
  }
})

OnMouseOverInline(SelectItem, {
  OnMouseOverImpl(node, event, CLS_MENU_BUTTON, CLS_MENU_BUTTON_HOVER,
                  MENU_ITEM_ICON_INDEX, MENU_ITEM_LABEL_INDEX);
})

//
// public function implementation
//

void Select(NN_CALLABLE,
            const SelectItem* items,
            size_t item_count,
            void* selected_data,
            VNodeEventListener on_click)
{
  const char* label = "";

  for (size_t i = 0; i < item_count; ++i) {
    if (items[i].data == selected_data) {
      label = items[i].label;
      break;
    }
  }

  NN_BOX({.on_click = on_click})
  {
    NN_BOX({
        .sclass = CLS_BUTTON,
        .on_mouse_enter = &SelectButton_OnMouseOver,
        .on_mouse_leave = &SelectButton_OnMouseOver,
        .on_click = &SelectButton_OnClick,
    })
    {
      NN_TEXT({.text = label, .sclass = CLS_BUTTON_TEXT});
      NN_TEXT({.text = ICON_ANGLE_DOWN, .sclass = CLS_BUTTON_ICON});
    }

    NN_BOX({.sclass = CLS_MENU, .popover = V_POPOVER_AUTO})
    {
      for (size_t i = 0; i < item_count; ++i) {
        const SelectItem* item = &items[i];

        NN_BOX({
            .sclass = CLS_MENU_BUTTON,
            .data = item->data,
            .on_mouse_enter = &SelectItem_OnMouseOver,
            .on_mouse_leave = &SelectItem_OnMouseOver,
            .on_click = &SelectItem_OnClick,
        })
        {
          NN_TEXT({.text = ICON_OK, .sclass = CLS_BUTTON_ICON});
          NN_TEXT({.text = item->label, .sclass = CLS_BUTTON_TEXT});
        }
      }
    }

    SetCheckmark(NN_SELF(), selected_data);
  }
}

//
// private function implementation
//

static bool SetCheckmark(VNode* select, void* selected_data)
{
  if (v_node_data(select) == selected_data) {
    return false;
  }

  VNode* select_menu = v_node_child_at(select, SELECT_POPOVER_INDEX);
  VNode* select_button = v_node_child_at(select, SELECT_BUTTON_INDEX);

  v_foreach_child(select_menu, item_node)
  {
    VNode* icon_node = v_node_child_at(item_node, MENU_ITEM_ICON_INDEX);
    VNode* label_node = v_node_child_at(item_node, MENU_ITEM_LABEL_INDEX);
    VStyle* icon_style = v_node_style(icon_node);

    if (v_node_data(item_node) == selected_data) {
      v_node_set_text(v_node_child_at(select_button, SELECT_BUTTON_LABEL_INDEX),
                      v_node_text(label_node));
      vs_set_visibility(icon_style, V_VISIBILITY_VISIBLE);
    } else {
      vs_set_visibility(icon_style, V_VISIBILITY_HIDDEN);
    }
  }

  v_node_set_data(select, selected_data);

  return true;
}

static void OnMouseOverImpl(VNode* node,
                            VNodeEvent* event,
                            const char* normal_class,
                            const char* hover_class,
                            int icon_index,
                            int label_index)
{
  VNode* label_node = v_node_child_at(node, label_index);
  VNode* icon_node = v_node_child_at(node, icon_index);
  // ensure the checkmark stay visible
  VVisibility visibility = vs_get_visibility(v_node_style(icon_node));

  if (event->type == V_NODE_EVENT_MOUSE_ENTER) {
    v_node_style_assign_class(node, hover_class);
    v_node_style_assign_class(label_node, CLS_BUTTON_TEXT_HOVER);
    v_node_style_assign_class(icon_node, CLS_BUTTON_ICON_HOVER);
  } else if (event->type == V_NODE_EVENT_MOUSE_LEAVE) {
    v_node_style_assign_class(node, normal_class);
    v_node_style_assign_class(label_node, CLS_BUTTON_TEXT);
    v_node_style_assign_class(icon_node, CLS_BUTTON_ICON);
  }

  vs_set_visibility(v_node_style(icon_node), visibility);
}
