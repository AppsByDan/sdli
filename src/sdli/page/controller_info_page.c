#include "page.h"

#include <sdli/node_notation.h>
#include <sdli/model/model.h>
#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

#include <stc/cstr.h>

//
// constants
//

#define NID_INSTANCE_ID "ci:iid"
#define NID_GUID "ci:guid"
#define NID_GAMEPAD_NAME "ci:gname"
#define NID_JOYSTICK_NAME "ci:jname"
#define NID_POWER_STATE "ci:power"
#define NID_BATTERY "ci:battery"
#define NID_JOYSTICK_TYPE "ci:jtype"
#define NID_GAMEPAD_TYPE "ci:gtype"
#define NID_CONNECTION_TYPE "ci:ctype"
#define NID_PRODUCT "ci:prod"
#define NID_PRODUCT_VERSION "ci:prod-ver"
#define NID_VENDOR "ci:vendor"
#define NID_FIRMWARE_VERSION "ci:firm"
#define NID_SERIAL_NUMBER "ci:serial"
#define NID_PATH "ci:path"
#define NID_HAPTIC "ci:haptic"
#define NID_BUTTON_COUNT "ci:buttons"
#define NID_AXIS_COUNT "ci:axes"
#define NID_POV_HAT_COUNT "ci:hats"
#define NID_BALL_COUNT "ci:balls"
#define NID_TOUCHPAD_COUNT "ci:tpads"
#define NID_STEAM_ID "ci:steam"
#define NID_MAPPINGS_TITLE "ci:mappings-h2"
#define NID_MAPPINGS "ci:mappings"
#define NID_PROPERTY_FMT "ci:prop:%i"

//
// private function declarations
//

static void OnNavigatorEvent(NavigatorEvent* event);

//
// private node event handlers
//

// clang-format off
OnClickInline(Back, {
  PageNavigator_Goto(PAGEID_CONTROLLER_LIST);
})
// clang-format on

//
// public function implementation
//

VNode* ControllerInfoPage(void)
{
  NN_BUILD_NEW(controller_info_page)
  {
    NN_BOX({.id = PAGEID_CONTROLLER_INFO, .sclass = CLS_PAGE})
    {
      NN_CALL(Button, "Back", NULL, &Back_OnClick);

      NN_TEXT({
          .text = STR(SID_CONTROLLER_INFO),
          .sclass = CLS_PAGE_H1,
      });
      NN_BOX({.sclass = CLS_SCROLLABLE})
      {
        NN_TEXT({.text = STR(SID_CAP_DETAILS), .sclass = CLS_PAGE_H2});
        NN_BOX({.sclass = CLS_LIST})
        {
          NN_CALL(KeyValueListItem, STR(SID_INSTANCE_ID), NID_INSTANCE_ID);
          NN_CALL(KeyValueListItem, STR(SID_GUID), NID_GUID);
          NN_CALL(KeyValueListItem, STR(SID_JOYSTICK_NAME), NID_JOYSTICK_NAME);
          NN_CALL(KeyValueListItem, STR(SID_GAMEPAD_NAME), NID_GAMEPAD_NAME);
          NN_CALL(KeyValueListItem, STR(SID_POWER_STATE), NID_POWER_STATE);
          NN_CALL(KeyValueListItem, STR(SID_BATTERY), NID_BATTERY);
          NN_CALL(KeyValueListItem, STR(SID_JOYSTICK_TYPE), NID_JOYSTICK_TYPE);
          NN_CALL(KeyValueListItem, STR(SID_GAMEPAD_TYPE), NID_GAMEPAD_TYPE);
          NN_CALL(KeyValueListItem, STR(SID_CONNECTION_TYPE),
                  NID_CONNECTION_TYPE);
          NN_CALL(KeyValueListItem, STR(SID_PRODUCT), NID_PRODUCT);
          NN_CALL(KeyValueListItem, STR(SID_PRODUCT_VERSION),
                  NID_PRODUCT_VERSION);
          NN_CALL(KeyValueListItem, STR(SID_VENDOR), NID_VENDOR);
          NN_CALL(KeyValueListItem, STR(SID_FIRMWARE_VERSION),
                  NID_FIRMWARE_VERSION);
          NN_CALL(KeyValueListItem, STR(SID_SERIAL_NUMBER), NID_SERIAL_NUMBER);
          NN_CALL(KeyValueListItem, STR(SID_PATH), NID_PATH);
          NN_CALL(KeyValueListItem, STR(SID_HAPTIC), NID_HAPTIC);
          NN_CALL(KeyValueListItem, STR(SID_BUTTON_COUNT), NID_BUTTON_COUNT);
          NN_CALL(KeyValueListItem, STR(SID_AXIS_COUNT), NID_AXIS_COUNT);
          NN_CALL(KeyValueListItem, STR(SID_POV_HAT_COUNT), NID_POV_HAT_COUNT);
          NN_CALL(KeyValueListItem, STR(SID_BALL_COUNT), NID_BALL_COUNT);
          NN_CALL(KeyValueListItem, STR(SID_TOUCHPAD_COUNT),
                  NID_TOUCHPAD_COUNT);
          NN_CALL(KeyValueListItemLast, STR(SID_STEAM_ID), NID_STEAM_ID);
        }

        NN_TEXT({
            .text = STR(SID_CAP_PROPERTIES),
            .sclass = CLS_PAGE_H2,
        });
        NN_BOX({.sclass = CLS_LIST})
        {
          const int property_count = Controller_GetPropertyCount();

          for (int i = 0; i < property_count; ++i) {
            const char* property_name = Controller_GetPropertyName(i);

            if (i == property_count - 1) {
              NN_CALL(KeyValueListItemLast, property_name, property_name);
            } else {
              NN_CALL(KeyValueListItem, property_name, property_name);
            }

            VNode* self = NN_SELF();
            VNode* item = v_node_last_child(self);
            VNode* property_value = KeyValueListItem_GetValue(item);

            v_node_set_id_fmt(property_value, NID_PROPERTY_FMT, i);
          }
        }

        NN_TEXT({
            .id = NID_MAPPINGS_TITLE,
            .text = "MAPPINGS",
            .sclass = CLS_PAGE_H2,
        });
        NN_BOX({.id = NID_MAPPINGS, .sclass = CLS_LIST});
      }
    }
  }

  return Navigable_Init(controller_info_page, &OnNavigatorEvent);
}

