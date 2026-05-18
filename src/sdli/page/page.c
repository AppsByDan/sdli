#include "page.h"

#include <sdli/util.h>
#include <sdli/widget/widget.h>

//
// external declarations
//

extern VNode* ControllerListPage(void);
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
  return Navigator_Init(node, &CreatePage);
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
  }

  return NULL;
}
