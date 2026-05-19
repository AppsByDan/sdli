#include "page.h"

#include <sdli/util.h>
#include <sdli/widget/widget.h>

//
// constants
//

#define NID_PAGE_NAVIGATOR "n:pnav"

//
// external declarations
//

extern VNode* ControllerListPage(void);
extern VNode* ControllerInfoPage(void);
extern VNode* SystemPage(void);

//
// private function declarations
//

static VNode* CreatePage(const char* id);

//
// public function implementation
//

VNode* PageNavigator_Init(VNode* node)
{
  v_node_set_id(node, NID_PAGE_NAVIGATOR);
  return Navigator_Init(node, &CreatePage);
}

void PageNavigator_Goto(const char* id)
{
  VNode* navigator = v_get_node_by_id(NID_PAGE_NAVIGATOR);
  assert(navigator);

  Navigator_Goto(navigator, id);
}

//
// private function implementation
//

static VNode* CreatePage(const char* id)
{
  if (StringEq(id, PAGEID_CONTROLLER_LIST)) {
    return ControllerListPage();
  } else if (StringEq(id, PAGEID_SYSTEM)) {
    return SystemPage();
  } else if (StringEq(id, PAGEID_CONTROLLER_INFO)) {
    return ControllerInfoPage();
  }

  return NULL;
}
