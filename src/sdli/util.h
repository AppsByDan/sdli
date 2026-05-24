#ifndef SDLI_UTIL_H
#define SDLI_UTIL_H

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <vuid.h>

//
// macros & constants
//

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define FN_NAME __func__
#elif defined(_MSC_VER)
#define FN_NAME __FUNCTION__
#elif defined(__GNUC__)
#define FN_NAME __FUNCTION__
#else
#define FN_NAME "unknown_function"
#endif

#define UNUSED_4(arg, ...) (void)arg
#define UNUSED_3(arg, ...) (void)arg, UNUSED_4(__VA_ARGS__, 0)
#define UNUSED_2(arg, ...) (void)arg, UNUSED_3(__VA_ARGS__, 0)
#define UNUSED_1(arg, ...) (void)arg, UNUSED_2(__VA_ARGS__, 0)
#define UNUSED(...) (UNUSED_1(__VA_ARGS__, 0))

#define SDLI_MIN(a, b) ((a) < (b) ? (a) : (b))
#define SDLI_MAX(a, b) ((a) > (b) ? (a) : (b))

#define Box v_box
#define Text v_txt
#define Image v_img
#define Children v_children

#define zv_init(STR) {STR, (isize)(sizeof("" STR) - 1)}

//
// public functions
//

void BindString(const char* id, const char* value);
void BindInt(const char* id, int value);
void BindU32(const char* id, uint32_t val);
void BindU64(const char* id, uint64_t val);
void BindBool(const char* id, bool val);
void BindFloat(const char* id, float val, int digits);

void SLog(const char* fmt, ...) VUID_GNUATTR(format(printf, 1, 2));
const char* SGetError(void);
void SLogCallError(const char* func_name);
void SLogCallErrorWithU64(const char* func_name, uint64_t value);
void SLogEvent(void* sdl_event);

// Helper to deal with SDL functions that return a NULL cstring.
static inline const char* EnsureString(const char* str, const char* default_str)
{
  return str ? str : default_str;
}

static inline bool StringEq(const char* a, const char* b)
{
  return a == b || strcmp(a, b) == 0;
}

static inline bool NodeIdEq(VNode* node, const char* id)
{
  return strcmp(v_node_id(node), id) == 0;
}

static inline void vs_set_border_all(VStyle* style, uint16_t value)
{
  vs_set_border(style, value, value, value, value);
}

static inline void vs_set_padding_all(VStyle* style, uint16_t value)
{
  vs_set_padding(style, value, value, value, value);
}

#endif  // SDLI_UTIL_H
