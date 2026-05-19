#include "widget.h"

#include <sdli/style.h>
#include <sdli/util.h>

//
// private function declarations
//

static VNode* KeyValueListItemImpl(const char* key_name,
                                   const char* value_id,
                                   bool is_last);

//
// public function implementation
//

VNode* KeyValueListItem(const char* key_name, const char* value_id)
{
  return KeyValueListItemImpl(key_name, value_id, false);
}

VNode* KeyValueListItemLast(const char* key_name, const char* value_id)
{
  return KeyValueListItemImpl(key_name, value_id, true);
}

//
// private function implementation
//

static VNode* KeyValueListItemImpl(const char* key_name,
                                   const char* value_id,
                                   bool is_last)
{
  // clang-format off
  return Box({
    .sclass = is_last ? CLS_LIST_ITEM_LAST : CLS_LIST_ITEM,
    Children(
      Text({
        .content.text = key_name,
        .sclass = CLS_LIST_ITEM_KEY_TEXT,
      }),
      Text({
        .id = value_id,
        .sclass = CLS_LIST_ITEM_VALUE_TEXT,
      })
    )
  });
  // clang-format on
}
