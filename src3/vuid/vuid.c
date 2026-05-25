#define VUID_INTERNAL_LINKAGE static
#ifndef VUID_INTERNAL_H
#define VUID_INTERNAL_H

#include "vuid.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For non-public api, shared across multiple compilation units. Amalgamated
// builds set this to static.
#ifndef VUID_PRIVATE
#define VUID_PRIVATE
#endif  // VUID_PRIVATE

//
// util api
//

#define VUID_FLOAT_MAX_INT (1 << 24)
// Default device pixel ratio.
#define VUID_DEFAULT_DPR (1.0f)
#define V_FNV_PRIME (16777619)
#define V_FNV_OFFSET_BASIS (2166136261)

static inline int v_char_tolower(int c) {
  return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

static inline bool v_char_is_alpha(int c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline bool v_char_is_digit(int c) {
  return c >= '0' && c <= '9';
}

static inline bool v_cstr_is_empty(const char* str) {
  return !str || *str == '\0';
}

static inline uint32_t v_fnv1_hash(const char* str) {
  uint32_t hash = V_FNV_OFFSET_BASIS;

  while (*str) {
    hash ^= (uint32_t)(unsigned char)(*str++);
    hash *= V_FNV_PRIME;
  }

  return hash;
}

static inline uint32_t v_fnv1_hash_n(const uint8_t* data, size_t size) {
  uint32_t hash = V_FNV_OFFSET_BASIS;

  for (size_t i = 0; i < size; i++) {
    hash ^= (uint32_t)data[i];
    hash *= V_FNV_PRIME;
  }

  return hash;
}

static inline uint32_t v_fnv1_hash_mix(uint32_t hash1, uint32_t hash2) {
  hash1 ^= hash2;
  hash1 *= V_FNV_PRIME;
  return hash1;
}

static inline uint32_t v_fnv1_hash_u32(uint32_t value) {
  return v_fnv1_hash_n((const uint8_t*)&value, sizeof(value));
}

static inline size_t v_hset_ensure_pow_2(size_t x, size_t max_pow_2) {
  size_t v = 8;

  while (v < x) {
    if (v >= max_pow_2) {
      return max_pow_2;
    }
    v <<= 1;
  }

  return v;
}

static inline int v_clamp_int(int value, int min, int max) {
  return value < min ? min : (value > max ? max : value);
}

static inline float v_clamp_float(float value, float min, float max) {
  return value < min ? min : (value > max ? max : value);
}

static inline VRect v_rect_intersect(VRect a, VRect b) {
  const float x1 = fmaxf(a.x, b.x);
  const float y1 = fmaxf(a.y, b.y);
  float x2 = fminf(a.x + a.width, b.x + b.width);
  float y2 = fminf(a.y + a.height, b.y + b.height);
  if (x2 < x1) {
    x2 = x1;
  }
  if (y2 < y1) {
    y2 = y1;
  }
  return (VRect){x1, y1, x2 - x1, y2 - y1};
}

static inline bool v_point_in_rect(float px,
                                   float py,
                                   float rx,
                                   float ry,
                                   float rw,
                                   float rh) {
  return px >= rx && px < rx + rw && py >= ry && py < ry + rh;
}

static inline float v_snap_to_grid_dpr(float value) {
  const float dpr = VUID_DEFAULT_DPR;  // TODO: get this from context
  return roundf(value * dpr) / dpr;
}

//
// mem api
//

typedef struct VWeakNodeRef {
  VNode* ref;
  uint32_t ref_count;
} VWeakNodeRef;

//
// string api
// based on the cstr implementation in https://github.com/stclib/STC
//

#define V_STRI_S_LAST (sizeof(VStringInternal) - 1)
#define V_STRI_S_CAP (V_STRI_S_LAST - 1)
#define V_STRI_MAX_CAP ((size_t) - 1 >> 1)

typedef union VStringInternal {
  struct {
    char data[sizeof(char*) + 2 * sizeof(size_t)];
  } sml;
  struct {
    char* data;
    size_t size;
    size_t ncap;
  } lon;
} VStringInternal;

static inline size_t v_stri_s_size(const VStringInternal* self) {
  return (size_t)(self)->sml.data[V_STRI_S_LAST];
}

static inline void v_stri_s_set_size(VStringInternal* self, size_t len) {
  self->sml.data[V_STRI_S_LAST] = (char)(len);
  self->sml.data[len] = '\0';
}

static inline char* v_stri_s_data(VStringInternal* self) {
  return self->sml.data;
}

static inline const char* v_stri_s_data_const(const VStringInternal* self) {
  return self->sml.data;
}

static inline size_t v_stri_l_size(const VStringInternal* self) {
  return self->lon.size;
}

static inline void v_stri_l_set_size(VStringInternal* self, size_t len) {
  self->lon.size = len;
  self->lon.data[len] = '\0';
}

static inline char* v_stri_l_data(VStringInternal* self) {
  return self->lon.data;
}

static inline const char* v_stri_l_data_const(const VStringInternal* self) {
  return self->lon.data;
}

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define v_stri_byte_rotl(x, b) ((x) << (b) * 8 | (x) >> (sizeof(x) - (b)) * 8)
static inline size_t v_stri_l_cap(const VStringInternal* self) {
  return ~v_stri_byte_rotl(self->lon.ncap, sizeof(self->lon.ncap) - 1);
}
static inline void v_stri_l_set_cap(VStringInternal* self, size_t cap) {
  self->lon.ncap = ~v_stri_byte_rotl(cap, 1);
}
#else
static inline size_t v_stri_l_cap(const VStringInternal* self) {
  return ~(self->lon.ncap);
}
static inline void v_stri_l_set_cap(VStringInternal* self, size_t cap) {
  self->lon.ncap = ~cap;
}
#endif

static inline bool v_stri_is_long(const VStringInternal* self) {
  return ((self)->sml.data[V_STRI_S_LAST] & 128) != 0;
}

static inline VStringInternal v_stri_init(void) {
  return (VStringInternal){0};
}

static inline void v_stri_clear(VStringInternal* self) {
  v_stri_is_long(self) ? v_stri_l_set_size(self, 0)
                       : v_stri_s_set_size(self, 0);
}

static inline size_t v_stri_size(const VStringInternal* self) {
  return v_stri_is_long(self) ? v_stri_l_size(self) : v_stri_s_size(self);
}

static inline const char* v_stri_str(const VStringInternal* self) {
  return v_stri_is_long(self) ? v_stri_l_data_const(self)
                              : v_stri_s_data_const(self);
}

static inline bool v_stri_is_empty(const VStringInternal* self) {
  return v_stri_size(self) == 0;
}

// clang-format off
VUID_PRIVATE void  v_stri_drop(VStringInternal* self);
VUID_PRIVATE char* v_stri_reserve(VStringInternal* self, size_t cap);
VUID_PRIVATE void  v_stri_assign(VStringInternal* self, const char* str);
VUID_PRIVATE void  v_stri_assign_n(VStringInternal* self, const char* str, size_t size);
VUID_PRIVATE void  v_stri_assign_vfmt(VStringInternal* self, const char* fmt, va_list args);
VUID_PRIVATE bool  v_stri_eq(const VStringInternal* a, const VStringInternal* b);
VUID_PRIVATE bool  v_stri_ieq(const VStringInternal* a, const VStringInternal* b);
VUID_PRIVATE bool  v_stri_eq_cstr_n(const VStringInternal* a, const char* b, size_t b_size);
VUID_PRIVATE bool  v_stri_eq_cstr(const VStringInternal* a, const char* b);
// clang-format on

//
// array api
//

typedef struct VArray {
  void* items;
  size_t item_size;
  size_t size;
  size_t capacity;
} VArray;

// clang-format off
VUID_PRIVATE VArray v_array_init(size_t item_size, size_t initial_capacity);
VUID_PRIVATE void   v_array_drop(VArray* self);
VUID_PRIVATE bool   v_array_push(VArray* self, void* item);
VUID_PRIVATE void*  v_array_push_one(VArray* self);
VUID_PRIVATE void   v_array_pop(VArray* self);
VUID_PRIVATE void*  v_array_get_mut(VArray* self, size_t index);
VUID_PRIVATE void   v_array_remove(VArray* self, size_t index);
VUID_PRIVATE void   v_array_apply(VArray* self, void (*fn)(void* item, void* ctx), void* ctx);
static inline void  v_array_clear(VArray* self) {
  self->size = 0;
}

// clang-format on

//
// hashset api
//

#define V_HSET_GROW_THRESHOLD (90)
#define V_HSET_MAX_CAPACITY (1 << 16)
#define V_HSET_NPOS ((size_t) - 1)

#define V_HASHSET_DECLARE(HASHSET_TYPE, KEY_TYPE, VALUE_TYPE, PREFIX)         \
  typedef struct HASHSET_TYPE {                                               \
    VALUE_TYPE* items;                                                        \
    uint8_t* metadata;                                                        \
    size_t capacity;                                                          \
    size_t size;                                                              \
  } HASHSET_TYPE;                                                             \
                                                                              \
  VUID_PRIVATE HASHSET_TYPE PREFIX##_init(void);                              \
  VUID_PRIVATE HASHSET_TYPE PREFIX##_init_with_capacity(size_t capacity);     \
  VUID_PRIVATE void PREFIX##_drop(HASHSET_TYPE* self);                        \
  VUID_PRIVATE bool PREFIX##_put(HASHSET_TYPE* self, VALUE_TYPE item);        \
  VUID_PRIVATE bool PREFIX##_remove_by_value(HASHSET_TYPE* self,              \
                                             const VALUE_TYPE* item);         \
  VUID_PRIVATE bool PREFIX##_remove(HASHSET_TYPE* self, const KEY_TYPE* key); \
  VUID_PRIVATE VALUE_TYPE* PREFIX##_get(const HASHSET_TYPE* self,             \
                                        const KEY_TYPE* key)

#define V_HSET_IMPL(HASHSET_TYPE, KEY_TYPE, VALUE_TYPE, PREFIX)                \
  static bool PREFIX##_put_no_check(HASHSET_TYPE* self, VALUE_TYPE item);      \
                                                                               \
  static size_t PREFIX##_find_index(const HASHSET_TYPE* self,                  \
                                    const KEY_TYPE* key, uint32_t key_hash) {  \
    if (self->capacity == 0) {                                                 \
      return V_HSET_NPOS;                                                      \
    }                                                                          \
                                                                               \
    const size_t mask = self->capacity - 1;                                    \
    size_t i = key_hash & mask;                                                \
    uint8_t current_psl = 0;                                                   \
                                                                               \
    for (;;) {                                                                 \
      const uint8_t slot_psl_plus_one = self->metadata[i];                     \
                                                                               \
      if (slot_psl_plus_one == 0) {                                            \
        return V_HSET_NPOS;                                                    \
      }                                                                        \
                                                                               \
      const uint8_t slot_psl = slot_psl_plus_one - 1;                          \
                                                                               \
      /* Early exit: if current_psl is already greater than the stored */      \
      /* PSL, the node definitely doesn't exist because Robin Hood would */    \
      /* have swapped it here. */                                              \
      if (current_psl > slot_psl) {                                            \
        return V_HSET_NPOS;                                                    \
      }                                                                        \
                                                                               \
      if (current_psl == slot_psl) {                                           \
        const VALUE_TYPE* value = &self->items[i];                             \
        if (VALUE_TYPE##_get_hash(value) == key_hash &&                        \
            VALUE_TYPE##_eq(key, value)) {                                     \
          return i;                                                            \
        }                                                                      \
      }                                                                        \
                                                                               \
      i = (i + 1) & mask;                                                      \
      current_psl++;                                                           \
                                                                               \
      if (current_psl == 255) {                                                \
        return V_HSET_NPOS;                                                    \
      }                                                                        \
    }                                                                          \
  }                                                                            \
                                                                               \
  static void PREFIX##_backshift_delete(HASHSET_TYPE* self, size_t i) {        \
    const size_t mask = self->capacity - 1;                                    \
    self->size--;                                                              \
                                                                               \
    for (;;) {                                                                 \
      const size_t next = (i + 1) & mask;                                      \
                                                                               \
      /* Stop if next slot is empty or its element is already at its */        \
      /* ideal slot*/                                                          \
      if (self->metadata[next] <= 1) {                                         \
        self->metadata[i] = 0;                                                 \
        return;                                                                \
      }                                                                        \
                                                                               \
      /* Shift next element back one slot and decrement its PSL */             \
      self->items[i] = self->items[next];                                      \
      self->metadata[i] = self->metadata[next] - 1;                            \
      i = next;                                                                \
    }                                                                          \
  }                                                                            \
                                                                               \
  static bool PREFIX##_grow(HASHSET_TYPE* self) {                              \
    if (self->capacity == V_HSET_MAX_CAPACITY) {                               \
      return false;                                                            \
    }                                                                          \
                                                                               \
    const size_t new_capacity = self->capacity == 0 ? 8 : self->capacity << 1; \
    HASHSET_TYPE new_set = PREFIX##_init_with_capacity(new_capacity);          \
                                                                               \
    if (!new_set.items) {                                                      \
      return false;                                                            \
    }                                                                          \
                                                                               \
    for (size_t i = 0; i < self->capacity; i++) {                              \
      if (self->metadata[i] != 0) {                                            \
        self->metadata[i] = 0;                                                 \
        if (!PREFIX##_put_no_check(&new_set, self->items[i])) {                \
          PREFIX##_drop(&new_set);                                             \
          return false;                                                        \
        }                                                                      \
      }                                                                        \
    }                                                                          \
                                                                               \
    self->size = 0;                                                            \
    PREFIX##_drop(self);                                                       \
    *self = new_set;                                                           \
                                                                               \
    return true;                                                               \
  }                                                                            \
                                                                               \
  static bool PREFIX##_put_no_check(HASHSET_TYPE* self, VALUE_TYPE item) {     \
    /* note: capacity limit is 2^24, mults will not overflow */                \
    if ((self->size + 1) * 100 >= self->capacity * V_HSET_GROW_THRESHOLD) {    \
      if (!PREFIX##_grow(self)) {                                              \
        VALUE_TYPE##_drop(&item);                                              \
        return false;                                                          \
      }                                                                        \
    }                                                                          \
                                                                               \
    const uint32_t item_hash = VALUE_TYPE##_get_hash(&item);                   \
    const size_t mask = self->capacity - 1;                                    \
    size_t i = item_hash & mask;                                               \
    uint8_t current_psl = 0;                                                   \
    VALUE_TYPE current_item = item;                                            \
                                                                               \
    for (;;) {                                                                 \
      if (self->metadata[i] == 0) {                                            \
        self->items[i] = current_item;                                         \
        self->metadata[i] = current_psl + 1;                                   \
        self->size++;                                                          \
        return true;                                                           \
      }                                                                        \
                                                                               \
      const uint8_t slot_psl = self->metadata[i] - 1;                          \
                                                                               \
      if (current_psl > slot_psl) {                                            \
        /* Swap rich item for poor item */                                     \
        VALUE_TYPE tmp_item = self->items[i];                                  \
        const uint8_t tmp_psl = slot_psl;                                      \
                                                                               \
        self->items[i] = current_item;                                         \
        self->metadata[i] = current_psl + 1;                                   \
                                                                               \
        current_item = tmp_item;                                               \
        current_psl = tmp_psl;                                                 \
      }                                                                        \
                                                                               \
      i = (i + 1) & mask;                                                      \
      current_psl++;                                                           \
                                                                               \
      /* Safety check to prevent infinite loop. Some robinhood */              \
      /* implementations throw an exception here, others grow and other */     \
      /* rehash with a different seed. We will reject the add, similar to */   \
      /* throwing an exception. */                                             \
      if (current_psl == 255) {                                                \
        VALUE_TYPE##_drop(&current_item);                                      \
        return false;                                                          \
      }                                                                        \
    }                                                                          \
  }                                                                            \
                                                                               \
  VUID_PRIVATE HASHSET_TYPE PREFIX##_init(void) {                              \
    return (HASHSET_TYPE){0};                                                  \
  }                                                                            \
                                                                               \
  VUID_PRIVATE HASHSET_TYPE PREFIX##_init_with_capacity(size_t capacity) {     \
    const size_t real_capacity =                                               \
        v_hset_ensure_pow_2(capacity, V_HSET_MAX_CAPACITY);                    \
    const size_t items_size = real_capacity * sizeof(VALUE_TYPE);              \
    const size_t metadata_size = real_capacity * sizeof(uint8_t);              \
    VALUE_TYPE* chunk =                                                        \
        v_ctx_alloc(V_AOP_CALLOC, NULL, 0, items_size + metadata_size);        \
                                                                               \
    if (!chunk) {                                                              \
      return (HASHSET_TYPE){0};                                                \
    }                                                                          \
                                                                               \
    return (HASHSET_TYPE){                                                     \
        .items = chunk,                                                        \
        .metadata = (uint8_t*)chunk + items_size,                              \
        .capacity = real_capacity,                                             \
    };                                                                         \
  }                                                                            \
                                                                               \
  VUID_PRIVATE void PREFIX##_drop(HASHSET_TYPE* self) {                        \
    if (self->items) {                                                         \
      const size_t capacity = self->capacity;                                  \
      const size_t items_size = capacity * sizeof(VALUE_TYPE);                 \
      const size_t metadata_size = capacity * sizeof(uint8_t);                 \
                                                                               \
      if (self->size > 0) {                                                    \
        for (size_t i = 0; i < capacity; i++) {                                \
          if (self->metadata[i] != 0) {                                        \
            VALUE_TYPE##_drop(&self->items[i]);                                \
          }                                                                    \
        }                                                                      \
      }                                                                        \
                                                                               \
      v_ctx_alloc(V_AOP_FREE, self->items, items_size + metadata_size, 0);     \
    }                                                                          \
  }                                                                            \
                                                                               \
  VUID_PRIVATE bool PREFIX##_put(HASHSET_TYPE* self, VALUE_TYPE item) {        \
    const KEY_TYPE item_key = KEY_TYPE##_from_value(&item);                    \
    const size_t existing =                                                    \
        PREFIX##_find_index(self, &item_key, VALUE_TYPE##_get_hash(&item));    \
                                                                               \
    if (existing != V_HSET_NPOS) {                                             \
      VALUE_TYPE##_drop(&item);                                                \
      return false;                                                            \
    }                                                                          \
                                                                               \
    return PREFIX##_put_no_check(self, item);                                  \
  }                                                                            \
                                                                               \
  VUID_PRIVATE VALUE_TYPE* PREFIX##_get(const HASHSET_TYPE* self,              \
                                        const KEY_TYPE* key) {                 \
    const size_t index =                                                       \
        PREFIX##_find_index(self, key, KEY_TYPE##_get_hash(key));              \
                                                                               \
    return (index != V_HSET_NPOS) ? &self->items[index] : NULL;                \
  }                                                                            \
                                                                               \
  VUID_PRIVATE bool PREFIX##_remove_by_value(HASHSET_TYPE* self,               \
                                             const VALUE_TYPE* item) {         \
    const KEY_TYPE key = KEY_TYPE##_from_value(item);                          \
    const size_t i =                                                           \
        PREFIX##_find_index(self, &key, VALUE_TYPE##_get_hash(item));          \
                                                                               \
    if (i == V_HSET_NPOS) {                                                    \
      return false;                                                            \
    }                                                                          \
                                                                               \
    VALUE_TYPE##_drop(&self->items[i]);                                        \
    PREFIX##_backshift_delete(self, i);                                        \
                                                                               \
    return true;                                                               \
  }                                                                            \
                                                                               \
  VUID_PRIVATE bool PREFIX##_remove(HASHSET_TYPE* self, const KEY_TYPE* key) { \
    const size_t i = PREFIX##_find_index(self, key, KEY_TYPE##_get_hash(key)); \
                                                                               \
    if (i == V_HSET_NPOS) {                                                    \
      return false;                                                            \
    }                                                                          \
                                                                               \
    VALUE_TYPE##_drop(&self->items[i]);                                        \
    PREFIX##_backshift_delete(self, i);                                        \
                                                                               \
    return true;                                                               \
  }

//
// resource api
//

typedef struct VResourceSub VResourceSub;

struct VResourceSub {
  VNode* node;
  VResourceSub* next;
  VResourceSub* prev;
};

typedef struct VResourceSubList {
  VResourceSub* active;
  uint32_t active_count;
  uint32_t pending_removes;
  uint32_t dispatch_depth;
} VResourceSubList;

typedef enum VResourceEvent {
  V_RESOURCE_EVENT_REMOVE,
} VResourceEvent;

typedef enum VResourceState {
  V_RESOURCE_STATE_INIT,
  V_RESOURCE_STATE_READY,
  V_RESOURCE_STATE_ERROR,
} VResourceState;

typedef enum VResourceType {
  V_RESOURCE_TYPE_IMAGE,
  V_RESOURCE_TYPE_FONT,
} VResourceType;

typedef struct VResource {
  uint32_t refs;
  uint32_t hash;
  VStringInternal id;
  VResourceSubList subscribers;
  // TODO: combine?
  VResourceType type;
  VResourceState state;
  bool is_persistent;
  bool has_pending_remove;

  union {
    struct {
      VGfxTexture* gfx_texture;
      VSize dimensions;
      bool owns_gfx_texture;
    } image;
    struct {
      VGfxFont* gfx_font;
      VArray font_faces;
      uint16_t font_id;
      bool owns_gfx_font;
    } font;
  } u;
} VResource;

// clang-format off
VUID_PRIVATE VResource*    v_resource__acquire_image(const char* src);
VUID_PRIVATE VResource*    v_resource__acquire_font(uint16_t id);
VUID_PRIVATE VResource*    v_resource__peek_font(uint16_t id);
VUID_PRIVATE void          v_resource__release(VResource* resource);
VUID_PRIVATE VGfxFontFace* v_resource__get_or_create_font_face(VResource* font, uint16_t font_size);
VUID_PRIVATE VResourceSub* v_resource__subscribe(VResource* resource, VNode* node);
VUID_PRIVATE void          v_resource__unsubscribe(VResourceSub* node_slot);
// clang-format on

typedef struct VResourcesValue {
  VResource* resource;
} VResourcesValue;

typedef struct VResourcesKey {
  VResourceType type;
  const char* id;
} VResourcesKey;

V_HASHSET_DECLARE(VResources, VResourcesKey, VResourcesValue, v_resources);

