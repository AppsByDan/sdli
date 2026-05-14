#include <sdli/app.h>

#include <assert.h>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

//
// private types
//

typedef struct App {
  SDL_Window* window;
  SDL_Renderer* renderer;
  TTF_TextEngine* text_engine;
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

  SDL_ShowWindow(g_app.window);
  g_app.is_running = true;

  return true;

error:
  App_Shutdown();
  return false;
}

void App_Shutdown(void)
{
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
        break;
    }
  }

  return g_app.is_running;
}

void App_Present(void)
{
  SDL_Renderer* renderer = g_app.renderer;

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  SDL_RenderPresent(renderer);
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
