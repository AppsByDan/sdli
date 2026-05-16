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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined __GNUC__
#define VUID_GNUATTR(...) __attribute__((__VA_ARGS__))
#else
#define VUID_GNUATTR(...)
#endif

typedef struct VNode VNode;
typedef struct VStyle VStyle;

typedef struct VGfxFont VGfxFont;
typedef struct VGfxFontFace VGfxFontFace;
typedef struct VGfxTextMesh VGfxTextMesh;
typedef struct VGfxTexture VGfxTexture;

typedef struct VGfxContext {
  void* user_data[4];
} VGfxContext;

typedef enum VDirection {
  V_DIRECTION_ROW,
  V_DIRECTION_COLUMN,
} VDirection;

typedef enum VWrap {
  V_WRAP_NONE,
  V_WRAP_WRAP,
} VWrap;

typedef enum VSizingTag {
  V_SIZING_FIT,
  V_SIZING_GROW,
  V_SIZING_FIXED,
} VSizingTag;

typedef enum VTextWrap {
  V_TEXT_WRAP_WRAP,
  V_TEXT_WRAP_NO_WRAP,
} VTextWrap;

typedef enum VOverflow {
  V_OVERFLOW_VISIBLE,
  V_OVERFLOW_HIDDEN,
  V_OVERFLOW_SCROLL,
} VOverflow;

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

typedef enum VAnchorTo {
  V_ANCHOR_TO_PARENT,
  V_ANCHOR_TO_ROOT,
} VAnchorTo;

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

typedef struct VSizing {
  VSizingTag tag;
  float min;
  float max;
} VSizing;

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