//
// style api
//

typedef enum VStyleProperty {
  VS_WIDTH = 0,
  VS_HEIGHT,
  VS_DIRECTION,
  VS_WRAP,
  VS_XALIGN,
  VS_YALIGN,
  VS_TALIGN,
  VS_TEXT_WRAP,
  VS_OVERFLOW,
  VS_GAP,
  VS_PADDING_TOP,
  VS_PADDING_RIGHT,
  VS_PADDING_BOTTOM,
  VS_PADDING_LEFT,
  VS_BORDER_TOP,
  VS_BORDER_RIGHT,
  VS_BORDER_BOTTOM,
  VS_BORDER_LEFT,
  VS_BORDER_RADIUS,
  VS_FONT,
  VS_FONT_SIZE,
  VS_SCROLLBAR_WIDTH,
  VS_SCROLLBAR_BORDER_RADIUS,
  VS_BACKGROUND,
  VS_COLOR,
  VS_BORDER_COLOR,
  VS_SCROLLBAR_THUMB,
  VS_SCROLLBAR_THUMB_HOVER,
  VS_ANCHOR_TO,
  VS_ANCHOR_ATTACH_POINT_X,
  VS_ANCHOR_ATTACH_POINT_Y,
  VS_ATTACH_POINT_X,
  VS_ATTACH_POINT_Y,
  VS_ATTACH_POINT_OFFSET_X,
  VS_ATTACH_POINT_OFFSET_Y,
  VS_ASPECT_RATIO,
  VS_POSITION,
  VS_TOP,
  VS_RIGHT,
  VS_BOTTOM,
  VS_LEFT,
  VS__STYLE_PROPERTY_COUNT,
} VStyleProperty;

struct VStyle {
  VSizing width;
  VSizing height;
  VColor background;
  VColor color;
  VColor border_color;
  VColor scrollbar_thumb;
  VColor scrollbar_thumb_hover;
  VDirection direction;
  VWrap wrap;
  VAlignX xalign;
  VAlignY yalign;
  VAlignX talign;
  VTextWrap text_wrap;
  VAnchorTo anchor_to;
  VAttachPointX anchor_attach_point_x;
  VAttachPointY anchor_attach_point_y;
  VAttachPointX attach_point_x;
  VAttachPointY attach_point_y;
  VOverflow overflow;
  VPosition position;
  uint16_t font;
  uint16_t font_size;
  uint16_t gap;
  uint16_t padding_top;
  uint16_t padding_right;
  uint16_t padding_bottom;
  uint16_t padding_left;
  uint16_t border_top;
  uint16_t border_right;
  uint16_t border_bottom;
  uint16_t border_left;
  uint16_t border_radius;
  uint16_t scrollbar_width;
  uint16_t scrollbar_border_radius;
  float top;
  float right;
  float bottom;
  float left;
  float attach_point_offset_x;
  float attach_point_offset_y;
  float aspect_ratio;
  uint32_t ref_count;

  VNode* owner;
  uint64_t is_set;
};

// clang-format off
VUID_PRIVATE VStyle* v_style__new(VNode* owner);
VUID_PRIVATE void    v_style__flatten(VStyle* a, VStyle* b);
// clang-format on

static inline bool vs_has_prop(const VStyle* style, VStyleProperty property) {
  return (style->is_set & (((uint64_t)1) << property)) != 0;
}

// TODO: these should probably return floats

#define vs_get_pt vs_get_padding_top
#define vs_get_pr vs_get_padding_right
#define vs_get_pb vs_get_padding_bottom
#define vs_get_pl vs_get_padding_left

#define vs_get_bt vs_get_border_top
#define vs_get_br vs_get_border_right
#define vs_get_bb vs_get_border_bottom
#define vs_get_bl vs_get_border_left

static inline VSizingTag vs__get_width_tag(const VStyle* style) {
  return vs_has_prop(style, VS_WIDTH) ? style->width.tag : V_SIZING_FIT;
}

static inline VSizingTag vs__get_height_tag(const VStyle* style) {
  return vs_has_prop(style, VS_HEIGHT) ? style->height.tag : V_SIZING_FIT;
}

//
// node api
//

typedef enum VNodeFlag {
  V_NODEFLAG_NONE = 0,
  V_NODEFLAG_HOVERED = 1 << 0,
  V_NODEFLAG_DIRTY = 1 << 1,
  V_NODEFLAG_HIDDEN = 1 << 2,
  V_NODEFLAG_POPOVER_OPEN = 1 << 3,
  V_NODEFLAG_ATTACHED = 1 << 4,
  V_NODEFLAG_LEAF = 1 << 5,
  V_NODEFLAG_HAS_FONT = 1 << 6,
} VNodeFlag;

struct VNode {
  // TODO: pack these into a single field
  VNodeTag tag;
  VPopover popover_type;
  uint32_t ref_count;
  uint32_t flags;

  VStringInternal id;
  uint32_t id_hash;

  VWeakNodeRef* parent;
  VNode* first_child;
  VNode* last_child;
  VNode* next_sibling;
  VNode* prev_sibling;
  int child_count;

  VEventListener event_listeners[V_EVENT__COUNT];

  VRect bounds;
  float min_width;
  float pref_width;
  float min_height;
  float pref_height;
  VStyle* style;

  float scroll_y;
  float content_height;

  VResource* font;
  VResourceSub* font_sub;
  VGfxFontFace* gfx_font_face;
  float font_size;

  union {
    VGfxTextMesh* gfx_text_mesh;
    VResource* image_resource;
  } res;

  union {
    VStringInternal text;
    VStringInternal src;
  } res_data;

  void* user_data;
  VWeakNodeRef* self_weak_ref;
};

// clang-format off
VUID_PRIVATE VNode*        v_node__constructor(VNodeTag tag);
VUID_PRIVATE void          v_node__destructor(VNode* node);
VUID_PRIVATE const VStyle* v_node__style_or_empty(const VNode* node);
VUID_PRIVATE VStyle*       v_node__style_mut(VNode* node);
VUID_PRIVATE void          v_node__set_attached(VNode* node, bool attached);
VUID_PRIVATE float         v_node__get_max_scroll(const VNode* node);
VUID_PRIVATE void          v_node__get_scrollbar_rect(VNode* node, float abs_x, float abs_y, VRect* track, VRect* thumb);
VUID_PRIVATE bool          v_node__is_descendant_of(const VNode* node, const VNode* ancestor);
VUID_PRIVATE bool          v_node__sync_font(VNode* node);
VUID_PRIVATE void          v_node__clear_font(VNode* node);
VUID_PRIVATE void          v_node__clear_font_face(VNode* node);
VUID_PRIVATE void          v_node__on_resource_event(VNode* node, VResourceEvent event, VResource* resource);
// clang-format on

static inline void v_node__set_flag(VNode* node, VNodeFlag flag) {
  node->flags |= flag;
}

static inline void v_node__clear_flag(VNode* node, VNodeFlag flag) {
  node->flags &= ~flag;
}

static inline bool v_node__has_flag(const VNode* node, VNodeFlag flag) {
  return (node->flags & flag) != 0;
}

static inline bool v_node__has_all_flags(const VNode* node, uint32_t flags) {
  return (node->flags & flags) == flags;
}

//
// style sheet api
//

typedef struct VStyleClass {
  VStringInternal id;
  VStyle* style;
  uint32_t id_hash;
  // TODO: move this flag into style
  bool is_ready;
} VStyleClass;

typedef struct VStyleClassHashSetKey {
  const char* id;
} VStyleClassHashSetKey;

VUID_PRIVATE void v_style_class__drop(VStyleClass* style_class);

V_HASHSET_DECLARE(VStyleClassHashSet,
                  VStyleClassHashSetKey,
                  VStyleClass,
                  v_style_class_hset);

//
// context api
//

typedef enum VAllocatorOp {
  V_AOP_ALLOC,
  V_AOP_CALLOC,
  V_AOP_REALLOC,
  V_AOP_FREE,
} VAllocatorOp;

typedef struct VNodeIdHashSetValue {
  VNode* node;
} VNodeIdHashSetValue;

typedef struct VNodeIdHashSetKey {
  const char* id;
} VNodeIdHashSetKey;

static inline VNodeIdHashSetValue VNodeIdHashSetValue_init(VNode* node) {
  return (VNodeIdHashSetValue){.node = node};
}

static inline VNodeIdHashSetKey VNodeIdHashSetKey_init(const char* id) {
  return (VNodeIdHashSetKey){.id = id};
}

V_HASHSET_DECLARE(VNodeIdHashSet,
                  VNodeIdHashSetKey,
                  VNodeIdHashSetValue,
                  v_node_id_hset);

typedef struct VContext {
  VNode* root;
  VNode* hovered_node;
  VNode* active_node;
  VNode* drag_node;
  float drag_start_y;
  float drag_start_scroll_y;
  VGfxContext gfx_context;

  uint32_t gfx_features;

  VStyle* empty_style;

  uint16_t scrollbar_width_default;
  VColor scrollbar_thumb_default;

  int root_width;
  int root_height;

  VArray popover_stack;
  VStyle* top_layer_style;
  VNodeIdHashSet nodes_by_id;
  VStyleClassHashSet style_classes_by_id;
  VResources resources;
  VArray event_path;

  int node_count;
  int style_count;
  int weak_ref_count;
  int resource_count;
} VContext;

// clang-format off
VUID_PRIVATE VContext*    v_ctx(void);
VUID_PRIVATE VGfxContext* v_gfx(void);
VUID_PRIVATE void*        v_ctx_alloc(VAllocatorOp op, void* ptr, size_t old_size, size_t new_size);
VUID_PRIVATE void         v_ctx__remove_input_node(VNode* node);
VUID_PRIVATE void         v_ctx__remove_popover_node(VNode* node);
VUID_PRIVATE bool         v_ctx__has_gfx_feature(VGfxFeature feature);
VUID_PRIVATE VArray*      v_ctx__get_event_path(void);
VUID_PRIVATE void         v_ctx__clear_event_path(void);
// clang-format on

#define v_ctx_new(TYPE) v_ctx_alloc(V_AOP_CALLOC, NULL, 0, sizeof(TYPE))
#define v_ctx_delete(PTR, TYPE) v_ctx_alloc(V_AOP_FREE, PTR, sizeof(TYPE), 0)

#endif  // VUID_INTERNAL_H

VUID_PRIVATE VArray v_array_init(size_t item_size, size_t initial_capacity) {
  VArray self;

  self.item_size = item_size;
  self.size = 0;
  self.capacity = initial_capacity;
  if (initial_capacity > 0) {
    // TODO: ??
    self.items =
        v_ctx_alloc(V_AOP_ALLOC, NULL, 0, item_size * initial_capacity);
  } else {
    self.items = NULL;
  }

  return self;
}

VUID_PRIVATE void v_array_drop(VArray* self) {
  if (self->items) {
    v_ctx_alloc(V_AOP_FREE, self->items, self->capacity * self->item_size, 0);
  }
}

VUID_PRIVATE bool v_array_push(VArray* self, void* item) {
  void* target = v_array_push_one(self);

  if (!target) {
    return false;
  }

  memcpy(target, item, self->item_size);

  return true;
}

VUID_PRIVATE void* v_array_push_one(VArray* self) {
  if (self->size >= self->capacity) {
    // TODO: what should the default be?
    // TODO: overflow
    const size_t new_capacity = self->capacity == 0 ? 8 : self->capacity * 2;
    void* new_items = v_ctx_alloc(V_AOP_REALLOC, self->items,
                                  self->item_size * self->capacity,
                                  self->item_size * new_capacity);

    if (!new_items) {
      // TODO: ??
      return NULL;
    }

    self->items = new_items;
    self->capacity = new_capacity;
  }

  void* item = (uint8_t*)(self->items) + self->size * self->item_size;

  self->size++;

  return item;
}

VUID_PRIVATE void v_array_pop(VArray* self) {
  if (self->size == 0) {
    return;
  }

  v_array_remove(self, self->size - 1);
}

VUID_PRIVATE void* v_array_get_mut(VArray* array, size_t index) {
  return index < array->size
             ? ((uint8_t*)(array->items) + index * array->item_size)
             : NULL;
}

VUID_PRIVATE void v_array_remove(VArray* self, size_t index) {
  if (index >= self->size) {
    return;
  }

  if (index == self->size - 1) {
    self->size--;
    return;
  }

  uint8_t* items = (uint8_t*)self->items;

  memmove(items + index * self->item_size,
          items + (index + 1) * self->item_size,
          (self->size - index - 1) * self->item_size);

  self->size--;
}

VUID_PRIVATE void v_array_apply(VArray* self,
                                void (*fn)(void* item, void* ctx),
                                void* ctx) {
  uint8_t* items = (uint8_t*)self->items;

  for (size_t i = 0; i < self->size; i++) {
    fn(items + i * self->item_size, ctx);
  }
}

static VContext g_context = {0};

VNode* v_root(void) {
  return g_context.root;
}

bool v_init(const VConfig* config) {
  g_context = (VContext){
      // TODO: defaults
      // background = (VColor){0, 0, 0, 255};
      // width = (VSizing){V_SIZING_GROW, 0, 0};
      // height = (VSizing){V_SIZING_GROW, 0, 0};
      .scrollbar_thumb_default = v_rgba(255, 255, 255, 255),
      .scrollbar_width_default = 20,
  };

  g_context.root = v_node__constructor(V_NODE_ROOT);
  g_context.empty_style = v_style__new(NULL);
  g_context.top_layer_style = v_style__new(NULL);

  if (!g_context.root || !g_context.empty_style || !g_context.top_layer_style) {
    goto error;
  }

  VStyle* root_style = v_node_style(g_context.root);

  if (!root_style) {
    goto error;
  }

  vs_set_width(root_style, V_FIXED(0));
  vs_set_height(root_style, V_FIXED(0));

  v_node__set_flag(g_context.root, V_NODEFLAG_ATTACHED);

  // TODO: are these initial capacities reasonable?
  // TODO: what if internal allocations fail?
  g_context.popover_stack = v_array_init(sizeof(VNode*), 8);
  g_context.event_path = v_array_init(sizeof(VNode*), 32);
  g_context.nodes_by_id = v_node_id_hset_init_with_capacity(32);
  g_context.style_classes_by_id = v_style_class_hset_init_with_capacity(32);
  g_context.resources = v_resources_init_with_capacity(32);

  if (config) {
    g_context.gfx_context = config->gfx_context;
  }

  g_context.gfx_features = v_gfx_features(&g_context.gfx_context);

  return true;

error:
  v_quit();
  return false;
}

void v_quit(void) {
  if (g_context.root) {
    v_node__set_attached(g_context.root, false);
    v_node_unref(g_context.root);
  }

  assert(g_context.nodes_by_id.size == 0);
  assert(g_context.popover_stack.size == 0);
  assert(g_context.event_path.size == 0);
  assert(g_context.active_node == NULL);
  assert(g_context.hovered_node == NULL);
  assert(g_context.drag_node == NULL);
  // TODO: audit resources

  v_node_id_hset_drop(&g_context.nodes_by_id);
  v_array_drop(&g_context.popover_stack);
  v_array_drop(&g_context.event_path);
  v_resources_drop(&g_context.resources);
  v_style_class_hset_drop(&g_context.style_classes_by_id);

  if (g_context.empty_style) {
    v_style_unref(g_context.empty_style);
  }

  if (g_context.top_layer_style) {
    v_style_unref(g_context.top_layer_style);
  }

  assert(g_context.node_count == 0);
  assert(g_context.style_count == 0);
  assert(g_context.weak_ref_count == 0);
  assert(g_context.resource_count == 0);

  g_context = (VContext){0};
}

VStyle* v_get_popover_layer_style(void) {
  return v_ctx()->top_layer_style;
}

VNode* v_get_node_by_id(const char* id) {
  if (!id || *id == '\0') {
    return NULL;
  }

  const VNodeIdHashSetKey key = VNodeIdHashSetKey_init(id);
  VNodeIdHashSetValue* value = v_node_id_hset_get(&g_context.nodes_by_id, &key);

  return value ? value->node : NULL;
}

VNode* v_get_node_by_id_fmt(const char* fmt, ...) {
  VStringInternal id = v_stri_init();

  va_list args;
  va_start(args, fmt);
  v_stri_assign_vfmt(&id, fmt, args);
  va_end(args);

  VNode* result = v_get_node_by_id(v_stri_str(&id));

  v_stri_drop(&id);

  return result;
}

VUID_PRIVATE VGfxContext* v_gfx(void) {
  return &g_context.gfx_context;
}

VUID_PRIVATE VContext* v_ctx(void) {
  return &g_context;
}

VUID_PRIVATE bool v_ctx__has_gfx_feature(VGfxFeature feature) {
  return (g_context.gfx_features & feature) != 0;
}

VUID_PRIVATE VArray* v_ctx__get_event_path(void) {
  return &g_context.event_path;
}

VUID_PRIVATE void v_ctx__clear_event_path(void) {
  VNode** items = g_context.event_path.items;
  const size_t size = g_context.event_path.size;

  for (size_t i = 0; i < size; i++) {
    v_node_unref(items[i]);
  }

  v_array_clear(&g_context.event_path);
}

/*
 * Remove any input interest related references to this node.
 * - caller is responsible for node state cleanup
 */
VUID_PRIVATE void v_ctx__remove_input_node(VNode* node) {
  if (g_context.hovered_node == node) {
    g_context.hovered_node = NULL;
  }

  if (g_context.active_node == node) {
    g_context.active_node = NULL;
  }

  if (g_context.drag_node == node) {
    g_context.drag_node = NULL;
  }
}

/*
 * Remove unowned node reference from popover stack.
 * - caller is responsible for node state cleanup
 */
VUID_PRIVATE void v_ctx__remove_popover_node(VNode* node) {
  for (size_t i = 0; i < g_context.popover_stack.size; i++) {
    if (((VNode**)g_context.popover_stack.items)[i] == node) {
      v_array_remove(&g_context.popover_stack, i);
      break;
    }
  }
}

VUID_PRIVATE void* v_ctx_alloc(VAllocatorOp op,
                               void* ptr,
                               size_t old_size,
                               size_t new_size) {
  (void)old_size;
  switch (op) {
    case V_AOP_ALLOC:
      return malloc(new_size);
    case V_AOP_CALLOC:
      return calloc(1, new_size);
    case V_AOP_REALLOC:
      return realloc(ptr, new_size);
    case V_AOP_FREE:
      free(ptr);
      return NULL;
    default:
      return NULL;
  }
}

static void v_node_draw_recursive(VNode* node,
                                  float abs_x,
                                  float abs_y,
                                  VRect current_clip);
static bool v_is_text_ready_for_draw(const VNode* text_node,
                                     const VStyle* text_node_style);

void v_draw(void) {
  VContext* ctx = v_ctx();
  const VRect initial_clip = {0, 0, 1e9f, 1e9f};

  v_node_draw_recursive(v_root(), 0, 0, initial_clip);

  if (ctx->popover_stack.size > 0) {
    // popover top layer
    if (vs_has_prop(ctx->top_layer_style, VS_BACKGROUND)) {
      const VColor top_layer_color = vs_get_background(ctx->top_layer_style);
      if (top_layer_color.a > 0) {
        // root bounds is absolute and does not change outside of layout.
        const VNode* root = v_ctx()->root;
        v_gfx_draw_fill_rect(v_gfx(), &root->bounds, &top_layer_color, 0);
      }
    }

    for (size_t i = 0; i < ctx->popover_stack.size; i++) {
      // popover bounds are absolute
      v_node_draw_recursive(((VNode**)ctx->popover_stack.items)[i], 0, 0,
                            initial_clip);
    }
  }
}

