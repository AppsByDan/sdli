#include <sdli/app.h>

#include <assert.h>

#include <sdli/style.h>
#include <sdli/vuid/vuid_sdl3.h>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vuid.h>
#include <stc/common.h>

//
// private types
//

typedef struct App {
  SDL_Window* window;
  SDL_Renderer* renderer;
  TTF_TextEngine* text_engine;
  Navigator screen_navigator;
  bool is_running;
} App;

//
// private function declarations
//

static SDL_Window* SCreateWindow(const char* title, int width, int height);
static SDL_Renderer* SCreateRenderer(SDL_Window* window);
static TTF_TextEngine* SCreateTextEngine(SDL_Renderer* renderer);

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
  assert(g_app.text_engine == NULL);

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
    SDL_Log("SDL_Init: %s", SDL_GetError());
    goto error;
  }

  g_app.window = SCreateWindow("SDL Insight", 1280, 720);

  if (!g_app.window) {
    goto error;
  }

  g_app.renderer = SCreateRenderer(g_app.window);

  if (!g_app.renderer) {
    goto error;
  }

  g_app.text_engine = SCreateTextEngine(g_app.renderer);

  if (!g_app.text_engine) {
    goto error;
  }

  const VConfig config = {
      .gfx_context.user_data =
          {
              [V_SDL3_RENDERER_ID] = g_app.renderer,
              [V_SDL3_TEXT_ENGINE_ID] = g_app.text_engine,
          },
  };

  if (!v_init(&config)) {
    goto error;
  }

  // TODO: temporary
  vs_set_background(v_node_style(v_root()), v_rgb(30, 30, 30));

  VNode* screen_layer = Box({
      .id = "screen_layer",
  });
  VStyle* screen_layer_style = v_node_style(screen_layer);

  vs_set_width(screen_layer_style, V_GROW());
  vs_set_height(screen_layer_style, V_GROW());

  v_node_append_child(v_root(), screen_layer);
  Navigator_Init(&g_app.screen_navigator, "screen_layer");

  SDL_ShowWindow(g_app.window);
  g_app.is_running = true;

  return true;

error:
  App_Shutdown();
  return false;
}

void App_Shutdown(void)
{
  Navigator_Drop(&g_app.screen_navigator);

  v_quit();

  if (g_app.text_engine) {
    TTF_DestroyRendererTextEngine(g_app.text_engine);
  }

  if (g_app.renderer) {
    SDL_DestroyRenderer(g_app.renderer);
  }

  if (g_app.window) {
    SDL_DestroyWindow(g_app.window);
  }

  TTF_Quit();
  SDL_Quit();

  g_app = (App){0};
}

bool App_ProcessEvents(void)
{
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_EVENT_QUIT:
        g_app.is_running = false;
        break;
      default:
        v_sdl3_handle_event(&event, g_app.renderer);
        break;
    }
  }

  return g_app.is_running;
}

void App_Present(void)
{
  SDL_Renderer* renderer = g_app.renderer;
  int width = 0;
  int height = 0;

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  SDL_GetWindowSize(g_app.window, &width, &height);
  v_layout(width, height);

  v_draw();

  SDL_RenderPresent(renderer);
}

void App_RegisterScreen(const char* id,
                        NavigableCreate create,
                        NavigableEnter enter,
                        NavigableLeave leave)
{
  Navigator_Register(&g_app.screen_navigator, id, create, enter, leave);
}

void App_PushScreen(const char* from, const char* to)
{
  Navigator_Push(&g_app.screen_navigator, from, to);
}

void App_PopScreen(const char* from)
{
  Navigator_Pop(&g_app.screen_navigator, from);
}

//
// private function implementation
//

static SDL_Window* SCreateWindow(const char* title, int width, int height)
{
  SDL_PropertiesID properties = SDL_CreateProperties();

  if (properties == 0) {
    SDL_Log("SDL_CreateProperties: %s", SDL_GetError());
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
    SDL_Log("CreateWindowWithProperties: %s", SDL_GetError());
  }

  SDL_DestroyProperties(properties);

  return result;
}

static SDL_Renderer* SCreateRenderer(SDL_Window* window)
{
  SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

  if (!renderer) {
    SDL_Log("SDL_CreateRenderer: %s", SDL_GetError());
    return NULL;
  }

  if (!SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE)) {
    SDL_SetRenderVSync(renderer, 1);
  }

  return renderer;
}

static TTF_TextEngine* SCreateTextEngine(SDL_Renderer* renderer)
{
  if (!TTF_Init()) {
    SDL_Log("TTF_Init: %s", SDL_GetError());
    return NULL;
  }

  TTF_TextEngine* text_engine = TTF_CreateRendererTextEngine(renderer);

  if (!text_engine) {
    SDL_Log("TTF_CreateRendererTextEngine: %s", SDL_GetError());
  }

  return text_engine;
}