typedef enum VKey {
  V_KEY_INVALID = 0,
  V_KEY_RETURN,
  V_KEY_ESCAPE,
  V_KEY_BACKSPACE,
  V_KEY_TAB,
  V_KEY_SPACE,
  V_KEY_EXCLAIM,
  V_KEY_DBLAPOSTROPHE,
  V_KEY_HASH,
  V_KEY_DOLLAR,
  V_KEY_PERCENT,
  V_KEY_AMPERSAND,
  V_KEY_APOSTROPHE,
  V_KEY_LEFTPAREN,
  V_KEY_RIGHTPAREN,
  V_KEY_ASTERISK,
  V_KEY_PLUS,
  V_KEY_COMMA,
  V_KEY_MINUS,
  V_KEY_PERIOD,
  V_KEY_SLASH,
  V_KEY_0,
  V_KEY_1,
  V_KEY_2,
  V_KEY_3,
  V_KEY_4,
  V_KEY_5,
  V_KEY_6,
  V_KEY_7,
  V_KEY_8,
  V_KEY_9,
  V_KEY_COLON,
  V_KEY_SEMICOLON,
  V_KEY_LESS,
  V_KEY_EQUALS,
  V_KEY_GREATER,
  V_KEY_QUESTION,
  V_KEY_AT,
  V_KEY_LEFTBRACKET,
  V_KEY_BACKSLASH,
  V_KEY_RIGHTBRACKET,
  V_KEY_CARET,
  V_KEY_UNDERSCORE,
  V_KEY_GRAVE,
  V_KEY_A,
  V_KEY_B,
  V_KEY_C,
  V_KEY_D,
  V_KEY_E,
  V_KEY_F,
  V_KEY_G,
  V_KEY_H,
  V_KEY_I,
  V_KEY_J,
  V_KEY_K,
  V_KEY_L,
  V_KEY_M,
  V_KEY_N,
  V_KEY_O,
  V_KEY_P,
  V_KEY_Q,
  V_KEY_R,
  V_KEY_S,
  V_KEY_T,
  V_KEY_U,
  V_KEY_V,
  V_KEY_W,
  V_KEY_X,
  V_KEY_Y,
  V_KEY_Z,
  V_KEY_LEFTBRACE,
  V_KEY_PIPE,
  V_KEY_RIGHTBRACE,
  V_KEY_TILDE,
  V_KEY_DELETE,
  V_KEY_PLUSMINUS,
  V_KEY_CAPSLOCK,
  V_KEY_F1,
  V_KEY_F2,
  V_KEY_F3,
  V_KEY_F4,
  V_KEY_F5,
  V_KEY_F6,
  V_KEY_F7,
  V_KEY_F8,
  V_KEY_F9,
  V_KEY_F10,
  V_KEY_F11,
  V_KEY_F12,
  V_KEY_PRINTSCREEN,
  V_KEY_SCROLLLOCK,
  V_KEY_PAUSE,
  V_KEY_INSERT,
  V_KEY_HOME,
  V_KEY_PAGEUP,
  V_KEY_END,
  V_KEY_PAGEDOWN,
  V_KEY_RIGHT,
  V_KEY_LEFT,
  V_KEY_DOWN,
  V_KEY_UP,
  V_KEY_NUMLOCKCLEAR,
  V_KEY_KP_DIVIDE,
  V_KEY_KP_MULTIPLY,
  V_KEY_KP_MINUS,
  V_KEY_KP_PLUS,
  V_KEY_KP_ENTER,
  V_KEY_KP_1,
  V_KEY_KP_2,
  V_KEY_KP_3,
  V_KEY_KP_4,
  V_KEY_KP_5,
  V_KEY_KP_6,
  V_KEY_KP_7,
  V_KEY_KP_8,
  V_KEY_KP_9,
  V_KEY_KP_0,
  V_KEY_KP_PERIOD,
  V_KEY_APPLICATION,
  V_KEY_POWER,
  V_KEY_KP_EQUALS,
  V_KEY_F13,
  V_KEY_F14,
  V_KEY_F15,
  V_KEY_F16,
  V_KEY_F17,
  V_KEY_F18,
  V_KEY_F19,
  V_KEY_F20,
  V_KEY_F21,
  V_KEY_F22,
  V_KEY_F23,
  V_KEY_F24,
  V_KEY_EXECUTE,
  V_KEY_HELP,
  V_KEY_MENU,
  V_KEY_SELECT,
  V_KEY_STOP,
  V_KEY_AGAIN,
  V_KEY_UNDO,
  V_KEY_CUT,
  V_KEY_COPY,
  V_KEY_PASTE,
  V_KEY_FIND,
  V_KEY_MUTE,
  V_KEY_VOLUMEUP,
  V_KEY_VOLUMEDOWN,
  V_KEY_KP_COMMA,
  V_KEY_KP_EQUALSAS400,
  V_KEY_ALTERASE,
  V_KEY_SYSREQ,
  V_KEY_CANCEL,
  V_KEY_CLEAR,
  V_KEY_PRIOR,
  V_KEY_RETURN2,
  V_KEY_SEPARATOR,
  V_KEY_OUT,
  V_KEY_OPER,
  V_KEY_CLEARAGAIN,
  V_KEY_CRSEL,
  V_KEY_EXSEL,
  V_KEY_KP_00,
  V_KEY_KP_000,
  V_KEY_THOUSANDSSEPARATOR,
  V_KEY_DECIMALSEPARATOR,
  V_KEY_CURRENCYUNIT,
  V_KEY_CURRENCYSUBUNIT,
  V_KEY_KP_LEFTPAREN,
  V_KEY_KP_RIGHTPAREN,
  V_KEY_KP_LEFTBRACE,
  V_KEY_KP_RIGHTBRACE,
  V_KEY_KP_TAB,
  V_KEY_KP_BACKSPACE,
  V_KEY_KP_A,
  V_KEY_KP_B,
  V_KEY_KP_C,
  V_KEY_KP_D,
  V_KEY_KP_E,
  V_KEY_KP_F,
  V_KEY_KP_XOR,
  V_KEY_KP_POWER,
  V_KEY_KP_PERCENT,
  V_KEY_KP_LESS,
  V_KEY_KP_GREATER,
  V_KEY_KP_AMPERSAND,
  V_KEY_KP_DBLAMPERSAND,
  V_KEY_KP_VERTICALBAR,
  V_KEY_KP_DBLVERTICALBAR,
  V_KEY_KP_COLON,
  V_KEY_KP_HASH,
  V_KEY_KP_SPACE,
  V_KEY_KP_AT,
  V_KEY_KP_EXCLAM,
  V_KEY_KP_MEMSTORE,
  V_KEY_KP_MEMRECALL,
  V_KEY_KP_MEMCLEAR,
  V_KEY_KP_MEMADD,
  V_KEY_KP_MEMSUBTRACT,
  V_KEY_KP_MEMMULTIPLY,
  V_KEY_KP_MEMDIVIDE,
  V_KEY_KP_PLUSMINUS,
  V_KEY_KP_CLEAR,
  V_KEY_KP_CLEARENTRY,
  V_KEY_KP_BINARY,
  V_KEY_KP_OCTAL,
  V_KEY_KP_DECIMAL,
  V_KEY_KP_HEXADECIMAL,
  V_KEY_LCTRL,
  V_KEY_LSHIFT,
  V_KEY_LALT,
  V_KEY_LGUI,
  V_KEY_RCTRL,
  V_KEY_RSHIFT,
  V_KEY_RALT,
  V_KEY_RGUI,
  V_KEY_MODE,
  V_KEY_SLEEP,
  V_KEY_WAKE,
  V_KEY_CHANNEL_INCREMENT,
  V_KEY_CHANNEL_DECREMENT,
  V_KEY_MEDIA_PLAY,
  V_KEY_MEDIA_PAUSE,
  V_KEY_MEDIA_RECORD,
  V_KEY_MEDIA_FAST_FORWARD,
  V_KEY_MEDIA_REWIND,
  V_KEY_MEDIA_NEXT_TRACK,
  V_KEY_MEDIA_PREVIOUS_TRACK,
  V_KEY_MEDIA_STOP,
  V_KEY_MEDIA_EJECT,
  V_KEY_MEDIA_PLAY_PAUSE,
  V_KEY_MEDIA_SELECT,
  V_KEY_AC_NEW,
  V_KEY_AC_OPEN,
  V_KEY_AC_CLOSE,
  V_KEY_AC_EXIT,
  V_KEY_AC_SAVE,
  V_KEY_AC_PRINT,
  V_KEY_AC_PROPERTIES,
  V_KEY_AC_SEARCH,
  V_KEY_AC_HOME,
  V_KEY_AC_BACK,
  V_KEY_AC_FORWARD,
  V_KEY_AC_STOP,
  V_KEY_AC_REFRESH,
  V_KEY_AC_BOOKMARKS,
  V_KEY_SOFTLEFT,
  V_KEY_SOFTRIGHT,
  V_KEY_CALL,
  V_KEY_ENDCALL,
  V_KEY_LEFT_TAB,
  V_KEY_LEVEL5_SHIFT,
  V_KEY_MULTI_KEY_COMPOSE,
  V_KEY_LMETA,
  V_KEY_RMETA,
  V_KEY_LHYPER,
  V_KEY_RHYPER,

} VKey;

