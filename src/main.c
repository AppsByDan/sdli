#include <sdli/app.h>
#include <sdli/assets.h>
#include <sdli/model/model.h>
#include <sdli/screen/screen.h>
#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

#include <SDL3/SDL.h>

#ifdef SDL_PLATFORM_LINUX
#define SDL_MAIN_NEEDED
#endif
#include <SDL3/SDL_main.h>

int main(int argc, char** argv)
{
  UNUSED(argc, argv);

  if (!App_Init()) {
    return 1;
  }

  SystemModel_Init();
  ControllerListModel_Init();
  ControllerInputModel_Init();

  LoadAssets();
  LoadStyleSheet();

  v_node_style_assign_class(v_root(), CLS_ROOT);
  v_node_append_child(v_root(), ScreenNavigator());

  ScreenNavigator_Goto(SCREENID_HOME);

  while (App_ProcessEvents()) {
    App_Present();
  }

  ControllerInputModel_Drop();
  ControllerListModel_Drop();
  SystemModel_Drop();

  App_Shutdown();

  return 0;
}
