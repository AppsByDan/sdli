// clang-format off
//
// vuid is an experimental retained mode UI framework.
//
// MIT License
//
// Copyright (c) 2026 Daniel Anderson
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// clang-format on

#ifndef VUID_H
#define VUID_H

#include <vuid_config.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined __GNUC__
#define VUID_GNUATTR(...) __attribute__((__VA_ARGS__))
#else
#define VUID_GNUATTR(...)
#endif

#ifndef VUID_API
#define VUID_API
#endif

#ifndef VUID_ASSERT
#include <assert.h>
#define VUID_ASSERT(EXPR) assert(EXPR)
#endif

// clang-format off

// ============================================================
// Forward declarations
// ============================================================

typedef struct VNode  VNode;
typedef struct VStyle VStyle;

// ============================================================
// Geometry
// ============================================================

typedef struct VSize {
  float width;
  float height;
} VSize;

typedef struct VRect {
  float x;
  float y;
  float width;
  float height;
} VRect;

typedef struct VColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} VColor;

// ============================================================
// Style enums and types
// ============================================================

typedef enum VStyleValueUnit {
  V_STYLE_VALUE_UNIT_AUTO,
  // TODO: should fit and grow be in their own enum?
  V_STYLE_VALUE_UNIT_FIT,
  V_STYLE_VALUE_UNIT_GROW,
  V_STYLE_VALUE_UNIT_PX,
} VStyleValueUnit;

typedef struct VStyleValue {
  VStyleValueUnit unit;
  union {
    float px;
  } value;
} VStyleValue;

typedef enum VPosition {
  V_POSITION_STATIC,
  V_POSITION_RELATIVE,
  V_POSITION_ABSOLUTE,
} VPosition;

typedef enum VDirection {
  V_DIRECTION_ROW,
  V_DIRECTION_COLUMN,
} VDirection;

typedef enum VWrap {
  V_WRAP_NONE,
  V_WRAP_WRAP,
} VWrap;

typedef enum VTextWrap {
  V_TEXT_WRAP_WRAP,
  V_TEXT_WRAP_NO_WRAP,
} VTextWrap;

typedef enum VOverflow {
  V_OVERFLOW_VISIBLE,
  V_OVERFLOW_HIDDEN,
  V_OVERFLOW_SCROLL,
} VOverflow;

typedef enum VVisibility {
  V_VISIBILITY_VISIBLE,
  V_VISIBILITY_HIDDEN,
} VVisibility;

typedef enum VAlignX {
  V_ALIGN_X_LEFT,
  V_ALIGN_X_CENTER,
  V_ALIGN_X_RIGHT,
} VAlignX;

typedef enum VAlignY {
  V_ALIGN_Y_TOP,
  V_ALIGN_Y_CENTER,
  V_ALIGN_Y_BOTTOM,
} VAlignY;

typedef enum VAnchorTo {
  V_ANCHOR_TO_PARENT,
} VAnchorTo;

typedef enum VPositionFallback {
  V_POSITION_FALLBACK_NONE,
  V_POSITION_FALLBACK_FLIP,
} VPositionFallback;

typedef enum VAttachPointX {
  V_ATTACH_POINT_X_LEFT,
  V_ATTACH_POINT_X_CENTER,
  V_ATTACH_POINT_X_RIGHT,
} VAttachPointX;

typedef enum VAttachPointY {
  V_ATTACH_POINT_Y_TOP,
  V_ATTACH_POINT_Y_CENTER,
  V_ATTACH_POINT_Y_BOTTOM,
} VAttachPointY;

// ============================================================
// Keyboard input
// ============================================================

