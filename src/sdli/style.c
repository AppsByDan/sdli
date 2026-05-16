#include <sdli/style.h>

#include <sdli/assets.h>

#include <vuid.h>

//
// public function implementation
//

void LoadStyleSheet(void)
{
  v_add_font_mem(FONT_NORMAL, SDLI_TTF.data, SDLI_TTF.size);
  v_add_font_mem(FONT_BOLD, SDLI_TTF_BOLD.data, SDLI_TTF_BOLD.size);
  v_add_font_mem(FONT_ICON, SDLI_TTF_ICON.data, SDLI_TTF_ICON.size);

  vss_with(S, CLS_FILL)
  {
    vs_set_width(S, V_GROW());
    vs_set_height(S, V_GROW());
  }

  vss_with(S, CLS_TEXT)
  {
    vs_set_font(S, FONT_NORMAL);
    vs_set_font_size(S, 16);
    vs_set_color(S, THEME_TEXT_COLOR);
  }

  vss_with(S, CLS_ICON)
  {
    vs_set_font(S, FONT_ICON);
    vs_set_font_size(S, THEME_ICON_FONT_SIZE);
    vs_set_color(S, THEME_TEXT_COLOR);
  }
}
