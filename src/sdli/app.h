#ifndef SDLI_APP_H
#define SDLI_APP_H

#include <stdbool.h>

#include <sdli/util.h>

//
// public functions
//

bool App_Init(void);
void App_Shutdown(void);

bool App_ProcessEvents(void);
void App_Present(void);

void App_RegisterScreen(const char* id,
                        NavigableCreate create,
                        NavigableEnter enter,
                        NavigableLeave leave);
void App_PushScreen(const char* from, const char* to);
void App_PopScreen(const char* from);

#endif  // SDLI_APP_H
