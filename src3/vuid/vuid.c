#include <vuid.h>

#define VUID_PKG static

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-function"
#elif defined(__GNUC__) || defined(__GNUG__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-function"
#elif defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable : 4505)
#endif
/*
 * The purpose of this file is to keep all of the standard C includes and some
 * compiler specific macros in one place, rather than sprinkling them throughout
 * the project. Additional considerations are organizing the project so
 * amalgamation and architectural ordering of layers is enforced.
 */

#ifndef VUID_STD_C_H
#define VUID_STD_C_H

#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED_8(arg, ...) (void)arg
#define UNUSED_7(arg, ...) (void)arg, UNUSED_8(__VA_ARGS__, 0)
#define UNUSED_6(arg, ...) (void)arg, UNUSED_7(__VA_ARGS__, 0)
#define UNUSED_5(arg, ...) (void)arg, UNUSED_6(__VA_ARGS__, 0)
#define UNUSED_4(arg, ...) (void)arg, UNUSED_5(__VA_ARGS__, 0)
#define UNUSED_3(arg, ...) (void)arg, UNUSED_4(__VA_ARGS__, 0)
#define UNUSED_2(arg, ...) (void)arg, UNUSED_3(__VA_ARGS__, 0)
#define UNUSED_1(arg, ...) (void)arg, UNUSED_2(__VA_ARGS__, 0)
#define UNUSED(...) (UNUSED_1(__VA_ARGS__, 0))

#define VUID_ALIGN_UP(X, A) (((X) + (A) - 1) & ~((A) - 1))
#define VUID_ARRAY_LEN(x) (sizeof(x) / sizeof(x[0]))

/*
 * Marks a function as package private. This is used for functions that are
 * non-public API that should be static in an amalgamated build.
 */
#ifndef VUID_PKG
#define VUID_PKG
#endif  // VUID_PKG

#endif  // VUID_STD_C_H
#ifndef VUID_STD_ALLOCATOR_H
#define VUID_STD_ALLOCATOR_H


typedef void* (*VAllocFn)(void*, size_t, bool);
typedef void* (*VReallocFn)(void*, void*, size_t, size_t);
typedef void (*VFreeFn)(void*, void*, size_t);

/*
 * Memory allocator interface
 *
 * The allocation functions expect the impl pointer, rather than a pointer to
 * the VAllocator itself. To enforce this, wrapper function are provided that
 * call allocator functions in an expected fashion.
 */
typedef struct VAllocator {
  void* impl;
  VAllocFn alloc_fn;
  VReallocFn realloc_fn;
  VFreeFn free_fn;
} VAllocator;

/* Initializes an allocator backed by malloc, realloc and free. */
VAllocator v_allocator_libc(void);

/* Allocates zero-initialized memory. */
static inline void* v_alloc_zero(const VAllocator* allocator, size_t size) {
  return allocator->alloc_fn(allocator->impl, size, true);
}

/* Allocates memory without zero-initializing it. */
static inline void* v_alloc_raw(const VAllocator* allocator, size_t size) {
  return allocator->alloc_fn(allocator->impl, size, false);
}

static inline void* v_realloc(const VAllocator* allocator,
                              void* ptr,
                              size_t old_size,
                              size_t new_size) {
  return allocator->realloc_fn(allocator->impl, ptr, old_size, new_size);
}

static inline void v_free(const VAllocator* allocator, void* ptr, size_t size) {
  allocator->free_fn(allocator->impl, ptr, size);
}

#endif  // VUID_STD_ALLOCATOR_H

#ifndef VUID_STD_MATHS_H
#define VUID_STD_MATHS_H


#define VUID_FLOAT_MAX_INT (1 << 24)
#define VUID_FLOAT_MIN_INT (-(1 << 24))

#ifndef M_PIf
#define M_PIf 3.14159265358979323846f
#endif

#define VUID_MAX(A, B) ((A) > (B) ? (A) : (B))

/*
 * Miscellaneous math utilities.
 */

static inline int v_clamp_int(int value, int min, int max) {
  return value < min ? min : (value > max ? max : value);
}

static inline float v_clamp_float(float value, float min, float max) {
  return value < min ? min : (value > max ? max : value);
}

static inline bool v_float_eq(float a, float b) {
  return fabs(a - b) < FLT_EPSILON;
}

static inline size_t v_ensure_pow_2(size_t x,
                                    size_t min_pow_2,
                                    size_t max_pow_2) {
  size_t v = min_pow_2;

  while (v < x) {
    if (v >= max_pow_2) {
      return max_pow_2;
    }
    v <<= 1;
  }

  return v;
}

#endif  // VUID_STD_MATHS_H

/*
 * Utility functions for FNV-1a hashing hash map keys.
 *
 * https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
 */

#ifndef VUID_STD_FNV1_H
#define VUID_STD_FNV1_H


#define VUID_FNV_PRIME (16777619)
#define VUID_FNV_OFFSET_BASIS (2166136261)

static inline uint32_t v_fnv1_hash(const char* str) {
  uint32_t hash = VUID_FNV_OFFSET_BASIS;

  while (*str) {
    hash ^= (uint32_t)(unsigned char)(*str++);
    hash *= VUID_FNV_PRIME;
  }

  return hash;
}

static inline uint32_t v_fnv1_hash_n(const uint8_t* data, size_t size) {
  uint32_t hash = VUID_FNV_OFFSET_BASIS;

  for (size_t i = 0; i < size; i++) {
    hash ^= (uint32_t)data[i];
    hash *= VUID_FNV_PRIME;
  }

  return hash;
}

static inline uint32_t v_fnv1_hash_mix(uint32_t hash1, uint32_t hash2) {
  hash1 ^= hash2;
  hash1 *= VUID_FNV_PRIME;
  return hash1;
}

static inline uint32_t v_fnv1_hash_u32(uint32_t value) {
  return v_fnv1_hash_n((const uint8_t*)&value, sizeof(value));
}

#endif  // VUID_STD_FNV1_H

#ifndef VUID_STD_STRING_H
#define VUID_STD_STRING_H


// (1 << 31)
#define VUID_STRING_EXTERNAL_BIT 0x80000000
// UINT32_MAX ^ (1 << 31) - sizeof (VString) - 1
#define VUID_STRING_MAX_SIZE \
  ((uint32_t)(0x7FFFFFFF) - (uint32_t)(sizeof(VString) - 1))

typedef struct VString {
  union {
    VAllocator* allocator;
    const char* external;
  } u;
  uint32_t meta;
  uint32_t refs;
} VString;

// clang-format off
VUID_PKG VString*    v_string_from(VAllocator* allocator, const char* str);
VUID_PKG VString*    v_string_from_vfmt(VAllocator* allocator, const char* fmt, va_list args);
VUID_PKG void        v_string_ref(VString* self);
VUID_PKG void        v_string_unref(VString* self);
VUID_PKG const char* v_string_cstr(const VString* self);
VUID_PKG bool        v_string_eq(const VString* a, const VString* b);
VUID_PKG bool        v_string_eq_cstr(const VString* a, const char* b);
VUID_PKG bool        v_string_is_empty(const VString* self);
VUID_PKG uint32_t    v_string_size(const VString* self);
// clang-format on

static inline bool v_string_is_external(const VString* self) {
  return (self->meta & VUID_STRING_EXTERNAL_BIT) != 0;
}

/*
 * Miscellaneous char and string functions.
 */

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

static inline bool v_cstr_eq(const char* a, const char* b) {
  return strcmp(a ? a : "", b ? b : "") == 0;
}

/*
 * UTF-8 support.
 */

/* Get a codepoint and the number of bytes it occupies from a UTF-8 string. */
VUID_PKG bool v_utf8_get_codepoint(const uint8_t* utf8,
                                   uint32_t* codepoint,
                                   uint32_t* size);

#endif  // VUID_STD_STRING_H

#ifndef VUID_STD_HMAP_H
#define VUID_STD_HMAP_H


/*
 * HashMap template implementation for internal vuid usage.
 *
 * vuid needs hash maps for node id look ups, style class lookups, etc. Most of
 * the use cases require an object or pointer to be stored as a value. The key
 * is embedded in the value: owned and allocated. It would be inefficient to
 * have an additional key field. When looking up, the caller often has a
 * const char* key that is not in the same format as the value's key. We don't
 * want to alloc something just for a hash map query. Also, the internal hash
 * maps do a lot of adds and deletes.
 *
 * This implementation is optimized for these use cases. The user provides a
 * Value type which is the type stored in the map. The user provides a Key type,
 * but it is primarily used for lookups and is not explicitly stored in the map.
 * The user provides function to extract a hash from a Value and a Key, a
 * function to convert a Value to a Key and a function to compare a Key with a
 * Value. And, a drop function is required for the value type.
 *
 * Note: size must be a power of 2 for the hash map to work correctly.
 */

/* % of hash map capacity at which to grow */
#define VUID_HMAP_GROW_THRESHOLD (90)
/* minimum capacity of the hash map */
#define VUID_HMAP_MIN_CAPACITY (8)
/* maximum capacity of the hash map. arbitrary limit based vuid usages */
#define VUID_HMAP_MAX_CAPACITY (1 << 16)
/* internal not found value */
#define VUID_HMAP_NPOS ((size_t) - 1)
/* smallest size of a hash map */
#define VUID_HMAP_BASE_CAPACITY (8)

/* Declaration of hash map type and functions. */
#define VUID_HMAP_DECLARE(HMAP_TYPE, KEY_TYPE, VALUE_TYPE, PREFIX)           \
  typedef struct HMAP_TYPE {                                                 \
    VAllocator* allocator;                                                   \
    VALUE_TYPE* items;                                                       \
    uint8_t* metadata;                                                       \
    size_t capacity;                                                         \
    size_t size;                                                             \
  } HMAP_TYPE;                                                               \
                                                                             \
  typedef struct HMAP_TYPE##Result {                                         \
    VALUE_TYPE* ref;                                                         \
    bool inserted;                                                           \
  } HMAP_TYPE##Result;                                                       \
                                                                             \
  VUID_PKG HMAP_TYPE PREFIX##_init(VAllocator* allocator);                   \
  VUID_PKG HMAP_TYPE PREFIX##_init_with_capacity(VAllocator* allocator,      \
                                                 size_t capacity);           \
  VUID_PKG void PREFIX##_drop(HMAP_TYPE* self);                              \
  VUID_PKG HMAP_TYPE##Result PREFIX##_put(HMAP_TYPE* self, VALUE_TYPE item); \
  VUID_PKG bool PREFIX##_remove_by_value(HMAP_TYPE* self,                    \
                                         const VALUE_TYPE* item);            \
  VUID_PKG bool PREFIX##_remove(HMAP_TYPE* self, KEY_TYPE key);              \
  VUID_PKG VALUE_TYPE* PREFIX##_get(const HMAP_TYPE* self, KEY_TYPE key)

/* Implementation of hash map functions. */
#define VUID_HMAP_IMPL(HMAP_TYPE, KEY_TYPE, VALUE_TYPE, PREFIX)                \
  static HMAP_TYPE##Result PREFIX##_put_no_check(HMAP_TYPE* self,              \
                                                 VALUE_TYPE item);             \
                                                                               \
  static size_t PREFIX##_find_index(const HMAP_TYPE* self,                     \
                                    const KEY_TYPE* key, uint32_t key_hash) {  \
    if (self->capacity == 0) {                                                 \
      return VUID_HMAP_NPOS;                                                   \
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
        return VUID_HMAP_NPOS;                                                 \
      }                                                                        \
                                                                               \
      const uint8_t slot_psl = slot_psl_plus_one - 1;                          \
                                                                               \
      /* Early exit: if current_psl is already greater than the stored */      \
      /* PSL, the node definitely doesn't exist because Robin Hood would */    \
      /* have swapped it here. */                                              \
      if (current_psl > slot_psl) {                                            \
        return VUID_HMAP_NPOS;                                                 \
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
        return VUID_HMAP_NPOS;                                                 \
      }                                                                        \
    }                                                                          \
  }                                                                            \
                                                                               \
  static void PREFIX##_backshift_delete(HMAP_TYPE* self, size_t i) {           \
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
  static bool PREFIX##_grow(HMAP_TYPE* self) {                                 \
    if (self->capacity == VUID_HMAP_MAX_CAPACITY) {                            \
      return false;                                                            \
    }                                                                          \
                                                                               \
    const size_t new_capacity =                                                \
        self->capacity == 0 ? VUID_HMAP_BASE_CAPACITY : self->capacity << 1;   \
    HMAP_TYPE new_map =                                                        \
        PREFIX##_init_with_capacity(self->allocator, new_capacity);            \
                                                                               \
    if (!new_map.items) {                                                      \
      return false;                                                            \
    }                                                                          \
                                                                               \
    VUID_ASSERT(self->items || self->capacity == 0); /* clang-tidy :) */       \
    for (size_t i = 0; i < self->capacity; i++) {                              \
      if (self->metadata[i] != 0) {                                            \
        self->metadata[i] = 0;                                                 \
        if (!PREFIX##_put_no_check(&new_map, self->items[i]).inserted) {       \
          PREFIX##_drop(&new_map);                                             \
          return false;                                                        \
        }                                                                      \
      }                                                                        \
    }                                                                          \
                                                                               \
    self->size = 0;                                                            \
    PREFIX##_drop(self);                                                       \
    *self = new_map;                                                           \
                                                                               \
    return true;                                                               \
  }                                                                            \
                                                                               \
  static HMAP_TYPE##Result PREFIX##_put_no_check(HMAP_TYPE* self,              \
                                                 VALUE_TYPE item) {            \
    /* note: capacity limit is 2^24, mults will not overflow */                \
    if ((self->size + 1) * 100 >= self->capacity * VUID_HMAP_GROW_THRESHOLD) { \
      if (!PREFIX##_grow(self)) {                                              \
        VALUE_TYPE##_drop(&item);                                              \
        return (HMAP_TYPE##Result){.inserted = false};                         \
      }                                                                        \
    }                                                                          \
                                                                               \
    VUID_ASSERT(self->items); /* clang-tidy :) */                              \
    const uint32_t item_hash = VALUE_TYPE##_get_hash(&item);                   \
    const size_t mask = self->capacity - 1;                                    \
    size_t i = item_hash & mask;                                               \
    uint8_t current_psl = 0;                                                   \
    VALUE_TYPE current_item = item;                                            \
    size_t result_index = VUID_HMAP_NPOS;                                      \
                                                                               \
    for (;;) {                                                                 \
      if (self->metadata[i] == 0) {                                            \
        self->items[i] = current_item;                                         \
        self->metadata[i] = current_psl + 1;                                   \
        self->size++;                                                          \
        if (result_index == VUID_HMAP_NPOS)                                    \
          result_index = i;                                                    \
        return (HMAP_TYPE##Result){.ref = &self->items[result_index],          \
                                   .inserted = true};                          \
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
        if (result_index == VUID_HMAP_NPOS)                                    \
          result_index = i;                                                    \
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
        return (HMAP_TYPE##Result){.inserted = false};                         \
      }                                                                        \
    }                                                                          \
  }                                                                            \
                                                                               \
  VUID_PKG HMAP_TYPE PREFIX##_init(VAllocator* allocator) {                    \
    return (HMAP_TYPE){.allocator = allocator};                                \
  }                                                                            \
                                                                               \
  VUID_PKG HMAP_TYPE PREFIX##_init_with_capacity(VAllocator* allocator,        \
                                                 size_t capacity) {            \
    const size_t real_capacity = v_ensure_pow_2(                               \
        capacity, VUID_HMAP_MIN_CAPACITY, VUID_HMAP_MAX_CAPACITY);             \
    const size_t items_size =                                                  \
        VUID_ALIGN_UP(real_capacity * sizeof(VALUE_TYPE), sizeof(void*));      \
    const size_t metadata_size = real_capacity * sizeof(uint8_t);              \
    uint8_t* chunk = v_alloc_zero(allocator, items_size + metadata_size);      \
                                                                               \
    if (!chunk) {                                                              \
      return (HMAP_TYPE){.allocator = allocator};                              \
    }                                                                          \
                                                                               \
    return (HMAP_TYPE){                                                        \
        .items = (VALUE_TYPE*)chunk,                                           \
        .metadata = chunk + items_size,                                        \
        .capacity = real_capacity,                                             \
        .allocator = allocator,                                                \
    };                                                                         \
  }                                                                            \
                                                                               \
  VUID_PKG void PREFIX##_drop(HMAP_TYPE* self) {                               \
    if (self->items) {                                                         \
      const size_t capacity = self->capacity;                                  \
      const size_t items_size =                                                \
          VUID_ALIGN_UP(capacity * sizeof(VALUE_TYPE), sizeof(void*));         \
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
      v_free(self->allocator, self->items, items_size + metadata_size);        \
    }                                                                          \
  }                                                                            \
                                                                               \
  VUID_PKG HMAP_TYPE##Result PREFIX##_put(HMAP_TYPE* self, VALUE_TYPE item) {  \
    const KEY_TYPE item_key = KEY_TYPE##_from_value(&item);                    \
    const size_t existing =                                                    \
        PREFIX##_find_index(self, &item_key, VALUE_TYPE##_get_hash(&item));    \
                                                                               \
    if (existing != VUID_HMAP_NPOS) {                                          \
      VALUE_TYPE##_drop(&item);                                                \
      return (HMAP_TYPE##Result){.inserted = false};                           \
    }                                                                          \
                                                                               \
    return PREFIX##_put_no_check(self, item);                                  \
  }                                                                            \
                                                                               \
  VUID_PKG VALUE_TYPE* PREFIX##_get(const HMAP_TYPE* self, KEY_TYPE key) {     \
    const size_t index =                                                       \
        PREFIX##_find_index(self, &key, KEY_TYPE##_get_hash(&key));            \
                                                                               \
    return (index != VUID_HMAP_NPOS) ? &self->items[index] : NULL;             \
  }                                                                            \
                                                                               \
  VUID_PKG bool PREFIX##_remove_by_value(HMAP_TYPE* self,                      \
                                         const VALUE_TYPE* item) {             \
    const KEY_TYPE key = KEY_TYPE##_from_value(item);                          \
    const size_t i =                                                           \
        PREFIX##_find_index(self, &key, VALUE_TYPE##_get_hash(item));          \
                                                                               \
    if (i == VUID_HMAP_NPOS) {                                                 \
      return false;                                                            \
    }                                                                          \
                                                                               \
    VUID_ASSERT(self->items); /* clang-tidy :) */                              \
    VALUE_TYPE##_drop(&self->items[i]);                                        \
    PREFIX##_backshift_delete(self, i);                                        \
                                                                               \
    return true;                                                               \
  }                                                                            \
                                                                               \
  VUID_PKG bool PREFIX##_remove(HMAP_TYPE* self, KEY_TYPE key) {               \
    const size_t i =                                                           \
        PREFIX##_find_index(self, &key, KEY_TYPE##_get_hash(&key));            \
                                                                               \
    if (i == VUID_HMAP_NPOS) {                                                 \
      return false;                                                            \
    }                                                                          \
                                                                               \
    VUID_ASSERT(self->items); /* clang-tidy :) */                              \
    VALUE_TYPE##_drop(&self->items[i]);                                        \
    PREFIX##_backshift_delete(self, i);                                        \
                                                                               \
    return true;                                                               \
  }

#endif  // VUID_STD_HMAP_H

#ifndef VUID_STD_BUFFER_H
#define VUID_STD_BUFFER_H


typedef struct VBuffer {
  const void* data;
  size_t size;
  VAllocator* allocator;
  void* source;
} VBuffer;

/* initialize a buffer with an unowned data pointer */
static inline VBuffer v_buffer_init_static(const void* data, size_t size) {
  return (VBuffer){
      .data = data,
      .size = size,
  };
}

/* initialize a buffer with a copy of the source data */
static inline bool v_buffer_from(VBuffer* self,
                                 const void* source,
                                 size_t size,
                                 VAllocator* allocator) {
  if (size == 0 || source == NULL) {
    *self = (VBuffer){0};
    return true;
  }

  void* data = v_alloc_raw(allocator, size);

  if (!data) {
    return false;
  }

  memcpy(data, source, size);

  *self = (VBuffer){
      .data = data,
      .size = size,
      .allocator = allocator,
      .source = data,
  };

  return true;
}

/* check if a buffer is empty */
static inline bool v_buffer_is_empty(const VBuffer* self) {
  return !self || !self->data || self->size == 0;
}

/* free memory resources owned by this buffer. */
static inline void v_buffer_drop(VBuffer* self) {
  if (self->allocator && self->source) {
    v_free(self->allocator, self->source, self->size);
  }
  *self = (VBuffer){0};
}

#endif  // VUID_STD_BUFFER_H

#ifndef VUID_STD_ARR_H
#define VUID_STD_ARR_H


#define VUID_ARRAY_BASE_CAPACITY (8)
#define VUID_ARRAY_GROW_FACTOR (2)

/*
 * Dynamic Array
 *
 * The implementation is straightforward. The array itself stores the size of an
 * item, mostly to avoid C templating with a macro. The array also holds a
 * pointer to it's allocator (prefer this to passing to each function call).
 *
 * TODO: use ints instead of size_t?
 */
typedef struct VArray {
  VAllocator* allocator;
  void* items;
  size_t item_size;
  size_t size;
  size_t capacity;
} VArray;

// clang-format off
/* Initialize array with capacity. If initial_capacity is 0, no allocation is performed until the first push. */
VUID_PKG VArray  v_array_init(VAllocator* allocator, size_t item_size, size_t initial_capacity);
/* Cleanup the array's internal resources. Contents of self are undefined after this call. */
VUID_PKG void    v_array_drop(VArray* self);
/* Get an item at the specified index. return NULL: if index is out of bounds. */
VUID_PKG void*   v_array_get(const VArray* self, size_t index);
/* Push an item to the end of the array. return false: allocation failed, true: success */
VUID_PKG bool    v_array_push(VArray* self, void* item);
/* Push but returns pointer to the new item uninitialized. return NULL: allocation failed */
VUID_PKG void*   v_array_push_one(VArray* self);
/* Push multiple unintialized items. return false: allocation failed, true: success */
VUID_PKG bool    v_array_push_n(VArray* self, size_t count);
/* Remove the last item from the array. If array is empty, noop. */
VUID_PKG void    v_array_pop(VArray* self);
/* Remove the last item from a ptr array and return its ptr. If array is empty, noop and return NULL. */
VUID_PKG void*   v_array_pop_ptr(VArray* self);
/* Remove an item at the specified index. If index is out of bounds, noop. */
VUID_PKG void    v_array_remove(VArray* self, size_t index);
/* Insert an item at the specified index, shifting elements right. If index >= size, appends. return false: allocation failed */
VUID_PKG bool    v_array_insert(VArray* self, size_t index, void* item);
/* Apply a function to each item in the array. Assumes fn will not modify the array. */
VUID_PKG void    v_array_apply(VArray* self, void (*fn)(void* item, void* ctx), void* ctx);
// clang-format on

/* Clear the array. No deallocation. */
static inline void v_array_clear(VArray* self) {
  self->size = 0;
}

/* Get an item without bounds checking */
static inline void* v_array_get_unchecked(const VArray* self, size_t index) {
  return (uint8_t*)(self->items) + index * self->item_size;
}

/* Get an item without bounds checking. Special case for pointer arrays. */
static inline void* v_array_get_ptr_unchecked(const VArray* self,
                                              size_t index) {
  return ((void**)self->items)[index];
}

#endif  // VUID_STD_ARR_H

#ifndef VUID_STD_FILE_H
#define VUID_STD_FILE_H


/*
 * Read an entire file into a heap-allocated buffer.
 * On success, writes the file size to *size_out and returns the buffer.
 * The caller must free the buffer with v_free(alloc, ptr, *size_out).
 * Returns NULL on failure.
 */
VUID_PKG uint8_t* v_file_read(VAllocator* alloc,
                              const char* path,
                              size_t* size_out);

#endif  // VUID_STD_FILE_H

#ifndef VUID_CORE_SHARED_H
#define VUID_CORE_SHARED_H



#define VUID_MAX_STYLE_NAME_LENGTH 63
#define VUID_MAX_NODE_ID_LENGTH 63
#define VUID_MAX_FONT_NAME_LENGTH 63

#define V_CHECK_CONTEXT(...)       \
  do {                             \
    if (!v_ctx_is_initialized()) { \
      return __VA_ARGS__;          \
    }                              \
  } while (0)

struct VStaticString;
typedef struct VNodeModule VNodeModule;
typedef struct VTextModule VTextModule;
typedef struct VImageStore VImageStore;

/* reference counted objects in vuid */
typedef enum VObjectType {
  V_OBJECT_TYPE_NODE,
  V_OBJECT_TYPE_STYLE,
  V_OBJECT_TYPE_WEAK_NODE_REF,
  V_OBJECT_TYPE_IMAGE,
  V_OBJECT_TYPE__COUNT,
} VObjectType;

// clang-format off
VUID_PKG VAllocator*  v_ctx_allocator(void);
VUID_PKG bool         v_ctx_is_initialized(void);
VUID_PKG void         v_ctx_object_inc(VObjectType type);
VUID_PKG void         v_ctx_object_dec(VObjectType type);
VUID_PKG VTextModule* v_ctx_text_module(void);
VUID_PKG VNodeModule* v_ctx_node_module(void);
VUID_PKG VImageStore* v_ctx_image_store(void);
VUID_PKG bool         v_ctx_intern_string(const char* str, struct VStaticString* out);
VUID_PKG uint32_t     v_ctx_create_texture(VPixelFormat format, const VImageBuffer* buffer);
VUID_PKG void         v_ctx_destroy_texture(uint32_t texture_id);
VUID_PKG void         v_ctx_request_render(void);
// clang-format on

#define v_ctx_new(TYPE) v_alloc_zero(v_ctx_allocator(), sizeof(TYPE))
#define v_ctx_delete(PTR, TYPE) v_free(v_ctx_allocator(), PTR, sizeof(TYPE))

#endif  // VUID_CORE_SHARED_H

#ifndef VUID_CORE_UTIL_H
#define VUID_CORE_UTIL_H



/*
 * Utility functions that are internal to vuid and operate on or related to vuid
 * types. Code that is vuid agnostic should go into the std layer.
 */

/* Default device pixel ratio. */
#define VUID_DEFAULT_DPR (1.0f)

/* Checks if two rectangles intersect and returns the intersection rectangle. */
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

/* Checks if a point is inside a rectangle. */
static inline bool v_point_in_rect(float px,
                                   float py,
                                   float rx,
                                   float ry,
                                   float rw,
                                   float rh) {
  return px >= rx && px < rx + rw && py >= ry && py < ry + rh;
}

/* Snaps a value to the nearest device pixel grid. */
static inline float v_snap_to_grid_dpr(float value) {
  const float dpr = VUID_DEFAULT_DPR;  // TODO: get this from context
  return roundf(value * dpr) / dpr;
}