typedef enum VKey {
  V_KEY_INVALID = 0,
  V_KEY_TAB = 1,
  V_KEY_LEFT_ARROW,
  V_KEY_RIGHT_ARROW,
  V_KEY_UP_ARROW,
  V_KEY_DOWN_ARROW,
  V_KEY_PAGE_UP,
  V_KEY_PAGE_DOWN,
  V_KEY_HOME,
  V_KEY_END,
  V_KEY_INSERT,
  V_KEY_DELETE,
  V_KEY_BACKSPACE,
  V_KEY_SPACE,
  V_KEY_ENTER,
  V_KEY_ESCAPE,
  V_KEY_LEFT_CTRL, V_KEY_LEFT_SHIFT, V_KEY_LEFT_ALT, V_KEY_LEFT_SUPER,
  V_KEY_RIGHT_CTRL, V_KEY_RIGHT_SHIFT, V_KEY_RIGHT_ALT, V_KEY_RIGHT_SUPER,
  V_KEY_MENU,
  V_KEY_0, V_KEY_1, V_KEY_2, V_KEY_3, V_KEY_4, V_KEY_5, V_KEY_6, V_KEY_7, V_KEY_8, V_KEY_9,
  V_KEY_A, V_KEY_B, V_KEY_C, V_KEY_D, V_KEY_E, V_KEY_F, V_KEY_G, V_KEY_H, V_KEY_I, V_KEY_J,
  V_KEY_K, V_KEY_L, V_KEY_M, V_KEY_N, V_KEY_O, V_KEY_P, V_KEY_Q, V_KEY_R, V_KEY_S, V_KEY_T,
  V_KEY_U, V_KEY_V, V_KEY_W, V_KEY_X, V_KEY_Y, V_KEY_Z,
  V_KEY_F1, V_KEY_F2, V_KEY_F3, V_KEY_F4, V_KEY_F5, V_KEY_F6,
  V_KEY_F7, V_KEY_F8, V_KEY_F9, V_KEY_F10, V_KEY_F11, V_KEY_F12,
  V_KEY_F13, V_KEY_F14, V_KEY_F15, V_KEY_F16, V_KEY_F17, V_KEY_F18,
  V_KEY_F19, V_KEY_F20, V_KEY_F21, V_KEY_F22, V_KEY_F23, V_KEY_F24,
  V_KEY_APOSTROPHE,
  V_KEY_COMMA,
  V_KEY_MINUS,
  V_KEY_PERIOD,
  V_KEY_SLASH,
  V_KEY_SEMICOLON,
  V_KEY_EQUAL,
  V_KEY_LEFT_BRACKET,
  V_KEY_BACKSLASH,
  V_KEY_RIGHT_BRACKET,
  V_KEY_GRAVE_ACCENT,
  V_KEY_CAPS_LOCK,
  V_KEY_SCROLL_LOCK,
  V_KEY_NUM_LOCK,
  V_KEY_PRINT_SCREEN,
  V_KEY_PAUSE,
  V_KEY_KEYPAD_0, V_KEY_KEYPAD_1, V_KEY_KEYPAD_2, V_KEY_KEYPAD_3, V_KEY_KEYPAD_4,
  V_KEY_KEYPAD_5, V_KEY_KEYPAD_6, V_KEY_KEYPAD_7, V_KEY_KEYPAD_8, V_KEY_KEYPAD_9,
  V_KEY_KEYPAD_DECIMAL,
  V_KEY_KEYPAD_DIVIDE,
  V_KEY_KEYPAD_MULTIPLY,
  V_KEY_KEYPAD_SUBTRACT,
  V_KEY_KEYPAD_ADD,
  V_KEY_KEYPAD_ENTER,
  V_KEY_KEYPAD_EQUAL,
  V_KEY_APP_BACK,
  V_KEY_APP_FORWARD,
  V_KEY_OEM_102,
} VKey;

typedef enum VKeyMod {
  V_KMOD_NONE   = 0,
  V_KMOD_LSHIFT = 1 << 0,
  V_KMOD_RSHIFT = 1 << 1,
  V_KMOD_LCTRL  = 1 << 2,
  V_KMOD_RCTRL  = 1 << 3,
  V_KMOD_LALT   = 1 << 4,
  V_KMOD_RALT   = 1 << 5,
  V_KMOD_LGUI   = 1 << 6,
  V_KMOD_RGUI   = 1 << 7,
  V_KMOD_NUM    = 1 << 8,
  V_KMOD_CAPS   = 1 << 9,
  V_KMOD_MODE   = 1 << 10,
  V_KMOD_SCROLL = 1 << 11,
} VKeyMod;

// ============================================================
// Input events (integration -> vuid)
// ============================================================

typedef enum VMouseButton {
  V_MOUSE_BUTTON_INVALID = 0,
  V_MOUSE_BUTTON_LEFT    = 1,
  V_MOUSE_BUTTON_RIGHT   = 2,
  V_MOUSE_BUTTON_MIDDLE  = 3,
} VMouseButton;

typedef enum VInputEventType {
  V_INPUT_EVENT_MOUSE_BUTTON,
  V_INPUT_EVENT_MOUSE_MOVE,
  V_INPUT_EVENT_MOUSE_WHEEL,
  V_INPUT_EVENT_KEY,
} VInputEventType;

typedef struct VInputEvent {
  VInputEventType type;
  union {
    struct {
      VMouseButton button;
      float        x;
      float        y;
      uint8_t      clicks;
      bool         down;
    } mouse_button;
    struct {
      float x;
      float y;
      float relative_x;
      float relative_y;
    } mouse_move;
    struct {
      float x;         /* scroll delta x */
      float y;         /* scroll delta y */
      float mouse_x;   /* cursor x */
      float mouse_y;   /* cursor y */
      float direction; /* 1 for normal, -1 for inverted (e.g. touchpad) */
    } mouse_wheel;
    struct {
      VKey     key;
      uint32_t modifiers;
      int      repeat_count;
      bool     down;
    } key;
  } u;
} VInputEvent;

// ============================================================
// Node events (vuid -> node callbacks)
// ============================================================

