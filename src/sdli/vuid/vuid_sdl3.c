#include "vuid_sdl3.h"

#include <sdli/util.h>
#include <SDL3_ttf/SDL_ttf.h>

#define V_GET_RENDERER(GFX) \
  ((SDL_Renderer*)(GFX)->user_data[V_SDL3_RENDERER_ID])
#define V_GET_TEXT_ENGINE(GFX) \
  ((TTF_TextEngine*)(GFX)->user_data[V_SDL3_TEXT_ENGINE_ID])

static const VKey SDLK_TO_KEY[178] = {
    [SDLK_RETURN] = V_KEY_RETURN,
    [SDLK_ESCAPE] = V_KEY_ESCAPE,
    [SDLK_BACKSPACE] = V_KEY_BACKSPACE,
    [SDLK_TAB] = V_KEY_TAB,
    [SDLK_SPACE] = V_KEY_SPACE,
    [SDLK_EXCLAIM] = V_KEY_EXCLAIM,
    [SDLK_DBLAPOSTROPHE] = V_KEY_DBLAPOSTROPHE,
    [SDLK_HASH] = V_KEY_HASH,
    [SDLK_DOLLAR] = V_KEY_DOLLAR,
    [SDLK_PERCENT] = V_KEY_PERCENT,
    [SDLK_AMPERSAND] = V_KEY_AMPERSAND,
    [SDLK_APOSTROPHE] = V_KEY_APOSTROPHE,
    [SDLK_LEFTPAREN] = V_KEY_LEFTPAREN,
    [SDLK_RIGHTPAREN] = V_KEY_RIGHTPAREN,
    [SDLK_ASTERISK] = V_KEY_ASTERISK,
    [SDLK_PLUS] = V_KEY_PLUS,
    [SDLK_COMMA] = V_KEY_COMMA,
    [SDLK_MINUS] = V_KEY_MINUS,
    [SDLK_PERIOD] = V_KEY_PERIOD,
    [SDLK_SLASH] = V_KEY_SLASH,
    [SDLK_0] = V_KEY_0,
    [SDLK_1] = V_KEY_1,
    [SDLK_2] = V_KEY_2,
    [SDLK_3] = V_KEY_3,
    [SDLK_4] = V_KEY_4,
    [SDLK_5] = V_KEY_5,
    [SDLK_6] = V_KEY_6,
    [SDLK_7] = V_KEY_7,
    [SDLK_8] = V_KEY_8,
    [SDLK_9] = V_KEY_9,
    [SDLK_COLON] = V_KEY_COLON,
    [SDLK_SEMICOLON] = V_KEY_SEMICOLON,
    [SDLK_LESS] = V_KEY_LESS,
    [SDLK_EQUALS] = V_KEY_EQUALS,
    [SDLK_GREATER] = V_KEY_GREATER,
    [SDLK_QUESTION] = V_KEY_QUESTION,
    [SDLK_AT] = V_KEY_AT,
    [SDLK_LEFTBRACKET] = V_KEY_LEFTBRACKET,
    [SDLK_BACKSLASH] = V_KEY_BACKSLASH,
    [SDLK_RIGHTBRACKET] = V_KEY_RIGHTBRACKET,
    [SDLK_CARET] = V_KEY_CARET,
    [SDLK_UNDERSCORE] = V_KEY_UNDERSCORE,
    [SDLK_GRAVE] = V_KEY_GRAVE,
    [SDLK_A] = V_KEY_A,
    [SDLK_B] = V_KEY_B,
    [SDLK_C] = V_KEY_C,
    [SDLK_D] = V_KEY_D,
    [SDLK_E] = V_KEY_E,
    [SDLK_F] = V_KEY_F,
    [SDLK_G] = V_KEY_G,
    [SDLK_H] = V_KEY_H,
    [SDLK_I] = V_KEY_I,
    [SDLK_J] = V_KEY_J,
    [SDLK_K] = V_KEY_K,
    [SDLK_L] = V_KEY_L,
    [SDLK_M] = V_KEY_M,
    [SDLK_N] = V_KEY_N,
    [SDLK_O] = V_KEY_O,
    [SDLK_P] = V_KEY_P,
    [SDLK_Q] = V_KEY_Q,
    [SDLK_R] = V_KEY_R,
    [SDLK_S] = V_KEY_S,
    [SDLK_T] = V_KEY_T,
    [SDLK_U] = V_KEY_U,
    [SDLK_V] = V_KEY_V,
    [SDLK_W] = V_KEY_W,
    [SDLK_X] = V_KEY_X,
    [SDLK_Y] = V_KEY_Y,
    [SDLK_Z] = V_KEY_Z,
    [SDLK_LEFTBRACE] = V_KEY_LEFTBRACE,
    [SDLK_PIPE] = V_KEY_PIPE,
    [SDLK_RIGHTBRACE] = V_KEY_RIGHTBRACE,
    [SDLK_TILDE] = V_KEY_TILDE,
    [SDLK_DELETE] = V_KEY_DELETE,
    [SDLK_PLUSMINUS] = V_KEY_PLUSMINUS,
};
static const VKey SDLK_MASKED_TO_KEY[291] = {
    [SDLK_CAPSLOCK & 0x1FF] = V_KEY_CAPSLOCK,
    [SDLK_F1 & 0x1FF] = V_KEY_F1,
    [SDLK_F2 & 0x1FF] = V_KEY_F2,
    [SDLK_F3 & 0x1FF] = V_KEY_F3,
    [SDLK_F4 & 0x1FF] = V_KEY_F4,
    [SDLK_F5 & 0x1FF] = V_KEY_F5,
    [SDLK_F6 & 0x1FF] = V_KEY_F6,
    [SDLK_F7 & 0x1FF] = V_KEY_F7,
    [SDLK_F8 & 0x1FF] = V_KEY_F8,
    [SDLK_F9 & 0x1FF] = V_KEY_F9,
    [SDLK_F10 & 0x1FF] = V_KEY_F10,
    [SDLK_F11 & 0x1FF] = V_KEY_F11,
    [SDLK_F12 & 0x1FF] = V_KEY_F12,
    [SDLK_PRINTSCREEN & 0x1FF] = V_KEY_PRINTSCREEN,
    [SDLK_SCROLLLOCK & 0x1FF] = V_KEY_SCROLLLOCK,
    [SDLK_PAUSE & 0x1FF] = V_KEY_PAUSE,
    [SDLK_INSERT & 0x1FF] = V_KEY_INSERT,
    [SDLK_HOME & 0x1FF] = V_KEY_HOME,
    [SDLK_PAGEUP & 0x1FF] = V_KEY_PAGEUP,
    [SDLK_END & 0x1FF] = V_KEY_END,
    [SDLK_PAGEDOWN & 0x1FF] = V_KEY_PAGEDOWN,
    [SDLK_RIGHT & 0x1FF] = V_KEY_RIGHT,
    [SDLK_LEFT & 0x1FF] = V_KEY_LEFT,
    [SDLK_DOWN & 0x1FF] = V_KEY_DOWN,
    [SDLK_UP & 0x1FF] = V_KEY_UP,
    [SDLK_NUMLOCKCLEAR & 0x1FF] = V_KEY_NUMLOCKCLEAR,
    [SDLK_KP_DIVIDE & 0x1FF] = V_KEY_KP_DIVIDE,
    [SDLK_KP_MULTIPLY & 0x1FF] = V_KEY_KP_MULTIPLY,
    [SDLK_KP_MINUS & 0x1FF] = V_KEY_KP_MINUS,
    [SDLK_KP_PLUS & 0x1FF] = V_KEY_KP_PLUS,
    [SDLK_KP_ENTER & 0x1FF] = V_KEY_KP_ENTER,
    [SDLK_KP_1 & 0x1FF] = V_KEY_KP_1,
    [SDLK_KP_2 & 0x1FF] = V_KEY_KP_2,
    [SDLK_KP_3 & 0x1FF] = V_KEY_KP_3,
    [SDLK_KP_4 & 0x1FF] = V_KEY_KP_4,
    [SDLK_KP_5 & 0x1FF] = V_KEY_KP_5,
    [SDLK_KP_6 & 0x1FF] = V_KEY_KP_6,
    [SDLK_KP_7 & 0x1FF] = V_KEY_KP_7,
    [SDLK_KP_8 & 0x1FF] = V_KEY_KP_8,
    [SDLK_KP_9 & 0x1FF] = V_KEY_KP_9,
    [SDLK_KP_0 & 0x1FF] = V_KEY_KP_0,
    [SDLK_KP_PERIOD & 0x1FF] = V_KEY_KP_PERIOD,
    [SDLK_APPLICATION & 0x1FF] = V_KEY_APPLICATION,
    [SDLK_POWER & 0x1FF] = V_KEY_POWER,
    [SDLK_KP_EQUALS & 0x1FF] = V_KEY_KP_EQUALS,
    [SDLK_F13 & 0x1FF] = V_KEY_F13,
    [SDLK_F14 & 0x1FF] = V_KEY_F14,
    [SDLK_F15 & 0x1FF] = V_KEY_F15,
    [SDLK_F16 & 0x1FF] = V_KEY_F16,
    [SDLK_F17 & 0x1FF] = V_KEY_F17,
    [SDLK_F18 & 0x1FF] = V_KEY_F18,
    [SDLK_F19 & 0x1FF] = V_KEY_F19,
    [SDLK_F20 & 0x1FF] = V_KEY_F20,
    [SDLK_F21 & 0x1FF] = V_KEY_F21,
    [SDLK_F22 & 0x1FF] = V_KEY_F22,
    [SDLK_F23 & 0x1FF] = V_KEY_F23,
    [SDLK_F24 & 0x1FF] = V_KEY_F24,
    [SDLK_EXECUTE & 0x1FF] = V_KEY_EXECUTE,
    [SDLK_HELP & 0x1FF] = V_KEY_HELP,
    [SDLK_MENU & 0x1FF] = V_KEY_MENU,
    [SDLK_SELECT & 0x1FF] = V_KEY_SELECT,
    [SDLK_STOP & 0x1FF] = V_KEY_STOP,
    [SDLK_AGAIN & 0x1FF] = V_KEY_AGAIN,
    [SDLK_UNDO & 0x1FF] = V_KEY_UNDO,
    [SDLK_CUT & 0x1FF] = V_KEY_CUT,
    [SDLK_COPY & 0x1FF] = V_KEY_COPY,
    [SDLK_PASTE & 0x1FF] = V_KEY_PASTE,
    [SDLK_FIND & 0x1FF] = V_KEY_FIND,
    [SDLK_MUTE & 0x1FF] = V_KEY_MUTE,
    [SDLK_VOLUMEUP & 0x1FF] = V_KEY_VOLUMEUP,
    [SDLK_VOLUMEDOWN & 0x1FF] = V_KEY_VOLUMEDOWN,
    [SDLK_KP_COMMA & 0x1FF] = V_KEY_KP_COMMA,
    [SDLK_KP_EQUALSAS400 & 0x1FF] = V_KEY_KP_EQUALSAS400,
    [SDLK_ALTERASE & 0x1FF] = V_KEY_ALTERASE,
    [SDLK_SYSREQ & 0x1FF] = V_KEY_SYSREQ,
    [SDLK_CANCEL & 0x1FF] = V_KEY_CANCEL,
    [SDLK_CLEAR & 0x1FF] = V_KEY_CLEAR,
    [SDLK_PRIOR & 0x1FF] = V_KEY_PRIOR,
    [SDLK_RETURN2 & 0x1FF] = V_KEY_RETURN2,
    [SDLK_SEPARATOR & 0x1FF] = V_KEY_SEPARATOR,
    [SDLK_OUT & 0x1FF] = V_KEY_OUT,
    [SDLK_OPER & 0x1FF] = V_KEY_OPER,
    [SDLK_CLEARAGAIN & 0x1FF] = V_KEY_CLEARAGAIN,
    [SDLK_CRSEL & 0x1FF] = V_KEY_CRSEL,
    [SDLK_EXSEL & 0x1FF] = V_KEY_EXSEL,
    [SDLK_KP_00 & 0x1FF] = V_KEY_KP_00,
    [SDLK_KP_000 & 0x1FF] = V_KEY_KP_000,
    [SDLK_THOUSANDSSEPARATOR & 0x1FF] = V_KEY_THOUSANDSSEPARATOR,
    [SDLK_DECIMALSEPARATOR & 0x1FF] = V_KEY_DECIMALSEPARATOR,
    [SDLK_CURRENCYUNIT & 0x1FF] = V_KEY_CURRENCYUNIT,
    [SDLK_CURRENCYSUBUNIT & 0x1FF] = V_KEY_CURRENCYSUBUNIT,
    [SDLK_KP_LEFTPAREN & 0x1FF] = V_KEY_KP_LEFTPAREN,
    [SDLK_KP_RIGHTPAREN & 0x1FF] = V_KEY_KP_RIGHTPAREN,
    [SDLK_KP_LEFTBRACE & 0x1FF] = V_KEY_KP_LEFTBRACE,
    [SDLK_KP_RIGHTBRACE & 0x1FF] = V_KEY_KP_RIGHTBRACE,
    [SDLK_KP_TAB & 0x1FF] = V_KEY_KP_TAB,
    [SDLK_KP_BACKSPACE & 0x1FF] = V_KEY_KP_BACKSPACE,
    [SDLK_KP_A & 0x1FF] = V_KEY_KP_A,
    [SDLK_KP_B & 0x1FF] = V_KEY_KP_B,
    [SDLK_KP_C & 0x1FF] = V_KEY_KP_C,
    [SDLK_KP_D & 0x1FF] = V_KEY_KP_D,
    [SDLK_KP_E & 0x1FF] = V_KEY_KP_E,
    [SDLK_KP_F & 0x1FF] = V_KEY_KP_F,
    [SDLK_KP_XOR & 0x1FF] = V_KEY_KP_XOR,
    [SDLK_KP_POWER & 0x1FF] = V_KEY_KP_POWER,
    [SDLK_KP_PERCENT & 0x1FF] = V_KEY_KP_PERCENT,
    [SDLK_KP_LESS & 0x1FF] = V_KEY_KP_LESS,
    [SDLK_KP_GREATER & 0x1FF] = V_KEY_KP_GREATER,
    [SDLK_KP_AMPERSAND & 0x1FF] = V_KEY_KP_AMPERSAND,
    [SDLK_KP_DBLAMPERSAND & 0x1FF] = V_KEY_KP_DBLAMPERSAND,
    [SDLK_KP_VERTICALBAR & 0x1FF] = V_KEY_KP_VERTICALBAR,
    [SDLK_KP_DBLVERTICALBAR & 0x1FF] = V_KEY_KP_DBLVERTICALBAR,
    [SDLK_KP_COLON & 0x1FF] = V_KEY_KP_COLON,
    [SDLK_KP_HASH & 0x1FF] = V_KEY_KP_HASH,
    [SDLK_KP_SPACE & 0x1FF] = V_KEY_KP_SPACE,
    [SDLK_KP_AT & 0x1FF] = V_KEY_KP_AT,
    [SDLK_KP_EXCLAM & 0x1FF] = V_KEY_KP_EXCLAM,
    [SDLK_KP_MEMSTORE & 0x1FF] = V_KEY_KP_MEMSTORE,
    [SDLK_KP_MEMRECALL & 0x1FF] = V_KEY_KP_MEMRECALL,
    [SDLK_KP_MEMCLEAR & 0x1FF] = V_KEY_KP_MEMCLEAR,
    [SDLK_KP_MEMADD & 0x1FF] = V_KEY_KP_MEMADD,
    [SDLK_KP_MEMSUBTRACT & 0x1FF] = V_KEY_KP_MEMSUBTRACT,
    [SDLK_KP_MEMMULTIPLY & 0x1FF] = V_KEY_KP_MEMMULTIPLY,
    [SDLK_KP_MEMDIVIDE & 0x1FF] = V_KEY_KP_MEMDIVIDE,
    [SDLK_KP_PLUSMINUS & 0x1FF] = V_KEY_KP_PLUSMINUS,
    [SDLK_KP_CLEAR & 0x1FF] = V_KEY_KP_CLEAR,
    [SDLK_KP_CLEARENTRY & 0x1FF] = V_KEY_KP_CLEARENTRY,
    [SDLK_KP_BINARY & 0x1FF] = V_KEY_KP_BINARY,
    [SDLK_KP_OCTAL & 0x1FF] = V_KEY_KP_OCTAL,
    [SDLK_KP_DECIMAL & 0x1FF] = V_KEY_KP_DECIMAL,
    [SDLK_KP_HEXADECIMAL & 0x1FF] = V_KEY_KP_HEXADECIMAL,
    [SDLK_LCTRL & 0x1FF] = V_KEY_LCTRL,
    [SDLK_LSHIFT & 0x1FF] = V_KEY_LSHIFT,
    [SDLK_LALT & 0x1FF] = V_KEY_LALT,
    [SDLK_LGUI & 0x1FF] = V_KEY_LGUI,
    [SDLK_RCTRL & 0x1FF] = V_KEY_RCTRL,
    [SDLK_RSHIFT & 0x1FF] = V_KEY_RSHIFT,
    [SDLK_RALT & 0x1FF] = V_KEY_RALT,
    [SDLK_RGUI & 0x1FF] = V_KEY_RGUI,
    [SDLK_MODE & 0x1FF] = V_KEY_MODE,
    [SDLK_SLEEP & 0x1FF] = V_KEY_SLEEP,
    [SDLK_WAKE & 0x1FF] = V_KEY_WAKE,
    [SDLK_CHANNEL_INCREMENT & 0x1FF] = V_KEY_CHANNEL_INCREMENT,
    [SDLK_CHANNEL_DECREMENT & 0x1FF] = V_KEY_CHANNEL_DECREMENT,
    [SDLK_MEDIA_PLAY & 0x1FF] = V_KEY_MEDIA_PLAY,
    [SDLK_MEDIA_PAUSE & 0x1FF] = V_KEY_MEDIA_PAUSE,
    [SDLK_MEDIA_RECORD & 0x1FF] = V_KEY_MEDIA_RECORD,
    [SDLK_MEDIA_FAST_FORWARD & 0x1FF] = V_KEY_MEDIA_FAST_FORWARD,
    [SDLK_MEDIA_REWIND & 0x1FF] = V_KEY_MEDIA_REWIND,
    [SDLK_MEDIA_NEXT_TRACK & 0x1FF] = V_KEY_MEDIA_NEXT_TRACK,
    [SDLK_MEDIA_PREVIOUS_TRACK & 0x1FF] = V_KEY_MEDIA_PREVIOUS_TRACK,
    [SDLK_MEDIA_STOP & 0x1FF] = V_KEY_MEDIA_STOP,
    [SDLK_MEDIA_EJECT & 0x1FF] = V_KEY_MEDIA_EJECT,
    [SDLK_MEDIA_PLAY_PAUSE & 0x1FF] = V_KEY_MEDIA_PLAY_PAUSE,
    [SDLK_MEDIA_SELECT & 0x1FF] = V_KEY_MEDIA_SELECT,
    [SDLK_AC_NEW & 0x1FF] = V_KEY_AC_NEW,
    [SDLK_AC_OPEN & 0x1FF] = V_KEY_AC_OPEN,
    [SDLK_AC_CLOSE & 0x1FF] = V_KEY_AC_CLOSE,
    [SDLK_AC_EXIT & 0x1FF] = V_KEY_AC_EXIT,
    [SDLK_AC_SAVE & 0x1FF] = V_KEY_AC_SAVE,
    [SDLK_AC_PRINT & 0x1FF] = V_KEY_AC_PRINT,
    [SDLK_AC_PROPERTIES & 0x1FF] = V_KEY_AC_PROPERTIES,
    [SDLK_AC_SEARCH & 0x1FF] = V_KEY_AC_SEARCH,
    [SDLK_AC_HOME & 0x1FF] = V_KEY_AC_HOME,
    [SDLK_AC_BACK & 0x1FF] = V_KEY_AC_BACK,
    [SDLK_AC_FORWARD & 0x1FF] = V_KEY_AC_FORWARD,
    [SDLK_AC_STOP & 0x1FF] = V_KEY_AC_STOP,
    [SDLK_AC_REFRESH & 0x1FF] = V_KEY_AC_REFRESH,
    [SDLK_AC_BOOKMARKS & 0x1FF] = V_KEY_AC_BOOKMARKS,
    [SDLK_SOFTLEFT & 0x1FF] = V_KEY_SOFTLEFT,
    [SDLK_SOFTRIGHT & 0x1FF] = V_KEY_SOFTRIGHT,
    [SDLK_CALL & 0x1FF] = V_KEY_CALL,
    [SDLK_ENDCALL & 0x1FF] = V_KEY_ENDCALL,
    [SDLK_LEFT_TAB & 0x1FF] = V_KEY_LEFT_TAB,
    [SDLK_LEVEL5_SHIFT & 0x1FF] = V_KEY_LEVEL5_SHIFT,
    [SDLK_MULTI_KEY_COMPOSE & 0x1FF] = V_KEY_MULTI_KEY_COMPOSE,
    [SDLK_LMETA & 0x1FF] = V_KEY_LMETA,
    [SDLK_RMETA & 0x1FF] = V_KEY_RMETA,
    [SDLK_LHYPER & 0x1FF] = V_KEY_LHYPER,
    [SDLK_RHYPER & 0x1FF] = V_KEY_RHYPER,
};
static const int SDLK_MASK = (0x20000000 | 0x40000000);

