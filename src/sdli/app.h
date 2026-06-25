#ifndef SDLI_APP_H
#define SDLI_APP_H

#include <stdbool.h>

//
// macros & constants
//

/* event dispatched before processing sdl events this frame */
#define EVT_BEFORE_PROCESS_EVENTS -1
/* event dispatched when all sdl events for this frame have been processed */
#define EVT_AFTER_PROCESS_EVENTS -2
/* event dispatched when an open file dialog has a result */
#define EVT_OPEN_FILE_DIALOG_RESULT -3
/* event dispatched when a save file dialog has a result */
#define EVT_SAVE_FILE_DIALOG_RESULT -4

//
// types
//

typedef void (*EventListener)(int event_type,
                              void* event_data,
                              void* user_data);

typedef enum FileDialogResult {
  FILE_DIALOG_RESULT_SUCCESS,
  FILE_DIALOG_RESULT_CANCELLED,
  FILE_DIALOG_RESULT_ERROR,
} FileDialogResult;

typedef struct FileDialogResultEvent {
  FileDialogResult result;
  const char* filename;
} FileDialogResultEvent;

//
// public functions
//

/*
 * App provides application lifecycle management and a simple event dispatcher
 * for SDL events. The App inits the SDL, SDL_TTF, window, renderer and vuid.
 *
 * NOTES
 * - The lifecycle is similar to SDL main callbacks, but those could not be used
 *   because we need more controller over the event loop.
 * - The event dispatcher is used by the model layer to subscribe to specific
 *   SDL events.
 * - In practice, the App is used by the models and main. The UI does not
 *   interact with it directly. Not the original intent, but I am fine with it
 *   for now.
 */

bool App_Init(void);
void App_Shutdown(void);
bool App_ProcessEvents(void);
void App_Present(void);
void App_AddEventListener(int event_type,
                          EventListener listener,
                          void* user_data);
void App_RemoveEventListener(int event_type, EventListener listener);

void App_ShowOpenFileDialog(void);
void App_ShowSaveFileDialog(void);

#endif  // SDLI_APP_H