typedef enum VNodeEventType {
  // TODO: partially implemented (sends on up, no mouse info in event)
  V_NODE_EVENT_CLICK,
  /*
   * Fired when a key is pressed. The event is delivered to the
   * focused node.
   *
   * - target:         The focused node that initially received
   *                   the key down event.
   * - related_target: null
   * - bubbles:        Yes
   * - cancelable:     Yes
   */
  V_NODE_EVENT_KEY_DOWN,
  /*
   * Fired when a key is released. The event is delivered to the
   * focused node.
   *
   * - target:         The focused node that initially received
   *                   the key up event.
   * - related_target: null
   * - bubbles:        Yes
   * - cancelable:     Yes
   */
  V_NODE_EVENT_KEY_UP,
  /*
   * Fired when the mouse cursor enters the node's visual area.
   *
   * - target:         The node that the cursor entered.
   * - related_target: The node that the cursor left.
   * - bubbles:        Yes
   * - cancelable:     Yes
   */
  V_NODE_EVENT_MOUSE_ENTER,
  /*
   * Fired when the mouse cursor leaves the node's visual area.
   *
   * - target:         The node that the cursor left.
   * - related_target: The node that the cursor entered.
   * - bubbles:        Yes
   * - cancelable:     Yes
   */
  V_NODE_EVENT_MOUSE_LEAVE,
  /*
   * Fired when a node loses focus.
   *
   * - target:         The node that lost focus.
   * - related_target: The node that gained focus as a result of this node losing focus.
   * - bubbles:        Yes
   * - cancelable:     Yes
   */
  V_NODE_EVENT_BLUR,
  /*
   * Fired when a node gains focus.
   *
   * - target:         The node that gained focus.
   * - related_target: The node that lost focus as a result of this node gaining focus.
   * - bubbles:        Yes
   * - cancelable:     Yes
   */
  V_NODE_EVENT_FOCUS,
  V_NODE_EVENT__COUNT,
} VNodeEventType;

typedef struct VNodeEvent {
  VNodeEventType    type;
  VNode*            target;
  VNode*            related_target;
  union {
    struct {
      VKey              key;
      uint32_t          modifiers;
      int               repeat_count;
      bool              down;
    } key_info;
  } u;
  uint32_t          internal;
} VNodeEvent;

typedef void (*VNodeEventListener)(VNode* node, VNodeEvent* event);

// ============================================================
// Node types
// ============================================================

typedef enum VNodeTag {
  V_NODE_ROOT,
  V_NODE_BOX,
  V_NODE_IMAGE,
  V_NODE_TEXT,
} VNodeTag;

typedef enum VPopover {
  V_POPOVER_NONE,
  V_POPOVER_AUTO,
  V_POPOVER_HINT,
  V_POPOVER_MANUAL,
} VPopover;

typedef struct VNodeArray {
  VNode** items;
  size_t  size;
} VNodeArray;

typedef struct VNodeConfig {
  const char*   id;
  const char*   sclass;
  VPopover      popover;
  void*         data;
  VNodeArray    children;
  struct {
    const char* src;
    const char* text;
  } content;
  VNodeEventListener on_click;
  VNodeEventListener on_mouse_enter;
  VNodeEventListener on_mouse_leave;
} VNodeConfig;

// ============================================================
// Render module types
// ============================================================

typedef enum VPixelFormat {
  V_PIXEL_FORMAT_UNKNOWN, /* unknown pixel format */
  V_PIXEL_FORMAT_A8,    /* 1 byte per pixel; alpha only (used by glyph atlas) */
  V_PIXEL_FORMAT_RGBA8, /* 4 bytes per pixel; R, G, B, A in memory order */
} VPixelFormat;

typedef struct VImageBuffer {
  const uint8_t* bytes;
  VPixelFormat   format;
  uint32_t       width;
  uint32_t       height;
  uint32_t       pitch;
  void*          idata;
} VImageBuffer;

typedef enum VTextureState {
  V_TEXTURE_PENDING,
  V_TEXTURE_READY,
  V_TEXTURE_CREATING,
  V_TEXTURE_UPDATING,
  V_TEXTURE_DESTROYING,
  V_TEXTURE_DESTROYED,
} VTextureState;

typedef struct VTextureInfo {
  uint32_t      id;             /* vuid-assigned opaque handle */
  VTextureState state;
  VPixelFormat  texture_format; /* set at create; integration may use a compatible format */
  uint32_t      width;          /* requested size on create; real GPU size when READY */
  uint32_t      height;
  VImageBuffer  pixels;         /* create: empty or must match w/h
                                 * update: non-empty, dimensions must match texture w/h
                                 * ready:  backing pixels retained for GPU restore */
  void*         idata;          /* integration stores its GPU texture handle here */
} VTextureInfo;

