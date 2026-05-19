#ifndef SDLI_SCREEN_H
#define SDLI_SCREEN_H

#include <vuid.h>

//
// macros & constants
//

#define SCREENID_HOME "s:home"
#define SCREENID_CONTROLLER_EVENTS "s:cevt"
#define SCREENID_CONTROLLER_CONFIG "s:ccfg"

//
// public functions
//

VNode* ScreenNavigator(void);
void ScreenNavigator_Goto(const char* id);

#endif  // SDLI_SCREEN_H
