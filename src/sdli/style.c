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

  vss_with(S, CLS_FILL)
  {
    vs_set_width(S, V_GROW());
    vs_set_height(S, V_GROW());
  }
}