typedef enum VKeyMod {
  V_KMOD_NONE = 0,
  V_KMOD_LSHIFT = 1 << 0,
  V_KMOD_RSHIFT = 1 << 1,
  V_KMOD_LCTRL = 1 << 2,
  V_KMOD_RCTRL = 1 << 3,
  V_KMOD_LALT = 1 << 4,
  V_KMOD_RALT = 1 << 5,
  V_KMOD_LGUI = 1 << 6,
  V_KMOD_RGUI = 1 << 7,
  V_KMOD_NUM = 1 << 8,
  V_KMOD_CAPS = 1 << 9,
  V_KMOD_MODE = 1 << 10,
  V_KMOD_SCROLL = 1 << 11,
} VKeyMod;

typedef struct VEvent {
  int type;
  VNode* target;
} VEvent;

typedef struct VKeyUpEvent {
  int type;
  VNode* target;
  VKey key;
  VKeyMod modifiers;
} VKeyUpEvent;

typedef struct VKeyDownEvent {
  int type;
  VNode* target;
  VKey key;
  VKeyMod modifiers;
  int repeat_count;
} VKeyDownEvent;

typedef enum VEventType {
  V_EVENT_CLICK,
  V_EVENT_MOUSE_ENTER,
  V_EVENT_MOUSE_LEAVE,
  V_EVENT_KEY_DOWN,
  V_EVENT_KEY_UP,
  V_EVENT_USER,
  V_EVENT__COUNT,
} VEventType;

