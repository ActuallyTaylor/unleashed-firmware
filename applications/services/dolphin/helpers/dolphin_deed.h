#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DolphinDeedDifficultyEasy,
    DolphinDeedDifficultyMedium,
    DolphinDeedDifficultyHard,
    DolphinDeedDifficultyDeadly,
    DolphinDeedDifficultyMAX
} DolphinDeedDifficulty;

typedef enum {
    DolphinAppSubGhz,
    DolphinAppRfid,
    DolphinAppNfc,
    DolphinAppIr,
    DolphinAppIbutton,
    DolphinAppBadusb,
    DolphinAppPlugin,
    DolphinAppMAX,
} DolphinApp;

typedef enum {
    DolphinDeedSubGhzReceiverInfo,
    DolphinDeedSubGhzSave,
    DolphinDeedSubGhzRawRec,
    DolphinDeedSubGhzAddManually,
    DolphinDeedSubGhzSend,
    DolphinDeedSubGhzFrequencyAnalyzer,

    DolphinDeedRfidRead,
    DolphinDeedRfidReadSuccess,
    DolphinDeedRfidSave,
    DolphinDeedRfidEmulate,
    DolphinDeedRfidAdd,

    DolphinDeedNfcRead,
    DolphinDeedNfcReadSuccess,
    DolphinDeedNfcSave,
    DolphinDeedNfcDetectReader,
    DolphinDeedNfcEmulate,
    DolphinDeedNfcMfcAdd,
    DolphinDeedNfcAddSave,
    DolphinDeedNfcAddEmulate,

    DolphinDeedIrSend,
    DolphinDeedIrLearnSuccess,
    DolphinDeedIrSave,

    DolphinDeedIbuttonRead,
    DolphinDeedIbuttonReadSuccess,
    DolphinDeedIbuttonSave,
    DolphinDeedIbuttonEmulate,
    DolphinDeedIbuttonAdd,

    DolphinDeedBadUsbPlayScript,

    DolphinDeedU2fAuthorized,
    DolphinDeedGpioUartBridge,

    DolphinDeedPluginStart,
    DolphinDeedPluginGameStart,
    DolphinDeedPluginGameWin,

    DolphinDeedMAX,

    DolphinDeedTestLeft,
    DolphinDeedTestRight,
} DolphinDeed;

typedef struct {
    DolphinApp app;
    DolphinDeedDifficulty difficulty;
} DolphinDeedWeight;

typedef struct {
    DolphinApp app;
    uint8_t icounter_limit;
} DolphinDeedLimits;

DolphinApp dolphin_deed_get_app(DolphinDeed deed);
uint8_t dolphin_deed_get_app_limit(DolphinApp app, uint8_t level);
uint8_t dolphin_deed_get_weight(DolphinDeed deed, uint8_t level);

#ifdef __cplusplus
}
#endif
