#include "../spi_mem_app_i.h"
#include "../lib/spi/spi_mem_chip.h"

#define SPI_MEM_VERSION_APP FAP_VERSION
#define SPI_MEM_DEVELOPER   "DrunkBatya"
#define SPI_MEM_GITHUB      "https://github.com/flipperdevices/flipperzero-good-faps"
#define SPI_MEM_NAME        "\e#\e!       SPI Mem Manager        \e!\n"
#define SPI_MEM_BLANK_INV   "\e#\e!                                                      \e!\n"

void spi_mem_scene_about_on_enter(void* context) {
    SPIMemApp* app = context;
    FuriString* tmp_string = furi_string_alloc();

    widget_add_text_box_element(
        app->widget, 0, 0, 128, 14, AlignCenter, AlignBottom, SPI_MEM_BLANK_INV, false);
    widget_add_text_box_element(
        app->widget, 0, 2, 128, 14, AlignCenter, AlignBottom, SPI_MEM_NAME, false);
    furi_string_printf(tmp_string, "\e#%s\n", SPI_MEM_UI_TEXT("Information", "信息"));
    furi_string_cat_printf(tmp_string, "%s %s\n", SPI_MEM_UI_TEXT("Version:", "版本:"), SPI_MEM_VERSION_APP);
    furi_string_cat_printf(tmp_string, "%s %s\n", SPI_MEM_UI_TEXT("Developed by:", "开发者:"), SPI_MEM_DEVELOPER);
    furi_string_cat_printf(tmp_string, "Github: %s\n\n", SPI_MEM_GITHUB);
    furi_string_cat_printf(tmp_string, "\e#%s\n", SPI_MEM_UI_TEXT("Description", "说明"));
    furi_string_cat_printf(
        tmp_string,
        SPI_MEM_UI_TEXT(
            "SPI memory dumper\n"
            "Originally written by Hedger, ghettorce and x893 at\n"
            "Flipper Hackathon 2021\n\n",
            "SPI 存储器读写工具\n"
            "原作者 Hedger, ghettorce 和 x893\n"
            "于 Flipper Hackathon 2021 编写\n\n"));
    widget_add_text_scroll_element(app->widget, 0, 16, 128, 50, furi_string_get_cstr(tmp_string));

    furi_string_free(tmp_string);
    view_dispatcher_switch_to_view(app->view_dispatcher, SPIMemViewWidget);
}

bool spi_mem_scene_about_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}
void spi_mem_scene_about_on_exit(void* context) {
    SPIMemApp* app = context;
    widget_reset(app->widget);
}
