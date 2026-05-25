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
  VNode* overlay_layer = Box({
      .id = NID_OVERLAY_LAYER,
      .sclass = CLS_OVERLAY_LAYER,
  });
  v_node_set_visible(overlay_layer, false);
  return overlay_layer;
}

void Overlay_Show(VNode* overlay, bool modal)
{
  UNUSED(modal);  // TODO: implement modal behavior

  VNode* overlay_layer = v_get_node_by_id(NID_OVERLAY_LAYER);

  if (!overlay_layer) {
    SLog("Overlay_Show: overlay layer node not found");
    return;
  }

  v_node_set_visible(overlay_layer, true);
  v_node_append_child(overlay_layer, overlay);
}

void Overlay_Dismiss(void)
{
  VNode* overlay_layer = v_get_node_by_id(NID_OVERLAY_LAYER);

  v_node_remove_children(overlay_layer);
  v_node_set_visible(overlay_layer, false);
}
