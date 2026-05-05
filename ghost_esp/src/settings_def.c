// settings_def.c
#include "settings_def.h"
#include "menu.h"
#include <stddef.h>
#include "callbacks.h"
const char* const SETTING_VALUE_NAMES_RGB_MODE[] = {
    GHOST_ESP_UI_TEXT("Stealth", "隐身"),
    GHOST_ESP_UI_TEXT("Normal", "正常"),
    GHOST_ESP_UI_TEXT("Rainbow", "彩虹")};
const char* const SETTING_VALUE_NAMES_CHANNEL_HOP[] =
    {"500ms", "1000ms", "2000ms", "3000ms", "4000ms"};
const char* const SETTING_VALUE_NAMES_BOOL[] = {
    GHOST_ESP_UI_TEXT("False", "关"),
    GHOST_ESP_UI_TEXT("True", "开")};
const char* const SETTING_VALUE_NAMES_ACTION[] = {
    GHOST_ESP_UI_TEXT("Press OK", "按OK"),
    GHOST_ESP_UI_TEXT("Press OK", "按OK")};
const char* const SETTING_VALUE_NAMES_LOG_VIEW[] = {
    GHOST_ESP_UI_TEXT("End", "末尾"),
    GHOST_ESP_UI_TEXT("Start", "开头")};

#include "settings_ui.h"

const SettingMetadata SETTING_METADATA[SETTINGS_COUNT] = {
    [SETTING_RGB_MODE] =
        {.name = GHOST_ESP_UI_TEXT("RGB Mode", "RGB 模式"),
         .data.setting =
             {.max_value = RGB_MODE_COUNT - 1,
              .value_names = SETTING_VALUE_NAMES_RGB_MODE,
              .uart_command = NULL},
         .is_action = false},
    [SETTING_CHANNEL_HOP_DELAY] =
        {.name = GHOST_ESP_UI_TEXT("Channel Switch Delay", "信道切换延迟"),
         .data.setting =
             {.max_value = CHANNEL_HOP_COUNT - 1,
              .value_names = SETTING_VALUE_NAMES_CHANNEL_HOP,
              .uart_command = "setsetting -i 2 -v"},
         .is_action = false},
    [SETTING_ENABLE_CHANNEL_HOPPING] =
        {.name = GHOST_ESP_UI_TEXT("Enable Channel Hopping", "启用信道跳频"),
         .data.setting =
             {.max_value = 1,
              .value_names = SETTING_VALUE_NAMES_BOOL,
              .uart_command = "setsetting -i 3 -v"},
         .is_action = false},
    [SETTING_ENABLE_RANDOM_BLE_MAC] =
        {.name = GHOST_ESP_UI_TEXT("Enable Random BLE Mac", "启用随机 BLE MAC"),
         .data.setting =
             {.max_value = 1,
              .value_names = SETTING_VALUE_NAMES_BOOL,
              .uart_command = "setsetting -i 4 -v"},
         .is_action = false},
    [SETTING_STOP_ON_BACK] =
        {.name = GHOST_ESP_UI_TEXT("Send Stop On Back", "返回时发送停止"),
         .data.setting =
             {.max_value = 1, .value_names = SETTING_VALUE_NAMES_BOOL, .uart_command = NULL},
         .is_action = false},
    [SETTING_ENABLE_FILTERING] =
        {.name = GHOST_ESP_UI_TEXT("(BETA) Enable UART Filtering", "(测试) 启用 UART 过滤"),
         .data.setting =
             {.max_value = 1, .value_names = SETTING_VALUE_NAMES_BOOL, .uart_command = NULL},
         .is_action = false},
    [SETTING_SHOW_INFO] =
        {.name = GHOST_ESP_UI_TEXT("App Info", "应用信息"),
         .data.action =
             {.name = GHOST_ESP_UI_TEXT("Show Info", "查看信息"),
              .command = NULL,
              .callback = &show_app_info},
         .is_action = true},
    [SETTING_REBOOT_ESP] =
        {.name = GHOST_ESP_UI_TEXT("Reboot ESP", "重启 ESP"),
         .data.action =
             {.name = GHOST_ESP_UI_TEXT("Reboot ESP", "重启 ESP"),
              .command = "handle_reboot",
              .callback = NULL},
         .is_action = true},
    [SETTING_CLEAR_LOGS] =
        {.name = GHOST_ESP_UI_TEXT("Clear Log Files", "清除日志"),
         .data.action =
             {.name = GHOST_ESP_UI_TEXT("Clear Log Files", "清除日志"),
              .command = NULL,
              .callback = &clear_log_files},
         .is_action = true},
    [SETTING_CLEAR_NVS] =
        {.name = GHOST_ESP_UI_TEXT("Clear NVS", "清除 NVS"),
         .data.action =
             {.name = GHOST_ESP_UI_TEXT("Clear NVS", "清除 NVS"),
              .command = "handle_clearnvs",
              .callback = NULL},
         .is_action = true},
    [SETTING_VIEW_LOGS_FROM_START] =
        {.name = GHOST_ESP_UI_TEXT("View Logs From", "日志查看起点"),
         .data.setting =
             {.max_value = 1, .value_names = SETTING_VALUE_NAMES_LOG_VIEW, .uart_command = NULL},
         .is_action = false},
    [SETTING_CLEAR_PCAPS] =
        {.name = GHOST_ESP_UI_TEXT("Clear PCAPs", "清除 PCAP"),
         .data.action =
             {.name = GHOST_ESP_UI_TEXT("Clear PCAPs", "清除 PCAP"),
              .command = NULL,
              .callback = &clear_pcap_files},
         .is_action = true},
    [SETTING_CLEAR_WARDRIVE] =
        {.name = GHOST_ESP_UI_TEXT("Clear Wardrives", "清除 Wardrive"),
         .data.action =
             {.name = GHOST_ESP_UI_TEXT("Clear Wardrives", "清除 Wardrive"),
              .command = NULL,
              .callback = &clear_wardrive_files},
         .is_action = true},
    [SETTING_DISABLE_ESP_CHECK] = {
        .name = GHOST_ESP_UI_TEXT("Disable ESP Check", "禁用 ESP 检测"),
        .data.setting =
            {.max_value = 1, .value_names = SETTING_VALUE_NAMES_BOOL, .uart_command = NULL},
        .is_action = false}};

bool setting_is_visible(SettingKey key) {
    if(key == SETTING_ENABLE_FILTERING || key == SETTING_CHANNEL_HOP_DELAY ||
       key == SETTING_ENABLE_CHANNEL_HOPPING || key == SETTING_ENABLE_RANDOM_BLE_MAC) {
        return false;
    }
    return true;
}

const SettingMetadata* settings_get_metadata(SettingKey key) {
    if(key >= SETTINGS_COUNT) {
        return NULL;
    }
    return &SETTING_METADATA[key];
}

// 6675636B796F7564656B69