static void v_node_draw_recursive(VNode* node,
                                  float abs_x,
                                  float abs_y,
                                  VRect current_clip) {
  static const VColor VUID_WHITE = {255, 255, 255, 255};
  static const VColor DEFAULT_CLEAR = {0};

  if (!v_node_is_visible(node)) {
    return;
  }

  const float current_abs_x = abs_x + node->bounds.x;
  const float current_abs_y = abs_y + node->bounds.y;
  const VRect background = {
      current_abs_x,
      current_abs_y,
      node->bounds.width,
      node->bounds.height,
  };
  const VStyle* style = v_node__style_or_empty(node);
  const VOverflow node_overflow = vs_get_overflow(style);
  const float node_pt = vs_get_pt(style);
  const float node_pr = vs_get_pr(style);
  const float node_pb = vs_get_pb(style);
  const float node_pl = vs_get_pl(style);
  const bool has_background_color = vs_has_prop(style, VS_BACKGROUND);
  const VGfxContext* gfx = v_gfx();
  float border_radius = vs_get_border_radius(style);

  if (border_radius > 0) {
    const float half_width = node->bounds.width / 2.0f;
    const float half_height = node->bounds.height / 2.0f;

    if (border_radius > half_width) {
      border_radius = half_width;
    }

    if (border_radius > half_height) {
      border_radius = half_height;
    }
  }

  if (node->tag == V_NODE_ROOT) {
    v_gfx_clear(gfx, &background,
                has_background_color ? &style->background : &DEFAULT_CLEAR);
  } else if (has_background_color) {
    v_gfx_draw_fill_rect(gfx, &background, &style->background, border_radius);
  }

  if (vs_has_prop(style, VS_BORDER_COLOR)) {
    const uint16_t bt = vs_get_bt(style);
    const uint16_t br = vs_get_br(style);
    const uint16_t bb = vs_get_bb(style);
    const uint16_t bl = vs_get_bl(style);

    v_gfx_draw_rect(gfx, &background, &style->border_color, bt, br, bb, bl,
                    border_radius);
  }

  switch (node->tag) {
    case V_NODE_TEXT: {
      if (v_is_text_ready_for_draw(node, style)) {
        const float bl = (float)vs_get_bl(style);
        const float br = (float)vs_get_br(style);
        const float bt = (float)vs_get_bt(style);
        const float bb = (float)vs_get_bb(style);
        const VRect content_box = {
            background.x + node_pl + bl,
            background.y + node_pt + bt,
            // TODO: want to match wrap measure in layout
            v_snap_to_grid_dpr(node->bounds.width) -
                (node_pl + node_pr + bl + br),
            v_snap_to_grid_dpr(node->bounds.height) -
                (node_pt + node_pb + bt + bb),
        };

        if (content_box.width > 0 && content_box.height > 0) {
          const VAlignX talign = vs_get_talign(style);
          const char* str = v_stri_str(&node->res_data.text);
          const bool wrap = vs_get_text_wrap(style) == V_TEXT_WRAP_WRAP;

          v_gfx_draw_text(gfx, node->res.gfx_text_mesh, str, &content_box, wrap,
                          talign, node->font->u.font.gfx_font,
                          (float)node->font_size, node->gfx_font_face,
                          &style->color);
        }
      }
      break;
    }
    case V_NODE_IMAGE: {
      VResource* image_resource = node->res.image_resource;

      if (image_resource && image_resource->state == V_RESOURCE_STATE_READY) {
        // TODO: not sure about this policy choice. border over image
        // configurable?
        const float node_bt = vs_get_bt(style);
        const float node_br = vs_get_br(style);
        const float node_bb = vs_get_bb(style);
        const float node_bl = vs_get_bl(style);
        const VRect image_rect = {
            background.x + node_pl + node_bl,
            background.y + node_pt + node_bt,
            node->bounds.width - (node_pl + node_pr + node_bl + node_br),
            node->bounds.height - (node_pt + node_pb + node_bt + node_bb),
        };

        const VColor* color =
            vs_has_prop(style, VS_COLOR) ? &style->color : &VUID_WHITE;

        v_gfx_draw_image(gfx, &image_rect, image_resource->u.image.gfx_texture,
                         color);
      }
      break;
    }
    default:
      break;
  }

  VNode* child = node->first_child;
  VRect next_clip = current_clip;
  bool clip_pushed = false;

  if (node_overflow == V_OVERFLOW_HIDDEN ||
      node_overflow == V_OVERFLOW_SCROLL) {
    const float node_bt = vs_get_bt(style);
    const float node_br = vs_get_br(style);
    const float node_bb = vs_get_bb(style);
    const float node_bl = vs_get_bl(style);
    const VRect inner_rect = {
        current_abs_x + node_pl + node_bl,
        current_abs_y + node_pt + node_bt,
        node->bounds.width - (node_pl + node_pr + node_bl + node_br),
        node->bounds.height - (node_pt + node_pb + node_bt + node_bb),
    };

    next_clip = v_rect_intersect(current_clip, inner_rect);
    v_gfx_set_clip(gfx, next_clip.x, next_clip.y, next_clip.width,
                   next_clip.height);
    clip_pushed = true;
  }

  while (child) {
    if (child->popover_type == V_POPOVER_NONE) {
      v_node_draw_recursive(child, current_abs_x,
                            current_abs_y - node->scroll_y, next_clip);
    }
    child = child->next_sibling;
  }

  if (clip_pushed) {
    v_gfx_set_clip(gfx, current_clip.x, current_clip.y, current_clip.width,
                   current_clip.height);
  }

  if (node_overflow == V_OVERFLOW_SCROLL &&
      node->content_height >
          node->bounds.height -
              (node_pt + node_pb + vs_get_bt(style) + vs_get_bb(style))) {
    VRect track;
    VRect thumb;

    v_node__get_scrollbar_rect(node, current_abs_x, current_abs_y, &track,
                               &thumb);

    const VColor scrollbar_thumb = vs_has_prop(style, VS_SCROLLBAR_THUMB)
                                       ? style->scrollbar_thumb
                                       : v_ctx()->scrollbar_thumb_default;

    // TODO: draw track?
    // TODO: hovered thumb check
    v_gfx_draw_fill_rect(gfx, &thumb, &scrollbar_thumb,
                         vs_get_scrollbar_border_radius(style));
  }
}

static bool v_is_text_ready_for_draw(const VNode* text_node,
                                     const VStyle* text_node_style) {
  // TODO: maybe this should be a node flag
  return v_node__has_flag(text_node, V_NODEFLAG_HAS_FONT) &&
         (text_node->res.gfx_text_mesh ||
          !v_ctx__has_gfx_feature(V_GFX_USES_TEXT_MESH)) &&
         !v_stri_is_empty(&text_node->res_data.text) &&
         vs_has_prop(text_node_style, VS_COLOR);
}

//
// node id hashset implementation
//

static uint32_t VNodeIdHashSetValue_get_hash(const VNodeIdHashSetValue* value) {
  return value->node->id_hash;
}

static uint32_t VNodeIdHashSetKey_get_hash(const VNodeIdHashSetKey* key) {
  return v_fnv1_hash(key->id);
}

static VNodeIdHashSetKey VNodeIdHashSetKey_from_value(
    const VNodeIdHashSetValue* value) {
  return (VNodeIdHashSetKey){.id = v_stri_str(&value->node->id)};
}

static bool VNodeIdHashSetValue_eq(const VNodeIdHashSetKey* key,
                                   const VNodeIdHashSetValue* value) {
  return v_stri_eq_cstr(&value->node->id, key->id);
}

// nodes are unowned, so we don't drop them when removing from the set
#define VNodeIdHashSetValue_drop(VALUE)

V_HSET_IMPL(VNodeIdHashSet,
            VNodeIdHashSetKey,
            VNodeIdHashSetValue,
            v_node_id_hset)

//
// style class hashset implementation
//

static uint32_t VStyleClass_get_hash(const VStyleClass* value) {
  return value->id_hash;
}

static uint32_t VStyleClassHashSetKey_get_hash(
    const VStyleClassHashSetKey* key) {
  return v_fnv1_hash(key->id);
}

static VStyleClassHashSetKey VStyleClassHashSetKey_from_value(
    const VStyleClass* value) {
  return (VStyleClassHashSetKey){.id = v_stri_str(&value->id)};
}

static bool VStyleClass_eq(const VStyleClassHashSetKey* key,
                           const VStyleClass* value) {
  return v_stri_eq_cstr(&value->id, key->id);
}

#define VStyleClass_drop v_style_class__drop

V_HSET_IMPL(VStyleClassHashSet,
            VStyleClassHashSetKey,
            VStyleClass,
            v_style_class_hset)

static void v_node__get_abs_pos(const VNode* node, float* x, float* y);
static VNode* v_node__find_common_ancestor(VNode* a, VNode* b);
static VNode* v_node__hit_test_recursive(VNode* node,
                                         float abs_x,
                                         float abs_y,
                                         float mx,
                                         float my,
                                         VRect clip);

void v_inject_mouse_button(const VMouseButtonData* data) {
  if (data->button != V_MOUSE_BUTTON_LEFT) {
    // TODO: only left mouse button is supported for now
    return;
  }

  VNode* root = v_root();
  VContext* ctx = v_ctx();
  VRect initial_clip = {0, 0, 1e9f, 1e9f};
  const float x = data->x;
  const float y = data->y;
  const bool down = data->down;
  VArray* popover_stack = &ctx->popover_stack;
  VNode* target_node = NULL;

  // 1. Popover Hit Testing (Top to Bottom)
  for (size_t i = popover_stack->size; i-- > 0;) {
    target_node = v_node__hit_test_recursive(((VNode**)popover_stack->items)[i],
                                             0, 0, x, y, initial_clip);
    if (target_node)
      break;
  }

  // 2. Normal Tree Hit Testing
  if (!target_node) {
    target_node = v_node__hit_test_recursive(root, 0, 0, x, y, initial_clip);
  }

  if (down) {
    // Light Dismiss for AUTO and HINT popovers
    for (size_t i = popover_stack->size; i-- > 0;) {
      VNode* stack_node = ((VNode**)popover_stack->items)[i];
      const VPopover type = stack_node->popover_type;
      if ((type == V_POPOVER_AUTO || type == V_POPOVER_HINT) &&
          !v_node__is_descendant_of(target_node, stack_node)) {
        v_node_hide_popover(stack_node);
      }
    }

    ctx->active_node = target_node;

    // Check for scrollbar hit
    for (VNode* curr = target_node; curr; curr = v_node_parent(curr)) {
      const VStyle* curr_style = v_node__style_or_empty(curr);

      if (vs_get_overflow(curr_style) == V_OVERFLOW_SCROLL) {
        float abs_x, abs_y;
        v_node__get_abs_pos(curr, &abs_x, &abs_y);
        VRect track, thumb;
        v_node__get_scrollbar_rect(curr, abs_x, abs_y, &track, &thumb);

        if (v_point_in_rect(x, y, track.x, track.y, track.width,
                            track.height)) {
          if (v_point_in_rect(x, y, thumb.x, thumb.y, thumb.width,
                              thumb.height)) {
            ctx->drag_node = curr;
            ctx->drag_start_y = y;
            ctx->drag_start_scroll_y = curr->scroll_y;
          } else {
            const float inner_h =
                curr->bounds.height -
                (vs_get_pt(curr_style) + vs_get_pb(curr_style) +
                 vs_get_bt(curr_style) + vs_get_bb(curr_style));
            if (y < thumb.y) {
              curr->scroll_y -= inner_h;
            } else {
              curr->scroll_y += inner_h;
            }

            const float max_scroll = v_node__get_max_scroll(curr);
            curr->scroll_y = fmaxf(0.0f, fminf(curr->scroll_y, max_scroll));
          }
          return;
        }
      }
    }
  } else {
    if (target_node == ctx->active_node && target_node != NULL &&
        ctx->drag_node == NULL) {
      // use web dispatch strategy of capturing the event path first, then
      // dispatching, to allow for cases where event handlers might remove nodes
      // from the tree (including the target node itself)

      VArray* event_path = v_ctx__get_event_path();
      assert(event_path->size == 0);

      // capture the event path
      for (VNode* curr = target_node; curr; curr = v_node_parent(curr)) {
        v_node_ref(curr);
        v_array_push(event_path, &curr);
      }

      VEvent event = {V_EVENT_CLICK, target_node};

      v_node_ref(target_node);

      for (size_t i = 0; i < event_path->size; i++) {
        VNode* node = ((VNode**)event_path->items)[i];
        if (node->event_listeners[V_EVENT_CLICK]) {
          node->event_listeners[V_EVENT_CLICK](node, &event);
          // TODO: this should bubble, not sure why the break is here ???
          break;
        }
      }

      v_node_unref(target_node);
      v_ctx__clear_event_path();
    }

    ctx->active_node = NULL;
    ctx->drag_node = NULL;
  }
}

void v_inject_mouse_move(const VMouseMoveData* data) {
  VNode* root = v_root();
  VContext* context = v_ctx();
  VNode* drag_node = context->drag_node;
  const float x = data->x;
  const float y = data->y;

  if (drag_node) {
    const VStyle* drag_style = v_node__style_or_empty(drag_node);
    const float dy = y - context->drag_start_y;
    const float inner_h = drag_node->bounds.height -
                          (vs_get_pt(drag_style) + vs_get_pb(drag_style) +
                           vs_get_bt(drag_style) + vs_get_bb(drag_style));
    const float thumb_h =
        fmaxf(20.0f, (inner_h / drag_node->content_height) * inner_h);
    const float max_scroll = v_node__get_max_scroll(drag_node);

    if (inner_h > thumb_h) {
      const float scroll_move = (dy / (inner_h - thumb_h)) * max_scroll;
      drag_node->scroll_y = fmaxf(
          0.0f, fminf(context->drag_start_scroll_y + scroll_move, max_scroll));
    }
    return;
  }

  VRect initial_clip = {0, 0, 1e9f, 1e9f};
  VNode* new_hovered = NULL;
  VArray* popover_stack = &context->popover_stack;

  // 1. Popover Hit Testing (Top to Bottom)
  for (size_t i = popover_stack->size; i-- > 0;) {
    new_hovered = v_node__hit_test_recursive(((VNode**)popover_stack->items)[i],
                                             0, 0, x, y, initial_clip);
    if (new_hovered)
      break;
  }

  // 2. Normal Tree Hit Testing
  if (!new_hovered) {
    new_hovered = v_node__hit_test_recursive(root, 0, 0, x, y, initial_clip);
  }

  // Dismiss HINT popovers when cursor leaves their subtree
  for (size_t i = popover_stack->size; i-- > 0;) {
    VNode* stack_node = ((VNode**)popover_stack->items)[i];
    if (stack_node->popover_type == V_POPOVER_HINT &&
        !v_node__is_descendant_of(new_hovered, stack_node)) {
      v_node_hide_popover(stack_node);
    }
  }

  VNode* hovered_node = context->hovered_node;

  if (new_hovered == hovered_node)
    return;

  VArray* event_path = v_ctx__get_event_path();
  assert(event_path->size == 0);
  VNode* fca = v_node__find_common_ancestor(hovered_node, new_hovered);

  // leave event path: hovered_node -> fca (exclusive)
  for (VNode* curr = hovered_node; curr && curr != fca;
       curr = v_node_parent(curr)) {
    v_node_ref(curr);
    v_array_push(event_path, &curr);
  }

  const size_t enter_start_index = event_path->size;

  // enter event path: new_hovered -> fca (exclusive)
  for (VNode* curr = new_hovered; curr && curr != fca;
       curr = v_node_parent(curr)) {
    v_node_ref(curr);
    v_array_push(event_path, &curr);
  }

  v_node_ref(new_hovered);
  v_node_ref(hovered_node);

  // dispatch leave events in order from hovered_node to fca
  for (size_t i = 0; i < enter_start_index; i++) {
    VNode* curr = ((VNode**)event_path->items)[i];
    v_node__clear_flag(curr, V_NODEFLAG_HOVERED);
    if (curr->event_listeners[V_EVENT_MOUSE_LEAVE]) {
      VEvent event = {V_EVENT_MOUSE_LEAVE, curr};
      curr->event_listeners[V_EVENT_MOUSE_LEAVE](curr, &event);
    }
  }

  // dispatch enter events in order from fca to new_hovered
  for (size_t i = enter_start_index; i < event_path->size; i++) {
    VNode* curr = ((VNode**)event_path->items)[i];
    v_node__set_flag(curr, V_NODEFLAG_HOVERED);
    if (curr->event_listeners[V_EVENT_MOUSE_ENTER]) {
      VEvent event = {V_EVENT_MOUSE_ENTER, curr};
      curr->event_listeners[V_EVENT_MOUSE_ENTER](curr, &event);
    }
  }

  // if we are the last holder of new_hovered, it was removed / deleted
  // during event dispatch. if that is the case, we should not store this ref in
  // the context.
  if (new_hovered && new_hovered->ref_count == 1) {
    context->hovered_node = NULL;
  } else {
    context->hovered_node = new_hovered;
  }

  v_node_unref(new_hovered);
  v_node_unref(hovered_node);
  v_ctx__clear_event_path();
}

void v_inject_mouse_wheel(const VMouseWheelData* data) {
  VContext* context = v_ctx();

  if (!context->hovered_node) {
    return;
  }

  for (VNode* curr = context->hovered_node; curr; curr = v_node_parent(curr)) {
    if (vs_get_overflow(v_node__style_or_empty(curr)) == V_OVERFLOW_SCROLL) {
      const float max_scroll = v_node__get_max_scroll(curr);
      // TODO: 20.f works on my machine, configurable?
      const float scroll_amount = data->y * data->direction * 20.0f;
      curr->scroll_y =
          fmaxf(0.0f, fminf(curr->scroll_y - scroll_amount, max_scroll));
      return;
    }
  }
}

void v_inject_key(VKey key, uint32_t modifiers, int repeat_count, bool down) {
  // TODO: active/focus is not implemented yet, just send the
  //       key events to root for now
  VNode* root = v_root();

  if (down) {
    VKeyDownEvent event = {
        V_EVENT_KEY_DOWN, root, key, modifiers, repeat_count,
    };
    if (root->event_listeners[V_EVENT_KEY_DOWN]) {
      root->event_listeners[V_EVENT_KEY_DOWN](root, (VEvent*)&event);
    }
  } else {
    VKeyUpEvent event = {V_EVENT_KEY_UP, root, key, modifiers};
    if (root->event_listeners[V_EVENT_KEY_UP]) {
      root->event_listeners[V_EVENT_KEY_UP](root, (VEvent*)&event);
    }
  }
}

static VNode* v_node__find_common_ancestor(VNode* a, VNode* b) {
  if (!a || !b) {
    return NULL;
  }

  for (VNode* curr_a = a; curr_a; curr_a = v_node_parent(curr_a)) {
    for (VNode* curr_b = b; curr_b; curr_b = v_node_parent(curr_b)) {
      if (curr_a == curr_b)
        return curr_a;
    }
  }
  return NULL;
}

static VNode* v_node__hit_test_recursive(VNode* node,
                                         float abs_x,
                                         float abs_y,
                                         float mx,
                                         float my,
                                         VRect clip) {
  if (!v_node_is_visible(node)) {
    return NULL;
  }

  const float current_abs_x = abs_x + node->bounds.x;
  const float current_abs_y = abs_y + node->bounds.y;

  // 1. Is the mouse within the node's visual area AND current clip?
  if (!v_point_in_rect(mx, my, current_abs_x, current_abs_y, node->bounds.width,
                       node->bounds.height) ||
      !v_point_in_rect(mx, my, clip.x, clip.y, clip.width, clip.height)) {
    return NULL;
  }

  // 2. Update clip for children if overflow is hidden or scroll
  VRect next_clip = clip;
  const VStyle* style = v_node__style_or_empty(node);
  const VOverflow node_overflow = vs_get_overflow(style);

  if (node_overflow == V_OVERFLOW_HIDDEN ||
      node_overflow == V_OVERFLOW_SCROLL) {
    const float node_pl = vs_get_pl(style);
    const float node_pt = vs_get_pt(style);
    const float node_pr = vs_get_pr(style);
    const float node_pb = vs_get_pb(style);
    const float node_bt = vs_get_bt(style);
    const float node_br = vs_get_br(style);
    const float node_bb = vs_get_bb(style);
    const float node_bl = vs_get_bl(style);
    const VRect inner_rect = {
        current_abs_x + node_pl + node_bl,
        current_abs_y + node_pt + node_bt,
        node->bounds.width - (node_pl + node_pr + node_bl + node_br),
        node->bounds.height - (node_pt + node_pb + node_bt + node_bb),
    };
    next_clip = v_rect_intersect(clip, inner_rect);

    // If mouse is outside the clip of a hidden container, it can't hit children
    if (!v_point_in_rect(mx, my, next_clip.x, next_clip.y, next_clip.width,
                         next_clip.height)) {
      return node;
    }
  }

  // 3. Check children in reverse order (top-most first)
  for (VNode* child = node->last_child; child; child = child->prev_sibling) {
    // Popover nodes have absolute-space bounds.x/y; skip them here since they
    // are hit-tested directly from the popover stack with abs_x/y = 0.
    if (child->popover_type != V_POPOVER_NONE)
      continue;
    VNode* hit = v_node__hit_test_recursive(child, current_abs_x,
                                            current_abs_y - node->scroll_y, mx,
                                            my, next_clip);
    if (hit) {
      return hit;
    }
  }

  return node;
}

static void v_node__get_abs_pos(const VNode* node, float* x, float* y) {
  *x = 0;
  *y = 0;
  for (const VNode* curr = node; curr; curr = v_node_parent(curr)) {
    VNode* p = v_node_parent(curr);
    *x += curr->bounds.x;
    if (p) {
      *y += (curr->bounds.y - p->scroll_y);
    } else {
      *y += curr->bounds.y;
    }
  }
}

/*
 * TODO: there are a lot of checks for visible, popover and absolute as we go
 * through the passes because these nodes are deferred to a later pass. these
 * are messy checks that happen over and over. need to find a cleaner way to
 * handle this. maybe a flag or something..
 *
 * TODO: right now styles are not computed or resolved until needed in the
 * layout pass this is making the code more complicated and less efficient. I
 * have not decided the best way to handle this.
 */

static inline bool v_style__is_absolute(const VStyle* style) {
  return vs_has_prop(style, VS_POSITION) &&
         style->position == V_POSITION_ABSOLUTE;
}

static inline bool v_node__is_absolute(const VNode* child) {
  return child->style ? v_style__is_absolute(child->style) : false;
}

static VSize v__compute_text_pref_size(VNode* node) {
  // TODO: there should be a style resolution step to resolve font info

  if (!v_stri_is_empty(&node->res_data.text) && v_node__sync_font(node)) {
    return v_gfx_measure_string(
        v_gfx(), node->font->u.font.gfx_font, (float)node->font_size,
        node->gfx_font_face, v_stri_str(&node->res_data.text),
        v_stri_size(&node->res_data.text), V_GFX_MEASURE_MODE_TEXT, 0);
  }

  return (VSize){0, 0};
}

static VSize v__compute_image_pref_size(VNode* node) {
  VResource* image_resource =
      node->res.image_resource ? node->res.image_resource : NULL;

  if (image_resource && image_resource->state == V_RESOURCE_STATE_READY) {
    return image_resource->u.image.dimensions;
  }

  return (VSize){0, 0};
}

