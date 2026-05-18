#ifndef SDLI_STYLE_H
#define SDLI_STYLE_H

//
// macros & constants
//

#define FONT_NORMAL (0)
#define FONT_BOLD (1)
#define FONT_ICON (2)

#define THEME_TEXT_FONT_SIZE (16)

#define THEME_ICON_FONT_SIZE (16)

#define THEME_BACKGROUND_1 ((VColor){0x2A, 0x2D, 0x34, 0xFF})
#define THEME_BACKGROUND_2 ((VColor){0x17, 0x1D, 0x25, 0xFF})
#define THEME_TEXT_COLOR ((VColor){0xB8, 0xBC, 0xBF, 0xFF})

#define THEME_SP_2XS (5)
#define THEME_SP_XS (8)
#define THEME_SP_SM (10)
#define THEME_SP_MD (20)
#define THEME_SP_LG (30)

#define CLS_FILL "fill"
#define CLS_TEXT "text"
#define CLS_ICON "icon"
#define CLS_ROOT "root"
#define CLS_CENTER_XY "center_xy"

//
// public functions
//

void LoadStyleSheet(void);

#endif  // SDLI_STYLE_H
