#include "style.h"

#include <assets.h>
#include <sdli/util.h>

//
// public function implementation
//

void LoadStyleSheet(void)
{
  v_add_font_mem(FONT_NORMAL, asset_font_regular_ttf,
                 asset_font_regular_ttf_len, V_MEMORY_MODE_READONLY);
  v_add_font_mem(FONT_BOLD, asset_font_bold_ttf, asset_font_bold_ttf_len,
                 V_MEMORY_MODE_READONLY);
  v_add_font_mem(FONT_ICON, asset_font_icon_ttf, asset_font_icon_ttf_len,
                 V_MEMORY_MODE_READONLY);

  vss_with(S, CLS_ROOT)
  {
    vs_set_background(S, THEME_BACKGROUND_2);
  }

  vss_with(S, CLS_FILL)
  {
    vs_set_width(S, V_GROW());
    vs_set_height(S, V_GROW());
  }

  vss_with(S, CLS_TEXT)
  {
    vs_set_font(S, FONT_NORMAL);
    vs_set_font_size(S, THEME_TEXT_FONT_SIZE);
    vs_set_color(S, THEME_TEXT_COLOR);
  }

  vss_with(S, CLS_ICON)
  {
    vs_set_font(S, FONT_ICON);
    vs_set_font_size(S, THEME_ICON_FONT_SIZE);
    vs_set_color(S, THEME_TEXT_COLOR);
  }

  // TODO: temporary
  vss_extend(S, CLS_CENTER_XY, CLS_FILL)
  {
    vs_set_xalign(S, V_ALIGN_X_CENTER);
    vs_set_yalign(S, V_ALIGN_Y_CENTER);
  }

  vss_extend(S, CLS_PAGE, CLS_FILL)
  {
    vs_set_direction(S, V_DIRECTION_COLUMN);
    vs_set_padding_all(S, THEME_SP_MD);
  }

  vss_with(S, CLS_PAGE_H1)
  {
    vs_set_color(S, THEME_TEXT_HIGHLIGHT_COLOR);
    vs_set_font(S, FONT_BOLD);
    vs_set_font_size(S, THEME_TEXT_FONT_SIZE_LG);
    vs_set_width(S, V_GROW());
    vs_set_padding_bottom(S, THEME_SP_MD);
  }

  vss_with(S, CLS_PAGE_H2)
  {
    vs_set_font(S, FONT_BOLD);
    vs_set_font_size(S, THEME_TEXT_FONT_SIZE);
    vs_set_color(S, THEME_TEXT_HIGHLIGHT_COLOR);
    vs_set_width(S, V_GROW());
    vs_set_padding(S, THEME_SP_MD, 0, 6, THEME_SP_MD + 1);
  }

  vss_with(S, CLS_BUTTON)
  {
    vs_set_padding(S, THEME_SP_SM, THEME_SP_MD, THEME_SP_SM, THEME_SP_MD);
    vs_set_border_radius(S, THEME_BUTTON_CORNER_RADIUS);
    vs_set_background(S, THEME_BUTTON_COLOR);
    vs_set_gap(S, THEME_SP_SM);
  }

  vss_extend(S, CLS_BUTTON_HOVER, CLS_BUTTON)
  {
    vs_set_background(S, THEME_HIGHLIGHT_1);
  }

  vss_extend(S, CLS_BUTTON_STRETCH, CLS_BUTTON)
  {
    vs_set_width(S, V_GROW());
    vs_set_xalign(S, V_ALIGN_X_CENTER);
  }

  vss_extend(S, CLS_BUTTON_STRETCH_HOVER, CLS_BUTTON_STRETCH)
  {
    vs_set_background(S, THEME_HIGHLIGHT_1);
  }

  vss_extend(S, CLS_MENU_BUTTON, CLS_BUTTON)
  {
    vs_set_width(S, V_GROW());
  }

  vss_extend(S, CLS_MENU_BUTTON_HOVER, CLS_MENU_BUTTON)
  {
    vs_set_background(S, THEME_HIGHLIGHT_1);
  }

  vss_extend(S, CLS_BUTTON_TEXT, CLS_TEXT) {}

  vss_extend(S, CLS_BUTTON_TEXT_HOVER, CLS_BUTTON_TEXT)
  {
    vs_set_color(S, THEME_TEXT_HIGHLIGHT_COLOR);
  }

  vss_extend(S, CLS_BUTTON_ICON, CLS_ICON) {}

  vss_extend(S, CLS_BUTTON_ICON_HOVER, CLS_BUTTON_ICON)
  {
    vs_set_color(S, THEME_TEXT_HIGHLIGHT_COLOR);
  }

  vss_with(S, CLS_MENU)
  {
    vs_set_border_radius(S, THEME_BUTTON_CORNER_RADIUS);
    vs_set_background(S, THEME_BUTTON_COLOR);
    vs_set_direction(S, V_DIRECTION_COLUMN);
    vs_set_attach_point_offset_y(S, THEME_SP_SM);
    vs_set_anchor_to(S, V_ANCHOR_TO_PARENT);
    vs_set_anchor_attach_point_x(S, V_ATTACH_POINT_X_RIGHT);
    vs_set_anchor_attach_point_y(S, V_ATTACH_POINT_Y_BOTTOM);
    vs_set_attach_point_x(S, V_ATTACH_POINT_X_RIGHT);
    vs_set_attach_point_y(S, V_ATTACH_POINT_Y_TOP);
  }

  vss_with(S, CLS_LIST)
  {
    vs_set_direction(S, V_DIRECTION_COLUMN);
    vs_set_padding_all(S, THEME_SP_MD);
    vs_set_width(S, V_GROW());
    vs_set_border_radius(S, THEME_BUTTON_CORNER_RADIUS);
    vs_set_border_all(S, THEME_BORDER);
    vs_set_border_color(S, THEME_LIST_BORDER_COLOR);
  }

  vss_extend(S, CLS_BUTTON_ROW, CLS_LIST)
  {
    vs_set_direction(S, V_DIRECTION_ROW);
    vs_set_gap(S, THEME_SP_SM);
  }

  vss_with(S, CLS_LIST_ITEM)
  {
    vs_set_direction(S, V_DIRECTION_ROW);
    vs_set_width(S, V_GROW());
    vs_set_padding(S, THEME_SP_SM, 0, THEME_SP_SM, 0);
    vs_set_border_bottom(S, THEME_BORDER);
    vs_set_border_color(S, THEME_LIST_BORDER_COLOR);
  }

  vss_extend(S, CLS_LIST_ITEM_LAST, CLS_LIST_ITEM)
  {
    vs_unset_border_bottom(S);
    vs_unset_border_color(S);
  }

  vss_with(S, CLS_LIST_ITEM_KEY_TEXT)
  {
    vs_set_font(S, FONT_NORMAL);
    vs_set_font_size(S, THEME_TEXT_FONT_SIZE);
    vs_set_color(S, THEME_TEXT_HIGHLIGHT_COLOR);
    vs_set_width(S, V_GROW());
  }

  vss_with(S, CLS_LIST_ITEM_VALUE_TEXT)
  {
    vs_set_font(S, FONT_NORMAL);
    vs_set_font_size(S, THEME_TEXT_FONT_SIZE);
    vs_set_color(S, THEME_TEXT_HIGHLIGHT_COLOR);
    vs_set_width(S, V_FIT());
  }

  vss_extend(S, CLS_SCROLLABLE, CLS_FILL)
  {
    vs_set_overflow(S, V_OVERFLOW_SCROLL);
    vs_set_direction(S, V_DIRECTION_COLUMN);
    vs_set_scrollbar_thumb(S, THEME_SCROLLBAR_COLOR);
    vs_set_scrollbar_width(S, THEME_SCROLLBAR_WIDTH);
    vs_set_padding_right(S, THEME_SCROLLBAR_WIDTH + THEME_SP_SM);
  }

  vss_extend(S, CLS_SCROLLABLE_LIST, CLS_SCROLLABLE)
  {
    vs_set_gap(S, THEME_SP_SM);
  }

  vss_extend(S, CLS_OVERLAY, CLS_FILL)
  {
    // TODO: not sure about sizing..
    vs_set_width(S, V_FIXED(400));
    vs_set_height(S, V_FIT(200));
    vs_set_xalign(S, V_ALIGN_X_CENTER);
    vs_set_direction(S, V_DIRECTION_COLUMN);
    vs_set_background(S, THEME_BACKGROUND_2);
    vs_set_gap(S, THEME_SP_XS);
    vs_set_padding_all(S, THEME_SP_MD);
    vs_set_border_radius(S, 14);
    vs_set_border_all(S, THEME_BORDER);
    vs_set_border_color(S, THEME_HIGHLIGHT_1);
  }

  vss_with(S, CLS_OVERLAY_TITLE)
  {
    vs_set_color(S, THEME_TEXT_HIGHLIGHT_COLOR);
    vs_set_font(S, FONT_BOLD);
    vs_set_font_size(S, THEME_TEXT_FONT_SIZE_LG);
    vs_set_padding_bottom(S, THEME_SP_MD);
  }

  vss_extend(S, CLS_OVERLAY_BODY_TEXT, CLS_TEXT)
  {
    vs_set_text_wrap(S, V_TEXT_WRAP_WRAP);
    // TODO: need grow to get wrapping to work.. hmm
    vs_set_width(S, V_GROW());
    vs_set_padding_bottom(S, THEME_SP_MD);
  }

  vss_extend(S, CLS_OVERLAY_LAYER, CLS_FILL)
  {
    vs_set_position(S, V_POSITION_ABSOLUTE);
    vs_set_xalign(S, V_ALIGN_X_CENTER);
    vs_set_yalign(S, V_ALIGN_Y_CENTER);
    vs_set_background(S, v_rgba(0, 0, 0, 128));
  }
}
