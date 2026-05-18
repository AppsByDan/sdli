#ifndef SDLI_PAGE_H
#define SDLI_PAGE_H

#include <vuid.h>

//
// macros & constants
//

#define PAGEID_CONTROLLER_LIST "clist_page"
#define PAGEID_SYSTEM "system_page"

//
// public functions
//

VNode* PageNavigator_Init(VNode* node);

#endif  // SDLI_PAGE_H
