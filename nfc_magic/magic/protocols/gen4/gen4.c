#include "gen4.h"
#include "core/check.h"

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define GEN4_UI_TEXT(en, zh) (zh)
#else
#define GEN4_UI_TEXT(en, zh) (en)
#endif

Gen4* gen4_alloc() {
    Gen4* instance = malloc(sizeof(Gen4));

    return instance;
}

void gen4_free(Gen4* instance) {
    furi_check(instance);

    free(instance);
}

void gen4_reset(Gen4* instance) {
    furi_check(instance);

    memset(&instance->config, 0, sizeof(Gen4Config));
    memset(&instance->revision, 0, sizeof(Gen4Revision));
}

void gen4_copy(Gen4* dest, const Gen4* source) {
    furi_check(dest);
    furi_check(source);

    memcpy(dest, source, sizeof(Gen4));
}

bool gen4_password_is_set(const Gen4Password* instance) {
    furi_check(instance);

    return (instance->bytes[0] || instance->bytes[1] || instance->bytes[2] || instance->bytes[3]);
}

void gen4_password_reset(Gen4Password* instance) {
    furi_check(instance);

    memset(instance->bytes, 0, GEN4_PASSWORD_LEN);
}

void gen4_password_copy(Gen4Password* dest, const Gen4Password* source) {
    furi_check(dest);
    furi_check(source);

    memcpy(dest->bytes, source->bytes, GEN4_PASSWORD_LEN);
}

const char* gen4_get_shadow_mode_name(Gen4ShadowMode mode) {
    switch(mode) {
    case Gen4ShadowModePreWrite:
        return GEN4_UI_TEXT("Pre-Write", "预写入");
    case Gen4ShadowModeRestore:
        return GEN4_UI_TEXT("Restore", "恢复");
    case Gen4ShadowModeDisabled:
        return GEN4_UI_TEXT("Disabled", "已禁用");
    case Gen4ShadowModeHighSpeedDisabled:
        return GEN4_UI_TEXT("Disabled (High-speed)", "已禁用 (高速)");
    case Gen4ShadowModeSplit:
        return GEN4_UI_TEXT("Split", "分离");
    default:
        return GEN4_UI_TEXT("Unknown", "未知");
    }
}

const char* gen4_get_direct_write_mode_name(Gen4DirectWriteBlock0Mode mode) {
    switch(mode) {
    case Gen4DirectWriteBlock0ModeEnabled:
        return GEN4_UI_TEXT("Enabled", "已启用");
    case Gen4DirectWriteBlock0ModeDisabled:
        return GEN4_UI_TEXT("Disabled", "已禁用");
    case Gen4DirectWriteBlock0ModeDefault:
        return GEN4_UI_TEXT("Default", "默认");
    default:
        return GEN4_UI_TEXT("Unknown", "未知");
    }
}

const char* gen4_get_uid_len_num(Gen4UIDLength code) {
    switch(code) {
    case Gen4UIDLengthSingle:
        return "4";
    case Gen4UIDLengthDouble:
        return "7";
    case Gen4UIDLengthTriple:
        return "10";
    default:
        return GEN4_UI_TEXT("Unknown", "未知");
    }
}

const char* gen4_get_configuration_name(const Gen4Config* config) {
    switch(config->data_parsed.protocol) {
    case Gen4ProtocolMfClassic: {
        switch(config->data_parsed.total_blocks) {
        case 255:
            return "MIFARE Classic 4K";
        case 63:
            return "MIFARE Classic 1K";
        case 19:
            return "MIFARE Classic Mini (0.3K)";
        default:
            return GEN4_UI_TEXT("Unknown", "未知");
        }
    } break;
    case Gen4ProtocolMfUltralight: {
        switch(config->data_parsed.total_blocks) {
        case 63:
            return "MIFARE Ultralight";
        case 127:
            return "NTAG 2XX";
        default:
            return GEN4_UI_TEXT("Unknown", "未知");
        }
    } break;
    default:
        return GEN4_UI_TEXT("Unknown", "未知");
        break;
    };
}
