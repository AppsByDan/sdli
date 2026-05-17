#ifndef SDLI_MODEL_H
#define SDLI_MODEL_H

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

void ControllerListModel_Init(void);
void ControllerListModel_Drop(void);

#endif  // SDLI_MODEL_H
