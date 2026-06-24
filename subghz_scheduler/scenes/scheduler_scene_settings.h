#pragma once

#ifndef SCHEDULER_UI_TEXT
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define SCHEDULER_UI_TEXT(en, zh) (zh)
#else
#define SCHEDULER_UI_TEXT(en, zh) (en)
#endif
#endif

typedef enum FileTxType {
    SchedulerFileTypeSingle,
    SchedulerFileTypePlaylist,
    SchedulerFileTypeNum
} FileTxType;
#define FILE_TYPE_COUNT SchedulerFileTypeNum
static const char* const file_type_text[SchedulerFileTypeNum] = {
    SCHEDULER_UI_TEXT("Single", "单文件"),
    SCHEDULER_UI_TEXT("Playlist", "播放列表"),
};

typedef enum SchedulerTxMode {
    SchedulerTxModeNormal,
    SchedulerTxModeImmediate,
    SchedulerTxModeOneShot,
    SchedulerTxModeSettingsNum,
} SchedulerTxMode;
#define TX_MODE_COUNT SchedulerTxModeSettingsNum
static const char* const mode_text[SchedulerTxModeSettingsNum] = {
    SCHEDULER_UI_TEXT("Normal", "普通"),
    SCHEDULER_UI_TEXT("Immed.", "立即"),
    SCHEDULER_UI_TEXT("1-Shot", "单次"),
};

typedef enum SchedulerTimingMode {
    SchedulerTimingModeRelative,
    SchedulerTimingModePrecise,
    SchedulerTimingModeNum
} SchedulerTimingMode;
#define TIMING_MODE_COUNT SchedulerTimingModeNum
static const char* const timing_mode_text[TIMING_MODE_COUNT] = {
    SCHEDULER_UI_TEXT("Relative", "相对"),
    SCHEDULER_UI_TEXT("Precise", "精确"),
};

#define TX_DELAY_COUNT 4
typedef enum SchedulerTxDelay {
    SchedulerTxDelay100 = 100,
    SchedulerTxDelay250 = 250,
    SchedulerTxDelay500 = 500,
    SchedulerTxDelay1000 = 1000
} SchedulerTxDelay;
static const char* const tx_delay_text[TX_DELAY_COUNT] = {"100ms", "250ms", "500ms", "1000ms"};
static const uint16_t tx_delay_value[TX_DELAY_COUNT] =
    {SchedulerTxDelay100, SchedulerTxDelay250, SchedulerTxDelay500, SchedulerTxDelay1000};

enum Intervals {
    Interval1Sec,
    Interval2Sec,
    Interval5Sec,
    Interval10Sec,
    Interval30Sec,
    Interval1Min,
    Interval2Min,
    Interval5Min,
    Interval10Min,
    Interval20Min,
    Interval30Min,
    Interval45Min,
    Interval1Hr,
    Interval2Hrs,
    Interval4Hrs,
    Interval8Hrs,
    Interval12Hrs,
    Interval24Hrs,
    IntervalSettingsNum
};

#define INTERVAL_COUNT IntervalSettingsNum
static const char* const interval_text[INTERVAL_COUNT] = {
    SCHEDULER_UI_TEXT("1 sec", "1 秒"),
    SCHEDULER_UI_TEXT("2 sec", "2 秒"),
    SCHEDULER_UI_TEXT("5 sec", "5 秒"),
    SCHEDULER_UI_TEXT("10 sec", "10 秒"),
    SCHEDULER_UI_TEXT("30 sec", "30 秒"),
    SCHEDULER_UI_TEXT("1 min", "1 分"),
    SCHEDULER_UI_TEXT("2 min", "2 分"),
    SCHEDULER_UI_TEXT("5 min", "5 分"),
    SCHEDULER_UI_TEXT("10 min", "10 分"),
    SCHEDULER_UI_TEXT("20 min", "20 分"),
    SCHEDULER_UI_TEXT("30 min", "30 分"),
    SCHEDULER_UI_TEXT("45 min", "45 分"),
    SCHEDULER_UI_TEXT("1 hr", "1 时"),
    SCHEDULER_UI_TEXT("2 hrs", "2 时"),
    SCHEDULER_UI_TEXT("4 hrs", "4 时"),
    SCHEDULER_UI_TEXT("8 hrs", "8 时"),
    SCHEDULER_UI_TEXT("12 hrs", "12 时"),
    SCHEDULER_UI_TEXT("24 hrs", "24 时")};
static const uint32_t interval_second_value[INTERVAL_COUNT] =
    {1, 2, 5, 10, 30, 60, 120, 300, 600, 1200, 1800, 2700, 3600, 7200, 14400, 28800, 43200, 86400};

#define REPEATS_COUNT 6
static const char* const tx_repeats_text[REPEATS_COUNT] = {
    "x1",
    "x2",
    "x3",
    "x4",
    "x5",
    "x6",
};
