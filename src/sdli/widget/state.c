#include "widget.h"

//
// private types
//

typedef struct State {
  ControllerId selected_controller;
  Locale locale;
} State;

//
// global state
//

static State g_state = {
    .selected_controller = 0,
    .locale = LOCALE_EN_US,
};

//
// public function implementation
//

ControllerId State_GetSelectedController(void)
{
  return g_state.selected_controller;
}

void State_SelectController(ControllerId id)
{
  g_state.selected_controller = id;
}

void State_ClearController(void)
{
  g_state.selected_controller = 0;
}

Locale State_GetLocale(void)
{
  return g_state.locale;
}

void State_SetLocale(Locale locale)
{
  g_state.locale = locale;
}