/* Data formats */
typedef enum VDataFormat {
    /* Default format, used for 0 initialization where vuid picks the default. */
    V_DATA_FORMAT_DEFAULT = 0,
    /* 8-bit unsigned integer format. */
    V_DATA_FORMAT_U8,
    /* 16-bit unsigned integer format. */
    V_DATA_FORMAT_U16,
    /* 32-bit unsigned integer format. */
    V_DATA_FORMAT_U32,
    /* 32-bit floating point format. */
    V_DATA_FORMAT_F32,
} VDataFormat;

/* Struct field */
typedef struct VStructField {
    /* data format/type of this field */
    VDataFormat format;
    /* byte offset of this field within the struct */
    size_t offset;
} VStructField;

/* Vertex data format. */
typedef struct VVertexStruct {
    VStructField xy;
    VStructField uv;
    VStructField r;
    VStructField g;
    VStructField b;
    VStructField a;
    size_t size;
} VVertexStruct;

typedef enum VCommandType {
  V_COMMAND_RENDER,
  V_COMMAND_SET_CLIP,
} VCommandType;

typedef struct VCommand {
  union {
    struct {
      uint32_t texture_id;
      uint32_t vertex_offset;
      uint32_t vertex_count;
      uint32_t index_offset;
      uint32_t index_count;
    } render;
    struct {
      int x;
      int y;
      int w;
      int h;
      bool clear;
    } set_clip;
  } u;
  VCommandType type;
} VCommand;

typedef struct VRenderData {
  void*                idata0; /* integration stores its platform/window data here */
  void*                idata1; /* integration stores its surface/renderer data here */
  VTextureInfo*        textures;
  const void*          vertices;
  const void*          indices;
  const VCommand*      commands;
  const VVertexStruct* vertex_format;
  uint32_t             texture_count;
  uint32_t             vertex_count;
  uint32_t             index_count;
  uint32_t             command_count;
  VDataFormat          index_format;
} VRenderData;

// ============================================================
// Context config
// ============================================================

/* Memory mode for handling external memory passed to vuid. */
typedef enum VMemoryMode {
  V_MEMORY_MODE_COPY, /* vuid immediately makes a copy of the memory */
  V_MEMORY_MODE_READONLY, /* vuid will not write to this memory and vuid assumes this memory is valid for the duration of its use */
} VMemoryMode;

typedef enum VImageLoaderOp {
  V_IMAGE_LOADER_OP_LOAD,
  V_IMAGE_LOADER_OP_FREE,
} VImageLoaderOp;

typedef bool (*VImageLoaderFn)(VImageLoaderOp op, VImageBuffer* buffer, const void* file_data, size_t file_size);

typedef struct VConfig {
  uint32_t atlas_size;     /* initial glyph atlas size in pixels; 0 = default */
  uint32_t atlas_max_size; /* maximum glyph atlas size in pixels; 0 = default */
  VImageLoaderFn image_loader; /* optional callback for loading images from src */
} VConfig;

// ============================================================
// Context and lifecycle
// ============================================================

VUID_API bool v_init(const VConfig* config);
VUID_API void v_quit(void);
VUID_API void v_process_events(void);
VUID_API void v_update(int width, int height);
VUID_API void v_render(void);

// ============================================================
// Render module
// ============================================================

VUID_API VRenderData* v_get_render_data(void);

// ============================================================
// Input module
// ============================================================

VUID_API void         v_add_input_event(VInputEvent* event);

// ============================================================
// Text module
// ============================================================

VUID_API bool         v_add_font(const char* name, const char* path);
VUID_API bool         v_add_font_mem(const char* name, const void* data, size_t size, VMemoryMode mode);
VUID_API void         v_remove_font(const char* name);

// ============================================================
// Image module
// ============================================================

VUID_API bool         v_add_image(const char* src);
VUID_API bool         v_add_image_mem(const char* src, const void* data, size_t size, VMemoryMode mode);
VUID_API void         v_remove_image(const char* src);

// ============================================================
// Document
// ============================================================

VUID_API VNode*       v_root(void);
VUID_API VNode*       v_get_node_by_id(const char* id);
VUID_API VNode*       v_get_node_by_id_fmt(const char* fmt, ...) VUID_GNUATTR(format(printf, 1, 2));
VUID_API void         v_set_popover_backdrop_color(VColor color);
VUID_API VColor       v_get_popover_backdrop_color(void);
VUID_API VNode*       v_get_focused_node(void);

// ============================================================
// Event API
// ============================================================

VUID_API void v_node_event_stop_propagation(VNodeEvent* event);

// ============================================================
// Node API
// ============================================================

VUID_API VNode*       v_node_new(VNodeTag tag);
VUID_API VNode*       v_node_new_cfg(VNodeTag tag, const VNodeConfig* config);

VUID_API const char*  v_node_id(const VNode* node);
VUID_API void         v_node_set_id(VNode* node, const char* id);
VUID_API void         v_node_set_id_fmt(VNode* node, const char* fmt, ...) VUID_GNUATTR(format(printf, 2, 3));

VUID_API VNodeTag     v_node_tag(const VNode* node);
VUID_API VStyle*      v_node_style(VNode* node);
VUID_API VNode*       v_node_parent(const VNode* node);