// Pass 1: Width Sizing (Bottom-up)
static void v_layout_pass1_width(VNode* node) {
  // note: first call is with root, which is always visible.

  const VStyle* style = v_node__style_or_empty(node);

  v_foreach_child(node, child) {
    if (v_node_is_visible(child)) {
      v_layout_pass1_width(child);
    }
  }

  float min_w;
  float pref_w;

  if (node->tag == V_NODE_TEXT) {
    // TODO: for wrap, this might be an unnecessary measure.
    const VSize size = v__compute_text_pref_size(node);
    pref_w = size.width;
    // Simple heuristic: min width is width of longest word or just a small
    // value For now, let's just return 0 or a very small value to allow
    // shrinking.
    min_w = 0;
    // keep the height so the height pass doesn't have to remeasure.
    node->pref_height = size.height;
  } else if (node->tag == V_NODE_IMAGE) {
    const VSize size = v__compute_image_pref_size(node);
    pref_w = size.width;
    min_w = 0;
  } else {
    const VDirection dir = vs_get_direction(style);
    const VWrap wrap = vs_get_wrap(style);
    int visible_children = 0;

    min_w = 0;
    pref_w = 0;

    if (dir == V_DIRECTION_ROW) {
      v_foreach_child(node, child) {
        if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
          continue;
        if (v_node__is_absolute(child))
          continue;
        visible_children++;
        // For wrap, min_width is the widest single child (worst case: one per
        // row). pref_width is still the sum (preferred: all on one row).
        if (wrap == V_WRAP_WRAP) {
          min_w = fmaxf(min_w, child->min_width);
        } else {
          min_w += child->min_width;
        }
        pref_w += child->pref_width;
      }
      if (visible_children > 1) {
        const float total_gap =
            (float)((visible_children - 1) * (int)vs_get_gap(style));
        if (wrap != V_WRAP_WRAP) {
          min_w += total_gap;
        }
        pref_w += total_gap;
      }
    } else {
      // COLUMN: width is the maximum of children
      v_foreach_child(node, child) {
        if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
          continue;
        if (v_node__is_absolute(child))
          continue;
        min_w = fmaxf(min_w, child->min_width);
        pref_w = fmaxf(pref_w, child->pref_width);
      }
    }
  }

  float ar = 0;
  if (vs_has_prop(style, VS_ASPECT_RATIO)) {
    ar = style->aspect_ratio;
  } else if (node->tag == V_NODE_IMAGE) {
    // TODO: not sure about this. might need to support aspect-ratio = auto
    const VSize size = v__compute_image_pref_size(node);
    if (size.height > 0) {
      ar = size.width / size.height;
    }
  }

  if (ar > 0) {
    const VSizing sh = vs_get_height(style);
    if (sh.tag == V_SIZING_FIXED) {
      const float inner_h = sh.min - (vs_get_pt(style) + vs_get_pb(style) +
                                      vs_get_bt(style) + vs_get_bb(style));
      pref_w = inner_h * ar;
    }
  }

  // Add padding and borders
  const float extra_w = (float)(vs_get_pl(style) + vs_get_pr(style) +
                                vs_get_bl(style) + vs_get_br(style));
  min_w += extra_w;
  pref_w += extra_w;

  // Resolve against style
  const VSizing sw = vs_get_width(style);
  if (sw.tag == V_SIZING_FIXED) {
    node->min_width = node->pref_width = sw.min;
  } else {
    node->min_width = fmaxf(sw.min, min_w);
    node->pref_width = fmaxf(sw.min, pref_w);
    if (sw.max > 0) {
      node->min_width = fminf(node->min_width, sw.max);
      node->pref_width = fminf(node->pref_width, sw.max);
    }
  }
}

// Pass 2: Width Distribution (Top-down)
static void v_layout_pass2_width_dist(VNode* node, float width) {
  // note: first call is with root, which is always visible.

  node->bounds.width = width;

  int visible_children = 0;

  // 1: visible popovers need a pass2 for their children
  // 2: absolute children get their width resolved out-of-flow
  // 3: count visible, non-popover, non-absolute children
  const VStyle* style = v_node__style_or_empty(node);
  const float inner_w = width - (vs_get_pl(style) + vs_get_pr(style) +
                                 vs_get_bl(style) + vs_get_br(style));

  v_foreach_child(node, child) {
    if (v_node_is_visible(child)) {
      const VStyle* child_style = v_node__style_or_empty(child);
      float cw;

      if (child->popover_type != V_POPOVER_NONE) {
        if (vs__get_width_tag(child_style) == V_SIZING_GROW) {
          cw = (vs_get_anchor_to(child_style) == V_ANCHOR_TO_ROOT)
                   ? v_root()->bounds.width
                   : width;
        } else {
          cw = child->pref_width;
        }
        v_layout_pass2_width_dist(child, cw);
      } else if (v_style__is_absolute(child_style)) {
        const VSizingTag wtag = vs__get_width_tag(child_style);
        if (wtag == V_SIZING_GROW) {
          cw = inner_w;
        } else if (wtag == V_SIZING_FIT && vs_has_prop(child_style, VS_LEFT) &&
                   vs_has_prop(child_style, VS_RIGHT)) {
          cw = fmaxf(0.0f, inner_w - vs_get_left(child_style) -
                               vs_get_right(child_style));
        } else {
          cw = child->pref_width;
        }
        v_layout_pass2_width_dist(child, cw);
      } else {
        visible_children++;
      }
    }
  }

  if (visible_children == 0)
    return;

  const VDirection dir = vs_get_direction(style);
  const float gap = vs_get_gap(style);

  if (dir == V_DIRECTION_ROW) {
    float fixed_w = 0;
    int grow_count = 0;
    v_foreach_child(node, child) {
      if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
        continue;

      const VStyle* child_style = v_node__style_or_empty(child);

      if (v_style__is_absolute(child_style))
        continue;

      if (vs__get_width_tag(child_style) == V_SIZING_GROW) {
        grow_count++;
      } else {
        fixed_w += child->pref_width;
      }
    }
    fixed_w += (visible_children - 1) * gap;

    // Multi-round distribution: clamp GROW children that can't receive their
    // minimum width, then redistribute the remaining space. Converges in at
    // most grow_count rounds; per_grow is monotonically non-increasing.
    int unclamped = grow_count;
    float clamped_total = 0;
    for (int round = 0; round < grow_count; round++) {
      if (unclamped == 0)
        break;
      const float per_grow =
          fmaxf(0.0f, (inner_w - fixed_w - clamped_total) / unclamped);
      int new_unclamped = 0;
      float new_clamped = 0;
      v_foreach_child(node, child) {
        if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
          continue;

        const VStyle* child_style = v_node__style_or_empty(child);

        if (v_style__is_absolute(child_style))
          continue;
        if (vs__get_width_tag(child_style) != V_SIZING_GROW)
          continue;

        if (child->min_width > per_grow) {
          new_clamped += child->min_width;
        } else {
          new_unclamped++;
        }
      }
      if (new_unclamped == unclamped)
        break;
      unclamped = new_unclamped;
      clamped_total = new_clamped;
    }

    const float grow_w =
        (unclamped > 0)
            ? fmaxf(0.0f, (inner_w - fixed_w - clamped_total) / unclamped)
            : 0.0f;

    v_foreach_child(node, child) {
      if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
        continue;

      const VStyle* child_style = v_node__style_or_empty(child);

      if (v_style__is_absolute(child_style))
        continue;

      float cw;
      if (vs__get_width_tag(child_style) == V_SIZING_GROW) {
        cw = fmaxf(child->min_width, grow_w);
      } else {
        cw = child->pref_width;
      }
      v_layout_pass2_width_dist(child, cw);
    }
  } else {
    // COLUMN
    v_foreach_child(node, child) {
      if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
        continue;

      const VStyle* child_style = v_node__style_or_empty(child);

      if (v_style__is_absolute(child_style))
        continue;
      float cw;
      if (vs__get_width_tag(child_style) == V_SIZING_GROW) {
        cw = inner_w;
      } else {
        cw = child->pref_width;
      }
      v_layout_pass2_width_dist(child, cw);
    }
  }
}

// Pass 3: Height Sizing (Bottom-up)
static void v_layout_pass3_height(VNode* node) {
  // note: first call is with root, which is always visible.

  v_foreach_child(node, child) {
    if (v_node_is_visible(child)) {
      v_layout_pass3_height(child);
    }
  }

  const VStyle* style = v_node__style_or_empty(node);
  float min_h = 0;
  float pref_h = 0;

  if (node->tag == V_NODE_TEXT) {
    if (v_node__has_flag(node, V_NODEFLAG_HAS_FONT) &&
        !v_stri_is_empty(&node->res_data.text)) {
      // TODO: for now, use the snapped width to measure
      const float inner_w = v_snap_to_grid_dpr(node->bounds.width) -
                            (float)(vs_get_pl(style) + vs_get_pr(style) +
                                    vs_get_bl(style) + vs_get_br(style));

      if (vs_get_text_wrap(style) == V_TEXT_WRAP_WRAP) {
        if (inner_w > 0) {
          // wrap_width = 0 means don't wrap on spaces (SDL3 ttf quirk)
          const VSize size = v_gfx_measure_string(
              v_gfx(), node->font->u.font.gfx_font, (float)node->font_size,
              node->gfx_font_face, v_stri_str(&node->res_data.text),
              v_stri_size(&node->res_data.text),
              V_GFX_MEASURE_MODE_TEXT_WRAPPED, (int)inner_w);
          pref_h = min_h = size.height;
        } else {
          // if there is no space for any text, leave pref and min at 0
        }
      } else /* V_TEXT_WRAP_NO_WRAP */ {
        // the pref_height comes from the measure in pass 1.
        pref_h = min_h = node->pref_height;
      }
    }
  } else if (node->tag == V_NODE_IMAGE) {
    const VSize size = v__compute_image_pref_size(node);
    pref_h = size.height;
    min_h = 0;
  } else {
    const VDirection dir = vs_get_direction(style);
    const VWrap wrap = vs_get_wrap(style);
    float main_min = 0;
    float main_pref = 0;
    float cross_min = 0;
    float cross_pref = 0;
    int visible_children = 0;

    if (dir == V_DIRECTION_ROW && wrap == V_WRAP_NONE) {
      v_foreach_child(node, child) {
        if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
          continue;
        if (v_node__is_absolute(child))
          continue;
        visible_children++;
        cross_pref = fmaxf(cross_pref, child->pref_height);
        cross_min = fmaxf(cross_min, child->min_height);
      }
      min_h = cross_min;
      pref_h = cross_pref;
    } else if (dir == V_DIRECTION_COLUMN && wrap == V_WRAP_NONE) {
      v_foreach_child(node, child) {
        if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
          continue;
        if (v_node__is_absolute(child))
          continue;
        visible_children++;
        main_min += child->min_height;
        main_pref += child->pref_height;
      }
      if (visible_children > 1) {
        const float total_gap =
            (float)((visible_children - 1) * (int)vs_get_gap(style));
        main_min += total_gap;
        main_pref += total_gap;
      }
      min_h = main_min;
      pref_h = main_pref;
    } else if (dir == V_DIRECTION_ROW && wrap == V_WRAP_WRAP) {
      // Complex wrap height calculation
      float current_row_w = 0;
      float current_row_h = 0;
      float total_h = 0;
      const float inner_w =
          node->bounds.width - (vs_get_pl(style) + vs_get_pr(style) +
                                vs_get_bl(style) + vs_get_br(style));
      const float gap = vs_get_gap(style);

      v_foreach_child(node, child) {
        if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
          continue;
        if (v_node__is_absolute(child))
          continue;
        if (current_row_w > 0 &&
            current_row_w + child->bounds.width > inner_w) {
          total_h += current_row_h + gap;
          current_row_w = 0;
          current_row_h = 0;
        }
        current_row_w += child->bounds.width + gap;
        current_row_h = fmaxf(current_row_h, child->pref_height);
      }
      total_h += current_row_h;
      min_h = pref_h = total_h;
    } else if (dir == V_DIRECTION_COLUMN && wrap == V_WRAP_WRAP) {
      const float extra_h = (float)(vs_get_pt(style) + vs_get_pb(style) +
                                    vs_get_bt(style) + vs_get_bb(style));
      VSizing sh = vs_get_height(style);
      // Column-wrap needs a height constraint to know when to break into a new
      // column. For FIXED height that constraint is known; for GROW/FIT it
      // isn't available until Pass 5, so fall back to a single column.
      const float inner_h =
          (sh.tag == V_SIZING_FIXED) ? fmaxf(0.0f, sh.min - extra_h) : FLT_MAX;
      const float gap = vs_get_gap(style);
      float current_col_h = 0;

      v_foreach_child(node, child) {
        if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
          continue;
        if (v_node__is_absolute(child))
          continue;
        if (current_col_h > 0 && current_col_h + child->pref_height > inner_h) {
          current_col_h = 0;
        }
        current_col_h += child->pref_height + gap;
      }
      pref_h = min_h = current_col_h;
    }
  }

  float ar = 0;
  if (vs_has_prop(style, VS_ASPECT_RATIO)) {
    ar = style->aspect_ratio;
  } else if (node->tag == V_NODE_IMAGE) {
    const VSize size = v__compute_image_pref_size(node);
    if (size.width > 0) {
      ar = (float)size.width / (float)size.height;
    }
  }

  if (ar > 0) {
    const float inner_w =
        node->bounds.width - (vs_get_pl(style) + vs_get_pr(style) +
                              vs_get_bl(style) + vs_get_br(style));
    pref_h = inner_w / ar;
  }

  node->content_height = pref_h;

  // Scroll containers can overflow; content size must not inflate min_h or
  // pass 4's clamping will size them to their full content height, leaving
  // no room to scroll (max_scroll would compute to 0).
  if (vs_get_overflow(style) == V_OVERFLOW_SCROLL) {
    min_h = 0;
  }

  // Add padding and borders
  const float extra_h = (float)(vs_get_pt(style) + vs_get_pb(style) +
                                vs_get_bt(style) + vs_get_bb(style));
  min_h += extra_h;
  pref_h += extra_h;

  // Resolve against style
  const VSizing sh = vs_get_height(style);
  if (sh.tag == V_SIZING_FIXED) {
    node->min_height = node->pref_height = sh.min;
  } else {
    node->min_height = fmaxf(sh.min, min_h);
    node->pref_height = fmaxf(sh.min, pref_h);
    if (sh.max > 0) {
      node->min_height = fminf(node->min_height, sh.max);
      node->pref_height = fminf(node->pref_height, sh.max);
    }
  }
}

// Pass 4: Height Distribution (Top-down)
static void v_layout_pass4_height_dist(VNode* node, float height) {
  // note: first call is with root, which is always visible.

  node->bounds.height = height;

  int visible_children = 0;

  // 1: visible popovers need a pass4 for their children
  // 2: absolute children get their height resolved out-of-flow
  // 3: count visible, non-popover, non-absolute children
  const VStyle* style = v_node__style_or_empty(node);
  const float inner_h = height - (vs_get_pt(style) + vs_get_pb(style) +
                                  vs_get_bt(style) + vs_get_bb(style));

  v_foreach_child(node, child) {
    if (v_node_is_visible(child)) {
      float ch;
      const VStyle* child_style = v_node__style_or_empty(child);

      if (child->popover_type != V_POPOVER_NONE) {
        if (vs__get_height_tag(child_style) == V_SIZING_GROW) {
          ch = (vs_get_anchor_to(child_style) == V_ANCHOR_TO_ROOT)
                   ? v_root()->bounds.height
                   : height;
        } else {
          ch = child->pref_height;
        }
        v_layout_pass4_height_dist(child, ch);
      } else if (v_style__is_absolute(child_style)) {
        const VSizingTag htag = vs__get_height_tag(child_style);
        if (htag == V_SIZING_GROW) {
          ch = inner_h;
        } else if (htag == V_SIZING_FIT && vs_has_prop(child_style, VS_TOP) &&
                   vs_has_prop(child_style, VS_BOTTOM)) {
          ch = fmaxf(0.0f, inner_h - vs_get_top(child_style) -
                               vs_get_bottom(child_style));
        } else {
          ch = child->pref_height;
        }
        v_layout_pass4_height_dist(child, ch);
      } else {
        visible_children++;
      }
    }
  }

  if (visible_children == 0)
    return;

  const VDirection dir = vs_get_direction(style);
  const VWrap wrap = vs_get_wrap(style);
  const float gap = vs_get_gap(style);

  if (dir == V_DIRECTION_COLUMN && wrap == V_WRAP_NONE) {
    float fixed_h = 0;
    int grow_count = 0;
    v_foreach_child(node, child) {
      if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
        continue;

      const VStyle* child_style = v_node__style_or_empty(child);

      if (v_style__is_absolute(child_style))
        continue;

      if (vs__get_height_tag(child_style) == V_SIZING_GROW) {
        if (vs_has_aspect_ratio(child_style)) {
          fixed_h += child->pref_height;
        } else {
          grow_count++;
        }
      } else {
        fixed_h += child->pref_height;
      }
    }
    fixed_h += (visible_children - 1) * gap;

    int unclamped = grow_count;
    float clamped_total = 0;
    for (int round = 0; round < grow_count; round++) {
      if (unclamped == 0)
        break;
      const float per_grow =
          fmaxf(0.0f, (inner_h - fixed_h - clamped_total) / unclamped);
      int new_unclamped = 0;
      float new_clamped = 0;
      v_foreach_child(node, child) {
        if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
          continue;

        const VStyle* child_style = v_node__style_or_empty(child);

        if (v_style__is_absolute(child_style))
          continue;

        if (vs__get_height_tag(child_style) != V_SIZING_GROW)
          continue;
        if (vs_has_prop(child_style, VS_ASPECT_RATIO))
          continue;
        if (child->min_height > per_grow) {
          new_clamped += child->min_height;
        } else {
          new_unclamped++;
        }
      }
      if (new_unclamped == unclamped)
        break;
      unclamped = new_unclamped;
      clamped_total = new_clamped;
    }

    const float grow_h =
        (unclamped > 0)
            ? fmaxf(0.0f, (inner_h - fixed_h - clamped_total) / unclamped)
            : 0.0f;
    float ch;

    v_foreach_child(node, child) {
      if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
        continue;

      const VStyle* child_style = v_node__style_or_empty(child);

      if (v_style__is_absolute(child_style))
        continue;

      if (vs__get_height_tag(child_style) == V_SIZING_GROW) {
        ch = vs_has_prop(child_style, VS_ASPECT_RATIO)
                 ? child->pref_height
                 : fmaxf(child->min_height, grow_h);
      } else {
        ch = child->pref_height;
      }
      v_layout_pass4_height_dist(child, ch);
    }
  } else if (dir == V_DIRECTION_ROW && wrap == V_WRAP_NONE) {
    float ch;

    v_foreach_child(node, child) {
      if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
        continue;

      const VStyle* child_style = v_node__style_or_empty(child);

      if (v_style__is_absolute(child_style))
        continue;

      if (vs__get_height_tag(child_style) == V_SIZING_GROW) {
        ch = vs_has_prop(child_style, VS_ASPECT_RATIO) ? child->pref_height
                                                       : inner_h;
      } else {
        ch = child->pref_height;
      }
      v_layout_pass4_height_dist(child, ch);
    }
  } else if (dir == V_DIRECTION_COLUMN && wrap == V_WRAP_WRAP) {
    v_foreach_child(node, child) {
      if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
        continue;
      if (v_node__is_absolute(child))
        continue;
      v_layout_pass4_height_dist(child, child->pref_height);
    }
  } else {
    // ROW_WRAP: assign each child the height of the tallest child in its row.
    const float inner_w =
        node->bounds.width - (vs_get_pl(style) + vs_get_pr(style) +
                              vs_get_bl(style) + vs_get_br(style));
    VNode* row_start = NULL;
    float current_row_w = 0;
    float current_row_h = 0;

    v_foreach_child(node, child) {
      if (!v_node_is_visible(child) || child->popover_type != V_POPOVER_NONE)
        continue;
      if (v_node__is_absolute(child))
        continue;

      if (current_row_w > 0 && current_row_w + child->bounds.width > inner_w) {
        // Flush the completed row: assign its height to every member.
        float ch;
        for (VNode* c = row_start; c != child; c = v_node_next_sibling(c)) {
          if (!v_node_is_visible(c) || c->popover_type != V_POPOVER_NONE)
            continue;

          const VStyle* child_style = v_node__style_or_empty(c);

          if (v_style__is_absolute(child_style))
            continue;
          ch = (vs__get_height_tag(child_style) == V_SIZING_GROW)
                   ? current_row_h
                   : c->pref_height;
          v_layout_pass4_height_dist(c, ch);
        }
        row_start = NULL;
        current_row_w = 0;
        current_row_h = 0;
      }

      if (!row_start)
        row_start = child;
      current_row_w += child->bounds.width + gap;
      current_row_h = fmaxf(current_row_h, child->pref_height);
    }

    // Flush the last row.
    float ch;
    for (VNode* c = row_start; c != NULL; c = v_node_next_sibling(c)) {
      if (!v_node_is_visible(c) || c->popover_type != V_POPOVER_NONE)
        continue;

      const VStyle* child_style = v_node__style_or_empty(c);

      if (v_style__is_absolute(child_style))
        continue;
      ch = (vs__get_height_tag(child_style) == V_SIZING_GROW) ? current_row_h
                                                              : c->pref_height;
      v_layout_pass4_height_dist(c, ch);
    }
  }
}

// x offset of relative-positioned nodes
static float v__get_relative_offset_x(const VStyle* child_style) {
  // can't use both left and right. resolve by favoring left.
  if (vs_has_prop(child_style, VS_LEFT)) {
    return child_style->left;
  } else if (vs_has_prop(child_style, VS_RIGHT)) {
    return -child_style->right;
  } else {
    return 0;
  }
}

// y offset of relative-positioned nodes
static float v__get_relative_offset_y(const VStyle* child_style) {
  // can't use both top and bottom. resolve by favoring top.
  if (vs_has_prop(child_style, VS_TOP)) {
    return child_style->top;
  } else if (vs_has_prop(child_style, VS_BOTTOM)) {
    return -child_style->bottom;
  } else {
    return 0;
  }
}

