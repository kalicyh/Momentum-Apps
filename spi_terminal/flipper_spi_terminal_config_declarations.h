#ifndef SPI_CONFIG_ADDED_INCLUDES
#include <furi_hal_spi_types.h>
#include "toolbox/value_index_ex.h"
#define SPI_CONFIG_ADDED_INCLUDES
#endif

#ifndef UNWRAP_ARGS
// Removes brackets from a list of values
#define UNWRAP_ARGS(...) __VA_ARGS__
#endif

#ifndef ADD_CONFIG_ENTRY
// This is just a dummy function to enable some fancy syntax highlighting and autocomplete.
// It will only be used at edit time.
#define ADD_CONFIG_ENTRY(                                                                           \
    label, helpText, name, type, defaultValue, valueIndexFunc, field, valuesCount, values, strings) \
    void(dummy_config_##name##_preview_func)() {                                                    \
        const char* labelstr = label;                                                               \
        UNUSED(labelstr);                                                                           \
        const char* helpstr = helpText;                                                             \
        UNUSED(helpstr);                                                                            \
        const type def = defaultValue;                                                              \
        const type vals[valuesCount] = {UNWRAP_ARGS values};                                        \
        size_t index = (valueIndexFunc)(def, vals, valuesCount);                                    \
        UNUSED(index);                                                                              \
        const char* const strs[valuesCount] = {UNWRAP_ARGS strings};                                \
        UNUSED(strs);                                                                               \
    }
#endif

// Non-parenthesized version for string literal concatenation contexts
#ifndef SPI_TERMINAL_STR
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define SPI_TERMINAL_STR(en, zh) zh
#else
#define SPI_TERMINAL_STR(en, zh) en
#endif
#endif

#ifndef FORMAT_VALUE_DESCRIPTION
#define FORMAT_VALUE_DESCRIPTION(header, text) "=== " header " ===\n" text "\n"
#endif

#ifndef SPI_TERMINAL_DEFAULT_PREFIX
#ifdef MOMENTUM_UI_LANG_ZH_CN
#define SPI_TERMINAL_DEFAULT_PREFIX "默认: "
#else
#define SPI_TERMINAL_DEFAULT_PREFIX "Default: "
#endif
#endif

#ifndef FORMAT_DESCRIPTION_MIN
#define FORMAT_DESCRIPTION_MIN(general_info, default_value_str) \
    general_info "\n\n" SPI_TERMINAL_DEFAULT_PREFIX default_value_str
#endif

#ifndef FORMAT_DESCRIPTION
#define FORMAT_DESCRIPTION(general_info, default_value_str, value_descriptions) \
    general_info "\n\n" UNWRAP_ARGS value_descriptions "\n" SPI_TERMINAL_DEFAULT_PREFIX default_value_str
#endif

ADD_CONFIG_ENTRY(
    SPI_TERMINAL_STR("Display mode", "显示模式"),
    FORMAT_DESCRIPTION(
        SPI_TERMINAL_STR("Changes, how the received data is displayed.", "更改接收数据的显示方式。"),
        "Auto",
        (FORMAT_VALUE_DESCRIPTION("Auto", SPI_TERMINAL_STR("Use ASCII, C escape sequence or Hex", "使用 ASCII、C 转义序列或十六进制"))
             FORMAT_VALUE_DESCRIPTION(
                 "Text",
                 SPI_TERMINAL_STR("Use ASCII for everything (Non printable chars are replaced by a space ' ')", "全部使用 ASCII（不可打印字符替换为空格）"))
                 FORMAT_VALUE_DESCRIPTION("Hex", SPI_TERMINAL_STR("Use Hex for everything", "全部使用十六进制"))
                     FORMAT_VALUE_DESCRIPTION("Binary", SPI_TERMINAL_STR("Use binary for everything", "全部使用二进制")))),
    display_mode,
    TerminalDisplayMode,
    TerminalDisplayModeAuto,
    value_index_display_mode,
    display_mode,
    4,
    (TerminalDisplayModeAuto,
     TerminalDisplayModeText,
     TerminalDisplayModeHex,
     TerminalDisplayModeBinary),
    ("Auto", "Text", "Hex", "Binary"))

