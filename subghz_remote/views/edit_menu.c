#include "edit_menu.h"
#include "../subghz_remote_app_i.h"
#include "../helpers/subrem_custom_button_info.h"

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define SUBREM_UI_TEXT(en, zh) (zh)
#else
#define SUBREM_UI_TEXT(en, zh) (en)
#endif

#include <input/input.h>
#include <gui/elements.h>

#define subrem_view_edit_menu_MAX_LABEL_LENGTH 12

#define FRAME_HEIGHT 12

struct SubRemViewEditMenu {
    View* view;
    SubRemViewEditMenuCallback callback;
    void* context;
};

typedef struct {
    FuriString* label;
    FuriString* file_path;
    SubRemLoadSubState sub_state;

    uint8_t chosen;
} SubRemViewEditMenuModel;

void subrem_view_edit_menu_set_callback(
    SubRemViewEditMenu* subrem_view_edit_menu,
    SubRemViewEditMenuCallback callback,
    void* context) {
    furi_assert(subrem_view_edit_menu);

    subrem_view_edit_menu->callback = callback;
    subrem_view_edit_menu->context = context;
}

void subrem_view_edit_menu_add_data_to_show(
    SubRemViewEditMenu* subrem_view_edit_remote,
    uint8_t index,
    FuriString* label,
    FuriString* path,
    uint8_t button,
    SubRemLoadSubState state) {
    furi_assert(subrem_view_edit_remote);

    with_view_model(
        subrem_view_edit_remote->view,
        SubRemViewEditMenuModel * model,
        {
            model->chosen = index;
            if(!furi_string_empty(label)) {
                furi_string_set(model->label, label);
            } else {
                furi_string_set(model->label, SUBREM_UI_TEXT("Empty label", "空标签"));
            }
            furi_string_set(model->file_path, path);
            if(button != ButtonOK && !furi_string_empty(path)) {
                furi_string_cat_printf(model->file_path, " (Btn. %s)", custom_button_text[button]);
            }
            model->sub_state = state;
        },
        true);
}

uint8_t subrem_view_edit_menu_get_index(SubRemViewEditMenu* subrem_view_edit_remote) {
    furi_assert(subrem_view_edit_remote);
    uint8_t index;

    with_view_model(
        subrem_view_edit_remote->view,
        SubRemViewEditMenuModel * model,
        { index = model->chosen; },
        true);
    return index;
}

void subrem_view_edit_menu_draw(Canvas* canvas, SubRemViewEditMenuModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_clear(canvas);

    // Draw bottom btn
    canvas_set_font(canvas, FontSecondary);
    elements_button_left(canvas, SUBREM_UI_TEXT("Back", "返回"));
    elements_button_center(canvas, SUBREM_UI_TEXT("Edit", "编辑"));
    elements_button_right(canvas, SUBREM_UI_TEXT("Save", "保存"));

    // Draw top frame
    canvas_draw_line(canvas, 1, 0, 125, 0);
    canvas_draw_box(canvas, 0, 1, 127, FRAME_HEIGHT - 2);
    canvas_draw_line(canvas, 1, FRAME_HEIGHT - 1, 125, FRAME_HEIGHT - 1);

    canvas_set_color(canvas, ColorWhite);

    // Draw btn name
    canvas_set_font(canvas, FontPrimary);
    switch(model->chosen) {
    case SubRemSubKeyNameUp:
        canvas_draw_str(canvas, 3, FRAME_HEIGHT - 2, SUBREM_UI_TEXT("UP", "上"));
        break;

    case SubRemSubKeyNameDown:
        canvas_draw_str(canvas, 3, FRAME_HEIGHT - 2, SUBREM_UI_TEXT("DOWN", "下"));
        break;

    case SubRemSubKeyNameLeft:
        canvas_draw_str(canvas, 3, FRAME_HEIGHT - 2, SUBREM_UI_TEXT("LEFT", "左"));
        break;

    case SubRemSubKeyNameRight:
        canvas_draw_str(canvas, 3, FRAME_HEIGHT - 2, SUBREM_UI_TEXT("RIGHT", "右"));
        break;

    case SubRemSubKeyNameOk:
        canvas_draw_str(canvas, 3, FRAME_HEIGHT - 2, SUBREM_UI_TEXT("OK", "确认"));
        break;

    default:
        break;
    }

    // Draw Label
    canvas_set_font(canvas, FontSecondary);
    elements_text_box(
        canvas,
        38,
        2,
        127 - 38,
        FRAME_HEIGHT,
        AlignCenter,
        AlignBottom,
        furi_string_empty(model->label) ? SUBREM_UI_TEXT("Empty label", "空标签") : furi_string_get_cstr(model->label),
        true);

    // Draw arrow
    canvas_set_color(canvas, ColorBlack);
    if(model->chosen != 0) {
        canvas_draw_icon(canvas, 119, 13, &I_Pin_arrow_up_7x9);
    }
    if(model->chosen != 4) {
        canvas_draw_icon_ex(canvas, 119, 42, &I_Pin_arrow_up_7x9, IconRotation180);
    }

    // Draw file_path
    if(model->sub_state == SubRemLoadSubStateOK) {
        canvas_set_font(canvas, FontSecondary);
        elements_text_box(
            canvas,
            1,
            FRAME_HEIGHT + 1,
            118,
            (63 - FRAME_HEIGHT * 2),
            AlignLeft,
            AlignTop,
            furi_string_get_cstr(model->file_path),
            false);
    } else if(furi_string_empty(model->file_path)) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 1, FRAME_HEIGHT * 2 - 2, SUBREM_UI_TEXT("Button not set", "按键未设置"));
    } else {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 1, FRAME_HEIGHT * 2 - 2, SUBREM_UI_TEXT("ERR:", "错误:"));
        canvas_set_font(canvas, FontSecondary);
        switch(model->sub_state) {
        case SubRemLoadSubStateErrorNoFile:
            canvas_draw_str(canvas, 26, FRAME_HEIGHT * 2 - 2, SUBREM_UI_TEXT("File not found", "文件未找到"));
            break;
        case SubRemLoadSubStateErrorFreq:
            canvas_draw_str(canvas, 26, FRAME_HEIGHT * 2 - 2, SUBREM_UI_TEXT("Bad frequency", "频率错误"));
            break;
        case SubRemLoadSubStateErrorMod:
            canvas_draw_str(canvas, 26, FRAME_HEIGHT * 2 - 2, SUBREM_UI_TEXT("Bad modulation", "调制方式错误"));
            break;
        case SubRemLoadSubStateErrorProtocol:
            canvas_draw_str(canvas, 26, FRAME_HEIGHT * 2 - 2, SUBREM_UI_TEXT("Unsupported protocol", "不支持的协议"));
            break;

        default:
            break;
        }
        elements_text_box(
            canvas,
            1,
            FRAME_HEIGHT * 2,
            118,
            30,
            AlignLeft,
            AlignTop,
            furi_string_get_cstr(model->file_path),
            false);
    }
}