// Pass 5: Positioning & Alignment (Top-down)
static void v_layout_pass5_pos(VNode* node, float x, float y) {
  node->bounds.x = v_snap_to_grid_dpr(x);
  node->bounds.y = v_snap_to_grid_dpr(y);
  v_node__clear_flag(node, V_NODEFLAG_DIRTY);

  const VStyle* style = v_node__style_or_empty(node);
  const VDirection dir = vs_get_direction(style);
  const VWrap wrap = vs_get_wrap(style);
  const float gap = vs_get_gap(style);
  const float pl = vs_get_pl(style);
  const float pt = vs_get_pt(style);
  const float pr = vs_get_pr(style);
  const float pb = vs_get_pb(style);
  const float bl = vs_get_bl(style);
  const float bt = vs_get_bt(style);
  const float br = vs_get_br(style);
  const float bb = vs_get_bb(style);
  const float origin_x = pl + bl;
  const float origin_y = pt + bt;
  const float inner_w = node->bounds.width - (pl + pr + bl + br);
  const float inner_h = node->bounds.height - (pt + pb + bt + bb);

  float cur_x = origin_x;
  float cur_y = origin_y;

  if (dir == V_DIRECTION_ROW && wrap == V_WRAP_NONE) {
    // Alignment along main axis (X): only flow children count.
    float total_w = 0;
    int visible_count = 0;
    v_foreach_child(node, child) {
      if (v_node_is_visible(child) && child->popover_type == V_POPOVER_NONE &&
          !v_node__is_absolute(child)) {
        total_w += child->bounds.width;
        visible_count++;
      }
    }
    if (visible_count > 1)
      total_w += (visible_count - 1) * gap;

    const VAlignX ax = vs_get_xalign(style);
    if (ax == V_ALIGN_X_CENTER)
      cur_x += fmaxf(0, (inner_w - total_w) / 2.0f);
    else if (ax == V_ALIGN_X_RIGHT)
      cur_x += fmaxf(0, inner_w - total_w);

    v_foreach_child(node, child) {
      if (!v_node_is_visible(child))
        continue;
      if (child->popover_type != V_POPOVER_NONE) {
        v_layout_pass5_pos(child, 0, 0);
        continue;
      }

      const VStyle* child_style = v_node__style_or_empty(child);
      const VPosition position = vs_get_position(child_style);

      if (position == V_POSITION_ABSOLUTE)
        continue;

      float child_x = cur_x;
      float child_y = cur_y;
      const VAlignY ay = vs_get_yalign(style);
      if (ay == V_ALIGN_Y_CENTER)
        child_y += fmaxf(0, (inner_h - child->bounds.height) / 2.0f);
      else if (ay == V_ALIGN_Y_BOTTOM)
        child_y += fmaxf(0, inner_h - child->bounds.height);
      if (position == V_POSITION_RELATIVE) {
        child_x += v__get_relative_offset_x(child_style);
        child_y += v__get_relative_offset_y(child_style);
      }
      v_layout_pass5_pos(child, child_x, child_y);
      cur_x += child->bounds.width + gap;
    }
  } else if (dir == V_DIRECTION_COLUMN && wrap == V_WRAP_NONE) {
    // Alignment along main axis (Y): only flow children count.
    float total_h = 0;
    int visible_count = 0;
    v_foreach_child(node, child) {
      if (v_node_is_visible(child) && child->popover_type == V_POPOVER_NONE &&
          !v_node__is_absolute(child)) {
        total_h += child->bounds.height;
        visible_count++;
      }
    }
    if (visible_count > 1)
      total_h += (visible_count - 1) * gap;

    const VAlignY ay = vs_get_yalign(style);
    if (ay == V_ALIGN_Y_CENTER)
      cur_y += fmaxf(0, (inner_h - total_h) / 2.0f);
    else if (ay == V_ALIGN_Y_BOTTOM)
      cur_y += fmaxf(0, inner_h - total_h);

    v_foreach_child(node, child) {
      if (!v_node_is_visible(child))
        continue;
      if (child->popover_type != V_POPOVER_NONE) {
        v_layout_pass5_pos(child, 0, 0);
        continue;
      }

      const VStyle* child_style = v_node__style_or_empty(child);
      const VPosition position = vs_get_position(child_style);

      if (position == V_POSITION_ABSOLUTE)
        continue;
      float child_x = cur_x;
      float child_y = cur_y;
      const VAlignX ax = vs_get_xalign(style);
      if (ax == V_ALIGN_X_CENTER)
        child_x += fmaxf(0, (inner_w - child->bounds.width) / 2.0f);
      else if (ax == V_ALIGN_X_RIGHT)
        child_x += fmaxf(0, inner_w - child->bounds.width);
      if (position == V_POSITION_RELATIVE) {
        child_x += v__get_relative_offset_x(child_style);
        child_y += v__get_relative_offset_y(child_style);
      }
      v_layout_pass5_pos(child, child_x, child_y);
      cur_y += child->bounds.height + gap;
    }
  } else if (dir == V_DIRECTION_ROW && wrap == V_WRAP_WRAP) {
    float row_h = 0;

    v_foreach_child(node, child) {
      if (!v_node_is_visible(child))
        continue;
      if (child->popover_type != V_POPOVER_NONE) {
        v_layout_pass5_pos(child, 0, 0);
        continue;
      }

      const VStyle* child_style = v_node__style_or_empty(child);
      const VPosition position = vs_get_position(child_style);

      if (position == V_POSITION_ABSOLUTE)
        continue;

      if (cur_x > origin_x &&
          cur_x + child->bounds.width > origin_x + inner_w) {
        cur_y += row_h + gap;
        cur_x = origin_x;
        row_h = 0;
      }
      float child_x = cur_x;
      float child_y = cur_y;
      if (position == V_POSITION_RELATIVE) {
        child_x += v__get_relative_offset_x(child_style);
        child_y += v__get_relative_offset_y(child_style);
      }
      v_layout_pass5_pos(child, child_x, child_y);
      cur_x += child->bounds.width + gap;
      row_h = fmaxf(row_h, child->bounds.height);
    }
  } else if (dir == V_DIRECTION_COLUMN && wrap == V_WRAP_WRAP) {
    float col_w = 0;

    v_foreach_child(node, child) {
      if (!v_node_is_visible(child))
        continue;
      if (child->popover_type != V_POPOVER_NONE) {
        v_layout_pass5_pos(child, 0, 0);
        continue;
      }

      const VStyle* child_style = v_node__style_or_empty(child);
      const VPosition position = vs_get_position(child_style);

      if (position == V_POSITION_ABSOLUTE)
        continue;

      if (cur_y > origin_y &&
          cur_y + child->bounds.height > origin_y + inner_h) {
        cur_x += col_w + gap;
        cur_y = origin_y;
        col_w = 0;
      }
      float child_x = cur_x;
      float child_y = cur_y;
      if (position == V_POSITION_RELATIVE) {
        child_x += v__get_relative_offset_x(child_style);
        child_y += v__get_relative_offset_y(child_style);
      }
      v_layout_pass5_pos(child, child_x, child_y);
      cur_y += child->bounds.height + gap;
      col_w = fmaxf(col_w, child->bounds.width);
    }
  }

  // Position absolute children relative to this node's content origin.
  v_foreach_child(node, child) {
    if (!v_node_is_visible(child))
      continue;
    const VStyle* child_style = v_node__style_or_empty(child);
    if (!v_style__is_absolute(child_style))
      continue;
    float child_x, child_y;
    if (vs_has_prop(child_style, VS_LEFT)) {
      child_x = origin_x + vs_get_left(child_style);
    } else if (vs_has_prop(child_style, VS_RIGHT)) {
      child_x =
          origin_x + inner_w - vs_get_right(child_style) - child->bounds.width;
    } else {
      child_x = origin_x;
    }
    if (vs_has_prop(child_style, VS_TOP)) {
      child_y = origin_y + vs_get_top(child_style);
    } else if (vs_has_prop(child_style, VS_BOTTOM)) {
      child_y = origin_y + inner_h - vs_get_bottom(child_style) -
                child->bounds.height;
    } else {
      child_y = origin_y;
    }
    v_layout_pass5_pos(child, child_x, child_y);
  }
}

static void v_node_get_abs_pos_point(VRect rect,
                                     VAttachPointX attach_point_x,
                                     VAttachPointY attach_point_y,
                                     float* out_x,
                                     float* out_y) {
  switch (attach_point_x) {
    case V_ATTACH_POINT_X_LEFT:
    default:
      *out_x = rect.x;
      break;
    case V_ATTACH_POINT_X_CENTER:
      *out_x = rect.x + rect.width / 2.0f;
      break;
    case V_ATTACH_POINT_X_RIGHT:
      *out_x = rect.x + rect.width;
      break;
  }
  switch (attach_point_y) {
    case V_ATTACH_POINT_Y_TOP:
    default:
      *out_y = rect.y;
      break;
    case V_ATTACH_POINT_Y_CENTER:
      *out_y = rect.y + rect.height / 2.0f;
      break;
    case V_ATTACH_POINT_Y_BOTTOM:
      *out_y = rect.y + rect.height;
      break;
  }
}

static void v_layout_pass6_popovers(VNode* node,
                                    float abs_x,
                                    float abs_y,
                                    float root_w,
                                    float root_h) {
  const float current_abs_x = abs_x + node->bounds.x;
  const float current_abs_y = abs_y + node->bounds.y;

  v_foreach_child(node, child) {
    if (!v_node_is_visible(child))
      continue;

    if (child->popover_type != V_POPOVER_NONE) {
      const VStyle* child_style = v_node__style_or_empty(child);
      const VAnchorTo attach = vs_get_anchor_to(child_style);
      VRect anchor_rect;

      if (attach == V_ANCHOR_TO_ROOT) {
        anchor_rect = (VRect){0, 0, root_w, root_h};
      } else {
        anchor_rect = (VRect){
            current_abs_x,
            current_abs_y,
            node->bounds.width,
            node->bounds.height,
        };
      }

      float ax, ay;
      v_node_get_abs_pos_point(
          anchor_rect, vs_get_anchor_attach_point_x(child_style),
          vs_get_anchor_attach_point_y(child_style), &ax, &ay);

      const VRect popover_rect = {
          0,
          0,
          child->bounds.width,
          child->bounds.height,
      };
      float px, py;
      v_node_get_abs_pos_point(popover_rect, vs_get_attach_point_x(child_style),
                               vs_get_attach_point_y(child_style), &px, &py);

      // popover bounds.x/y are in absolute screen space coordinates
      // because they are drawn relative to root (0,0) in Pass B/C
      child->bounds.x = v_snap_to_grid_dpr(
          ax - px + vs_get_attach_point_offset_x(child_style));
      child->bounds.y = v_snap_to_grid_dpr(
          ay - py + vs_get_attach_point_offset_y(child_style));
    }

    v_layout_pass6_popovers(child, current_abs_x,
                            current_abs_y - node->scroll_y, root_w, root_h);
  }
}

void v_layout(int width, int height) {
  VNode* root = v_root();
  VContext* ctx = v_ctx();

  width = v_clamp_int(width, 0, VUID_FLOAT_MAX_INT);
  height = v_clamp_int(height, 0, VUID_FLOAT_MAX_INT);

  const float width_f = (float)width;
  const float height_f = (float)height;

  assert(root->style);

  // ensure the root's dimensions are also reflected in its style.
  root->style->width = V_FIXED(width_f);
  root->style->is_set |= ((uint64_t)1) << VS_WIDTH;
  root->style->height = V_FIXED(height_f);
  root->style->is_set |= ((uint64_t)1) << VS_HEIGHT;

  if (width != ctx->root_width || height != ctx->root_height) {
    v_node_mark_dirty(root);
  } else if (!v_node__has_flag(root, V_NODEFLAG_DIRTY)) {
    return;
  }

  ctx->root_width = width;
  ctx->root_height = height;

  v_layout_pass1_width(root);
  v_layout_pass2_width_dist(root, width_f);
  v_layout_pass3_height(root);
  v_layout_pass4_height_dist(root, height_f);
  v_layout_pass5_pos(root, 0, 0);
  v_layout_pass6_popovers(root, 0, 0, width_f, height_f);
}

static bool v_node__has_parent(const VNode* node);
static void v_node__set_attached_recursive(VNode* node, bool attached);
static bool v_node__is_legal_id(const char* id);
static VWeakNodeRef* v_node__get_weak_ref(VNode* node);
static VWeakNodeRef* v_node__aquire_weak_ref(VWeakNodeRef* weak_ref);
static void v_node__release_weak_ref(VWeakNodeRef* weak_ref);
static void v_node__sync_text_resource(VNode* text_node);
static bool v_node__can_insert(const VNode* parent, const VNode* child);

VNode* v_new_node(VNodeTag tag) {
  return v_new_node_with_cfg(tag, NULL);
}

VNode* v_new_node_with_cfg(VNodeTag tag, const VNodeConfig* config) {
  if (tag == V_NODE_ROOT) {
    return NULL;
  }

  VNode* node = v_node__constructor(tag);

  if (node && config) {
    if (config->id) {
      v_node_set_id(node, config->id);
    }

    if (config->sclass) {
      v_node_style_assign_class(node, config->sclass);
    }

    if (config->popover != V_POPOVER_NONE) {
      v_node_set_popover(node, config->popover);
    }

    if (config->data) {
      v_node_set_data(node, config->data);
    }

    switch (tag) {
      case V_NODE_TEXT:
        if (config->content.text) {
          v_node_set_text(node, config->content.text);
        }
        break;
      case V_NODE_IMAGE:
        if (config->content.src) {
          v_node_set_src(node, config->content.src);
        }
        break;
      default:
        break;
    }

    if (config->on_click) {
      v_node_set_event_listener(node, V_EVENT_CLICK, config->on_click);
    }

    if (config->on_mouse_enter) {
      v_node_set_event_listener(node, V_EVENT_MOUSE_ENTER,
                                config->on_mouse_enter);
    }

    if (config->on_mouse_leave) {
      v_node_set_event_listener(node, V_EVENT_MOUSE_LEAVE,
                                config->on_mouse_leave);
    }

    if (config->on_user) {
      v_node_set_event_listener(node, V_EVENT_USER, config->on_user);
    }

    // TODO: image and text should not allow children. assert? delete them?

    const VNodeArray* children = &config->children;

    for (size_t i = 0; i < children->size; i++) {
      // TODO: what if append fails?
      v_node_append_child(node, children->items[i]);
    }
  }

  return node;
}

const char* v_node_id(const VNode* node) {
  return node ? v_stri_str(&node->id) : "";
}

void v_node_set_id(VNode* node, const char* id) {
  // TODO: review (confusing code)
  if (id == NULL) {
    id = "";
  }

  if (!node || v_stri_eq_cstr(&node->id, id) || !v_node__is_legal_id(id)) {
    return;
  }

  const bool attached = v_node__has_flag(node, V_NODEFLAG_ATTACHED);
  VContext* ctx = v_ctx();

  if (attached && !v_stri_is_empty(&node->id)) {
    const VNodeIdHashSetValue value = VNodeIdHashSetValue_init(node);
    v_node_id_hset_remove_by_value(&ctx->nodes_by_id, &value);
  }

  v_stri_assign(&node->id, id);
  node->id_hash = v_fnv1_hash(v_stri_str(&node->id));

  if (attached && !v_stri_is_empty(&node->id)) {
    v_node_id_hset_put(&ctx->nodes_by_id, VNodeIdHashSetValue_init(node));
  }
}

void v_node_set_id_fmt(VNode* node, const char* fmt, ...) {
  // TODO: review (confusing code)
  if (!node) {
    return;
  }

  const bool attached = v_node__has_flag(node, V_NODEFLAG_ATTACHED);
  VContext* ctx = v_ctx();

  if (attached && !v_stri_is_empty(&node->id)) {
    const VNodeIdHashSetValue value = VNodeIdHashSetValue_init(node);
    v_node_id_hset_remove_by_value(&ctx->nodes_by_id, &value);
  }

  // assign directly to VNodeId to avoid extra allocs
  va_list args;
  va_start(args, fmt);
  v_stri_assign_vfmt(&node->id, fmt, args);
  node->id_hash = v_fnv1_hash(v_stri_str(&node->id));
  va_end(args);

  // original id is gone.. clear the id as a fallback
  if (!v_node__is_legal_id(v_stri_str(&node->id))) {
    v_stri_clear(&node->id);
    node->id_hash = v_fnv1_hash(v_stri_str(&node->id));
    return;
  }

  if (attached && !v_stri_is_empty(&node->id)) {
    v_node_id_hset_put(&ctx->nodes_by_id, VNodeIdHashSetValue_init(node));
  }
}

VNodeTag v_node_tag(const VNode* node) {
  // null node is undefined behavior, return something rather than crashing
  return node ? node->tag : V_NODE_BOX;
}

VStyle* v_node_style(VNode* node) {
  return node ? v_node__style_mut(node) : NULL;
}

VNode* v_node_parent(const VNode* node) {
  return node && node->parent ? node->parent->ref : NULL;
}

VNode* v_node_first_child(const VNode* node) {
  return node ? node->first_child : NULL;
}

VNode* v_node_last_child(const VNode* node) {
  return node ? node->last_child : NULL;
}

VNode* v_node_next_sibling(const VNode* node) {
  return node ? node->next_sibling : NULL;
}

VNode* v_node_prev_sibling(const VNode* node) {
  return node ? node->prev_sibling : NULL;
}

void v_node_mark_dirty(VNode* node) {
  VNode* walker = node;

  while (walker) {
    if (v_node__has_flag(walker, V_NODEFLAG_DIRTY)) {
      return;
    }
    v_node__set_flag(walker, V_NODEFLAG_DIRTY);
    walker = v_node_parent(walker);
  }
}

bool v_node_is_dirty(const VNode* node) {
  return node && v_node__has_flag(node, V_NODEFLAG_DIRTY);
}

void v_node_set_visible(VNode* node, bool is_visible) {
  if (!node || node->tag == V_NODE_ROOT) {
    return;
  }

  bool current = v_node__has_flag(node, V_NODEFLAG_HIDDEN) == false;
  if (current == is_visible) {
    return;
  }

  if (is_visible) {
    v_node__clear_flag(node, V_NODEFLAG_HIDDEN);
  } else {
    v_node__set_flag(node, V_NODEFLAG_HIDDEN);
  }

  v_node_mark_dirty(node);
}

bool v_node_is_visible(const VNode* node) {
  return node && !v_node__has_flag(node, V_NODEFLAG_HIDDEN);
}

void v_node_set_event_listener(VNode* node,
                               int event_type,
                               VEventListener listener) {
  if (event_type < 0 || event_type >= V_EVENT__COUNT) {
    return;
  }

  node->event_listeners[event_type] = listener;
}

void v_node_reset_scroll_y(VNode* node) {
  if (node) {
    node->scroll_y = 0;
  }
}

int v_node_child_count(const VNode* node) {
  return node ? node->child_count : 0;
}

VNode* v_node_child_at(const VNode* node, int index) {
  int i = 0;

  v_foreach_child(node, child) {
    if (i++ == index) {
      return child;
    }
  }

  return NULL;
}

bool v_node_append_child(VNode* node, VNode* child) {
  if (!v_node__can_insert(node, child)) {
    // TODO: unref child?
    return false;
  }

  VWeakNodeRef* parent = v_node__get_weak_ref(node);

  if (!parent) {
    // TODO: unref child?
    return false;
  }

  child->parent = parent;
  child->next_sibling = NULL;
  child->prev_sibling = node->last_child;

  if (node->last_child) {
    node->last_child->next_sibling = child;
  } else {
    node->first_child = child;
  }

  node->last_child = child;
  node->child_count++;

  if (v_node__has_flag(node, V_NODEFLAG_ATTACHED)) {
    v_node__set_attached_recursive(child, true);
  }

  v_node_mark_dirty(node);

  return true;
}

bool v_node_insert_before(VNode* node, VNode* child, VNode* reference_node) {
  if (reference_node == NULL) {
    return v_node_append_child(node, child);
  }

  if (!v_node__can_insert(node, child) ||
      v_node_parent(reference_node) != node) {
    // TODO: unref child?
    return false;
  }

  VWeakNodeRef* parent = v_node__get_weak_ref(node);

  if (!parent) {
    // TODO: unref child?
    return false;
  }

  child->parent = parent;
  child->next_sibling = reference_node;
  child->prev_sibling = reference_node->prev_sibling;

  if (reference_node->prev_sibling) {
    reference_node->prev_sibling->next_sibling = child;
  } else {
    node->first_child = child;
  }

  reference_node->prev_sibling = child;
  node->child_count++;

  if (v_node__has_flag(node, V_NODEFLAG_ATTACHED)) {
    v_node__set_attached_recursive(child, true);
  }

  v_node_mark_dirty(node);

  return true;
}

bool v_node_prepend_child(VNode* node, VNode* child) {
  if (!v_node__can_insert(node, child)) {
    // TODO: unref child?
    return false;
  }

  VWeakNodeRef* parent = v_node__get_weak_ref(node);

  if (!parent) {
    // TODO: unref child?
    return false;
  }

  child->parent = parent;
  child->prev_sibling = NULL;
  child->next_sibling = node->first_child;

  if (node->first_child) {
    node->first_child->prev_sibling = child;
  } else {
    node->last_child = child;
  }

  node->first_child = child;
  node->child_count++;

  if (v_node__has_flag(node, V_NODEFLAG_ATTACHED)) {
    v_node__set_attached_recursive(child, true);
  }

  v_node_mark_dirty(node);

  return true;
}

bool v_node_remove_child(VNode* node, VNode* child) {
  if (!node || !child || v_node_parent(child) != node) {
    // TODO: unref child?
    return false;
  }

  if (v_node__has_flag(child, V_NODEFLAG_ATTACHED)) {
    v_node__set_attached_recursive(child, false);
  }

  if (child->prev_sibling) {
    child->prev_sibling->next_sibling = child->next_sibling;
  } else {
    node->first_child = child->next_sibling;
  }

  if (child->next_sibling) {
    child->next_sibling->prev_sibling = child->prev_sibling;
  } else {
    node->last_child = child->prev_sibling;
  }

  v_node__release_weak_ref(child->parent);
  child->parent = NULL;
  child->next_sibling = NULL;
  child->prev_sibling = NULL;
  v_node_unref(child);

  // TODO: remove the child and its children from the context?

  node->child_count--;
  v_node_mark_dirty(node);

  return true;
}

bool v_node_replace_child(VNode* node, VNode* new_child, VNode* old_child) {
  if (!v_node__can_insert(node, new_child) ||
      v_node_parent(old_child) != node) {
    // TODO: unref new_child?
    return false;
  }

  VWeakNodeRef* parent = v_node__get_weak_ref(node);

  if (!parent) {
    // TODO: unref child?
    return false;
  }

  if (v_node__has_flag(old_child, V_NODEFLAG_ATTACHED)) {
    v_node__set_attached_recursive(old_child, false);
  }

  if (old_child->prev_sibling) {
    old_child->prev_sibling->next_sibling = new_child;
  } else {
    node->first_child = new_child;
  }

  if (old_child->next_sibling) {
    old_child->next_sibling->prev_sibling = new_child;
  } else {
    node->last_child = new_child;
  }

  new_child->parent = parent;
  new_child->next_sibling = old_child->next_sibling;
  new_child->prev_sibling = old_child->prev_sibling;

  if (v_node__has_flag(node, V_NODEFLAG_ATTACHED)) {
    v_node__set_attached_recursive(new_child, true);
  }

  v_node__release_weak_ref(old_child->parent);
  old_child->parent = NULL;
  old_child->next_sibling = NULL;
  old_child->prev_sibling = NULL;
  v_node_unref(old_child);

  v_node_mark_dirty(node);

  return true;
}

void v_node_remove_children(VNode* node) {
  if (!node) {
    return;
  }

  VNode* child = node->first_child;

  while (child) {
    VNode* next = child->next_sibling;

    if (!v_node_remove_child(node, child)) {
      // TODO: assert
      return;
    }

    child = next;
  }
}

