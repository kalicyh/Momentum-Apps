#include "sam_startup_ui.h"

#include <stdio.h>

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define SEADER_STARTUP_TEXT(en, zh) (zh)
#else
#define SEADER_STARTUP_TEXT(en, zh) (en)
#endif

const char* seader_startup_stage_header(SeaderStartupStage stage) {
    switch(stage) {
    case SeaderStartupStageRetryingBoard:
        return SEADER_STARTUP_TEXT("Retrying Board", "正在重试读卡板");
    case SeaderStartupStageCheckingSam:
        return SEADER_STARTUP_TEXT("Checking SAM", "检查 SAM");
    case SeaderStartupStageNone:
    default:
        return SEADER_STARTUP_TEXT("Starting", "启动中");
    }
}

const char* seader_startup_stage_text(SeaderStartupStage stage) {
    switch(stage) {
    case SeaderStartupStageRetryingBoard:
        return SEADER_STARTUP_TEXT("Power cycle\nand retry", "重新上电\n并重试");
    case SeaderStartupStageCheckingSam:
        return SEADER_STARTUP_TEXT("Waiting for\nCCID/SAM", "等待\nCCID/SAM");
    case SeaderStartupStageNone:
    default:
        return NULL;
    }
}

const char* seader_board_status_detail_title(SeaderBoardStatus status) {
    switch(status) {
    case SeaderBoardStatusFaultPreEnable:
    case SeaderBoardStatusFaultPostEnable:
        return SEADER_STARTUP_TEXT("Board Fault", "读卡板故障");
    case SeaderBoardStatusNoResponse:
        return SEADER_STARTUP_TEXT("No Response", "无响应");
    case SeaderBoardStatusPowerLost:
        return SEADER_STARTUP_TEXT("Power Lost", "供电丢失");
    case SeaderBoardStatusRetryRequested:
        return SEADER_STARTUP_TEXT("Retry Board", "重试读卡板");
    case SeaderBoardStatusPowerReadyPendingValidation:
        return SEADER_STARTUP_TEXT("Checking SAM", "检查 SAM");
    case SeaderBoardStatusReady:
    case SeaderBoardStatusUnknown:
    default:
        return SEADER_STARTUP_TEXT("No SAM Found", "未找到 SAM");
    }
}

const char* seader_board_status_detail_body(SeaderBoardStatus status, bool retry_exhausted) {
    switch(status) {
    case SeaderBoardStatusFaultPreEnable:
        return SEADER_STARTUP_TEXT("5V fault before\nenable", "启用前\n5V 故障");
    case SeaderBoardStatusFaultPostEnable:
        return SEADER_STARTUP_TEXT("5V fault after\nenable", "启用后\n5V 故障");
    case SeaderBoardStatusNoResponse:
        return retry_exhausted ?
                   SEADER_STARTUP_TEXT("Board powered,\nno SAM after retry", "读卡板已上电\n重试后无 SAM") :
                   SEADER_STARTUP_TEXT("Board powered,\nno CCID/SAM reply", "读卡板已上电\n无 CCID/SAM 响应");
    case SeaderBoardStatusPowerLost:
        return SEADER_STARTUP_TEXT("USB/5V removed\nboard unpowered", "USB/5V 断开\n读卡板未供电");
    case SeaderBoardStatusRetryRequested:
        return SEADER_STARTUP_TEXT("Power cycle the\nboard and retry", "重启读卡板\n并重试");
    case SeaderBoardStatusPowerReadyPendingValidation:
        return SEADER_STARTUP_TEXT("Board powered,\nchecking SAM", "读卡板已上电\n正在检查 SAM");
    case SeaderBoardStatusReady:
    case SeaderBoardStatusUnknown:
    default:
        return SEADER_STARTUP_TEXT("No SAM detected\non the board", "读卡板上\n未检测到 SAM");
    }
}

const char* seader_board_status_detail_hint(SeaderBoardStatus status) {
    switch(status) {
    case SeaderBoardStatusFaultPreEnable:
    case SeaderBoardStatusFaultPostEnable:
        return SEADER_STARTUP_TEXT("Check board/cable", "检查读卡板/线缆");
    case SeaderBoardStatusNoResponse:
        return SEADER_STARTUP_TEXT("Reseat board/SAM", "重插读卡板/SAM");
    case SeaderBoardStatusPowerLost:
        return SEADER_STARTUP_TEXT("Reconnect power", "重新连接电源");
    case SeaderBoardStatusRetryRequested:
        return SEADER_STARTUP_TEXT("Retry bus check", "重试总线检查");
    case SeaderBoardStatusPowerReadyPendingValidation:
        return SEADER_STARTUP_TEXT("Wait for SAM", "等待 SAM");
    case SeaderBoardStatusReady:
    case SeaderBoardStatusUnknown:
    default:
        return SEADER_STARTUP_TEXT("Insert supported SAM", "插入支持的 SAM");
    }
}

size_t seader_format_atr_summary(const uint8_t* atr, size_t len, char* out, size_t out_size) {
    if(!out || out_size == 0U) {
        return 0U;
    }

    if(!atr || len == 0U) {
        return (size_t)snprintf(out, out_size, "ATR: unavailable");
    }

    const size_t shown = len < 6U ? len : 6U;
    size_t used = (size_t)snprintf(out, out_size, "ATR:");
    for(size_t i = 0; i < shown && used + 4U < out_size; i++) {
        used += (size_t)snprintf(out + used, out_size - used, " %02X", atr[i]);
    }

    if(len > shown && used + 4U < out_size) {
        used += (size_t)snprintf(out + used, out_size - used, "...");
    }

    return used;
}
