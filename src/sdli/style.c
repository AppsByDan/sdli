#include <sdli/style.h>

#include <sdli/assets.h>

#include <vuid.h>

void LoadStyleSheet(void)
{
  v_add_font_mem(FONT_NORMAL, SDLI_TTF.data, SDLI_TTF.size);
  v_add_font_mem(FONT_BOLD, SDLI_TTF_BOLD.data, SDLI_TTF_BOLD.size);
}