void v_node_set_text(VNode* node, const char* value) {
  if (!node || node->tag != V_NODE_TEXT) {
    return;
  }

  const size_t len = value ? strlen(value) : 0;

  if (v_stri_eq_cstr_n(&node->res_data.text, value, len)) {
    return;
  }

  v_stri_assign_n(&node->res_data.text, value, len);

  v_node__sync_text_resource(node);
  v_node_mark_dirty(node);
}

void v_node_set_text_fmt(VNode* node, const char* fmt, ...) {
  if (!node || node->tag != V_NODE_TEXT) {
    return;
  }

  va_list args;
  va_start(args, fmt);
  v_stri_assign_vfmt(&node->res_data.text, fmt, args);
  va_end(args);

  // note: cannot determine if string changed without doing an
  // alloc. for now, prefer to always trigger a layout

  v_node__sync_text_resource(node);
  v_node_mark_dirty(node);
}

const char* v_node_text(const VNode* node) {
  return (node && node->tag == V_NODE_TEXT) ? v_stri_str(&node->res_data.text)
                                            : "";
}

void v_node_set_src(VNode* node, const char* src) {
  if (!node || node->tag != V_NODE_IMAGE) {
    return;
  }

  src = src ? src : "";

  if (v_stri_eq_cstr(&node->res_data.src, src)) {
    return;
  }

  if (node->res.image_resource) {
    v_resource__release(node->res.image_resource);
    node->res.image_resource = NULL;
  }

  if (*src != '\0') {
    node->res.image_resource = v_resource__acquire_image(src);
    // TODO: handle failure
  }

  // TODO: this can fail
  v_stri_assign(&node->res_data.src, src);

  v_node_mark_dirty(node);
}

const char* v_node_src(const VNode* node) {
  if (node && node->tag == V_NODE_IMAGE) {
    return v_stri_str(&node->res_data.src);
  } else {
    return "";
  }
}

void v_node_set_data(VNode* node, void* data) {
  node->user_data = data;
}

void* v_node_data(const VNode* node) {
  return node->user_data;
}

void v_node_dispatch_user_event(VNode* node, VEvent* event) {
  if (node && node->event_listeners[V_EVENT_USER]) {
    node->event_listeners[V_EVENT_USER](node, event);
  }
}

void v_node_style_assign(VNode* node, VStyle* style) {
  if (node) {
    v_node_style_reset(node);
    v_node_style_apply(node, style);
  }
}

void v_node_style_apply(VNode* node, VStyle* style) {
  // TODO: should style == NULL be a reset?
  if (node && style) {
    VStyle* node_style = v_node__style_mut(node);

    if (node_style != style) {
      v_style__flatten(node_style, style);
    }
  }
}

void v_node_style_reset(VNode* node) {
  if (node) {
    v_style_reset(node->style);
  }
}

VPopover v_node_popover(const VNode* node) {
  return node ? node->popover_type : V_POPOVER_NONE;
}

void v_node_set_popover(VNode* node, VPopover type) {
  if (!node || node->popover_type == type)
    return;

  if (v_node__has_flag(node, V_NODEFLAG_POPOVER_OPEN)) {
    v_node_hide_popover(node);
  }

  node->popover_type = type;
}

bool v_node_show_popover(VNode* node) {
  if (!node || node->popover_type == V_POPOVER_NONE ||
      !v_node__has_flag(node, V_NODEFLAG_ATTACHED) ||
      v_node__has_flag(node, V_NODEFLAG_POPOVER_OPEN))
    return false;

  VContext* ctx = v_ctx();

  if (node->popover_type == V_POPOVER_AUTO) {
    for (size_t i = ctx->popover_stack.size; i-- > 0;) {
      VNode* stack_node = ((VNode**)ctx->popover_stack.items)[i];
      if (stack_node->popover_type == V_POPOVER_AUTO &&
          !v_node__is_descendant_of(node, stack_node)) {
        v_node_hide_popover(stack_node);
      }
    }
  }
  // HINT: does not close other popovers on show

  // TODO: assert?
  if (!v_array_push(&ctx->popover_stack, &node))
    return false;

  v_node__set_flag(node, V_NODEFLAG_POPOVER_OPEN);
  v_node_set_visible(node, true);
  v_node_mark_dirty(node);
  return true;
}

void v_node_hide_popover(VNode* node) {
  if (!node || !v_node__has_flag(node, V_NODEFLAG_POPOVER_OPEN))
    return;

  v_ctx__remove_popover_node(node);
  v_node__clear_flag(node, V_NODEFLAG_POPOVER_OPEN);
  v_node_set_visible(node, false);
  v_node_mark_dirty(node);
}

void v_node_ref(VNode* node) {
  if (node) {
    if (node->ref_count < UINT32_MAX) {
      node->ref_count++;
    } else {
      assert(false && "VNode ref count overflow");
    }
  }
}

void v_node_unref(VNode* node) {
  if (node) {
    const uint32_t ref_count = node->ref_count;

    if (ref_count > 1) {
      node->ref_count--;
    } else if (ref_count == 1) {
      v_node__destructor(node);
    } else {
      assert(false && "VNode ref count underflow");
    }
  }
}

uint32_t v_node_ref_count(const VNode* node) {
  return node ? node->ref_count : 0;
}

VUID_PRIVATE const VStyle* v_node__style_or_empty(const VNode* node) {
  const VStyle* style = node->style;

  return style ? style : v_ctx()->empty_style;
}

VUID_PRIVATE VStyle* v_node__style_mut(VNode* node) {
  VStyle* style = node->style;

  if (!style) {
    style = node->style = v_style__new(node);
  }

  return style;
}

VUID_PRIVATE VNode* v_node__constructor(VNodeTag tag) {
  VNode* node = v_ctx_new(VNode);

  if (node) {
    node->ref_count = 1;
    node->tag = tag;
    node->flags = V_NODEFLAG_DIRTY;

    if (tag == V_NODE_TEXT) {
      v_node__set_flag(node, V_NODEFLAG_LEAF);
    } else if (tag == V_NODE_IMAGE) {
      v_node__set_flag(node, V_NODEFLAG_LEAF);
    }

    v_ctx()->node_count++;
  }

  return node;
}

VUID_PRIVATE void v_node__destructor(VNode* node) {
  assert(v_node__has_parent(node) == false);
  assert(!v_node__has_flag(node, V_NODEFLAG_ATTACHED));

  v_node_remove_children(node);

  // release resources
  if (node->tag == V_NODE_TEXT) {
    v_stri_drop(&node->res_data.text);

    if (node->res.gfx_text_mesh) {
      v_gfx_object_free(v_gfx(), V_GFX_OBJECT_TEXT_MESH,
                        node->res.gfx_text_mesh);
    }

    v_resource__unsubscribe(node->font_sub);

    if (node->font) {
      v_resource__release(node->font);
    }
  } else if (node->tag == V_NODE_IMAGE) {
    v_stri_drop(&node->res_data.src);
    if (node->res.image_resource) {
      v_resource__release(node->res.image_resource);
    }
  }

  v_stri_drop(&node->id);

  // release style and clear unowned reference to us
  if (node->style) {
    node->style->owner = NULL;
    v_style_unref(node->style);
  }

  // release weak ref and revoke the ref to us
  if (node->self_weak_ref) {
    node->self_weak_ref->ref = NULL;
    v_node__release_weak_ref(node->self_weak_ref);
  }

  v_ctx_delete(node, VNode);

  v_ctx()->node_count--;
}

VUID_PRIVATE void v_node__set_attached(VNode* node, bool attached) {
  if (attached) {
    v_node__set_flag(node, V_NODEFLAG_ATTACHED);
    if (!v_stri_is_empty(&node->id)) {
      v_node_id_hset_put(&v_ctx()->nodes_by_id, VNodeIdHashSetValue_init(node));
    }
  } else {
    v_ctx__remove_input_node(node);
    if (v_node__has_flag(node, V_NODEFLAG_POPOVER_OPEN)) {
      v_node_hide_popover(node);
    }
    v_node__clear_flag(node, V_NODEFLAG_ATTACHED);
    if (!v_stri_is_empty(&node->id)) {
      const VNodeIdHashSetValue value = VNodeIdHashSetValue_init(node);
      v_node_id_hset_remove_by_value(&v_ctx()->nodes_by_id, &value);
    }
  }
}

VUID_PRIVATE float v_node__get_max_scroll(const VNode* node) {
  const VStyle* style = v_node__style_or_empty(node);
  const float inner_h =
      node->bounds.height - (vs_get_pt(style) + vs_get_pb(style) +
                             vs_get_bt(style) + vs_get_bb(style));
  return fmaxf(0.0f, node->content_height - inner_h);
}

VUID_PRIVATE void v_node__get_scrollbar_rect(VNode* node,
                                             float abs_x,
                                             float abs_y,
                                             VRect* track,
                                             VRect* thumb) {
  const VStyle* style = v_node__style_or_empty(node);
  const float scrollbar_width = vs_get_scrollbar_width(style);
  const float node_pt = vs_get_pt(style);
  const float node_bt = vs_get_bt(style);
  const float node_br = vs_get_br(style);
  const float inner_h = node->bounds.height - (node_pt + vs_get_pb(style) +
                                               node_bt + vs_get_bb(style));
  const float max_scroll = v_node__get_max_scroll(node);

  track->x = abs_x + node->bounds.width - (scrollbar_width + node_br);
  track->y = abs_y + node_pt + node_bt;
  track->width = scrollbar_width;
  track->height = inner_h;

  if (node->content_height <= inner_h) {
    *thumb = (VRect){0, 0, 0, 0};
    return;
  }

  const float thumb_h =
      fmaxf(20.0f, (inner_h / node->content_height) * inner_h);
  const float thumb_y =
      (max_scroll > 0) ? (node->scroll_y / max_scroll) * (inner_h - thumb_h)
                       : 0;

  thumb->x = track->x;
  thumb->y = track->y + thumb_y;
  thumb->width = track->width;
  thumb->height = thumb_h;
}

static void v_node__set_attached_recursive(VNode* node, bool attached) {
  v_node__set_attached(node, attached);

  v_foreach_child(node, child) {
    v_node__set_attached_recursive(child, attached);
  }
}

VUID_PRIVATE bool v_node__is_descendant_of(const VNode* node,
                                           const VNode* ancestor) {
  if (!node || !ancestor)
    return false;
  if (node == ancestor)
    return true;
  return v_node__is_descendant_of(v_node_parent(node), ancestor);
}

VUID_PRIVATE bool v_node__sync_font(VNode* node) {
  if (v_node__has_flag(node, V_NODEFLAG_HAS_FONT)) {
    return true;
  }

  const VStyle* style = node->style;
  uint16_t font_id;

  if (!style) {
    return false;
  }

  if (!vs_has_prop(style, VS_FONT)) {
    if (node->font) {
      v_node__clear_font(node);
    }
    goto missing_face;
  }

  font_id = style->font;

  if (!vs_has_prop(style, VS_FONT_SIZE)) {
    goto missing_face;
  }

  if (node->font == NULL || node->font->u.font.font_id != font_id) {
    if (node->font) {
      v_node__clear_font(node);
    }

    node->font = v_resource__acquire_font(font_id);

    if (node->font) {
      node->font_sub = v_resource__subscribe(node->font, node);
      assert(node->font_sub);
    }
  }

  if (node->font == NULL || node->font->state != V_RESOURCE_STATE_READY)
    goto missing_face;

  if (v_ctx__has_gfx_feature(V_GFX_USES_FONT_FACE)) {
    if (node->gfx_font_face == NULL || node->font_size != style->font_size) {
      node->gfx_font_face =
          v_resource__get_or_create_font_face(node->font, style->font_size);

      if (!node->gfx_font_face)
        goto missing_face;
    }
  }

  v_node__set_flag(node, V_NODEFLAG_HAS_FONT);
  return true;

missing_face:
  node->gfx_font_face = NULL;
  node->font_size = 0;
  return false;
}

VUID_PRIVATE void v_node__clear_font(VNode* node) {
  if (v_node__has_flag(node, V_NODEFLAG_HAS_FONT)) {
    v_node__clear_flag(node, V_NODEFLAG_HAS_FONT);

    // SDL3 ttf quirk: TTF_Text holds a reference to it's TTF_Font. Since vuid
    // supports dynamic removing fonts, TTF_Font needs to be cleared from the
    // TTF_Text to avoid zombie TTF_Font objects. this is not a memory leak or
    // correctness issue, but supporting the use case of really removing a font
    // completely from the system.
    if (v_ctx__has_gfx_feature(V_GFX_USES_TEXT_MESH) &&
        node->res.gfx_text_mesh) {
      v_gfx_text_mesh_clear_font(v_gfx(), node->res.gfx_text_mesh);
    }

    if (node->font) {
      v_resource__unsubscribe(node->font_sub);
      node->font_sub = NULL;
      v_resource__release(node->font);
      node->font = NULL;
    }

    node->gfx_font_face = NULL;
    node->font_size = 0;
  }
}

VUID_PRIVATE void v_node__clear_font_face(VNode* node) {
  if (v_node__has_flag(node, V_NODEFLAG_HAS_FONT)) {
    // clear face, but keep font resource. the user might be changing font size
    v_node__clear_flag(node, V_NODEFLAG_HAS_FONT);
    node->gfx_font_face = NULL;
    node->font_size = 0;
  }
}

VUID_PRIVATE void v_node__on_resource_event(VNode* node,
                                            VResourceEvent event,
                                            VResource* resource) {
  if (event == V_RESOURCE_EVENT_REMOVE && node->font == resource) {
    v_node__clear_font(node);
    v_node_mark_dirty(node);
  }
}

// use rules for html ids
static bool v_node__is_legal_id(const char* id) {
  // empty string is legal
  if (*id == '\0') {
    return true;
  }

  if (!v_char_is_alpha(*id)) {
    return false;
  }

  id++;

  while (*id) {
    int c = *id++;

    if (!(v_char_is_alpha(c) || v_char_is_digit(c) || c == '_' || c == '-' ||
          c == '.' || c == ':')) {
      return false;
    }
  }

  return true;
}

static VWeakNodeRef* v_node__get_weak_ref(VNode* node) {
  VWeakNodeRef* result = node->self_weak_ref;

  if (!result) {
    result = v_ctx_new(VWeakNodeRef);

    // TODO: handle allocation failure
    assert(result);

    if (!result) {
      return NULL;
    }

    v_ctx()->weak_ref_count++;
    result->ref_count = 1;  // this ref is for the node itself
    result->ref = node;
    node->self_weak_ref = result;
  }

  return v_node__aquire_weak_ref(result);
}

static VWeakNodeRef* v_node__aquire_weak_ref(VWeakNodeRef* weak_ref) {
  if (weak_ref->ref_count < UINT32_MAX) {
    weak_ref->ref_count++;
  } else {
    assert(false && "VWeakNodeRef ref count overflow");
  }

  return weak_ref;
}

static void v_node__release_weak_ref(VWeakNodeRef* weak_ref) {
  if (weak_ref->ref_count > 1) {
    weak_ref->ref_count--;
  } else if (weak_ref->ref_count == 1) {
    v_ctx_delete(weak_ref, VWeakNodeRef);
    v_ctx()->weak_ref_count--;
  } else {
    assert(false && "VWeakNodeRef ref count underflow");
  }
}

static bool v_node__has_parent(const VNode* node) {
  return node->parent && node->parent->ref;
}

static void v_node__sync_text_resource(VNode* text_node) {
  if (v_ctx__has_gfx_feature(V_GFX_USES_TEXT_MESH)) {
    // TODO: this op can fail
    v_gfx_text_mesh_update(v_gfx(), &text_node->res.gfx_text_mesh,
                           v_stri_str(&text_node->res_data.text),
                           v_stri_size(&text_node->res_data.text));
  }
}

static bool v_node__can_insert(const VNode* parent, const VNode* child) {
  return parent && child && !v_node__has_flag(parent, V_NODEFLAG_LEAF) &&
         parent != child && child->tag != V_NODE_ROOT &&
         !v_node__has_parent(child);
}

#define VUID_INT_ID_AS_CSTR(VAR, ID)                     \
  char temp_buf[32];                                     \
  snprintf(temp_buf, sizeof(temp_buf), "%i", (int)(ID)); \
  const char* VAR = temp_buf;

typedef struct VFontFace {
  VGfxFontFace* gfx_face;
  uint16_t font_size;
} VFontFace;

// clang-format off
static void     v_resource__destructor(VResource* resource);
static void     v_resource__ref(VResource* resource);
static void     v_resource__unref(VResource* resource);
static uint32_t v_resource__hash_raw(VResourceType type, const char* id);
static void     v_resource__revoke(VResource* resource);
static void     v_resource__release_gfx(VResource* resource);
static void     v_resource__load_image(VResource* resource, const VGfxDataSource* source, float width, float height, VGfxTexture* gfx_texture, bool owns_gfx_texture);
static void     v_resource__load_font(VResource* resource, uint16_t font_id, const VGfxDataSource* source, VGfxFont* gfx_font, bool owns_gfx_font);
static void     v_resource__init_font_fields(VResource* resource);

static void       v_resources__remove(VResourceType type, const char* id);
static VResource* v_resources__find(VResourceType type, const char* id);
static VResource* v_resources__insert(VResourceType type, const char* id);

static VResourceSub* v_rsl__subscribe(VResourceSubList* rsl, VNode* node);
static void          v_rsl__unsubscribe(VResourceSubList* rsl, VResourceSub* sub);
static void          v_rsl__send_resource_event(VResourceSubList* rsl, VResourceEvent event, VResource* resource);
static void          v_rsl__drop(VResourceSubList* rsl);
static void          v_rsl__delete_subscription(VResourceSubList* rsl, VResourceSub* sub);

static void v__font_face_free(void* item, void* ctx);
// clang-format on

bool v_add_image(const char* src) {
  if (v_cstr_is_empty(src))
    return false;
  if (v_resources__find(V_RESOURCE_TYPE_IMAGE, src))
    return false;

  VResource* resource = v_resources__insert(V_RESOURCE_TYPE_IMAGE, src);

  if (!resource) {
    return false;
  }

  const VGfxDataSource source = {.u.path = src, .type = V_GFX_DATA_SOURCE_PATH};

  v_resource__load_image(resource, &source, 0, 0, NULL, false);
  resource->is_persistent = true;

  return resource;
}

bool v_add_image_mem(const char* src, const void* data, size_t size) {
  if (v_cstr_is_empty(src) || size == 0)
    return false;
  if (v_resources__find(V_RESOURCE_TYPE_IMAGE, src))
    return false;

  VResource* resource = v_resources__insert(V_RESOURCE_TYPE_IMAGE, src);

  if (!resource) {
    return false;
  }

  const VGfxDataSource source = {
      .u.buffer = {.data = data, .size = size},
      .type = V_GFX_DATA_SOURCE_BUFFER,
  };

  v_resource__load_image(resource, &source, 0, 0, NULL, false);
  resource->is_persistent = true;

  return resource;
}

bool v_add_image_raw(const char* src,
                     VGfxTexture* gfx_texture,
                     VSize size,
                     bool owns_gfx_texture) {
  if (v_cstr_is_empty(src))
    return false;
  if (v_resources__find(V_RESOURCE_TYPE_IMAGE, src))
    return false;

  VResource* resource = v_resources__insert(V_RESOURCE_TYPE_IMAGE, src);

  if (!resource) {
    return false;
  }

  v_resource__load_image(resource, NULL, size.width, size.height, gfx_texture,
                         owns_gfx_texture);
  resource->is_persistent = true;

  return resource;
}

void v_remove_image(const char* src) {
  if (!v_cstr_is_empty(src))
    v_resources__remove(V_RESOURCE_TYPE_IMAGE, src);
}

bool v_add_font(uint16_t id, const char* path) {
  if (v_cstr_is_empty(path))
    return false;

  VUID_INT_ID_AS_CSTR(id_cstr, id);

  if (v_resources__find(V_RESOURCE_TYPE_FONT, id_cstr) != NULL)
    return false;

  VResource* resource = v_resources__insert(V_RESOURCE_TYPE_FONT, id_cstr);

  if (!resource) {
    return false;
  }

  const VGfxDataSource source = {
      .u.path = path,
      .type = V_GFX_DATA_SOURCE_PATH,
  };

  v_resource__load_font(resource, id, &source, NULL, false);
  resource->is_persistent = true;

  return resource;
}

bool v_add_font_mem(uint16_t id, const void* data, size_t size) {
  if (!size)
    return false;

  VUID_INT_ID_AS_CSTR(id_cstr, id);

  if (v_resources__find(V_RESOURCE_TYPE_FONT, id_cstr) != NULL) {
    return false;
  }

  VResource* resource = v_resources__insert(V_RESOURCE_TYPE_FONT, id_cstr);

  if (!resource) {
    return false;
  }

  const VGfxDataSource source = {
      .u.buffer = {.data = data, .size = size},
      .type = V_GFX_DATA_SOURCE_BUFFER,
  };

  v_resource__load_font(resource, id, &source, NULL, false);

  return resource;
}

bool v_add_font_raw(uint16_t id, VGfxFont* gfx_font, bool owns_gfx_font) {
  if (!gfx_font)
    return false;

  VUID_INT_ID_AS_CSTR(id_cstr, id);

  if (v_resources__find(V_RESOURCE_TYPE_FONT, id_cstr) != NULL) {
    return false;
  }

  VResource* resource = v_resources__insert(V_RESOURCE_TYPE_FONT, id_cstr);

  if (!resource) {
    return false;
  }

  v_resource__load_font(resource, id, NULL, gfx_font, owns_gfx_font);

  return resource;
}

void v_remove_font(uint16_t id) {
  VUID_INT_ID_AS_CSTR(id_cstr, id);
  v_resources__remove(V_RESOURCE_TYPE_FONT, id_cstr);
}

VUID_PRIVATE VResource* v_resource__acquire_image(const char* src) {
  VResource* resource = v_resources__find(V_RESOURCE_TYPE_IMAGE, src);

  if (!resource) {
    resource = v_resources__insert(V_RESOURCE_TYPE_IMAGE, src);

    if (!resource) {
      return NULL;
    }

    const VGfxDataSource source = {
        .u.path = src,
        .type = V_GFX_DATA_SOURCE_PATH,
    };

    v_resource__load_image(resource, &source, 0, 0, NULL, false);
  }

  v_resource__ref(resource);
  return resource;
}

