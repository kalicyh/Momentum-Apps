#pragma once

#include "eth_worker.h"
#include "eth_save_process.h"
#include <gui/gui.h>

#define SCREEN_SYMBOLS_WIDTH 30

EthViewProcess* ethernet_view_process_malloc(EthWorkerProcess type, EthernetSaveConfig* config);
void ethernet_view_process_free(EthViewProcess* evp);

void ethernet_view_process_draw(EthViewProcess* process, Canvas* canvas);
void ethernet_view_process_keyevent(EthViewProcess* process, InputKey key);
void ethernet_view_process_print(EthViewProcess* process, const char* str);
void ethernet_view_process_move(EthViewProcess* process, int8_t shift);
void evp_printf(EthViewProcess* process, const char* format, ...);

typedef struct u8x8_struct u8x8_t;
typedef struct u8x8_display_info_struct u8x8_display_info_t;
typedef struct u8x8_tile_struct u8x8_tile_t;

typedef uint8_t (*u8x8_msg_cb)(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
typedef uint16_t (*u8x8_char_cb)(u8x8_t* u8x8, uint8_t b);

struct u8x8_struct {
    const u8x8_display_info_t* display_info;
    u8x8_char_cb next_cb; /*  procedure, which will be used to get the next char from the string */
    u8x8_msg_cb display_cb;
    u8x8_msg_cb cad_cb;
    u8x8_msg_cb byte_cb;
    u8x8_msg_cb gpio_and_delay_cb;
    uint32_t bus_clock; /* can be used by the byte function to store the clock speed of the bus */
    const uint8_t* font;
    uint16_t encoding; /* encoding result for utf8 decoder in next_cb */
    uint8_t x_offset; /* copied from info struct, can be modified in flip mode */
    uint8_t is_font_inverse_mode; /* 0: normal, 1: font glyphs are inverted */
    uint8_t
        i2c_address; /* a valid i2c adr. Initially this is 255, but this is set to something useful during DISPLAY_INIT */
    /* i2c_address is the address for writing data to the display */
    /* usually, the lowest bit must be zero for a valid address */
    uint8_t i2c_started; /* for i2c interface */
    //uint8_t device_address;	/* OBSOLETE???? - this is the device address, replacement for U8X8_MSG_CAD_SET_DEVICE */
    uint8_t utf8_state; /* number of chars which are still to scan */
    uint8_t gpio_result; /* return value from the gpio call (only for MENU keys at the moment) */
    uint8_t debounce_default_pin_state;
    uint8_t debounce_last_pin_state;
    uint8_t debounce_state;
    uint8_t debounce_result_msg; /* result msg or event after debounce */
#ifdef U8X8_WITH_USER_PTR
    void* user_ptr;
#endif
#ifdef U8X8_USE_PINS
    uint8_t pins
        [U8X8_PIN_CNT]; /* defines a pinlist: Mainly a list of pins for the Arduino Environment, use U8X8_PIN_xxx to access */
#endif
};

typedef uint8_t u8g2_uint_t; /* for pixel position only */
typedef int8_t u8g2_int_t; /* introduced for circle calculation */
typedef int16_t u8g2_long_t; /* introduced for ellipse calculation */

typedef struct u8g2_struct u8g2_t;
typedef struct u8g2_cb_struct u8g2_cb_t;

typedef void (*u8g2_update_dimension_cb)(u8g2_t* u8g2);
typedef void (*u8g2_update_page_win_cb)(u8g2_t* u8g2);
typedef void (
    *u8g2_draw_l90_cb)(u8g2_t* u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir);
typedef void (*u8g2_draw_ll_hvline_cb)(
    u8g2_t* u8g2,
    u8g2_uint_t x,
    u8g2_uint_t y,
    u8g2_uint_t len,
    uint8_t dir);

typedef uint8_t (*u8g2_get_kerning_cb)(u8g2_t* u8g2, uint16_t e1, uint16_t e2);

/* from ucglib... */
struct _u8g2_font_info_t {
    /* offset 0 */
    uint8_t glyph_cnt;
    uint8_t bbx_mode;
    uint8_t bits_per_0;
    uint8_t bits_per_1;

    /* offset 4 */
    uint8_t bits_per_char_width;
    uint8_t bits_per_char_height;
    uint8_t bits_per_char_x;
    uint8_t bits_per_char_y;
    uint8_t bits_per_delta_x;

    /* offset 9 */
    int8_t max_char_width;
    int8_t
        max_char_height; /* overall height, NOT ascent. Instead ascent = max_char_height + y_offset */
    int8_t x_offset;
    int8_t y_offset;

    /* offset 13 */
    int8_t ascent_A;
    int8_t descent_g; /* usually a negative value */
    int8_t ascent_para;
    int8_t descent_para;

    /* offset 17 */
    uint16_t start_pos_upper_A;
    uint16_t start_pos_lower_a;

    /* offset 21 */
#ifdef U8G2_WITH_UNICODE
    uint16_t start_pos_unicode;
#endif
};
typedef struct _u8g2_font_info_t u8g2_font_info_t;

/* from ucglib... */
struct _u8g2_font_decode_t {
    const uint8_t* decode_ptr; /* pointer to the compressed data */

    u8g2_uint_t target_x;
    u8g2_uint_t target_y;

    int8_t x; /* local coordinates, (0,0) is upper left */
    int8_t y;
    int8_t glyph_width;
    int8_t glyph_height;

    uint8_t decode_bit_pos; /* bitpos inside a byte of the compressed data */
    uint8_t is_transparent;
    uint8_t fg_color;
    uint8_t bg_color;
#ifdef U8G2_WITH_FONT_ROTATION
    uint8_t dir; /* direction */
#endif
};
typedef struct _u8g2_font_decode_t u8g2_font_decode_t;

struct _u8g2_kerning_t {
    uint16_t first_table_cnt;
    uint16_t second_table_cnt;
    const uint16_t* first_encoding_table;
    const uint16_t* index_to_second_table;
    const uint16_t* second_encoding_table;
    const uint8_t* kerning_values;
};
typedef struct _u8g2_kerning_t u8g2_kerning_t;

struct u8g2_cb_struct {
    u8g2_update_dimension_cb update_dimension;
    u8g2_update_page_win_cb update_page_win;
    u8g2_draw_l90_cb draw_l90;
};

typedef u8g2_uint_t (*u8g2_font_calc_vref_fnptr)(u8g2_t* u8g2);

struct u8g2_struct {
    u8x8_t u8x8;
    u8g2_draw_ll_hvline_cb ll_hvline; /* low level hvline procedure */
    const u8g2_cb_t* cb; /* callback drawprocedures, can be replaced for rotation */

    /* the following variables must be assigned during u8g2 setup */
    uint8_t*
        tile_buf_ptr; /* ptr to memory area with u8x8.display_info->tile_width * 8 * tile_buf_height bytes */
    uint8_t tile_buf_height; /* height of the tile memory area in tile rows */
    uint8_t tile_curr_row; /* current row for picture loop */

    /* dimension of the buffer in pixel */
    u8g2_uint_t pixel_buf_width; /* equal to tile_buf_width*8 */
    u8g2_uint_t pixel_buf_height; /* tile_buf_height*8 */
    u8g2_uint_t pixel_curr_row; /* u8g2.tile_curr_row*8 */

    /* the following variables are set by the update dimension callback */
    /* this is the clipbox after rotation for the hvline procedures */
    //u8g2_uint_t buf_x0;	/* left corner of the buffer */
    //u8g2_uint_t buf_x1;	/* right corner of the buffer (excluded) */
    u8g2_uint_t buf_y0;
    u8g2_uint_t buf_y1;

    /* display dimensions in pixel for the user, calculated in u8g2_update_dimension_common()  */
    u8g2_uint_t width;
    u8g2_uint_t height;

    /* this is the clip box for the user to check if a specific box has an intersection */
    /* use u8g2_IsIntersection from u8g2_intersection.c to test against this intersection */
    /* actually, this window describes the position of the current page */
    u8g2_uint_t user_x0; /* left corner of the buffer */
    u8g2_uint_t user_x1; /* right corner of the buffer (excluded) */
    u8g2_uint_t user_y0; /* upper edge of the buffer */
    u8g2_uint_t user_y1; /* lower edge of the buffer (excluded) */

#ifdef U8G2_WITH_CLIP_WINDOW_SUPPORT
    /* clip window */
    u8g2_uint_t clip_x0; /* left corner of the clip window */
    u8g2_uint_t clip_x1; /* right corner of the clip window (excluded) */
    u8g2_uint_t clip_y0; /* upper edge of the clip window */
    u8g2_uint_t clip_y1; /* lower edge of the clip window (excluded) */
#endif /* U8G2_WITH_CLIP_WINDOW_SUPPORT */

    /* information about the current font */
    const uint8_t* font; /* current font for all text procedures */
    // removed: const u8g2_kerning_t *kerning;		/* can be NULL */
    // removed: u8g2_get_kerning_cb get_kerning_cb;

    u8g2_font_calc_vref_fnptr font_calc_vref;
    u8g2_font_decode_t font_decode; /* new font decode structure */
    u8g2_font_info_t font_info; /* new font info structure */

#ifdef U8G2_WITH_CLIP_WINDOW_SUPPORT
    /* 1 of there is an intersection between user_?? and clip_?? box */
    uint8_t is_page_clip_window_intersection;
#endif /* U8G2_WITH_CLIP_WINDOW_SUPPORT */

    uint8_t font_height_mode;
    int8_t font_ref_ascent;
    int8_t font_ref_descent;

    int8_t glyph_x_offset; /* set by u8g2_GetGlyphWidth as a side effect */

    uint8_t bitmap_transparency; /* black pixels will be treated as transparent (not drawn) */

    uint8_t draw_color; /* 0: clear pixel, 1: set pixel, modified and restored by font procedures */
    /* draw_color can be used also directly by the user API */

    // the following variable should be renamed to is_buffer_auto_clear
    uint8_t
        is_auto_page_clear; /* set to 0 to disable automatic clear of the buffer in firstPage() and nextPage() */
};

typedef struct EthViewProcessLine {
    char data[SCREEN_SYMBOLS_WIDTH];
} EthViewProcessLine;

struct EthViewProcess {
    EthViewProcessLine* fifo;
    EthWorkerProcess type;
    uint8_t strings_cnt;
    uint8_t carriage;
    uint8_t position;
    uint8_t autofill;
    uint8_t editing;
    uint8_t x;
    uint8_t y;
    void* draw_struct;
};

typedef struct EthViewDrawInit {
    uint8_t* mac;
    uint8_t current_octet;
} EthViewDrawInit;

typedef enum {
    EthViewDrawStaticModeIp,
    EthViewDrawStaticModeMask,
    EthViewDrawStaticModeGateway,
    EthViewDrawStaticModeDNS
} EthViewDrawStaticMode;
typedef struct EthViewDrawStatic {
    EthViewDrawStaticMode current_mode;
    uint8_t* ip;
    uint8_t* mask;
    uint8_t* gateway;
    uint8_t* dns;
    uint8_t current_digit;
    uint8_t editing;
} EthViewDrawStatic;

typedef struct EthViewDrawPing {
    uint8_t current_digit;
    uint8_t* ip;
} EthViewDrawPing;