#ifndef SDLI_PAGE_H
#define SDLI_PAGE_H

#include <vuid.h>

//
// macros & constants
//

#define PAGEID_CONTROLLER_LIST "p:clist"
#define PAGEID_CONTROLLER_INFO "p:cinfo"
#define PAGEID_SYSTEM "p:system"

//
// public functions
//

VNode* PageNavigator_Init(VNode* node);
void PageNavigator_Goto(const char* id);

#endif  // SDLI_PAGE_H
