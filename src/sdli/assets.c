#include <sdli/assets.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <inttypes.h>
#include <windows.h>
#else
#include <incbin.h>
#endif

//
// global state
//

ConstBlob SDLI_FONT_NORMAL = {0};
ConstBlob SDLI_FONT_BOLD = {0};

//
// function implementation
//

#ifdef _WIN32
static ConstBlob LoadCustomData(HMODULE module, LPCSTR id)
{
  // note: if loading fails, something is very wrong, so we just abort
  HRSRC resource_info = FindResourceA(module, id, "CUSTOMDATA");

  if (!resource_info) {
    printf("Error: FindResourceA(%s): %" PRIuPTR "\n", id,
           (size_t)(GetLastError()));
    fflush(stdout);
    abort();
  }

  HGLOBAL resource = LoadResource(module, resource_info);
  if (!resource) {
    printf("Error: LoadResource(%s): %" PRIuPTR "\n", id,
           (size_t)(GetLastError()));
    fflush(stdout);
    abort();
  }

  LPVOID message = LockResource(resource);
  if (!message) {
    printf("Error: LockResource(%s)\n", id);
    fflush(stdout);
    abort();
  }

  DWORD resourceSize = SizeofResource(module, resource_info);

  return (ConstBlob){message, (size_t)(resourceSize)};
}

void LoadAssets(void)
{
  HMODULE module = NULL;

  SDLI_FONT_NORMAL = LoadCustomData(module, "SDLI_FONT_NORMAL");
  SDLI_FONT_BOLD = LoadCustomData(module, "SDLI_FONT_BOLD");
}
#else
// SDLI_ASSETS_PATH: absolute path to assets directory (CMakeLists.txt)
INCBIN(FontNormal, SDLI_ASSETS_PATH "fonts/roboto-regular.ttf");
INCBIN(FontBold, SDLI_ASSETS_PATH "fonts/roboto-bold.ttf");

void LoadAssets(void)
{
  SDLI_FONT_NORMAL = (ConstBlob){gFontNormalData, (size_t)(gFontNormalSize)};
  SDLI_FONT_BOLD = (ConstBlob){gFontBoldData, (size_t)(gFontBoldSize)};
}
#endif
