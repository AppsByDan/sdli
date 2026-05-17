#include <sdli/app.h>
#include <sdli/assets.h>
#include <sdli/model/model.h>
#include <sdli/screen/screen.h>
#include <sdli/style.h>
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

  LoadStyleSheet();
  vs_set_background(v_node_style(v_root()), THEME_BACKGROUND_2);

  SystemModel_Init();
  ControllerListModel_Init();

  RegisterHomeScreen();

  App_PushScreen(NULL, SCREENID_HOME);

  while (App_ProcessEvents()) {
    App_Present();
  }

  ControllerListModel_Drop();
  SystemModel_Drop();

  App_Shutdown();

  return 0;
}
