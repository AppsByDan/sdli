#ifndef SDLI_MODEL_H
#define SDLI_MODEL_H

#include <stdbool.h>
#include <stdint.h>

//
// types
//

typedef uint32_t ControllerId;

typedef struct ControllerProperty {
  const char* name;
  bool value;
} ControllerProperty;

/*
 * SystemModel exposes system and platform information from SDL.
 */

void SystemModel_Init(void);
void SystemModel_Drop(void);

const char* SystemModel_GetSdlVersion(void);
const char* SystemModel_GetSdlRevision(void);
const char* SystemModel_GetPlatformName(void);
const char* SystemModel_GetTheme(void);
const char* SystemModel_GetPowerState(void);
int SystemModel_GetBatteryLevel(void);
int SystemModel_GetCpuCoreCount(void);
int SystemModel_GetCpuCacheLineSize(void);
int SystemModel_GetRamMiB(void);

/*
 * ControllerListModel manages Controllers connected to the system.
 */

void ControllerListModel_Init(void);
void ControllerListModel_Drop(void);
ControllerId* ControllerListModel_SortControllers(int* out_count);

/*
 * Controller exposes SDL Joystick and Gamepad information about a specific
 * controller.
 */

const char* Controller_GetName(ControllerId id);
const char* Controller_GetGUID(ControllerId id);
const char* Controller_GetPath(ControllerId id);
const char* Controller_GetJoystickType(ControllerId id);
const char* Controller_GetGamepadType(ControllerId id);
const char* Controller_GetConnectionType(ControllerId id);
const char* Controller_GetPowerState(ControllerId id);
int Controller_GetBatteryLevel(ControllerId id);
int Controller_GetProduct(ControllerId id);
int Controller_GetProductVersion(ControllerId id);
int Controller_GetFirmwareVersion(ControllerId id);
int Controller_GetVendor(ControllerId id);
const char* Controller_GetSerial(ControllerId id);
bool Controller_IsHaptic(ControllerId id);
int Controller_GetButtonCount(ControllerId id);
int Controller_GetAxisCount(ControllerId id);
int Controller_GetHatCount(ControllerId id);
int Controller_GetBallCount(ControllerId id);
int Controller_GetTouchpadCount(ControllerId id);
uint64_t Controller_GetSteamHandle(ControllerId id);
const ControllerProperty* Controller_GetProperties(ControllerId id,
                                                   int* out_count);
const char** Controller_GetPropertyNames(int* out_count);

#endif  // SDLI_MODEL_H
