#include "../minesweeper.h"

#ifdef MOMENTUM_UI_LANG_ZH_CN
static const char* info_string =
    "--   游戏说明   --\n\n"
    "1. 按 OK 清除格子。\n\n"
    "2. 在数字格上长按 OK，\n"
    "若周围旗子数量正确，\n"
    "会自动清除周围格子。\n\n"
    "3. 在格子上长按 Back\n"
    "可插旗标记。\n\n"
    "4. 在已清除格上长按 Back\n"
    "可跳到最近的目标格。\n\n"
    "---   设置说明   ---\n\n"
    "可在设置中调整难度和\n"
    "地图尺寸，最大为 1024\n"
    "格(32x32)。修改这些\n"
    "设置时会提示确认，\n"
    "因为它会重置棋盘。\n\n"
    "----   反馈   ----\n"
    "该选项用于启用/禁用\n"
    "振动和声音反馈。\n\n"
    "-  保证可解  -\n"
    "该选项会在生成新棋盘时\n"
    "启用验证器。\n\n"
    "警告:\n"
    "开启后生成新地图时会有\n"
    "额外开销，可能需要几秒\n"
    "才能得到有效地图。期间\n"
    "界面可能短暂停顿，通常\n"
    "会在几秒内恢复。\n\n"
    "如果要反馈问题，可访问:\n\n"
    "github.com/squee72564/\n"
    "F0_Minesweeper_Fap\n\n"
    "祝你玩得开心!";
#else
static const char* info_string = "--   GAME INFO BELOW   --\n\n"
                                 "1. Press OK to clear a tile.\n\n"
                                 "2. Hold OK on a numbered tile\n"
                                 "to clear all surrounding\n"
                                 "tiles if the correct amount\n"
                                 "of flags are set.\n\n"
                                 "3. Hold Back on a tile to\n"
                                 "mark it with a flag.\n\n"
                                 "4. Hold back on a cleared\n"
                                 "tile to jump to the\n"
                                 "closest tile.\n\n"
                                 "---    SETTINGS INFO    ---\n\n"
                                 "Difficulty and map\n"
                                 "dimensions can be changed\n"
                                 "in the settings with a\n"
                                 "max map size of 1024\n"
                                 "tiles (32x32).\n"
                                 "You will be prompted to\n"
                                 "confirm any changes to these\n"
                                 "settings as it will reset the\n"
                                 "board.\n\n"
                                 "----      FEEDBACK     ----\n"
                                 "This settings enables/disables\n"
                                 "Haptic/Sound feedback for the\n"
                                 "game.\n\n"
                                 "-  ENSURE SOLVABLE  -\n"
                                 "This is a setting that\n"
                                 "enables a board verifier\n"
                                 "when generating a new\n"
                                 "board.\n\n"
                                 "WARNING!:\n"
                                 "This setting will introduce\n"
                                 "a variable amount of\n"
                                 "overhead when generating\n"
                                 "a new map. It can take\n"
                                 "several seconds for a\n"
                                 "valid map to generate. The\n"
                                 "UI may hang and stop for a\n"
                                 "while but it should resolve\n"
                                 "in a few seconds.\n\n"
                                 "Enjoy the game and if you\n"
                                 "want to reach out about an\n"
                                 "issue go to the git hub repo\n"
                                 "for this app:\n\n"
                                 "'github.com/squee72564/\n"
                                 "F0_Minesweeper_Fap'\n\n"
                                 "Thanks and enjoy!";
#endif

void minesweeper_scene_info_screen_on_enter(void* context) {
    furi_assert(context);

    MineSweeperApp* app = (MineSweeperApp*)context;

    text_box_set_text(app->info_screen, info_string);
    text_box_set_font(app->info_screen, TextBoxFontText);
    text_box_set_focus(app->info_screen, TextBoxFocusStart);

    view_dispatcher_switch_to_view(app->view_dispatcher, MineSweeperInfoView);
}

bool minesweeper_scene_info_screen_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);

    MineSweeperApp* app = context;
    UNUSED(event);
    UNUSED(app);

    bool consumed = false;

    return consumed;
}

void minesweeper_scene_info_screen_on_exit(void* context) {
    furi_assert(context);
    MineSweeperApp* app = (MineSweeperApp*)context;

    text_box_reset(app->info_screen);
}