/* Checks if two VColor style objects are equal */
static inline bool v_color_eq(VColor a, VColor b) {
  return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

static inline bool v_style_value_eq(VStyleValue a, VStyleValue b) {
  if (a.unit == b.unit) {
    switch (a.unit) {
      case V_STYLE_VALUE_UNIT_PX:
        return v_float_eq(a.value.px, b.value.px);
      case V_STYLE_VALUE_UNIT_FIT:
      case V_STYLE_VALUE_UNIT_GROW:
      case V_STYLE_VALUE_UNIT_AUTO:
        return true;
      default:
        break;
    }
  }

  return false;
}

#endif  // VUID_CORE_UTIL_H

#ifndef VUID_CORE_WEAK_REF_H
#define VUID_CORE_WEAK_REF_H


// clang-format off

/*
 * VWeakRef
 *
 * Weak references are reference-counted objects that hold a pointer to a target object. The
 * target object hold a reference to the weak ref. When the target object is destroyed, it
 * clears the weak ref's target. If the weak ref has any holders, then the target object
 * is no longer available.
 *
 * vuid's uses of weak refs are limited to node references in the scene graph. This solution
 * works for this use case, but is not a general-purpose weak reference implementation.
 */
typedef struct VWeakRef {
  void*    ref;       /* target object */
  uint32_t ref_count; /* reference count of this weak ref (not the target object) */
} VWeakRef;

/* create a new weak ref with a target object */
VUID_PKG VWeakRef* v_weak_ref_new(void* ref);
/* get the target object or null if not available */
VUID_PKG void*     v_weak_ref_get(const VWeakRef* weak_ref);
/* acquire a reference to the weak ref, increasing ref count */
VUID_PKG VWeakRef* v_weak_ref_acquire(VWeakRef* weak_ref);
/* release a reference to the weak ref, decreasing ref count and freeing if zero */
VUID_PKG void      v_weak_ref_release(VWeakRef* weak_ref);
// clang-format on

#endif  // VUID_CORE_WEAK_REF_H

#ifndef VUID_CORE_COMMAND_QUEUE_H
#define VUID_CORE_COMMAND_QUEUE_H



struct VInsets;

// clang-format off

/* Command queue for draw calls. */
typedef struct VCommandQueue {
  VArray /*<VCommand[]>*/ commands;
  VArray /*<VVertex[]>*/  vertices;
  VArray /*<uint32_t[]>*/ indices;

  uint32_t cmd_vertex_offset;     /* next available vertex offset */
  uint32_t cmd_index_offset;      /* next available index offset */
  uint32_t cmd_vertex_index_base; /* base vertex index for the current command */
  uint32_t batch_quad_count;      /* number of quads in the current batch */

  VVertexStruct vertex_format;
  VDataFormat index_format;
} VCommandQueue;

/* initialize command queue */
VUID_PKG bool v_command_queue_init(VCommandQueue* self, VAllocator* allocator);
/* release command queue resources. self pointer is not free'd */
VUID_PKG void v_command_queue_drop(VCommandQueue* self);
/* clear all commands */
VUID_PKG void v_command_queue_clear(VCommandQueue* self);
/* open a new batch of textured quads */
VUID_PKG void v_command_queue_open_batch(VCommandQueue* self);
/* queue the current batch of textured quads as a command */
VUID_PKG void v_command_queue_close_batch(VCommandQueue* self, uint32_t texture_id);
/* add a quad to the current batch */
VUID_PKG bool v_command_queue_add_quad(VCommandQueue* self, const VRect* quad, float u0, float v0, float u1, float v1, VColor color);
/* queue a textured quad command */
VUID_PKG void v_command_queue_cmd_textured_quad(VCommandQueue* self, const VRect* quad, uint32_t texture_id, float u0, float v0, float u1, float v1, VColor color);
/* queue a filled rectangle command */
VUID_PKG void v_command_queue_cmd_fill_rect(VCommandQueue* self, const VRect* quad, VColor color);
/*
 * queue a filled rounded rectangle command.
 *
 * skip_outer_fringe enables a halo or skirt around the shape to smooth edges. if a stroke
 * is to draw above this shape, outer fringe should be skipped.
 */
VUID_PKG void v_command_queue_cmd_fill_rounded_rect(VCommandQueue* self, const VRect* quad, float border_radius, VColor color, bool skip_outer_fringe);
/* queue a stroked rectangle command */
VUID_PKG void v_command_queue_cmd_stroke_rect(VCommandQueue* self, const VRect* quad, const struct VInsets* border, VColor color);
/* queue a stroked rounded rectangle command */
VUID_PKG void v_command_queue_cmd_stroke_rounded_rect(VCommandQueue* self, const VRect* quad, float border_radius, float thickness, VColor color);
/* queue a set clip command */
VUID_PKG void v_command_queue_cmd_set_clip(VCommandQueue* self, int x, int y, int width, int height);
/* queue a reset clip command */
VUID_PKG void v_command_queue_cmd_reset_clip(VCommandQueue* self);

// clang-format on

#endif  // VUID_CORE_COMMAND_QUEUE_H

#ifndef VUID_CORE_STRING_POOL_H
#define VUID_CORE_STRING_POOL_H


typedef struct VStaticStringSetKey {
  const char* str;
  uint32_t str_hash;
} VStaticStringSetKey;

typedef struct VStaticString {
  char* str;
  uint32_t hash;
  uint32_t size;
} VStaticString;

VUID_HMAP_DECLARE(VStaticStringSet,
                  VStaticStringSetKey,
                  VStaticString,
                  v_static_string_set);

/*
 * Pool of interned strings.
 * - Strings are case sensitive.
 * - VStaticString objects can be held until string pool is dropped.
 */
typedef struct VStringPool {
  VStaticStringSet strings;
} VStringPool;

/*
 * Initialize the string pool with the given allocator.
 * Returns true if initialization is successful, false otherwise.
 */
VUID_PKG bool v_string_pool_init(VStringPool* self,
                                 VAllocator* allocator,
                                 uint32_t initial_capacity);
/*
 * Release the resources associated with the string pool. All VStaticString
 * objects obtained from this pool become invalid.
 */
VUID_PKG void v_string_pool_drop(VStringPool* self);
/*
 * Interns the given string in the string pool. If the string is already
 * interned, returns the existing VStaticString. If allocation fails, returns
 * false.
 */
VUID_PKG bool v_string_pool_intern(VStringPool* self,
                                   const char* str,
                                   VStaticString* out);

static inline bool v_static_string_is_empty(const VStaticString* str) {
  return str->size == 0;
}

static inline const char* v_static_string_to_str(const VStaticString* str) {
  return str ? str->str : "";
}

#endif  // VUID_CORE_STRING_POOL_H

#ifndef VUID_CORE_INSETS_H
#define VUID_CORE_INSETS_H


/* Insets structure representing padding, margins or other trbl things. */
typedef struct VInsets {
  float top;
  float right;
  float bottom;
  float left;
} VInsets;

/* Creates a VInsets with trbl values. */
static inline VInsets v_insets(float top,
                               float right,
                               float bottom,
                               float left) {
  return (VInsets){top, right, bottom, left};
}

/* Gets the total horizontal insets (left + right). */
static inline float v_insets_width(const VInsets* insets) {
  return insets->right + insets->left;
}

/* Gets the total vertical insets (top + bottom). */
static inline float v_insets_height(const VInsets* insets) {
  return insets->top + insets->bottom;
}

#endif  // VUID_CORE_INSETS_H

#ifndef VUID_IMAGE_IMAGE_STORE_H
#define VUID_IMAGE_IMAGE_STORE_H



typedef struct VImage VImage;

struct VImage {
  VString* src;
  VImageBuffer buffer;
  VBuffer file_buffer;
  uint32_t texture_id;
  uint32_t src_hash;
  uint32_t width;
  uint32_t height;
  uint32_t ref_count;
};

typedef struct VImageHashMapValue {
  VImage* image;
  bool persist;
} VImageHashMapValue;

typedef struct VImageHashMapKey {
  const char* src;
} VImageHashMapKey;

VUID_HMAP_DECLARE(VImageHashMap,
                  VImageHashMapKey,
                  VImageHashMapValue,
                  v_image_hmap);

struct VImageStore {
  VAllocator* allocator;
  VImageHashMap images;
  VImageLoaderFn loader;
};

VUID_PKG bool v_image_store_init(VImageStore* self,
                                 VAllocator* allocator,
                                 VImageLoaderFn loader);
VUID_PKG void v_image_store_drop(VImageStore* self);

VUID_PKG VImage* v_image_store_acquire(VImageStore* self, VString* src);
VUID_PKG void v_image_store_release(VImageStore* self, VImage* image);

VUID_PKG bool v_image_store_persist(VImageStore* self, const char* src);
VUID_PKG bool v_image_store_persist_mem(VImageStore* self,
                                        const char* src,
                                        VBuffer* buffer);
VUID_PKG void v_image_store_remove(VImageStore* self, const char* src);

/* exposed for testing */
VUID_PKG VImageHashMapValue* v_image_store_get(VImageStore* self,
                                               const char* src);

VUID_PKG VSize v_image_get_size(const VImage* image);
VUID_PKG uint32_t v_image_get_texture_id(const VImage* image);

#endif  // VUID_IMAGE_IMAGE_STORE_H

#ifndef VUID_TEXT_DEPS_H
#define VUID_TEXT_DEPS_H

/* Text Engine external dependencies. */

#if defined(VUID_TEXT_ENGINE_FT)

#include <ft2build.h>
#include FT_FREETYPE_H
#if defined(VUID_TEXT_ENGINE_HB_SHAPER)
#include <hb-ft.h>
#endif

/* FT load flags shared by the shaper and rasterizer. */
#define VUID_FT_LOAD_FLAGS (FT_LOAD_DEFAULT | FT_LOAD_TARGET_LIGHT)

#elif defined(VUID_TEXT_ENGINE_STB)

#include <stb_truetype.h>

#elif defined(VUID_TEXT_ENGINE_HB)

#include <hb-ot.h>
#include <hb-raster.h>
// fallthrough

#else

// At compile time, define one of the following macros to select the text
// engine for font file loading and font rasterization:
//
// VUID_TEXT_ENGINE_FT
// VUID_TEXT_ENGINE_STB
// VUID_TEXT_ENGINE_HB
//
#error "No text engine defined."

#endif

#if defined(VUID_TEXT_ENGINE_HB) || defined(VUID_TEXT_ENGINE_HB_SHAPER)

#include <hb.h>

#define VUID_HB_SUBPIXEL_BITS 6 /* 26.6 fixed-point, like FreeType */
#define VUID_HB_FONT_SCALE (1 << VUID_HB_SUBPIXEL_BITS)

#endif

#endif  // VUID_TEXT_DEPS_H
#ifndef VUID_TEXT_MOD_FWD_H
#define VUID_TEXT_MOD_FWD_H




typedef struct VTextLayout VTextLayout;
typedef struct VFontData VFontData;
typedef struct VGlyphAtlas VGlyphAtlas;

typedef struct VFontMetrics {
  float ascent;      /* positive, pixels above baseline */
  float descent;     /* negative, pixels below baseline */
  float line_height; /* line advance in pixels */
} VFontMetrics;

typedef struct VAtlasCacheEntry VAtlasCacheEntry;

/* Text engine global state. */
typedef struct VTextEngine {
#if defined(VUID_TEXT_ENGINE_FT)
  FT_Library ft_library;
#endif
#if defined(VUID_TEXT_ENGINE_HB)
  hb_raster_draw_t* hb_draw;
#endif
#if defined(VUID_TEXT_ENGINE_HB_SHAPER)
  hb_buffer_t* hb_buffer;
#endif
  int padding;
} VTextEngine;

/* Text engine font state. */
typedef struct VTextEngineFont {
#if defined(VUID_TEXT_ENGINE_FT)
  FT_Face ft_face;
#endif
#if defined(VUID_TEXT_ENGINE_STB)
  stbtt_fontinfo stb_fontinfo;
  float scale; /* font size scale factor */
#endif
#if defined(VUID_TEXT_ENGINE_HB) || defined(VUID_TEXT_ENGINE_HB_SHAPER)
  hb_face_t* hb_face;
  hb_font_t* hb_font;
#endif
} VTextEngineFont;

/* Shaped glyph info. */
typedef struct VShapedGlyph {
  uint32_t glyph_id;
  float x_advance;
  float y_advance;
  float x_offset;
  float y_offset;
  /* byte offset of the source codepoint in the UTF-8 string */
  uint32_t cluster;
} VShapedGlyph;

/*
 * Holds glyph bitmap information needed for atlas placement and rendering. This
 * has been abstracted so each font engine can implement their custom glyph
 * bitmap sizing, bearing and rasterization logic, while the atlas packing and
 * cache management can remain common code.
 */
typedef struct VGlyphBitmapDesc {
  uint32_t glyph_id;
  float bearing_x;
  float bearing_y;
  uint16_t width;
  uint16_t height;
} VGlyphBitmapDesc;

#endif  // VUID_TEXT_MOD_FWD_H

#ifndef VUID_TEXT_FONT_DATA_H
#define VUID_TEXT_FONT_DATA_H




/*
 * Raw font data + backend-specific handle for a single loaded font.
 * Internal to the text module — not included by other modules.
 */
struct VFontData {
  VAllocator* allocator;
  VStaticString name;
  VBuffer file_buffer;
  VTextEngine* text_engine;
  VTextEngineFont font;
  uint16_t font_id;
  /* last size set via v_font_data_set_pixel_size */
  uint16_t current_pixel_size;
};

/*
 * Allocate and load a font from raw bytes.
 * bytes are copied internally; the caller's buffer can be freed after this
 * call. ft_library is the FT_Library cast to void*; pass NULL for STB path.
 * Returns NULL on failure.
 */
VUID_PKG VFontData* v_font_data_new(VAllocator* allocator,
                                    VTextEngine* text_engine,
                                    VBuffer* file_buffer,
                                    uint16_t font_id,
                                    const char* name);

/* Free all backend resources and the struct itself. */
VUID_PKG void v_font_data_drop(VFontData* self);

/*
 * Set the active pixel size on the backend face.
 * Must be called before any shaping or metric queries at a new size.
 * Returns false on failure.
 */
VUID_PKG bool v_font_data_set_pixel_size(VFontData* self, uint16_t pixel_size);

VUID_PKG bool v_font_data_is_legal_name(const char* name);

VUID_PKG bool v_font_data_get_metrics(VFontData* self,
                                      VFontMetrics* font_metrics);

#endif  // VUID_TEXT_FONT_DATA_H

#ifndef VUID_TEXT_TEXT_MODULE_H
#define VUID_TEXT_TEXT_MODULE_H




#define VUID_ATLAS_TEXTURE_ID 1

/* ---- Glyph atlas ---- */

/* Lookup key for the atlas cache: (glyph_id, font_id, pixel_size). */
typedef struct VAtlasCacheKey {
  uint32_t glyph_id;
  uint32_t packed_fp; /* (font_id << 16) | pixel_size */
} VAtlasCacheKey;

/* One cached glyph in the atlas texture. */
struct VAtlasCacheEntry {
  uint32_t glyph_id;
  uint16_t font_id;
  uint16_t pixel_size;
  uint16_t atlas_x, atlas_y; /* top-left pixel in the atlas */
  uint16_t atlas_w, atlas_h; /* bitmap dimensions in pixels */
  float bearing_x;           /* FreeType bitmap_left */
  float bearing_y; /* FreeType bitmap_top (positive = above baseline) */
  uint32_t last_used_frame;
};

VUID_HMAP_DECLARE(VAtlasCacheMap,
                  VAtlasCacheKey,
                  VAtlasCacheEntry,
                  v_atlas_cache_map);

/*
 * CPU-side glyph atlas. Owns an A8 (1 byte per pixel) pixel buffer and a
 * skyline bin-packer state. Glyph bitmaps are rasterized into this buffer on
 * demand; modified_count is bumped after every pixel write so the draw pass
 * knows to re-upload the GPU texture. The atlas is always square; when full
 * it doubles in size (up to max_size) before falling back to a full clear.
 * size_version is bumped on every resize so the renderer can recreate the
 * GPU texture.
 */
struct VGlyphAtlas {
  VAllocator* allocator;
  uint8_t* pixels; /* row-major, one byte per pixel (A8) */
  uint16_t width, height;
  uint16_t max_size; /* maximum square dimension; growth stops here */
  VAtlasCacheMap cache;
  VArray skyline; /* VAtlasSkylineNode[] — private element type */
  uint32_t modified_count;
  uint32_t size_version; /* bumped each time the atlas is resized */
};

/* ---- Text glyph structs ---- */

/* A single positioned glyph in a laid-out text run. */
typedef struct VTextGlyph {
  uint32_t glyph_id;
  float x; /* pen x + x_offset, from left edge of layout box */
  float y; /* baseline_y + y_offset, from top edge of layout box */
} VTextGlyph;

/* A single line in a text layout. */
typedef struct VTextLine {
  uint32_t glyph_start; /* index into VTextLayout.glyphs */
  uint32_t glyph_count;
  float width;
  float baseline_y; /* from top of layout bounding box */
} VTextLine;

/* Immutable result of laying out a text string. Heap-allocated as one block. */
struct VTextLayout {
  VAllocator* allocator;
  VTextGlyph* glyphs;
  uint32_t glyph_count;
  VTextLine* lines;
  uint32_t line_count;
  float width;
  float height;
  float ascent; /* of the first line, positive */
  uint16_t font_id;
  uint16_t pixel_size;
  uint32_t rasterized_at; /* the glyph atlas generation this layout was
                             rasterized at  */
};

struct VTextModule {
  VAllocator* allocator;
  VTextEngine text_engine;
  // TODO: why is this a pointer to VFontData*?
  VArray /* VFontData*[] */ font_data;
  // TODO: this could be addressed with an arena
  /* scratch buffers reused across layout calls */
  VArray /* VShapedGlyph[] */ scratch_shaped;
  VArray /* VTextGlyph[] */ scratch_glyphs;
  VArray /* VTextLine[] */ scratch_lines;
  VGlyphAtlas atlas;
  uint32_t current_frame;
  uint16_t next_font_id; /* starts at 1; 0 is the invalid sentinel */
};

VUID_PKG bool v_text_module_init(VTextModule* self,
                                 VAllocator* allocator,
                                 uint32_t atlas_size,
                                 uint32_t atlas_max_size);

VUID_PKG void v_text_module_drop(VTextModule* self);

VUID_PKG void* v_text_module_get_hb_buffer(void);

VUID_PKG bool v_text_module_add_font(VTextModule* self,
                                     const char* name,
                                     VBuffer* file_buffer);

VUID_PKG void v_text_module_remove_font(VTextModule* self, const char* name);

/*
 * Create a laid-out VTextLayout for the given UTF-8 text and style.
 * wrap_width is used for line breaking (WRAP mode) and text alignment.
 * Returns a heap-allocated layout on success, NULL on failure.
 * The caller must free it with v_text_module_destroy_layout.
 */
VUID_PKG VTextLayout* v_text_module_create_layout(VTextModule* self,
                                                  uint16_t font_id,
                                                  uint16_t pixel_size,
                                                  const VString* utf8,
                                                  VTextWrap wrap,
                                                  VAlignX talign,
                                                  float wrap_width);

/* v_text_module_create_layout() that accepts a UTF-8 c string and length.*/
VUID_PKG VTextLayout* v_text_module_create_layout_z(VTextModule* self,
                                                    uint16_t font_id,
                                                    uint16_t pixel_size,
                                                    const char* utf8,
                                                    size_t utf8_len,
                                                    VTextWrap wrap,
                                                    VAlignX talign,
                                                    float wrap_width);

VUID_PKG void v_text_module_destroy_layout(VTextModule* self,
                                           VTextLayout* layout);

/* Look up font data by font_id. Returns NULL if not found. */
VUID_PKG VFontData* v_text_module_get_font_data(VTextModule* self,
                                                uint16_t font_id);
/* Look font by name. Returns NULL if not found.*/
VUID_PKG VFontData* v_text_module_find_font_by_name(VTextModule* self,
                                                    const char* font_name);
/* Look font ID by name. Returns 0 if not found. */
VUID_PKG uint16_t v_text_module_find_font_id_by_name(VTextModule* self,
                                                     const char* font_name);

VUID_PKG void v_text_module_maybe_rasterize_glyphs(VTextModule* self,
                                                   VTextLayout* layout);

#endif  // VUID_TEXT_TEXT_MODULE_H

#ifndef VUID_TEXT_ATLAS_H
#define VUID_TEXT_ATLAS_H



VUID_PKG bool v_glyph_atlas_init(VGlyphAtlas* self,
                                 VAllocator* alloc,
                                 uint32_t width,
                                 uint32_t max_size);
VUID_PKG void v_glyph_atlas_drop(VGlyphAtlas* self);
/* Clear all cached entries and reset the skyline packer. */
VUID_PKG void v_glyph_atlas_clear(VGlyphAtlas* self);

/*
 * Look up a glyph in the atlas, rasterizing it if not already present. The
 * rasterization is at the font_data's current pixel_size, which must be set
 * before calling this. On success returns a pointer into the atlas cache (valid
 * until the atlas is cleared or dropped).  On atlas-full, the atlas is
 * auto-cleared and the rasterization is retried once. Returns NULL on failure
 * (glyph too large, FT error, OOM).
 */
VUID_PKG VAtlasCacheEntry* v_glyph_atlas_get_or_rasterize(
    VGlyphAtlas* self,
    VTextEngine* text_engine,
    VFontData* font_data,
    uint32_t glyph_id,
    uint32_t frame);

VUID_PKG uint32_t v_glyph_atlas_get_modified_count(VGlyphAtlas* atlas);
VUID_PKG uint16_t v_glyph_atlas_get_size(const VGlyphAtlas* atlas);
VUID_PKG VImageBuffer v_glyph_atlas_get_pixel_buffer(const VGlyphAtlas* atlas);

#endif  // VUID_TEXT_ATLAS_H

#ifndef VUID_TEXT_TEXT_ENGINE_H
#define VUID_TEXT_TEXT_ENGINE_H

/*
 * Text engine is the interface to a backend that handles font loading, font
 * shaping and font rasterization. vuid supports freetype, harfbuzz and stb
 * truetype. harfbuzz can run standalone with its internal rasterizer. If
 * available, freetype and stb will use harfbuzz for shaping. Otherwise, those
 * engines will handle shaping in a very basic manner.
 */



// clang-format off
/* initialize the text engine */
VUID_PKG bool v_te_init(VTextEngine* self);
/* cleanup the text engine resources */
VUID_PKG void v_te_drop(VTextEngine* self);
/* init a font. bytes is the ttf. caller guarantees its validity over the font's lifetime. */
VUID_PKG bool v_te_init_font(VTextEngine* self, VTextEngineFont* font, const uint8_t* bytes, size_t size);
/* cleanup a font's resources */
VUID_PKG void v_te_drop_font(VTextEngine* self, VTextEngineFont* font);
/* set the font pixel size. */
VUID_PKG bool v_te_set_font_pixel_size(VTextEngine* self, VTextEngineFont* font, uint16_t pixel_size);
/* get font metrics in pixels. */
VUID_PKG bool v_te_get_font_metrics(VTextEngine* self, VTextEngineFont* font, float* ascent, float* descent, float* line_height);
/* shape a UTF-8 string into an array of glyphs. */
VUID_PKG bool v_te_shape(VTextEngine* self, VTextEngineFont* font, const char* utf8, uint32_t utf8_len, VArray* out);
/* get bitmap size. used by glyph atlas for rect packing. */
VUID_PKG bool v_te_get_glyph_bitmap_desc(VTextEngine* self, VTextEngineFont* font, uint32_t glyph_id, VGlyphBitmapDesc* bitmap_desc);
/* rasterize a glyph into a target bitmap. used by glyph atlas. */
VUID_PKG bool v_te_rasterize_glyph(VTextEngine* self, VTextEngineFont* font, VGlyphBitmapDesc* bitmap_desc, uint8_t* target, uint16_t target_width, uint16_t target_padding, uint16_t tx, uint16_t ty);
// clang-format on

#endif  // VUID_TEXT_TEXT_ENGINE_H

#ifndef VUID_TEXT_TEXT_LAYOUT_H
#define VUID_TEXT_TEXT_LAYOUT_H




/*
 * Build a VTextLayout from shaped glyphs and font metrics.
 * scratch_glyphs and scratch_lines are temporary VArrays reused by the caller;
 * their contents are overwritten but the arrays are not dropped.
 * Returns a heap-allocated VTextLayout on success, NULL on OOM.
 */
VUID_PKG VTextLayout* v_text_layout_build(VAllocator* allocator,
                                          const VShapedGlyph* shaped,
                                          uint32_t shaped_count,
                                          const char* utf8,
                                          const VFontMetrics* font_metrics,
                                          VTextWrap wrap,
                                          VAlignX talign,
                                          float wrap_width,
                                          VArray* scratch_glyphs,
                                          VArray* scratch_lines);

VUID_PKG void v_text_layout_render(const VTextLayout* layout,
                                   VCommandQueue* cmdq,
                                   float x,
                                   float y,
                                   VColor color,
                                   VGlyphAtlas* atlas);

VUID_PKG VSize v_text_layout_get_size(const VTextLayout* text_layout);

#endif  // VUID_TEXT_TEXT_LAYOUT_H

#ifndef VUID_NODE_MOD_FWD_H
#define VUID_NODE_MOD_FWD_H




typedef enum VStyleProperty {
  VS_WIDTH = 0,
  VS_MIN_WIDTH,
  VS_MAX_WIDTH,
  VS_HEIGHT,
  VS_MIN_HEIGHT,
  VS_MAX_HEIGHT,
  VS_DIRECTION,
  VS_VISIBILITY,
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
  VS_MARGIN_TOP,
  VS_MARGIN_RIGHT,
  VS_MARGIN_BOTTOM,
  VS_MARGIN_LEFT,
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

typedef struct VStyleClass {
  VStaticString id;
  VStyle* style;
  // TODO: move this flag into style
  bool is_ready;
} VStyleClass;

typedef struct VStyleClassHashMapKey {
  const char* id;
  uint32_t hash;
} VStyleClassHashMapKey;

VUID_HMAP_DECLARE(VStyleClassHashMap,
                  VStyleClassHashMapKey,
                  VStyleClass,
                  v_style_class_hmap);

#endif  // VUID_NODE_MOD_FWD_H

#ifndef VUID_NODE_NODE_INPUT_H
#define VUID_NODE_NODE_INPUT_H


VUID_PKG void v_node_on_mouse_button(VNode* self,
                                     VNodeModule* mod,
                                     const VInputEvent* event);

VUID_PKG void v_node_on_mouse_move(VNode* self,
                                   VNodeModule* mod,
                                   const VInputEvent* event);

VUID_PKG void v_node_on_mouse_wheel(VNode* self,
                                    VNodeModule* mod,
                                    const VInputEvent* event);

VUID_PKG void v_node_on_key(VNode* self,
                            VNodeModule* mod,
                            const VInputEvent* event);

#endif  // VUID_NODE_NODE_INPUT_H

#ifndef VUID_NODE_NODE_H
#define VUID_NODE_NODE_H





typedef void (*VNodeCallbackFn)(VNode* node, void* user_data);
struct WeakRef;

typedef enum VNodeFlag {
  V_NODEFLAG_NONE = 0,
  V_NODEFLAG_DIRTY = 1 << 0,
  V_NODEFLAG_HIDDEN = 1 << 1,
  V_NODEFLAG_POPOVER_OPEN = 1 << 2,
  V_NODEFLAG_ATTACHED = 1 << 3,
  V_NODEFLAG_LEAF = 1 << 4,
  /* flag to determine if a popover was invoked during a mouse click */
  V_NODEFLAG_POPOVER_INVOKED = 1 << 5,
} VNodeFlag;

struct VNode {
  // TODO: pack these into a single field
  VNodeTag tag;
  VPopover popover_type;
  uint32_t ref_count;
  uint32_t flags;

  VStaticString id;

  VWeakRef* parent;
  VNode* first_child;
  VNode* last_child;
  VNode* next_sibling;
  VNode* prev_sibling;
  int child_count;

  VNodeEventListener event_listeners[V_NODE_EVENT__COUNT];

  VStyle* style;

  float scroll_y;
  float content_height;

  union {
    VImage* image_resource;
    VTextLayout* text_layout;
  } res;

  union {
    VString* text;
    VString* src;
  } res_data;

  void* user_data;
  VWeakRef* self_weak_ref;

  // Layout Info

  /* x/y relative to parent. if popover, x/y is absolute. w/h is content box. */
  VRect bounds;
  /* resolved margin px values. no info about auto or percent. */
  VInsets margin;
  /* resolved padding px. */
  VInsets padding;
  /* resolved border px. */
  VInsets border;

  // cached layout values.

  float min_width;
  float pref_width;
  float min_height;
  float pref_height;
};

typedef enum VNodeEventFlag {
  V_NODE_EVENT_FLAG_NONE = 0,
  V_NODE_EVENT_FLAG_CANCELLED = 1 << 0,
} VNodeEventFlag;

// clang-format off
VUID_PKG void          v_node_get_abs_pos(const VNode* node, float* x, float* y);
VUID_PKG void          v_node_set_attached(VNode* node, bool attached);
VUID_PKG float         v_node_get_max_scroll(const VNode* node);
VUID_PKG void          v_node_get_scrollbar_rect(VNode* node, float abs_x, float abs_y, VRect* track, VRect* thumb);
VUID_PKG bool          v_node_is_descendant_of(const VNode* node, const VNode* ancestor);
VUID_PKG VNode*        v_node_find_common_ancestor(VNode* a, VNode* b);
VUID_PKG VNode*        v_node_constructor(VNodeTag tag);
VUID_PKG void          v_node_render_root(VCommandQueue* cmdq);
VUID_PKG const VStyle* v_node_get_style_or_empty(const VNode* node);
VUID_PKG VTextLayout*  v_node_get_text_layout(const VNode* node);
VUID_PKG void          v_node_set_text_layout(VNode* node, VTextLayout* layout);
// clang-format on

static inline void v_node_set_flag(VNode* node, VNodeFlag flag) {
  node->flags |= flag;
}

static inline void v_node_clear_flag(VNode* node, VNodeFlag flag) {
  node->flags &= ~flag;
}

static inline bool v_node_has_flag(const VNode* node, VNodeFlag flag) {
  return (node->flags & flag) != 0;
}

static inline bool v_node_has_all_flags(const VNode* node, uint32_t flags) {
  return (node->flags & flags) == flags;
}

static inline bool v_node_has_parent(const VNode* node) {
  return node->parent && node->parent->ref;
}

/* preorder traversal of node and its children */
static inline void v_node_visit(VNode* node,
                                VNodeCallbackFn visit,
                                void* user_data) {
  visit(node, user_data);

  v_foreach_child(node, child) {
    v_node_visit(child, visit, user_data);
  }
}

static inline void v_node_event_set_flag(VNodeEvent* event,
                                         VNodeEventFlag flag) {
  event->internal |= flag;
}

static inline bool v_node_event_has_flag(const VNodeEvent* event,
                                         VNodeEventFlag flag) {
  return (event->internal & flag) != 0;
}

static inline bool v_node_event_was_cancelled(const VNodeEvent* event) {
  return v_node_event_has_flag(event, V_NODE_EVENT_FLAG_CANCELLED);
}

static inline float v_node_inset_left(const VNode* node) {
  return node->border.left + node->padding.left;
}

static inline float v_node_inset_top(const VNode* node) {
  return node->border.top + node->padding.top;
}

static inline float v_node_inset_width(const VNode* node) {
  return node->border.left + node->border.right + node->padding.left +
         node->padding.right;
}

static inline float v_node_inset_height(const VNode* node) {
  return node->border.top + node->border.bottom + node->padding.top +
         node->padding.bottom;
}

// starting from the node itself, traverse up the ancestors until null
#define v_foreach_ancestor(NODE, ITER) \
  for (VNode* ITER = NODE; ITER; ITER = v_node_parent(ITER))

#endif  // VUID_NODE_NODE_H

#ifndef VUID_NODE_STYLE_PROPERTY_H
#define VUID_NODE_STYLE_PROPERTY_H



typedef enum VStylePropertyTag {
  VSTAG_UNSET,
  VSTAG_SIZING,
  VSTAG_COLOR,
  VSTAG_STRING,
  VSTAG_STYLE_VALUE,
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
  VSTAG_ENUM_VISIBILITY,
} VStylePropertyTag;

typedef struct VStylePropertyMeta {
  VStylePropertyTag tag;

  union {
    VStyleValue style_value;
    VColor color;
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
    VVisibility visibility;
    const char* str;
  } default_value;

  union {
    void (*style_value)(VStyle*, VStyleValue);
    void (*color)(VStyle*, VColor);
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
    void (*visibility)(VStyle*, VVisibility);
    void (*str)(VStyle*, const char*);
  } set_fn;

  union {
    VStyleValue (*style_value)(const VStyle*);
    VColor (*color)(const VStyle*);
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
    VVisibility (*visibility)(const VStyle*);
    const char* (*str)(const VStyle*);
  } get_fn;

  union {
    bool (*style_value)(VStyleValue*);
    bool (*str)(const char**);
  } validate_fn;

  /* if true, change to this prop marks owner as dirty. */
  bool affects_layout;
  /* if true, change to this prop requires owner text layout to rebuild. */
  bool affects_text_style;
  /* if true, change to this prop requires a re-render of the owner. */
  bool affects_render;
} VStylePropertyMeta;

VUID_PKG const VStylePropertyMeta* v_style_get_prop_meta(
    VStyleProperty property);

#endif  // VUID_NODE_STYLE_PROPERTY_H

#ifndef VUID_NODE_STYLE_H
#define VUID_NODE_STYLE_H



typedef enum VSizeMode {
  V_SIZE_MODE_FIT,
  V_SIZE_MODE_GROW,
  V_SIZE_MODE_FIXED,
} VSizeMode;

struct VStyle {
  const char* font;
  VStyleValue width;
  VStyleValue min_width;
  VStyleValue max_width;
  VStyleValue height;
  VStyleValue min_height;
  VStyleValue max_height;
  VStyleValue top;
  VStyleValue right;
  VStyleValue bottom;
  VStyleValue left;
  VStyleValue gap;
  VStyleValue aspect_ratio;
  VStyleValue margin_top;
  VStyleValue margin_right;
  VStyleValue margin_bottom;
  VStyleValue margin_left;
  VStyleValue attach_point_offset_x;
  VStyleValue attach_point_offset_y;
  VStyleValue padding_top;
  VStyleValue padding_right;
  VStyleValue padding_bottom;
  VStyleValue padding_left;
  VStyleValue border_top;
  VStyleValue border_right;
  VStyleValue border_bottom;
  VStyleValue border_left;
  VStyleValue font_size;
  VStyleValue border_radius;
  VStyleValue scrollbar_width;
  VStyleValue scrollbar_border_radius;
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
  VVisibility visibility;

  uint32_t ref_count;

  VNode* owner;
  uint64_t is_set;
};

VUID_PKG VStyle* v_style_new(VNode* owner);

VUID_PKG bool v_style_class_init(VStyleClass* self, const char* id);
VUID_PKG void v_style_class_drop(VStyleClass* self);
VUID_PKG bool v_style_class_is_valid_name(const char* name);
VUID_PKG void v_style_flatten(VStyle* a, VStyle* b);
VUID_PKG VNode* v_style_get_owner(const VStyle* style);
VUID_PKG void v_style_set_owner(VStyle* style, VNode* owner);

static inline VSizeMode v_style_resolve_size_mode(VStyleValueUnit unit) {
  switch (unit) {
    case V_STYLE_VALUE_UNIT_AUTO:
      // TODO: auto policy should consider parent's direction
      return V_SIZE_MODE_FIT;
    case V_STYLE_VALUE_UNIT_FIT:
      return V_SIZE_MODE_FIT;
    case V_STYLE_VALUE_UNIT_GROW:
      return V_SIZE_MODE_GROW;
    default:
      return V_SIZE_MODE_FIXED;
  }
}

static inline float v_style_resolve_length(const VStyleValue* value) {
  switch (value->unit) {
    case V_STYLE_VALUE_UNIT_PX:
      return value->value.px;
    case V_STYLE_VALUE_UNIT_AUTO:
    default:
      return 0.0f;
  }
}

static inline float v_style_resolve_length_ceil(const VStyleValue* value) {
  switch (value->unit) {
    case V_STYLE_VALUE_UNIT_PX:
      return ceilf(value->value.px);
    case V_STYLE_VALUE_UNIT_AUTO:
    default:
      return 0.0f;
  }
}

/* check if a style has a specific property set */
static inline bool vs_has_prop(const VStyle* style, VStyleProperty property) {
  return (style->is_set & (((uint64_t)1) << property)) != 0;
}

// TODO: these resolve functions are kinda awful.. refactor

static inline uint16_t v_style_resolve_font_size(const VStyle* style) {
  return vs_has_prop(style, VS_FONT_SIZE)
             ? (uint16_t)v_style_resolve_length_ceil(&style->font_size)
             : 0;
}

static inline uint16_t v_style_resolve_scrollbar_width(const VStyle* style) {
  return vs_has_prop(style, VS_SCROLLBAR_WIDTH)
             ? (uint16_t)v_style_resolve_length_ceil(&style->scrollbar_width)
             : 0;
}

static inline uint16_t v_style_resolve_scrollbar_border_radius(
    const VStyle* style) {
  return vs_has_prop(style, VS_SCROLLBAR_BORDER_RADIUS)
             ? (uint16_t)v_style_resolve_length_ceil(
                   &style->scrollbar_border_radius)
             : 0;
}

static inline uint16_t v_style_resolve_border_radius(const VStyle* style) {
  return vs_has_prop(style, VS_BORDER_RADIUS)
             ? (uint16_t)v_style_resolve_length_ceil(&style->border_radius)
             : 0;
}

static inline VSizeMode v_style_resolve_width_size_mode(const VStyle* style) {
  return v_style_resolve_size_mode(
      vs_has_width(style) ? vs_get_width(style).unit : V_STYLE_VALUE_UNIT_AUTO);
}

static inline VSizeMode v_style_resolve_height_size_mode(const VStyle* style) {
  return v_style_resolve_size_mode(vs_has_height(style)
                                       ? vs_get_height(style).unit
                                       : V_STYLE_VALUE_UNIT_AUTO);
}

static inline bool v_style_resolve_fixed_width(const VStyle* style,
                                               float* out) {
  return vs_has_prop(style, VS_WIDTH) &&
         style->width.unit == V_STYLE_VALUE_UNIT_PX &&
         (*out = style->width.value.px, true);
}

static inline bool v_style_resolve_fixed_height(const VStyle* style,
                                                float* out) {
  return vs_has_prop(style, VS_HEIGHT) &&
         style->height.unit == V_STYLE_VALUE_UNIT_PX &&
         (*out = style->height.value.px, true);
}

static inline float v_style_resolve_min_width(const VStyle* style) {
  return vs_has_prop(style, VS_MIN_WIDTH)
             ? v_style_resolve_length(&style->min_width)
             : 0.0f;
}

static inline float v_style_resolve_max_width(const VStyle* style) {
  return vs_has_prop(style, VS_MAX_WIDTH)
             ? v_style_resolve_length(&style->max_width)
             : 0.0f;
}

static inline float v_style_resolve_min_height(const VStyle* style) {
  return vs_has_prop(style, VS_MIN_HEIGHT)
             ? v_style_resolve_length(&style->min_height)
             : 0.0f;
}

static inline float v_style_resolve_max_height(const VStyle* style) {
  return vs_has_prop(style, VS_MAX_HEIGHT)
             ? v_style_resolve_length(&style->max_height)
             : 0.0f;
}

static inline float v_style_resolve_top(const VStyle* style) {
  return vs_has_prop(style, VS_TOP) ? v_style_resolve_length(&style->top)
                                    : 0.0f;
}

static inline float v_style_resolve_right(const VStyle* style) {
  return vs_has_prop(style, VS_RIGHT) ? v_style_resolve_length(&style->right)
                                      : 0.0f;
}

static inline float v_style_resolve_bottom(const VStyle* style) {
  return vs_has_prop(style, VS_BOTTOM) ? v_style_resolve_length(&style->bottom)
                                       : 0.0f;
}

static inline float v_style_resolve_left(const VStyle* style) {
  return vs_has_prop(style, VS_LEFT) ? v_style_resolve_length(&style->left)
                                     : 0.0f;
}

static inline float v_style_resolve_gap(const VStyle* style) {
  return vs_has_prop(style, VS_GAP) ? v_style_resolve_length(&style->gap)
                                    : 0.0f;
}

static inline float v_style_resolve_aspect_ratio(const VStyle* style) {
  // TODO: support auto
  return vs_has_prop(style, VS_ASPECT_RATIO)
             ? v_style_resolve_length(&style->aspect_ratio)
             : 0.0f;
}

static inline bool v_style_margin_is_auto_top(const VStyle* style) {
  return vs_has_prop(style, VS_MARGIN_TOP) &&
         style->margin_top.unit == V_STYLE_VALUE_UNIT_AUTO;
}

static inline bool v_style_margin_is_auto_right(const VStyle* style) {
  return vs_has_prop(style, VS_MARGIN_RIGHT) &&
         style->margin_right.unit == V_STYLE_VALUE_UNIT_AUTO;
}

static inline bool v_style_margin_is_auto_bottom(const VStyle* style) {
  return vs_has_prop(style, VS_MARGIN_BOTTOM) &&
         style->margin_bottom.unit == V_STYLE_VALUE_UNIT_AUTO;
}

static inline bool v_style_margin_is_auto_left(const VStyle* style) {
  return vs_has_prop(style, VS_MARGIN_LEFT) &&
         style->margin_left.unit == V_STYLE_VALUE_UNIT_AUTO;
}

static inline float v_style_resolve_attach_point_offset_x(const VStyle* style) {
  return vs_has_prop(style, VS_ATTACH_POINT_OFFSET_X)
             ? v_style_resolve_length(&style->attach_point_offset_x)
             : 0.0f;
}

static inline float v_style_resolve_attach_point_offset_y(const VStyle* style) {
  return vs_has_prop(style, VS_ATTACH_POINT_OFFSET_Y)
             ? v_style_resolve_length(&style->attach_point_offset_y)
             : 0.0f;
}

#endif  // VUID_NODE_STYLE_H

#ifndef VUID_NODE_NODE_MODULE_H
#define VUID_NODE_NODE_MODULE_H




typedef struct VNodeIdSetValue {
  VNode* node;
} VNodeIdSetValue;

typedef struct VNodeIdSetKey {
  const char* id;
} VNodeIdSetKey;

static inline VNodeIdSetValue VNodeIdSetValue_init(VNode* node) {
  return (VNodeIdSetValue){.node = node};
}

static inline VNodeIdSetKey VNodeIdSetKey_init(const char* id) {
  return (VNodeIdSetKey){.id = id};
}

VUID_HMAP_DECLARE(VNodeIdSet, VNodeIdSetKey, VNodeIdSetValue, v_node_id_set);

struct VNodeModule {
  VAllocator* allocator;
  VNodeIdSet nodes_by_id;
  VStyleClassHashMap style_sheet;
  VNode* root;
  int root_width;
  int root_height;
  VStyle* empty_style;
  VArray /*<Node*[]>*/ popover_stack;
  VArray /*<Node*[]>*/ event_path;
  VNode* hovered_node;
  VNode* under_mouse_node;
  VNode* drag_node;
  VNode* focused_node;
  float drag_start_y;
  float drag_start_scroll_y;
  VColor popover_backdrop_color;
};

// clang-format off
VUID_PKG bool    v_node_module_init(VNodeModule* self, VAllocator* allocator);
VUID_PKG bool    v_node_module_post_init(VNodeModule* self);
VUID_PKG void    v_node_module_drop(VNodeModule* self);

VUID_PKG VArray* v_node_module_get_popover_stack(void);
VUID_PKG void    v_node_module_add_node_id(VNode* node);
VUID_PKG void    v_node_module_remove_node_id(VNode* node);
VUID_PKG void    v_node_module_remove_popover_node(VNode* node);
VUID_PKG void    v_node_module_remove_input_node(VNode* node);

/* get the start index of the event path for pushing new nodes */
VUID_PKG size_t  v_event_path_start(VArray* event_path);
/* reset the event path to a given start index, unrefing nodes that are removed from the path */
VUID_PKG void    v_event_path_reset(VArray* event_path, size_t start_index);
/* add a node to the event path. if the node does not have the event listener, it is not added to the path */
VUID_PKG void    v_event_path_push_ref(VArray* event_path, VNodeEventType type, VNode* node);
/* dispatch an event along the event path from start_index (inclusive) to end_index (exclusive) */
VUID_PKG void    v_event_path_dispatch(VArray* event_path, VNodeEvent* event, size_t start_index, size_t end_index);
// clang-format on

static inline VStyleClassHashMap* v_node_module_get_style_sheet(void) {
  return &v_ctx_node_module()->style_sheet;
}

#endif  // VUID_NODE_NODE_MODULE_H

#ifndef VUID_NODE_NODE_LAYOUT_H
#define VUID_NODE_NODE_LAYOUT_H


void v_node_layout(VNode* self, VNodeModule* mod, int width, int height);

#endif  // VUID_NODE_NODE_LAYOUT_H


static void* v_allocator_libc_alloc(void* impl,
                                    size_t size,
                                    bool zero_initialize) {
  UNUSED(impl);
  void* chunk = malloc(size);

  if (chunk && zero_initialize) {
    memset(chunk, 0, size);
  }

  return chunk;
}

static void* v_allocator_libc_realloc(void* impl,
                                      void* ptr,
                                      size_t old_size,
                                      size_t new_size) {
  UNUSED(impl, old_size);
  return realloc(ptr, new_size);
}

static void v_allocator_libc_free(void* impl, void* ptr, size_t size) {
  UNUSED(impl, size);
  free(ptr);
}

VAllocator v_allocator_libc(void) {
  return (VAllocator){
      .impl = NULL,
      .alloc_fn = &v_allocator_libc_alloc,
      .realloc_fn = &v_allocator_libc_realloc,
      .free_fn = &v_allocator_libc_free,
  };
}


static inline bool v_array__grow(VArray* self) {
  // TODO: capacity calc can overflow
  // TODO: is grow factor of 2 what we want?
  const size_t new_capacity = self->capacity == 0
                                  ? VUID_ARRAY_BASE_CAPACITY
                                  : self->capacity * VUID_ARRAY_GROW_FACTOR;
  void* new_items =
      v_realloc(self->allocator, self->items, self->item_size * self->capacity,
                self->item_size * new_capacity);
  // TODO: handle allocation failure

  if (!new_items) {
    return false;
  }

  self->items = new_items;
  self->capacity = new_capacity;

  return true;
}

VUID_PKG VArray v_array_init(VAllocator* allocator,
                             size_t item_size,
                             size_t initial_capacity) {
  VArray self = {
      .allocator = allocator,
      .item_size = item_size,
      .capacity = initial_capacity,
  };

  if (initial_capacity > 0) {
    v_array__grow(&self);
  }

  return self;
}

VUID_PKG void v_array_drop(VArray* self) {
  if (self->items) {
    v_free(self->allocator, self->items, self->capacity * self->item_size);
  }
}

VUID_PKG bool v_array_push(VArray* self, void* item) {
  void* target = v_array_push_one(self);

  if (target) {
    memcpy(target, item, self->item_size);
  }

  return (target != NULL);
}

VUID_PKG void* v_array_push_one(VArray* self) {
  if (self->size >= self->capacity && !v_array__grow(self)) {
    return NULL;
  }

  return v_array_get_unchecked(self, self->size++);
}

VUID_PKG bool v_array_push_n(VArray* self, size_t count) {
  if (self->size + count > self->capacity && !v_array__grow(self)) {
    return false;
  }

  self->size += count;
  return true;
}

VUID_PKG void v_array_pop(VArray* self) {
  if (self->size > 0) {
    --self->size;
  }
}

VUID_PKG void* v_array_pop_ptr(VArray* self) {
  void* result;

  if (self->size > 0) {
    result = ((void**)self->items)[self->size - 1];
    self->size--;
  } else {
    result = NULL;
  }

  return result;
}

VUID_PKG void* v_array_get(const VArray* array, size_t index) {
  return index < array->size ? v_array_get_unchecked(array, index) : NULL;
}

VUID_PKG void v_array_remove(VArray* self, size_t index) {
  if (index >= self->size) {
    return;
  }

  if (index < self->size - 1) {
    memmove(v_array_get_unchecked(self, index),
            v_array_get_unchecked(self, index + 1),
            (self->size - index - 1) * self->item_size);
  }

  --self->size;
}

VUID_PKG bool v_array_insert(VArray* self, size_t index, void* item) {
  if (self->size >= self->capacity && !v_array__grow(self)) {
    return false;
  }

  if (index >= self->size) {
    return v_array_push(self, item);
  }

  memmove(v_array_get_unchecked(self, index + 1),
          v_array_get_unchecked(self, index),
          (self->size - index) * self->item_size);

  memcpy(v_array_get_unchecked(self, index), item, self->item_size);
  self->size++;
  return true;
}

VUID_PKG void v_array_apply(VArray* self,
                            void (*fn)(void* item, void* ctx),
                            void* ctx) {
  for (size_t i = 0; i < self->size; i++) {
    fn(v_array_get_unchecked(self, i), ctx);
  }
}



static FILE* v_fopen_rb(VAllocator* allocator, const char* path);

VUID_PKG uint8_t* v_file_read(VAllocator* allocator,
                              const char* path,
                              size_t* size_out) {
  FILE* f = v_fopen_rb(allocator, path);

  if (!f) {
    return NULL;
  }

  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return NULL;
  }

  long file_size = ftell(f);
  if (file_size <= 0) {
    fclose(f);
    return NULL;
  }

  rewind(f);

  uint8_t* buf = v_alloc_raw(allocator, (size_t)file_size);
  if (!buf) {
    fclose(f);
    return NULL;
  }

  size_t n_read = fread(buf, 1, (size_t)file_size, f);
  fclose(f);

  if (n_read != (size_t)file_size) {
    v_free(allocator, buf, (size_t)file_size);
    return NULL;
  }

  *size_out = (size_t)file_size;
  return buf;
}

/* use this trick instead of including windows.h */
#if defined(_WIN32)
#define VUID_WIN32_CP_UTF8 65001
extern __declspec(dllimport) int __stdcall MultiByteToWideChar(
    unsigned int code_page,
    unsigned long flags,
    const char* mb_str,
    int mb_str_len,
    wchar_t* wide_str,
    int wide_str_len);
#endif

static FILE* v_fopen_rb(VAllocator* allocator, const char* path) {
  FILE* f;
#if defined(_WIN32)
  int pathw_len = MultiByteToWideChar(VUID_WIN32_CP_UTF8, 0, path, -1, NULL, 0);

  if (pathw_len <= 0) {
    return NULL;
  }

  // TODO: use a scratch arena?
  wchar_t* pathw = v_alloc_raw(allocator, (size_t)pathw_len * sizeof(wchar_t));

  if (!pathw) {
    return NULL;
  }

  int result =
      MultiByteToWideChar(VUID_WIN32_CP_UTF8, 0, path, -1, pathw, pathw_len);

  if (result == 0) {
    f = NULL;
  } else {
#if defined(_MSC_VER) && _MSC_VER >= 1400
    if (0 != _wfopen_s(&f, pathw, L"rb")) {
      f = 0;
    }
#else
    f = _wfopen(pathw, L"rb");
#endif
  }
  v_free(allocator, pathw, (size_t)pathw_len * sizeof(wchar_t));
#else
  UNUSED(allocator);
  f = fopen(path, "rb");
#endif
  return f;
}


#define VUID_UTF8_ACCEPT 0
#define VUID_UTF8_REJECT 1

// TODO: not sure about this design, but fine for now
static VString VUID_EMPTY_STRING = {
    .u.external = "",
    .meta = VUID_STRING_EXTERNAL_BIT,
    .refs = 0,
};

static uint32_t v_utf8_decode(uint32_t* state, uint32_t* codep, uint32_t byte);

VUID_PKG bool v_utf8_get_codepoint(const uint8_t* utf8,
                                   uint32_t* codepoint,
                                   uint32_t* size) {
  uint32_t state = VUID_UTF8_ACCEPT;
  uint32_t cp = 0;
  uint32_t s = 0;

  while (*utf8) {
    if (v_utf8_decode(&state, &cp, (uint8_t)*utf8) == VUID_UTF8_ACCEPT) {
      *codepoint = cp;
      *size = s + 1;
      return true;
    }
    s++;
  }

  return false;
}

VUID_PKG VString* v_string_from(VAllocator* allocator, const char* str) {
  const size_t size = str ? strlen(str) : 0;

  if (size == 0) {
    return &VUID_EMPTY_STRING;
  }

  if (size > VUID_STRING_MAX_SIZE) {
    return NULL;
  }

  VString* result = v_alloc_raw(allocator, sizeof(VString) + size + 1);

  if (result) {
    char* data = (char*)(result + 1);

    memcpy(data, str, size);
    data[size] = '\0';
    result->u.allocator = allocator;
    result->meta = (uint32_t)size;
    result->refs = 1;
  }

  return result;
}

VUID_PKG VString* v_string_from_vfmt(VAllocator* allocator,
                                     const char* fmt,
                                     va_list args) {
  va_list args_copy;

  va_copy(args_copy, args);
  int size = vsnprintf(NULL, 0, fmt, args_copy);
  va_end(args_copy);

  if (size == 0) {
    return &VUID_EMPTY_STRING;
  }

  if (size > (int)VUID_STRING_MAX_SIZE) {
    return NULL;
  }

  VString* result = v_alloc_raw(allocator, sizeof(VString) + size + 1);

  if (result) {
    char* data = (char*)(result + 1);

    vsnprintf(data, size + 1, fmt, args);
    result->u.allocator = allocator;
    result->meta = (uint32_t)size;
    result->refs = 1;
  }

  return result;
}

VUID_PKG void v_string_ref(VString* self) {
  if (self && !v_string_is_external(self)) {
    if (self->refs < UINT32_MAX) {
      self->refs++;
    } else {
      // TODO: VUID_ASSERT(false && "VString ref count overflow");
    }
  }
}

VUID_PKG void v_string_unref(VString* self) {
  if (self && !v_string_is_external(self)) {
    const uint32_t ref_count = self->refs;

    if (ref_count > 1) {
      self->refs--;
    } else if (ref_count == 1) {
      v_free(self->u.allocator, self,
             (uint32_t)sizeof(VString) + self->meta + 1);
    } else {
      // TODO: VUID_ASSERT(false && "VString ref count underflow");
    }
  }
}

VUID_PKG const char* v_string_cstr(const VString* self) {
  if (!self) {
    return "";
  }

  return v_string_is_external(self) ? self->u.external
                                    : (const char*)(self + 1);
}

VUID_PKG bool v_string_eq(const VString* a, const VString* b) {
  return (a == b) || v_cstr_eq(v_string_cstr(a), v_string_cstr(b));
}

VUID_PKG bool v_string_eq_cstr(const VString* a, const char* b) {
  return v_cstr_eq(v_string_cstr(a), b);
}

VUID_PKG bool v_string_is_empty(const VString* self) {
  return self ? (self->meta & ~(VUID_STRING_EXTERNAL_BIT)) == 0 : true;
}

VUID_PKG uint32_t v_string_size(const VString* self) {
  return self ? self->meta & ~(VUID_STRING_EXTERNAL_BIT) : 0;
}

// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

// clang-format off
static const uint8_t VUID_UTF8D[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
  8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
  0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
  0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
  0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
  1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
  1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
  1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};
// clang-format on

static uint32_t v_utf8_decode(uint32_t* state, uint32_t* codep, uint32_t byte) {
  uint32_t type = VUID_UTF8D[byte];

  *codep = (*state != VUID_UTF8_ACCEPT) ? (byte & 0x3fu) | (*codep << 6)
                                        : (0xff >> type) & (byte);

  *state = VUID_UTF8D[256 + *state * 16 + type];
  return *state;
}






typedef struct VContext VContext;

static void v_ctx_pre_render(VContext* ctx);
static void v_ctx_post_render(VContext* ctx);
static VTextureInfo* v_ctx_get_texture(VContext* ctx, uint32_t texture_id);
static void v_rasterize_text_node(VNode* node);
static void v_ctx_render_data_sync(VContext* ctx);

/* ============================================================
 * Internal context definition
 * ============================================================ */

struct VContext {
  VAllocator allocator;

  VNodeModule node_module;
  VTextModule text_module;
  VImageStore image_store;

  VRenderData render_data;
  VArray /*<VTextureInfo[]>*/ textures;
  VCommandQueue command_queue;
  VStringPool string_pool;   /* node id and style class interned strings */
  uint32_t next_texture_id;  /* starts at 1; 0 is the invalid sentinel */
  uint32_t atlas_texture_id; /* texture id for the glyph atlas */
  uint32_t atlas_last_modified_count;
  uint32_t atlas_last_size;

  VArray /*<VInputEvent[]>*/ input_event_queue;

  int object_count[V_OBJECT_TYPE__COUNT];
  bool initialized;
  bool needs_render;
};

static VContext g_context = {0};

/* ============================================================
 * Internal context API (vuid/context/mod.h)
 * ============================================================ */

/* ============================================================
 * Public API  (vuid.h)
 * ============================================================ */

VUID_API bool v_init(const VConfig* config) {
  if (g_context.initialized) {
    return false;
  }

  uint32_t atlas_size = config ? config->atlas_size : 0;
  uint32_t atlas_max_size = config ? config->atlas_max_size : 0;
  VImageLoaderFn image_loader = config ? config->image_loader : NULL;

  g_context = (VContext){
      .initialized = true,
      .allocator = v_allocator_libc(),
      .next_texture_id = 1,
  };

  VAllocator* allocator = &g_context.allocator;

  g_context.input_event_queue =
      v_array_init(allocator, sizeof(VInputEvent), 32);
  g_context.textures = v_array_init(allocator, sizeof(VTextureInfo), 8);

  VTextureInfo atlas_texture_info = {
      .id = g_context.next_texture_id++,
      .state = V_TEXTURE_PENDING,
  };

  if (!v_array_push(&g_context.textures, &atlas_texture_info)) {
    goto error;
  }

  VUID_ASSERT(atlas_texture_info.id == VUID_ATLAS_TEXTURE_ID);
  g_context.atlas_texture_id = atlas_texture_info.id;

  g_context.render_data = (VRenderData){
      .textures = g_context.textures.items,
      .texture_count = (uint32_t)g_context.textures.size,
  };

  if (!v_string_pool_init(&g_context.string_pool, allocator, 512)) {
    goto error;
  }

  if (!v_command_queue_init(&g_context.command_queue, allocator)) {
    goto error;
  }

  if (!v_image_store_init(&g_context.image_store, allocator, image_loader)) {
    goto error;
  }

  if (!v_text_module_init(&g_context.text_module, allocator, atlas_size,
                          atlas_max_size)) {
    goto error;
  }

  if (!v_node_module_init(&g_context.node_module, allocator)) {
    goto error;
  }

  if (!v_node_module_post_init(&g_context.node_module)) {
    goto error;
  }

  v_ctx_render_data_sync(&g_context);

  return true;

error:
  v_quit();
  return false;
}

VUID_API void v_quit(void) {
  // TODO: what happens if quit is called during a callback?
  V_CHECK_CONTEXT();

  v_node_module_drop(&g_context.node_module);
  v_text_module_drop(&g_context.text_module);
  v_image_store_drop(&g_context.image_store);

  v_array_drop(&g_context.input_event_queue);
  v_array_drop(&g_context.textures);
  v_string_pool_drop(&g_context.string_pool);
  v_command_queue_drop(&g_context.command_queue);

  VUID_ASSERT(g_context.object_count[V_OBJECT_TYPE_NODE] == 0);
  VUID_ASSERT(g_context.object_count[V_OBJECT_TYPE_STYLE] == 0);
  VUID_ASSERT(g_context.object_count[V_OBJECT_TYPE_WEAK_NODE_REF] == 0);
  VUID_ASSERT(g_context.object_count[V_OBJECT_TYPE_IMAGE] == 0);

  g_context = (VContext){0};
}

VUID_API void v_update(int width, int height) {
  V_CHECK_CONTEXT();
  v_node_layout(g_context.node_module.root, &g_context.node_module, width,
                height);
}

VUID_API void v_render(void) {
  V_CHECK_CONTEXT();
  v_ctx_pre_render(&g_context);

  if (g_context.needs_render) {
    v_command_queue_clear(&g_context.command_queue);
    v_node_render_root(&g_context.command_queue);
    g_context.needs_render = false;

    v_ctx_post_render(&g_context);
  }
}

VUID_API VRenderData* v_get_render_data(void) {
  V_CHECK_CONTEXT(NULL);
  return &g_context.render_data;
}

VUID_API bool v_add_font(const char* name, const char* path) {
  V_CHECK_CONTEXT(0);

  if (!v_font_data_is_legal_name(name) || v_cstr_is_empty(path)) {
    return false;
  }

  size_t size = 0;
  uint8_t* data = v_file_read(&g_context.allocator, path, &size);

  if (!data) {
    return false;
  }

  VBuffer file_buffer = {
      .allocator = &g_context.allocator,
      .data = data,
      .size = size,
      .source = data,
  };

  return v_text_module_add_font(&g_context.text_module, name, &file_buffer);
}

VUID_API bool v_add_font_mem(const char* name,
                             const void* data,
                             size_t size,
                             VMemoryMode mode) {
  V_CHECK_CONTEXT(0);

  if (!v_font_data_is_legal_name(name) || !data || size == 0) {
    return false;
  }

  VBuffer file_buffer;

  switch (mode) {
    case V_MEMORY_MODE_COPY:
      if (!v_buffer_from(&file_buffer, data, size, &g_context.allocator)) {
        return false;
      }
      break;
    case V_MEMORY_MODE_READONLY:
      file_buffer = v_buffer_init_static(data, size);
      break;
    default:
      return false;
  }

  if (v_buffer_is_empty(&file_buffer)) {
    return false;
  }

  return v_text_module_add_font(&g_context.text_module, name, &file_buffer);
}

VUID_API void v_remove_font(const char* name) {
  V_CHECK_CONTEXT();
  if (v_cstr_is_empty(name)) {
    return;
  }

  v_text_module_remove_font(&g_context.text_module, name);
}

VUID_API void v_process_events(void) {
  // TODO: check that this function is not called reentrantly or another
  // lifecycle function is called
  V_CHECK_CONTEXT();

  for (size_t i = 0; i < g_context.input_event_queue.size; i++) {
    VInputEvent* event = v_array_get_unchecked(&g_context.input_event_queue, i);

    switch (event->type) {
      case V_INPUT_EVENT_MOUSE_BUTTON:
        v_node_on_mouse_button(g_context.node_module.root,
                               &g_context.node_module, event);
        break;
      case V_INPUT_EVENT_MOUSE_MOVE:
        v_node_on_mouse_move(g_context.node_module.root, &g_context.node_module,
                             event);
        break;
      case V_INPUT_EVENT_MOUSE_WHEEL:
        v_node_on_mouse_wheel(g_context.node_module.root,
                              &g_context.node_module, event);
        break;
      case V_INPUT_EVENT_KEY:
        v_node_on_key(g_context.node_module.focused_node,
                      &g_context.node_module, event);
        break;
      default:
        break;
    }
  }

  v_array_clear(&g_context.input_event_queue);
}

VUID_API void v_add_input_event(VInputEvent* event) {
  V_CHECK_CONTEXT();
  v_array_push(&g_context.input_event_queue, event);
}

VUID_API VNode* v_root(void) {
  V_CHECK_CONTEXT(NULL);
  return g_context.node_module.root;
}

VUID_API VNode* v_get_node_by_id(const char* id) {
  V_CHECK_CONTEXT(NULL);

  if (v_cstr_is_empty(id)) {
    return NULL;
  }

  VNodeIdSetValue* value = v_node_id_set_get(&g_context.node_module.nodes_by_id,
                                             VNodeIdSetKey_init(id));

  return value ? value->node : NULL;
}

VUID_API VNode* v_get_node_by_id_fmt(const char* fmt, ...) {
  V_CHECK_CONTEXT(NULL);

  if (v_cstr_is_empty(fmt)) {
    return NULL;
  }

  // TODO: use a scratch buffer
  char buf[VUID_MAX_NODE_ID_LENGTH + 1];

  va_list args;
  va_start(args, fmt);
  int result = vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if (result < 0 || result > VUID_MAX_NODE_ID_LENGTH) {
    return NULL;
  }

  return v_get_node_by_id(buf);
}

VUID_API void v_set_popover_backdrop_color(VColor c) {
  V_CHECK_CONTEXT();
  g_context.node_module.popover_backdrop_color = c;
}

VColor v_get_popover_backdrop_color(void) {
  V_CHECK_CONTEXT((VColor){0});
  return g_context.node_module.popover_backdrop_color;
}

VUID_API VNode* v_get_focused_node(void) {
  V_CHECK_CONTEXT(NULL);
  return g_context.node_module.focused_node;
}

VUID_API bool v_add_image(const char* src) {
  V_CHECK_CONTEXT(false);
  VImageStore* image_store = v_ctx_image_store();

  return v_image_store_persist(image_store, src);
}

VUID_API bool v_add_image_mem(const char* src,
                              const void* data,
                              size_t size,
                              VMemoryMode mode) {
  V_CHECK_CONTEXT(false);
  VBuffer buffer;

  switch (mode) {
    case V_MEMORY_MODE_COPY:
      if (!v_buffer_from(&buffer, data, size, &g_context.allocator)) {
        return false;
      }
      break;
    case V_MEMORY_MODE_READONLY:
      buffer = v_buffer_init_static(data, size);
      break;
    default:
      return false;
  }

  VImageStore* image_store = v_ctx_image_store();

  return v_image_store_persist_mem(image_store, src, &buffer);
}

VUID_API void v_remove_image(const char* src) {
  V_CHECK_CONTEXT();
  VImageStore* image_store = v_ctx_image_store();

  v_image_store_remove(image_store, src);
}

VUID_PKG bool v_ctx_is_initialized(void) {
  return g_context.initialized;
}

VUID_PKG VAllocator* v_ctx_allocator(void) {
  return &g_context.allocator;
}

VUID_PKG void v_ctx_object_inc(VObjectType type) {
  g_context.object_count[type]++;
}

VUID_PKG void v_ctx_object_dec(VObjectType type) {
  g_context.object_count[type]--;
}

VUID_PKG VTextModule* v_ctx_text_module(void) {
  return &g_context.text_module;
}

VUID_PKG VNodeModule* v_ctx_node_module(void) {
  return &g_context.node_module;
}

VUID_PKG VImageStore* v_ctx_image_store(void) {
  return &g_context.image_store;
}

VUID_PKG bool v_ctx_intern_string(const char* str, VStaticString* out) {
  return v_string_pool_intern(&g_context.string_pool, str, out);
}

VUID_PKG void v_ctx_request_render(void) {
  if (!g_context.needs_render) {
    g_context.needs_render = true;
  }
}

//
// render stuff
//

// TODO: this should not be public. fix image store design.
VUID_PKG uint32_t v_ctx_create_texture(VPixelFormat format,
                                       const VImageBuffer* buffer) {
  VContext* ctx = &g_context;

  VTextureInfo texture_info = {
      .id = ctx->next_texture_id,
      .width = buffer->width,
      .height = buffer->height,
      .texture_format = format,
      .state = V_TEXTURE_CREATING,
      .pixels = *buffer,
  };

  if (!v_array_push(&ctx->textures, &texture_info)) {
    return 0;
  }

  return ctx->next_texture_id++;
}

// TODO: this should not be public. fix image store design.
VUID_PKG void v_ctx_destroy_texture(uint32_t texture_id) {
  VTextureInfo* info = v_ctx_get_texture(&g_context, texture_id);

  if (info && info->state != V_TEXTURE_DESTROYED) {
    info->state = V_TEXTURE_DESTROYING;
  }
}

static void v_ctx_post_render(VContext* ctx) {
  v_ctx_render_data_sync(ctx);
}

static void v_ctx_pre_render(VContext* ctx) {
  VGlyphAtlas* atlas = &ctx->text_module.atlas;

  v_rasterize_text_node(ctx->node_module.root);

  // sync atlas texture state with the glyph atlas
  if (v_glyph_atlas_get_size(atlas) != ctx->atlas_last_size) {
    VTextureInfo* info = v_ctx_get_texture(ctx, ctx->atlas_texture_id);
    VUID_ASSERT(info);
    VImageBuffer pixel_buffer = v_glyph_atlas_get_pixel_buffer(atlas);

    // TODO: this is an upsert/resize, rather than a create..
    info->state = V_TEXTURE_CREATING;
    info->texture_format = pixel_buffer.format;
    info->width = pixel_buffer.width;
    info->height = pixel_buffer.height;
    info->pixels = pixel_buffer;

    ctx->atlas_last_size = v_glyph_atlas_get_size(atlas);
    ctx->atlas_last_modified_count = v_glyph_atlas_get_modified_count(atlas);

    v_ctx_request_render();
  } else if (v_glyph_atlas_get_modified_count(atlas) !=
             ctx->atlas_last_modified_count) {
    VTextureInfo* info = v_ctx_get_texture(ctx, ctx->atlas_texture_id);
    VUID_ASSERT(info);

    info->state = V_TEXTURE_UPDATING;
    info->pixels = v_glyph_atlas_get_pixel_buffer(atlas);

    ctx->atlas_last_modified_count = v_glyph_atlas_get_modified_count(atlas);

    v_ctx_request_render();
  }
}

static void v_ctx_render_data_sync(VContext* ctx) {
  ctx->render_data.vertices = ctx->command_queue.vertices.items;
  ctx->render_data.vertex_count = (uint32_t)ctx->command_queue.vertices.size;

  ctx->render_data.indices = ctx->command_queue.indices.items;
  ctx->render_data.index_count = (uint32_t)ctx->command_queue.indices.size;

  ctx->render_data.commands = ctx->command_queue.commands.items;
  ctx->render_data.command_count = (uint32_t)ctx->command_queue.commands.size;

  ctx->render_data.textures = ctx->textures.items;
  ctx->render_data.texture_count = (uint32_t)ctx->textures.size;

  ctx->render_data.vertex_format = &ctx->command_queue.vertex_format;
  ctx->render_data.index_format = ctx->command_queue.index_format;
}

static VTextureInfo* v_ctx_get_texture(VContext* ctx, uint32_t texture_id) {
  if (texture_id > 0) {
    for (uint32_t i = 0; i < ctx->textures.size; i++) {
      VTextureInfo* info = v_array_get_unchecked(&ctx->textures, i);
      if (info->id == texture_id) {
        return info;
      }
    }
  }

  return NULL;
}

static void v_rasterize_text_node(VNode* node) {
  v_foreach_child(node, child) {
    VNodeTag tag = v_node_tag(child);

    if (tag == V_NODE_TEXT && v_node_is_visible(child)) {
      VTextLayout* text_layout = v_node_get_text_layout(child);
      if (text_layout) {
        v_text_module_maybe_rasterize_glyphs(&g_context.text_module,
                                             text_layout);
      }
    } else if (tag == V_NODE_BOX) {
      v_rasterize_text_node(child);
    }
  }
}





typedef struct VVertex {
  float x, y;
  float u, v;
#ifdef VUID_VERTEX_COLOR_U8
  uint8_t r, g, b, a;
#else
  float r, g, b, a;
#endif
} VVertex;

static void v_command_queue_cmd_render(VCommandQueue* self,
                                       uint32_t texture_id,
                                       uint32_t vertex_offset,
                                       uint32_t vertex_count,
                                       uint32_t index_offset,
                                       uint32_t index_count);
static uint32_t v_build_fan_indices(uint32_t* indices,
                                    uint32_t indices_offset,
                                    uint32_t anchor,
                                    uint32_t perim_base,
                                    uint32_t n_perim);
static uint32_t v_build_skirt_indices(uint32_t* indices,
                                      uint32_t indices_offset,
                                      uint32_t core_base,
                                      uint32_t fringe_base,
                                      uint32_t n_perim,
                                      bool inward);

static inline void v_vertex_set(VVertex* self,
                                float x,
                                float y,
                                float u,
                                float v,
                                VColor color) {
  self->x = x;
  self->y = y;
  self->u = u;
  self->v = v;

#ifdef VUID_VERTEX_COLOR_U8
  self->r = color.r;
  self->g = color.g;
  self->b = color.b;
  self->a = color.a;
#else
  self->r = (float)color.r / 255.f;
  self->g = (float)color.g / 255.f;
  self->b = (float)color.b / 255.f;
  self->a = (float)color.a / 255.f;
#endif
}

/* sets a vertex with position and color. uv is 0. */
static inline void v_vertex_set_xyrgba(VVertex* self,
                                       float x,
                                       float y,
                                       VColor color) {
  v_vertex_set(self, x, y, 0.0f, 0.0f, color);
}

/* sets a vertex with position, color and overriden alpha. uv is 0. */
static inline void v_vertex_set_xyrgbx(VVertex* self,
                                       float x,
                                       float y,
                                       VColor color,
                                       uint8_t alpha) {
  v_vertex_set(self, x, y, 0.0f, 0.0f, color);
#ifdef VUID_VERTEX_COLOR_U8
  self->a = alpha;
#else
  self->a = alpha / 255.f;
#endif
}

VUID_PKG bool v_command_queue_init(VCommandQueue* self, VAllocator* allocator) {
  VDataFormat color_format;
#ifdef VUID_VERTEX_COLOR_U8
  color_format = V_DATA_FORMAT_U8;
#else
  color_format = V_DATA_FORMAT_F32;
#endif
  // TODO: make vertex layout configurable
  *self = (VCommandQueue){
      .vertices = v_array_init(allocator, sizeof(VVertex), 1024),
      .indices = v_array_init(allocator, sizeof(uint32_t), 1024),
      .commands = v_array_init(allocator, sizeof(VCommand), 256),
      .vertex_format =
          {
              .xy = {V_DATA_FORMAT_F32, offsetof(VVertex, x)},
              .uv = {V_DATA_FORMAT_F32, offsetof(VVertex, u)},
              .r = {color_format, offsetof(VVertex, r)},
              .g = {color_format, offsetof(VVertex, g)},
              .b = {color_format, offsetof(VVertex, b)},
              .a = {color_format, offsetof(VVertex, a)},
              .size = sizeof(VVertex),
          },
      .index_format = V_DATA_FORMAT_U32,
  };

  return true;
}

VUID_PKG void v_command_queue_drop(VCommandQueue* self) {
  v_array_drop(&self->vertices);
  v_array_drop(&self->indices);
  v_array_drop(&self->commands);
}

VUID_PKG void v_command_queue_clear(VCommandQueue* self) {
  v_array_clear(&self->vertices);
  v_array_clear(&self->indices);
  v_array_clear(&self->commands);
}

VUID_PKG void v_command_queue_cmd_textured_quad(VCommandQueue* self,
                                                const VRect* quad,
                                                uint32_t texture_id,
                                                float u0,
                                                float v0,
                                                float u1,
                                                float v1,
                                                VColor color) {
  v_command_queue_open_batch(self);
  v_command_queue_add_quad(self, quad, u0, v0, u1, v1, color);
  v_command_queue_close_batch(self, texture_id);
}

VUID_PKG void v_command_queue_cmd_fill_rect(VCommandQueue* self,
                                            const VRect* quad,
                                            VColor color) {
  v_command_queue_open_batch(self);
  v_command_queue_add_quad(self, quad, 0, 0, 0, 0, color);
  v_command_queue_close_batch(self, 0);
}

VUID_PKG void v_command_queue_cmd_fill_rounded_rect(VCommandQueue* self,
                                                    const VRect* quad,
                                                    float border_radius,
                                                    VColor color,
                                                    bool skip_outer_fringe) {
  if (border_radius <= 0.0f) {
    v_command_queue_cmd_fill_rect(self, quad, color);
    return;
  }

  float r = border_radius;

  /* adaptive tessellation */
  const uint32_t segments_per_corner =
      (uint32_t)v_clamp_float(r * 0.5f, 4.0f, 16.0f);
  const uint32_t n_perim = 4 * segments_per_corner;

  /* compute counts */
  const uint32_t vertex_count =
      skip_outer_fringe
          ? (1 + n_perim)
          : (1 + n_perim +
             n_perim); /* center + core perimeter + (optional) fringe */

  const uint32_t index_count =
      skip_outer_fringe ? (n_perim * 3 /* fan */)
                        : (n_perim * 3 /* fan */ + n_perim * 6 /* skirt */);

  uint32_t vertex_offset = (uint32_t)self->vertices.size;
  uint32_t index_offset = (uint32_t)self->indices.size;

  if (!v_array_push_n(&self->vertices, vertex_count)) {
    return;
  }

  if (!v_array_push_n(&self->indices, index_count)) {
    self->vertices.size -= vertex_count;
    return;
  }

  VVertex* verts = v_array_get_unchecked(&self->vertices, vertex_offset);
  uint32_t* inds = v_array_get_unchecked(&self->indices, index_offset);

  /* anchor at rectangle center */
  const float cx = quad->x + quad->width * 0.5f;
  const float cy = quad->y + quad->height * 0.5f;

  /* corner centers in order: top-left, top-right, bottom-right, bottom-left */
  const float cx_corner[4] = {
      quad->x + r,
      quad->x + quad->width - r,
      quad->x + quad->width - r,
      quad->x + r,
  };
  const float cy_corner[4] = {
      quad->y + r,
      quad->y + r,
      quad->y + quad->height - r,
      quad->y + quad->height - r,
  };

  /* start angles (radians) for the four corners (CCW) */
  const float start_angle[4] = {
      M_PIf,
      3.0f * M_PIf / 2.0f,
      0.0f,
      M_PIf / 2.0f,
  };

  uint32_t vi = 0; /* next vertex index relative to verts base */

  v_vertex_set_xyrgba(&verts[vi++], cx, cy, color);

  /* build core perimeter vertices (clockwise/ccw ordering is not critical for
   * fan fill) */
  for (uint32_t corner = 0; corner < 4; corner++) {
    for (uint32_t s = 0; s < segments_per_corner; s++) {
      float t = (float)s / (float)segments_per_corner;
      float angle = start_angle[corner] + t * (M_PIf / 2.0f);
      float px = cx_corner[corner] + cosf(angle) * r;
      float py = cy_corner[corner] + sinf(angle) * r;

      v_vertex_set_xyrgba(&verts[vi++], px, py, color);
    }
  }

  const uint32_t perim_base = 1;
  const uint32_t fringe_base = 1 + n_perim;

  /* build fringe vertices (extrude outward by 1.0f px) with alpha=0 */
  if (!skip_outer_fringe) {
    for (uint32_t i = 0; i < n_perim; i++) {
      const float px = verts[perim_base + i].x;
      const float py = verts[perim_base + i].y;
      const float dx = px - cx;
      const float dy = py - cy;
      const float len = sqrtf(dx * dx + dy * dy);
      const float nx = len > 0.0f ? dx / len : 0.0f;
      const float ny = len > 0.0f ? dy / len : 0.0f;

      v_vertex_set_xyrgbx(&verts[fringe_base + i], px + nx * 1.0f,
                          py + ny * 1.0f, color, 0);
    }
  }

  /* triangulate core (fan) */
  uint32_t ii = v_build_fan_indices(inds, 0, 0, perim_base, n_perim);

  /* triangulate fringe skirt (strip) */
  if (!skip_outer_fringe) {
    v_build_skirt_indices(inds, ii, perim_base, fringe_base, n_perim, false);
  }

  /* emit render command (solid color, no texture)
   * indices are relative to the start vertex passed to the renderer */
  v_command_queue_cmd_render(self, 0, vertex_offset, vertex_count, index_offset,
                             index_count);
}

VUID_PKG void v_command_queue_cmd_stroke_rounded_rect(VCommandQueue* self,
                                                      const VRect* quad,
                                                      float border_radius,
                                                      float thickness,
                                                      VColor color) {
  if (thickness <= 0.0f) {
    return;
  }

  /* if thickness covers the whole rect, just fill it */
  if (thickness * 2.0f >= quad->width || thickness * 2.0f >= quad->height) {
    v_command_queue_cmd_fill_rounded_rect(self, quad, border_radius, color,
                                          false);
    return;
  }

  float r = border_radius;

  /* inner radius */
  float r_in = r - thickness;
  if (r_in < 0.0f) {
    r_in = 0.0f;
  }

  /* inner rect */
  VRect inner = {
      quad->x + thickness,
      quad->y + thickness,
      quad->width - 2.0f * thickness,
      quad->height - 2.0f * thickness,
  };

  /* adaptive tessellation */
  const uint32_t segments_per_corner =
      (uint32_t)v_clamp_float(r * 0.5f, 4.0f, 16.0f);
  const uint32_t n_perim = 4 * segments_per_corner;

  const uint32_t vertex_count =
      (n_perim * 4); /* outer core, inner core, outer fringe, inner fringe */
  const uint32_t index_count = (n_perim * 18);

  uint32_t vertex_offset = (uint32_t)self->vertices.size;
  uint32_t index_offset = (uint32_t)self->indices.size;

  if (!v_array_push_n(&self->vertices, vertex_count)) {
    return;
  }

  if (!v_array_push_n(&self->indices, index_count)) {
    self->vertices.size -= vertex_count;
    return;
  }

  VVertex* verts = v_array_get_unchecked(&self->vertices, vertex_offset);
  uint32_t* inds = v_array_get_unchecked(&self->indices, index_offset);

  /* rectangle center for simple normal estimation (match fill strategy) */
  const float cx = quad->x + quad->width * 0.5f;
  const float cy = quad->y + quad->height * 0.5f;

  /* outer corner centers */
  const float ocx[4] = {
      quad->x + r,
      quad->x + quad->width - r,
      quad->x + quad->width - r,
      quad->x + r,
  };
  const float ocy[4] = {
      quad->y + r,
      quad->y + r,
      quad->y + quad->height - r,
      quad->y + quad->height - r,
  };

  /* inner corner centers */
  const float icx[4] = {
      inner.x + r_in,
      inner.x + inner.width - r_in,
      inner.x + inner.width - r_in,
      inner.x + r_in,
  };
  const float icy[4] = {
      inner.y + r_in,
      inner.y + r_in,
      inner.y + inner.height - r_in,
      inner.y + inner.height - r_in,
  };

  const float start_angle[4] = {
      M_PIf,
      3.0f * M_PIf / 2.0f,
      0.0f,
      M_PIf / 2.0f,
  };

  /* fill outer core vertices */
  uint32_t vi = 0;
  for (uint32_t corner = 0; corner < 4; corner++) {
    for (uint32_t s = 0; s < segments_per_corner; s++) {
      const float t = (float)s / (float)segments_per_corner;
      const float angle = start_angle[corner] + t * (M_PIf / 2.0f);
      const float px = ocx[corner] + cosf(angle) * r;
      const float py = ocy[corner] + sinf(angle) * r;

      v_vertex_set_xyrgba(&verts[vi++], px, py, color);
    }
  }

  const uint32_t outer_base = 0;
  const uint32_t inner_base = outer_base + n_perim;
  const uint32_t outer_fringe_base = inner_base + n_perim;
  const uint32_t inner_fringe_base = outer_fringe_base + n_perim;

  /* fill inner core vertices */
  for (uint32_t corner = 0; corner < 4; corner++) {
    for (uint32_t s = 0; s < segments_per_corner; s++) {
      const float t = (float)s / (float)segments_per_corner;
      const float angle = start_angle[corner] + t * (M_PIf / 2.0f);
      const float px = icx[corner] + cosf(angle) * r_in;
      const float py = icy[corner] + sinf(angle) * r_in;

      v_vertex_set_xyrgba(&verts[vi++], px, py, color);
    }
  }

  /* outer fringe (extrude outward) */
  for (uint32_t i = 0; i < n_perim; i++) {
    const float px = verts[outer_base + i].x;
    const float py = verts[outer_base + i].y;
    const float dx = px - cx;
    const float dy = py - cy;
    const float len = sqrtf(dx * dx + dy * dy);
    const float nx = len > 0.0f ? dx / len : 0.0f;
    const float ny = len > 0.0f ? dy / len : 0.0f;

    v_vertex_set_xyrgbx(&verts[vi++], px + nx * 1.0f, py + ny * 1.0f, color, 0);
  }

  /* inner fringe (extrude inward) */
  for (uint32_t i = 0; i < n_perim; i++) {
    const float px = verts[inner_base + i].x;
    const float py = verts[inner_base + i].y;
    const float dx = px - cx;
    const float dy = py - cy;
    const float len = sqrtf(dx * dx + dy * dy);
    const float nx = len > 0.0f ? dx / len : 0.0f;
    const float ny = len > 0.0f ? dy / len : 0.0f;

    v_vertex_set_xyrgbx(&verts[vi++], px - nx * 1.0f, py - ny * 1.0f, color, 0);
  }

  /* build indices */
  uint32_t ii = 0;

  /* ring between outer and inner cores */
  ii = v_build_skirt_indices(inds, ii, outer_base, inner_base, n_perim, false);

  /* outer skirt */
  ii = v_build_skirt_indices(inds, ii, outer_base, outer_fringe_base, n_perim,
                             false);

  /* inner skirt (stitched inward) */
  v_build_skirt_indices(inds, ii, inner_base, inner_fringe_base, n_perim, true);

  v_command_queue_cmd_render(self, 0, vertex_offset, vertex_count, index_offset,
                             index_count);
}

VUID_PKG void v_command_queue_cmd_stroke_rect(VCommandQueue* self,
                                              const VRect* quad,
                                              const struct VInsets* border,
                                              VColor color) {
  VRect rect;

  v_command_queue_open_batch(self);

  if (border->top > 0) {
    rect.x = quad->x;
    rect.y = quad->y;
    rect.width = quad->width;
    rect.height = border->top;
    v_command_queue_add_quad(self, &rect, 0, 0, 0, 0, color);
  }

  if (border->bottom > 0) {
    rect.x = quad->x;
    rect.y = quad->y + quad->height - border->bottom;
    rect.width = quad->width;
    rect.height = border->bottom;
    v_command_queue_add_quad(self, &rect, 0, 0, 0, 0, color);
  }

  if (border->left > 0) {
    rect.x = quad->x;
    rect.y = quad->y + border->top;
    rect.width = border->left;
    rect.height = quad->height - border->top - border->bottom;
    v_command_queue_add_quad(self, &rect, 0, 0, 0, 0, color);
  }

  if (border->right > 0) {
    rect.x = quad->x + quad->width - border->right;
    rect.y = quad->y + border->top;
    rect.width = border->right;
    rect.height = quad->height - border->top - border->bottom;
    v_command_queue_add_quad(self, &rect, 0, 0, 0, 0, color);
  }

  v_command_queue_close_batch(self, 0);
}

VUID_PKG bool v_command_queue_add_quad(VCommandQueue* self,
                                       const VRect* quad,
                                       float u0,
                                       float v0,
                                       float u1,
                                       float v1,
                                       VColor color) {
  static const uint32_t QUAD[] = {0, 1, 2, 0, 2, 3};
  const uint32_t vertex_offset = (uint32_t)self->vertices.size;
  const uint32_t index_offset = (uint32_t)self->indices.size;

  if (!v_array_push_n(&self->vertices, 4)) {
    return false;
  }

  if (!v_array_push_n(&self->indices, 6)) {
    self->vertices.size -= 4;
    return false;
  }

  VVertex* vertex = v_array_get_unchecked(&self->vertices, vertex_offset);

  v_vertex_set(&vertex[0], quad->x, quad->y, u0, v0, color);
  v_vertex_set(&vertex[1], quad->x + quad->width, quad->y, u1, v0, color);
  v_vertex_set(&vertex[2], quad->x + quad->width, quad->y + quad->height, u1,
               v1, color);
  v_vertex_set(&vertex[3], quad->x, quad->y + quad->height, u0, v1, color);

  uint32_t* indices = v_array_get_unchecked(&self->indices, index_offset);
  const uint32_t vertex_index_base = self->cmd_vertex_index_base;

  for (size_t i = 0; i < VUID_ARRAY_LEN(QUAD); i++) {
    indices[i] = vertex_index_base + QUAD[i];
  }

  self->cmd_vertex_index_base += 4;
  self->batch_quad_count++;

  return true;
}

VUID_PKG void v_command_queue_open_batch(VCommandQueue* self) {
  self->cmd_vertex_offset = (uint32_t)self->vertices.size;
  self->cmd_index_offset = (uint32_t)self->indices.size;
  self->cmd_vertex_index_base = 0;
  self->batch_quad_count = 0;
}

VUID_PKG void v_command_queue_close_batch(VCommandQueue* self,
                                          uint32_t texture_id) {
  uint32_t batch_quad_count = self->batch_quad_count;

  if (batch_quad_count > 0) {
    VCommand command = {
        .u.render.texture_id = texture_id,
        .u.render.vertex_offset = self->cmd_vertex_offset,
        .u.render.index_offset = self->cmd_index_offset,
        .u.render.vertex_count = batch_quad_count * 4,
        .u.render.index_count = batch_quad_count * 6,
        .type = V_COMMAND_RENDER,
    };

    v_array_push(&self->commands, &command);
  }
}

VUID_PKG void v_command_queue_cmd_set_clip(VCommandQueue* self,
                                           int x,
                                           int y,
                                           int width,
                                           int height) {
  VCommand cmd = {
      .type = V_COMMAND_SET_CLIP,
      .u.set_clip = {.x = x, .y = y, .w = width, .h = height},
  };

  v_array_push(&self->commands, &cmd);
}

VUID_PKG void v_command_queue_cmd_reset_clip(VCommandQueue* self) {
  VCommand cmd = {
      .type = V_COMMAND_SET_CLIP,
      .u.set_clip.clear = true,
  };

  v_array_push(&self->commands, &cmd);
}

static void v_command_queue_cmd_render(VCommandQueue* self,
                                       uint32_t texture_id,
                                       uint32_t vertex_offset,
                                       uint32_t vertex_count,
                                       uint32_t index_offset,
                                       uint32_t index_count) {
  VCommand command = {
      .u.render.texture_id = texture_id,
      .u.render.vertex_offset = vertex_offset,
      .u.render.vertex_count = vertex_count,
      .u.render.index_offset = index_offset,
      .u.render.index_count = index_count,
      .type = V_COMMAND_RENDER,
  };

  v_array_push(&self->commands, &command);
}

/* Build indices for a triangle fan vertex set. */
static uint32_t v_build_fan_indices(uint32_t* indices,
                                    uint32_t indices_offset,
                                    uint32_t anchor,
                                    uint32_t perim_base,
                                    uint32_t n_perim) {
  uint32_t ii = indices_offset;

  for (uint32_t i = 0; i < n_perim; i++) {
    uint32_t next = (i + 1) % n_perim;
    indices[ii++] = anchor;
    indices[ii++] = (perim_base + i);
    indices[ii++] = (perim_base + next);
  }

  return ii;
}

/*
 * Build indices for a rounded rectangle skirt. The skirt can go outward for
 * rounded rect fill and stroke shapes. The skirt can go inward for rounded rect
 * stroke shapes.
 */
static uint32_t v_build_skirt_indices(uint32_t* indices,
                                      uint32_t indices_offset,
                                      uint32_t core_base,
                                      uint32_t fringe_base,
                                      uint32_t n_perim,
                                      bool inward) {
  uint32_t ii = indices_offset;

  for (uint32_t i = 0; i < n_perim; i++) {
    uint32_t next = (i + 1) % n_perim;
    uint32_t c0 = (core_base + i);
    uint32_t c1 = (core_base + next);
    uint32_t f0 = (fringe_base + i);
    uint32_t f1 = (fringe_base + next);

    if (!inward) {
      indices[ii++] = c0;
      indices[ii++] = f0;
      indices[ii++] = f1;

      indices[ii++] = c0;
      indices[ii++] = f1;
      indices[ii++] = c1;
    } else {
      indices[ii++] = c0;
      indices[ii++] = f1;
      indices[ii++] = f0;

      indices[ii++] = c0;
      indices[ii++] = c1;
      indices[ii++] = f1;
    }
  }

  return ii;
}




VUID_PKG bool v_string_pool_init(VStringPool* self,
                                 VAllocator* allocator,
                                 uint32_t initial_capacity) {
  self->strings =
      v_static_string_set_init_with_capacity(allocator, initial_capacity);
  return true;
}

VUID_PKG void v_string_pool_drop(VStringPool* self) {
  v_static_string_set_drop(&self->strings);
  *self = (VStringPool){0};
}

VUID_PKG bool v_string_pool_intern(VStringPool* self,
                                   const char* str,
                                   VStaticString* out) {
  if (v_cstr_is_empty(str)) {
    return false;
  }

  VStaticStringSetKey key = {.str = str, .str_hash = v_fnv1_hash(str)};
  VStaticString* existing = v_static_string_set_get(&self->strings, key);

  if (existing) {
    *out = *existing;
    return true;
  }

  // TODO: allocator should be passed in and also accessible to drop
  VAllocator* allocator = v_ctx_allocator();
  uint32_t size = (uint32_t)strlen(str);

  // TODO: these should probably be pooled allocs
  char* interned_str = v_alloc_raw(allocator, size + 1);

  if (!interned_str) {
    return false;
  }

  memcpy(interned_str, str, size);
  interned_str[size] = '\0';

  VStaticString value = {
      .str = interned_str,
      .hash = key.str_hash,
      .size = size,
  };
  VStaticStringSetResult result =
      v_static_string_set_put(&self->strings, value);

  if (result.inserted) {
    *out = *result.ref;
  }

  return result.inserted;
}

//
// style class hash map implementation
//

static uint32_t VStaticStringSetKey_get_hash(const VStaticStringSetKey* key) {
  return key->str_hash;
}

static uint32_t VStaticString_get_hash(const VStaticString* value) {
  return value->hash;
}

static VStaticStringSetKey VStaticStringSetKey_from_value(
    const VStaticString* value) {
  return (VStaticStringSetKey){.str = value->str, .str_hash = value->hash};
}

static bool VStaticString_eq(const VStaticStringSetKey* key,
                             const VStaticString* value) {
  return strcmp(value->str, key->str) == 0;
}

static void VStaticString_drop(VStaticString* value) {
  VAllocator* allocator = v_ctx_allocator();
  v_free(allocator, value->str, value->size + 1);
}

VUID_HMAP_IMPL(VStaticStringSet,
               VStaticStringSetKey,
               VStaticString,
               v_static_string_set)




VUID_PKG VWeakRef* v_weak_ref_new(void* ref) {
  VWeakRef* weak_ref = v_ctx_new(VWeakRef);
  VUID_ASSERT(weak_ref);

  if (weak_ref) {
    v_ctx_object_inc(V_OBJECT_TYPE_WEAK_NODE_REF);
    weak_ref->ref_count = 1;
    weak_ref->ref = ref;
  }

  return (VWeakRef*)weak_ref;
}

VUID_PKG void* v_weak_ref_get(const VWeakRef* weak_ref) {
  return weak_ref ? weak_ref->ref : NULL;
}

VUID_PKG VWeakRef* v_weak_ref_acquire(VWeakRef* weak_ref) {
  if (weak_ref->ref_count < UINT32_MAX) {
    weak_ref->ref_count++;
  } else {
    VUID_ASSERT(false && "VWeakRef ref count overflow");
  }

  return weak_ref;
}

VUID_PKG void v_weak_ref_release(VWeakRef* weak_ref) {
  if (weak_ref->ref_count > 1) {
    weak_ref->ref_count--;
  } else if (weak_ref->ref_count == 1) {
    v_ctx_delete(weak_ref, VWeakRef);
    v_ctx_object_dec(V_OBJECT_TYPE_WEAK_NODE_REF);
  } else {
    VUID_ASSERT(false && "VWeakRef ref count underflow");
  }
}



static VImage* v_image_new(VImageStore* self, VString* str);
static VImage* v_image_new_mem(VImageStore* self,
                               VString* str,
                               VBuffer* file_buffer);
static VImage* v_image_ref(VImage* image);
static void v_image_unref(VImage* image);
static void v_image_clear(VImage* image);
static bool v_null_image_loader(VImageLoaderOp op,
                                VImageBuffer* buffer,
                                const void* file_data,
                                size_t file_size);

VUID_PKG bool v_image_store_init(VImageStore* self,
                                 VAllocator* allocator,
                                 VImageLoaderFn loader) {
  self->allocator = allocator;
  self->images = v_image_hmap_init_with_capacity(allocator, 8);
  self->loader = loader ? loader : v_null_image_loader;
  return true;
}

VUID_PKG void v_image_store_drop(VImageStore* self) {
  // TODO: audit image refs?
  v_image_hmap_drop(&self->images);
}

VUID_PKG VImage* v_image_store_acquire(VImageStore* self, VString* src) {
  if (v_string_is_empty(src)) {
    return NULL;
  }

  VImageHashMapValue* value = v_image_store_get(self, v_string_cstr(src));

  if (value) {
    return v_image_ref(value->image);
  }

  VImage* image = v_image_new(self, src);

  if (!image) {
    return NULL;
  }

  VImageHashMapResult result =
      v_image_hmap_put(&self->images, (VImageHashMapValue){.image = image});

  return result.inserted ? v_image_ref(result.ref->image) : NULL;
}

VUID_PKG bool v_image_store_persist(VImageStore* self, const char* src) {
  if (v_cstr_is_empty(src)) {
    return false;
  }

  VImageHashMapValue* value = v_image_store_get(self, src);

  if (value) {
    value->persist = true;
    return true;
  }

  VString* src_as_string = v_string_from(self->allocator, src);
  VImage* image = v_image_new(self, src_as_string);

  v_string_unref(src_as_string);

  if (!image) {
    return false;
  }

  VImageHashMapResult result =
      v_image_hmap_put(&self->images, (VImageHashMapValue){.image = image});

  if (result.inserted) {
    result.ref->persist = true;
    return true;
  }

  return false;
}

// TODO: duplicate code
VUID_PKG bool v_image_store_persist_mem(VImageStore* self,
                                        const char* src,
                                        VBuffer* buffer) {
  if (v_cstr_is_empty(src) || v_buffer_is_empty(buffer)) {
    v_buffer_drop(buffer);
    return false;
  }

  VImageHashMapValue* value = v_image_store_get(self, src);

  if (value) {
    value->persist = true;
    v_buffer_drop(buffer);
    return true;
  }

  VString* src_as_string = v_string_from(self->allocator, src);
  VImage* image = v_image_new_mem(self, src_as_string, buffer);

  v_string_unref(src_as_string);

  if (!image) {
    return false;
  }

  VImageHashMapResult result =
      v_image_hmap_put(&self->images, (VImageHashMapValue){.image = image});

  if (result.inserted) {
    result.ref->persist = true;
  }

  return result.inserted;
}

VUID_PKG void v_image_store_remove(VImageStore* self, const char* src) {
  if (v_cstr_is_empty(src)) {
    return;
  }

  VImageHashMapValue* value = v_image_store_get(self, src);

  if (value) {
    value->persist = false;
    v_image_ref(value->image);
    v_image_hmap_remove_by_value(&self->images, value);
    // nodes might be holding this VImage. clear the contents, but leave src
    v_image_clear(value->image);
    v_image_unref(value->image);
  }
}

VUID_PKG void v_image_store_release(VImageStore* self, VImage* image) {
  if (image == NULL) {
    return;
  }

  VImageHashMapValue* value =
      v_image_store_get(self, v_string_cstr(image->src));

  if (value == NULL || value->image != image) {
    v_image_unref(image);
  } else {
    VUID_ASSERT(value->image == image);
    VUID_ASSERT(value->image->ref_count >= 2);

    if (image->ref_count == 2) {
      if (value->persist) {
        value->persist = false;
      } else {
        v_image_hmap_remove(
            &self->images,
            (VImageHashMapKey){.src = v_string_cstr(image->src)});
      }
      v_image_unref(image);
    } else if (image->ref_count > 2) {
      v_image_unref(image);
    } else {
      VUID_ASSERT(false && "VImage ref count underflow");
    }
  }
}

VUID_PKG VImageHashMapValue* v_image_store_get(VImageStore* self,
                                               const char* src) {
  return v_image_hmap_get(&self->images, (VImageHashMapKey){.src = src});
}

VUID_PKG VSize v_image_get_size(const VImage* image) {
  return image ? (VSize){(float)image->width, (float)image->height}
               : (VSize){0, 0};
}

VUID_PKG uint32_t v_image_get_texture_id(const VImage* image) {
  return image ? image->texture_id : 0;
}

static VImage* v_image_new(VImageStore* self, VString* src) {
  if (v_string_is_empty(src)) {
    return NULL;
  }

  size_t file_data_size = 0;
  uint8_t* file_data =
      v_file_read(self->allocator, v_string_cstr(src), &file_data_size);

  if (!file_data) {
    return NULL;
  }

  VBuffer file_buffer = {
      .allocator = self->allocator,
      .data = file_data,
      .size = file_data_size,
      .source = file_data,
  };

  return v_image_new_mem(self, src, &file_buffer);
}

static VImage* v_image_new_mem(VImageStore* self,
                               VString* src,
                               VBuffer* file_buffer) {
  if (!src) {
    return NULL;
  }

  VImage* image = v_alloc_zero(self->allocator, sizeof(VImage));

  if (!image) {
    v_buffer_drop(file_buffer);
    return NULL;
  }

  image->ref_count = 1;
  v_ctx_object_inc(V_OBJECT_TYPE_IMAGE);
  image->file_buffer = *file_buffer;

  if (!self->loader(V_IMAGE_LOADER_OP_LOAD, &image->buffer, file_buffer->data,
                    file_buffer->size)) {
    v_image_unref(image);
    return NULL;
  }

  if (image->buffer.width > VUID_FLOAT_MAX_INT ||
      image->buffer.height > VUID_FLOAT_MAX_INT) {
    v_image_unref(image);
    return NULL;
  }

  uint32_t texture_id =
      v_ctx_create_texture(V_PIXEL_FORMAT_RGBA8, &image->buffer);

  if (!texture_id) {
    v_image_unref(image);
    return NULL;
  }

  image->texture_id = texture_id;
  image->src = src;
  image->src_hash = v_fnv1_hash(v_string_cstr(image->src));
  image->width = image->buffer.width;
  image->height = image->buffer.height;

  v_string_ref(image->src);

  return image;
}

static VImage* v_image_ref(VImage* image) {
  if (image) {
    if (image->ref_count < UINT32_MAX) {
      image->ref_count++;
    } else {
      VUID_ASSERT(false && "VImage ref count overflow");
    }
  }
  return image;
}

static void v_image_unref(VImage* image) {
  if (image) {
    const uint32_t ref_count = image->ref_count;

    if (ref_count > 1) {
      image->ref_count--;
    } else if (ref_count == 1) {
      VImageStore* image_store = v_ctx_image_store();

      v_image_clear(image);
      v_string_unref(image->src);
      v_free(image_store->allocator, image, sizeof(VImage));
      v_ctx_object_dec(V_OBJECT_TYPE_IMAGE);
    } else {
      VUID_ASSERT(false && "VImage ref count underflow");
    }
  }
}

static void v_image_clear(VImage* image) {
  VImageStore* image_store = v_ctx_image_store();

  image_store->loader(V_IMAGE_LOADER_OP_FREE, &image->buffer, NULL, 0);
  image->buffer = (VImageBuffer){0};

  if (image->texture_id) {
    v_ctx_destroy_texture(image->texture_id);
    image->texture_id = 0;
  }

  image->width = image->height = 0;
  v_buffer_drop(&image->file_buffer);
}

static bool v_null_image_loader(VImageLoaderOp op,
                                VImageBuffer* buffer,
                                const void* file_data,
                                size_t file_size) {
  UNUSED(op, buffer, file_data, file_size);
  return false;
}

//
// image store hash map implementation
//

static uint32_t VImageHashMapValue_get_hash(const VImageHashMapValue* value) {
  return value->image->src_hash;
}

static uint32_t VImageHashMapKey_get_hash(const VImageHashMapKey* key) {
  return v_fnv1_hash(key->src);
}

static VImageHashMapKey VImageHashMapKey_from_value(
    const VImageHashMapValue* value) {
  return (VImageHashMapKey){
      .src = v_string_cstr(value->image->src),
  };
}

static bool VImageHashMapValue_eq(const VImageHashMapKey* key,
                                  const VImageHashMapValue* value) {
  return v_string_eq_cstr(value->image->src, key->src);
}

static void VImageHashMapValue_drop(VImageHashMapValue* value) {
  v_image_unref(value->image);
}

VUID_HMAP_IMPL(VImageHashMap,
               VImageHashMapKey,
               VImageHashMapValue,
               v_image_hmap)



static bool is_whitespace_at_cluster(const char* utf8, uint32_t cluster) {
  unsigned char c = (unsigned char)utf8[cluster];
  return c == 0x20u || c == 0x09u || c == 0x0Au || c == 0x0Du;
}

/* Sum of x_advance for glyphs [start, start+count). */
static float measure_line_width(const VShapedGlyph* shaped,
                                uint32_t start,
                                uint32_t count) {
  float w = 0.0f;
  for (uint32_t i = 0; i < count; i++) {
    w += shaped[start + i].x_advance;
  }
  return w;
}

static bool emit_line(VArray* scratch_lines,
                      uint32_t glyph_start,
                      uint32_t glyph_count) {
  VTextLine line = {
      .glyph_start = glyph_start,
      .glyph_count = glyph_count,
      .width = 0.0f,
      .baseline_y = 0.0f,
  };
  return v_array_push(scratch_lines, &line);
}

VTextLayout* v_text_layout_build(VAllocator* allocator,
                                 const VShapedGlyph* shaped,
                                 uint32_t shaped_count,
                                 const char* utf8,
                                 const VFontMetrics* font_metrics,
                                 VTextWrap wrap,
                                 VAlignX talign,
                                 float wrap_width,
                                 VArray* scratch_glyphs,
                                 VArray* scratch_lines) {
  v_array_clear(scratch_glyphs);
  v_array_clear(scratch_lines);

  // ---- Line-breaking pass ----
  if (shaped_count == 0 || wrap == V_TEXT_WRAP_NO_WRAP) {
    if (!emit_line(scratch_lines, 0, shaped_count)) {
      return NULL;
    }
  } else {
    // Greedy line break: track the last whitespace glyph as a break point.
    uint32_t line_start = 0;
    float line_x = 0.0f;
    int32_t break_at = -1;      /* shaped[] index of last whitespace glyph */
    float x_after_break = 0.0f; /* line_x after including the break glyph */

    for (uint32_t i = 0; i < shaped_count; i++) {
      bool is_space = is_whitespace_at_cluster(utf8, shaped[i].cluster);
      if (is_space) {
        break_at = (int32_t)i;
      }
      line_x += shaped[i].x_advance;
      if (is_space && break_at == (int32_t)i) {
        x_after_break = line_x;
      }

      if (line_x > wrap_width && i > line_start) {
        if (break_at >= (int32_t)line_start) {
          // Soft break: line = [line_start, break_at), next starts after.
          if (!emit_line(scratch_lines, line_start,
                         (uint32_t)break_at - line_start)) {
            return NULL;
          }
          line_start = (uint32_t)break_at + 1;
          line_x = line_x - x_after_break;
          break_at = -1;
          x_after_break = 0.0f;
        } else {
          // Hard break before the current glyph (no whitespace available).
          if (!emit_line(scratch_lines, line_start, i - line_start)) {
            return NULL;
          }
          line_start = i;
          line_x = shaped[i].x_advance;
          break_at = -1;
          x_after_break = 0.0f;
        }
      }
    }
    // Remaining glyphs form the last line.
    if (!emit_line(scratch_lines, line_start, shaped_count - line_start)) {
      return NULL;
    }
  }

  // ---- Positioning pass ----
  float baseline_y = font_metrics->ascent;
  float max_line_width = 0.0f;

  for (size_t li = 0; li < scratch_lines->size; li++) {
    VTextLine* line = v_array_get_unchecked(scratch_lines, li);
    uint32_t shaped_start = line->glyph_start;
    float line_width =
        measure_line_width(shaped, shaped_start, line->glyph_count);
    line->width = line_width;
    line->baseline_y = baseline_y;

    float box_width = (wrap_width > 0.0f) ? wrap_width : line_width;
    float cursor_x = 0.0f;
    if (talign == V_ALIGN_X_CENTER) {
      cursor_x = (box_width - line_width) * 0.5f;
    } else if (talign == V_ALIGN_X_RIGHT) {
      cursor_x = box_width - line_width;
    }

    // Redirect glyph_start to the upcoming offset in scratch_glyphs.
    line->glyph_start = (uint32_t)scratch_glyphs->size;

    for (uint32_t gi = 0; gi < line->glyph_count; gi++) {
      const VShapedGlyph* sg = &shaped[shaped_start + gi];
      VTextGlyph g = {
          .glyph_id = sg->glyph_id,
          .x = cursor_x + sg->x_offset,
          .y = baseline_y + sg->y_offset,
      };
      if (!v_array_push(scratch_glyphs, &g)) {
        return NULL;
      }
      cursor_x += sg->x_advance;
    }

    if (line_width > max_line_width)
      max_line_width = line_width;
    baseline_y += font_metrics->line_height;
  }

  // ---- Allocate the layout as a single contiguous block ----
  uint32_t glyph_count = (uint32_t)scratch_glyphs->size;
  uint32_t line_count = (uint32_t)scratch_lines->size;
  size_t glyphs_bytes = glyph_count * sizeof(VTextGlyph);
  size_t lines_bytes = line_count * sizeof(VTextLine);
  size_t total = sizeof(VTextLayout) + glyphs_bytes + lines_bytes;

  VTextLayout* layout = v_alloc_zero(allocator, total);
  if (!layout)
    return NULL;

  layout->rasterized_at = 0;
  layout->allocator = allocator;
  layout->glyph_count = glyph_count;
  layout->line_count = line_count;
  layout->width = max_line_width;
  layout->height = (float)line_count * font_metrics->line_height;
  layout->ascent = font_metrics->ascent;
  layout->glyphs = (VTextGlyph*)((uint8_t*)layout + sizeof(VTextLayout));
  layout->lines = (VTextLine*)((uint8_t*)layout->glyphs + glyphs_bytes);
  memcpy(layout->glyphs, scratch_glyphs->items, glyphs_bytes);
  memcpy(layout->lines, scratch_lines->items, lines_bytes);
  return layout;
}

VUID_PKG VSize v_text_layout_get_size(const VTextLayout* text_layout) {
  return text_layout ? (VSize){text_layout->width, text_layout->height}
                     : (VSize){0, 0};
}

VUID_PKG void v_text_layout_render(const VTextLayout* layout,
                                   VCommandQueue* cmdq,
                                   float x,
                                   float y,
                                   VColor color,
                                   VGlyphAtlas* atlas) {
  if (!layout || layout->glyph_count == 0) {
    return;
  }

  // TODO: many things...
  //       - should we skip glyphs that are completely outside the
  //       current clip?
  //       - x,y are subpixel coords; should we round them or let the
  //       GPU handle it?
  //       - should text layout cache these quads so we don't have to
  //       regenerate them every frame?
  //       - yes, this is inefficient
  const float atlas_w = (float)atlas->width;
  const float atlas_h = (float)atlas->height;

  v_command_queue_open_batch(cmdq);

  for (uint32_t gi = 0; gi < layout->glyph_count; gi++) {
    const VTextGlyph* g = &layout->glyphs[gi];
    const VAtlasCacheKey key = {
        .glyph_id = g->glyph_id,
        .packed_fp = ((uint32_t)layout->font_id << 16) | layout->pixel_size,
    };
    const VAtlasCacheEntry* entry = v_atlas_cache_map_get(&atlas->cache, key);
    if (!entry || entry->atlas_w == 0 || entry->atlas_h == 0) {
      continue;
    }

    VRect quad = {
        .x = x + g->x + entry->bearing_x,
        .y = y + g->y - entry->bearing_y,
        .width = (float)entry->atlas_w,
        .height = (float)entry->atlas_h,
    };

    v_command_queue_add_quad(
        cmdq,
        &quad,                                               // glyph_box
        (float)entry->atlas_x / atlas_w,                     // u0
        (float)entry->atlas_y / atlas_h,                     // v0
        (float)(entry->atlas_x + entry->atlas_w) / atlas_w,  // u1
        (float)(entry->atlas_y + entry->atlas_h) / atlas_h,  // v1
        color                                                // color
    );
  }

  v_command_queue_close_batch(cmdq, VUID_ATLAS_TEXTURE_ID);
}



#if defined(VUID_TEXT_ENGINE_HB)

static inline hb_bool_t v_hb_font_has_color(hb_face_t* face) {
  return hb_ot_color_has_paint(face) || hb_ot_color_has_layers(face) ||
         hb_ot_color_has_png(face);
}

VUID_PKG bool v_te_init(VTextEngine* self) {
  self->hb_buffer = hb_buffer_create();
  if (!self->hb_buffer) {
    v_te_drop(self);
    return false;
  }

  self->hb_draw = hb_raster_draw_create_or_fail();
  if (!self->hb_draw) {
    v_te_drop(self);
    return false;
  }

  return true;
}

VUID_PKG void v_te_drop(VTextEngine* self) {
  hb_buffer_destroy(self->hb_buffer);
  hb_raster_draw_destroy(self->hb_draw);

  *self = (VTextEngine){0};
}

VUID_PKG bool v_te_init_font(VTextEngine* self,
                             VTextEngineFont* font,
                             const uint8_t* bytes,
                             size_t size) {
  UNUSED(self);
  hb_blob_t* hb_blob = hb_blob_create((const char*)bytes, (unsigned)size,
                                      HB_MEMORY_MODE_READONLY, NULL, NULL);

  font->hb_face = hb_blob ? hb_face_create_or_fail(hb_blob, 0) : NULL;
  font->hb_font = font->hb_face ? hb_font_create(font->hb_face) : NULL;

  hb_blob_destroy(hb_blob);

  // only support single color fonts until atlas supports ARGB

  if (!font->hb_face || !font->hb_font || v_hb_font_has_color(font->hb_face)) {
    v_te_drop_font(self, font);
    return false;
  }

  return true;
}

VUID_PKG void v_te_drop_font(VTextEngine* self, VTextEngineFont* font) {
  UNUSED(self);

  hb_face_destroy(font->hb_face);
  hb_font_destroy(font->hb_font);

  *font = (VTextEngineFont){0};
}

VUID_PKG bool v_te_set_font_pixel_size(VTextEngine* self,
                                       VTextEngineFont* font,
                                       uint16_t pixel_size) {
  UNUSED(self);
  const unsigned int px = (unsigned int)pixel_size;
  const int scale = (int)px * (int)VUID_HB_FONT_SCALE;

  if (font->hb_font) {
    hb_font_set_scale(font->hb_font, scale, scale);
  }

  return true;
}

VUID_PKG bool v_te_get_font_metrics(VTextEngine* self,
                                    VTextEngineFont* font,
                                    float* ascent,
                                    float* descent,
                                    float* line_height) {
  UNUSED(self);
  const hb_position_t scale = (hb_position_t)VUID_HB_FONT_SCALE;
  hb_font_extents_t extents = {0};

  hb_font_get_extents_for_direction(font->hb_font, HB_DIRECTION_LTR, &extents);

  *ascent = (float)(extents.ascender / scale);
  *descent = (float)(extents.descender / scale);
  *line_height =
      (float)((extents.line_gap + extents.ascender - extents.descender) /
              scale);

  return true;
}

VUID_PKG bool v_te_get_glyph_bitmap_desc(VTextEngine* self,
                                         VTextEngineFont* font,
                                         uint32_t glyph_id,
                                         VGlyphBitmapDesc* bitmap_desc) {
  hb_raster_draw_t* hb_draw = self->hb_draw;

  hb_raster_draw_reset(hb_draw);
  hb_raster_draw_set_scale_factor(hb_draw, (float)VUID_HB_FONT_SCALE,
                                  (float)VUID_HB_FONT_SCALE);
  hb_raster_draw_set_transform(hb_draw, 1, 0, 0, 1, 0, 0);

  /* Try to compute pixel extents from glyph extents without rendering. */
  hb_glyph_extents_t gext;
  hb_raster_extents_t extents = {0};
  bool have_pixel_extents = false;

  if (hb_font_get_glyph_extents(font->hb_font, glyph_id, &gext) &&
      hb_raster_draw_set_glyph_extents(hb_draw, &gext)) {
    have_pixel_extents = hb_raster_draw_get_extents(hb_draw, &extents);
  }

  /* Fallback to rendering the glyph to obtain pixel extents. */
  if (!have_pixel_extents) {
    hb_raster_draw_glyph(hb_draw, font->hb_font, glyph_id);

    hb_raster_image_t* img = hb_raster_draw_render(hb_draw);
    if (!img) {
      return false;
    }

    if (hb_raster_image_get_format(img) != HB_RASTER_FORMAT_A8) {
      hb_raster_draw_recycle_image(hb_draw, img);
      return false;
    }

    hb_raster_image_get_extents(img, &extents);
    hb_raster_draw_recycle_image(hb_draw, img);
  }

  *bitmap_desc = (VGlyphBitmapDesc){
      .glyph_id = glyph_id,
      .width = extents.width,
      .height = extents.height,
      .bearing_x = (float)extents.x_origin,
      /* convert to vuid's coordinate system: top-left origin */
      .bearing_y = (float)(extents.y_origin + (int)extents.height),
  };

  return true;
}

VUID_PKG bool v_te_rasterize_glyph(VTextEngine* self,
                                   VTextEngineFont* font,
                                   VGlyphBitmapDesc* bitmap_desc,
                                   uint8_t* target,
                                   uint16_t target_width,
                                   uint16_t target_padding,
                                   uint16_t ax,
                                   uint16_t ay) {
  hb_raster_draw_t* hb_draw = self->hb_draw;

  hb_raster_draw_reset(hb_draw);
  hb_raster_draw_set_scale_factor(hb_draw, (float)VUID_HB_FONT_SCALE,
                                  (float)VUID_HB_FONT_SCALE);
  hb_raster_draw_set_transform(hb_draw, 1, 0, 0, 1, 0, 0);

  /* If available, set glyph extents to get deterministic bounds. */
  hb_glyph_extents_t gext;
  if (hb_font_get_glyph_extents(font->hb_font, bitmap_desc->glyph_id, &gext)) {
    hb_raster_draw_set_glyph_extents(hb_draw, &gext);
  }

  hb_raster_draw_glyph(hb_draw, font->hb_font, bitmap_desc->glyph_id);

  hb_raster_image_t* img = hb_raster_draw_render(hb_draw);

  if (!img) {
    return false;
  }

  const uint8_t* buffer = hb_raster_image_get_buffer(img);
  hb_raster_extents_t extents = {0};

  hb_raster_image_get_extents(img, &extents);

  const uint16_t bmp_w = (uint16_t)extents.width;
  const uint16_t bmp_h = (uint16_t)extents.height;
  const size_t pitch = (size_t)extents.stride;

  /* hb origin: bottom-left. convert to vuid origin: top-left */
  for (uint16_t row = 0; row < bmp_h; row++) {
    const size_t src_row = (size_t)(bmp_h - 1 - row);
    memcpy(target + (size_t)((ay + target_padding + row) * target_width + ax +
                             target_padding),
           buffer + src_row * pitch, (size_t)bmp_w);
  }

  hb_raster_draw_recycle_image(hb_draw, img);
  return true;
}
#endif  // VUID_TEXT_ENGINE_HB

#ifdef VUID_TEXT_ENGINE_HB_SHAPER
VUID_PKG bool v_te_shape(VTextEngine* self,
                         VTextEngineFont* font,
                         const char* utf8,
                         uint32_t utf8_len,
                         VArray* out) {
  hb_buffer_t* buf = self->hb_buffer;

  if (!buf) {
    return false;
  }

  hb_buffer_add_utf8(buf, utf8, (int)utf8_len, 0, (int)utf8_len);
  hb_buffer_guess_segment_properties(buf);
  hb_shape(font->hb_font, buf, NULL, 0);

  const hb_position_t scale = (hb_position_t)VUID_HB_FONT_SCALE;
  unsigned int glyph_count = 0;
  hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buf, &glyph_count);
  hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buf, NULL);
  bool result = true;

  for (unsigned int i = 0; i < glyph_count; i++) {
    VShapedGlyph g = {
        .glyph_id = info[i].codepoint,
        .x_advance = (float)(pos[i].x_advance / scale),
        .y_advance = (float)(pos[i].y_advance / scale),
        .x_offset = (float)(pos[i].x_offset / scale),
        .y_offset = (float)(pos[i].y_offset / scale),
        .cluster = info[i].cluster,
    };
    if (!v_array_push(out, &g)) {
      result = false;
      break;
    }
  }

  hb_buffer_clear_contents(buf);
  return result;
}
#endif  // VUID_TEXT_ENGINE_HB_SHAPER



