#include "../flipcheckers.h"

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define FLIPCHECKERS_UI_TEXT(en, zh) (zh)
#else
#define FLIPCHECKERS_UI_TEXT(en, zh) (en)
#endif

#define ABOUT_TEXT_EN                                    \
    "\e#Flip Checkers\e# " FLIPCHECKERS_VERSION "\n" \
    "\n"                                              \
    "Classic checkers game for\n"                     \
    "Flipper Zero.\n"                                 \
    "\n"                                              \
    "Features:\n"                                     \
    "- Multi-jump captures\n"                         \
    "- CPU opponents (levels 1-3)\n"                  \
    "- Mandatory jump rule\n"                         \
    "- Save & resume games\n"                         \
    "\n"                                              \
    "Controls:\n"                                     \
    "- D-pad: move cursor\n"                          \
    "- OK: select / confirm\n"                        \
    "- Back: cancel / menu"

#define ABOUT_TEXT_ZH                                    \
    "\e#Flip Checkers\e# " FLIPCHECKERS_VERSION "\n" \
    "\n"                                              \
    "Flipper Zero 经典跳棋\n"                          \
    "\n"                                              \
    "功能:\n"                                          \
    "- 连续跳吃\n"                                     \
    "- AI 对手 (1-3级)\n"                              \
    "- 强制跳吃规则\n"                                  \
    "- 保存/恢复游戏\n"                                 \
    "\n"                                              \
    "操作:\n"                                          \
    "- 方向键: 移动光标\n"                              \
    "- OK: 选择/确认\n"                                 \
    "- 返回: 取消/菜单"

#define ABOUT_TEXT FLIPCHECKERS_UI_TEXT(ABOUT_TEXT_EN, ABOUT_TEXT_ZH)

void flipcheckers_scene_about_on_enter(void* context) {
    furi_assert(context);
    FlipCheckers* app = context;

    text_box_set_font(app->text_box, TextBoxFontText);
    text_box_set_text(app->text_box, ABOUT_TEXT);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipCheckersViewIdAbout);
}

bool flipcheckers_scene_about_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void flipcheckers_scene_about_on_exit(void* context) {
    furi_assert(context);
    FlipCheckers* app = context;
    text_box_reset(app->text_box);
}
