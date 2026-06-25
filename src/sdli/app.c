#include <sdli/app.h>

#include <stdlib.h>

#include <sdli/util.h>

#include <SDL3/SDL.h>
#include <stc/common.h>
#include <stc/cstr.h>

#include <vuid_sdl3.h>
#include <vuid_sdl3_renderer.h>

//
// private types
//

typedef struct EventGroupListener {
  EventListener listener;
  void* user_data;
} EventGroupListener;

#define i_no_clone
#define i_no_emplace
#define i_key EventGroupListener
#define i_type event_group_listeners
#include <stc/vec.h>

typedef struct EventGroup {
  event_group_listeners listeners;
  int dispatch_depth;
  bool needs_compaction;
} EventGroup;

static void EventGroup_drop(EventGroup* group);

#define i_no_clone
#define i_no_emplace
#define i_key int
#define i_valclass EventGroup
#define i_type event_map
#include <stc/hmap.h>

typedef struct App {
  SDL_Window* window;
  SDL_Renderer* renderer;
  event_map event_listeners;
  cstr file_dialog_filename;
  Uint32 file_dialog_sdl_event_type;
  int file_dialog_event_type;
  bool is_running;
  bool is_file_dialog_showing;
} App;

//
// private function declarations
//

static SDL_Window* SCreateWindow(const char* title, int width, int height);
static SDL_Renderer* SCreateRenderer(SDL_Window* window);
static void DispatchEvent(int event_type, void* event_data);
static void DialogFileCallback(void* userdata,
                               const char* const* filelist,
                               int filter);
static void HandleFileDialogResult(App* app, FileDialogResult result);
//
// global state
//

static App g_app = {0};

//
// public function implementation
//

bool App_Init(void)
{
  assert(g_app.window == NULL);
  assert(g_app.renderer == NULL);

  SLog("%s", FN_NAME);

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SLogCallError("SDL_Init(SDL_INIT_VIDEO)");
    goto error;
  }

  g_app.file_dialog_sdl_event_type = SDL_RegisterEvents(1);

  if (g_app.file_dialog_sdl_event_type == 0) {
    SLogCallError("SDL_RegisterEvents");
    goto error;
  }

  // allow app to continue if these subsystems fail

  if (SDL_Init(SDL_INIT_JOYSTICK)) {
    if (!SDL_Init(SDL_INIT_GAMEPAD)) {
      SLogCallError("SDL_Init(SDL_INIT_GAMEPAD)");
    }
  } else {
    SLogCallError("SDL_Init(SDL_INIT_JOYSTICK)");
  }

  if (!SDL_Init(SDL_INIT_HAPTIC)) {
    SLogCallError("SDL_Init(SDL_INIT_HAPTIC)");
  }

  g_app.window = SCreateWindow("SDL Insight", 1280, 720);

  if (!g_app.window) {
    goto error;
  }

  g_app.renderer = SCreateRenderer(g_app.window);

  if (!g_app.renderer) {
    goto error;
  }

  const VConfig config = {
      .image_loader = &v_sdl3_image_loader,
  };

  if (!v_init(&config)) {
    SLog("ERROR: v_init() failed");
    goto error;
  }

  if (!v_sdl3_init()) {
    SLog("ERROR: v_sdl3_init() failed");
    goto error;
  }

  if (!v_sdl3_renderer_init(g_app.renderer)) {
    SLog("ERROR: v_sdl3_renderer_init() failed");
    goto error;
  }

  SDL_ShowWindow(g_app.window);
  g_app.is_running = true;

  SLog("%s: success", FN_NAME);

  return true;

error:
  App_Shutdown();
  return false;
}

void App_Shutdown(void)
{
  SLog("%s", FN_NAME);

  cstr_drop(&g_app.file_dialog_filename);

  v_sdl3_renderer_shutdown();
  v_sdl3_shutdown();
  v_quit();

  if (g_app.renderer) {
    SDL_DestroyRenderer(g_app.renderer);
  }

  if (g_app.window) {
    SDL_DestroyWindow(g_app.window);
  }

  SDL_Quit();

  g_app = (App){0};
}

bool App_ProcessEvents(void)
{
  SDL_Event event;

  DispatchEvent(EVT_BEFORE_PROCESS_EVENTS, NULL);

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_EVENT_QUIT:
        g_app.is_running = false;
        break;
      default:
        v_sdl3_process_event(&event);

        if (event.type == g_app.file_dialog_sdl_event_type) {
          HandleFileDialogResult(&g_app, (FileDialogResult)event.user.code);
        } else if (event.type < UINT32_MAX) {
          DispatchEvent((int)event.type, &event);
        }
        break;
    }
  }

  DispatchEvent(EVT_AFTER_PROCESS_EVENTS, NULL);

  return g_app.is_running;
}

void App_Present(void)
{
  SDL_Renderer* renderer = g_app.renderer;
  int width = 0;
  int height = 0;

  SDL_GetRenderOutputSize(renderer, &width, &height);

  v_process_events();
  v_update(width, height);
  v_render();

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
  v_sdl3_renderer_render();

  SDL_RenderPresent(renderer);
}

void App_AddEventListener(int event_type,
                          EventListener listener,
                          void* user_data)
{
  event_map* map = &g_app.event_listeners;
  event_map_value* group = event_map_get_mut(map, event_type);

  if (!group) {
    EventGroup new_group = {.listeners =
                                event_group_listeners_with_capacity(8)};
    event_map_result insert_result =
        event_map_insert(map, event_type, new_group);

    if (!insert_result.inserted) {
      // TODO: better error handling
      abort();
    }

    group = insert_result.ref;
  }

  EventGroupListener egl = {.listener = listener, .user_data = user_data};

  event_group_listeners_push_back(&group->second.listeners, egl);
}