bool subrem_view_edit_menu_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubRemViewEditMenu* subrem_view_edit_menu = context;

    if((event->key == InputKeyBack || event->key == InputKeyLeft) &&
       event->type == InputTypeShort) {
        subrem_view_edit_menu->callback(
            SubRemCustomEventViewEditMenuBack, subrem_view_edit_menu->context);
        return true;
    } else if(event->key == InputKeyUp && event->type == InputTypeShort) {
        with_view_model(
            subrem_view_edit_menu->view,
            SubRemViewEditMenuModel * model,
            {
                if(model->chosen > 0) {
                    model->chosen -= 1;
                };
            },
            true);
        subrem_view_edit_menu->callback(
            SubRemCustomEventViewEditMenuUP, subrem_view_edit_menu->context);
        return true;
    } else if(event->key == InputKeyDown && event->type == InputTypeShort) {
        with_view_model(
            subrem_view_edit_menu->view,
            SubRemViewEditMenuModel * model,
            {
                if(model->chosen < 4) {
                    model->chosen += 1;
                };
            },
            true);
        subrem_view_edit_menu->callback(
            SubRemCustomEventViewEditMenuDOWN, subrem_view_edit_menu->context);
        return true;
    } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
        subrem_view_edit_menu->callback(
            SubRemCustomEventViewEditMenuEdit, subrem_view_edit_menu->context);
        return true;
    } else if(event->key == InputKeyRight && event->type == InputTypeShort) {
        subrem_view_edit_menu->callback(
            SubRemCustomEventViewEditMenuSave, subrem_view_edit_menu->context);
        return true;
    }

    return true;
}

void subrem_view_edit_menu_enter(void* context) {
    furi_assert(context);
}

void subrem_view_edit_menu_exit(void* context) {
    furi_assert(context);
}

SubRemViewEditMenu* subrem_view_edit_menu_alloc(void) {
    SubRemViewEditMenu* subrem_view_edit_menu = malloc(sizeof(SubRemViewEditMenu));

    // View allocation and configuration
    subrem_view_edit_menu->view = view_alloc();
    view_allocate_model(
        subrem_view_edit_menu->view, ViewModelTypeLocking, sizeof(SubRemViewEditMenuModel));
    view_set_context(subrem_view_edit_menu->view, subrem_view_edit_menu);
    view_set_draw_callback(
        subrem_view_edit_menu->view, (ViewDrawCallback)subrem_view_edit_menu_draw);
    view_set_input_callback(subrem_view_edit_menu->view, subrem_view_edit_menu_input);
    view_set_enter_callback(subrem_view_edit_menu->view, subrem_view_edit_menu_enter);
    view_set_exit_callback(subrem_view_edit_menu->view, subrem_view_edit_menu_exit);

    with_view_model(
        subrem_view_edit_menu->view,
        SubRemViewEditMenuModel * model,
        {
            model->label = furi_string_alloc(); // furi_string_alloc_set_str("LABEL");
            model->file_path = furi_string_alloc(); // furi_string_alloc_set_str("FILE_PATH");

            model->chosen = 0;
        },
        true);
    return subrem_view_edit_menu;
}

void subrem_view_edit_menu_free(SubRemViewEditMenu* subghz_edit_menu) {
    furi_assert(subghz_edit_menu);

    with_view_model(
        subghz_edit_menu->view,
        SubRemViewEditMenuModel * model,
        {
            furi_string_free(model->label);
            furi_string_free(model->file_path);
        },
        true);
    view_free(subghz_edit_menu->view);
    free(subghz_edit_menu);
}

View* subrem_view_edit_menu_get_view(SubRemViewEditMenu* subrem_view_edit_menu) {
    furi_assert(subrem_view_edit_menu);
    return subrem_view_edit_menu->view;
}