VUID_API VNode*       v_node_first_child(const VNode* node);
VUID_API VNode*       v_node_last_child(const VNode* node);
VUID_API VNode*       v_node_next_sibling(const VNode* node);
VUID_API VNode*       v_node_prev_sibling(const VNode* node);

VUID_API int          v_node_child_count(const VNode* node);
VUID_API VNode*       v_node_child_at(const VNode* node, int index);
VUID_API bool         v_node_append_child(VNode* node, VNode* child);
VUID_API bool         v_node_prepend_child(VNode* node, VNode* child);
VUID_API bool         v_node_remove_child(VNode* node, VNode* child);
VUID_API bool         v_node_insert_before(VNode* node, VNode* child, VNode* reference_node);
VUID_API bool         v_node_replace_child(VNode* node, VNode* new_child, VNode* old_child);
VUID_API bool         v_node_remove(VNode* node);
VUID_API void         v_node_remove_children(VNode* node);

VUID_API void         v_node_style_assign(VNode* node, VStyle* style);
VUID_API void         v_node_style_apply(VNode* node, VStyle* style);
VUID_API void         v_node_style_reset(VNode* node);

VUID_API void         v_node_mark_dirty(VNode* node);
VUID_API bool         v_node_is_dirty(const VNode* node);
VUID_API bool         v_node_is_visible(const VNode* node);
VUID_API void         v_node_set_visible(VNode* node, bool is_visible);

VUID_API void*        v_node_data(const VNode* node);
VUID_API void         v_node_set_data(VNode* node, void* data);

VUID_API VPopover     v_node_popover(const VNode* node);
VUID_API void         v_node_set_popover(VNode* node, VPopover type);
VUID_API bool         v_node_show_popover(VNode* node);
VUID_API void         v_node_hide_popover(VNode* node);
VUID_API void         v_node_toggle_popover(VNode* node);

VUID_API bool         v_node_focus(VNode* node);
VUID_API bool         v_node_blur(VNode* node);
VUID_API bool         v_node_has_focus(const VNode* node);

VUID_API void         v_node_set_event_listener(VNode* node, int event_type, VNodeEventListener listener);

VUID_API void         v_node_reset_scroll_y(VNode* node);

VUID_API const char*  v_node_text(const VNode* node);
VUID_API void         v_node_set_text(VNode* node, const char* value);
VUID_API void         v_node_set_text_fmt(VNode* node, const char* fmt, ...) VUID_GNUATTR(format(printf, 2, 3));

VUID_API const char*  v_node_src(const VNode* node);
VUID_API void         v_node_set_src(VNode* node, const char* src);

VUID_API void         v_node_ref(VNode* node);
VUID_API void         v_node_unref(VNode* node);
VUID_API uint32_t     v_node_ref_count(const VNode* node);

// ============================================================
// Style API
// ============================================================

VUID_API float        v_style_measure_text_w(const VStyle* style, const char* text);
VUID_API void         v_style_reset(VStyle* style);
VUID_API void         v_style_ref(VStyle* style);
VUID_API void         v_style_unref(VStyle* style);
VUID_API uint32_t     v_style_ref_count(const VStyle* style);

// ============================================================
// Style sheet
// ============================================================

VUID_API VStyle*      vss_get_class(const char* name);
VUID_API bool         vss_has_class(const char* name);
VUID_API bool         vss_remove_class(const char* name);

/* For use by vss_with / vss_extend macros only */
VUID_API VStyle*      vss__start_class(const char* name, const char* base);
VUID_API void         vss__end_class(const char* name);

static inline void v_node_style_assign_class(VNode* node, const char* name) {
  if (node) v_node_style_assign(node, vss_get_class(name));
}

static inline void v_node_style_apply_class(VNode* node, const char* name) {
  if (node) v_node_style_apply(node, vss_get_class(name));
}

// ============================================================
// Style property accessors (vs_*)
// ============================================================