VFontData* v_font_data_new(VAllocator* allocator,
                           VTextEngine* text_engine,
                           VBuffer* file_buffer,
                           uint16_t font_id,
                           const char* name) {
  VFontData* self = v_alloc_zero(allocator, sizeof(VFontData));
  if (!self) {
    v_buffer_drop(file_buffer);
    return NULL;
  }

  self->font_id = font_id;
  self->allocator = allocator;
  self->file_buffer = *file_buffer;
  self->text_engine = text_engine;
  *file_buffer = (VBuffer){0};  // move

  if (!v_ctx_intern_string(name, &self->name)) {
    v_font_data_drop(self);
    return NULL;
  }

  if (!v_te_init_font(text_engine, &self->font, self->file_buffer.data,
                      self->file_buffer.size)) {
    v_font_data_drop(self);
    return NULL;
  }

  return self;
}

void v_font_data_drop(VFontData* self) {
  if (!self) {
    return;
  }

  v_te_drop_font(self->text_engine, &self->font);
  v_buffer_drop(&self->file_buffer);
  v_free(self->allocator, self, sizeof(VFontData));
}

bool v_font_data_set_pixel_size(VFontData* self, uint16_t pixel_size) {
  if (self->current_pixel_size == pixel_size) {
    return true;
  } else if (v_te_set_font_pixel_size(self->text_engine, &self->font,
                                      pixel_size)) {
    self->current_pixel_size = pixel_size;
    return true;
  } else {
    return false;
  }
}