typedef void (*VEventListener)(VNode* listener, VEvent* event);

#define V_EVENT_USER_BASE (0xFFFF)

typedef enum VMouseButton {
  V_MOUSE_BUTTON_INVALID = 0,
  V_MOUSE_BUTTON_LEFT = 1,
  V_MOUSE_BUTTON_RIGHT = 2,
  V_MOUSE_BUTTON_MIDDLE = 3,
} VMouseButton;

typedef struct VMouseButtonData {
  VMouseButton button;
  float x;
  float y;
  uint8_t clicks;
  bool down;
} VMouseButtonData;

typedef struct VMouseWheelData {
  float x;
  float y;
  float mouse_x;
  float mouse_y;
  float direction;
} VMouseWheelData;

typedef struct VMouseMoveData {
  float x;
  float y;
  float relative_x;
  float relative_y;
} VMouseMoveData;

typedef struct VNodeArray {
  VNode** items;
  size_t size;
} VNodeArray;

typedef struct VNodeConfig {
  const char* id;
  const char* sclass;
  VPopover popover;
  void* data;
  VNodeArray children;
  struct {
    const char* src;
    const char* text;
  } content;
  VEventListener on_click;
  VEventListener on_mouse_enter;
  VEventListener on_mouse_leave;
  VEventListener on_user;
} VNodeConfig;

#define V_SDL3_RENDERER_ID (0)
#define V_SDL3_TEXT_ENGINE_ID (1)

typedef struct VConfig {
  VGfxContext gfx_context;
} VConfig;

// clang-format off

bool        v_init(const VConfig* config);
void        v_quit(void);
void        v_layout(int width, int height);
void        v_draw(void);
void        v_inject_mouse_button(const VMouseButtonData* data);
void        v_inject_mouse_move(const VMouseMoveData* data);
void        v_inject_mouse_wheel(const VMouseWheelData* data);
void        v_inject_key(VKey key, uint32_t modifiers, int repeat_count, bool down);
bool        v_add_font(uint16_t id, const char* path);
bool        v_add_font_mem(uint16_t id, const void* data, size_t size);
bool        v_add_font_raw(uint16_t id, VGfxFont* gfx_font, bool owns_gfx_font);
void        v_remove_font(uint16_t id);
bool        v_add_image(const char* src);
bool        v_add_image_mem(const char* src, const void* data, size_t size);
bool        v_add_image_raw(const char* src, VGfxTexture* gfx_texture, VSize size, bool owns_gfx_texture);
void        v_remove_image(const char* src);
VNode*      v_root(void);
VNode*      v_get_node_by_id(const char* id);
VNode*      v_get_node_by_id_fmt(const char* fmt, ...) VUID_GNUATTR(format(printf, 1, 2));
VStyle*     v_get_popover_layer_style(void);

VNode*      v_new_node(VNodeTag tag);
VNode*      v_new_node_with_cfg(VNodeTag tag, const VNodeConfig* config);

const char* v_node_id(const VNode* node);
void        v_node_set_id(VNode* node, const char* id);
void        v_node_set_id_fmt(VNode* node, const char* fmt, ...) VUID_GNUATTR(format(printf, 2, 3));

VNodeTag    v_node_tag(const VNode* node);
VStyle*     v_node_style(VNode* node);
VNode*      v_node_parent(const VNode* node);

VNode*      v_node_first_child(const VNode* node);
VNode*      v_node_last_child(const VNode* node);
VNode*      v_node_next_sibling(const VNode* node);
VNode*      v_node_prev_sibling(const VNode* node);

int         v_node_child_count(const VNode* node);
VNode*      v_node_child_at(const VNode* node, int index);
bool        v_node_insert_before(VNode* node, VNode* child, VNode* reference_node);
bool        v_node_append_child(VNode* node, VNode* child);
bool        v_node_prepend_child(VNode* node, VNode* child);
bool        v_node_remove_child(VNode* node, VNode* child);
bool        v_node_replace_child(VNode* node, VNode* new_child, VNode* old_child);
void        v_node_remove_children(VNode* node);