//
// private function implementation
//

static void OnNavigatorEvent(NavigatorEvent* event)
{
  if (event->type == NAVIGATOR_EVENT_ENTER) {
    ControllerId controller_id = State_GetSelectedController();
    // TODO: what if controller_id is 0?

    // TODO: localize some values (?)
    // TODO: battery should be %

    BindU32(NID_INSTANCE_ID, controller_id);
    BindString(NID_GUID, Controller_GetGUID(controller_id));
    BindString(NID_POWER_STATE, Controller_GetPowerState(controller_id));
    BindString(NID_GAMEPAD_NAME, Controller_GetGamepadName(controller_id));
    BindString(NID_JOYSTICK_NAME, Controller_GetJoystickName(controller_id));
    BindInt(NID_BATTERY, Controller_GetBatteryLevel(controller_id));
    BindString(NID_JOYSTICK_TYPE, Controller_GetJoystickType(controller_id));
    BindString(NID_GAMEPAD_TYPE, Controller_GetGamepadType(controller_id));
    BindString(NID_CONNECTION_TYPE,
               Controller_GetConnectionType(controller_id));
    BindInt(NID_PRODUCT, Controller_GetProduct(controller_id));
    BindInt(NID_PRODUCT_VERSION, Controller_GetProductVersion(controller_id));
    BindInt(NID_VENDOR, Controller_GetVendor(controller_id));
    BindInt(NID_FIRMWARE_VERSION, Controller_GetFirmwareVersion(controller_id));
    BindString(NID_SERIAL_NUMBER, Controller_GetSerial(controller_id));
    BindString(NID_PATH, Controller_GetPath(controller_id));
    BindBool(NID_HAPTIC, Controller_IsHaptic(controller_id));
    BindInt(NID_BUTTON_COUNT, Controller_GetButtonCount(controller_id));
    BindInt(NID_AXIS_COUNT, Controller_GetAxisCount(controller_id));
    BindInt(NID_POV_HAT_COUNT, Controller_GetHatCount(controller_id));
    BindInt(NID_BALL_COUNT, Controller_GetBallCount(controller_id));
    BindInt(NID_TOUCHPAD_COUNT, Controller_GetTouchpadCount(controller_id));
    BindU64(NID_STEAM_ID, Controller_GetSteamHandle(controller_id));

    const int property_count = Controller_GetPropertyCount();
    cstr temp = cstr_init();

    for (int i = 0; i < property_count; ++i) {
      cstr_clear(&temp);
      cstr_append_fmt(&temp, NID_PROPERTY_FMT, i);
      BindBool(cstr_str(&temp), Controller_GetPropertyValue(controller_id, i));
    }

    cstr_drop(&temp);

    VNode* mappings_title = v_get_node_by_id(NID_MAPPINGS_TITLE);
    VNode* mappings = v_get_node_by_id(NID_MAPPINGS);
    bool has_mapping = Controller_HasMapping(controller_id);

    v_node_set_visible(mappings_title, has_mapping);
    v_node_set_visible(mappings, has_mapping);
    v_node_remove_children(mappings);

    if (!has_mapping) {
      return;
    }

    NN_BUILD_APPEND(mappings)
    {
      const int binding_count = Controller_GetBindingCount(controller_id);

      for (int i = 0; i < binding_count; i++) {
        const char* binding_name = Controller_GetBindingName(controller_id, i);

        if (i == binding_count - 1) {
          NN_CALL(KeyValueListItemLast, binding_name, NULL);
        } else {
          NN_CALL(KeyValueListItem, binding_name, NULL);
        }

        VNode* self = NN_SELF();
        VNode* kv_list_item = v_node_last_child(self);

        v_node_set_text(KeyValueListItem_GetValue(kv_list_item),
                        Controller_GetBindingValue(controller_id, i));
      }
    }
  }
}
