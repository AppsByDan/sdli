#include <sdli/app.h>
#include <sdli/assets.h>
#include <sdli/util.h>

#include <SDL3/SDL.h>

#ifdef SDL_PLATFORM_LINUX
#define SDL_MAIN_NEEDED
#endif
#include <SDL3/SDL_main.h>

int main(int argc, char** argv)
{
  UNUSED(argc, argv);

  LoadAssets();

  if (!App_Init()) {
    return 1;
  }

  while (App_ProcessEvents()) {
    App_Present();
  }

  App_Shutdown();

  return 0;
}