VUID_PKG bool v_font_data_is_legal_name(const char* name) {
  return !v_cstr_is_empty(name) && strlen(name) <= VUID_MAX_FONT_NAME_LENGTH;
}

VUID_PKG bool v_font_data_get_metrics(VFontData* self,
                                      VFontMetrics* font_metrics) {
  return v_te_get_font_metrics(self->text_engine, &self->font,
                               &font_metrics->ascent, &font_metrics->descent,
                               &font_metrics->line_height);
}

#if defined(VUID_TEXT_ENGINE_STB)



VUID_PKG bool v_te_init(VTextEngine* self) {
#ifdef VUID_TEXT_ENGINE_HB_SHAPER
  self->hb_buffer = hb_buffer_create();

  if (!self->hb_buffer) {
    v_te_drop(self);
    return false;
  }
#else
  UNUSED(self);
#endif  // VUID_TEXT_ENGINE_HB_SHAPER

  return true;
}

VUID_PKG void v_te_drop(VTextEngine* self) {
#ifdef VUID_TEXT_ENGINE_HB_SHAPER
  if (self->hb_buffer) {
    hb_buffer_destroy((hb_buffer_t*)self->hb_buffer);
    self->hb_buffer = NULL;
  }
#else
  UNUSED(self);
#endif  // VUID_TEXT_ENGINE_HB_SHAPER
}

VUID_PKG bool v_te_init_font(VTextEngine* self,
                             VTextEngineFont* font,
                             const uint8_t* bytes,
                             size_t size) {
  UNUSED(self, size);

  int offset = stbtt_GetFontOffsetForIndex(bytes, 0);
  if (offset < 0 || !stbtt_InitFont(&font->stb_fontinfo, bytes, offset)) {
    return false;
  }

#ifdef VUID_TEXT_ENGINE_HB_SHAPER
  hb_blob_t* hb_blob = hb_blob_create((const char*)bytes, (unsigned)size,
                                      HB_MEMORY_MODE_READONLY, NULL, NULL);
  font->hb_face = hb_blob ? hb_face_create_or_fail(hb_blob, 0) : NULL;
  font->hb_font = font->hb_face ? hb_font_create(font->hb_face) : NULL;

  hb_blob_destroy(hb_blob);

  if (!font->hb_face || !font->hb_font) {
    v_te_drop_font(self, font);
    return false;
  }
#endif  // VUID_TEXT_ENGINE_HB_SHAPER

  return true;
}

VUID_PKG void v_te_drop_font(VTextEngine* self, VTextEngineFont* font) {
  UNUSED(self);

#ifdef VUID_TEXT_ENGINE_HB_SHAPER
  hb_face_destroy(font->hb_face);
  hb_font_destroy(font->hb_font);
#endif  // VUID_TEXT_ENGINE_HB_SHAPER

  *font = (VTextEngineFont){0};
}

