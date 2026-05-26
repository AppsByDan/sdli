#include "widget.h"

#include <sdli/util.h>

//
// private types
//

typedef struct StateData {
  const char* key;
  void* data;
  void (*free_data)(void*);
} StateData;

typedef struct State {
  ControllerId selected_controller;
  Locale locale;
  StateData data[2];
  size_t data_size;
} State;

//
// global state
//

static State g_state = {
    .locale = LOCALE_EN_US,
};

//
// public function implementation
//

void State_Init(void) {}

void State_Drop(void)
{
  for (size_t i = 0; i < g_state.data_size; i++) {
    if (g_state.data[i].free_data) {
      g_state.data[i].free_data(g_state.data[i].data);
    }
  }
  g_state.data_size = 0;
}

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

void State_SetData(const char* key, void* data, void (*free_data)(void*))
{
  // the limit is compile time
  for (size_t i = 0; i < g_state.data_size; i++) {
    if (StringEq(g_state.data[i].key, key)) {
      abort();
    }
  }

  if (g_state.data_size >= u_arraylen(g_state.data)) {
    abort();
  }

  g_state.data[g_state.data_size++] = (StateData){
      .key = key,
      .data = data,
      .free_data = free_data,
  };
}

void* State_GetData(const char* key)
{
  for (size_t i = 0; i < g_state.data_size; i++) {
    if (StringEq(g_state.data[i].key, key)) {
      return g_state.data[i].data;
    }
  }
  return NULL;
}