ADD_CONFIG_ENTRY(
    SPI_TERMINAL_STR("Terminal Buffer behaviour", "终端缓冲区行为"),
    FORMAT_DESCRIPTION(
        SPI_TERMINAL_STR("Configures, how the Terminal Screen buffer behaves, if you leave/enter the Terminal View.", "配置离开/进入终端视图时缓冲区的行为。"),
        SPI_TERMINAL_STR("Clear", "清除"),
        (FORMAT_VALUE_DESCRIPTION("Clear", SPI_TERMINAL_STR("Terminal Buffer will be cleared", "终端缓冲区将被清除"))
             FORMAT_VALUE_DESCRIPTION("Keep", SPI_TERMINAL_STR("The Buffer will keep it's content", "缓冲区将保留其内容")))),
    terminal_buffer_behaviour,
    TerminalBufferBehaviour,
    TerminalBufferBehaviourClear,
    value_index_buffer_behaviour,
    terminal_buffer_behaviour,
    2,
    (TerminalBufferBehaviourClear, TerminalBufferBehaviourKeep),
    ("Clear", "Keep"))

ADD_CONFIG_ENTRY(
    SPI_TERMINAL_STR("DMA RX Buffer size", "DMA 接收缓冲区大小"),
    FORMAT_DESCRIPTION(
        SPI_TERMINAL_STR("Sets the buffer size for a SPI read request. A higher value results in a less frequent update. A higher value will result in a faster data rate."
        "SPI Terminal utilizes the inbuild DMA controller for a event/interrupt based receive and transmit system. This removes the overhead of a polling based system.",
        "设置 SPI 读取请求的缓冲区大小。较大值更新频率较低，但数据速率更快。"
        "SPI 终端使用内置 DMA 控制器进行基于事件/中断的收发，避免了轮询开销。"),
        "1",
        ("1-256 byte")),
    rx_dma_buffer_size,
    size_t,
    1,
    value_index_size_t,
    rx_dma_buffer_size,
    9,
    (1, 2, 4, 8, 16, 32, 64, 128, 256),
    ("1", "2", "4", "8", "16", "32", "64", "128", "256"))

ADD_CONFIG_ENTRY(
    SPI_TERMINAL_STR("Mode", "模式"),
    FORMAT_DESCRIPTION(
        SPI_TERMINAL_STR("Configures the SPI-Mode.", "配置 SPI 模式。"),
        SPI_TERMINAL_STR("Slave", "从机"),
        (FORMAT_VALUE_DESCRIPTION("Master", SPI_TERMINAL_STR("Flipper is controlling the SPI clock", "Flipper 控制 SPI 时钟"))
             FORMAT_VALUE_DESCRIPTION("Slave", SPI_TERMINAL_STR("A external device is controlling the SPI clock", "外部设备控制 SPI 时钟")))),
    spi_mode,
    uint32_t,
    LL_SPI_MODE_SLAVE,
    value_index_uint32,
    spi.Mode,
    2,
    (LL_SPI_MODE_MASTER, LL_SPI_MODE_SLAVE),
    ("Master", "Slave"))

ADD_CONFIG_ENTRY(
    SPI_TERMINAL_STR("Direction", "方向"),
    FORMAT_DESCRIPTION(
        SPI_TERMINAL_STR("Configures the transfer direction.", "配置传输方向。"),
        SPI_TERMINAL_STR("Full Duplex", "全双工"),
        (FORMAT_VALUE_DESCRIPTION(
            "Full Duplex",
            SPI_TERMINAL_STR("Data is transmitted in both directions at the same time (RX and TX). Both data lines are used simultaneously.", "数据同时双向传输（收发），两条数据线同时使用。"))
             FORMAT_VALUE_DESCRIPTION(
                 "Simplex RX",
                 SPI_TERMINAL_STR("Only receive data. Can be used to 'sniff' the communication between devices.", "仅接收数据。可用于嗅探设备间的通信。"))
                 FORMAT_VALUE_DESCRIPTION(
                     "Half Duplex RX/TX",
                     SPI_TERMINAL_STR("Only send data. In this mode, RX and TX modes are altered after every transfer. This alows the use of only one data line", "仅发送数据。收发模式每次传输后交替，允许仅使用一条数据线")))),
    spi_direction,
    uint32_t,
    LL_SPI_FULL_DUPLEX,
    value_index_uint32,
    spi.TransferDirection,
    4,
    (LL_SPI_FULL_DUPLEX, LL_SPI_SIMPLEX_RX, LL_SPI_HALF_DUPLEX_RX, LL_SPI_HALF_DUPLEX_TX),
    ("Full Duplex", "Simplex RX", "Half Duplex RX", "Half Duplex TX"))