void        v_node_style_assign(VNode* node, VStyle* style);
void        v_node_style_apply(VNode* node, VStyle* style);
void        v_node_style_reset(VNode* node);

void        v_node_mark_dirty(VNode* node);
bool        v_node_is_dirty(const VNode* node);

bool        v_node_is_visible(const VNode* node);
void        v_node_set_visible(VNode* node, bool is_visible);

void*       v_node_data(const VNode* node);
void        v_node_set_data(VNode* node, void* data);

void        v_node_dispatch_user_event(VNode* node, VEvent* event);

VPopover    v_node_popover(const VNode* node);
void        v_node_set_popover(VNode* node, VPopover type);
bool        v_node_show_popover(VNode* node);
void        v_node_hide_popover(VNode* node);

void        v_node_set_event_listener(VNode* node, int event_type, VEventListener listener);

void        v_node_reset_scroll_y(VNode* node);

const char* v_node_text(const VNode* node);
void        v_node_set_text(VNode* node, const char* value);
void        v_node_set_text_fmt(VNode* node, const char* fmt, ...) VUID_GNUATTR(format(printf, 2, 3));

const char* v_node_src(const VNode* node);
void        v_node_set_src(VNode* node, const char* src);

void        v_node_ref(VNode* node);
void        v_node_unref(VNode* node);
uint32_t    v_node_ref_count(const VNode* node);

// clang-format on

// clang-format off

float       v_style_measure_text_w(const VStyle* style, const char* text);
void        v_style_reset(VStyle* style);

void        v_style_ref(VStyle* style);
void        v_style_unref(VStyle* style);
uint32_t    v_style_ref_count(const VStyle* style);

// clang-format on

// clang-format off

VStyle* vss_get_class(const char* name);
bool    vss_has_class(const char* name);
bool    vss_remove_class(const char* name);

VStyle* vss__start_class(const char* name, const char* base);
void    vss__end_class(const char* name);

static inline void v_node_style_assign_class(VNode* node, const char* name) {
  if (node) v_node_style_assign(node, vss_get_class(name));
}

static inline void v_node_style_apply_class(VNode* node, const char* name) {
  if (node) v_node_style_apply(node, vss_get_class(name));
}

// clang-format on

// clang-format off

void       vs_set_width(VStyle* style, VSizing value);
VSizing    vs_get_width(const VStyle* style);
void       vs_unset_width(VStyle* style);
bool       vs_has_width(const VStyle* style);

void       vs_set_height(VStyle* style, VSizing value);
VSizing    vs_get_height(const VStyle* style);
void       vs_unset_height(VStyle* style);
bool       vs_has_height(const VStyle* style);

void       vs_set_direction(VStyle* style, VDirection value);
VDirection vs_get_direction(const VStyle* style);
void       vs_unset_direction(VStyle* style);
bool       vs_has_direction(const VStyle* style);

void       vs_set_wrap(VStyle* style, VWrap value);
VWrap      vs_get_wrap(const VStyle* style);
void       vs_unset_wrap(VStyle* style);
bool       vs_has_wrap(const VStyle* style);

void       vs_set_xalign(VStyle* style, VAlignX value);
VAlignX    vs_get_xalign(const VStyle* style);
void       vs_unset_xalign(VStyle* style);
bool       vs_has_xalign(const VStyle* style);

void       vs_set_yalign(VStyle* style, VAlignY value);
VAlignY    vs_get_yalign(const VStyle* style);
void       vs_unset_yalign(VStyle* style);
bool       vs_has_yalign(const VStyle* style);

void       vs_set_talign(VStyle* style, VAlignX value);
VAlignX    vs_get_talign(const VStyle* style);
void       vs_unset_talign(VStyle* style);
bool       vs_has_talign(const VStyle* style);

void       vs_set_text_wrap(VStyle* style, VTextWrap value);
VTextWrap  vs_get_text_wrap(const VStyle* style);
void       vs_unset_text_wrap(VStyle* style);
bool       vs_has_text_wrap(const VStyle* style);

void       vs_set_overflow(VStyle* style, VOverflow value);
VOverflow  vs_get_overflow(const VStyle* style);
void       vs_unset_overflow(VStyle* style);
bool       vs_has_overflow(const VStyle* style);

