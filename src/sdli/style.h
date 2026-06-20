#ifndef SDLI_STYLE_H
#define SDLI_STYLE_H

#include "util.h"

//
// macros & constants
//

#define FONT_NORMAL "Font-Normal"
#define FONT_BOLD "Font-Bold"
#define FONT_ICON "Font-Icon"

#define THEME_TEXT_FONT_SIZE (16)
#define THEME_TEXT_FONT_SIZE_MD (18)
#define THEME_TEXT_FONT_SIZE_LG (20)

#define THEME_ICON_FONT_SIZE (16)

#define THEME_BACKGROUND_1 ((VColor){0x2A, 0x2D, 0x34, 0xFF})
#define THEME_BACKGROUND_2 ((VColor){0x17, 0x1D, 0x25, 0xFF})
#define THEME_TEXT_COLOR ((VColor){0xB8, 0xBC, 0xBF, 0xFF})
#define THEME_TEXT_HIGHLIGHT_COLOR ((VColor){0xFF, 0xFF, 0xFF, 0xFF})
#define THEME_SCROLLBAR_COLOR ((VColor){0x44, 0x44, 0x44, 0xFF})
#define THEME_HIGHLIGHT_1 ((VColor){0x3D, 0x44, 0x50, 0xFF})
#define THEME_BUTTON_COLOR ((VColor){0x29, 0x2E, 0x36, 0xFF})
#define THEME_APP_TITLE_COLOR ((VColor){0x1A, 0x9F, 0xFF, 0xFF})
#define THEME_UNMAPPED_COLOR ((VColor){0x59, 0x5E, 0x66, 0xFF})
#define THEME_LIST_BORDER_COLOR ((VColor){0x44, 0x44, 0x44, 0xFF})

#define THEME_SP_2XS (5)
#define THEME_SP_XS (8)
#define THEME_SP_SM (10)
#define THEME_SP_MD (20)
#define THEME_SP_LG (30)

#define THEME_BORDER (1)
#define THEME_EVENT_BUTTON_BORDER (2)
#define THEME_BUTTON_CORNER_RADIUS (4)
#define THEME_SCROLLBAR_WIDTH (14)
#define THEME_GAMEPAD_BUTTON_HEIGHT (42)
#define THEME_JOYSTICK_BUTTON_SIZE (48)

#define CLS_FILL "fill"
#define CLS_TEXT "text"
#define CLS_ICON "icon"
#define CLS_ROOT "root"
#define CLS_CENTER_XY "center-xy"
#define CLS_PAGE "page"
#define CLS_PAGE_H1 "page-h1"
#define CLS_PAGE_H2 "page-h2"
#define CLS_BUTTON "btn"
#define CLS_BUTTON_HOVER "btn-hover"
#define CLS_BUTTON_TEXT "btn-text"
#define CLS_BUTTON_TEXT_HOVER "btn-text-hover"
#define CLS_LIST "list"
#define CLS_LIST_ITEM "li"
#define CLS_LIST_ITEM_LAST "li-last"
#define CLS_LIST_ITEM_KEY_TEXT "li-key-text"
#define CLS_LIST_ITEM_VALUE_TEXT "li-value-text"
#define CLS_SCROLLABLE "sbox"
#define CLS_SCROLLABLE_LIST "slist"
#define CLS_OVERLAY "overlay"
#define CLS_OVERLAY_LAYER "overlay-layer"
// TODO: temporary
#define CLS_BUTTON_ROW "btn-group"

//
// public functions
//

void LoadStyleSheet(void);

#endif  // SDLI_STYLE_H