ADD_CONFIG_ENTRY(
    SPI_TERMINAL_STR("Data Width", "数据宽度"),
    FORMAT_DESCRIPTION_MIN(
        SPI_TERMINAL_STR("Sets the data width of a single transfer.", "设置单次传输的数据宽度。"),
        "8 bit"),
    spi_data_width,
    uint32_t,
    LL_SPI_DATAWIDTH_8BIT,
    value_index_uint32,
    spi.DataWidth,
    13,
    (LL_SPI_DATAWIDTH_4BIT,
     LL_SPI_DATAWIDTH_5BIT,
     LL_SPI_DATAWIDTH_6BIT,
     LL_SPI_DATAWIDTH_7BIT,
     LL_SPI_DATAWIDTH_8BIT,
     LL_SPI_DATAWIDTH_9BIT,
     LL_SPI_DATAWIDTH_10BIT,
     LL_SPI_DATAWIDTH_11BIT,
     LL_SPI_DATAWIDTH_12BIT,
     LL_SPI_DATAWIDTH_13BIT,
     LL_SPI_DATAWIDTH_14BIT,
     LL_SPI_DATAWIDTH_15BIT,
     LL_SPI_DATAWIDTH_16BIT),
    ("4 bit",
     "5 bit",
     "6 bit",
     "7 bit",
     "8 bit",
     "9 bit",
     "10 bit",
     "11 bit",
     "12 bit",
     "13 bit",
     "14 bit",
     "15 bit",
     "16 bit"))

ADD_CONFIG_ENTRY(
    SPI_TERMINAL_STR("Clock polarity", "时钟极性"),
    FORMAT_DESCRIPTION(
        SPI_TERMINAL_STR("Configures the polarity of the SPI clock.", "配置 SPI 时钟的极性。"),
        SPI_TERMINAL_STR("High", "高"),
        (FORMAT_VALUE_DESCRIPTION("Low", SPI_TERMINAL_STR("Clock line is 0 (low, off) on idle", "空闲时时钟线为 0（低/关）"))
             FORMAT_VALUE_DESCRIPTION("High", SPI_TERMINAL_STR("Clock line is 1 (high, on) on idle", "空闲时时钟线为 1（高/开）")))),
    spi_clock_polarity,
    uint32_t,
    LL_SPI_POLARITY_LOW,
    value_index_uint32,
    spi.ClockPolarity,
    2,
    (LL_SPI_POLARITY_LOW, LL_SPI_POLARITY_HIGH),
    ("Low", "Heigh"))

ADD_CONFIG_ENTRY(
    SPI_TERMINAL_STR("Clock Phase", "时钟相位"),
    FORMAT_DESCRIPTION(
        SPI_TERMINAL_STR("Defines, on which clock edge the data is sampled.", "定义在哪个时钟沿采样数据。"),
        "1-Edge",
        (FORMAT_VALUE_DESCRIPTION(
            "1-Edge",
            SPI_TERMINAL_STR("Trigger on the first clock polarity change (CP: High, CLK: ...1 1010 1010 1... => Trigger on the transition from 1 to 0)", "在第一个时钟极性变化时触发"))
             FORMAT_VALUE_DESCRIPTION(
                 "2-Edge",
                 SPI_TERMINAL_STR("Trigger on the second clock polarity change (CP: High, CLK: ...1 1010 1010 1... => Trigger on the transition from 0 to 1)", "在第二个时钟极性变化时触发")))),
    spi_clock_phase,
    uint32_t,
    LL_SPI_PHASE_1EDGE,
    value_index_uint32,
    spi.ClockPhase,
    2,
    (LL_SPI_PHASE_1EDGE, LL_SPI_PHASE_2EDGE),
    ("1-Edge", "2-Edge"))