VUID_PKG bool v_te_set_font_pixel_size(VTextEngine* self,
                                       VTextEngineFont* font,
                                       uint16_t pixel_size) {
  UNUSED(self);
  font->scale =
      stbtt_ScaleForMappingEmToPixels(&font->stb_fontinfo, pixel_size);
  return true;
}

VUID_PKG bool v_te_get_font_metrics(VTextEngine* self,
                                    VTextEngineFont* font,
                                    float* ascent,
                                    float* descent,
                                    float* line_height) {
  UNUSED(self);
  const float scale = font->scale;
  int ascent_u;
  int descent_u;
  int line_gap_u;

  stbtt_GetFontVMetrics(&font->stb_fontinfo, &ascent_u, &descent_u,
                        &line_gap_u);

  *ascent = (float)ascent_u * scale;
  *descent = (float)descent_u * scale;
  *line_height = (float)(ascent_u - descent_u + line_gap_u) * scale;

  return true;
}

VUID_PKG bool v_te_get_glyph_bitmap_desc(VTextEngine* self,
                                         VTextEngineFont* font,
                                         uint32_t glyph_id,
                                         VGlyphBitmapDesc* bitmap_desc) {
  UNUSED(self);
  const float scale = font->scale;
  int ix0 = 0;
  int iy0 = 0;
  int ix1 = 0;
  int iy1 = 0;

  stbtt_GetGlyphBitmapBox(&font->stb_fontinfo, (int)glyph_id, scale, scale,
                          &ix0, &iy0, &ix1, &iy1);

  bitmap_desc->bearing_x = (float)ix0;
  bitmap_desc->bearing_y = (float)(-iy0);
  bitmap_desc->width = (uint16_t)(ix1 - ix0);
  bitmap_desc->height = (uint16_t)(iy1 - iy0);
  bitmap_desc->glyph_id = glyph_id;

  return true;
}

VUID_PKG bool v_te_rasterize_glyph(VTextEngine* self,
                                   VTextEngineFont* font,
                                   VGlyphBitmapDesc* bitmap_desc,
                                   uint8_t* target,
                                   uint16_t target_width,
                                   uint16_t target_padding,
                                   uint16_t ax,
                                   uint16_t ay) {
  UNUSED(self);
  const float scale = font->scale;
  const int bmp_w = (int)bitmap_desc->width;
  const int bmp_h = (int)bitmap_desc->height;

  // Render directly into the atlas pixel buffer (stride = atlas width).
  stbtt_MakeGlyphBitmap(
      &font->stb_fontinfo,
      target + (ay + target_padding) * target_width + (ax + target_padding),
      bmp_w, bmp_h, (int)target_width, scale, scale,
      (int)bitmap_desc->glyph_id);

  return true;
}

#ifndef VUID_TEXT_ENGINE_HB_SHAPER
VUID_PKG bool v_te_shape(VTextEngine* self,
                         VTextEngineFont* font,
                         const char* utf8,
                         uint32_t utf8_len,
                         VArray* out) {
  UNUSED(self);
  const stbtt_fontinfo* info = &font->stb_fontinfo;
  const float scale = font->scale;
  const unsigned char* s = (const unsigned char*)utf8;
  const unsigned char* end = s + utf8_len;
  uint32_t cluster = 0;
  int prev_glyph_idx = 0;

  while (s < end) {
    uint32_t codepoint;
    uint32_t consumed;

    if (!v_utf8_get_codepoint(s, &codepoint, &consumed)) {
      // invalid UTF-8 sequence. bail.
      return false;
    }

    int glyph_idx = stbtt_FindGlyphIndex(info, (int)codepoint);

    float x_advance = 0.0f;
    if (glyph_idx > 0) {
      int adv_w, lsb;
      stbtt_GetGlyphHMetrics(info, glyph_idx, &adv_w, &lsb);
      x_advance = (float)adv_w * scale;
    }

    // Apply kerning to the previous glyph's advance.
    if (prev_glyph_idx > 0 && glyph_idx > 0 && out->size > 0) {
      int kern = stbtt_GetGlyphKernAdvance(info, prev_glyph_idx, glyph_idx);
      if (kern != 0) {
        VShapedGlyph* prev_sg =
            (VShapedGlyph*)v_array_get_unchecked(out, out->size - 1);
        prev_sg->x_advance += (float)kern * scale;
      }
    }

    VShapedGlyph g = {
        .glyph_id = (uint32_t)glyph_idx,
        .x_advance = x_advance,
        .y_advance = 0.0f,
        .x_offset = 0.0f,
        .y_offset = 0.0f,
        .cluster = cluster,
    };
    if (!v_array_push(out, &g)) {
      return false;
    }

    prev_glyph_idx = glyph_idx;
    cluster += consumed;
    s += consumed;
  }

  return true;
}
#endif  // !VUID_TEXT_ENGINE_HB_SHAPER
#endif  // VUID_TEXT_ENGINE_STB




VUID_PKG bool v_text_module_init(VTextModule* self,
                                 VAllocator* allocator,
                                 uint32_t atlas_size,
                                 uint32_t atlas_max_size) {
  *self = (VTextModule){
      .allocator = allocator,
      .next_font_id = 1,
  };

  if (!v_te_init(&self->text_engine)) {
    goto defer;
  }

  if (!v_glyph_atlas_init(&self->atlas, allocator, (uint16_t)atlas_size,
                          (uint16_t)atlas_max_size)) {
    goto defer;
  }

  // TODO: allocs can fail
  self->font_data = v_array_init(allocator, sizeof(void*), 4);
  self->scratch_shaped = v_array_init(allocator, sizeof(VShapedGlyph), 64);
  self->scratch_glyphs = v_array_init(allocator, sizeof(VTextGlyph), 64);
  self->scratch_lines = v_array_init(allocator, sizeof(VTextLine), 8);

  return true;

defer:
  v_text_module_drop(self);
  *self = (VTextModule){0};
  return false;
}

VUID_PKG void v_text_module_drop(VTextModule* self) {
  for (size_t i = 0; i < self->font_data.size; i++) {
    VFontData* data = v_array_get_ptr_unchecked(&self->font_data, i);
    v_font_data_drop(data);
  }

  v_array_drop(&self->font_data);
  v_array_drop(&self->scratch_shaped);
  v_array_drop(&self->scratch_glyphs);
  v_array_drop(&self->scratch_lines);
  v_glyph_atlas_drop(&self->atlas);

  v_te_drop(&self->text_engine);
}

VUID_PKG void* v_text_module_get_hb_buffer(void) {
#ifdef VUID_FONT_SHAPER_HARFBUZZ
  VTextModule* text_module = v_ctx_text_module();
  return text_module->hb_buffer;
#else
  return NULL;
#endif
}

VUID_PKG bool v_text_module_add_font(VTextModule* self,
                                     const char* name,
                                     VBuffer* file_buffer) {
  if (v_text_module_find_font_by_name(self, name) != NULL) {
    v_buffer_drop(file_buffer);
    return false;
  }

  uint16_t font_id = self->next_font_id;
  VFontData* font_data = v_font_data_new(self->allocator, &self->text_engine,
                                         file_buffer, font_id, name);

  if (!font_data) {
    return false;
  }

  if (!v_array_push(&self->font_data, &font_data)) {
    v_font_data_drop(font_data);
    return false;
  }

  self->next_font_id++;

  return true;
}

VUID_PKG void v_text_module_remove_font(VTextModule* self, const char* name) {
  VFontData* font = NULL;

  for (size_t i = 0; i < self->font_data.size; i++) {
    VFontData* data = v_array_get_ptr_unchecked(&self->font_data, i);
    if (v_cstr_eq(data->name.str, name)) {
      font = data;
      v_font_data_drop(font);
      v_array_remove(&self->font_data, i);
      break;
    }
  }
}

VUID_PKG VTextLayout* v_text_module_create_layout(VTextModule* self,
                                                  uint16_t font_id,
                                                  uint16_t pixel_size,
                                                  const VString* utf8,
                                                  VTextWrap wrap,
                                                  VAlignX talign,
                                                  float wrap_width) {
  return v_text_module_create_layout_z(
      self, font_id, pixel_size, v_string_cstr(utf8),
      (size_t)v_string_size(utf8), wrap, talign, wrap_width);
}

VUID_PKG VTextLayout* v_text_module_create_layout_z(VTextModule* self,
                                                    uint16_t font_id,
                                                    uint16_t pixel_size,
                                                    const char* utf8,
                                                    size_t utf8_len,
                                                    VTextWrap wrap,
                                                    VAlignX talign,
                                                    float wrap_width) {
  VFontData* font_data = v_text_module_get_font_data(self, font_id);

  if (!font_data) {
    return NULL;
  }

  if (!v_font_data_set_pixel_size(font_data, pixel_size)) {
    return NULL;
  }

  VFontMetrics font_metrics;
  if (!v_font_data_get_metrics(font_data, &font_metrics)) {
    return NULL;
  }

  v_array_clear(&self->scratch_shaped);

  if (!v_te_shape(&self->text_engine, &font_data->font, utf8,
                  (uint32_t)utf8_len, &self->scratch_shaped)) {
    return NULL;
  }

  VTextLayout* layout = v_text_layout_build(
      self->allocator, self->scratch_shaped.items,
      (uint32_t)self->scratch_shaped.size, utf8, &font_metrics, wrap, talign,
      wrap_width, &self->scratch_glyphs, &self->scratch_lines);
  if (layout) {
    layout->font_id = font_id;
    layout->pixel_size = pixel_size;
  }
  return layout;
}

VUID_PKG void v_text_module_destroy_layout(VTextModule* self,
                                           VTextLayout* layout) {
  UNUSED(self);
  if (layout) {
    size_t total = sizeof(VTextLayout) +
                   layout->glyph_count * sizeof(VTextGlyph) +
                   layout->line_count * sizeof(VTextLine);
    v_free(layout->allocator, layout, total);
  }
}

VUID_PKG VFontData* v_text_module_get_font_data(VTextModule* self,
                                                uint16_t font_id) {
  for (size_t i = 0; i < self->font_data.size; i++) {
    VFontData* data = v_array_get_ptr_unchecked(&self->font_data, i);
    if (data->font_id == font_id) {
      return data;
    }
  }
  return NULL;
}

VUID_PKG VFontData* v_text_module_find_font_by_name(VTextModule* self,
                                                    const char* font_name) {
  for (size_t i = 0; i < self->font_data.size; i++) {
    VFontData* data = v_array_get_ptr_unchecked(&self->font_data, i);
    if (v_cstr_eq(data->name.str, font_name)) {
      return data;
    }
  }
  return NULL;
}

VUID_PKG uint16_t v_text_module_find_font_id_by_name(VTextModule* self,
                                                     const char* font_name) {
  VFontData* font_data = v_text_module_find_font_by_name(self, font_name);
  return font_data ? font_data->font_id : 0;
}

VUID_PKG void v_text_module_maybe_rasterize_glyphs(VTextModule* self,
                                                   VTextLayout* layout) {
  VGlyphAtlas* glyph_atlas = &self->atlas;
  uint32_t modified_count = glyph_atlas->modified_count;

  // TODO: not sure about this check
  if (layout->rasterized_at > 0 && layout->rasterized_at == modified_count) {
    return;
  }

  VFontData* fd = v_text_module_get_font_data(self, layout->font_id);

  if (!fd) {
    return;
  }

  v_font_data_set_pixel_size(fd, layout->pixel_size);
  for (uint32_t i = 0; i < layout->glyph_count; i++) {
    v_glyph_atlas_get_or_rasterize(glyph_atlas, &self->text_engine, fd,
                                   layout->glyphs[i].glyph_id,
                                   self->current_frame);
  }

  layout->rasterized_at = modified_count;
}






/* border size for atlas glyphs */
#define VUID_GLYPH_PAD (1)
/* default atlas w/h */
#define VUID_DEFAULT_ATLAS_SIZE (512)

/* get the font engine specific bitmap description for a glyph */
static bool v__get_glyph_bitmap_desc(VTextEngine* text_engine,
                                     uint32_t glyph_id,

                                     VFontData* font_data,
                                     VGlyphBitmapDesc* bitmap_desc);

/* rasterize a glyph into the atlas with the current font engine */
static bool v__rasterize_glyph(VGlyphAtlas* self,
                               VTextEngine* text_engine,
                               VGlyphBitmapDesc* bitmap_desc,
                               VFontData* font_data,
                               uint16_t ax,
                               uint16_t ay);

/* ---- Skyline bin-packer node (private) ---- */

typedef struct VAtlasSkylineNode {
  uint16_t x, y, w;
} VAtlasSkylineNode;

/* ---- VAtlasCacheMap HMAP implementation ---- */

static uint32_t VAtlasCacheEntry_get_hash(const VAtlasCacheEntry* v) {
  uint32_t pfp = ((uint32_t)v->font_id << 16) | v->pixel_size;
  return v_fnv1_hash_mix(v_fnv1_hash_u32(v->glyph_id), v_fnv1_hash_u32(pfp));
}

static uint32_t VAtlasCacheKey_get_hash(const VAtlasCacheKey* k) {
  return v_fnv1_hash_mix(v_fnv1_hash_u32(k->glyph_id),
                         v_fnv1_hash_u32(k->packed_fp));
}

static VAtlasCacheKey VAtlasCacheKey_from_value(const VAtlasCacheEntry* v) {
  return (VAtlasCacheKey){
      .glyph_id = v->glyph_id,
      .packed_fp = ((uint32_t)v->font_id << 16) | v->pixel_size,
  };
}

static bool VAtlasCacheEntry_eq(const VAtlasCacheKey* k,
                                const VAtlasCacheEntry* v) {
  uint32_t pfp = ((uint32_t)v->font_id << 16) | v->pixel_size;
  return k->glyph_id == v->glyph_id && k->packed_fp == pfp;
}

static void VAtlasCacheEntry_drop(VAtlasCacheEntry* v) {
  UNUSED(v);
}

VUID_HMAP_IMPL(VAtlasCacheMap,
               VAtlasCacheKey,
               VAtlasCacheEntry,
               v_atlas_cache_map)

/* ---- Skyline packer helpers ---- */

/*
 * Return the maximum skyline y in the horizontal range [x, x+w).
 * Used to find how high a rect placed starting at x would need to sit.
 */
static uint16_t skyline_max_height_in_range(const VArray* skyline,
                                            uint16_t x,
                                            uint16_t w) {
  uint16_t max_y = 0;
  for (size_t i = 0; i < skyline->size; i++) {
    const VAtlasSkylineNode* n = v_array_get_unchecked(skyline, i);
    if ((uint32_t)n->x + n->w <= x)
      continue;
    if (n->x >= (uint32_t)x + w)
      break;
    if (n->y > max_y)
      max_y = n->y;
  }
  return max_y;
}

/*
 * Find the best position to place a rect of size rw×rh.
 * "Best" means lowest resulting top edge (minimum waste of vertical space).
 * Returns true and sets *out_x, *out_y on success.
 */
static bool skyline_find_best(VArray* skyline,
                              uint16_t atlas_w,
                              uint16_t atlas_h,
                              uint16_t rw,
                              uint16_t rh,
                              uint16_t* out_x,
                              uint16_t* out_y) {
  bool found = false;
  uint16_t best_y = 0, best_x = 0;

  for (size_t i = 0; i < skyline->size; i++) {
    const VAtlasSkylineNode* n = v_array_get_unchecked(skyline, i);
    uint16_t x = n->x;

    if ((uint32_t)x + rw > atlas_w)
      continue;

    uint16_t y = skyline_max_height_in_range(skyline, x, rw);
    if ((uint32_t)y + rh > atlas_h)
      continue;

    if (!found || y < best_y) {
      best_y = y;
      best_x = x;
      found = true;
    }
  }

  if (!found)
    return false;
  *out_x = best_x;
  *out_y = best_y;
  return true;
}

/*
 * Update the skyline after placing a rect at [px, px+rw) with its new top
 * edge at new_top.  Removes all nodes fully covered by the rect and inserts
 * up to three replacement nodes (left remnant, new node, right remnant).
 */
static bool skyline_place(VArray* skyline,
                          uint16_t px,
                          uint16_t new_top,
                          uint16_t rw) {
  size_t first = skyline->size, last = 0;

  for (size_t i = 0; i < skyline->size; i++) {
    const VAtlasSkylineNode* n = v_array_get_unchecked(skyline, i);
    if ((uint32_t)n->x + n->w <= px)
      continue;
    if (n->x >= (uint32_t)px + rw)
      break;
    if (first == skyline->size)
      first = i;
    last = i;
  }

  if (first == skyline->size)
    return false;

  VAtlasSkylineNode first_n =
      *(const VAtlasSkylineNode*)v_array_get_unchecked(skyline, first);
  VAtlasSkylineNode last_n =
      *(const VAtlasSkylineNode*)v_array_get_unchecked(skyline, last);

  // Remove covered nodes from back to front so indices stay valid.
  for (size_t i = last + 1; i-- > first;) {
    v_array_remove(skyline, i);
  }

  // Insert in reverse order at index `first` so the final layout is:
  //   [..., left_remnant?, new_node, right_remnant?, ...]
  if ((uint32_t)last_n.x + last_n.w > (uint32_t)px + rw) {
    VAtlasSkylineNode right = {
        .x = (uint16_t)((uint32_t)px + rw),
        .y = last_n.y,
        .w = (uint16_t)((uint32_t)last_n.x + last_n.w - ((uint32_t)px + rw)),
    };
    if (!v_array_insert(skyline, first, &right))
      return false;
  }

  VAtlasSkylineNode new_node = {.x = px, .y = new_top, .w = rw};
  if (!v_array_insert(skyline, first, &new_node))
    return false;

  if (first_n.x < px) {
    VAtlasSkylineNode left = {
        .x = first_n.x,
        .y = first_n.y,
        .w = (uint16_t)(px - first_n.x),
    };
    if (!v_array_insert(skyline, first, &left))
      return false;
  }

  return true;
}

// Merge adjacent skyline nodes at the same height.
static void skyline_merge(VArray* skyline) {
  size_t i = 0;
  while (i + 1 < skyline->size) {
    VAtlasSkylineNode* a = v_array_get_unchecked(skyline, i);
    const VAtlasSkylineNode* b = v_array_get_unchecked(skyline, i + 1);
    if (a->y == b->y) {
      a->w = (uint16_t)((uint32_t)a->w + b->w);
      v_array_remove(skyline, i + 1);
    } else {
      i++;
    }
  }
}

/*
 * Double the atlas dimensions (up to max_size), expanding the pixel buffer
 * in-place and extending the skyline to cover the new right region.
 * Returns true on success; leaves atlas unchanged on failure.
 *
 * Growth is transactional: the skyline node is appended first so that if
 * the pixel realloc fails, a simple pop restores the original skyline state.
 */
static bool atlas_try_grow(VGlyphAtlas* self) {
  if (self->width >= self->max_size) {
    return false;
  }

  uint32_t new_dim = (uint32_t)self->width * 2;
  if (new_dim > self->max_size) {
    new_dim = self->max_size;
  }

  uint16_t old_w = self->width;
  uint16_t right_w = (uint16_t)(new_dim - old_w);

  /* Append the right-region skyline node before resizing pixels so that
   * if the realloc fails we can roll back with a simple pop. */
  VAtlasSkylineNode right_node = {.x = old_w, .y = 0, .w = right_w};
  if (!v_array_push(&self->skyline, &right_node)) {
    return false;
  }

  size_t old_bytes = (size_t)old_w * old_w;
  size_t new_bytes = (size_t)new_dim * new_dim;
  uint8_t* buf = v_realloc(self->allocator, self->pixels, old_bytes, new_bytes);
  if (!buf) {
    v_array_remove(&self->skyline, self->skyline.size - 1);
    return false;
  }
  self->pixels = buf;

  /* Expand old rows into the wider layout, working bottom-to-top so that
   * the right-half zeroing of row r cannot clobber the src data of row r+1
   * (which hasn't been moved yet). */
  for (int row = (int)old_w - 1; row >= 0; row--) {
    uint8_t* src = buf + (size_t)row * old_w;
    uint8_t* dst = buf + (size_t)row * new_dim;
    memmove(dst, src, old_w);
    memset(dst + old_w, 0, right_w);
  }

  /* Zero the new bottom region (rows old_w..new_dim-1). */
  memset(buf + (size_t)old_w * new_dim, 0, (size_t)(new_dim - old_w) * new_dim);

  skyline_merge(&self->skyline);

  self->width = (uint16_t)new_dim;
  self->height = (uint16_t)new_dim;
  self->size_version++;

  return true;
}

/* ---- Public API ---- */

VUID_PKG bool v_glyph_atlas_init(VGlyphAtlas* self,
                                 VAllocator* alloc,
                                 uint32_t width,
                                 uint32_t max_size) {
  if (width == 0) {
    width = VUID_DEFAULT_ATLAS_SIZE;
  }

  if (max_size == 0) {
    max_size = width;
  }

  width = (uint32_t)v_ensure_pow_2((size_t)width, 32, UINT32_MAX);
  max_size =
      (uint32_t)v_ensure_pow_2((size_t)max_size, (size_t)width, UINT32_MAX);

  if (width > max_size) {
    return false;
  }

  *self = (VGlyphAtlas){
      .allocator = alloc,
      .width = width,
      .height = width, /* always square */
      .max_size = max_size,
      .pixels = v_alloc_zero(alloc, (size_t)width * width),
      .cache = v_atlas_cache_map_init(alloc),
      .skyline = v_array_init(alloc, sizeof(VAtlasSkylineNode), 16),
  };

  VAtlasSkylineNode initial = {.x = 0, .y = 0, .w = width};

  if (!v_array_push(&self->skyline, &initial)) {
    v_glyph_atlas_drop(self);
    return false;
  }

  return true;
}

VUID_PKG void v_glyph_atlas_drop(VGlyphAtlas* self) {
  if (self->pixels) {
    v_free(self->allocator, self->pixels, (size_t)self->width * self->height);
    self->pixels = NULL;
  }
  v_atlas_cache_map_drop(&self->cache);
  v_array_drop(&self->skyline);
}

VUID_PKG void v_glyph_atlas_clear(VGlyphAtlas* self) {
  memset(self->pixels, 0, (size_t)self->width * self->height);
  v_atlas_cache_map_drop(&self->cache);
  self->cache = v_atlas_cache_map_init(self->allocator);
  v_array_clear(&self->skyline);
  VAtlasSkylineNode initial = {.x = 0, .y = 0, .w = self->width};
  v_array_push(&self->skyline, &initial);
}

VUID_PKG VAtlasCacheEntry* v_glyph_atlas_get_or_rasterize(
    VGlyphAtlas* self,
    VTextEngine* text_engine,
    VFontData* font_data,
    uint32_t glyph_id,
    uint32_t frame) {
  uint16_t pixel_size = font_data->current_pixel_size;
  VAtlasCacheKey key = {
      .glyph_id = glyph_id,
      .packed_fp = ((uint32_t)font_data->font_id << 16) | pixel_size,
  };

  VAtlasCacheEntry* cached = v_atlas_cache_map_get(&self->cache, key);
  if (cached) {
    cached->last_used_frame = frame;
    return cached;
  }

  VGlyphBitmapDesc bitmap;

  if (!v__get_glyph_bitmap_desc(text_engine, glyph_id, font_data, &bitmap)) {
    return NULL;
  }

  // Zero-size glyphs (e.g. space): cache with zero atlas dimensions.
  if (bitmap.width == 0 || bitmap.height == 0) {
    VAtlasCacheEntry entry = {
        .glyph_id = glyph_id,
        .font_id = font_data->font_id,
        .pixel_size = pixel_size,
        .atlas_x = 0,
        .atlas_y = 0,
        .atlas_w = 0,
        .atlas_h = 0,
        .bearing_x = bitmap.bearing_x,
        .bearing_y = bitmap.bearing_y,
        .last_used_frame = frame,
    };

    VAtlasCacheMapResult result = v_atlas_cache_map_put(&self->cache, entry);
    return result.inserted ? result.ref : NULL;
  }

  // Row stride: pitch is positive for FT_RENDER_MODE_NORMAL.
  uint16_t real_w = bitmap.width + (VUID_GLYPH_PAD * 2);
  uint16_t real_h = bitmap.height + (VUID_GLYPH_PAD * 2);

  /* Strategy: pack → grow+pack → clear+pack → fail.
   * NOTE: the clear fallback discards all cached glyph entries; layouts that
   * have already set needs_rasterization=false will not re-rasterize, so any
   * text rendered before the clear may have missing glyphs until the next
   * layout invalidation. */
  uint16_t ax = 0, ay = 0;
  bool packed = skyline_find_best(&self->skyline, self->width, self->height,
                                  real_w, real_h, &ax, &ay);
  if (!packed) {
    if (atlas_try_grow(self)) {
      packed = skyline_find_best(&self->skyline, self->width, self->height,
                                 real_w, real_h, &ax, &ay);
    }
  }
  if (!packed) {
    v_glyph_atlas_clear(self);
    packed = skyline_find_best(&self->skyline, self->width, self->height,
                               real_w, real_h, &ax, &ay);
  }
  if (!packed) {
    return NULL;
  }

  if (!skyline_place(&self->skyline, ax, ay + real_h, real_w)) {
    return NULL;
  }
  skyline_merge(&self->skyline);

  if (!v__rasterize_glyph(self, text_engine, &bitmap, font_data, ax, ay)) {
    return NULL;
  }

  self->modified_count++;

  VAtlasCacheEntry entry = {
      .glyph_id = glyph_id,
      .font_id = font_data->font_id,
      .pixel_size = pixel_size,
      .atlas_x = ax + VUID_GLYPH_PAD,
      .atlas_y = ay + VUID_GLYPH_PAD,
      .atlas_w = bitmap.width,
      .atlas_h = bitmap.height,
      .bearing_x = bitmap.bearing_x,
      .bearing_y = bitmap.bearing_y,
      .last_used_frame = frame,
  };

  VAtlasCacheMapResult result = v_atlas_cache_map_put(&self->cache, entry);
  return result.inserted ? result.ref : NULL;
}

VUID_PKG uint32_t v_glyph_atlas_get_modified_count(VGlyphAtlas* atlas) {
  return atlas->modified_count;
}

VUID_PKG uint16_t v_glyph_atlas_get_size(const VGlyphAtlas* atlas) {
  return atlas->width;
}

VUID_PKG VImageBuffer v_glyph_atlas_get_pixel_buffer(const VGlyphAtlas* atlas) {
  VImageBuffer result = {
      .width = atlas->width,
      .height = atlas->height,
      .bytes = atlas->pixels,
      .format = V_PIXEL_FORMAT_A8,
  };
  return result;
}

static bool v__get_glyph_bitmap_desc(VTextEngine* text_engine,
                                     uint32_t glyph_id,
                                     VFontData* font_data,
                                     VGlyphBitmapDesc* bitmap_desc) {
  return v_te_get_glyph_bitmap_desc(text_engine, &font_data->font, glyph_id,
                                    bitmap_desc);
}

static bool v__rasterize_glyph(VGlyphAtlas* self,
                               VTextEngine* text_engine,
                               VGlyphBitmapDesc* bitmap_desc,
                               VFontData* font_data,
                               uint16_t ax,
                               uint16_t ay) {
  return v_te_rasterize_glyph(text_engine, &font_data->font, bitmap_desc,
                              self->pixels, self->width, VUID_GLYPH_PAD, ax,
                              ay);
}

#if defined(VUID_TEXT_ENGINE_FT)


VUID_PKG bool v_te_init(VTextEngine* self) {
  FT_Library library;
  if (FT_Init_FreeType(&library) != 0) {
    return false;
  }

  *self = (VTextEngine){
      .ft_library = library,
  };

#ifdef VUID_TEXT_ENGINE_HB_SHAPER
  self->hb_buffer = hb_buffer_create();

  if (!self->hb_buffer) {
    v_te_drop(self);
    return false;
  }
#endif  // VUID_TEXT_ENGINE_HB_SHAPER

  return true;
}

VUID_PKG void v_te_drop(VTextEngine* self) {
#ifdef VUID_TEXT_ENGINE_HB_SHAPER
  if (self->hb_buffer) {
    hb_buffer_destroy((hb_buffer_t*)self->hb_buffer);
    self->hb_buffer = NULL;
  }
#endif  // VUID_TEXT_ENGINE_HB_SHAPER

  if (self->ft_library) {
    FT_Done_FreeType(self->ft_library);
    self->ft_library = NULL;
  }
}

VUID_PKG bool v_te_init_font(VTextEngine* self,
                             VTextEngineFont* font,
                             const uint8_t* bytes,
                             size_t size) {
  FT_Face face;
  FT_Error err;

  err = FT_New_Memory_Face(self->ft_library, bytes, (FT_Long)size, 0, &face);

  if (err) {
    // TODO: log freetype error
    // printf("FT_New_Memory_Face error: %s\n", FT_Error_String(err));
    return false;
  }

  *font = (VTextEngineFont){
      .ft_face = face,
  };

  FT_Select_Charmap(face, FT_ENCODING_UNICODE);

#if defined(VUID_TEXT_ENGINE_HB_SHAPER)
  font->hb_face = hb_ft_face_create_referenced(face);
  font->hb_font = hb_ft_font_create_referenced(face);

  if (!font->hb_face || !font->hb_font) {
    v_te_drop_font(self, font);
    return false;
  }

  hb_ft_font_set_load_flags(font->hb_font, VUID_FT_LOAD_FLAGS);
#endif  // VUID_TEXT_ENGINE_HB_SHAPER

  return true;
}

VUID_PKG void v_te_drop_font(VTextEngine* self, VTextEngineFont* font) {
  UNUSED(self);

#if defined(VUID_TEXT_ENGINE_HB_SHAPER)
  if (font->hb_font) {
    hb_font_destroy(font->hb_font);
    font->hb_font = NULL;
  }
  if (font->hb_face) {
    hb_face_destroy(font->hb_face);
    font->hb_face = NULL;
  }
#endif  // VUID_TEXT_ENGINE_HB_SHAPER

  if (font->ft_face) {
    FT_Done_Face(font->ft_face);
    font->ft_face = NULL;
  }

  *font = (VTextEngineFont){0};
}

VUID_PKG bool v_te_set_font_pixel_size(VTextEngine* self,
                                       VTextEngineFont* font,
                                       uint16_t pixel_size) {
  UNUSED(self);
  return (FT_Set_Pixel_Sizes(font->ft_face, 0, (FT_UInt)pixel_size) == 0);
}

VUID_PKG bool v_te_get_font_metrics(VTextEngine* self,
                                    VTextEngineFont* font,
                                    float* ascent,
                                    float* descent,
                                    float* line_height) {
  UNUSED(self);

  FT_Face face = font->ft_face;

  *ascent = (float)face->size->metrics.ascender / 64.0f;
  *descent = (float)face->size->metrics.descender / 64.0f;
  *line_height = (float)face->size->metrics.height / 64.0f;

  return true;
}

VUID_PKG bool v_te_get_glyph_bitmap_desc(VTextEngine* self,
                                         VTextEngineFont* font,
                                         uint32_t glyph_id,
                                         VGlyphBitmapDesc* bitmap_desc) {
  UNUSED(self);
  FT_Face ft_face = font->ft_face;

  if (FT_Load_Glyph(ft_face, glyph_id, VUID_FT_LOAD_FLAGS) != 0) {
    return false;
  }

  if (FT_Render_Glyph(font->ft_face->glyph, FT_RENDER_MODE_NORMAL) != 0) {
    return false;
  }

  bitmap_desc->glyph_id = glyph_id;
  bitmap_desc->width = (uint16_t)ft_face->glyph->bitmap.width;
  bitmap_desc->height = (uint16_t)ft_face->glyph->bitmap.rows;
  bitmap_desc->bearing_x = (float)ft_face->glyph->bitmap_left;
  bitmap_desc->bearing_y = (float)ft_face->glyph->bitmap_top;

  return true;
}

VUID_PKG bool v_te_rasterize_glyph(VTextEngine* self,
                                   VTextEngineFont* font,
                                   VGlyphBitmapDesc* bitmap_desc,
                                   uint8_t* target,
                                   uint16_t target_width,
                                   uint16_t target_padding,
                                   uint16_t tx,
                                   uint16_t ty) {
  UNUSED(self, bitmap_desc);
  FT_Bitmap* bmp = &font->ft_face->glyph->bitmap;
  uint16_t bmp_w = (uint16_t)bmp->width;
  uint16_t bmp_h = (uint16_t)bmp->rows;
  size_t pitch = (bmp->pitch >= 0) ? (size_t)bmp->pitch : (size_t)(-bmp->pitch);

  for (uint16_t row = 0; row < bmp_h; row++) {
    memcpy(target + ((size_t)(ty + target_padding + row) * target_width + tx +
                     target_padding),
           bmp->buffer + (size_t)row * pitch, bmp_w);
  }

  return true;
}

#ifndef VUID_TEXT_ENGINE_HB_SHAPER
VUID_PKG bool v_te_shape(VTextEngine* self,
                         VTextEngineFont* font,
                         const char* utf8,
                         uint32_t utf8_len,
                         VArray* out) {
  UNUSED(self);
  FT_Face ft_face = font->ft_face;
  const unsigned char* s = (const unsigned char*)utf8;
  const unsigned char* end = s + utf8_len;
  uint32_t cluster = 0;
  FT_UInt prev_glyph_idx = 0;
  bool has_kerning = (bool)FT_HAS_KERNING(ft_face);

  while (s < end) {
    uint32_t codepoint;
    uint32_t consumed;

    if (!v_utf8_get_codepoint(s, &codepoint, &consumed)) {
      // invalid UTF-8 sequence. bail.
      return false;
    }

    FT_UInt glyph_idx = FT_Get_Char_Index(ft_face, (FT_ULong)codepoint);

    float x_advance = 0.0f;
    if (glyph_idx > 0 &&
        FT_Load_Glyph(ft_face, glyph_idx, VUID_FT_LOAD_FLAGS) == 0) {
      FT_GlyphSlot slot = ft_face->glyph;
      x_advance = (float)slot->advance.x / 64.0f;
      // lsb_delta - rsb_delta corrects for sub-pixel hinting drift.
      x_advance += (float)(slot->lsb_delta - slot->rsb_delta) / 64.0f;
    }

    // Apply kerning to the previous glyph's advance.
    if (has_kerning && prev_glyph_idx > 0 && glyph_idx > 0 && out->size > 0) {
      FT_Vector delta;
      if (FT_Get_Kerning(ft_face, prev_glyph_idx, glyph_idx, FT_KERNING_DEFAULT,
                         &delta) == 0 &&
          delta.x != 0) {
        VShapedGlyph* prev_sg =
            (VShapedGlyph*)v_array_get_unchecked(out, out->size - 1);
        prev_sg->x_advance += (float)delta.x / 64.0f;
      }
    }

    VShapedGlyph g = {
        .glyph_id = glyph_idx,
        .x_advance = x_advance,
        .y_advance = 0.0f,
        .x_offset = 0.0f,
        .y_offset = 0.0f,
        .cluster = cluster,
    };
    if (!v_array_push(out, &g)) {
      return false;
    }

    prev_glyph_idx = glyph_idx;
    cluster += consumed;
    s += consumed;
  }
  return true;
}
#endif  // !VUID_TEXT_ENGINE_HB_SHAPER
#endif  // VUID_TEXT_ENGINE_FT





#define VUID_EQ(A, B) ((A) == (B))
#define VUID_SV_ZERO \
  { .unit = V_STYLE_VALUE_UNIT_PX, .value.px = 0.0f }
#define VUID_VALIDATE_PROP(PROP, META_KEY, VALUE) \
  g_style_props[PROP].validate_fn.META_KEY(&VALUE)
#define VUID_VALIDATE_NONE(PROP, META_KEY, VALUE)

// TODO: send event to owner, rather than doing the owner's work here
#define VUID_PROPERTY_UPDATE_OWNER(STYLE, PROPERTY)            \
  do {                                                         \
    VNode* node = v_style_get_owner(STYLE);                    \
                                                               \
    if (node) {                                                \
      if (g_style_props[PROPERTY].affects_text_style &&        \
          node->tag == V_NODE_TEXT && node->res.text_layout) { \
        v_text_module_destroy_layout(v_ctx_text_module(),      \
                                     node->res.text_layout);   \
        node->res.text_layout = NULL;                          \
      }                                                        \
      if (g_style_props[PROPERTY].affects_layout) {            \
        v_node_mark_dirty(node);                               \
      }                                                        \
      if (g_style_props[PROPERTY].affects_render) {            \
        v_ctx_request_render();                                \
      }                                                        \
    }                                                          \
  } while (0)

#define VUID_PROPERTY_FUNCTIONS_IMPL(PROPERTY, FIELD, TYPE, CMP, META_KEY, \
                                     VAL)                                  \
  VUID_API void vs_set_##FIELD(VStyle* style, TYPE value) {                \
    if (!style) {                                                          \
      return;                                                              \
    }                                                                      \
    VAL(PROPERTY, META_KEY, value);                                        \
    if (vs_has_prop(style, PROPERTY) && CMP(style->FIELD, value)) {        \
      return;                                                              \
    }                                                                      \
                                                                           \
    style->FIELD = value;                                                  \
    style->is_set |= (((uint64_t)1) << PROPERTY);                          \
    VUID_PROPERTY_UPDATE_OWNER(style, PROPERTY);                           \
  }                                                                        \
                                                                           \
  VUID_API TYPE vs_get_##FIELD(const VStyle* style) {                      \
    return (style && vs_has_prop(style, PROPERTY))                         \
               ? style->FIELD                                              \
               : g_style_props[PROPERTY].default_value.META_KEY;           \
  }                                                                        \
                                                                           \
  VUID_API void vs_unset_##FIELD(VStyle* style) {                          \
    const uint64_t PROPERTY_MASK = (((uint64_t)1) << PROPERTY);            \
    if (style && (style->is_set & PROPERTY_MASK)) {                        \
      style->is_set &= ~PROPERTY_MASK;                                     \
      VUID_PROPERTY_UPDATE_OWNER(style, PROPERTY);                         \
    }                                                                      \
  }                                                                        \
  VUID_API bool vs_has_##FIELD(const VStyle* style) {                      \
    return (style && vs_has_prop(style, PROPERTY));                        \
  }