uint32_t v_gfx_features(const VGfxContext* gfx)
{
  UNUSED(gfx);
  return V_GFX_USES_FONT_FACE | V_GFX_USES_TEXT_MESH;
}

VGfxFont* v_gfx_font_new(const VGfxContext* gfx, const VGfxDataSource* source)
{
  UNUSED(gfx);

  if (source->type == V_GFX_DATA_SOURCE_PATH) {
    return (VGfxFont*)SDL_IOFromFile(source->u.path, "rb");
  } else {
    return (VGfxFont*)SDL_IOFromConstMem(source->u.buffer.data,
                                         source->u.buffer.size);
  }
}

VGfxFontFace* v_gfx_font_face_new(const VGfxContext* gfx,
                                  VGfxFont* font,
                                  float font_size)
{
  UNUSED(gfx);
  SDL_IOStream* stream = (void*)font;
  return (void*)TTF_OpenFontIO(stream, false, (float)font_size);
}

bool v_gfx_text_mesh_update(const VGfxContext* gfx,
                            VGfxTextMesh** text_mesh_inout,
                            const char* str,
                            size_t str_size)
{
  TTF_Text* text = (void*)(*text_mesh_inout);

  if (!text) {
    TTF_TextEngine* text_engine = V_GET_TEXT_ENGINE(gfx);

    text = TTF_CreateText(text_engine, NULL, str, str_size);

    if (!text) {
      return false;
    }

    *text_mesh_inout = (VGfxTextMesh*)(void*)text;
  }

  return TTF_SetTextString(text, str, str_size);
}