void App_RemoveEventListener(int event_type, EventListener listener)
{
  if (listener == NULL) {
    return;
  }

  event_map* map = &g_app.event_listeners;
  event_map_value* group = event_map_get_mut(map, event_type);

  if (!group) {
    return;
  }

  const isize len = event_group_listeners_size(&group->second.listeners);

  for (isize i = 0; i < len; i++) {
    EventGroupListener* egl =
        event_group_listeners_at_mut(&group->second.listeners, i);
    if (egl->listener == listener) {
      if (group->second.dispatch_depth > 0) {
        egl->listener = NULL;
        group->second.needs_compaction = true;
      } else {
        event_group_listeners_erase_n(&group->second.listeners, i, 1);
      }
      break;
    }
  }
}

void App_ShowOpenFileDialog(void)
{
  assert(!g_app.is_file_dialog_showing);
  g_app.is_file_dialog_showing = true;
  g_app.file_dialog_event_type = EVT_OPEN_FILE_DIALOG_RESULT;
  SDL_ShowOpenFileDialog(&DialogFileCallback, &g_app, g_app.window, NULL, 0,
                         NULL, false);
}

void App_ShowSaveFileDialog(void)
{
  assert(!g_app.is_file_dialog_showing);
  g_app.is_file_dialog_showing = true;
  g_app.file_dialog_event_type = EVT_SAVE_FILE_DIALOG_RESULT;
  SDL_ShowSaveFileDialog(&DialogFileCallback, &g_app, g_app.window, NULL, 0,
                         NULL);
}

//
// private function implementation
//

static SDL_Window* SCreateWindow(const char* title, int width, int height)
{
  SDL_PropertiesID properties = SDL_CreateProperties();

  if (properties == 0) {
    SLogCallError("SDL_CreateProperties");
    return NULL;
  }

  SDL_SetStringProperty(properties, SDL_PROP_WINDOW_CREATE_TITLE_STRING, title);
  SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
  SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER,
                        height);
  SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER,
                        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
  SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_X_NUMBER,
                        SDL_WINDOWPOS_CENTERED);
  SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_Y_NUMBER,
                        SDL_WINDOWPOS_CENTERED);

  SDL_Window* result = SDL_CreateWindowWithProperties(properties);

  if (!result) {
    SLogCallError("SDL_CreateWindowWithProperties");
  }

  SDL_DestroyProperties(properties);

  return result;
}

static SDL_Renderer* SCreateRenderer(SDL_Window* window)
{
  SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

  if (!renderer) {
    SLogCallError("SDL_CreateRenderer");
    return NULL;
  }

  if (!SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE)) {
    SDL_SetRenderVSync(renderer, 1);
  }

  return renderer;
}

static void EventGroup_drop(EventGroup* group)
{
  event_group_listeners_drop(&group->listeners);
}

// Dispatch an event to all listeners of the given type.
// - removals are safe during dispatch
// - adds are safe during dispatch, but if a new listeners is added during
//   dispatch, it will not be called until the next time the event is dispatched
static void DispatchEvent(int event_type, void* event_data)
{
  event_map* map = &g_app.event_listeners;
  event_map_value* group = event_map_get_mut(map, event_type);

  if (!group) {
    return;
  }

  const isize len = event_group_listeners_size(&group->second.listeners);

  group->second.dispatch_depth++;
  for (isize i = 0; i < len; i++) {
    EventGroupListener* egl =
        event_group_listeners_at_mut(&group->second.listeners, i);
    if (egl->listener) {
      egl->listener(event_type, event_data, egl->user_data);
    }
  }
  group->second.dispatch_depth--;

  if (group->second.dispatch_depth == 0 && group->second.needs_compaction) {
    const isize latest_len =
        event_group_listeners_size(&group->second.listeners);

    for (isize i = latest_len - 1; i >= 0; --i) {
      const EventGroupListener* egl =
          event_group_listeners_at(&group->second.listeners, i);
      if (!egl->listener) {
        event_group_listeners_erase_n(&group->second.listeners, i, 1);
      }
    }

    group->second.needs_compaction = false;
  }
}

static void DialogFileCallback(void* userdata,
                               const char* const* filelist,
                               int filter)
{
  UNUSED(filter);
  App* app = userdata;
  SDL_Event result;
  FileDialogResult code;

  if (!filelist) {
    code = FILE_DIALOG_RESULT_ERROR;
  } else if (*filelist == NULL) {
    code = FILE_DIALOG_RESULT_CANCELLED;
  } else {
    code = FILE_DIALOG_RESULT_SUCCESS;
    cstr_assign(&app->file_dialog_filename, *filelist);
  }

  result.user.type = app->file_dialog_sdl_event_type;
  result.user.code = (Sint32)code;

  SDL_PushEvent(&result);
}

static void HandleFileDialogResult(App* app, FileDialogResult result)
{
  FileDialogResultEvent app_event = {
      .filename = result == FILE_DIALOG_RESULT_SUCCESS
                      ? cstr_str(&app->file_dialog_filename)
                      : NULL,
      .result = result,
  };

  app->is_file_dialog_showing = false;
  DispatchEvent(app->file_dialog_event_type, &app_event);
}
