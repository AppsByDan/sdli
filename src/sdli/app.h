#ifndef SDLI_APP_H
#define SDLI_APP_H

#include <stdbool.h>

#include <sdli/util.h>

//
// macros & constants
//

// sdl event dispatched before processing sdl events this frame
#define EVT_BEFORE_PROCESS_EVENTS -1
// sdl event dispatched when all sdl events for this frame have been processed
#define EVT_AFTER_PROCESS_EVENTS -2

//
// types
//

typedef void (*EventListener)(int event_type,
                              void* event_data,
                              void* user_data);

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
void App_AddEventListener(int event_type,
                          EventListener listener,
                          void* user_data);
void App_RemoveEventListener(int event_type, EventListener listener);

#endif  // SDLI_APP_H
