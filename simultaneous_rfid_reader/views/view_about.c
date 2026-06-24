#include "view_about.h"

/**
 * @brief      Callback for returning to submenu.
 * @details    This function is called when user press back button.
 * @param      context  The context - unused
 * @return     next view id
*/
uint32_t uhf_reader_navigation_about_submenu_callback(void* context) {
    UNUSED(context);
    return UHFReaderViewSubmenu;
}

/**
 * @brief      Allocates the about screen
 * @details    Allocates the contents of the about screen.
 * @param      app  The UHFReaderApp used to allocate variables 
*/
void view_about_alloc(UHFReaderApp* App) {
    
    //Creating the about widget 
    App->WidgetAbout = widget_alloc();
    FuriString* TmpString = furi_string_alloc();
    widget_add_text_box_element(
        App->WidgetAbout, 0, 0, 128, 14, AlignCenter, AlignBottom, UHF_RFID_BLANK_INV, false);
    widget_add_text_box_element(
        App->WidgetAbout, 0, 0, 128, 14, AlignCenter, AlignBottom, UHF_RFID_NAME, false);
    
    //Adding version and developer information
    furi_string_printf(TmpString, "\e#%s\n", RFID_READER_UI_TEXT("Information:", "信息:"));
    furi_string_cat_printf(TmpString, RFID_READER_UI_TEXT("Version: %s\n", "版本: %s\n"), UHF_RFID_VERSION_APP);
    furi_string_cat_printf(TmpString, RFID_READER_UI_TEXT("Developed by: %s\n", "开发者: %s\n"), UHF_RFID_MEM_DEVELOPER);
    furi_string_cat_printf(TmpString, "Github: %s\n\n", UHF_RFID_GITHUB);
    furi_string_cat_printf(TmpString, "\e#%s\n", RFID_READER_UI_TEXT("Description:", "说明:"));

    //Section with high level overview of app functions
    furi_string_cat_printf(
        TmpString,
        RFID_READER_UI_TEXT(
            "UHF RFID Reader\n"
            "Made for use with a M6E, M7E, or YRM100 compatible reader.\n"
            "Can read up to 150 tags simultaneously using M6E or M7E reader!\n"
            "Can read/write, save, lock, kill, and dump data from read tags.\n\n",
            "UHF RFID 读写器\n"
            "兼容 M6E、M7E 或 YRM100 读写模块。\n"
            "使用 M6E 或 M7E 可同时读取最多150个标签!\n"
            "支持读写、保存、锁定、销毁及数据导出。\n\n"));

    //Hardware requirements
    furi_string_cat_printf(TmpString, "\e#%s\n", RFID_READER_UI_TEXT("Hardware Requirements:", "硬件要求:"));
    furi_string_cat_printf(
        TmpString,
        RFID_READER_UI_TEXT(
            "Any of these options work, however, changes to the wiring and configuration may be necessary.\n"
            "A M6E, M7E, or YRM100 UHF RFID Reader is required!\n"
            "If using a M6E or M7E reader, the following are required:\n"
            "- ThingMagic Nano Embedded RFID Reader Module (M6E or M7E variant)\n"
            "- SparkFun Simultaneous RFID Reader(M6E or M7E variant)\n"
            "- Raspberry Pi Zero (To connect to M6E/M7E board)\n"
            "- Custom UHF RFID Flipper Zero Board (Coming Soon)\n\n",
            "以下方案均可使用,但可能需要调整接线和配置。\n"
            "需要 M6E、M7E 或 YRM100 UHF RFID 读写器!\n"
            "若使用 M6E 或 M7E,还需以下组件:\n"
            "- ThingMagic Nano RFID 模块 (M6E 或 M7E)\n"
            "- SparkFun Simultaneous RFID Reader (M6E 或 M7E)\n"
            "- Raspberry Pi Zero (连接 M6E/M7E)\n"
            "- 定制 UHF RFID Flipper Zero 扩展板 (即将推出)\n\n"));

    //The configuration screen
    furi_string_cat_printf(TmpString, "\e#%s\n", RFID_READER_UI_TEXT("Configuration:", "配置:"));
    furi_string_cat_printf(
        TmpString,
        RFID_READER_UI_TEXT(
            "To operate, select the reader module first, then connect.\n"
            "Next, you may toggle other reader settings.\n"
            " The configuration menu has the following options:\n"
            "- Select reader module\n"
            "- Set the power of the reader(M6E/M7E power range 0-2700, YRM100X 1-27)\n"
            "- Change the Baud Rate\n"
            "- Change the Region\n"
            "- Set default access password for reading/writing\n"
            "- Set write save mode (Turn on to save and update stored fields of tag written)\n"
            "- Save configuration settings (Future)\n"
            "- Set/Detect UHF RFID Tag Type (Future)\n"
            "- Toggle the antenna selection (Future for M6E and M7E Only)\n\n",
            "操作步骤: 先选择读写模块,再连接。\n"
            "然后可调整其他设置。\n"
            "配置菜单包含以下选项:\n"
            "- 选择读写模块\n"
            "- 设置功率 (M6E/M7E: 0-2700, YRM100X: 1-27)\n"
            "- 更改波特率\n"
            "- 更改区域\n"
            "- 设置默认访问密码\n"
            "- 写入保存模式 (开启后写入时自动更新已保存标签)\n"
            "- 保存配置 (未来)\n"
            "- 设置/检测标签类型 (未来)\n"
            "- 切换天线 (仅M6E/M7E, 即将推出)\n\n"));

    //Read screen information
    furi_string_cat_printf(TmpString, "\e#%s\n", RFID_READER_UI_TEXT("Read:", "读取:"));
    furi_string_cat_printf(
        TmpString,
        RFID_READER_UI_TEXT(
            " The read menu has the following options:\n"
            "- Press Ok to start/stop reading\n"
            "- Press Up to save the selected EPC\n"
            "- Hold Up to pause scrolling the selected EPC\n"
            "- Press Down to see TID, EPC, User, and Reserved Memory\n"
            "- Press Left/Right to cycle through tags read (M6E & M7E Only)\n\n",
            " 读取菜单:\n"
            "- 按 OK 开始/停止读取\n"
            "- 按上键保存当前EPC\n"
            "- 长按上键暂停滚动\n"
            "- 按下键查看TID、EPC、用户区和保留区\n"
            "- 按左右键切换标签 (仅M6E/M7E)\n\n"));

    //Write screen information
    furi_string_cat_printf(TmpString, "\e#%s\n", RFID_READER_UI_TEXT("Write:", "写入:"));
    furi_string_cat_printf(
        TmpString,
        RFID_READER_UI_TEXT(
            " The write menu has the following options:\n"
            "- Press Ok to start/stop writing\n"
            "- Press Left to modify the EPC value\n"
            "- Press Right to modify Reserved Memory (First 4 bytes = kill password, last 4 bytes = access password)\n"
            "- Press Up to modify the User Memory Bank\n"
            "- Press Down to modify the TID (Supported but usually locked by manufacturer)\n\n",
            " 写入菜单:\n"
            "- 按 OK 开始/停止写入\n"
            "- 按左键修改EPC\n"
            "- 按右键修改保留区 (前4字节=销毁密码, 后4字节=访问密码)\n"
            "- 按上键修改用户区\n"
            "- 按下键修改TID (通常被厂商锁定)\n\n"));

    furi_string_cat_printf(TmpString, "\e#%s\n", RFID_READER_UI_TEXT("Lock:", "锁定:"));
    furi_string_cat_printf(
        TmpString,
        RFID_READER_UI_TEXT(
            " The Lock menu has the following options:\n"
            "- Set access password (Can alternatively be set through the write menu)\n"
            "- Pick the Memory Bank or Password to Lock/Unlock, or Permanently Lock/Unlock (In the Open or Secured State)\n"
            "- Select the Lock Mode\n"
            "- Press Execute to perform the lock action\n"
            "- To perform a lock action, first ensure the AP has been set in the config menu, and then select a lock action\n"
            "- If the password is wrong, an error sequence will beep\n\n",
            " 锁定菜单:\n"
            "- 设置访问密码 (也可在写入菜单中设置)\n"
            "- 选择要锁定/解锁的存储区或密码\n"
            "- 选择锁定模式\n"
            "- 按执行按钮执行锁定操作\n"
            "- 执行前请先在配置菜单中设置 AP\n"
            "- 密码错误时会发出错误提示音\n\n"));
    furi_string_cat_printf(TmpString, "\e#%s\n", RFID_READER_UI_TEXT("Kill:", "销毁:"));
    furi_string_cat_printf(
        TmpString,
        RFID_READER_UI_TEXT(
            " The Kill menu has the following options:\n"
            "- Set kill password (Can alternatively be set through the write menu)\n"
            "- Press Kill Tag and confirm the kill password to permanently inactivate the tag!\n"
            "- If the password is wrong, an error sequence will beep\n\n",
            " 销毁菜单:\n"
            "- 设置销毁密码 (也可在写入菜单中设置)\n"
            "- 按销毁标签并确认密码以永久禁用标签!\n"
            "- 密码错误时会发出错误提示音\n\n"));
    
    //Adding the widget to the view dispatcher 
    widget_add_text_scroll_element(
        App->WidgetAbout, 0, 16, 128, 50, furi_string_get_cstr(TmpString));
    furi_string_free(TmpString);
    view_set_previous_callback(
        widget_get_view(App->WidgetAbout), uhf_reader_navigation_about_submenu_callback);
    view_dispatcher_add_view(
        App->ViewDispatcher, UHFReaderViewAbout, widget_get_view(App->WidgetAbout));
}

/**
 * @brief      Frees the about view
 * @details    Frees all variables associated with the about widget.
 * @param      app  The UHFReaderApp - used to free the widget.
*/
void view_about_free(UHFReaderApp* App) {
    view_dispatcher_remove_view(App->ViewDispatcher, UHFReaderViewAbout);
    widget_free(App->WidgetAbout);
}
