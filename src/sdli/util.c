#include <sdli/util.h>

#include <stdio.h>
#include <inttypes.h>

//
// public function implementation
//

void BindString(const char* id, const char* value)
{
  v_node_set_text(v_get_node_by_id(id), EnsureString(value, ""));
}

void BindInt(const char* id, int value)
{
  v_node_set_text_fmt(v_get_node_by_id(id), "%i", value);
}

void BindU32(const char* id, uint32_t val)
{
  v_node_set_text_fmt(v_get_node_by_id(id), "%" PRIu32, val);
}

void BindU64(const char* id, uint64_t val)
{
  v_node_set_text_fmt(v_get_node_by_id(id), "%" PRIu64, val);
}

void BindBool(const char* id, bool val)
{
  v_node_set_text(v_get_node_by_id(id), val ? "YES" : "NO");
}

void BindFloat(const char* id, float val, int digits)
{
  v_node_set_text_fmt(v_get_node_by_id(id), "%.*f", digits, val);
}
