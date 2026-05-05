#include "barcode_utils.h"

#ifndef BARCODE_GEN_UI_TEXT
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define BARCODE_GEN_UI_TEXT(en, zh) (zh)
#else
#define BARCODE_GEN_UI_TEXT(en, zh) (en)
#endif
#endif

BarcodeTypeObj* barcode_type_objs[NUMBER_OF_BARCODE_TYPES] = {NULL};

void init_types() {
    BarcodeTypeObj* upc_a = malloc(sizeof(BarcodeTypeObj));
    upc_a->name = "UPC-A";
    upc_a->type = UPCA;
    upc_a->min_digits = 11;
    upc_a->max_digits = 12;
    upc_a->start_pos = 16;
    barcode_type_objs[UPCA] = upc_a;

    BarcodeTypeObj* ean_8 = malloc(sizeof(BarcodeTypeObj));
    ean_8->name = "EAN-8";
    ean_8->type = EAN8;
    ean_8->min_digits = 7;
    ean_8->max_digits = 8;
    ean_8->start_pos = 32;
    barcode_type_objs[EAN8] = ean_8;

    BarcodeTypeObj* ean_13 = malloc(sizeof(BarcodeTypeObj));
    ean_13->name = "EAN-13";
    ean_13->type = EAN13;
    ean_13->min_digits = 12;
    ean_13->max_digits = 13;
    ean_13->start_pos = 16;
    barcode_type_objs[EAN13] = ean_13;

    BarcodeTypeObj* code_39 = malloc(sizeof(BarcodeTypeObj));
    code_39->name = "CODE-39";
    code_39->type = CODE39;
    code_39->min_digits = 1;
    code_39->max_digits = -1;
    code_39->start_pos = 0;
    barcode_type_objs[CODE39] = code_39;

    BarcodeTypeObj* code_128 = malloc(sizeof(BarcodeTypeObj));
    code_128->name = "CODE-128";
    code_128->type = CODE128;
    code_128->min_digits = 1;
    code_128->max_digits = -1;
    code_128->start_pos = 0;
    barcode_type_objs[CODE128] = code_128;

    BarcodeTypeObj* code_128c = malloc(sizeof(BarcodeTypeObj));
    code_128c->name = "CODE-128C";
    code_128c->type = CODE128C;
    code_128c->min_digits = 2;
    code_128c->max_digits = -1;
    code_128c->start_pos = 0;
    barcode_type_objs[CODE128C] = code_128c;

    BarcodeTypeObj* codabar = malloc(sizeof(BarcodeTypeObj));
    codabar->name = "Codabar";
    codabar->type = CODABAR;
    codabar->min_digits = 1;
    codabar->max_digits = -1;
    codabar->start_pos = 0;
    barcode_type_objs[CODABAR] = codabar;

    BarcodeTypeObj* unknown = malloc(sizeof(BarcodeTypeObj));
    unknown->name = "Unknown";
    unknown->type = UNKNOWN;
    unknown->min_digits = 0;
    unknown->max_digits = 0;
    unknown->start_pos = 0;
    barcode_type_objs[UNKNOWN] = unknown;
}

void free_types() {
    for(int i = 0; i < NUMBER_OF_BARCODE_TYPES; i++) {
        free(barcode_type_objs[i]);
    }
}

BarcodeTypeObj* get_type(FuriString* type_string) {
    if(furi_string_cmp_str(type_string, "UPC-A") == 0) {
        return barcode_type_objs[UPCA];
    }
    if(furi_string_cmp_str(type_string, "EAN-8") == 0) {
        return barcode_type_objs[EAN8];
    }
    if(furi_string_cmp_str(type_string, "EAN-13") == 0) {
        return barcode_type_objs[EAN13];
    }
    if(furi_string_cmp_str(type_string, "CODE-39") == 0) {
        return barcode_type_objs[CODE39];
    }
    if(furi_string_cmp_str(type_string, "CODE-128") == 0) {
        return barcode_type_objs[CODE128];
    }
    if(furi_string_cmp_str(type_string, "CODE-128C") == 0) {
        return barcode_type_objs[CODE128C];
    }
    if(furi_string_cmp_str(type_string, "Codabar") == 0) {
        return barcode_type_objs[CODABAR];
    }

    return barcode_type_objs[UNKNOWN];
}

const char* get_error_code_name(ErrorCode error_code) {
    switch(error_code) {
    case WrongNumberOfDigits:
        return BARCODE_GEN_UI_TEXT("Wrong # Of Characters", "字符数量错误");
    case InvalidCharacters:
        return BARCODE_GEN_UI_TEXT("Invalid Characters", "无效字符");
    case UnsupportedType:
        return BARCODE_GEN_UI_TEXT("Unsupported Type", "不支持的类型");
    case FileOpening:
        return BARCODE_GEN_UI_TEXT("File Opening Error", "文件打开错误");
    case InvalidFileData:
        return BARCODE_GEN_UI_TEXT("Invalid File Data", "无效的文件数据");
    case MissingEncodingTable:
        return BARCODE_GEN_UI_TEXT("Missing Encoding Table", "缺少编码表");
    case EncodingTableError:
        return BARCODE_GEN_UI_TEXT("Encoding Table Error", "编码表错误");
    case OKCode:
        return "OK";
    default:
        return BARCODE_GEN_UI_TEXT("Unknown Code", "未知错误");
    };
}

const char* get_error_code_message(ErrorCode error_code) {
    switch(error_code) {
    case WrongNumberOfDigits:
        return BARCODE_GEN_UI_TEXT(
            "The barcode has too many or\ntoo few characters.",
            "条码的字符数量\n过多或过少");
    case InvalidCharacters:
        return BARCODE_GEN_UI_TEXT(
            "The barcode data has invalid\ncharacters",
            "条码数据包含无效字符");
    case UnsupportedType:
        return BARCODE_GEN_UI_TEXT(
            "This barcode type is not\nsupported by this application",
            "此应用程序不支持\n该条码类型");
    case FileOpening:
        return BARCODE_GEN_UI_TEXT(
            "The barcode file could not\nbe opened",
            "无法打开条码文件");
    case InvalidFileData:
        return BARCODE_GEN_UI_TEXT(
            "File data contains incorrect\ninformation",
            "文件数据包含\n不正确的信息");
    case MissingEncodingTable:
        return BARCODE_GEN_UI_TEXT(
            "The encoding table files are\nmissing. Please redownload \nthis app, or consult the \ngithub readme",
            "编码表文件缺失。\n请重新下载此应用\n或查阅github说明");
    case EncodingTableError:
        return BARCODE_GEN_UI_TEXT(
            "Either the characters you\nentered are incorrect or there\nis a problem with the\nencoding table",
            "您输入的字符不正确\n或编码表存在问题");
    case OKCode:
        return "OK";
    default:
        return BARCODE_GEN_UI_TEXT(
            "Could not read barcode data",
            "无法读取条码数据");
    };
}