void       vs_set_gap(VStyle* style, uint16_t value);
uint16_t   vs_get_gap(const VStyle* style);
void       vs_unset_gap(VStyle* style);
bool       vs_has_gap(const VStyle* style);

void       vs_set_padding_top(VStyle* style, uint16_t value);
uint16_t   vs_get_padding_top(const VStyle* style);
void       vs_unset_padding_top(VStyle* style);
bool       vs_has_padding_top(const VStyle* style);

void       vs_set_padding_right(VStyle* style, uint16_t value);
uint16_t   vs_get_padding_right(const VStyle* style);
void       vs_unset_padding_right(VStyle* style);
bool       vs_has_padding_right(const VStyle* style);

void       vs_set_padding_bottom(VStyle* style, uint16_t value);
uint16_t   vs_get_padding_bottom(const VStyle* style);
void       vs_unset_padding_bottom(VStyle* style);
bool       vs_has_padding_bottom(const VStyle* style);

void       vs_set_padding_left(VStyle* style, uint16_t value);
uint16_t   vs_get_padding_left(const VStyle* style);
void       vs_unset_padding_left(VStyle* style);
bool       vs_has_padding_left(const VStyle* style);

void       vs_set_border_top(VStyle* style, uint16_t value);
uint16_t   vs_get_border_top(const VStyle* style);
void       vs_unset_border_top(VStyle* style);
bool       vs_has_border_top(const VStyle* style);

void       vs_set_border_right(VStyle* style, uint16_t value);
uint16_t   vs_get_border_right(const VStyle* style);
void       vs_unset_border_right(VStyle* style);
bool       vs_has_border_right(const VStyle* style);

void       vs_set_border_bottom(VStyle* style, uint16_t value);
uint16_t   vs_get_border_bottom(const VStyle* style);
void       vs_unset_border_bottom(VStyle* style);
bool       vs_has_border_bottom(const VStyle* style);

void       vs_set_border_left(VStyle* style, uint16_t value);
uint16_t   vs_get_border_left(const VStyle* style);
void       vs_unset_border_left(VStyle* style);
bool       vs_has_border_left(const VStyle* style);

void       vs_set_border_radius(VStyle* style, uint16_t value);
uint16_t   vs_get_border_radius(const VStyle* style);
void       vs_unset_border_radius(VStyle* style);
bool       vs_has_border_radius(const VStyle* style);

void       vs_set_scrollbar_width(VStyle* style, uint16_t value);
uint16_t   vs_get_scrollbar_width(const VStyle* style);
void       vs_unset_scrollbar_width(VStyle* style);
bool       vs_has_scrollbar_width(const VStyle* style);

void       vs_set_scrollbar_border_radius(VStyle* style, uint16_t value);
uint16_t   vs_get_scrollbar_border_radius(const VStyle* style);
void       vs_unset_scrollbar_border_radius(VStyle* style);
bool       vs_has_scrollbar_border_radius(const VStyle* style);

void       vs_set_font(VStyle* style, uint16_t value);
uint16_t   vs_get_font(const VStyle* style);
void       vs_unset_font(VStyle* style);
bool       vs_has_font(const VStyle* style);

void       vs_set_font_size(VStyle* style, uint16_t value);
uint16_t   vs_get_font_size(const VStyle* style);
void       vs_unset_font_size(VStyle* style);
bool       vs_has_font_size(const VStyle* style);

void       vs_set_background(VStyle* style, VColor color);
VColor     vs_get_background(const VStyle* style);
void       vs_unset_background(VStyle* style);
bool       vs_has_background(const VStyle* style);

void       vs_set_color(VStyle* style, VColor value);
VColor     vs_get_color(const VStyle* style);
void       vs_unset_color(VStyle* style);
bool       vs_has_color(const VStyle* style);

void       vs_set_border_color(VStyle* style, VColor value);
VColor     vs_get_border_color(const VStyle* style);
void       vs_unset_border_color(VStyle* style);
bool       vs_has_border_color(const VStyle* style);

