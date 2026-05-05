#include "scenes.h"
#include "../flipper_spi_terminal.h"

const char* about_text =
    SPI_TERMINAL_UI_TEXT(
        "SPI TERMINAL\n"
        "\n"
        "By JAN WIESEMANN.de\n"
        "https://github.com/janwiesemann/flipper-spi-terminal\n"
        "\n"
        "SPI TERMINAL is a SPI App, which allows you to control external devices using SPI. Your Flipper can act as a SPI Master or Slave device. The Slave mode allows you to sniff the communication between different SPI peripherals.\n"
        "\n"
        "The App uses the Low-Level SPI Interface of the STM32WB55RG Microprocessor. All data is transmitted with DMA Sub-module and can reach speeds of up to 32 Mbit/s in Master and up to 24 Mbit/s in Slave mode.\n"
        "\n"
        "The inbuild documentation is based on the 'STM32F7 - SPI' (Revision 1.0) presentation and the STM 'RM0434' Reference Manual. Both referenced are linked to in the GitHub README.MD."
        "\n"
        "=== Pin configuration ===\n"
        "CAUTION: Flipper Zero's pins are only 5V tolerant, if they are configured as an input. Due to this limitation, only 3.3V signals should be used!\n"
        "\n"
        "MOSI: 2 (PA7) Master In / Slave Out\n"
        "MISO: 3 (PA6) Master Out / Salve In\n"
        "  CS: 4 (PA4, NSS) Non Slave Select\n"
        " SCK: 5 (PB3) Serial Clock\n"
        "\n"
        "=== Terminal View ===\n"
        "Use the 'Up' and 'Down' keys to scroll.\n"
        "Press 'Back' to navigate to the main menu.\n"
        "Hold 'Back' to clear the screen buffer.",
        "SPI 终端\n"
        "\n"
        "作者: JAN WIESEMANN.de\n"
        "https://github.com/janwiesemann/flipper-spi-terminal\n"
        "\n"
        "SPI 终端是一款 SPI 应用，允许你通过 SPI 控制外部设备。你的 Flipper 可以作为 SPI 主机或从机设备。从机模式可以嗅探不同 SPI 外设之间的通信。\n"
        "\n"
        "本应用使用 STM32WB55RG 微处理器的底层 SPI 接口。所有数据通过 DMA 子模块传输，主机模式最高速度可达 32 Mbit/s，从机模式可达 24 Mbit/s。\n"
        "\n"
        "内置文档基于 'STM32F7 - SPI'（修订版 1.0）演示文稿和 STM 'RM0434' 参考手册。相关链接见 GitHub README.MD。"
        "\n"
        "=== 引脚配置 ===\n"
        "注意：Flipper Zero 的引脚仅在配置为输入时支持 5V。因此，应仅使用 3.3V 信号！\n"
        "\n"
        "MOSI: 2 (PA7) 主机输入/从机输出\n"
        "MISO: 3 (PA6) 主机输出/从机输入\n"
        "  CS: 4 (PA4, NSS) 片选\n"
        " SCK: 5 (PB3) 时钟\n"
        "\n"
        "=== 终端视图 ===\n"
        "使用上/下键滚动。\n"
        "按返回键导航到主菜单。\n"
        "长按返回键清除屏幕缓冲区。");

void flipper_spi_terminal_scene_about_alloc(FlipperSPITerminalApp* app) {
    app->about_screen = text_box_alloc();

    text_box_set_text(app->about_screen, about_text);

    view_dispatcher_add_view(
        app->view_dispatcher,
        FlipperSPITerminalAppSceneAbout,
        text_box_get_view(app->about_screen));
}

void flipper_spi_terminal_scene_about_free(FlipperSPITerminalApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, FlipperSPITerminalAppSceneAbout);

    text_box_free(app->about_screen);
}

void flipper_spi_terminal_scene_about_on_enter(void* context) {
    SPI_TERM_CONTEXT_TO_APP(context);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipperSPITerminalAppSceneAbout);
}

bool flipper_spi_terminal_scene_about_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);

    return false;
}

void flipper_spi_terminal_scene_about_on_exit(void* context) {
    UNUSED(context);
}
