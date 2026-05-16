#include <sdli/screen/screen.h>

#include <sdli/app.h>
#include <sdli/style.h>
#include <sdli/util.h>

//
// private function declarations
//

static VNode* Create(const char* id);
static void Enter(VNode* node, NavigationDirection direction);
static void Leave(VNode* node, NavigationDirection direction);

//
// public function implementation
//

void RegisterHomeScreen(void)
{
  App_RegisterScreen(SCREENID_HOME, &Create, &Enter, &Leave);
}

//
// private function implementation
//

static VNode* Create(const char* id)
{
  VNode* home_screen = Box({
      .id = id,
      .sclass = CLS_FILL,
  });

  return home_screen;
}

static void Enter(VNode* node, NavigationDirection direction)
{
  UNUSED(node, direction);
}

static void Leave(VNode* node, NavigationDirection direction)
{
  UNUSED(node, direction);
}
