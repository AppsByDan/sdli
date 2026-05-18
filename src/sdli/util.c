#include <sdli/util.h>

#include <stdio.h>

//
// public function implementation
//

void BindString(const char* id, const char* value)
{
  v_node_set_text(v_get_node_by_id(id), EnsureString(value, ""));
}

void BindInt(const char* id, int value)
{
  char buffer[22];  // s64 -> 21 chars max + '\0'

  if (snprintf(&buffer[0], sizeof(buffer), "%i", value) < 1) {
    buffer[0] = '\0';
  }

  v_node_set_text(v_get_node_by_id(id), &buffer[0]);
}