VUID_PRIVATE VResource* v_resource__acquire_font(uint16_t id) {
  VUID_INT_ID_AS_CSTR(id_cstr, id);

  VResource* resource = v_resources__find(V_RESOURCE_TYPE_FONT, id_cstr);

  if (resource) {
    v_resource__ref(resource);
  }

  return resource;
}

VUID_PRIVATE VResource* v_resource__peek_font(uint16_t id) {
  VUID_INT_ID_AS_CSTR(id_cstr, id);

  return v_resources__find(V_RESOURCE_TYPE_FONT, id_cstr);
}

VUID_PRIVATE void v_resource__release(VResource* resource) {
  // 2: resource manager + caller => after release no client own the resource
  if (resource->refs == 2 && !resource->is_persistent) {
    VContext* ctx = v_ctx();
    v_resources_remove_by_value(&ctx->resources,
                                &(VResourcesValue){.resource = resource});
  }

  v_resource__unref(resource);
}

VUID_PRIVATE VGfxFontFace* v_resource__get_or_create_font_face(
    VResource* font,
    uint16_t font_size) {
  assert(v_ctx__has_gfx_feature(V_GFX_USES_FONT_FACE));

  if (font->type != V_RESOURCE_TYPE_FONT ||
      font->state != V_RESOURCE_STATE_READY) {
    return NULL;
  }

  VArray* font_faces = &font->u.font.font_faces;
  const size_t face_count = font_faces->size;
  VFontFace* items = (VFontFace*)font_faces->items;

  for (size_t i = 0; i < face_count; i++) {
    VFontFace* font_face = &items[i];
    if (font_face->font_size == font_size) {
      return font_face->gfx_face;
    }
  }

  VFontFace* new_face = v_array_push_one(font_faces);

  if (!new_face) {
    return NULL;
  }

  VGfxFontFace* gfx_font_face =
      v_gfx_font_face_new(v_gfx(), font->u.font.gfx_font, (float)font_size);

  if (!gfx_font_face) {
    v_array_pop(font_faces);
    return NULL;
  }

  *new_face = (VFontFace){
      .gfx_face = gfx_font_face,
      .font_size = font_size,
  };

  return gfx_font_face;
}

VUID_PRIVATE VResourceSub* v_resource__subscribe(VResource* resource,
                                                 VNode* node) {
  assert(resource);
  assert(node);
  return v_rsl__subscribe(&resource->subscribers, node);
}

VUID_PRIVATE void v_resource__unsubscribe(VResourceSub* node_slot) {
  if (node_slot) {
    v_rsl__unsubscribe(&node_slot->node->font->subscribers, node_slot);
  }
}

static void v_resource__destructor(VResource* resource) {
  v_rsl__drop(&resource->subscribers);
  v_resource__release_gfx(resource);
  v_stri_drop(&resource->id);
  v_ctx_delete(resource, VResource);
  v_ctx()->resource_count--;
}

static void v_resource__load_image(VResource* resource,
                                   const VGfxDataSource* source,
                                   float width,
                                   float height,
                                   VGfxTexture* gfx_texture,
                                   bool owns_gfx_texture) {
  if (source != NULL) {
    VSize size;
    gfx_texture = v_gfx_new_image(v_gfx(), source, &size);
    owns_gfx_texture = true;
    width = size.width;
    height = size.height;
  }

  resource->u.image.gfx_texture = gfx_texture;
  resource->u.image.owns_gfx_texture = owns_gfx_texture;
  resource->u.image.dimensions.width = width;
  resource->u.image.dimensions.height = height;

  // TODO: not sure about this, especially for v_add_image* calls
  resource->state =
      gfx_texture ? V_RESOURCE_STATE_READY : V_RESOURCE_STATE_ERROR;
}

static void v_resource__load_font(VResource* resource,
                                  uint16_t font_id,
                                  const VGfxDataSource* source,
                                  VGfxFont* gfx_font,
                                  bool owns_gfx_font) {
  if (source != NULL) {
    gfx_font = v_gfx_font_new(v_gfx(), source);
    owns_gfx_font = true;
  }

  resource->u.font.gfx_font = gfx_font;
  resource->u.font.owns_gfx_font = owns_gfx_font;
  resource->u.font.font_id = font_id;

  // TODO: not sure about this, especially for v_add_font* calls
  resource->state = gfx_font ? V_RESOURCE_STATE_READY : V_RESOURCE_STATE_ERROR;
}

static void v_resource__init_font_fields(VResource* resource) {
  memset(&resource->u.font, 0, sizeof(resource->u.font));
  resource->u.font.font_faces = v_array_init(sizeof(VFontFace), 0);
}

static void v_resource__release_gfx(VResource* resource) {
  switch (resource->type) {
    case V_RESOURCE_TYPE_IMAGE:
      if (resource->u.image.gfx_texture && resource->u.image.owns_gfx_texture) {
        v_gfx_object_free(v_gfx(), V_GFX_OBJECT_IMAGE,
                          resource->u.image.gfx_texture);
        resource->u.image.gfx_texture = NULL;
      }
      break;
    case V_RESOURCE_TYPE_FONT: {
      VArray* font_faces = &resource->u.font.font_faces;

      if (v_ctx__has_gfx_feature(V_GFX_USES_FONT_FACE)) {
        v_array_apply(font_faces, &v__font_face_free, v_gfx());
        v_array_drop(font_faces);
      } else {
        assert(font_faces->size == 0);
      }

      if (resource->u.font.gfx_font && resource->u.font.owns_gfx_font) {
        v_gfx_object_free(v_gfx(), V_GFX_OBJECT_FONT,
                          resource->u.font.gfx_font);
      }
      break;
    }
    default:
      assert(false && "invalid resource type");
      break;
  }
}

static void v_resource__revoke(VResource* resource) {
  v_resource__release_gfx(resource);

  switch (resource->type) {
    case V_RESOURCE_TYPE_IMAGE:
      memset(&resource->u, 0, sizeof(resource->u));
      break;
    case V_RESOURCE_TYPE_FONT: {
      v_resource__init_font_fields(resource);
      break;
    }
    default:
      assert(false && "invalid resource type");
      break;
  }

  resource->state = V_RESOURCE_STATE_ERROR;
}

static void v_resource__ref(VResource* resource) {
  if (resource) {
    if (resource->refs < UINT32_MAX) {
      resource->refs++;
    } else {
      assert(false && "VResource ref count overflow");
    }
  }
}

static void v_resource__unref(VResource* resource) {
  if (resource) {
    if (resource->refs > 1) {
      resource->refs--;
    } else if (resource->refs == 1) {
      v_resource__destructor(resource);
    } else {
      assert(false && "VResource ref count underflow");
    }
  }
}

static uint32_t v_resource__hash_raw(VResourceType type, const char* id) {
  return v_fnv1_hash_mix(v_fnv1_hash_u32((uint32_t)type), v_fnv1_hash(id));
}

static VResource* v_resources__find(VResourceType type, const char* id) {
  const VResourcesValue* value = v_resources_get(
      &v_ctx()->resources, &(VResourcesKey){.type = type, .id = id});

  return value ? value->resource : NULL;
}

static VResource* v_resources__insert(VResourceType type, const char* id) {
  VResource* resource = v_ctx_new(VResource);

  if (!resource) {
    return NULL;
  }

  resource->refs = 1;
  resource->state = V_RESOURCE_STATE_INIT;
  resource->type = type;
  resource->hash = v_resource__hash_raw(type, id);
  v_stri_assign(&resource->id, id);  // TODO: this can fail

  if (type == V_RESOURCE_TYPE_FONT) {
    v_resource__init_font_fields(resource);
    resource->is_persistent = true;
  }

  VContext* ctx = v_ctx();
  const bool inserted =
      v_resources_put(&ctx->resources, (VResourcesValue){.resource = resource});

  if (!inserted) {
    // put drops cleans up value on failure
    return NULL;
  }

  ctx->resource_count++;

  return resource;
}

static void v_resources__remove(VResourceType type, const char* id) {
  VResource* resource = v_resources__find(type, id);

  if (resource == NULL || resource->has_pending_remove) {
    return;
  }

  VContext* ctx = v_ctx();

  resource->has_pending_remove = true;
  resource->is_persistent = false;
  v_resource__revoke(resource);

  v_resources_remove_by_value(&ctx->resources,
                              &(VResourcesValue){.resource = resource});

  v_rsl__send_resource_event(&resource->subscribers, V_RESOURCE_EVENT_REMOVE,
                             resource);
}

static void v__font_face_free(void* item, void* ctx) {
  VFontFace* font_face = (VFontFace*)item;
  v_gfx_object_free(ctx, V_GFX_OBJECT_FONT_FACE, font_face->gfx_face);
}

static VResourceSub* v_rsl__subscribe(VResourceSubList* rsl, VNode* node) {
  VResourceSub* sub = v_ctx_new(VResourceSub);

  if (!sub) {
    return NULL;
  }

  sub->node = node;
  sub->prev = NULL;
  sub->next = rsl->active;
  if (rsl->active) {
    rsl->active->prev = sub;
  }
  rsl->active = sub;
  rsl->active_count++;

  return sub;
}

static void v_rsl__unsubscribe(VResourceSubList* rsl, VResourceSub* sub) {
  if (rsl->dispatch_depth > 0) {
    sub->node = NULL;
    rsl->pending_removes++;
    rsl->active_count--;
  } else {
    v_rsl__delete_subscription(rsl, sub);
    rsl->active_count--;
  }
}

static void v_rsl__send_resource_event(VResourceSubList* rsl,
                                       VResourceEvent event,
                                       VResource* resource) {
  VResourceSub* sub = rsl->active;

  rsl->dispatch_depth++;
  while (sub) {
    if (sub->node) {
      v_node__on_resource_event(sub->node, event, resource);
    }
    sub = sub->next;
  }
  rsl->dispatch_depth--;

  if (rsl->dispatch_depth == 0 && rsl->pending_removes > 0) {
    sub = rsl->active;

    while (sub) {
      VResourceSub* next = sub->next;

      if (sub->node == NULL) {
        v_rsl__delete_subscription(rsl, sub);

        if (--rsl->pending_removes == 0) {
          break;
        }
      }

      sub = next;
    }
  }
}

static void v_rsl__drop(VResourceSubList* rsl) {
  assert(rsl->active_count == 0);
  VResourceSub* sub = rsl->active;

  while (sub) {
    VResourceSub* next = sub->next;
    v_rsl__delete_subscription(rsl, sub);
    sub = next;
  }
}

static void v_rsl__delete_subscription(VResourceSubList* rsl,
                                       VResourceSub* sub) {
  VResourceSub* prev = sub->prev;
  VResourceSub* next = sub->next;

  if (prev) {
    prev->next = next;
  }

  if (next) {
    next->prev = prev;
  }

  if (sub == rsl->active) {
    rsl->active = next;
  }

  v_ctx_delete(sub, VResourceSub);
}

//
// resource manager hashset implementation
//

static uint32_t VResourcesValue_get_hash(const VResourcesValue* value) {
  return value->resource->hash;
}

static uint32_t VResourcesKey_get_hash(const VResourcesKey* key) {
  return v_resource__hash_raw(key->type, key->id);
}

static VResourcesKey VResourcesKey_from_value(const VResourcesValue* value) {
  return (VResourcesKey){
      .type = value->resource->type,
      .id = v_stri_str(&value->resource->id),
  };
}

static bool VResourcesValue_eq(const VResourcesKey* key,
                               const VResourcesValue* value) {
  return key->type == value->resource->type &&
         v_stri_eq_cstr(&value->resource->id, key->id);
}

static void VResourcesValue_drop(VResourcesValue* value) {
  v_resource__unref(value->resource);
}

V_HSET_IMPL(VResources, VResourcesKey, VResourcesValue, v_resources)

VUID_PRIVATE char* v_stri_reserve(VStringInternal* self, size_t cap) {
  const size_t real_cap = (cap > V_STRI_MAX_CAP) ? V_STRI_MAX_CAP : cap;

  if (v_stri_is_long(self)) {
    if (real_cap > v_stri_l_cap(self)) {
      self->lon.data = v_ctx_alloc(V_AOP_REALLOC, self->lon.data,
                                   v_stri_l_cap(self) + 1, real_cap + 1);
      v_stri_l_set_cap(self, real_cap);
    }
    return self->lon.data;
  } else if (real_cap > V_STRI_S_CAP) {
    size_t len = v_stri_s_size(self);
    // TODO: ??
    char* data = v_ctx_alloc(V_AOP_ALLOC, NULL, 0, real_cap + 1);

    memcpy(data, self->sml.data, len + 1);
    self->lon.data = data;
    self->lon.size = len;
    v_stri_l_set_cap(self, real_cap);

    return data;
  } else {
    return self->sml.data;
  }
}

VUID_PRIVATE void v_stri_assign(VStringInternal* self, const char* str) {
  v_stri_assign_n(self, str, str ? strlen(str) : 0);
}

VUID_PRIVATE void v_stri_assign_n(VStringInternal* self,
                                  const char* str,
                                  size_t size) {
  const size_t str_size = (size > V_STRI_MAX_CAP) ? V_STRI_MAX_CAP : size;

  if (str_size == 0) {
    v_stri_clear(self);
    return;
  }

  char* data = v_stri_reserve(self, str_size);

  if (!data) {
    // TODO: assert? return false?
    return;
  }

  memmove(data, str, str_size);

  if (v_stri_is_long(self)) {
    v_stri_l_set_size(self, str_size);
  } else {
    v_stri_s_set_size(self, str_size);
  }
}

VUID_PRIVATE bool v_stri_eq(const VStringInternal* a,
                            const VStringInternal* b) {
  const size_t a_size = v_stri_size(a);
  const size_t b_size = v_stri_size(b);

  if (a_size != b_size) {
    return false;
  }

  if (a_size == 0) {
    return true;
  }

  const char* a_buf = v_stri_str(a);
  const char* b_buf = v_stri_str(b);

  return a_buf == b_buf || memcmp(a_buf, b_buf, a_size) == 0;
}

VUID_PRIVATE bool v_stri_eq_cstr_n(const VStringInternal* a,
                                   const char* b,
                                   size_t b_size) {
  const size_t a_size = v_stri_size(a);

  if (a_size != b_size) {
    return false;
  }

  if (a_size == 0) {
    return true;
  }

  const char* a_buf = v_stri_str(a);

  return a_buf == b || memcmp(a_buf, b, b_size) == 0;
}

VUID_PRIVATE bool v_stri_eq_cstr(const VStringInternal* a, const char* b) {
  const size_t a_size = v_stri_size(a);

  if (b == NULL || *b == '\0') {
    return a_size == 0;
  }

  if (a_size == 0) {
    return false;
  }

  return strcmp(v_stri_str(a), b) == 0;
}

VUID_PRIVATE bool v_stri_ieq(const VStringInternal* a,
                             const VStringInternal* b) {
  const size_t a_size = v_stri_size(a);
  const size_t b_size = v_stri_size(b);

  if (a_size != b_size) {
    return false;
  }

  if (a_size == 0) {
    return true;
  }

  const char* a_buf = v_stri_str(a);
  const char* b_buf = v_stri_str(b);

  if (a_buf == b_buf) {
    return true;
  }

  for (size_t i = 0; i < a_size; i++) {
    if (v_char_tolower(a_buf[i]) != v_char_tolower(b_buf[i])) {
      return false;
    }
  }

  return true;
}

VUID_PRIVATE void v_stri_drop(VStringInternal* self) {
  if (v_stri_is_long(self)) {
    v_ctx_alloc(V_AOP_FREE, self->lon.data, self->lon.size + 1, 0);
  }
}

VUID_PRIVATE void v_stri_assign_vfmt(VStringInternal* self,
                                     const char* fmt,
                                     va_list args) {
  va_list args2;
  va_copy(args2, args);
  const int n = vsnprintf(NULL, 0ULL, fmt, args);

  if (n < 0) {
    // TODO: assert?
    return;
  }

  const size_t len = (size_t)n;
  // TODO: this is an alloc..
  vsnprintf(v_stri_reserve(self, len), len + 1, fmt, args2);
  va_end(args2);

  if (v_stri_is_long(self)) {
    v_stri_l_set_size(self, len);
  } else {
    v_stri_s_set_size(self, len);
  }
}

void v_style_reset(VStyle* style) {
  if (style && style->is_set) {
    VNode* owner = style->owner;

    // TODO: send event to owner
    if (owner) {
      if (owner->tag == V_NODE_TEXT) {
        // TODO: must call unset functions to notify owner of changes (ugly)
        if (vs_has_prop(style, VS_FONT)) {
          vs_unset_font(style);
        }

        if (vs_has_prop(style, VS_FONT_SIZE)) {
          vs_unset_font_size(style);
        }
      }

      // ok, for now since most props don't need cleanup or have side effects
      style->is_set = 0;
      v_node_mark_dirty(owner);
    } else {
      style->is_set = 0;
    }
  }
}

void v_style_ref(VStyle* style) {
  if (style) {
    if (style->ref_count < UINT32_MAX) {
      style->ref_count++;
    } else {
      assert(false && "VStyle ref count overflow");
    }
  }
}

void v_style_unref(VStyle* style) {
  if (style) {
    const uint32_t ref_count = style->ref_count;

    if (ref_count > 1) {
      style->ref_count--;
    } else if (ref_count == 1) {
      v_ctx_delete(style, VStyle);
      v_ctx()->style_count--;
    } else {
      assert(false && "VStyle ref count underflow");
    }
  }
}

uint32_t v_style_ref_count(const VStyle* style) {
  return style ? style->ref_count : 0;
}

// TODO: not the api i want, but sdli needs this
float v_style_measure_text_w(const VStyle* style, const char* text) {
  if (v_cstr_is_empty(text) || !style || !vs_has_font(style) ||
      !vs_has_font_size(style)) {
    return 0.f;
  }

  const uint16_t font_id = vs_get_font(style);
  const uint16_t font_size = vs_get_font_size(style);
  VResource* font = v_resource__peek_font(font_id);

  if (font == NULL) {
    return 0.f;
  }

  VGfxFontFace* gfx_font_face;

  if (v_ctx__has_gfx_feature(V_GFX_USES_FONT_FACE)) {
    gfx_font_face = v_resource__get_or_create_font_face(font, font_size);

    if (gfx_font_face == NULL) {
      return 0.f;
    }
  } else {
    gfx_font_face = NULL;
  }

  return v_gfx_measure_string(v_gfx(), font->u.font.gfx_font, (float)font_size,
                              gfx_font_face, text, strlen(text),
                              V_GFX_MEASURE_MODE_TEXT, 0)
      .width;
}

VUID_PRIVATE VStyle* v_style__new(VNode* owner) {
  VStyle* style = v_ctx_new(VStyle);

  if (style) {
    style->owner = owner;
    style->ref_count = 1;
    v_ctx()->style_count++;
  }

  return style;
}

#define VUID_EQ(A, B) ((A) == (B))

// TODO: send event to owner, rather than doing the owner's work here
#define VUID_PROPERTY_UPDATE_OWNER(STYLE, PROPERTY)     \
  do {                                                  \
    VNode* node = (STYLE)->owner;                       \
                                                        \
    if (node) {                                         \
      v_node_mark_dirty(node);                          \
                                                        \
      if (g_style_props[PROPERTY].affects_text_style && \
          node->tag == V_NODE_TEXT) {                   \
        if (PROPERTY == VS_FONT) {                      \
          v_node__clear_font(node);                     \
        }                                               \
        if (PROPERTY == VS_FONT_SIZE) {                 \
          v_node__clear_font_face(node);                \
        }                                               \
      }                                                 \
    }                                                   \
  } while (0)

#define VUID_PROPERTY_FUNCTIONS_IMPL(PROPERTY, FIELD, TYPE, CMP, META_KEY) \
  void vs_set_##FIELD(VStyle* style, TYPE value) {                         \
    if (!style) {                                                          \
      return;                                                              \
    }                                                                      \
                                                                           \
    if (vs_has_prop(style, PROPERTY) && CMP(style->FIELD, value)) {        \
      return;                                                              \
    }                                                                      \
                                                                           \
    style->FIELD = value;                                                  \
    style->is_set |= (((uint64_t)1) << PROPERTY);                          \
    VUID_PROPERTY_UPDATE_OWNER(style, PROPERTY);                           \
  }                                                                        \
                                                                           \
  TYPE vs_get_##FIELD(const VStyle* style) {                               \
    return (style && vs_has_prop(style, PROPERTY))                         \
               ? style->FIELD                                              \
               : g_style_props[PROPERTY].default_value.META_KEY;           \
  }                                                                        \
                                                                           \
  void vs_unset_##FIELD(VStyle* style) {                                   \
    const uint64_t PROPERTY_MASK = (((uint64_t)1) << PROPERTY);            \
    if (style && (style->is_set & PROPERTY_MASK)) {                        \
      style->is_set &= ~PROPERTY_MASK;                                     \
      VUID_PROPERTY_UPDATE_OWNER(style, PROPERTY);                         \
    }                                                                      \
  }                                                                        \
  bool vs_has_##FIELD(const VStyle* style) {                               \
    return (style && vs_has_prop(style, PROPERTY));                        \
  }

typedef enum VStylePropertyTag {
  VSTAG_UNSET,
  VSTAG_SIZING,
  VSTAG_COLOR,
  VSTAG_UINT,
  VSTAG_ENUM_DIRECTION,
  VSTAG_ENUM_WRAP,
  VSTAG_ENUM_XALIGN,
  VSTAG_ENUM_YALIGN,
  VSTAG_ENUM_OVERFLOW,
  VSTAG_ENUM_TEXT_WRAP,
  VSTAG_ENUM_ANCHOR_TO,
  VSTAG_ENUM_ATTACH_POINT_X,
  VSTAG_ENUM_ATTACH_POINT_Y,
  VSTAG_ENUM_POSITION,
  VSTAG_FLOAT,
} VStylePropertyTag;