static bool v_style_validate_string(const char** value);
static bool v_style_validate_float(VStyleValue* value);
static bool v_style_validate_gte0_auto(VStyleValue* value);
static bool v_style_validate_dimension(VStyleValue* value);
static bool v_style_validate_gte0(VStyleValue* value);

static inline void v_clamp_length(float* value) {
  const float v = *value;

  if (isnan(v)) {
    *value = 0;
  } else if (v > VUID_FLOAT_MAX_INT) {
    *value = VUID_FLOAT_MAX_INT;
  } else if (v < VUID_FLOAT_MIN_INT) {
    *value = -VUID_FLOAT_MIN_INT;
  }
}

static inline void v_clamp_length_gte0(float* value) {
  const float v = *value;

  if (isnan(v) || v < 0) {
    *value = 0;
  } else if (v > VUID_FLOAT_MAX_INT) {
    *value = VUID_FLOAT_MAX_INT;
  }
}

static const VStylePropertyMeta g_style_props[VS__STYLE_PROPERTY_COUNT] = {
    // clang-format off
    [VS_WIDTH] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value.unit = V_STYLE_VALUE_UNIT_AUTO,
      .validate_fn.style_value = &v_style_validate_dimension,
      .set_fn.style_value = &vs_set_width,
      .get_fn.style_value = &vs_get_width,
      .affects_layout = true,
    },
    [VS_MIN_WIDTH] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0_auto,
      .set_fn.style_value = &vs_set_min_width,
      .get_fn.style_value = &vs_get_min_width,
      .affects_layout = true,
    },
    [VS_MAX_WIDTH] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0_auto,
      .set_fn.style_value = &vs_set_max_width,
      .get_fn.style_value = &vs_get_max_width,
      .affects_layout = true,
    },
    [VS_HEIGHT] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value.unit = V_STYLE_VALUE_UNIT_AUTO,
      .validate_fn.style_value = &v_style_validate_dimension,
      .set_fn.style_value = &vs_set_height,
      .get_fn.style_value = &vs_get_height,
      .affects_layout = true,
    },
    [VS_MIN_HEIGHT] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0_auto,
      .set_fn.style_value = &vs_set_min_height,
      .get_fn.style_value = &vs_get_min_height,
      .affects_layout = true,
    },
    [VS_MAX_HEIGHT] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0_auto,
      .set_fn.style_value = &vs_set_max_height,
      .get_fn.style_value = &vs_get_max_height,
      .affects_layout = true,
    },
    [VS_DIRECTION] = {
      .tag = VSTAG_ENUM_DIRECTION,
      .default_value.direction = V_DIRECTION_ROW,
      .set_fn.direction = &vs_set_direction,
      .get_fn.direction = &vs_get_direction,
      .affects_layout = true,
    },
    [VS_VISIBILITY] = {
      .tag = VSTAG_ENUM_VISIBILITY,
      .default_value.visibility = V_VISIBILITY_VISIBLE,
      .set_fn.visibility = &vs_set_visibility,
      .get_fn.visibility = &vs_get_visibility,
      .affects_render = true,
    },
    [VS_WRAP] = {
      .tag = VSTAG_ENUM_WRAP,
      .default_value.wrap = V_WRAP_NONE,
      .set_fn.wrap = &vs_set_wrap,
      .get_fn.wrap = &vs_get_wrap,
      .affects_layout = true,
    },
    [VS_XALIGN] = {
      .tag = VSTAG_ENUM_XALIGN,
      .default_value.xalign = V_ALIGN_X_LEFT,
      .set_fn.xalign = &vs_set_xalign,
      .get_fn.xalign = &vs_get_xalign,
      .affects_layout = true,
    },
    [VS_YALIGN] = {
      .tag = VSTAG_ENUM_YALIGN,
      .default_value.yalign = V_ALIGN_Y_TOP,
      .set_fn.yalign = &vs_set_yalign,
      .get_fn.yalign = &vs_get_yalign,
      .affects_layout = true,
    },
    [VS_TALIGN] = {
      .tag = VSTAG_ENUM_XALIGN,
      .default_value.talign = V_ALIGN_X_LEFT,
      .set_fn.xalign = &vs_set_talign,
      .get_fn.xalign = &vs_get_talign,
      // TODO: VTextLayout needs to change to make this a render only prop
      .affects_layout = true,
      .affects_text_style = true,
    },
    [VS_TEXT_WRAP] = {
      .tag = VSTAG_ENUM_TEXT_WRAP,
      .default_value.text_wrap = V_TEXT_WRAP_WRAP,
      .set_fn.text_wrap = &vs_set_text_wrap,
      .get_fn.text_wrap = &vs_get_text_wrap,
      .affects_layout = true,
      .affects_text_style = true,
    },
    [VS_OVERFLOW] = {
      .tag = VSTAG_ENUM_OVERFLOW,
      .default_value.overflow = V_OVERFLOW_VISIBLE,
      .set_fn.overflow = &vs_set_overflow,
      .get_fn.overflow = &vs_get_overflow,
      .affects_layout = true,
    },
    [VS_GAP] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0,
      .set_fn.style_value = &vs_set_gap,
      .get_fn.style_value = &vs_get_gap,
      .affects_layout = true,
    },
    [VS_PADDING_TOP] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0,
      .set_fn.style_value = &vs_set_padding_top,
      .get_fn.style_value = &vs_get_padding_top,
      .affects_layout = true,
    },
    [VS_PADDING_RIGHT] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0,
      .set_fn.style_value = &vs_set_padding_right,
      .get_fn.style_value = &vs_get_padding_right,
      .affects_layout = true,
    },
    [VS_PADDING_BOTTOM] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0,
      .set_fn.style_value = &vs_set_padding_bottom,
      .get_fn.style_value = &vs_get_padding_bottom,
      .affects_layout = true,
    },
    [VS_PADDING_LEFT] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0,
      .set_fn.style_value = &vs_set_padding_left,
      .get_fn.style_value = &vs_get_padding_left,
      .affects_layout = true,
    },
    [VS_MARGIN_TOP] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0_auto,
      .set_fn.style_value = &vs_set_margin_top,
      .get_fn.style_value = &vs_get_margin_top,
      .affects_layout = true,
    },
    [VS_MARGIN_RIGHT] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0_auto,
      .set_fn.style_value = &vs_set_margin_right,
      .get_fn.style_value = &vs_get_margin_right,
      .affects_layout = true,
    },
    [VS_MARGIN_BOTTOM] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0_auto,
      .set_fn.style_value = &vs_set_margin_bottom,
      .get_fn.style_value = &vs_get_margin_bottom,
      .affects_layout = true,
    },
    [VS_MARGIN_LEFT] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0_auto,
      .set_fn.style_value = &vs_set_margin_left,
      .get_fn.style_value = &vs_get_margin_left,
      .affects_layout = true,
    },
    [VS_BORDER_TOP] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0,
      .set_fn.style_value = &vs_set_border_top,
      .get_fn.style_value = &vs_get_border_top,
      .affects_layout = true,
    },
    [VS_BORDER_RIGHT] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0,
      .set_fn.style_value = &vs_set_border_right,
      .get_fn.style_value = &vs_get_border_right,
      .affects_layout = true,
    },
    [VS_BORDER_BOTTOM] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0,
      .set_fn.style_value = &vs_set_border_bottom,
      .get_fn.style_value = &vs_get_border_bottom,
      .affects_layout = true,
    },
    [VS_BORDER_LEFT] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0,
      .set_fn.style_value = &vs_set_border_left,
      .get_fn.style_value = &vs_get_border_left,
      .affects_layout = true,
    },
    [VS_BORDER_RADIUS] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0,
      .set_fn.style_value = &vs_set_border_radius,
      .get_fn.style_value = &vs_get_border_radius,
      .affects_render = true,
    },
    [VS_FONT] = {
      .tag = VSTAG_STRING,
      .default_value.str = "",
      .validate_fn.str = &v_style_validate_string,
      .set_fn.str = &vs_set_font,
      .get_fn.str = &vs_get_font,
      .affects_layout = true,
      .affects_text_style = true,
    },
    [VS_FONT_SIZE] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0,
      .set_fn.style_value = &vs_set_font_size,
      .get_fn.style_value = &vs_get_font_size,
      .affects_layout = true,
      .affects_text_style = true,
    },
    [VS_SCROLLBAR_WIDTH] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0,
      .set_fn.style_value = &vs_set_scrollbar_width,
      .get_fn.style_value = &vs_get_scrollbar_width,
      .affects_layout = true,
    },
    [VS_SCROLLBAR_BORDER_RADIUS] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0,
      .set_fn.style_value = &vs_set_scrollbar_border_radius,
      .get_fn.style_value = &vs_get_scrollbar_border_radius,
      .affects_render = true,
    },
    [VS_BACKGROUND] = {
      .tag = VSTAG_COLOR,
      .default_value.color = {0, 0, 0, 0},
      .set_fn.color = &vs_set_background,
      .get_fn.color = &vs_get_background,
      .affects_render = true,
    },
    [VS_COLOR] = {
      .tag = VSTAG_COLOR,
      .default_value.color = {0, 0, 0, 0},
      .set_fn.color = &vs_set_color,
      .get_fn.color = &vs_get_color,
      .affects_render = true,
    },
    [VS_BORDER_COLOR] = {
      .tag = VSTAG_COLOR,
      .default_value.color = {0, 0, 0, 0},
      .set_fn.color = &vs_set_border_color,
      .get_fn.color = &vs_get_border_color,
      .affects_render = true,
    },
    [VS_SCROLLBAR_THUMB] = {
      .tag = VSTAG_COLOR,
      .default_value.color = {0, 0, 0, 0},
      .set_fn.color = &vs_set_scrollbar_thumb,
      .get_fn.color = &vs_get_scrollbar_thumb,
      .affects_render = true,
    },
    [VS_SCROLLBAR_THUMB_HOVER] = {
      .tag = VSTAG_COLOR,
      .default_value.color = {0, 0, 0, 0},
      .set_fn.color = &vs_set_scrollbar_thumb_hover,
      .get_fn.color = &vs_get_scrollbar_thumb_hover,
      .affects_render = true,
    },
    [VS_ANCHOR_TO] = {
      .tag = VSTAG_ENUM_ANCHOR_TO,
      .default_value.anchor_to = V_ANCHOR_TO_PARENT,
      .set_fn.anchor_to = &vs_set_anchor_to,
      .get_fn.anchor_to = &vs_get_anchor_to,
      .affects_layout = true,
    },
    [VS_ANCHOR_ATTACH_POINT_X] = {
      .tag = VSTAG_ENUM_ATTACH_POINT_X,
      .default_value.attach_point_x = V_ATTACH_POINT_X_LEFT,
      .set_fn.attach_point_x = &vs_set_anchor_attach_point_x,
      .get_fn.attach_point_x = &vs_get_anchor_attach_point_x,
      .affects_layout = true,
    },
    [VS_ANCHOR_ATTACH_POINT_Y] = {
      .tag = VSTAG_ENUM_ATTACH_POINT_Y,
      .default_value.attach_point_y = V_ATTACH_POINT_Y_TOP,
      .set_fn.attach_point_y = &vs_set_anchor_attach_point_y,
      .get_fn.attach_point_y = &vs_get_anchor_attach_point_y,
      .affects_layout = true,
    },
    [VS_ATTACH_POINT_X] = {
      .tag = VSTAG_ENUM_ATTACH_POINT_X,
      .default_value.attach_point_x = V_ATTACH_POINT_X_LEFT,
      .set_fn.attach_point_x = &vs_set_attach_point_x,
      .get_fn.attach_point_x = &vs_get_attach_point_x,
      .affects_layout = true,
    },
    [VS_ATTACH_POINT_Y] = {
      .tag = VSTAG_ENUM_ATTACH_POINT_Y,
      .default_value.attach_point_y = V_ATTACH_POINT_Y_TOP,
      .set_fn.attach_point_y = &vs_set_attach_point_y,
      .get_fn.attach_point_y = &vs_get_attach_point_y,
      .affects_layout = true,
    },
    [VS_ATTACH_POINT_OFFSET_X] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_float,
      .set_fn.style_value = &vs_set_attach_point_offset_x,
      .get_fn.style_value = &vs_get_attach_point_offset_x,
      .affects_layout = true,
    },
    [VS_ATTACH_POINT_OFFSET_Y] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_float,
      .set_fn.style_value = &vs_set_attach_point_offset_y,
      .get_fn.style_value = &vs_get_attach_point_offset_y,
      .affects_layout = true,
    },
    [VS_ASPECT_RATIO] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_gte0,
      .set_fn.style_value = &vs_set_aspect_ratio,
      .get_fn.style_value = &vs_get_aspect_ratio,
      .affects_layout = true,
    },
    [VS_POSITION] = {
      .tag = VSTAG_ENUM_POSITION,
      .default_value.position = V_POSITION_STATIC,
      .set_fn.position = &vs_set_position,
      .get_fn.position = &vs_get_position,
      .affects_layout = true,
    },
    [VS_TOP] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_float,
      .set_fn.style_value = &vs_set_top,
      .get_fn.style_value = &vs_get_top,
      .affects_layout = true,
    },
    [VS_RIGHT] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_float,
      .set_fn.style_value = &vs_set_right,
      .get_fn.style_value = &vs_get_right,
      .affects_layout = true,
    },
    [VS_BOTTOM] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_float,
      .set_fn.style_value = &vs_set_bottom,
      .get_fn.style_value = &vs_get_bottom,
      .affects_layout = true,
    },
    [VS_LEFT] = {
      .tag = VSTAG_STYLE_VALUE,
      .default_value.style_value = VUID_SV_ZERO,
      .validate_fn.style_value = &v_style_validate_float,
      .set_fn.style_value = &vs_set_left,
      .get_fn.style_value = &vs_get_left,
      .affects_layout = true,
    },
    // clang-format on
};

VUID_PKG const VStylePropertyMeta* v_style_get_prop_meta(
    VStyleProperty property) {
  return (property < VS__STYLE_PROPERTY_COUNT) ? &g_style_props[property]
                                               : NULL;
}

static bool v_style_validate_string(const char** value) {
  if (*value == NULL) {
    *value = "";
  }
  return true;
}

static bool v_style_validate_float(VStyleValue* value) {
  switch (value->unit) {
    case V_STYLE_VALUE_UNIT_PX:
      v_clamp_length(&value->value.px);
      return true;
    default:
      return false;
  }
}

static bool v_style_validate_gte0_auto(VStyleValue* value) {
  switch (value->unit) {
    case V_STYLE_VALUE_UNIT_PX:
      v_clamp_length_gte0(&value->value.px);
      return true;
    case V_STYLE_VALUE_UNIT_AUTO:
      return true;
    default:
      return false;
  }
}

static bool v_style_validate_dimension(VStyleValue* value) {
  switch (value->unit) {
    case V_STYLE_VALUE_UNIT_PX:
      v_clamp_length_gte0(&value->value.px);
      return true;
    case V_STYLE_VALUE_UNIT_AUTO:
    case V_STYLE_VALUE_UNIT_FIT:
    case V_STYLE_VALUE_UNIT_GROW:
      return true;
    default:
      return false;
  }
}

static bool v_style_validate_gte0(VStyleValue* value) {
  switch (value->unit) {
    case V_STYLE_VALUE_UNIT_PX:
      v_clamp_length_gte0(&value->value.px);
      return true;
    default:
      return false;
  }
}

#ifdef _MSC_VER
// property specific ifs trigger C4127 warning
#pragma warning(push)
#pragma warning(disable : 4127)
#endif

// clang-format off
VUID_PROPERTY_FUNCTIONS_IMPL(VS_WIDTH, width, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_MIN_WIDTH, min_width, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_MAX_WIDTH, max_width, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_HEIGHT, height, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_MIN_HEIGHT, min_height, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_MAX_HEIGHT, max_height, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_DIRECTION, direction, VDirection, VUID_EQ, direction, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_VISIBILITY, visibility, VVisibility, VUID_EQ, visibility, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_WRAP, wrap, VWrap, VUID_EQ, wrap, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_XALIGN, xalign, VAlignX, VUID_EQ, xalign, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_YALIGN, yalign, VAlignY, VUID_EQ, yalign, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_TALIGN, talign, VAlignX, VUID_EQ, talign, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_TEXT_WRAP, text_wrap, VTextWrap, VUID_EQ, text_wrap, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_OVERFLOW, overflow, VOverflow, VUID_EQ, overflow, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_GAP, gap, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_PADDING_TOP, padding_top, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_PADDING_RIGHT, padding_right, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_PADDING_BOTTOM, padding_bottom, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_PADDING_LEFT, padding_left, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_MARGIN_TOP, margin_top, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_MARGIN_RIGHT, margin_right, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_MARGIN_BOTTOM, margin_bottom, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_MARGIN_LEFT, margin_left, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BORDER_TOP, border_top, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BORDER_RIGHT, border_right, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BORDER_BOTTOM, border_bottom, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BORDER_LEFT, border_left, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BORDER_RADIUS, border_radius, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_FONT, font, const char*, v_cstr_eq, str, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_FONT_SIZE, font_size, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_SCROLLBAR_WIDTH, scrollbar_width, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_SCROLLBAR_BORDER_RADIUS, scrollbar_border_radius, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BACKGROUND, background, VColor, v_color_eq, color, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_COLOR, color, VColor, v_color_eq, color, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BORDER_COLOR, border_color, VColor, v_color_eq, color, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_SCROLLBAR_THUMB, scrollbar_thumb, VColor, v_color_eq, color, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_SCROLLBAR_THUMB_HOVER, scrollbar_thumb_hover, VColor, v_color_eq, color, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ANCHOR_TO, anchor_to, VAnchorTo, VUID_EQ, anchor_to, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ANCHOR_ATTACH_POINT_X, anchor_attach_point_x, VAttachPointX, VUID_EQ, attach_point_x, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ANCHOR_ATTACH_POINT_Y, anchor_attach_point_y, VAttachPointY, VUID_EQ, attach_point_y, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ATTACH_POINT_X, attach_point_x, VAttachPointX, VUID_EQ, attach_point_x, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ATTACH_POINT_Y, attach_point_y, VAttachPointY, VUID_EQ, attach_point_y, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ATTACH_POINT_OFFSET_X, attach_point_offset_x, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ATTACH_POINT_OFFSET_Y, attach_point_offset_y, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_ASPECT_RATIO, aspect_ratio, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_POSITION, position, VPosition, VUID_EQ, position, VUID_VALIDATE_NONE)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_TOP, top, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_RIGHT, right, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_BOTTOM, bottom, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
VUID_PROPERTY_FUNCTIONS_IMPL(VS_LEFT, left, VStyleValue, v_style_value_eq, style_value, VUID_VALIDATE_PROP)
// clang-format on

#ifdef _MSC_VER
#pragma warning(pop)
#endif






static void v_style_destructor(VStyle* style);

VUID_API void v_style_reset(VStyle* style) {
  V_CHECK_CONTEXT();

  if (style && style->is_set) {
    VNode* owner = style->owner;

    // TODO: send event to owner
    if (owner) {
      if (v_node_tag(owner) == V_NODE_TEXT) {
        // TODO: must call unset functions to notify owner of changes (ugly)
        if (vs_has_font(style)) {
          vs_unset_font(style);
        }

        if (vs_has_font_size(style)) {
          vs_unset_font_size(style);
        }
      }

      v_node_mark_dirty(owner);
    }

    // ok, for now since most props don't need cleanup or have side effects
    style->is_set = 0;
  }
}

VUID_API void v_style_ref(VStyle* style) {
  V_CHECK_CONTEXT();

  if (style) {
    if (style->ref_count < UINT32_MAX) {
      style->ref_count++;
    } else {
      VUID_ASSERT(false && "VStyle ref count overflow");
    }
  }
}

VUID_API void v_style_unref(VStyle* style) {
  V_CHECK_CONTEXT();

  if (style) {
    const uint32_t ref_count = style->ref_count;

    if (ref_count > 1) {
      style->ref_count--;
    } else if (ref_count == 1) {
      v_style_destructor(style);
    } else {
      VUID_ASSERT(false && "VStyle ref count underflow");
    }
  }
}

VUID_API uint32_t v_style_ref_count(const VStyle* style) {
  V_CHECK_CONTEXT(0);
  return style ? style->ref_count : 0;
}

// TODO: not the api i want, but sdli needs this
VUID_API float v_style_measure_text_w(const VStyle* style, const char* text) {
  V_CHECK_CONTEXT(0.f);

  if (v_cstr_is_empty(text) || !style || !vs_has_font(style) ||
      !vs_has_font_size(style)) {
    return 0.f;
  }

  // TODO: this measure is heavy.. can we measure without an alloc?
  VTextModule* text_module = v_ctx_text_module();
  const uint16_t font_id =
      v_text_module_find_font_id_by_name(text_module, vs_get_font(style));

  if (!font_id) {
    return 0.f;
  }

  const uint16_t font_size = v_style_resolve_font_size(style);

  VTextLayout* layout = v_text_module_create_layout_z(
      text_module, font_id, font_size, text, strlen(text), V_TEXT_WRAP_NO_WRAP,
      V_ALIGN_X_LEFT, 0.f);
  VSize size = v_text_layout_get_size(layout);

  v_text_module_destroy_layout(text_module, layout);

  return size.width;
}

VUID_PKG VStyle* v_style_new(VNode* owner) {
  VStyle* style = v_ctx_new(VStyle);

  if (style) {
    style->owner = owner;
    style->ref_count = 1;
    v_ctx_object_inc(V_OBJECT_TYPE_STYLE);
  }

  return style;
}

VUID_PKG VNode* v_style_get_owner(const VStyle* style) {
  return style ? style->owner : NULL;
}

VUID_PKG void v_style_set_owner(VStyle* style, VNode* owner) {
  if (style) {
    style->owner = owner;
  }
}

VUID_PKG void v_style_flatten(VStyle* a, VStyle* b) {
  for (int i = 0; i < VS__STYLE_PROPERTY_COUNT; i++) {
    if (!vs_has_prop(b, (VStyleProperty)i)) {
      continue;
    }

    const VStylePropertyMeta* meta = v_style_get_prop_meta((VStyleProperty)i);

    switch (meta->tag) {
      case VSTAG_STYLE_VALUE:
        meta->set_fn.style_value(a, meta->get_fn.style_value(b));
        break;
      case VSTAG_COLOR:
        meta->set_fn.color(a, meta->get_fn.color(b));
        break;
      case VSTAG_STRING:
        meta->set_fn.str(a, meta->get_fn.str(b));
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
      case VSTAG_ENUM_WRAP:
        meta->set_fn.wrap(a, meta->get_fn.wrap(b));
        break;
      case VSTAG_ENUM_TEXT_WRAP:
        meta->set_fn.text_wrap(a, meta->get_fn.text_wrap(b));
        break;
      case VSTAG_ENUM_POSITION:
        meta->set_fn.position(a, meta->get_fn.position(b));
        break;
      default:
        // TODO: unreachable
        break;
    }
  }
}

VUID_PKG bool v_style_class_init(VStyleClass* self, const char* id) {
  VStyle* style = v_style_new(NULL);

  if (!style) {
    return false;
  }

  *self = (VStyleClass){
      .is_ready = false,
      .style = style,
  };

  bool result = v_ctx_intern_string(id, &self->id);

  if (!result) {
    v_style_unref(style);
  }

  return result;
}

VUID_PKG void v_style_class_drop(VStyleClass* self) {
  v_style_unref(self->style);
}

VUID_PKG bool v_style_class_is_valid_name(const char* name) {
  if (!name || !v_char_is_alpha(*name)) {
    return false;
  }

  size_t len = 1;
  name++;

  while (*name) {
    int c = *name;

    if (!(v_char_is_alpha(c) || c == '_' || c == '-' || v_char_is_digit(c))) {
      return false;
    }

    name++;
    len++;

    if (len > VUID_MAX_STYLE_NAME_LENGTH) {
      return false;
    }
  }

  return true;
}

static void v_style_destructor(VStyle* style) {
  v_ctx_delete(style, VStyle);
  v_ctx_object_dec(V_OBJECT_TYPE_STYLE);
}





/*
 * TODO: right now styles are not computed or resolved until needed in the
 * layout pass this is making the code more complicated and less efficient. I
 * have not decided the best way to handle this.
 */

/*
 * Iterate over child nodes that are "in-flow" (visible, non-popover,
 * non-absolute).
 * - ALWAYS use a braced body.
 * - break and continue function as expected within the loop body.
 */
#define v_foreach_flow_child(NODE, CHILD) \
  v_foreach_child(NODE, CHILD) if (v_is_in_flow(CHILD))

static inline bool v_style__is_absolute(const VStyle* style) {
  return vs_get_position(style) == V_POSITION_ABSOLUTE;
}

static inline bool v_is_in_flow(const VNode* node) {
  return v_node_is_visible(node) && node->popover_type == V_POPOVER_NONE &&
         vs_get_position(node->style) != V_POSITION_ABSOLUTE;
}

static VSize v__compute_text_pref_size(VNode* node, const VStyle* node_style) {
  if (v_string_is_empty(node->res_data.text) || !vs_has_font(node_style) ||
      !vs_has_font_size(node_style)) {
    return (VSize){0, 0};
  }

  VTextLayout* text_layout = v_node_get_text_layout(node);
  VTextModule* text_module = v_ctx_text_module();

  if (text_layout) {
    v_text_module_destroy_layout(text_module, text_layout);
  }

  uint16_t font_id =
      v_text_module_find_font_id_by_name(text_module, vs_get_font(node_style));

  if (!font_id) {
    return (VSize){0, 0};
  }

  text_layout = v_text_module_create_layout(
      text_module, font_id, v_style_resolve_font_size(node_style),
      node->res_data.text, V_TEXT_WRAP_NO_WRAP, vs_get_talign(node_style),
      0.0f);

  v_node_set_text_layout(node, text_layout);

  return v_text_layout_get_size(text_layout);
}

static VSize v__compute_image_pref_size(VNode* node) {
  return v_image_get_size(node->res.image_resource);
}

static void v_resolve_style(VNode* node, const VStyle* style) {
  node->margin.left = vs_has_prop(style, VS_MARGIN_LEFT)
                          ? v_style_resolve_length(&style->margin_left)
                          : 0.0f;
  node->margin.right = vs_has_prop(style, VS_MARGIN_RIGHT)
                           ? v_style_resolve_length(&style->margin_right)
                           : 0.0f;
  node->margin.top = vs_has_prop(style, VS_MARGIN_TOP)
                         ? v_style_resolve_length(&style->margin_top)
                         : 0.0f;
  node->margin.bottom = vs_has_prop(style, VS_MARGIN_BOTTOM)
                            ? v_style_resolve_length(&style->margin_bottom)
                            : 0.0f;

  node->padding.left = vs_has_prop(style, VS_PADDING_LEFT)
                           ? v_style_resolve_length(&style->padding_left)
                           : 0.0f;
  node->padding.right = vs_has_prop(style, VS_PADDING_RIGHT)
                            ? v_style_resolve_length(&style->padding_right)
                            : 0.0f;
  node->padding.bottom = vs_has_prop(style, VS_PADDING_BOTTOM)
                             ? v_style_resolve_length(&style->padding_bottom)
                             : 0.0f;
  node->padding.top = vs_has_prop(style, VS_PADDING_TOP)
                          ? v_style_resolve_length(&style->padding_top)
                          : 0.0f;

  // note: ceil border, as fractional border pixels are not supported
  node->border.left = vs_has_prop(style, VS_BORDER_LEFT)
                          ? v_style_resolve_length_ceil(&style->border_left)
                          : 0.0f;
  node->border.right = vs_has_prop(style, VS_BORDER_RIGHT)
                           ? v_style_resolve_length_ceil(&style->border_right)
                           : 0.0f;
  node->border.top = vs_has_prop(style, VS_BORDER_TOP)
                         ? v_style_resolve_length_ceil(&style->border_top)
                         : 0.0f;
  node->border.bottom = vs_has_prop(style, VS_BORDER_BOTTOM)
                            ? v_style_resolve_length_ceil(&style->border_bottom)
                            : 0.0f;
}

// Pass 1: Width Sizing (Bottom-up)
static void v_layout_pass1_width(VNode* node) {
  // note: first call is with root, which is always visible.

  if (!v_node_is_dirty(node)) {
    // this is a bottom-up pass. mark dirty propagates up. if this
    // node is clean, the descendants are clean as well and we can
    // skip the rest of this pass.
    return;
  }

  const VStyle* style = v_node_get_style_or_empty(node);

  v_resolve_style(node, style);

  v_foreach_child(node, child) {
    if (v_node_is_visible(child)) {
      v_layout_pass1_width(child);
    }
  }

  float min_w;
  float pref_w;

  if (node->tag == V_NODE_TEXT) {
    // TODO: for wrap, this might be an unnecessary measure.
    const VSize size = v__compute_text_pref_size(node, style);
    // round up to the nearest grid number so subsquent pass/measure will fit
    pref_w = ceilf(size.width);
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
      v_foreach_flow_child(node, child) {
        visible_children++;
        const float child_margin_w = child->margin.left + child->margin.right;
        // For wrap, min_width is the widest single child (worst case: one per
        // row). pref_width is still the sum (preferred: all on one row).
        if (wrap == V_WRAP_WRAP) {
          min_w = fmaxf(min_w, child->min_width + child_margin_w);
        } else {
          min_w += child->min_width + child_margin_w;
        }
        pref_w += child->pref_width + child_margin_w;
      }
      if (visible_children > 1) {
        const float total_gap =
            (float)(visible_children - 1) * v_style_resolve_gap(style);
        if (wrap != V_WRAP_WRAP) {
          min_w += total_gap;
        }
        pref_w += total_gap;
      }
    } else {
      if (wrap == V_WRAP_NONE) {
        // COLUMN: width is the maximum of children
        v_foreach_flow_child(node, child) {
          const float child_margin_w = child->margin.left + child->margin.right;
          min_w = fmaxf(min_w, child->min_width + child_margin_w);
          pref_w = fmaxf(pref_w, child->pref_width + child_margin_w);
        }
      } else {
        // COLUMN_WRAP: preferred width is the sum of column widths. Uses
        // children's pref_height from the previous layout to simulate column
        // breaks; converges after the first frame. If height is not FIXED the
        // column-break point is unknown, so falls back to single-column max.
        const float extra_h = v_node_inset_height(node);
        float fixed_width;
        const float col_inner_h =
            v_style_resolve_fixed_height(style, &fixed_width)
                ? fmaxf(0.0f, fixed_width - extra_h)
                : FLT_MAX;
        const float gap = v_style_resolve_gap(style);
        float current_col_h = 0.0f;
        float current_col_pref_w = 0.0f;

        v_foreach_flow_child(node, child) {
          const float child_margin_w = child->margin.left + child->margin.right;
          min_w = fmaxf(min_w, child->min_width + child_margin_w);
          if (current_col_h > 0.0f &&
              current_col_h + child->pref_height > col_inner_h) {
            pref_w += current_col_pref_w + gap;  // completed column + gap
            current_col_h = 0.0f;
            current_col_pref_w = 0.0f;
          }
          current_col_h += child->pref_height + gap;
          current_col_pref_w =
              fmaxf(current_col_pref_w, child->pref_width + child_margin_w);
        }
        pref_w += current_col_pref_w;  // last column (no trailing gap)
      }
    }
  }

  float ar = 0;
  if (vs_has_aspect_ratio(style)) {
    ar = v_style_resolve_aspect_ratio(style);
  } else if (node->tag == V_NODE_IMAGE) {
    // TODO: not sure about this. might need to support aspect-ratio = auto
    const VSize size = v__compute_image_pref_size(node);
    if (size.height > 0) {
      ar = size.width / size.height;
    }
  }

  if (ar > 0) {
    float fixed_height;
    if (v_style_resolve_fixed_height(style, &fixed_height)) {
      const float inner_h =
          fmaxf(0.0f, fixed_height - v_node_inset_height(node));
      pref_w = inner_h * ar;
    }
  }

  // Add padding and borders
  const float extra_w = v_node_inset_width(node);
  min_w += extra_w;
  pref_w += extra_w;

  // Resolve against style
  float fixed_width;

  if (v_style_resolve_fixed_width(style, &fixed_width)) {
    node->min_width = node->pref_width = fixed_width;
  } else {
    const float style_min_width = v_style_resolve_min_width(style);
    const float style_max_width = v_style_resolve_max_width(style);

    node->min_width = fmaxf(style_min_width, min_w);
    node->pref_width = fmaxf(style_min_width, pref_w);

    if (style_max_width > 0) {
      node->min_width = fminf(node->min_width, style_max_width);
      node->pref_width = fminf(node->pref_width, style_max_width);
    }
  }
}

// Pass 2: Width Distribution (Top-down)
static void v_layout_pass2_width_dist(VNode* node,
                                      float width,
                                      float root_width) {
  // note: first call is with root, which is always visible.

  if (!v_node_is_dirty(node)) {
    // on float compare: node->bounds is set in a previous layout pass.
    // despite floats lacking precision, the layout computation is
    // deterministic. if the new computation EXACTLY matches the previous
    // one, then we conclude the dimension has not changed.
    if (node->bounds.width == width) {
      // if this node is not dirty and its dimension has not changed, its
      // children do not need layout and we can skip the rest of this pass.
      return;
    }

    // the node is not dirty but its dimension changed. pass5 will need
    // to run on this node to ensure positioning is correct. mark just
    // this node dirty. the dirty is not propagated up the tree because
    // all scenarios that bring us here mean the parent is dirty.
    //
    // TODO: we should assert parent is dirty here.
    v_node_set_flag(node, V_NODEFLAG_DIRTY);
  }

  node->bounds.width = width;

  int visible_children = 0;
  float fixed_w = 0;
  int grow_count = 0;

  // 1: visible popovers need a pass2 for their children
  // 2: absolute children get their width resolved out-of-flow
  // 3: count visible, non-popover, non-absolute children; tally ROW grow/fixed
  const VStyle* style = v_node_get_style_or_empty(node);
  const float inner_w = width - v_node_inset_width(node);
  const VDirection dir = vs_get_direction(style);
  const float gap = v_style_resolve_gap(style);

  v_foreach_child(node, child) {
    if (v_node_is_visible(child)) {
      const VStyle* child_style = v_node_get_style_or_empty(child);
      const VSizeMode wtag = v_style_resolve_width_size_mode(child_style);
      float cw;

      if (child->popover_type != V_POPOVER_NONE) {
        if (wtag == V_SIZE_MODE_GROW) {
          cw = (vs_get_anchor_to(child_style) == V_ANCHOR_TO_ROOT) ? root_width
                                                                   : width;
        } else {
          cw = child->pref_width;
        }
        v_layout_pass2_width_dist(child, cw, root_width);
      } else if (v_style__is_absolute(child_style)) {
        if (wtag == V_SIZE_MODE_GROW) {
          cw = inner_w;
        } else if (wtag == V_SIZE_MODE_FIT && vs_has_left(child_style) &&
                   vs_has_right(child_style)) {
          cw = fmaxf(0.0f, inner_w - v_style_resolve_left(child_style) -
                               v_style_resolve_right(child_style));
        } else {
          cw = child->pref_width;
        }
        v_layout_pass2_width_dist(child, cw, root_width);
      } else {
        visible_children++;
        if (dir == V_DIRECTION_ROW) {
          if (wtag == V_SIZE_MODE_GROW) {
            grow_count++;
          } else {
            fixed_w +=
                child->pref_width + child->margin.left + child->margin.right;
          }
        }
      }
    }
  }

  if (visible_children == 0) {
    return;
  }

  if (dir == V_DIRECTION_ROW) {
    fixed_w += (visible_children - 1) * gap;

    // Multi-round distribution: clamp GROW children that can't receive their
    // minimum width, then redistribute the remaining space. Converges in at
    // most grow_count rounds; per_grow is monotonically non-increasing.
    int unclamped = grow_count;
    float clamped_total = 0;
    for (int round = 0; round < grow_count; round++) {
      if (unclamped == 0) {
        break;
      }
      const float per_grow =
          fmaxf(0.0f, (inner_w - fixed_w - clamped_total) / unclamped);
      int new_unclamped = 0;
      float new_clamped = 0;
      v_foreach_flow_child(node, child) {
        const VStyle* child_style = v_node_get_style_or_empty(child);
        if (v_style_resolve_width_size_mode(child_style) != V_SIZE_MODE_GROW) {
          continue;
        }
        const float child_margin_w = child->margin.left + child->margin.right;
        if (child->min_width + child_margin_w > per_grow) {
          new_clamped += child->min_width + child_margin_w;
        } else {
          new_unclamped++;
        }
      }
      if (new_unclamped == unclamped) {
        break;
      }
      unclamped = new_unclamped;
      clamped_total = new_clamped;
    }

    const float grow_w =
        (unclamped > 0)
            ? fmaxf(0.0f, (inner_w - fixed_w - clamped_total) / unclamped)
            : 0.0f;

    v_foreach_flow_child(node, child) {
      const VStyle* child_style = v_node_get_style_or_empty(child);
      float cw;
      if (v_style_resolve_width_size_mode(child_style) == V_SIZE_MODE_GROW) {
        const float child_margin_w = child->margin.left + child->margin.right;
        cw = fmaxf(child->min_width, grow_w - child_margin_w);
      } else {
        cw = child->pref_width;
      }
      v_layout_pass2_width_dist(child, cw, root_width);
    }
  } else {
    // COLUMN
    v_foreach_flow_child(node, child) {
      const VStyle* child_style = v_node_get_style_or_empty(child);
      float cw;
      if (v_style_resolve_width_size_mode(child_style) == V_SIZE_MODE_GROW) {
        cw = fmaxf(0.0f, inner_w - child->margin.left - child->margin.right);
      } else {
        cw = child->pref_width;
      }
      v_layout_pass2_width_dist(child, cw, root_width);
    }
  }
}

