#include "widget.h"

#include <sdli/style.h>
#include <sdli/util.h>

//
// private function declarations
//

static void KeyValueListItem2Impl(NN_CALLABLE,
                                  const char* key_name,
                                  const char* value_id,
                                  bool is_last);

//
// public function implementation
//

void KeyValueListItem(NN_CALLABLE, const char* key_name, const char* value_id)
{
  KeyValueListItem2Impl(NN_STATE(), key_name, value_id, false);
}

void KeyValueListItemLast(NN_CALLABLE,
                          const char* key_name,
                          const char* value_id)
{
  KeyValueListItem2Impl(NN_STATE(), key_name, value_id, true);
}

VNode* KeyValueListItem_GetValue(VNode* kv_list_item)
{
  return v_node_child_at(kv_list_item, 1);
}

//
// private function implementation
//

static void KeyValueListItem2Impl(NN_CALLABLE,
                                  const char* key_name,
                                  const char* value_id,
                                  bool is_last)
{
  NN_BOX({.sclass = is_last ? CLS_LIST_ITEM_LAST : CLS_LIST_ITEM})
  {
    NN_TEXT({
        .text = key_name,
        .sclass = CLS_LIST_ITEM_KEY_TEXT,
    });
    NN_TEXT({
        .id = value_id,
        .sclass = CLS_LIST_ITEM_VALUE_TEXT,
    });
  }
}