typedef struct VStylePropertyMeta {
  VStylePropertyTag tag;

  union {
    VSizing sizing;
    VColor color;
    uint16_t uint;
    float fval;
    VDirection direction;
    VWrap wrap;
    VAlignX xalign;
    VAlignY yalign;
    VAlignX talign;
    VTextWrap text_wrap;
    VOverflow overflow;
    VAnchorTo anchor_to;
    VAttachPointX attach_point_x;
    VAttachPointY attach_point_y;
    VPosition position;
  } default_value;

  union {
    void (*sizing)(VStyle*, VSizing);
    void (*color)(VStyle*, VColor);
    void (*uint)(VStyle*, uint16_t);
    void (*fval)(VStyle*, float);
    void (*direction)(VStyle*, VDirection);
    void (*wrap)(VStyle*, VWrap);
    void (*xalign)(VStyle*, VAlignX);
    void (*yalign)(VStyle*, VAlignY);
    void (*overflow)(VStyle*, VOverflow);
    void (*anchor_to)(VStyle*, VAnchorTo);
    void (*attach_point_x)(VStyle*, VAttachPointX);
    void (*attach_point_y)(VStyle*, VAttachPointY);
    void (*position)(VStyle*, VPosition);
    void (*text_wrap)(VStyle*, VTextWrap);
  } set_fn;

  union {
    VSizing (*sizing)(const VStyle*);
    VColor (*color)(const VStyle*);
    uint16_t (*uint)(const VStyle*);
    float (*fval)(const VStyle*);
    VDirection (*direction)(const VStyle*);
    VWrap (*wrap)(const VStyle*);
    VAlignX (*xalign)(const VStyle*);
    VAlignY (*yalign)(const VStyle*);
    VOverflow (*overflow)(const VStyle*);
    VAnchorTo (*anchor_to)(const VStyle*);
    VAttachPointX (*attach_point_x)(const VStyle*);
    VAttachPointY (*attach_point_y)(const VStyle*);
    VPosition (*position)(const VStyle*);
    VTextWrap (*text_wrap)(const VStyle*);
  } get_fn;

  bool affects_text_style;
} VStylePropertyMeta;

static bool vs_float_eq(float a, float b);
static bool vs_color_eq(VColor a, VColor b);
static bool vs_sizing_eq(VSizing a, VSizing b);

static const VStylePropertyMeta g_style_props[VS__STYLE_PROPERTY_COUNT] = {
    // clang-format off
    [VS_WIDTH] = {
      .tag = VSTAG_SIZING,
      .default_value.sizing = {V_SIZING_FIT},
      .set_fn.sizing = &vs_set_width,
      .get_fn.sizing = &vs_get_width,
    },
    [VS_HEIGHT] = {
      .tag = VSTAG_SIZING,
      .default_value.sizing = {V_SIZING_FIT},
      .set_fn.sizing = &vs_set_height,
      .get_fn.sizing = &vs_get_height,
    },
    [VS_DIRECTION] = {
      .tag = VSTAG_ENUM_DIRECTION,
      .default_value.direction = V_DIRECTION_ROW,
      .set_fn.direction = &vs_set_direction,
      .get_fn.direction = &vs_get_direction,
    },
    [VS_WRAP] = {
      .tag = VSTAG_ENUM_WRAP,
      .default_value.wrap = V_WRAP_NONE,
      .set_fn.wrap = &vs_set_wrap,
      .get_fn.wrap = &vs_get_wrap,
    },
    [VS_XALIGN] = {
      .tag = VSTAG_ENUM_XALIGN,
      .default_value.xalign = V_ALIGN_X_LEFT,
      .set_fn.xalign = &vs_set_xalign,
      .get_fn.xalign = &vs_get_xalign,
    },
    [VS_YALIGN] = {
      .tag = VSTAG_ENUM_YALIGN,
      .default_value.yalign = V_ALIGN_Y_TOP,
      .set_fn.yalign = &vs_set_yalign,
      .get_fn.yalign = &vs_get_yalign,
    },
    [VS_TALIGN] = {
      .tag = VSTAG_ENUM_XALIGN,
      .default_value.talign = V_ALIGN_X_LEFT,
      .set_fn.xalign = &vs_set_talign,
      .get_fn.xalign = &vs_get_talign,
    },
    [VS_TEXT_WRAP] = {
      .tag = VSTAG_ENUM_TEXT_WRAP,
      .default_value.text_wrap = V_TEXT_WRAP_NO_WRAP,
      .set_fn.text_wrap = &vs_set_text_wrap,
      .get_fn.text_wrap = &vs_get_text_wrap,
    },
    [VS_OVERFLOW] = {
      .tag = VSTAG_ENUM_OVERFLOW,
      .default_value.overflow = V_OVERFLOW_VISIBLE,
      .set_fn.overflow = &vs_set_overflow,
      .get_fn.overflow = &vs_get_overflow,
    },
    [VS_GAP] = {
      .tag = VSTAG_UINT,
      .default_value.uint = 0,
      .set_fn.uint = &vs_set_gap,
      .get_fn.uint = &vs_get_gap,
    },
    [VS_PADDING_TOP] = {
      .tag = VSTAG_UINT,
      .default_value.uint = 0,
      .set_fn.uint = &vs_set_padding_top,
      .get_fn.uint = &vs_get_padding_top,
    },
    [VS_PADDING_RIGHT] = {
      .tag = VSTAG_UINT,
      .default_value.uint = 0,
      .set_fn.uint = &vs_set_padding_right,
      .get_fn.uint = &vs_get_padding_right,
    },
    [VS_PADDING_BOTTOM] = {
      .tag = VSTAG_UINT,
      .default_value.uint = 0,
      .set_fn.uint = &vs_set_padding_bottom,
      .get_fn.uint = &vs_get_padding_bottom,
    },
    [VS_PADDING_LEFT] = {
      .tag = VSTAG_UINT,
      .default_value.uint = 0,
      .set_fn.uint = &vs_set_padding_left,
      .get_fn.uint = &vs_get_padding_left,
    },
    [VS_BORDER_TOP] = {
      .tag = VSTAG_UINT,
      .default_value.uint = 0,
      .set_fn.uint = &vs_set_border_top,
      .get_fn.uint = &vs_get_border_top,
    },
    [VS_BORDER_RIGHT] = {
      .tag = VSTAG_UINT,
      .default_value.uint = 0,
      .set_fn.uint = &vs_set_border_right,
      .get_fn.uint = &vs_get_border_right,
    },
    [VS_BORDER_BOTTOM] = {
      .tag = VSTAG_UINT,
      .default_value.uint = 0,
      .set_fn.uint = &vs_set_border_bottom,
      .get_fn.uint = &vs_get_border_bottom,
    },
    [VS_BORDER_LEFT] = {
      .tag = VSTAG_UINT,
      .default_value.uint = 0,
      .set_fn.uint = &vs_set_border_left,
      .get_fn.uint = &vs_get_border_left,
    },
    [VS_BORDER_RADIUS] = {
      .tag = VSTAG_UINT,
      .default_value.uint = 0,
      .set_fn.uint = &vs_set_border_radius,
      .get_fn.uint = &vs_get_border_radius,
    },
    [VS_FONT] = {
      .tag = VSTAG_UINT,
      .default_value.uint = 0,
      .set_fn.uint = &vs_set_font,
      .get_fn.uint = &vs_get_font,
      .affects_text_style = true,
    },
    [VS_FONT_SIZE] = {
      .tag = VSTAG_UINT,
      .default_value.uint = 0,
      .set_fn.uint = &vs_set_font_size,
      .get_fn.uint = &vs_get_font_size,
      .affects_text_style = true,
    },
    [VS_SCROLLBAR_WIDTH] = {
      .tag = VSTAG_UINT,
      .default_value.uint = 0,
      .set_fn.uint = &vs_set_scrollbar_width,
      .get_fn.uint = &vs_get_scrollbar_width,
    },
    [VS_SCROLLBAR_BORDER_RADIUS] = {
      .tag = VSTAG_UINT,
      .default_value.uint = 0,
      .set_fn.uint = &vs_set_scrollbar_border_radius,
      .get_fn.uint = &vs_get_scrollbar_border_radius,
    },
    [VS_BACKGROUND] = {
      .tag = VSTAG_COLOR,
      .default_value.color = {0, 0, 0, 0},
      .set_fn.color = &vs_set_background,
      .get_fn.color = &vs_get_background,
    },
    [VS_COLOR] = {
      .tag = VSTAG_COLOR,
      .default_value.color = {0, 0, 0, 0},
      .set_fn.color = &vs_set_color,
      .get_fn.color = &vs_get_color,
    },
    [VS_BORDER_COLOR] = {
      .tag = VSTAG_COLOR,
      .default_value.color = {0, 0, 0, 0},
      .set_fn.color = &vs_set_border_color,
      .get_fn.color = &vs_get_border_color,
    },
    [VS_SCROLLBAR_THUMB] = {
      .tag = VSTAG_COLOR,
      .default_value.color = {0, 0, 0, 0},
      .set_fn.color = &vs_set_scrollbar_thumb,
      .get_fn.color = &vs_get_scrollbar_thumb,
    },
    [VS_SCROLLBAR_THUMB_HOVER] = {
      .tag = VSTAG_COLOR,
      .default_value.color = {0, 0, 0, 0},
      .set_fn.color = &vs_set_scrollbar_thumb_hover,
      .get_fn.color = &vs_get_scrollbar_thumb_hover,
    },
    [VS_ANCHOR_TO] = {
      .tag = VSTAG_ENUM_ANCHOR_TO,
      .default_value.anchor_to = V_ANCHOR_TO_PARENT,
      .set_fn.anchor_to = &vs_set_anchor_to,
      .get_fn.anchor_to = &vs_get_anchor_to,
    },
    [VS_ANCHOR_ATTACH_POINT_X] = {
      .tag = VSTAG_ENUM_ATTACH_POINT_X,
      .default_value.attach_point_x = V_ATTACH_POINT_X_LEFT,
      .set_fn.attach_point_x = &vs_set_anchor_attach_point_x,
      .get_fn.attach_point_x = &vs_get_anchor_attach_point_x,
    },
    [VS_ANCHOR_ATTACH_POINT_Y] = {
      .tag = VSTAG_ENUM_ATTACH_POINT_Y,
      .default_value.attach_point_y = V_ATTACH_POINT_Y_TOP,
      .set_fn.attach_point_y = &vs_set_anchor_attach_point_y,
      .get_fn.attach_point_y = &vs_get_anchor_attach_point_y,
    },
    [VS_ATTACH_POINT_X] = {
      .tag = VSTAG_ENUM_ATTACH_POINT_X,
      .default_value.attach_point_x = V_ATTACH_POINT_X_LEFT,
      .set_fn.attach_point_x = &vs_set_attach_point_x,
      .get_fn.attach_point_x = &vs_get_attach_point_x,
    },
    [VS_ATTACH_POINT_Y] = {
      .tag = VSTAG_ENUM_ATTACH_POINT_Y,
      .default_value.attach_point_y = V_ATTACH_POINT_Y_TOP,
      .set_fn.attach_point_y = &vs_set_attach_point_y,
      .get_fn.attach_point_y = &vs_get_attach_point_y,
    },
    [VS_ATTACH_POINT_OFFSET_X] = {
      .tag = VSTAG_FLOAT,
      .default_value.fval = 0,
      .set_fn.fval = &vs_set_attach_point_offset_x,
      .get_fn.fval = &vs_get_attach_point_offset_x,
    },
    [VS_ATTACH_POINT_OFFSET_Y] = {
      .tag = VSTAG_FLOAT,
      .default_value.fval = 0,
      .set_fn.fval = &vs_set_attach_point_offset_y,
      .get_fn.fval = &vs_get_attach_point_offset_y,
    },
    [VS_ASPECT_RATIO] = {
      .tag = VSTAG_FLOAT,
      .default_value.fval = 0,
      .set_fn.fval = &vs_set_aspect_ratio,
      .get_fn.fval = &vs_get_aspect_ratio,
    },
    [VS_POSITION] = {
      .tag = VSTAG_ENUM_POSITION,
      .default_value.position = V_POSITION_STATIC,
      .set_fn.position = &vs_set_position,
      .get_fn.position = &vs_get_position,
    },
    [VS_TOP] = {
      .tag = VSTAG_FLOAT,
      .default_value.fval = 0,
      .set_fn.fval = &vs_set_top,
      .get_fn.fval = &vs_get_top,
    },
    [VS_RIGHT] = {
      .tag = VSTAG_FLOAT,
      .default_value.fval = 0,
      .set_fn.fval = &vs_set_right,
      .get_fn.fval = &vs_get_right,
    },
    [VS_BOTTOM] = {
      .tag = VSTAG_FLOAT,
      .default_value.fval = 0,
      .set_fn.fval = &vs_set_bottom,
      .get_fn.fval = &vs_get_bottom,
    },
    [VS_LEFT] = {
      .tag = VSTAG_FLOAT,
      .default_value.fval = 0,
      .set_fn.fval = &vs_set_left,
      .get_fn.fval = &vs_get_left,
    },
    // clang-format on
};

VUID_PRIVATE void v_style__flatten(VStyle* a, VStyle* b) {
  for (int i = 0; i < VS__STYLE_PROPERTY_COUNT; i++) {
    if (!vs_has_prop(b, (VStyleProperty)i)) {
      continue;
    }

    const VStylePropertyMeta* meta = &g_style_props[i];

    switch (meta->tag) {
      case VSTAG_SIZING:
        meta->set_fn.sizing(a, meta->get_fn.sizing(b));
        break;
      case VSTAG_COLOR:
        meta->set_fn.color(a, meta->get_fn.color(b));
        break;
      case VSTAG_UINT:
        meta->set_fn.uint(a, meta->get_fn.uint(b));
        break;
      case VSTAG_ENUM_DIRECTION:
        meta->set_fn.direction(a, meta->get_fn.direction(b));
        break;
      case VSTAG_ENUM_XALIGN:
        meta->set_fn.xalign(a, meta->get_fn.xalign(b));
        break;
      case VSTAG_ENUM_YALIGN:
        meta->set_fn.yalign(a, meta->get_fn.yalign(b));
        break;
      case VSTAG_ENUM_OVERFLOW:
        meta->set_fn.overflow(a, meta->get_fn.overflow(b));
        break;
      case VSTAG_ENUM_ANCHOR_TO:
        meta->set_fn.anchor_to(a, meta->get_fn.anchor_to(b));
        break;
      case VSTAG_ENUM_ATTACH_POINT_X:
        meta->set_fn.attach_point_x(a, meta->get_fn.attach_point_x(b));
        break;
      case VSTAG_ENUM_ATTACH_POINT_Y:
        meta->set_fn.attach_point_y(a, meta->get_fn.attach_point_y(b));
        break;
      case VSTAG_FLOAT:
        meta->set_fn.fval(a, meta->get_fn.fval(b));
        break;
      default:
        // TODO: unreachable
        break;
    }
  }
}

static bool vs_float_eq(float a, float b) {
  return fabs(a - b) < FLT_EPSILON;
}

static bool vs_color_eq(VColor a, VColor b) {
  return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

static bool vs_sizing_eq(VSizing a, VSizing b) {
  if (a.tag == b.tag) {
    switch (a.tag) {
      case V_SIZING_FIXED:
      case V_SIZING_FIT:
        return vs_float_eq(a.min, b.min) && vs_float_eq(a.max, b.max);
      case V_SIZING_GROW:
        return vs_float_eq(a.min, b.min);
      default:
        break;
    }
  }

  return false;
}

#ifdef _MSC_VER
// property specific ifs trigger C4127 warning
#pragma warning(push)
#pragma warning(disable : 4127)
#endif

VUID_PROPERTY_FUNCTIONS_IMPL(VS_WIDTH, width, VSizing, vs_sizing_eq, sizing)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_HEIGHT, height, VSizing, vs_sizing_eq, sizing)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_DIRECTION,
                             direction,
                             VDirection,
                             VUID_EQ,
                             direction)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_WRAP, wrap, VWrap, VUID_EQ, wrap)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_XALIGN, xalign, VAlignX, VUID_EQ, xalign)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_YALIGN, yalign, VAlignY, VUID_EQ, yalign)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_TALIGN, talign, VAlignX, VUID_EQ, talign)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_TEXT_WRAP,
                             text_wrap,
                             VTextWrap,
                             VUID_EQ,
                             text_wrap)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_OVERFLOW,
                             overflow,
                             VOverflow,
                             VUID_EQ,
                             overflow)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_GAP, gap, uint16_t, VUID_EQ, uint)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_PADDING_TOP,
                             padding_top,
                             uint16_t,
                             VUID_EQ,
                             uint)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_PADDING_RIGHT,
                             padding_right,
                             uint16_t,
                             VUID_EQ,
                             uint)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_PADDING_BOTTOM,
                             padding_bottom,
                             uint16_t,
                             VUID_EQ,
                             uint)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_PADDING_LEFT,
                             padding_left,
                             uint16_t,
                             VUID_EQ,
                             uint)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BORDER_TOP, border_top, uint16_t, VUID_EQ, uint)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BORDER_RIGHT,
                             border_right,
                             uint16_t,
                             VUID_EQ,
                             uint)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BORDER_BOTTOM,
                             border_bottom,
                             uint16_t,
                             VUID_EQ,
                             uint)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BORDER_LEFT,
                             border_left,
                             uint16_t,
                             VUID_EQ,
                             uint)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BORDER_RADIUS,
                             border_radius,
                             uint16_t,
                             VUID_EQ,
                             uint)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_FONT, font, uint16_t, VUID_EQ, uint)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_FONT_SIZE, font_size, uint16_t, VUID_EQ, uint)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_SCROLLBAR_WIDTH,
                             scrollbar_width,
                             uint16_t,
                             VUID_EQ,
                             uint)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_SCROLLBAR_BORDER_RADIUS,
                             scrollbar_border_radius,
                             uint16_t,
                             VUID_EQ,
                             uint)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BACKGROUND,
                             background,
                             VColor,
                             vs_color_eq,
                             color)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_COLOR, color, VColor, vs_color_eq, color)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BORDER_COLOR,
                             border_color,
                             VColor,
                             vs_color_eq,
                             color)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_SCROLLBAR_THUMB,
                             scrollbar_thumb,
                             VColor,
                             vs_color_eq,
                             color)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_SCROLLBAR_THUMB_HOVER,
                             scrollbar_thumb_hover,
                             VColor,
                             vs_color_eq,
                             color)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ANCHOR_TO,
                             anchor_to,
                             VAnchorTo,
                             VUID_EQ,
                             anchor_to)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ANCHOR_ATTACH_POINT_X,
                             anchor_attach_point_x,
                             VAttachPointX,
                             VUID_EQ,
                             attach_point_x)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ANCHOR_ATTACH_POINT_Y,
                             anchor_attach_point_y,
                             VAttachPointY,
                             VUID_EQ,
                             attach_point_y)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ATTACH_POINT_X,
                             attach_point_x,
                             VAttachPointX,
                             VUID_EQ,
                             attach_point_x)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ATTACH_POINT_Y,
                             attach_point_y,
                             VAttachPointY,
                             VUID_EQ,
                             attach_point_y)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ATTACH_POINT_OFFSET_X,
                             attach_point_offset_x,
                             float,
                             vs_float_eq,
                             fval)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ATTACH_POINT_OFFSET_Y,
                             attach_point_offset_y,
                             float,
                             vs_float_eq,
                             fval)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ASPECT_RATIO,
                             aspect_ratio,
                             float,
                             vs_float_eq,
                             fval)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_POSITION,
                             position,
                             VPosition,
                             VUID_EQ,
                             position)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_TOP, top, float, vs_float_eq, fval)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_RIGHT, right, float, vs_float_eq, fval)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BOTTOM, bottom, float, vs_float_eq, fval)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_LEFT, left, float, vs_float_eq, fval)

#ifdef _MSC_VER
#pragma warning(pop)
#endif

static bool v_is_legal_class_name(const char* name);
static bool v_style_class_init(const char* id, VStyleClass* out);

VStyle* vss__start_class(const char* name, const char* base) {
  if (!v_is_legal_class_name(name)) {
    return NULL;
  }

  VStyleClassHashSet* style_classes = &v_ctx()->style_classes_by_id;
  VStyleClass* existing_class =
      v_style_class_hset_get(style_classes, &(VStyleClassHashSetKey){name});
  VStyle* style;

  if (existing_class) {
    existing_class->is_ready = false;
    v_style_reset(existing_class->style);
    style = existing_class->style;
  } else {
    VStyleClass new_style_class;

    if (!v_style_class_init(name, &new_style_class)) {
      return NULL;
    }

    if (!v_style_class_hset_put(style_classes, new_style_class)) {
      return NULL;
    }

    style = new_style_class.style;
  }

  VStyle* base_style = base ? vss_get_class(base) : NULL;

  if (base_style) {
    v_style__flatten(style, base_style);
  }

  return style;
}

void vss__end_class(const char* name) {
  if (!name) {
    return;
  }

  VStyleClass* style_class = v_style_class_hset_get(
      &v_ctx()->style_classes_by_id, &(VStyleClassHashSetKey){name});

  if (style_class) {
    style_class->is_ready = true;
  }
}

VStyle* vss_get_class(const char* name) {
  if (!name) {
    return NULL;
  }

  VStyleClass* style_class = v_style_class_hset_get(
      &v_ctx()->style_classes_by_id, &(VStyleClassHashSetKey){name});

  return (style_class && style_class->is_ready) ? style_class->style : NULL;
}

bool vss_has_class(const char* name) {
  // TODO: check ready?
  return name &&
         (v_style_class_hset_get(&v_ctx()->style_classes_by_id,
                                 &(VStyleClassHashSetKey){name}) != NULL);
}

bool vss_remove_class(const char* name) {
  return name && v_style_class_hset_remove(&v_ctx()->style_classes_by_id,
                                           &(VStyleClassHashSetKey){name});
}

VUID_PRIVATE void v_style_class__drop(VStyleClass* style_class) {
  if (style_class->style) {
    v_style_unref(style_class->style);
  }
  v_stri_drop(&style_class->id);
}

static bool v_is_legal_class_name(const char* name) {
  if (!name || !v_char_is_alpha(*name)) {
    return false;
  }

  name++;

  while (*name) {
    int c = *name;

    if (!(v_char_is_alpha(c) || c == '_' || c == '-' || v_char_is_digit(c))) {
      return false;
    }

    name++;
  }

  return true;
}

static bool v_style_class_init(const char* id, VStyleClass* out) {
  VStyle* style = v_style__new(NULL);

  if (!style) {
    return false;
  }

  *out = (VStyleClass){
      .id_hash = v_fnv1_hash(id),
      .is_ready = false,
      .style = style,
  };

  // TODO: this can fail!
  v_stri_assign(&out->id, id);

  return true;
}