void v_gfx_text_mesh_clear_font(const VGfxContext* gfx, VGfxTextMesh* text_mesh)
{
  UNUSED(gfx);
  TTF_SetTextFont((void*)text_mesh, NULL);
}

VSize v_gfx_measure_string(const VGfxContext* gfx,
                           VGfxFont* font,
                           float font_size,
                           VGfxFontFace* font_face,
                           const char* text,
                           size_t text_size,
                           VGfxMeasureMode mode,
                           int wrap_width)
{
  UNUSED(gfx, font, font_size);
  int w = 0;
  int h = 0;
  TTF_Font* ttf_font = (void*)font_face;
  bool result;

  if (mode == V_GFX_MEASURE_MODE_TEXT_WRAPPED) {
    result =
        TTF_GetStringSizeWrapped(ttf_font, text, text_size, wrap_width, &w, &h);
  } else {
    result = TTF_GetStringSize(ttf_font, text, text_size, &w, &h);
  }

  return result ? (VSize){(float)w, (float)h} : (VSize){0, 0};
}

VGfxTexture* v_gfx_new_image(const VGfxContext* gfx,
                             const VGfxDataSource* source,
                             VSize* size_out)
{
  UNUSED(gfx);
  SDL_Renderer* renderer = V_GET_RENDERER(gfx);
  SDL_Surface* surface;

  if (source->type == V_GFX_DATA_SOURCE_PATH) {
    surface = SDL_LoadBMP(source->u.path);
  } else {
    surface = SDL_LoadBMP_IO(
        SDL_IOFromConstMem(source->u.buffer.data, source->u.buffer.size), true);
  }

  if (!surface) {
    return NULL;
  }

  size_out->width = (float)surface->w;
  size_out->height = (float)surface->h;

  SDL_Texture* result = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_DestroySurface(surface);

  return (void*)result;
}

