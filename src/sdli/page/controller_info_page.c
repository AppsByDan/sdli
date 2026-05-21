#include "page.h"

#include <sdli/model/model.h>
#include <sdli/strings.h>
#include <sdli/style.h>
#include <sdli/util.h>
#include <sdli/widget/widget.h>

//
// constants
//

#define NID_INSTANCE_ID "ci:iid"
#define NID_GUID "ci:guid"
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

//
// private function declarations
//

static void OnNavigatorEvent(NavigatorEvent* event);
static void OnBackButtonClick(VNode* node, VEvent* event);

//
// public function implementation
//

VNode* ControllerInfoPage(void)
{
  VNode* properties_list = Box({.sclass = CLS_LIST});
  // clang-format off
  VNode* page = Box({
    .id = PAGEID_CONTROLLER_INFO,
    .sclass = CLS_PAGE,
    Children(
      Button("Back", NULL, &OnBackButtonClick),
      Text({.content.text = STR(SID_CONTROLLER_INFO), .sclass = CLS_PAGE_H1}),
      Text({.content.text = STR(SID_CAP_DETAILS), .sclass = CLS_PAGE_H2}),
      Box({
        .sclass= CLS_LIST,
        Children(
          KeyValueListItem(STR(SID_INSTANCE_ID), NID_INSTANCE_ID),
          KeyValueListItem(STR(SID_GUID), NID_GUID),
          KeyValueListItem(STR(SID_POWER_STATE), NID_POWER_STATE),
          KeyValueListItem(STR(SID_BATTERY), NID_BATTERY),
          KeyValueListItem(STR(SID_JOYSTICK_TYPE), NID_JOYSTICK_TYPE),
          KeyValueListItem(STR(SID_GAMEPAD_TYPE), NID_GAMEPAD_TYPE),
          KeyValueListItem(STR(SID_CONNECTION_TYPE), NID_CONNECTION_TYPE),
          KeyValueListItem(STR(SID_PRODUCT), NID_PRODUCT),
          KeyValueListItem(STR(SID_PRODUCT_VERSION), NID_PRODUCT_VERSION),
          KeyValueListItem(STR(SID_VENDOR), NID_VENDOR),
          KeyValueListItem(STR(SID_FIRMWARE_VERSION), NID_FIRMWARE_VERSION),
          KeyValueListItem(STR(SID_SERIAL_NUMBER), NID_SERIAL_NUMBER),
          KeyValueListItem(STR(SID_PATH), NID_PATH),
          KeyValueListItem(STR(SID_HAPTIC), NID_HAPTIC),
          KeyValueListItem(STR(SID_BUTTON_COUNT), NID_BUTTON_COUNT),
          KeyValueListItem(STR(SID_AXIS_COUNT), NID_AXIS_COUNT),
          KeyValueListItem(STR(SID_POV_HAT_COUNT), NID_POV_HAT_COUNT),
          KeyValueListItem(STR(SID_BALL_COUNT), NID_BALL_COUNT),
          KeyValueListItem(STR(SID_TOUCHPAD_COUNT), NID_TOUCHPAD_COUNT),
          KeyValueListItemLast(STR(SID_STEAM_ID), NID_STEAM_ID),
        )
      }),
      Text({.content.text = STR(SID_CAP_PROPERTIES), .sclass = CLS_PAGE_H2}),
      properties_list
    )
  });
  // clang-format on

  int property_count = 0;
  const char** property_names = Controller_GetPropertyNames(&property_count);

  for (int i = 0; i < property_count; ++i) {
    const char* property_name = property_names[i];
    if (i == property_count - 1) {
      v_node_append_child(properties_list,
                          KeyValueListItemLast(property_name, property_name));
    } else {
      v_node_append_child(properties_list,
                          KeyValueListItem(property_name, property_name));
    }
  }

  return Navigable_Init(page, &OnNavigatorEvent);
}

//
// private function implementation
//

static void OnNavigatorEvent(NavigatorEvent* event)
{
  if (event->type == NAVIGATOR_EVENT_ENTER) {
    ControllerId controller_id = ControllerListModel_GetSelectedController();
    // TODO: what if controller_id is 0?

    // TODO: localize some values (?)
    // TODO: battery should be %

    BindU32(NID_INSTANCE_ID, controller_id);
    BindString(NID_GUID, Controller_GetGUID(controller_id));
    BindString(NID_POWER_STATE, Controller_GetPowerState(controller_id));
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

    int property_count = 0;
    const ControllerProperty* properties =
        Controller_GetProperties(controller_id, &property_count);

    for (int i = 0; i < property_count; ++i) {
      const ControllerProperty* property = &properties[i];
      BindBool(property->name, property->value);
    }
  }
}

static void OnBackButtonClick(VNode* node, VEvent* event)
{
  UNUSED(node, event);
  // TODO: this should be a back navigate, not a goto
  PageNavigator_Goto(PAGEID_CONTROLLER_LIST);
}