void       vs_set_scrollbar_thumb(VStyle* style, VColor thumb);
VColor     vs_get_scrollbar_thumb(const VStyle* style);
void       vs_unset_scrollbar_thumb(VStyle* style);
bool       vs_has_scrollbar_thumb(const VStyle* style);

void       vs_set_scrollbar_thumb_hover(VStyle* style, VColor thumb_hover);
VColor     vs_get_scrollbar_thumb_hover(const VStyle* style);
void       vs_unset_scrollbar_thumb_hover(VStyle* style);
bool       vs_has_scrollbar_thumb_hover(const VStyle* style);

void       vs_set_anchor_to(VStyle* style, VAnchorTo value);
VAnchorTo vs_get_anchor_to(const VStyle* style);
void       vs_unset_anchor_to(VStyle* style);
bool       vs_has_anchor_to(const VStyle* style);

void       vs_set_anchor_attach_point_x(VStyle* style, VAttachPointX value);
VAttachPointX vs_get_anchor_attach_point_x(const VStyle* style);
void       vs_unset_anchor_attach_point_x(VStyle* style);
bool       vs_has_anchor_attach_point_x(const VStyle* style);

void       vs_set_anchor_attach_point_y(VStyle* style, VAttachPointY value);
VAttachPointY vs_get_anchor_attach_point_y(const VStyle* style);
void       vs_unset_anchor_attach_point_y(VStyle* style);
bool       vs_has_anchor_attach_point_y(const VStyle* style);

void       vs_set_attach_point_x(VStyle* style, VAttachPointX value);
VAttachPointX vs_get_attach_point_x(const VStyle* style);
void       vs_unset_attach_point_x(VStyle* style);
bool       vs_has_attach_point_x(const VStyle* style);

void       vs_set_attach_point_y(VStyle* style, VAttachPointY value);
VAttachPointY vs_get_attach_point_y(const VStyle* style);
void       vs_unset_attach_point_y(VStyle* style);
bool       vs_has_attach_point_y(const VStyle* style);

void       vs_set_attach_point_offset_x(VStyle* style, float value);
float      vs_get_attach_point_offset_x(const VStyle* style);
void       vs_unset_attach_point_offset_x(VStyle* style);
bool       vs_has_attach_point_offset_x(const VStyle* style);

void       vs_set_attach_point_offset_y(VStyle* style, float value);
float      vs_get_attach_point_offset_y(const VStyle* style);
void       vs_unset_attach_point_offset_y(VStyle* style);
bool       vs_has_attach_point_offset_y(const VStyle* style);

void       vs_set_aspect_ratio(VStyle* style, float value);
float      vs_get_aspect_ratio(const VStyle* style);
void       vs_unset_aspect_ratio(VStyle* style);
bool       vs_has_aspect_ratio(const VStyle* style);

static inline void vs_set_padding(VStyle* style, uint16_t t, uint16_t r, uint16_t b, uint16_t l) {
  vs_set_padding_top(style, t);
  vs_set_padding_right(style, r);
  vs_set_padding_bottom(style, b);
  vs_set_padding_left(style, l);
}

static inline void vs_set_border(VStyle* style, uint16_t t, uint16_t r, uint16_t b, uint16_t l) {
  vs_set_border_top(style, t);
  vs_set_border_right(style, r);
  vs_set_border_bottom(style, b);
  vs_set_border_left(style, l);
}

// clang-format on

typedef enum VGfxFeature {
  V_GFX_USES_FONT_FACE = 1 << 0,
  V_GFX_USES_TEXT_MESH = 1 << 1,
} VGfxFeature;

typedef enum VGfxObjectType {
  V_GFX_OBJECT_FONT,
  V_GFX_OBJECT_FONT_FACE,
  V_GFX_OBJECT_TEXT_MESH,
  V_GFX_OBJECT_IMAGE,
} VGfxObjectType;

typedef enum VGfxMeasureMode {
  V_GFX_MEASURE_MODE_TEXT,
  V_GFX_MEASURE_MODE_TEXT_WRAPPED,
} VGfxMeasureMode;

typedef enum VGfxDataSourceType {
  V_GFX_DATA_SOURCE_PATH,
  V_GFX_DATA_SOURCE_BUFFER,
} VGfxDataSourceType;