VUID_API void          vs_set_width(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_width(const VStyle* style);
VUID_API void          vs_unset_width(VStyle* style);
VUID_API bool          vs_has_width(const VStyle* style);

VUID_API void          vs_set_min_width(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_min_width(const VStyle* style);
VUID_API void          vs_unset_min_width(VStyle* style);
VUID_API bool          vs_has_min_width(const VStyle* style);

VUID_API void          vs_set_max_width(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_max_width(const VStyle* style);
VUID_API void          vs_unset_max_width(VStyle* style);
VUID_API bool          vs_has_max_width(const VStyle* style);

VUID_API void          vs_set_height(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_height(const VStyle* style);
VUID_API void          vs_unset_height(VStyle* style);
VUID_API bool          vs_has_height(const VStyle* style);

VUID_API void          vs_set_min_height(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_min_height(const VStyle* style);
VUID_API void          vs_unset_min_height(VStyle* style);
VUID_API bool          vs_has_min_height(const VStyle* style);

VUID_API void          vs_set_max_height(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_max_height(const VStyle* style);
VUID_API void          vs_unset_max_height(VStyle* style);
VUID_API bool          vs_has_max_height(const VStyle* style);

VUID_API void          vs_set_direction(VStyle* style, VDirection value);
VUID_API VDirection    vs_get_direction(const VStyle* style);
VUID_API void          vs_unset_direction(VStyle* style);
VUID_API bool          vs_has_direction(const VStyle* style);

VUID_API void          vs_set_visibility(VStyle* style, VVisibility value);
VUID_API VVisibility   vs_get_visibility(const VStyle* style);
VUID_API void          vs_unset_visibility(VStyle* style);
VUID_API bool          vs_has_visibility(const VStyle* style);

VUID_API void          vs_set_wrap(VStyle* style, VWrap value);
VUID_API VWrap         vs_get_wrap(const VStyle* style);
VUID_API void          vs_unset_wrap(VStyle* style);
VUID_API bool          vs_has_wrap(const VStyle* style);

VUID_API void          vs_set_xalign(VStyle* style, VAlignX value);
VUID_API VAlignX       vs_get_xalign(const VStyle* style);
VUID_API void          vs_unset_xalign(VStyle* style);
VUID_API bool          vs_has_xalign(const VStyle* style);

VUID_API void          vs_set_yalign(VStyle* style, VAlignY value);
VUID_API VAlignY       vs_get_yalign(const VStyle* style);
VUID_API void          vs_unset_yalign(VStyle* style);
VUID_API bool          vs_has_yalign(const VStyle* style);

VUID_API void          vs_set_talign(VStyle* style, VAlignX value);
VUID_API VAlignX       vs_get_talign(const VStyle* style);
VUID_API void          vs_unset_talign(VStyle* style);
VUID_API bool          vs_has_talign(const VStyle* style);

VUID_API void          vs_set_text_wrap(VStyle* style, VTextWrap value);
VUID_API VTextWrap     vs_get_text_wrap(const VStyle* style);
VUID_API void          vs_unset_text_wrap(VStyle* style);
VUID_API bool          vs_has_text_wrap(const VStyle* style);

VUID_API void          vs_set_overflow(VStyle* style, VOverflow value);
VUID_API VOverflow     vs_get_overflow(const VStyle* style);
VUID_API void          vs_unset_overflow(VStyle* style);
VUID_API bool          vs_has_overflow(const VStyle* style);

VUID_API void          vs_set_gap(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_gap(const VStyle* style);
VUID_API void          vs_unset_gap(VStyle* style);
VUID_API bool          vs_has_gap(const VStyle* style);

VUID_API void          vs_set_padding_top(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_padding_top(const VStyle* style);
VUID_API void          vs_unset_padding_top(VStyle* style);
VUID_API bool          vs_has_padding_top(const VStyle* style);

VUID_API void          vs_set_padding_right(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_padding_right(const VStyle* style);
VUID_API void          vs_unset_padding_right(VStyle* style);
VUID_API bool          vs_has_padding_right(const VStyle* style);

VUID_API void          vs_set_padding_bottom(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_padding_bottom(const VStyle* style);
VUID_API void          vs_unset_padding_bottom(VStyle* style);
VUID_API bool          vs_has_padding_bottom(const VStyle* style);

VUID_API void          vs_set_padding_left(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_padding_left(const VStyle* style);
VUID_API void          vs_unset_padding_left(VStyle* style);
VUID_API bool          vs_has_padding_left(const VStyle* style);

VUID_API void          vs_set_margin_top(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_margin_top(const VStyle* style);
VUID_API void          vs_unset_margin_top(VStyle* style);
VUID_API bool          vs_has_margin_top(const VStyle* style);

VUID_API void          vs_set_margin_right(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_margin_right(const VStyle* style);
VUID_API void          vs_unset_margin_right(VStyle* style);
VUID_API bool          vs_has_margin_right(const VStyle* style);

VUID_API void          vs_set_margin_bottom(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_margin_bottom(const VStyle* style);
VUID_API void          vs_unset_margin_bottom(VStyle* style);
VUID_API bool          vs_has_margin_bottom(const VStyle* style);

VUID_API void          vs_set_margin_left(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_margin_left(const VStyle* style);
VUID_API void          vs_unset_margin_left(VStyle* style);
VUID_API bool          vs_has_margin_left(const VStyle* style);

VUID_API void          vs_set_border_top(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_border_top(const VStyle* style);
VUID_API void          vs_unset_border_top(VStyle* style);
VUID_API bool          vs_has_border_top(const VStyle* style);

VUID_API void          vs_set_border_right(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_border_right(const VStyle* style);
VUID_API void          vs_unset_border_right(VStyle* style);
VUID_API bool          vs_has_border_right(const VStyle* style);

VUID_API void          vs_set_border_bottom(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_border_bottom(const VStyle* style);
VUID_API void          vs_unset_border_bottom(VStyle* style);
VUID_API bool          vs_has_border_bottom(const VStyle* style);

VUID_API void          vs_set_border_left(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_border_left(const VStyle* style);
VUID_API void          vs_unset_border_left(VStyle* style);
VUID_API bool          vs_has_border_left(const VStyle* style);

VUID_API void          vs_set_border_radius(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_border_radius(const VStyle* style);
VUID_API void          vs_unset_border_radius(VStyle* style);
VUID_API bool          vs_has_border_radius(const VStyle* style);

VUID_API void          vs_set_scrollbar_width(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_scrollbar_width(const VStyle* style);
VUID_API void          vs_unset_scrollbar_width(VStyle* style);
VUID_API bool          vs_has_scrollbar_width(const VStyle* style);

VUID_API void          vs_set_scrollbar_border_radius(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_scrollbar_border_radius(const VStyle* style);
VUID_API void          vs_unset_scrollbar_border_radius(VStyle* style);
VUID_API bool          vs_has_scrollbar_border_radius(const VStyle* style);

VUID_API void          vs_set_font(VStyle* style, const char* name);
VUID_API const char*   vs_get_font(const VStyle* style);
VUID_API void          vs_unset_font(VStyle* style);
VUID_API bool          vs_has_font(const VStyle* style);

VUID_API void          vs_set_font_size(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_font_size(const VStyle* style);
VUID_API void          vs_unset_font_size(VStyle* style);
VUID_API bool          vs_has_font_size(const VStyle* style);

VUID_API void          vs_set_background(VStyle* style, VColor color);
VUID_API VColor        vs_get_background(const VStyle* style);
VUID_API void          vs_unset_background(VStyle* style);
VUID_API bool          vs_has_background(const VStyle* style);

VUID_API void          vs_set_color(VStyle* style, VColor value);
VUID_API VColor        vs_get_color(const VStyle* style);
VUID_API void          vs_unset_color(VStyle* style);
VUID_API bool          vs_has_color(const VStyle* style);

VUID_API void          vs_set_border_color(VStyle* style, VColor value);
VUID_API VColor        vs_get_border_color(const VStyle* style);
VUID_API void          vs_unset_border_color(VStyle* style);
VUID_API bool          vs_has_border_color(const VStyle* style);

VUID_API void          vs_set_scrollbar_thumb(VStyle* style, VColor thumb);
VUID_API VColor        vs_get_scrollbar_thumb(const VStyle* style);
VUID_API void          vs_unset_scrollbar_thumb(VStyle* style);
VUID_API bool          vs_has_scrollbar_thumb(const VStyle* style);

VUID_API void          vs_set_scrollbar_thumb_hover(VStyle* style, VColor thumb_hover);
VUID_API VColor        vs_get_scrollbar_thumb_hover(const VStyle* style);
VUID_API void          vs_unset_scrollbar_thumb_hover(VStyle* style);
VUID_API bool          vs_has_scrollbar_thumb_hover(const VStyle* style);

VUID_API void          vs_set_anchor_to(VStyle* style, VAnchorTo value);
VUID_API VAnchorTo     vs_get_anchor_to(const VStyle* style);
VUID_API void          vs_unset_anchor_to(VStyle* style);
VUID_API bool          vs_has_anchor_to(const VStyle* style);

VUID_API void          vs_set_position_fallback(VStyle* style, VPositionFallback value);
VUID_API VPositionFallback vs_get_position_fallback(const VStyle* style);
VUID_API void          vs_unset_position_fallback(VStyle* style);
VUID_API bool          vs_has_position_fallback(const VStyle* style);

VUID_API void          vs_set_anchor_attach_point_x(VStyle* style, VAttachPointX value);
VUID_API VAttachPointX vs_get_anchor_attach_point_x(const VStyle* style);
VUID_API void          vs_unset_anchor_attach_point_x(VStyle* style);
VUID_API bool          vs_has_anchor_attach_point_x(const VStyle* style);

VUID_API void          vs_set_anchor_attach_point_y(VStyle* style, VAttachPointY value);
VUID_API VAttachPointY vs_get_anchor_attach_point_y(const VStyle* style);
VUID_API void          vs_unset_anchor_attach_point_y(VStyle* style);
VUID_API bool          vs_has_anchor_attach_point_y(const VStyle* style);

VUID_API void          vs_set_attach_point_x(VStyle* style, VAttachPointX value);
VUID_API VAttachPointX vs_get_attach_point_x(const VStyle* style);
VUID_API void          vs_unset_attach_point_x(VStyle* style);
VUID_API bool          vs_has_attach_point_x(const VStyle* style);

VUID_API void          vs_set_attach_point_y(VStyle* style, VAttachPointY value);
VUID_API VAttachPointY vs_get_attach_point_y(const VStyle* style);
VUID_API void          vs_unset_attach_point_y(VStyle* style);
VUID_API bool          vs_has_attach_point_y(const VStyle* style);

VUID_API void          vs_set_attach_point_offset_x(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_attach_point_offset_x(const VStyle* style);
VUID_API void          vs_unset_attach_point_offset_x(VStyle* style);
VUID_API bool          vs_has_attach_point_offset_x(const VStyle* style);

VUID_API void          vs_set_attach_point_offset_y(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_attach_point_offset_y(const VStyle* style);
VUID_API void          vs_unset_attach_point_offset_y(VStyle* style);
VUID_API bool          vs_has_attach_point_offset_y(const VStyle* style);

VUID_API void          vs_set_aspect_ratio(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_aspect_ratio(const VStyle* style);
VUID_API void          vs_unset_aspect_ratio(VStyle* style);
VUID_API bool          vs_has_aspect_ratio(const VStyle* style);

VUID_API void          vs_set_position(VStyle* style, VPosition value);
VUID_API VPosition     vs_get_position(const VStyle* style);
VUID_API void          vs_unset_position(VStyle* style);
VUID_API bool          vs_has_position(const VStyle* style);

VUID_API void          vs_set_top(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_top(const VStyle* style);
VUID_API void          vs_unset_top(VStyle* style);
VUID_API bool          vs_has_top(const VStyle* style);

VUID_API void          vs_set_right(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_right(const VStyle* style);
VUID_API void          vs_unset_right(VStyle* style);
VUID_API bool          vs_has_right(const VStyle* style);

VUID_API void          vs_set_bottom(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_bottom(const VStyle* style);
VUID_API void          vs_unset_bottom(VStyle* style);
VUID_API bool          vs_has_bottom(const VStyle* style);

VUID_API void          vs_set_left(VStyle* style, VStyleValue value);
VUID_API VStyleValue   vs_get_left(const VStyle* style);
VUID_API void          vs_unset_left(VStyle* style);
VUID_API bool          vs_has_left(const VStyle* style);

static inline void vs_set_padding(VStyle* style, VStyleValue t, VStyleValue r, VStyleValue b, VStyleValue l) {
  vs_set_padding_top(style, t);
  vs_set_padding_right(style, r);
  vs_set_padding_bottom(style, b);
  vs_set_padding_left(style, l);
}

static inline void vs_set_border(VStyle* style, VStyleValue t, VStyleValue r, VStyleValue b, VStyleValue l) {
  vs_set_border_top(style, t);
  vs_set_border_right(style, r);
  vs_set_border_bottom(style, b);
  vs_set_border_left(style, l);
}

static inline void vs_set_margin(VStyle* style, VStyleValue t, VStyleValue r, VStyleValue b, VStyleValue l) {
  vs_set_margin_top(style, t);
  vs_set_margin_right(style, r);
  vs_set_margin_bottom(style, b);
  vs_set_margin_left(style, l);
}

// ============================================================
// Helpers and macros
// ============================================================

static inline VColor v_rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (VColor){r, g, b, 255};
}

static inline VColor v_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return (VColor){r, g, b, a};
}

static inline VStyleValue v_px(float px) {
  return (VStyleValue){V_STYLE_VALUE_UNIT_PX, {.px = px}};
}

static inline VStyleValue v_fit(void) {
  return (VStyleValue){V_STYLE_VALUE_UNIT_FIT, {.px = 0}};
}

static inline VStyleValue v_grow(void) {
  return (VStyleValue){V_STYLE_VALUE_UNIT_GROW, {.px = 0}};
}

static inline VStyleValue v_auto(void) {
  return (VStyleValue){V_STYLE_VALUE_UNIT_AUTO, {.px = 0}};
}

#define v_foreach_child(NODE, CHILD)                            \
  for (VNode* CHILD = v_node_first_child(NODE); CHILD != NULL; \
       CHILD = v_node_next_sibling(CHILD))

#define v_box(...) v_node_new_cfg(V_NODE_BOX,   &(VNodeConfig)__VA_ARGS__)
#define v_txt(...) v_node_new_cfg(V_NODE_TEXT,  &(VNodeConfig)__VA_ARGS__)
#define v_img(...) v_node_new_cfg(V_NODE_IMAGE, &(VNodeConfig)__VA_ARGS__)

#define v_children(...)                                      \
  .children = (VNodeArray) {                                 \
    .items = (VNode*[]){__VA_ARGS__},                        \
    .size = sizeof((VNode*[]){__VA_ARGS__}) / sizeof(VNode*) \
  }

#define vss_with(VAR, NAME)                                      \
  for (VStyle* VAR = vss__start_class(NAME, NULL); VAR != NULL;  \
       (VAR = NULL, vss__end_class(NAME)))

#define vss_extend(VAR, NAME, BASE)                              \
  for (VStyle* VAR = vss__start_class(NAME, BASE); VAR != NULL;  \
       (VAR = NULL, vss__end_class(NAME)))

// clang-format on

#endif  // VUID_H