ADD_CONFIG_ENTRY(
    SPI_TERMINAL_STR("Non Slave Select", "片选(NSS)"),
    FORMAT_DESCRIPTION(
        SPI_TERMINAL_STR("Configures the slave select behavior on the CS pin", "配置 CS 引脚的片选行为"),
        "Soft",
        (FORMAT_VALUE_DESCRIPTION(
            "Soft",
            SPI_TERMINAL_STR("NSS managed by the software. In this mode, Flipper SPI Terminal is not using any Slave Select mechanisms and is always sending or receiving data.", "由软件管理 NSS。此模式下不使用片选机制，始终收发数据。"))
             FORMAT_VALUE_DESCRIPTION(
                 "Hard Input",
                 SPI_TERMINAL_STR("If Flipper Zero is acting as a Slave, CS-Pin is used as standard chip select. If Flipper Zero is configured as Master, this pin can be used for a 'Multi Master' configuration.", "从机模式下 CS 引脚用作标准片选。主机模式下可用于多主机配置。"))
                 FORMAT_VALUE_DESCRIPTION(
                     "Hard Output",
                     SPI_TERMINAL_STR(" (Only used in Master-Mode) CS is driven low as soon as the Terminal Screen is entered. Due to a Flipper Firmware limitation, the pin might send a pulse to the other devices.", "（仅主机模式）进入终端界面时 CS 拉低。由于固件限制，引脚可能发送脉冲。")))),
    spi_nss,
    uint32_t,
    LL_SPI_NSS_SOFT,
    value_index_uint32,
    spi.NSS,
    3,
    (LL_SPI_NSS_SOFT, LL_SPI_NSS_HARD_INPUT, LL_SPI_NSS_HARD_OUTPUT),
    ("Soft", "Hard Input", "Hard Output"))

ADD_CONFIG_ENTRY(
    SPI_TERMINAL_STR("Baudrate prescaler", "波特率预分频"),
    FORMAT_DESCRIPTION_MIN(
        SPI_TERMINAL_STR("Sets, by how many cycles, the SPI clock is divided. Flippers SPI clock is running at 32MHz. A prescaler of Div 2 will result int a baudrate of around 16 Mbit.", "设置 SPI 时钟分频系数。Flipper SPI 时钟为 32MHz，2 分频约 16Mbit 波特率。"),
        "Div 32"),
    spi_baud_rate,
    uint32_t,
    LL_SPI_BAUDRATEPRESCALER_DIV32,
    value_index_uint32,
    spi.BaudRate,
    8,
    (LL_SPI_BAUDRATEPRESCALER_DIV2,
     LL_SPI_BAUDRATEPRESCALER_DIV4,
     LL_SPI_BAUDRATEPRESCALER_DIV8,
     LL_SPI_BAUDRATEPRESCALER_DIV16,
     LL_SPI_BAUDRATEPRESCALER_DIV32,
     LL_SPI_BAUDRATEPRESCALER_DIV64,
     LL_SPI_BAUDRATEPRESCALER_DIV128,
     LL_SPI_BAUDRATEPRESCALER_DIV256),
    ("Div 2", "Div 4", "Div 8", "Div 16", "Div 32", "Div 64", "Div 128", "Div 256"))

ADD_CONFIG_ENTRY(
    SPI_TERMINAL_STR("Bit-order", "位序"),
    FORMAT_DESCRIPTION(
        SPI_TERMINAL_STR("Sets, in which order bytes are received.", "设置字节接收顺序。"),
        "MSB",
        (SPI_TERMINAL_STR("Most significant bit or Least significant bit", "最高有效位或最低有效位"))),
    spi_bit_order,
    uint32_t,
    LL_SPI_MSB_FIRST,
    value_index_uint32,
    spi.BitOrder,
    2,
    (LL_SPI_MSB_FIRST, LL_SPI_LSB_FIRST),
    ("MSB", "LSB"))

ADD_CONFIG_ENTRY(
    "CRC",
    FORMAT_DESCRIPTION_MIN(SPI_TERMINAL_STR("Enables or disables the CRC calculation.", "启用或禁用 CRC 计算。"), SPI_TERMINAL_STR("Disabled", "禁用")),
    spi_crc_calculation,
    uint32_t,
    LL_SPI_CRCCALCULATION_DISABLE,
    value_index_uint32,
    spi.CRCCalculation,
    2,
    (LL_SPI_CRCCALCULATION_DISABLE, LL_SPI_CRCCALCULATION_ENABLE),
    ("Disabled", "Enabled"))

ADD_CONFIG_ENTRY(
    SPI_TERMINAL_STR("CRC Poly", "CRC 多项式"),
    FORMAT_DESCRIPTION_MIN(SPI_TERMINAL_STR("Sets the CRC Polynomial value.", "设置 CRC 多项式值。"), "/"),
    spi_crc_poly,
    uint32_t,
    7,
    value_index_uint32,
    spi.CRCPoly,
    10,
    (2, 3, 4, 5, 6, 7, 8, 9, 10, 11),
    ("2", "3", "4", "5", "6", "7", "8", "9", "10", "11"))