void v_gfx_object_free(const VGfxContext* gfx,
                       VGfxObjectType type,
                       void* object)
{
  UNUSED(gfx);
  switch (type) {
    case V_GFX_OBJECT_FONT:
      SDL_CloseIO((SDL_IOStream*)object);
      break;
    case V_GFX_OBJECT_FONT_FACE:
      TTF_CloseFont((TTF_Font*)object);
      break;
    case V_GFX_OBJECT_TEXT_MESH:
      TTF_DestroyText((TTF_Text*)object);
      break;
    case V_GFX_OBJECT_IMAGE:
      SDL_DestroyTexture((SDL_Texture*)object);
      break;
    default:
      break;
  }
}

void v_gfx_clear(const VGfxContext* gfx, const VRect* rect, const VColor* color)
{
  UNUSED(rect);
  SDL_Renderer* renderer = V_GET_RENDERER(gfx);
  SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
  SDL_RenderClear(renderer);
}

void v_gfx_draw_fill_rect(const VGfxContext* gfx,
                          const VRect* rect,
                          const VColor* color,
                          uint16_t border_radius)
{
  UNUSED(border_radius);
  SDL_Renderer* renderer = V_GET_RENDERER(gfx);
  const SDL_FRect sdl_rect = {rect->x, rect->y, rect->width, rect->height};

  SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
  SDL_RenderFillRect(renderer, &sdl_rect);
}