// Pass 3: Height Sizing (Bottom-up)
static void v_layout_pass3_height(VNode* node) {
  // note: first call is with root, which is always visible.

  if (!v_node_is_dirty(node)) {
    // this is a bottom-up pass. mark dirty propagates up. if this
    // node is clean, the descendants are clean as well and we can
    // skip the rest of this pass.
    return;
  }

  v_foreach_child(node, child) {
    if (v_node_is_visible(child)) {
      v_layout_pass3_height(child);
    }
  }

  const VStyle* style = v_node_get_style_or_empty(node);
  float min_h = 0;
  float pref_h = 0;

  if (node->tag == V_NODE_TEXT) {
    const VStyle* style_for_text = v_node_get_style_or_empty(node);

    if (!v_string_is_empty(node->res_data.text) &&
        vs_has_font(style_for_text) && vs_has_font_size(style_for_text)) {
      // TODO: for now, use the snapped width to measure
      const float inner_w =
          v_snap_to_grid_dpr(node->bounds.width - v_node_inset_width(node));

      if (vs_get_text_wrap(style_for_text) == V_TEXT_WRAP_WRAP) {
        if (inner_w > 0) {
          VTextModule* text_module = v_ctx_text_module();
          VTextLayout* text_layout = v_node_get_text_layout(node);
          if (text_layout) {
            v_text_module_destroy_layout(text_module, text_layout);
          }

          // TODO: this should be resolved
          uint16_t font_id = v_text_module_find_font_id_by_name(
              text_module, vs_get_font(style_for_text));

          text_layout = v_text_module_create_layout(
              text_module, font_id, v_style_resolve_font_size(style_for_text),
              node->res_data.text, V_TEXT_WRAP_WRAP,
              vs_get_talign(style_for_text), inner_w);

          if (text_layout) {
            VSize size = v_text_layout_get_size(text_layout);
            pref_h = min_h = size.height;
          }

          v_node_set_text_layout(node, text_layout);
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
      v_foreach_flow_child(node, child) {
        visible_children++;
        const float child_margin_h = child->margin.top + child->margin.bottom;
        cross_pref = fmaxf(cross_pref, child->pref_height + child_margin_h);
        cross_min = fmaxf(cross_min, child->min_height + child_margin_h);
      }
      min_h = cross_min;
      pref_h = cross_pref;
    } else if (dir == V_DIRECTION_COLUMN && wrap == V_WRAP_NONE) {
      v_foreach_flow_child(node, child) {
        visible_children++;
        const float child_margin_h = child->margin.top + child->margin.bottom;
        main_min += child->min_height + child_margin_h;
        main_pref += child->pref_height + child_margin_h;
      }
      if (visible_children > 1) {
        const float total_gap =
            (float)(visible_children - 1) * v_style_resolve_gap(style);
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
      const float inner_w = node->bounds.width - v_node_inset_width(node);
      const float gap = v_style_resolve_gap(style);

      v_foreach_flow_child(node, child) {
        const float child_total_w =
            child->bounds.width + child->margin.left + child->margin.right;
        const float child_total_h =
            child->pref_height + child->margin.top + child->margin.bottom;
        if (current_row_w > 0 && current_row_w + child_total_w > inner_w) {
          total_h += current_row_h + gap;
          current_row_w = 0;
          current_row_h = 0;
        }
        current_row_w += child_total_w + gap;
        current_row_h = fmaxf(current_row_h, child_total_h);
      }
      total_h += current_row_h;
      min_h = pref_h = total_h;
    } else if (dir == V_DIRECTION_COLUMN && wrap == V_WRAP_WRAP) {
      const float extra_h = v_node_inset_height(node);
      // Column-wrap needs a height constraint to know when to break into a new
      // column. For FIXED height that constraint is known; for GROW/FIT it
      // isn't available until Pass 5, so fall back to a single column.
      float fixed_height;
      const float inner_h = v_style_resolve_fixed_height(style, &fixed_height)
                                ? fmaxf(0.0f, fixed_height - extra_h)
                                : FLT_MAX;
      const float gap = v_style_resolve_gap(style);
      float current_col_h = 0;

      v_foreach_flow_child(node, child) {
        if (current_col_h > 0 && current_col_h + child->pref_height > inner_h) {
          current_col_h = 0;
        }
        current_col_h += child->pref_height + gap;
      }
      pref_h = min_h = current_col_h;
    }
  }

  float ar = 0;
  if (vs_has_aspect_ratio(style)) {
    ar = v_style_resolve_aspect_ratio(style);
  } else if (node->tag == V_NODE_IMAGE) {
    const VSize size = v__compute_image_pref_size(node);
    if (size.width > 0) {
      ar = (float)size.width / (float)size.height;
    }
  }

  if (ar > 0) {
    const float inner_w = node->bounds.width - v_node_inset_width(node);
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
  const float extra_h = v_node_inset_height(node);
  min_h += extra_h;
  pref_h += extra_h;

  // Resolve against style
  float fixed_height;
  if (v_style_resolve_fixed_height(style, &fixed_height)) {
    node->min_height = node->pref_height = fixed_height;
  } else {
    const float style_min_height = v_style_resolve_min_height(style);
    const float style_max_height = v_style_resolve_max_height(style);

    node->min_height = fmaxf(style_min_height, min_h);
    node->pref_height = fmaxf(style_min_height, pref_h);

    if (style_max_height > 0) {
      node->min_height = fminf(node->min_height, style_max_height);
      node->pref_height = fminf(node->pref_height, style_max_height);
    }
  }
}

static void v_layout_pass4_height_dist(VNode* node,
                                       float height);  // forward decl

// Assign row_h to each flow child in [start, end) along the sibling list.
// Pass end=NULL to flush to the end of the list (last row).
static void v__flush_row(VNode* start, VNode* end, float row_h) {
  for (VNode* c = start; c != NULL && c != end; c = v_node_next_sibling(c)) {
    if (!v_is_in_flow(c)) {
      continue;
    }
    const VStyle* cs = v_node_get_style_or_empty(c);
    const float ch = (v_style_resolve_height_size_mode(cs) == V_SIZE_MODE_GROW)
                         ? row_h
                         : c->pref_height;
    v_layout_pass4_height_dist(c, ch);
  }
}

// Pass 4: Height Distribution (Top-down)
static void v_layout_pass4_height_dist(VNode* node, float height) {
  // note: first call is with root, which is always visible.

  if (!v_node_is_dirty(node)) {
    // on float compare: node->bounds is set in a previous layout pass.
    // despite floats lacking precision, the layout computation is
    // deterministic. if the new computation EXACTLY matches the previous
    // one, then we conclude the dimension has not changed.
    if (node->bounds.height == height) {
      // if this node is not dirty and its dimension has not changed, its
      // children do not need layout and we can skip the rest of this pass.
      return;
    }

    // the node is not dirty but its dimension changed. pass5 will need
    // to run on this node to ensure positioning is correct. mark just
    // this node dirty. the dirty is not propagated up the tree because
    // all scenarios that bring us here mean the parent is dirty.
    //
    // TODO: we should assert parent is dirty here.
    v_node_set_flag(node, V_NODEFLAG_DIRTY);
  }

  node->bounds.height = height;

  int visible_children = 0;
  float fixed_h = 0;
  int grow_count = 0;

  // 1: visible popovers need a pass4 for their children
  // 2: absolute children get their height resolved out-of-flow
  // 3: count visible, non-popover, non-absolute children; tally COLUMN
  // grow/fixed
  const VStyle* style = v_node_get_style_or_empty(node);
  const float inner_h = height - v_node_inset_height(node);
  const VDirection dir = vs_get_direction(style);
  const VWrap wrap = vs_get_wrap(style);
  const float gap = v_style_resolve_gap(style);

  v_foreach_child(node, child) {
    if (v_node_is_visible(child)) {
      float ch;
      const VStyle* child_style = v_node_get_style_or_empty(child);
      const VSizeMode htag = v_style_resolve_height_size_mode(child_style);

      if (child->popover_type != V_POPOVER_NONE) {
        if (htag == V_SIZE_MODE_GROW) {
          ch = (vs_get_anchor_to(child_style) == V_ANCHOR_TO_ROOT)
                   ? v_root()->bounds.height
                   : height;
        } else {
          ch = child->pref_height;
        }
        v_layout_pass4_height_dist(child, ch);
      } else if (v_style__is_absolute(child_style)) {
        // TODO: has top && has bottom seems wrong
        if (htag == V_SIZE_MODE_GROW) {
          ch = inner_h;
        } else if (htag == V_SIZE_MODE_FIT && vs_has_top(child_style) &&
                   vs_has_bottom(child_style)) {
          ch = fmaxf(0.0f, inner_h - v_style_resolve_top(child_style) -
                               v_style_resolve_bottom(child_style));
        } else {
          ch = child->pref_height;
        }
        v_layout_pass4_height_dist(child, ch);
      } else {
        visible_children++;
        if (dir == V_DIRECTION_COLUMN && wrap == V_WRAP_NONE) {
          if (htag == V_SIZE_MODE_GROW && !vs_has_aspect_ratio(child_style)) {
            grow_count++;
          } else {
            fixed_h +=
                child->pref_height + child->margin.top + child->margin.bottom;
          }
        }
      }
    }
  }

  if (visible_children == 0) {
    return;
  }

  if (dir == V_DIRECTION_COLUMN && wrap == V_WRAP_NONE) {
    fixed_h += (visible_children - 1) * gap;

    int unclamped = grow_count;
    float clamped_total = 0;
    for (int round = 0; round < grow_count; round++) {
      if (unclamped == 0) {
        break;
      }
      const float per_grow =
          fmaxf(0.0f, (inner_h - fixed_h - clamped_total) / unclamped);
      int new_unclamped = 0;
      float new_clamped = 0;
      v_foreach_flow_child(node, child) {
        const VStyle* child_style = v_node_get_style_or_empty(child);
        if (v_style_resolve_height_size_mode(child_style) != V_SIZE_MODE_GROW) {
          continue;
        }
        if (vs_has_aspect_ratio(child_style)) {
          continue;
        }
        const float child_margin_h = child->margin.top + child->margin.bottom;
        if (child->min_height + child_margin_h > per_grow) {
          new_clamped += child->min_height + child_margin_h;
        } else {
          new_unclamped++;
        }
      }
      if (new_unclamped == unclamped) {
        break;
      }
      unclamped = new_unclamped;
      clamped_total = new_clamped;
    }

    const float grow_h =
        (unclamped > 0)
            ? fmaxf(0.0f, (inner_h - fixed_h - clamped_total) / unclamped)
            : 0.0f;

    v_foreach_flow_child(node, child) {
      const VStyle* child_style = v_node_get_style_or_empty(child);
      float ch;
      if (v_style_resolve_height_size_mode(child_style) == V_SIZE_MODE_GROW) {
        const float child_margin_h = child->margin.top + child->margin.bottom;
        ch = vs_has_aspect_ratio(child_style)
                 ? child->pref_height
                 : fmaxf(child->min_height, grow_h - child_margin_h);
      } else {
        ch = child->pref_height;
      }
      v_layout_pass4_height_dist(child, ch);
    }
  } else if (dir == V_DIRECTION_ROW && wrap == V_WRAP_NONE) {
    v_foreach_flow_child(node, child) {
      const VStyle* child_style = v_node_get_style_or_empty(child);
      float ch;
      if (v_style_resolve_height_size_mode(child_style) == V_SIZE_MODE_GROW) {
        const float child_margin_h = child->margin.top + child->margin.bottom;
        ch = vs_has_aspect_ratio(child_style)
                 ? child->pref_height
                 : fmaxf(0.0f, inner_h - child_margin_h);
      } else {
        ch = child->pref_height;
      }
      v_layout_pass4_height_dist(child, ch);
    }
  } else if (dir == V_DIRECTION_COLUMN && wrap == V_WRAP_WRAP) {
    v_foreach_flow_child(node, child) {
      v_layout_pass4_height_dist(child, child->pref_height);
    }
  } else {
    // ROW_WRAP: assign each child the height of the tallest child in its row.
    const float inner_w = node->bounds.width - v_node_inset_width(node);
    VNode* row_start = NULL;
    float current_row_w = 0;
    float current_row_h = 0;

    v_foreach_flow_child(node, child) {
      const float child_total_w =
          child->bounds.width + child->margin.left + child->margin.right;
      if (current_row_w > 0 && current_row_w + child_total_w > inner_w) {
        v__flush_row(row_start, child, current_row_h);
        row_start = NULL;
        current_row_w = 0;
        current_row_h = 0;
      }
      if (!row_start) {
        row_start = child;
      }
      current_row_w += child_total_w + gap;
      current_row_h = fmaxf(current_row_h, child->pref_height);
    }

    v__flush_row(row_start, NULL, current_row_h);
  }
}

// x offset of relative-positioned nodes
static float v__get_relative_offset_x(const VStyle* child_style) {
  // can't use both left and right. resolve by favoring left.
  if (vs_has_left(child_style)) {
    return v_style_resolve_left(child_style);
  } else if (vs_has_right(child_style)) {
    return -v_style_resolve_right(child_style);
  } else {
    return 0;
  }
}

// y offset of relative-positioned nodes
static float v__get_relative_offset_y(const VStyle* child_style) {
  // can't use both top and bottom. resolve by favoring top.
  if (vs_has_top(child_style)) {
    return v_style_resolve_top(child_style);
  } else if (vs_has_bottom(child_style)) {
    return -v_style_resolve_bottom(child_style);
  } else {
    return 0;
  }
}

// Pass 5: Positioning & Alignment (Top-down)
static void v_layout_pass5_pos(VNode* node, float x, float y) {
  const float sx = v_snap_to_grid_dpr(x);
  const float sy = v_snap_to_grid_dpr(y);

  if (!v_node_is_dirty(node) && sx == node->bounds.x && sy == node->bounds.y) {
    // no reason to process children if this node is not dirty and it has not
    // moved. cpu gets a vacation.
    return;
  }

  node->bounds.x = sx;
  node->bounds.y = sy;

  v_node_clear_flag(node, V_NODEFLAG_DIRTY);

  const VStyle* style = v_node_get_style_or_empty(node);
  const VDirection dir = vs_get_direction(style);
  const VWrap wrap = vs_get_wrap(style);
  const float gap = v_style_resolve_gap(style);
  const float origin_x = v_node_inset_left(node);
  const float origin_y = v_node_inset_top(node);
  const float inner_w = node->bounds.width - v_node_inset_width(node);
  const float inner_h = node->bounds.height - v_node_inset_height(node);

  float cur_x = origin_x;
  float cur_y = origin_y;

  if (dir == V_DIRECTION_ROW && wrap == V_WRAP_NONE) {
    // Alignment along main axis (X): only flow children count.
    // Auto margins on the main axis absorb free space before xalign is applied.
    float total_w = 0;
    int visible_count = 0;
    int auto_margin_slots_x = 0;
    v_foreach_flow_child(node, child) {
      const VStyle* child_style = v_node_get_style_or_empty(child);
      if (v_style_margin_is_auto_left(child_style)) {
        auto_margin_slots_x++;
      } else {
        total_w += child->margin.left;
      }
      if (v_style_margin_is_auto_right(child_style)) {
        auto_margin_slots_x++;
      } else {
        total_w += child->margin.right;
      }
      total_w += child->bounds.width;
      visible_count++;
    }
    if (visible_count > 1) {
      total_w += (visible_count - 1) * gap;
    }
    const float free_w = fmaxf(0.0f, inner_w - total_w);
    const float auto_w =
        (auto_margin_slots_x > 0) ? free_w / auto_margin_slots_x : 0.0f;

    if (auto_margin_slots_x == 0) {
      const VAlignX ax = vs_get_xalign(style);
      if (ax == V_ALIGN_X_CENTER) {
        cur_x += fmaxf(0, (inner_w - total_w) / 2.0f);
      } else if (ax == V_ALIGN_X_RIGHT) {
        cur_x += fmaxf(0, inner_w - total_w);
      }
    }

    v_foreach_child(node, child) {
      if (!v_node_is_visible(child)) {
        continue;
      }
      if (child->popover_type != V_POPOVER_NONE) {
        v_layout_pass5_pos(child, 0, 0);
        continue;
      }

      const VStyle* child_style = v_node_get_style_or_empty(child);
      const VPosition position = vs_get_position(child_style);

      if (position == V_POSITION_ABSOLUTE) {
        continue;
      }

      // Main axis: resolve auto left/right margins.
      const float eff_margin_left = v_style_margin_is_auto_left(child_style)
                                        ? auto_w
                                        : child->margin.left;
      const float eff_margin_right = v_style_margin_is_auto_right(child_style)
                                         ? auto_w
                                         : child->margin.right;
      // Cross axis: auto top/bottom margins override parent yalign per-child.
      float child_y;
      const bool auto_top_y = v_style_margin_is_auto_top(child_style);
      const bool auto_bot_y = v_style_margin_is_auto_bottom(child_style);
      if (auto_top_y || auto_bot_y) {
        const float fixed_cross = child->bounds.height +
                                  (auto_top_y ? 0.0f : child->margin.top) +
                                  (auto_bot_y ? 0.0f : child->margin.bottom);
        const float free_cross = fmaxf(0.0f, inner_h - fixed_cross);
        child_y =
            cur_y + (auto_top_y ? (auto_bot_y ? free_cross / 2.0f : free_cross)
                                : child->margin.top);
      } else {
        child_y = cur_y + child->margin.top;
        const VAlignY ay = vs_get_yalign(style);
        if (ay == V_ALIGN_Y_CENTER) {
          child_y += fmaxf(0, (inner_h - child->bounds.height) / 2.0f);
        } else if (ay == V_ALIGN_Y_BOTTOM) {
          child_y += fmaxf(0, inner_h - child->bounds.height);
        }
      }
      float child_x = cur_x + eff_margin_left;
      if (position == V_POSITION_RELATIVE) {
        child_x += v__get_relative_offset_x(child_style);
        child_y += v__get_relative_offset_y(child_style);
      }
      v_layout_pass5_pos(child, child_x, child_y);
      cur_x += eff_margin_left + child->bounds.width + eff_margin_right + gap;
    }
  } else if (dir == V_DIRECTION_COLUMN && wrap == V_WRAP_NONE) {
    // Alignment along main axis (Y): only flow children count.
    // Auto margins on the main axis absorb free space before yalign is applied.
    float total_h = 0;
    int visible_count = 0;
    int auto_margin_slots_y = 0;
    v_foreach_flow_child(node, child) {
      const VStyle* child_style = v_node_get_style_or_empty(child);
      if (v_style_margin_is_auto_top(child_style)) {
        auto_margin_slots_y++;
      } else {
        total_h += child->margin.top;
      }
      if (v_style_margin_is_auto_bottom(child_style)) {
        auto_margin_slots_y++;
      } else {
        total_h += child->margin.bottom;
      }
      total_h += child->bounds.height;
      visible_count++;
    }
    if (visible_count > 1) {
      total_h += (visible_count - 1) * gap;
    }
    const float free_h = fmaxf(0.0f, inner_h - total_h);
    const float auto_h =
        (auto_margin_slots_y > 0) ? free_h / auto_margin_slots_y : 0.0f;

    if (auto_margin_slots_y == 0) {
      const VAlignY ay = vs_get_yalign(style);
      if (ay == V_ALIGN_Y_CENTER) {
        cur_y += fmaxf(0, (inner_h - total_h) / 2.0f);
      } else if (ay == V_ALIGN_Y_BOTTOM) {
        cur_y += fmaxf(0, inner_h - total_h);
      }
    }

    v_foreach_child(node, child) {
      if (!v_node_is_visible(child)) {
        continue;
      }
      if (child->popover_type != V_POPOVER_NONE) {
        v_layout_pass5_pos(child, 0, 0);
        continue;
      }

      const VStyle* child_style = v_node_get_style_or_empty(child);
      const VPosition position = vs_get_position(child_style);

      if (position == V_POSITION_ABSOLUTE) {
        continue;
      }
      // Main axis: resolve auto top/bottom margins.
      const float eff_margin_top =
          v_style_margin_is_auto_top(child_style) ? auto_h : child->margin.top;
      const float eff_margin_bottom = v_style_margin_is_auto_bottom(child_style)
                                          ? auto_h
                                          : child->margin.bottom;
      // Cross axis: auto left/right margins override parent xalign per-child.
      float child_x;
      const bool auto_left_x = v_style_margin_is_auto_left(child_style);
      const bool auto_right_x = v_style_margin_is_auto_right(child_style);
      if (auto_left_x || auto_right_x) {
        const float fixed_cross = child->bounds.width +
                                  (auto_left_x ? 0.0f : child->margin.left) +
                                  (auto_right_x ? 0.0f : child->margin.right);
        const float free_cross = fmaxf(0.0f, inner_w - fixed_cross);
        child_x = cur_x + (auto_left_x
                               ? (auto_right_x ? free_cross / 2.0f : free_cross)
                               : child->margin.left);
      } else {
        child_x = cur_x + child->margin.left;
        const VAlignX ax = vs_get_xalign(style);
        if (ax == V_ALIGN_X_CENTER) {
          child_x += fmaxf(0, (inner_w - child->bounds.width) / 2.0f);
        } else if (ax == V_ALIGN_X_RIGHT) {
          child_x += fmaxf(0, inner_w - child->bounds.width);
        }
      }
      float child_y = cur_y + eff_margin_top;
      if (position == V_POSITION_RELATIVE) {
        child_x += v__get_relative_offset_x(child_style);
        child_y += v__get_relative_offset_y(child_style);
      }
      v_layout_pass5_pos(child, child_x, child_y);
      cur_y += eff_margin_top + child->bounds.height + eff_margin_bottom + gap;
    }
  } else if (dir == V_DIRECTION_ROW && wrap == V_WRAP_WRAP) {
    float row_h = 0;

    v_foreach_child(node, child) {
      if (!v_node_is_visible(child)) {
        continue;
      }
      if (child->popover_type != V_POPOVER_NONE) {
        v_layout_pass5_pos(child, 0, 0);
        continue;
      }

      const VStyle* child_style = v_node_get_style_or_empty(child);
      const VPosition position = vs_get_position(child_style);

      if (position == V_POSITION_ABSOLUTE) {
        continue;
      }

      const float child_total_w =
          child->margin.left + child->bounds.width + child->margin.right;
      if (cur_x > origin_x && cur_x + child_total_w > origin_x + inner_w) {
        cur_y += row_h + gap;
        cur_x = origin_x;
        row_h = 0;
      }
      float child_x = cur_x + child->margin.left;
      float child_y = cur_y + child->margin.top;
      if (position == V_POSITION_RELATIVE) {
        child_x += v__get_relative_offset_x(child_style);
        child_y += v__get_relative_offset_y(child_style);
      }
      v_layout_pass5_pos(child, child_x, child_y);
      cur_x +=
          child->margin.left + child->bounds.width + child->margin.right + gap;
      row_h = fmaxf(row_h, child->bounds.height + child->margin.top +
                               child->margin.bottom);
    }
  } else if (dir == V_DIRECTION_COLUMN && wrap == V_WRAP_WRAP) {
    float col_w = 0;

    v_foreach_child(node, child) {
      if (!v_node_is_visible(child)) {
        continue;
      }
      if (child->popover_type != V_POPOVER_NONE) {
        v_layout_pass5_pos(child, 0, 0);
        continue;
      }

      const VStyle* child_style = v_node_get_style_or_empty(child);
      const VPosition position = vs_get_position(child_style);

      if (position == V_POSITION_ABSOLUTE) {
        continue;
      }

      const float child_total_h =
          child->margin.top + child->bounds.height + child->margin.bottom;
      if (cur_y > origin_y && cur_y + child_total_h > origin_y + inner_h) {
        cur_x += col_w + gap;
        cur_y = origin_y;
        col_w = 0;
      }
      float child_x = cur_x + child->margin.left;
      float child_y = cur_y + child->margin.top;
      if (position == V_POSITION_RELATIVE) {
        child_x += v__get_relative_offset_x(child_style);
        child_y += v__get_relative_offset_y(child_style);
      }
      v_layout_pass5_pos(child, child_x, child_y);
      cur_y +=
          child->margin.top + child->bounds.height + child->margin.bottom + gap;
      col_w = fmaxf(col_w, child->bounds.width + child->margin.left +
                               child->margin.right);
    }
  }

  // Position absolute children relative to this node's content origin.
  v_foreach_child(node, child) {
    if (!v_node_is_visible(child)) {
      continue;
    }
    const VStyle* child_style = v_node_get_style_or_empty(child);
    if (!v_style__is_absolute(child_style)) {
      continue;
    }
    float child_x, child_y;
    if (vs_has_left(child_style)) {
      child_x =
          origin_x + v_style_resolve_left(child_style) + child->margin.left;
    } else if (vs_has_right(child_style)) {
      child_x = origin_x + inner_w - v_style_resolve_right(child_style) -
                child->bounds.width - child->margin.right;
    } else {
      child_x = origin_x + child->margin.left;
    }
    if (vs_has_top(child_style)) {
      child_y = origin_y + v_style_resolve_top(child_style) + child->margin.top;
    } else if (vs_has_bottom(child_style)) {
      child_y = origin_y + inner_h - v_style_resolve_bottom(child_style) -
                child->bounds.height - child->margin.bottom;
    } else {
      child_y = origin_y + child->margin.top;
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
    if (!v_node_is_visible(child)) {
      continue;
    }

    if (child->popover_type != V_POPOVER_NONE) {
      const VStyle* child_style = v_node_get_style_or_empty(child);
      const VAnchorTo attach = vs_get_anchor_to(child_style);
      VRect anchor_rect;

      if (attach == V_ANCHOR_TO_ROOT) {
        anchor_rect = (VRect){
            0,
            0,
            root_w,
            root_h,
        };
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
          ax - px + v_style_resolve_attach_point_offset_x(child_style));
      child->bounds.y = v_snap_to_grid_dpr(
          ay - py + v_style_resolve_attach_point_offset_y(child_style));
    }

    v_layout_pass6_popovers(child, current_abs_x,
                            current_abs_y - node->scroll_y, root_w, root_h);
  }
}

void v_node_layout(VNode* self, VNodeModule* mod, int width, int height) {
  width = v_clamp_int(width, 0, VUID_FLOAT_MAX_INT);
  height = v_clamp_int(height, 0, VUID_FLOAT_MAX_INT);

  const float width_f = (float)width;
  const float height_f = (float)height;

  VStyle* style = v_node_style(self);

  if (width != mod->root_width || height != mod->root_height) {
    vs_set_width(style, v_px(width_f));
    mod->root_width = width;
    vs_set_height(style, v_px(height_f));
    mod->root_height = height;
    v_node_mark_dirty(self);
  } else if (!v_node_has_flag(self, V_NODEFLAG_DIRTY)) {
    return;
  }

  v_layout_pass1_width(self);
  v_layout_pass2_width_dist(self, width_f, width_f);
  v_layout_pass3_height(self);
  v_layout_pass4_height_dist(self, height_f);
  v_layout_pass5_pos(self, 0, 0);
  v_layout_pass6_popovers(self, 0, 0, width_f, height_f);

  v_ctx_request_render();
}





VUID_PKG bool v_node_module_init(VNodeModule* self, VAllocator* allocator) {
  *self = (VNodeModule){
      .allocator = allocator,
      .nodes_by_id = v_node_id_set_init(allocator),
      .popover_stack = v_array_init(allocator, sizeof(VNode*), 8),
      .event_path = v_array_init(allocator, sizeof(VNode*), 32),
      .style_sheet = v_style_class_hmap_init(allocator),
  };

  return true;
}

VUID_PKG bool v_node_module_post_init(VNodeModule* self) {
  self->empty_style = v_style_new(NULL);

  if (self->empty_style == NULL) {
    v_node_module_drop(self);
    return false;
  }

  self->root = v_node_constructor(V_NODE_ROOT);

  if (self->root == NULL) {
    v_node_module_drop(self);
    return false;
  }

  v_node_set_attached(self->root, true);

  self->focused_node = self->root;

  return true;
}

VUID_PKG void v_node_module_drop(VNodeModule* self) {
  if (self->empty_style) {
    v_style_unref(self->empty_style);
  }

  if (self->root) {
    v_node_remove_children(self->root);
    v_node_set_attached(self->root, false);
    v_node_unref(self->root);
  }

  VUID_ASSERT(self->nodes_by_id.size == 0);
  VUID_ASSERT(self->popover_stack.size == 0);
  VUID_ASSERT(self->event_path.size == 0);
  VUID_ASSERT(self->under_mouse_node == NULL);
  VUID_ASSERT(self->hovered_node == NULL);
  VUID_ASSERT(self->drag_node == NULL);

  v_style_class_hmap_drop(&self->style_sheet);
  v_node_id_set_drop(&self->nodes_by_id);
  v_array_drop(&self->popover_stack);
  v_array_drop(&self->event_path);
}

VUID_PKG VArray* v_node_module_get_popover_stack(void) {
  return &v_ctx_node_module()->popover_stack;
}

VUID_PKG void v_node_module_add_node_id(VNode* node) {
  v_node_id_set_put(&v_ctx_node_module()->nodes_by_id,
                    VNodeIdSetValue_init(node));
}

VUID_PKG void v_node_module_remove_node_id(VNode* node) {
  const VNodeIdSetValue value = VNodeIdSetValue_init(node);
  v_node_id_set_remove_by_value(&v_ctx_node_module()->nodes_by_id, &value);
}

VUID_PKG void v_node_module_remove_popover_node(VNode* node) {
  VArray* popover_stack = &v_ctx_node_module()->popover_stack;

  for (size_t i = 0; i < popover_stack->size; i++) {
    if (v_array_get_ptr_unchecked(popover_stack, i) == node) {
      v_array_remove(popover_stack, i);
      break;
    }
  }
}

VUID_PKG void v_node_module_remove_input_node(VNode* node) {
  VNodeModule* mod = v_ctx_node_module();

  if (mod->hovered_node == node) {
    mod->hovered_node = NULL;
  }

  if (mod->under_mouse_node == node) {
    mod->under_mouse_node = NULL;
  }

  if (mod->drag_node == node) {
    mod->drag_node = NULL;
  }

  if (mod->focused_node == node) {
    v_node_blur(node);
  }
}

VUID_PKG size_t v_event_path_start(VArray* event_path) {
  return event_path->size;
}

VUID_PKG void v_event_path_reset(VArray* event_path, size_t start_index) {
  while (event_path->size > start_index) {
    v_node_unref(v_array_pop_ptr(event_path));
  }
}

VUID_PKG void v_event_path_push_ref(VArray* event_path,
                                    VNodeEventType type,
                                    VNode* node) {
  if (node->event_listeners[type]) {
    v_node_ref(node);
    v_array_push(event_path, &node);
  }
}

VUID_PKG void v_event_path_dispatch(VArray* event_path,
                                    VNodeEvent* event,
                                    size_t start_index,
                                    size_t end_index) {
  VNodeEventType type = event->type;

  for (size_t i = start_index; i < end_index; i++) {
    VNode* curr = v_array_get_ptr_unchecked(event_path, i);
    if (curr->event_listeners[type]) {
      curr->event_listeners[type](curr, event);
    }
    if (v_node_event_was_cancelled(event)) {
      break;
    }
  }
}

static uint32_t VNodeIdSetValue_get_hash(const VNodeIdSetValue* value) {
  return value->node->id.hash;
}

static uint32_t VNodeIdSetKey_get_hash(const VNodeIdSetKey* key) {
  return v_fnv1_hash(key->id);
}

static VNodeIdSetKey VNodeIdSetKey_from_value(const VNodeIdSetValue* value) {
  return (VNodeIdSetKey){.id = value->node->id.str};
}

static bool VNodeIdSetValue_eq(const VNodeIdSetKey* key,
                               const VNodeIdSetValue* value) {
  return strcmp(key->id, value->node->id.str) == 0;
}

// nodes are unowned, so we don't drop them when removing from the set
#define VNodeIdSetValue_drop(VALUE)

VUID_HMAP_IMPL(VNodeIdSet, VNodeIdSetKey, VNodeIdSetValue, v_node_id_set)




static VNode* v_node_hit_test_recursive(VNode* node,
                                        float abs_x,
                                        float abs_y,
                                        float mx,
                                        float my,
                                        VRect clip);

VUID_PKG void v_node_on_mouse_button(VNode* self,
                                     VNodeModule* mod,
                                     const VInputEvent* event) {
  if (event->u.mouse_button.button != V_MOUSE_BUTTON_LEFT) {
    // TODO: only left mouse button is supported for now
    return;
  }

  VRect initial_clip = {0, 0, 1e9f, 1e9f};
  const float x = event->u.mouse_button.x;
  const float y = event->u.mouse_button.y;
  const bool down = event->u.mouse_button.down;
  VArray* popover_stack = &mod->popover_stack;
  VNode* target_node = NULL;

  // 1. Popover Hit Testing (Top to Bottom)
  for (size_t i = popover_stack->size; i-- > 0;) {
    target_node = v_node_hit_test_recursive(
        v_array_get_ptr_unchecked(popover_stack, i), 0, 0, x, y, initial_clip);
    if (target_node) {
      break;
    }
  }

  // 2. Normal Tree Hit Testing
  if (!target_node) {
    target_node = v_node_hit_test_recursive(self, 0, 0, x, y, initial_clip);
  }

  // TODO: this does not dispatch mouse button events, but handles scrollbar
  // click and an on_click event on mouse up. fine for the current app needs,
  // but needs to be more robust.

  if (down) {
    mod->under_mouse_node = target_node;

    // Check for scrollbar hit
    v_foreach_ancestor(target_node, curr) {
      const VStyle* curr_style = v_node_get_style_or_empty(curr);

      if (vs_get_overflow(curr_style) == V_OVERFLOW_SCROLL) {
        float abs_x, abs_y;
        v_node_get_abs_pos(curr, &abs_x, &abs_y);
        VRect track, thumb;
        v_node_get_scrollbar_rect(curr, abs_x, abs_y, &track, &thumb);

        if (v_point_in_rect(x, y, track.x, track.y, track.width,
                            track.height) &&
            v_point_in_rect(x, y, thumb.x, thumb.y, thumb.width,
                            thumb.height)) {
          mod->drag_node = curr;
          mod->drag_start_y = y;
          mod->drag_start_scroll_y = curr->scroll_y;
        } else {
          /* TODO: bounds and inset may not be up to date here. layout after
           * event? */
          const float inner_h = curr->bounds.height - v_node_inset_height(curr);
          if (y < thumb.y) {
            curr->scroll_y -= inner_h;
          } else {
            curr->scroll_y += inner_h;
          }

          const float max_scroll = v_node_get_max_scroll(curr);
          curr->scroll_y = fmaxf(0.0f, fminf(curr->scroll_y, max_scroll));
        }
        return;
      }
    }
  } else {
    for (size_t i = 0; i < popover_stack->size; i++) {
      VNode* stack_node = v_array_get_ptr_unchecked(popover_stack, i);
      v_node_clear_flag(stack_node, V_NODEFLAG_POPOVER_INVOKED);
    }

    v_node_ref(target_node);

    if (target_node != NULL && target_node == mod->under_mouse_node &&
        mod->drag_node == NULL) {
      VArray* event_path = &mod->event_path;
      const size_t start_index = v_event_path_start(event_path);

      v_foreach_ancestor(target_node, curr) {
        v_event_path_push_ref(event_path, V_NODE_EVENT_CLICK, curr);
      }

      const size_t end_index = event_path->size;

      if (end_index > start_index) {
        VNodeEvent event = {
            .type = V_NODE_EVENT_CLICK,
            .target = target_node,
        };

        v_event_path_dispatch(event_path, &event, start_index, end_index);
        v_event_path_reset(event_path, start_index);
      }
    }

    // light dismiss auto and hint popovers. if the popover was invoked during
    // the click dispatch, it will not be dismissed.
    for (size_t i = popover_stack->size; i-- > 0;) {
      VNode* stack_node = v_array_get_ptr_unchecked(popover_stack, i);

      if (v_node_has_flag(stack_node, V_NODEFLAG_POPOVER_INVOKED)) {
        continue;
      }

      const VPopover type = stack_node->popover_type;

      if ((type == V_POPOVER_AUTO || type == V_POPOVER_HINT) &&
          !v_node_is_descendant_of(target_node, stack_node)) {
        v_node_hide_popover(stack_node);
      }
    }

    v_node_unref(target_node);
    mod->under_mouse_node = NULL;
    mod->drag_node = NULL;
  }
}

VUID_PKG void v_node_on_mouse_move(VNode* self,
                                   VNodeModule* mod,
                                   const VInputEvent* input_event) {
  /* Dragging */

  VNode* drag_node = mod->drag_node;
  const float x = input_event->u.mouse_move.x;
  const float y = input_event->u.mouse_move.y;

  if (drag_node) {
    const float dy = y - mod->drag_start_y;
    /* TODO: bounds and inset may not be up to date here. layout after event? */
    const float inner_h =
        drag_node->bounds.height - v_node_inset_height(drag_node);
    const float thumb_h =
        fmaxf(20.0f, (inner_h / drag_node->content_height) * inner_h);
    const float max_scroll = v_node_get_max_scroll(drag_node);

    if (inner_h > thumb_h) {
      const float scroll_move = (dy / (inner_h - thumb_h)) * max_scroll;
      drag_node->scroll_y = fmaxf(
          0.0f, fminf(mod->drag_start_scroll_y + scroll_move, max_scroll));
    }

    v_ctx_request_render();
    return;
  }

  /* Hit Testing */

  // TODO: duplicate code in v_node_on_mouse_button

  VRect initial_clip = {0, 0, 1e9f, 1e9f};
  VNode* new_hovered = NULL;
  VArray* popover_stack = &mod->popover_stack;

  // popover Hit Testing (Top to Bottom)
  for (size_t i = popover_stack->size; i-- > 0;) {
    new_hovered = v_node_hit_test_recursive(
        v_array_get_ptr_unchecked(popover_stack, i), 0, 0, x, y, initial_clip);
    if (new_hovered) {
      break;
    }
  }

  // normal Tree Hit Testing
  if (!new_hovered) {
    new_hovered = v_node_hit_test_recursive(self, 0, 0, x, y, initial_clip);
  }

  // dismiss HINT popovers when cursor leaves their subtree
  for (size_t i = popover_stack->size; i-- > 0;) {
    VNode* stack_node = v_array_get_ptr_unchecked(popover_stack, i);
    if (stack_node->popover_type == V_POPOVER_HINT &&
        !v_node_is_descendant_of(new_hovered, stack_node)) {
      v_node_hide_popover(stack_node);
    }
  }

  /* Dispatch Events */

  VNode* hovered_node = mod->hovered_node;

  if (new_hovered == hovered_node) {
    return;
  }

  VArray* event_path = &mod->event_path;
  VNode* fca = v_node_find_common_ancestor(hovered_node, new_hovered);
  const size_t event_path_start_index = v_event_path_start(event_path);
  const size_t leave_start_index = v_event_path_start(event_path);

  // leave event path: hovered_node -> fca (exclusive)
  for (VNode* curr = hovered_node; curr && curr != fca;
       curr = v_node_parent(curr)) {
    v_event_path_push_ref(event_path, V_NODE_EVENT_MOUSE_LEAVE, curr);
  }

  const size_t enter_start_index = event_path->size;

  // enter event path: new_hovered -> fca (exclusive)
  for (VNode* curr = new_hovered; curr && curr != fca;
       curr = v_node_parent(curr)) {
    v_event_path_push_ref(event_path, V_NODE_EVENT_MOUSE_ENTER, curr);
  }

  v_node_ref(new_hovered);
  v_node_ref(hovered_node);

  if (enter_start_index > leave_start_index) {
    VNodeEvent leave_event = {
        .type = V_NODE_EVENT_MOUSE_LEAVE,
        .target = hovered_node,
        .related_target = new_hovered,
    };
    v_event_path_dispatch(event_path, &leave_event, leave_start_index,
                          enter_start_index);
  }

  const size_t end_index = event_path->size;

  if (end_index > enter_start_index) {
    VNodeEvent enter_event = {
        .type = V_NODE_EVENT_MOUSE_ENTER,
        .target = new_hovered,
        .related_target = hovered_node,
    };
    v_event_path_dispatch(event_path, &enter_event, enter_start_index,
                          end_index);
  }

  // the new_hovered ref could have been removed from the tree or destroyed
  // during event dispatch. mod->hovered_node is unowned, so we need to check
  // that new_hovered is owned by the root to safely set hovered_node.
  if (new_hovered && !v_node_has_flag(new_hovered, V_NODEFLAG_ATTACHED)) {
    mod->hovered_node = NULL;
  } else {
    mod->hovered_node = new_hovered;
  }

  v_node_unref(new_hovered);
  v_node_unref(hovered_node);

  v_event_path_reset(event_path, event_path_start_index);
}

VUID_PKG void v_node_on_mouse_wheel(VNode* self,
                                    VNodeModule* mod,
                                    const VInputEvent* input_event) {
  UNUSED(self);
  if (!mod->hovered_node) {
    return;
  }

  v_foreach_ancestor(mod->hovered_node, curr) {
    if (vs_get_overflow(v_node_get_style_or_empty(curr)) == V_OVERFLOW_SCROLL) {
      const float max_scroll = v_node_get_max_scroll(curr);
      // TODO: 20.f works on my machine, configurable?
      const float scroll_amount = input_event->u.mouse_wheel.y *
                                  input_event->u.mouse_wheel.direction * 20.0f;
      curr->scroll_y =
          fmaxf(0.0f, fminf(curr->scroll_y - scroll_amount, max_scroll));
      v_ctx_request_render();
      return;
    }
  }
}

VUID_PKG void v_node_on_key(VNode* self,
                            VNodeModule* mod,
                            const VInputEvent* input_event) {
  if (self == NULL) {
    // TODO: assert?
    return;
  }

  VArray* event_path = &mod->event_path;
  const size_t start_index = v_event_path_start(event_path);
  const VNodeEventType type =
      input_event->u.key.down ? V_NODE_EVENT_KEY_DOWN : V_NODE_EVENT_KEY_UP;

  v_foreach_ancestor(self, curr) {
    if (curr->event_listeners[type]) {
      v_event_path_push_ref(event_path, type, curr);
    }
  }

  const size_t end_index = event_path->size;

  if (end_index > start_index) {
    VNodeEvent key_event = {
        .type = type,
        .target = self,
        .u.key_info =
            {
                .key = input_event->u.key.key,
                .modifiers = input_event->u.key.modifiers,
                .repeat_count = input_event->u.key.repeat_count,
                .down = input_event->u.key.down,
            },
    };

    v_event_path_dispatch(event_path, &key_event, start_index, end_index);

    v_event_path_reset(event_path, start_index);
  }
}

static VNode* v_node_hit_test_recursive(VNode* node,
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
  const VStyle* style = v_node_get_style_or_empty(node);
  const VOverflow node_overflow = vs_get_overflow(style);

  if (node_overflow == V_OVERFLOW_HIDDEN ||
      node_overflow == V_OVERFLOW_SCROLL) {
    const VRect inner_rect = {
        current_abs_x + v_node_inset_left(node),
        current_abs_y + v_node_inset_top(node),
        node->bounds.width - v_node_inset_width(node),
        node->bounds.height - v_node_inset_height(node),
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
    if (child->popover_type != V_POPOVER_NONE) {
      continue;
    }
    VNode* hit = v_node_hit_test_recursive(child, current_abs_x,
                                           current_abs_y - node->scroll_y, mx,
                                           my, next_clip);
    if (hit) {
      return hit;
    }
  }

  return node;
}



static VStyleClassHashMapKey v_style_sheet_make_key(const char* name) {
  return (VStyleClassHashMapKey){.id = name, .hash = v_fnv1_hash(name)};
}

VUID_API VStyle* vss_get_class(const char* name) {
  V_CHECK_CONTEXT(NULL);
  if (v_cstr_is_empty(name)) {
    return NULL;
  }

  VStyleClassHashMap* style_sheet = v_node_module_get_style_sheet();
  VStyleClass* style_class =
      v_style_class_hmap_get(style_sheet, v_style_sheet_make_key(name));

  return (style_class && style_class->is_ready) ? style_class->style : NULL;
}

VUID_API bool vss_has_class(const char* name) {
  V_CHECK_CONTEXT(false);
  return vss_get_class(name) != NULL;
}

VUID_API bool vss_remove_class(const char* name) {
  V_CHECK_CONTEXT(false);

  if (v_cstr_is_empty(name)) {
    return false;
  }

  VStyleClassHashMap* style_sheet = v_node_module_get_style_sheet();

  return v_style_class_hmap_remove(style_sheet, v_style_sheet_make_key(name));
}

VUID_API VStyle* vss__start_class(const char* name, const char* base) {
  V_CHECK_CONTEXT(NULL);

  if (!v_style_class_is_valid_name(name)) {
    return NULL;
  }

  VStyleClassHashMap* style_sheet = v_node_module_get_style_sheet();
  VStyleClass* existing_class =
      v_style_class_hmap_get(style_sheet, v_style_sheet_make_key(name));
  VStyle* style;

  if (existing_class) {
    existing_class->is_ready = false;
    v_style_reset(existing_class->style);
    style = existing_class->style;
  } else {
    VStyleClass new_style_class;

    if (!v_style_class_init(&new_style_class, name)) {
      return NULL;
    }

    VStyleClassHashMapResult result =
        v_style_class_hmap_put(style_sheet, new_style_class);

    if (!result.inserted) {
      return NULL;
    }

    style = new_style_class.style;
  }

  VStyle* base_style = base ? vss_get_class(base) : NULL;

  if (base_style) {
    v_style_flatten(style, base_style);
  }

  return style;
}

VUID_API void vss__end_class(const char* name) {
  V_CHECK_CONTEXT();

  if (v_cstr_is_empty(name)) {
    return;
  }

  VStyleClassHashMap* style_sheet = v_node_module_get_style_sheet();
  VStyleClass* style_class =
      v_style_class_hmap_get(style_sheet, v_style_sheet_make_key(name));

  if (style_class) {
    style_class->is_ready = true;
  }
}

//
// style class hash map implementation
//

static uint32_t VStyleClass_get_hash(const VStyleClass* value) {
  return value->id.hash;
}

static uint32_t VStyleClassHashMapKey_get_hash(
    const VStyleClassHashMapKey* key) {
  return key->hash;
}

static VStyleClassHashMapKey VStyleClassHashMapKey_from_value(
    const VStyleClass* value) {
  return (VStyleClassHashMapKey){.id = value->id.str, .hash = value->id.hash};
}

static bool VStyleClass_eq(const VStyleClassHashMapKey* key,
                           const VStyleClass* value) {
  return strcmp(value->id.str, key->id) == 0;
}

#define VStyleClass_drop v_style_class_drop

VUID_HMAP_IMPL(VStyleClassHashMap,
               VStyleClassHashMapKey,
               VStyleClass,
               v_style_class_hmap)





static void v_node_render(VCommandQueue* cmdq,
                          VNode* node,
                          float abs_x,
                          float abs_y,
                          VRect clip,
                          bool skip_popovers);
static float v_constrain_border_radius_to_rect(uint16_t border_radius,
                                               float width,
                                               float height);

static bool v_resolve_visibility(const VNode* node) {
  return v_node_is_visible(node) &&
         vs_get_visibility(node->style) == V_VISIBILITY_VISIBLE;
}

VUID_PKG void v_node_render_root(VCommandQueue* cmdq) {
  // note: root cannot be invisible

  VNodeModule* node_module = v_ctx_node_module();
  VNode* root = node_module->root;
  VRect root_clip = {0, 0, root->bounds.width, root->bounds.height};

  v_node_render(cmdq, root, 0, 0, root_clip, true);
  v_command_queue_cmd_reset_clip(cmdq);

  VArray* popover_stack = &node_module->popover_stack;

  if (popover_stack->size > 0) {
    VColor backdrop_color = v_get_popover_backdrop_color();

    if (backdrop_color.a > 0) {
      v_command_queue_cmd_fill_rect(
          cmdq, &(VRect){0, 0, root->bounds.width, root->bounds.height},
          backdrop_color);
    }

    for (size_t i = 0; i < popover_stack->size; i++) {
      VNode* popover = v_array_get_ptr_unchecked(popover_stack, i);

      if (v_resolve_visibility(popover)) {
        v_node_render(cmdq, popover, 0, 0, root_clip, false);
        v_command_queue_cmd_reset_clip(cmdq);
      }
    }
  }
}

static void v_node_render(VCommandQueue* cmdq,
                          VNode* node,
                          float abs_x,
                          float abs_y,
                          VRect clip,
                          bool skip_popovers) {
  abs_x += node->bounds.x;
  abs_y += node->bounds.y;

  const VStyle* style = v_node_get_style_or_empty(node);
  const VRect box = {
      abs_x,
      abs_y,
      node->bounds.width,
      node->bounds.height,
  };

  VOverflow overflow = vs_get_overflow(style);
  VRect child_clip;
  bool was_clipped;
  float child_abs_y;

  if (overflow == V_OVERFLOW_HIDDEN || overflow == V_OVERFLOW_SCROLL) {
    child_clip = v_rect_intersect(clip, box);
    v_command_queue_cmd_set_clip(cmdq, (int)child_clip.x, (int)child_clip.y,
                                 (int)child_clip.width, (int)child_clip.height);
    was_clipped = true;
    child_abs_y = abs_y - node->scroll_y;
  } else {
    child_clip = clip;
    was_clipped = false;
    child_abs_y = abs_y;
  }

  const float border_radius = v_constrain_border_radius_to_rect(
      v_style_resolve_border_radius(style), node->bounds.width,
      node->bounds.height);
  const bool has_border_color = vs_has_border_color(style);
  float one_border = 0;

  if (has_border_color) {
    if (border_radius > 0) {
      // rounded rect borders are drawn with uniform thickness. use the max of
      // all 4 border insets to get the value.
      one_border = node->border.top;

      if (node->border.left > one_border) {
        one_border = node->border.left;
      }

      if (node->border.bottom > one_border) {
        one_border = node->border.bottom;
      }

      if (node->border.right > one_border) {
        one_border = node->border.right;
      }
    }
  }

  if (vs_has_background(style)) {
    if (border_radius > 0) {
      // skip the outer fringe if drawing a border over this rounded rect
      const bool skip_fringe = has_border_color && one_border > 0;
      v_command_queue_cmd_fill_rounded_rect(
          cmdq, &box, border_radius, vs_get_background(style), skip_fringe);
    } else {
      v_command_queue_cmd_fill_rect(cmdq, &box, vs_get_background(style));
    }
  }

  if (has_border_color) {
    if (border_radius > 0) {
      if (one_border > 0) {
        v_command_queue_cmd_stroke_rounded_rect(
            cmdq, &box, border_radius, one_border, vs_get_border_color(style));
      }
    } else {
      v_command_queue_cmd_stroke_rect(cmdq, &box, &node->border,
                                      vs_get_border_color(style));
    }
  }

  switch (node->tag) {
    case V_NODE_TEXT: {
      if (vs_has_color(style)) {
        const VTextLayout* layout = v_node_get_text_layout(node);
        const float x = box.x + v_node_inset_left(node);
        const float y = box.y + v_node_inset_top(node);

        v_text_layout_render(layout, cmdq, x, y, vs_get_color(style),
                             &v_ctx_text_module()->atlas);
      }

      break;
    }
    case V_NODE_IMAGE: {
      const uint32_t texture_id =
          v_image_get_texture_id(node->res.image_resource);

      if (texture_id != 0) {
        const VRect image_rect = {
            box.x + v_node_inset_left(node),
            box.y + v_node_inset_top(node),
            box.width - v_node_inset_width(node),
            box.height - v_node_inset_height(node),
        };

        const VColor color =
            vs_has_color(style) ? vs_get_color(style) : v_rgb(255, 255, 255);

        v_command_queue_cmd_textured_quad(cmdq, &image_rect, texture_id, 0, 0,
                                          1, 1, color);
      }
      break;
    }
    default:
      break;
  }

  v_foreach_child(node, child) {
    if (v_resolve_visibility(child) &&
        !(skip_popovers && child->popover_type != V_POPOVER_NONE)) {
      v_node_render(cmdq, child, abs_x, child_abs_y, child_clip, skip_popovers);
    }
  }

  if (vs_get_overflow(style) == V_OVERFLOW_SCROLL &&
      node->content_height > node->bounds.height - v_node_inset_height(node)) {
    VRect track;
    VRect thumb;

    v_node_get_scrollbar_rect(node, abs_x, abs_y, &track, &thumb);

    // TODO: what should be the default?
    const VColor scrollbar_thumb = vs_has_scrollbar_thumb(style)
                                       ? vs_get_scrollbar_thumb(style)
                                       : v_rgb(128, 128, 128);

    // TODO: draw track?
    // TODO: hovered thumb check

    const float scrollbar_border_radius = v_constrain_border_radius_to_rect(
        v_style_resolve_scrollbar_border_radius(style), thumb.width,
        thumb.height);

    v_command_queue_cmd_fill_rounded_rect(cmdq, &thumb, scrollbar_border_radius,
                                          scrollbar_thumb, false);
  }

  if (was_clipped) {
    v_command_queue_cmd_set_clip(cmdq, (int)clip.x, (int)clip.y,
                                 (int)clip.width, (int)clip.height);
  }
}

static float v_constrain_border_radius_to_rect(uint16_t border_radius,
                                               float width,
                                               float height) {
  float result = (float)border_radius;

  if (result > 0) {
    const float half_width = width / 2.0f;
    const float half_height = height / 2.0f;

    if (result > half_width) {
      result = half_width;
    }

    if (result > half_height) {
      result = half_height;
    }
  }

  return result;
}






static void v_node_destructor(VNode* node);
static VStyle* v_node_get_or_create_style(VNode* node);
static void v_node_set_attached_recursive(VNode* node, bool attached);
static bool v_node__is_legal_id(const char* id);
static VWeakRef* v_node_get_or_create_self_weak_ref(VNode* node);
static bool v_node__can_insert(const VNode* parent, const VNode* child);
static VStyle* v_get_empty_style(void);
static void v_node_focus_event_dispatch(VNodeModule* mod,
                                        VNodeEventType type,
                                        VNode* target,
                                        VNode* related_target);
static bool v_node_do_focus_change(VNodeModule* mod, VNode* new_focus);

VUID_API VNode* v_node_new(VNodeTag tag) {
  return v_node_new_cfg(tag, NULL);
}

VUID_API VNode* v_node_new_cfg(VNodeTag tag, const VNodeConfig* config) {
  if (tag == V_NODE_ROOT) {
    return NULL;
  }

  VNode* node = v_node_constructor(tag);

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
      v_node_set_event_listener(node, V_NODE_EVENT_CLICK, config->on_click);
    }

    if (config->on_mouse_enter) {
      v_node_set_event_listener(node, V_NODE_EVENT_MOUSE_ENTER,
                                config->on_mouse_enter);
    }

    if (config->on_mouse_leave) {
      v_node_set_event_listener(node, V_NODE_EVENT_MOUSE_LEAVE,
                                config->on_mouse_leave);
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

VUID_API const char* v_node_id(const VNode* node) {
  return node && !v_static_string_is_empty(&node->id) ? node->id.str : "";
}

VUID_API void v_node_set_id(VNode* node, const char* id) {
  if (!node || v_cstr_eq(node->id.str, id)) {
    return;
  }

  const bool attached = v_node_has_flag(node, V_NODEFLAG_ATTACHED);

  if (v_cstr_is_empty(id)) {
    if (attached) {
      v_node_module_remove_node_id(node);
    }

    node->id = (VStaticString){0};
  } else if (v_node__is_legal_id(id)) {
    if (attached && !v_static_string_is_empty(&node->id)) {
      v_node_module_remove_node_id(node);
    }

    if (!v_ctx_intern_string(id, &node->id)) {
      return;
    }

    if (attached) {
      v_node_module_add_node_id(node);
    }
  } else {
    // illegal id, do nothing
  }
}

VUID_API void v_node_set_id_fmt(VNode* node, const char* fmt, ...) {
  va_list args;
  // TODO: use a scratch buffer
  char buf[VUID_MAX_NODE_ID_LENGTH + 1];
  int n;

  va_start(args, fmt);
  n = vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if (n < 0 || n > VUID_MAX_NODE_ID_LENGTH) {
    return;
  }

  v_node_set_id(node, buf);
}

VUID_API VNodeTag v_node_tag(const VNode* node) {
  // null node is undefined behavior, return something rather than crashing
  return node ? node->tag : V_NODE_BOX;
}

VUID_API VStyle* v_node_style(VNode* node) {
  return node ? v_node_get_or_create_style(node) : NULL;
}

VUID_API VNode* v_node_parent(const VNode* node) {
  return node ? v_weak_ref_get(node->parent) : NULL;
}

VUID_API VNode* v_node_first_child(const VNode* node) {
  return node ? node->first_child : NULL;
}

VUID_API VNode* v_node_last_child(const VNode* node) {
  return node ? node->last_child : NULL;
}

VUID_API VNode* v_node_next_sibling(const VNode* node) {
  return node ? node->next_sibling : NULL;
}

VUID_API VNode* v_node_prev_sibling(const VNode* node) {
  return node ? node->prev_sibling : NULL;
}

VUID_API void v_node_mark_dirty(VNode* node) {
  VNode* walker = node;

  while (walker) {
    if (!v_node_has_flag(walker, V_NODEFLAG_DIRTY)) {
      v_node_set_flag(walker, V_NODEFLAG_DIRTY);
    }
    walker = v_node_parent(walker);
  }
}

VUID_API bool v_node_is_dirty(const VNode* node) {
  return node && v_node_has_flag(node, V_NODEFLAG_DIRTY);
}

VUID_API void v_node_set_visible(VNode* node, bool is_visible) {
  if (!node || node->tag == V_NODE_ROOT) {
    return;
  }

  bool current = v_node_has_flag(node, V_NODEFLAG_HIDDEN) == false;
  if (current == is_visible) {
    return;
  }

  if (is_visible) {
    v_node_clear_flag(node, V_NODEFLAG_HIDDEN);
  } else {
    v_node_set_flag(node, V_NODEFLAG_HIDDEN);
  }

  v_node_mark_dirty(node);
}

VUID_API bool v_node_is_visible(const VNode* node) {
  return node && !v_node_has_flag(node, V_NODEFLAG_HIDDEN);
}

VUID_API void v_node_set_event_listener(VNode* node,
                                        int event_type,
                                        VNodeEventListener listener) {
  if (event_type < 0 || event_type >= V_NODE_EVENT__COUNT) {
    return;
  }

  node->event_listeners[event_type] = listener;
}

VUID_API void v_node_reset_scroll_y(VNode* node) {
  if (node) {
    node->scroll_y = 0;
    v_ctx_request_render();
  }
}

VUID_API int v_node_child_count(const VNode* node) {
  return node ? node->child_count : 0;
}

VUID_API VNode* v_node_child_at(const VNode* node, int index) {
  int i = 0;

  v_foreach_child(node, child) {
    if (i++ == index) {
      return child;
    }
  }

  return NULL;
}

VUID_API bool v_node_append_child(VNode* node, VNode* child) {
  if (!v_node__can_insert(node, child)) {
    // TODO: unref child?
    return false;
  }

  VWeakRef* parent = v_node_get_or_create_self_weak_ref(node);

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

  if (v_node_has_flag(node, V_NODEFLAG_ATTACHED)) {
    v_node_set_attached_recursive(child, true);
  }

  v_node_mark_dirty(node);

  return true;
}

VUID_API bool v_node_insert_before(VNode* node,
                                   VNode* child,
                                   VNode* reference_node) {
  if (reference_node == NULL) {
    return v_node_append_child(node, child);
  }

  if (!v_node__can_insert(node, child) ||
      v_node_parent(reference_node) != node) {
    // TODO: unref child?
    return false;
  }

  VWeakRef* parent = v_node_get_or_create_self_weak_ref(node);

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

  if (v_node_has_flag(node, V_NODEFLAG_ATTACHED)) {
    v_node_set_attached_recursive(child, true);
  }

  v_node_mark_dirty(node);

  return true;
}

VUID_API bool v_node_prepend_child(VNode* node, VNode* child) {
  if (!v_node__can_insert(node, child)) {
    // TODO: unref child?
    return false;
  }

  VWeakRef* parent = v_node_get_or_create_self_weak_ref(node);

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

  if (v_node_has_flag(node, V_NODEFLAG_ATTACHED)) {
    v_node_set_attached_recursive(child, true);
  }

  v_node_mark_dirty(node);

  return true;
}

VUID_API bool v_node_remove_child(VNode* node, VNode* child) {
  if (!node || !child || v_node_parent(child) != node) {
    // TODO: unref child?
    return false;
  }

  if (v_node_has_flag(child, V_NODEFLAG_ATTACHED)) {
    v_node_set_attached_recursive(child, false);
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

  v_weak_ref_release(child->parent);
  child->parent = NULL;
  child->next_sibling = NULL;
  child->prev_sibling = NULL;
  v_node_unref(child);

  // TODO: remove the child and its children from the context?

  node->child_count--;
  v_node_mark_dirty(node);

  return true;
}

VUID_API bool v_node_replace_child(VNode* node,
                                   VNode* new_child,
                                   VNode* old_child) {
  if (!v_node__can_insert(node, new_child) ||
      v_node_parent(old_child) != node) {
    // TODO: unref new_child?
    return false;
  }

  VWeakRef* parent = v_node_get_or_create_self_weak_ref(node);

  if (!parent) {
    // TODO: unref child?
    return false;
  }

  if (v_node_has_flag(old_child, V_NODEFLAG_ATTACHED)) {
    v_node_set_attached_recursive(old_child, false);
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

  if (v_node_has_flag(node, V_NODEFLAG_ATTACHED)) {
    v_node_set_attached_recursive(new_child, true);
  }

  v_weak_ref_release(old_child->parent);
  old_child->parent = NULL;
  old_child->next_sibling = NULL;
  old_child->prev_sibling = NULL;
  v_node_unref(old_child);

  v_node_mark_dirty(node);

  return true;
}

VUID_API bool v_node_remove(VNode* node) {
  if (!node || !node->parent) {
    return false;
  }

  return v_node_remove_child(v_weak_ref_get(node->parent), node);
}

VUID_API void v_node_remove_children(VNode* node) {
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

VUID_API void v_node_set_text(VNode* node, const char* value) {
  if (!node || node->tag != V_NODE_TEXT ||
      v_string_eq_cstr(node->res_data.text, value)) {
    return;
  }

  VString* new_text = v_string_from(v_ctx_allocator(), value);

  v_string_unref(node->res_data.text);
  node->res_data.text = new_text;
  v_node_mark_dirty(node);
}

VUID_API void v_node_set_text_fmt(VNode* node, const char* fmt, ...) {
  if (!node || node->tag != V_NODE_TEXT) {
    return;
  }

  va_list args;
  va_start(args, fmt);
  VString* new_text = v_string_from_vfmt(v_ctx_allocator(), fmt, args);
  va_end(args);

  if (v_string_eq(node->res_data.text, new_text)) {
    v_string_unref(new_text);
  } else {
    v_string_unref(node->res_data.text);
    node->res_data.text = new_text;
    v_node_mark_dirty(node);
  }
}

VUID_API const char* v_node_text(const VNode* node) {
  return (node && node->tag == V_NODE_TEXT) ? v_string_cstr(node->res_data.text)
                                            : "";
}

VUID_API void v_node_set_src(VNode* node, const char* src) {
  if (!node || node->tag != V_NODE_IMAGE) {
    return;
  }

  if (v_string_eq_cstr(node->res_data.src, src)) {
    return;
  }

  VImageStore* image_store = v_ctx_image_store();

  if (node->res.image_resource) {
    v_image_store_release(image_store, node->res.image_resource);
    node->res.image_resource = NULL;
  }

  if (node->res_data.src) {
    v_string_unref(node->res_data.src);
    node->res_data.src = NULL;
  }

  if (!v_cstr_is_empty(src)) {
    // set the src even if the acquire fails. if src fails, the image resource
    // will just be NULL. TODO: review this policy.
    VString* new_src = v_string_from(v_ctx_allocator(), src);

    node->res_data.src = new_src;

    VImage* new_image = v_image_store_acquire(image_store, new_src);

    node->res.image_resource = new_image;
  }

  v_node_mark_dirty(node);
}

VUID_API const char* v_node_src(const VNode* node) {
  return v_string_cstr(node && node->tag == V_NODE_IMAGE ? node->res_data.src
                                                         : NULL);
}

VUID_API void v_node_set_data(VNode* node, void* data) {
  if (node) {
    node->user_data = data;
  }
}

VUID_API void* v_node_data(const VNode* node) {
  return node ? node->user_data : NULL;
}

VUID_API void v_node_style_assign(VNode* node, VStyle* style) {
  if (node) {
    v_node_style_reset(node);
    v_node_style_apply(node, style);
  }
}

VUID_API void v_node_style_apply(VNode* node, VStyle* style) {
  // TODO: should style == NULL be a reset?
  if (node && style) {
    VStyle* node_style = v_node_get_or_create_style(node);

    if (node_style != style) {
      v_style_flatten(node_style, style);
    }
  }
}

VUID_API void v_node_style_reset(VNode* node) {
  if (node) {
    v_style_reset(node->style);
  }
}

VUID_API VPopover v_node_popover(const VNode* node) {
  return node ? node->popover_type : V_POPOVER_NONE;
}

VUID_API void v_node_set_popover(VNode* node, VPopover type) {
  if (!node || node->popover_type == type) {
    return;
  }

  if (v_node_has_flag(node, V_NODEFLAG_POPOVER_OPEN)) {
    v_node_hide_popover(node);
  }

  node->popover_type = type;
}

VUID_API bool v_node_show_popover(VNode* node) {
  if (!node || node->popover_type == V_POPOVER_NONE ||
      !v_node_has_flag(node, V_NODEFLAG_ATTACHED) ||
      v_node_has_flag(node, V_NODEFLAG_POPOVER_OPEN)) {
    return false;
  }

  VArray* popover_stack = v_node_module_get_popover_stack();

  if (node->popover_type == V_POPOVER_AUTO) {
    for (size_t i = popover_stack->size; i-- > 0;) {
      VNode* stack_node = v_array_get_ptr_unchecked(popover_stack, i);
      if (stack_node->popover_type == V_POPOVER_AUTO &&
          !v_node_is_descendant_of(node, stack_node)) {
        v_node_hide_popover(stack_node);
      }
    }
  }
  // HINT: does not close other popovers on show

  // TODO: assert?
  if (!v_array_push(popover_stack, &node))
    return false;

  v_node_set_flag(node, V_NODEFLAG_POPOVER_OPEN);
  v_node_set_flag(node, V_NODEFLAG_POPOVER_INVOKED);
  v_node_set_visible(node, true);
  v_node_mark_dirty(node);
  return true;
}

VUID_API void v_node_hide_popover(VNode* node) {
  if (!node || !v_node_has_flag(node, V_NODEFLAG_POPOVER_OPEN)) {
    return;
  }

  v_node_module_remove_popover_node(node);
  v_node_clear_flag(node, V_NODEFLAG_POPOVER_OPEN);
  v_node_clear_flag(node, V_NODEFLAG_POPOVER_INVOKED);
  v_node_set_visible(node, false);
  v_node_mark_dirty(node);
}

VUID_API void v_node_toggle_popover(VNode* node) {
  if (!node) {
    return;
  }

  if (v_node_has_flag(node, V_NODEFLAG_POPOVER_OPEN)) {
    v_node_hide_popover(node);
  } else {
    v_node_show_popover(node);
  }
}

VUID_API void v_node_ref(VNode* node) {
  if (node) {
    if (node->ref_count < UINT32_MAX) {
      node->ref_count++;
    } else {
      VUID_ASSERT(false && "VNode ref count overflow");
    }
  }
}

VUID_API void v_node_unref(VNode* node) {
  if (node) {
    const uint32_t ref_count = node->ref_count;

    if (ref_count > 1) {
      node->ref_count--;
    } else if (ref_count == 1) {
      v_node_destructor(node);
    } else {
      VUID_ASSERT(false && "VNode ref count underflow");
    }
  }
}

VUID_API uint32_t v_node_ref_count(const VNode* node) {
  return node ? node->ref_count : 0;
}

VUID_API void v_node_event_stop_propagation(VNodeEvent* event) {
  if (event) {
    v_node_event_set_flag(event, V_NODE_EVENT_FLAG_CANCELLED);
  }
}

VUID_PKG const VStyle* v_node_get_style_or_empty(const VNode* node) {
  return node->style ? node->style : v_get_empty_style();
}

VUID_PKG VTextLayout* v_node_get_text_layout(const VNode* node) {
  return node && node->tag == V_NODE_TEXT ? node->res.text_layout : NULL;
}

VUID_PKG void v_node_set_text_layout(VNode* node, VTextLayout* layout) {
  if (node && node->tag == V_NODE_TEXT) {
    node->res.text_layout = layout;
  }
}

VUID_PKG void v_node_set_attached(VNode* node, bool attached) {
  if (attached) {
    v_node_set_flag(node, V_NODEFLAG_ATTACHED);
    if (!v_static_string_is_empty(&node->id)) {
      v_node_module_add_node_id(node);
    }
  } else {
    v_node_module_remove_input_node(node);
    if (v_node_has_flag(node, V_NODEFLAG_POPOVER_OPEN)) {
      v_node_hide_popover(node);
    }
    v_node_clear_flag(node, V_NODEFLAG_ATTACHED);
    if (!v_static_string_is_empty(&node->id)) {
      v_node_module_remove_node_id(node);
    }
  }
}

VUID_PKG float v_node_get_max_scroll(const VNode* node) {
  const float inner_h = node->bounds.height - v_node_inset_height(node);
  return fmaxf(0.0f, node->content_height - inner_h);
}

VUID_PKG void v_node_get_scrollbar_rect(VNode* node,
                                        float abs_x,
                                        float abs_y,
                                        VRect* track,
                                        VRect* thumb) {
  const VStyle* style = v_node_get_style_or_empty(node);
  const float scrollbar_width = v_style_resolve_scrollbar_width(style);
  const float inner_h = node->bounds.height - v_node_inset_height(node);
  const float max_scroll = v_node_get_max_scroll(node);

  const float content_x = abs_x + v_node_inset_left(node);
  const float content_y = abs_y + v_node_inset_top(node);
  const float content_w =
      node->bounds.width - node->border.left - node->border.right;
  const float content_right = content_x + content_w;

  track->width = scrollbar_width;
  track->height = inner_h;
  track->x = content_right - scrollbar_width;
  track->y = content_y;

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

VUID_PKG bool v_node_is_descendant_of(const VNode* node,
                                      const VNode* ancestor) {
  if (!node || !ancestor)
    return false;
  if (node == ancestor)
    return true;
  return v_node_is_descendant_of(v_node_parent(node), ancestor);
}

VUID_PKG void v_node_get_abs_pos(const VNode* node, float* x, float* y) {
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

VUID_PKG VNode* v_node_find_common_ancestor(VNode* a, VNode* b) {
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

VUID_PKG VNode* v_node_constructor(VNodeTag tag) {
  VNode* node = v_ctx_new(VNode);

  if (node) {
    node->ref_count = 1;
    node->tag = tag;
    node->flags = V_NODEFLAG_DIRTY;

    if (tag == V_NODE_TEXT) {
      v_node_set_flag(node, V_NODEFLAG_LEAF);
    } else if (tag == V_NODE_IMAGE) {
      v_node_set_flag(node, V_NODEFLAG_LEAF);
    }

    v_ctx_object_inc(V_OBJECT_TYPE_NODE);
  }

  return node;
}

VUID_API bool v_node_focus(VNode* self) {
  V_CHECK_CONTEXT(false);
  return self ? v_node_do_focus_change(v_ctx_node_module(), self) : false;
}

VUID_API bool v_node_blur(VNode* self) {
  V_CHECK_CONTEXT(false);
  return self ? v_node_do_focus_change(v_ctx_node_module(), v_root()) : false;
}

VUID_API bool v_node_has_focus(const VNode* node) {
  V_CHECK_CONTEXT(false);
  VNodeModule* mod = v_ctx_node_module();
  return node ? node == mod->focused_node : false;
}

static VStyle* v_node_get_or_create_style(VNode* node) {
  VStyle* style = node->style;

  if (!style) {
    style = node->style = v_style_new(node);
  }

  return style;
}

static void v_node_destructor(VNode* node) {
  VUID_ASSERT(v_node_has_parent(node) == false);
  VUID_ASSERT(!v_node_has_flag(node, V_NODEFLAG_ATTACHED));

  v_node_remove_children(node);

  // release resources
  if (node->tag == V_NODE_TEXT) {
    v_string_unref(node->res_data.text);

    if (node->res.text_layout) {
      v_text_module_destroy_layout(v_ctx_text_module(), node->res.text_layout);
    }
  } else if (node->tag == V_NODE_IMAGE) {
    v_string_unref(node->res_data.src);
    if (node->res.image_resource) {
      v_image_store_release(v_ctx_image_store(), node->res.image_resource);
    }
  }

  // release style and clear unowned reference to us
  if (node->style) {
    v_style_set_owner(node->style, NULL);
    v_style_unref(node->style);
  }

  // release weak ref and revoke the ref to us
  if (node->self_weak_ref) {
    node->self_weak_ref->ref = NULL;
    v_weak_ref_release(node->self_weak_ref);
  }

  v_ctx_delete(node, VNode);

  v_ctx_object_dec(V_OBJECT_TYPE_NODE);
}

static void v_node_set_attached_recursive(VNode* node, bool attached) {
  v_node_set_attached(node, attached);

  v_foreach_child(node, child) {
    v_node_set_attached_recursive(child, attached);
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

  size_t len = 1;

  id++;

  while (*id) {
    int c = *id++;

    if (!(v_char_is_alpha(c) || v_char_is_digit(c) || c == '_' || c == '-' ||
          c == '.' || c == ':')) {
      return false;
    }

    len++;

    if (len > VUID_MAX_NODE_ID_LENGTH) {
      return false;
    }
  }

  return true;
}

static bool v_node__can_insert(const VNode* parent, const VNode* child) {
  return parent && child && !v_node_has_flag(parent, V_NODEFLAG_LEAF) &&
         parent != child && child->tag != V_NODE_ROOT &&
         !v_node_has_parent(child);
}

static VWeakRef* v_node_get_or_create_self_weak_ref(VNode* node) {
  VWeakRef* weak_ref = node->self_weak_ref;

  if (!weak_ref) {
    weak_ref = v_weak_ref_new(node);

    if (!weak_ref) {
      return NULL;
    }

    node->self_weak_ref = weak_ref;
  }

  return v_weak_ref_acquire(weak_ref);
}

static VStyle* v_get_empty_style(void) {
  return v_ctx_node_module()->empty_style;
}

static void v_node_focus_event_dispatch(VNodeModule* mod,
                                        VNodeEventType type,
                                        VNode* target,
                                        VNode* related_target) {
  VArray* event_path = &mod->event_path;
  const uint32_t start_index = v_event_path_start(event_path);

  v_foreach_ancestor(target, node) {
    v_event_path_push_ref(event_path, type, node);
  }

  const uint32_t end_index = event_path->size;

  if (end_index > start_index) {
    VNodeEvent event = {
        .type = type,
        .target = target,
        .related_target = related_target,
    };
    v_event_path_dispatch(event_path, &event, start_index, end_index);
    v_event_path_reset(event_path, start_index);
  }
}

static bool v_node_do_focus_change(VNodeModule* mod, VNode* new_focus) {
  if (!v_node_has_flag(new_focus, V_NODEFLAG_ATTACHED)) {
    return false;
  }

  VNode* old_focus = mod->focused_node;

  if (new_focus == old_focus) {
    return true;
  }

  mod->focused_node = new_focus;

  v_node_ref(old_focus);
  v_node_ref(new_focus);

  v_node_focus_event_dispatch(mod, V_NODE_EVENT_BLUR, old_focus, new_focus);
  v_node_focus_event_dispatch(mod, V_NODE_EVENT_FOCUS, new_focus, old_focus);

  v_node_unref(old_focus);
  v_node_unref(new_focus);

  return true;
}


#if defined(__clang__)
    #pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
    #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
    #pragma warning(pop)
#endif
