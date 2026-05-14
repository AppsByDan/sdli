#ifndef SDLI_APP_H
#define SDLI_APP_H

#include <stdbool.h>

bool App_Init(void);
void App_Shutdown(void);

bool App_ProcessEvents(void);
void App_Present(void);

#endif  // SDLI_APP_H