void v_gfx_draw_rect(const VGfxContext* gfx,
                     const VRect* rect,
                     const VColor* color,
                     uint16_t bt,
                     uint16_t br,
                     uint16_t bb,
                     uint16_t bl,
                     uint16_t border_radius)
{
  UNUSED(border_radius);
  SDL_Renderer* renderer = V_GET_RENDERER(gfx);

  const float t = (float)bt;
  const float r = (float)br;
  const float b = (float)bb;
  const float l = (float)bl;
  SDL_FRect rects[4];
  int size = 0;

  if (bt > 0) {
    rects[size++] = (SDL_FRect){rect->x, rect->y, rect->width, t};
  }

  if (bb > 0) {
    rects[size++] =
        (SDL_FRect){rect->x, rect->y + rect->height - b, rect->width, b};
  }

  if (bl > 0) {
    rects[size++] = (SDL_FRect){rect->x, rect->y + t, l, rect->height - t - b};
  }

  if (br > 0) {
    rects[size++] = (SDL_FRect){
        rect->x + rect->width - r,
        rect->y + t,
        r,
        rect->height - t - b,
    };
  }

  if (size > 0) {
    SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
    SDL_RenderRects(renderer, rects, size);
  }
}

void v_gfx_draw_text(const VGfxContext* gfx,
                     VGfxTextMesh* text_mesh,
                     const char* str,
                     const VRect* content_box,
                     bool wrap,
                     VAlignX align,
                     VGfxFont* font,
                     float font_size,
                     VGfxFontFace* font_face,
                     const VColor* color)
{
  UNUSED(gfx, str, font, font_size);

  TTF_Text* text = (void*)text_mesh;
  TTF_Font* text_font = (void*)font_face;
  int text_wrap_width = 0;
  float x = content_box->x;
  float y = content_box->y;

  if (TTF_GetTextFont(text) != text_font) {
    TTF_SetTextFont(text, text_font);
  }

  if (wrap) {
    const int content_box_width = (int)content_box->width;

    TTF_GetTextWrapWidth(text, &text_wrap_width);

    if (text_wrap_width != content_box_width) {
      TTF_SetTextWrapWidth(text, content_box_width);
    }
  } else {
    TTF_SetTextWrapWidth(text, 0);
  }

  TTF_HorizontalAlignment text_align;

  switch (align) {
    case V_ALIGN_X_CENTER:
      text_align = TTF_HORIZONTAL_ALIGN_CENTER;
      break;
    case V_ALIGN_X_RIGHT:
      text_align = TTF_HORIZONTAL_ALIGN_RIGHT;
      break;
    default:
      text_align = TTF_HORIZONTAL_ALIGN_LEFT;
      break;
  }

  if (TTF_GetFontWrapAlignment(text_font) != text_align) {
    TTF_SetFontWrapAlignment(text_font, text_align);
  }

  if (align != V_ALIGN_X_LEFT) {
    int text_width = 0;

    if (TTF_GetTextSize(text, &text_width, NULL)) {
      if (align == V_ALIGN_X_CENTER) {
        x += SDL_roundf((content_box->width * 0.5f) - (float)text_width * 0.5f);
      } else if (align == V_ALIGN_X_RIGHT) {
        x += SDL_roundf(content_box->width - (float)text_width);
      }
    }
  }

  TTF_SetTextColor(text, color->r, color->g, color->b, color->a);

  TTF_DrawRendererText(text, x, y);
}