typedef struct VGfxDataSource {
  union {
    const char* path;
    struct {
      const void* data;
      size_t size;
    } buffer;
  } u;
  VGfxDataSourceType type;
} VGfxDataSource;

// clang-format off
uint32_t      v_gfx_features(const VGfxContext* gfx);
VGfxFont*     v_gfx_font_new(const VGfxContext* gfx, const VGfxDataSource* source);
VGfxFontFace* v_gfx_font_face_new(const VGfxContext* gfx, VGfxFont* font, float font_size);
bool          v_gfx_text_mesh_update(const VGfxContext* gfx, VGfxTextMesh** text_mesh_inout, const char* str, size_t str_size);
VGfxTexture*  v_gfx_new_image(const VGfxContext* gfx, const VGfxDataSource* source, VSize* size_out);
void          v_gfx_object_free(const VGfxContext* gfx, VGfxObjectType type, void* object);
void          v_gfx_text_mesh_clear_font(const VGfxContext* gfx, VGfxTextMesh* text_mesh);
VSize         v_gfx_measure_string(const VGfxContext* gfx, VGfxFont* font, float font_size, VGfxFontFace* font_face, const char* text, size_t text_size, VGfxMeasureMode mode, int wrap_width);
void          v_gfx_clear(const VGfxContext* gfx, const VRect* rect, const VColor* color);
void          v_gfx_draw_rect(const VGfxContext* gfx, const VRect* rect, const VColor* color, uint16_t bt, uint16_t br, uint16_t bb, uint16_t bl, uint16_t border_radius);
void          v_gfx_draw_fill_rect(const VGfxContext* gfx, const VRect* rect, const VColor* color, uint16_t border_radius);
void          v_gfx_draw_text(const VGfxContext* gfx, VGfxTextMesh* text_mesh, const char* str, const VRect* content_box, bool wrap, VAlignX align, VGfxFont* font, float font_size, VGfxFontFace* font_face, const VColor* color);
void          v_gfx_draw_image(const VGfxContext* gfx, const VRect* rect, VGfxTexture* texture, const VColor* color);
void          v_gfx_set_clip(const VGfxContext* gfx, float x, float y, float w, float h);
// clang-format on

// clang-format off
bool         v_itx_map_key(int virtual_key, int physical_key, int raw_modifiers, VKey* key_out, uint32_t* modifiers_out);
VMouseButton v_itx_map_mouse_button(int button);
// clang-format on

static inline VColor v_rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (VColor){r, g, b, 255};
}

static inline VColor v_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return (VColor){r, g, b, a};
}

#define v_foreach_child(NODE, CHILD)                           \
  for (VNode* CHILD = v_node_first_child(NODE); CHILD != NULL; \
       CHILD = v_node_next_sibling(CHILD))

#define v_box(...) v_new_node_with_cfg(V_NODE_BOX, &(VNodeConfig)__VA_ARGS__)
#define v_txt(...) v_new_node_with_cfg(V_NODE_TEXT, &(VNodeConfig)__VA_ARGS__)
#define v_img(...) v_new_node_with_cfg(V_NODE_IMAGE, &(VNodeConfig)__VA_ARGS__)

#define v_children(...)                                      \
  .children = (VNodeArray) {                                 \
    .items = (VNode*[]){__VA_ARGS__},                        \
    .size = sizeof((VNode*[]){__VA_ARGS__}) / sizeof(VNode*) \
  }

#define vss_with(VAR, NAME)                                     \
  for (VStyle* VAR = vss__start_class(NAME, NULL); VAR != NULL; \
       (VAR = NULL, vss__end_class(NAME)))

#define vss_extend(VAR, NAME, BASE)                             \
  for (VStyle* VAR = vss__start_class(NAME, BASE); VAR != NULL; \
       (VAR = NULL, vss__end_class(NAME)))

// TODO: should these be VS?
#define V_FIXED(VALUE) ((VSizing){V_SIZING_FIXED, VALUE, VALUE})
#define V_FIT(...) ((VSizing){V_SIZING_FIT, __VA_ARGS__})
#define V_GROW(...) ((VSizing){V_SIZING_GROW, __VA_ARGS__})

#endif  // VUID_H
