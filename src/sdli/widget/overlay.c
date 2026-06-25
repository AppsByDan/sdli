#include "widget.h"

#include <sdli/style.h>
#include <sdli/util.h>

//
// constants
//

#define NID_OVERLAY_LAYER "overlay-layer"

//
// public function implementation
//

VNode* OverlayLayer(void)
{
  NN_BUILD_NEW(overlay_layer)
  {
    NN_BOX({
        .id = NID_OVERLAY_LAYER,
        .sclass = CLS_OVERLAY_LAYER,
        .hidden = true,
    });
  }

  return overlay_layer;
}

void Overlay_Show(const char* title,
                  const char* body,
                  const OverlayButton* buttons,
                  size_t button_count)
{
  VNode* overlay_layer = v_get_node_by_id(NID_OVERLAY_LAYER);

  v_node_remove_children(overlay_layer);
  v_node_set_visible(overlay_layer, true);

  NN_BUILD_APPEND(overlay_layer)
  {
    NN_BOX({.sclass = CLS_OVERLAY})
    {
      NN_TEXT({.text = title, .sclass = CLS_OVERLAY_TITLE});
      NN_TEXT({.text = body, .sclass = CLS_OVERLAY_BODY_TEXT});
      for (size_t i = 0; i < button_count; i++) {
        NN_CALL(ButtonStretch, buttons[i].label, buttons[i].data,
                buttons[i].on_click);
      }
    }
  }
}

void Overlay_Dismiss(void)
{
  VNode* overlay_layer = v_get_node_by_id(NID_OVERLAY_LAYER);
  assert(overlay_layer);

  v_node_remove_children(overlay_layer);
  v_node_set_visible(overlay_layer, false);
}

void Overlay_Cancel(VNode* node, VNodeEvent* event)
{
  UNUSED(node, event);
  Overlay_Dismiss();
}