void v_gfx_draw_image(const VGfxContext* gfx,
                      const VRect* rect,
                      VGfxTexture* texture,
                      const VColor* color)
{
  SDL_Renderer* renderer = V_GET_RENDERER(gfx);
  SDL_Texture* sdl_texture = (void*)texture;
  const SDL_FRect dest = {rect->x, rect->y, rect->width, rect->height};

  SDL_SetTextureColorMod(sdl_texture, color->r, color->g, color->b);
  SDL_SetTextureAlphaMod(sdl_texture, color->a);
  SDL_RenderTexture(renderer, sdl_texture, NULL, &dest);
}

void v_gfx_set_clip(const VGfxContext* gfx, float x, float y, float w, float h)
{
  SDL_Renderer* renderer = V_GET_RENDERER(gfx);
  SDL_Rect clip = {(int)x, (int)y, (int)w, (int)h};

  SDL_SetRenderClipRect(renderer, &clip);
}

bool v_itx_map_key(int virtual_key,
                   int physical_key,
                   int raw_modifiers,
                   VKey* key_out,
                   VKeyMod* modifiers_out)
{
  UNUSED(physical_key);

  VKey key;

  if (virtual_key <= 0) {
    key = V_KEY_INVALID;
  } else if ((virtual_key & SDLK_MASK) != 0) {
    if ((virtual_key & 0x1FF) <
        (int)(sizeof(SDLK_MASKED_TO_KEY) / sizeof(SDLK_MASKED_TO_KEY[0]))) {
      key = SDLK_MASKED_TO_KEY[virtual_key & 0x1FF];
    } else {
      key = V_KEY_INVALID;
    }
  } else {
    if (virtual_key < (int)(sizeof(SDLK_TO_KEY) / sizeof(SDLK_TO_KEY[0]))) {
      key = SDLK_TO_KEY[virtual_key];
    } else {
      key = V_KEY_INVALID;
    }
  }

  if (key == V_KEY_INVALID) {
    return false;
  }

  if (key_out) {
    *key_out = key;
  }

  if (modifiers_out) {
    uint32_t modifiers = 0;

    if (raw_modifiers & SDL_KMOD_LSHIFT) {
      modifiers |= V_KMOD_LSHIFT;
    }
    if (raw_modifiers & SDL_KMOD_RSHIFT) {
      modifiers |= V_KMOD_RSHIFT;
    }
    if (raw_modifiers & SDL_KMOD_LCTRL) {
      modifiers |= V_KMOD_LCTRL;
    }
    if (raw_modifiers & SDL_KMOD_RCTRL) {
      modifiers |= V_KMOD_RCTRL;
    }
    if (raw_modifiers & SDL_KMOD_LALT) {
      modifiers |= V_KMOD_LALT;
    }
    if (raw_modifiers & SDL_KMOD_RALT) {
      modifiers |= V_KMOD_RALT;
    }
    if (raw_modifiers & SDL_KMOD_LGUI) {
      modifiers |= V_KMOD_LGUI;
    }
    if (raw_modifiers & SDL_KMOD_RGUI) {
      modifiers |= V_KMOD_RGUI;
    }
    if (raw_modifiers & SDL_KMOD_NUM) {
      modifiers |= V_KMOD_NUM;
    }
    if (raw_modifiers & SDL_KMOD_CAPS) {
      modifiers |= V_KMOD_CAPS;
    }
    if (raw_modifiers & SDL_KMOD_MODE) {
      modifiers |= V_KMOD_MODE;
    }
    if (raw_modifiers & SDL_KMOD_SCROLL) {
      modifiers |= V_KMOD_SCROLL;
    }

    *modifiers_out = modifiers;
  }

  return true;
}

VMouseButton v_itx_map_mouse_button(int button)
{
  switch (button) {
    case SDL_BUTTON_LEFT:
      return V_MOUSE_BUTTON_LEFT;
    case SDL_BUTTON_RIGHT:
      return V_MOUSE_BUTTON_RIGHT;
    case SDL_BUTTON_MIDDLE:
      return V_MOUSE_BUTTON_MIDDLE;
    default:
      return V_MOUSE_BUTTON_INVALID;
  }
}
