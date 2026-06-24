#include <sdli/page/page.h>

#include <sdli/node_notation.h>
#include <sdli/model/model.h>
#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

//
// constants
//

#define NID_SDL_VERSION "sys:ver"
#define NID_SDL_REVISION "sys:rev"
#define NID_PLATFORM "sys:platform"
#define NID_THEME "sys:theme"
#define NID_POWER_STATE "sys:power"
#define NID_BATTERY "sys:battery"
#define NID_CPU_CORES "sys:cores"
#define NID_CACHE_LINE_SIZE "sys:cache"
#define NID_SYSTEM_RAM "sys:ram"

//
// private function declarations
//

static void OnNavigatorEvent(NavigatorEvent* event);

//
// public function implementation
//

VNode* SystemPage(void)
{
  NN_BUILD_NEW(system_page)
  {
    NN_BOX({.id = PAGEID_SYSTEM, .sclass = CLS_PAGE})
    {
      NN_TEXT({.text = STR(SID_SYSTEM), .sclass = CLS_PAGE_H1});
      NN_BOX({.sclass = CLS_LIST})
      {
        NN_CALL(KeyValueListItem, STR(SID_VERSION), NID_SDL_VERSION);
        NN_CALL(KeyValueListItemLast, STR(SID_REVISION), NID_SDL_REVISION);
      }

      NN_TEXT({.text = STR(SID_CAP_OS), .sclass = CLS_PAGE_H2});
      NN_BOX({.sclass = CLS_LIST})
      {
        NN_CALL(KeyValueListItem, STR(SID_PLATFORM), NID_PLATFORM);
        NN_CALL(KeyValueListItemLast, STR(SID_THEME), NID_THEME);
      }

      NN_TEXT({.text = STR(SID_CAP_HARDWARE), .sclass = CLS_PAGE_H2});
      NN_BOX({.sclass = CLS_LIST})
      {
        NN_CALL(KeyValueListItem, STR(SID_POWER_STATE), NID_POWER_STATE);
        NN_CALL(KeyValueListItem, STR(SID_BATTERY), NID_BATTERY);
        NN_CALL(KeyValueListItem, STR(SID_CPU_CORES), NID_CPU_CORES);
        NN_CALL(KeyValueListItem, STR(SID_CACHE_LINE_SIZE),
                NID_CACHE_LINE_SIZE);
        NN_CALL(KeyValueListItemLast, STR(SID_SYSTEM_RAM), NID_SYSTEM_RAM);
      }
    }
  }

  return Navigable_Init(system_page, &OnNavigatorEvent);
}

//
// private function implementation
//

static void OnNavigatorEvent(NavigatorEvent* event)
{
  if (event->type == NAVIGATOR_EVENT_ENTER) {
    // TODO: values with units need to be formatted to locale
    BindString(NID_SDL_VERSION, SystemModel_GetSdlVersion());
    BindString(NID_SDL_REVISION, SystemModel_GetSdlRevision());
    BindString(NID_PLATFORM, SystemModel_GetPlatformName());
    BindString(NID_THEME, SystemModel_GetTheme());
    BindString(NID_POWER_STATE, SystemModel_GetPowerState());
    BindInt(NID_BATTERY, SystemModel_GetBatteryLevel());
    BindInt(NID_CPU_CORES, SystemModel_GetCpuCoreCount());
    BindInt(NID_CACHE_LINE_SIZE, SystemModel_GetCpuCacheLineSize());
    BindInt(NID_SYSTEM_RAM, SystemModel_GetRamMiB());
  }
}
