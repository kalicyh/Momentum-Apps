#pragma once

#include "rolling_flaws_structs.h"

#define ROLLING_FLAWS_ABOUT_TEXT                                                \
    ROLLING_FLAWS_UI_TEXT(                                                       \
        "Rolling code receiver\n version 1.5\n"                                 \
        "---\n"                                                                 \
        "Practice rolling code attacks without risking a desync!\n"             \
        "This app is for educational\n"                                         \
        "purposes only.\n---\n"                                                 \
        "Protocol KeeLoq (DoorHan) is\n"                                        \
        "currently supported.  More\n"                                          \
        "protocols added in future.\n\n"                                        \
        ":::Config supported:::\n"                                              \
        "Frequency: The\n frequency to TX/RX.\n"                                \
        "Protocol: KL(DH) = KeeLoq\n MF=DoorHan.\n"                            \
        " KL(All) = KeeLoq (any MF)\n"                                         \
        " KL(Custom) = KeeLoq\n"                                                \
        " (specific MF) set when doing\n a 'Sync Remote'.\n"                    \
        "Fix [SN+Btn]: The SN+button to decode. (20000000 is\n"                 \
        " test decode).\n"                                                      \
        "Replay attack: Allow replay\n attacks.\n"                              \
        "Window [next]: How many\n"                                             \
        " counts forward are\n acceptable?\n"                                   \
        "Window [future]: How many\n"                                           \
        " counts forward are\n considered future?\n"                            \
        "Window [gap]: How far can\n"                                           \
        " two sequential future counts\n be apart?\n"                           \
        "SN00 attack: Allow decoded\n 00 to wildcard SN.\n"                     \
        "SN bits [cfw*]: Number of\n"                                           \
        " bits to compare. (custom fw\n only?)\n"                               \
        "Count 0 opens: Count of 0 is\n treated as a match.\n"                  \
        "=========\n"                                                           \
        "author: @codeallnight\n"                                               \
        "https://discord.com/invite/NsjCvqwPAd\n"                              \
        "https://youtube.com/@MrDerekJamison",                                  \
        "滚动码接收器\n 版本 1.5\n" \
        "---\n"                                                                 \
        "练习滚动码攻击而不会失去同步!\n" \
        "此应用仅用于教育\n" \
        "目的.\n---\n"                                      \
        "当前支持 KeeLoq (DoorHan) 协议.\n" \
        "未来将添加更多\n" \
        "协议.\n\n"                                        \
        ":::支持的配置:::\n" \
        "频率: 发射/接收频率.\n" \
        "协议: KL(DH) = KeeLoq\n 厂商=DoorHan.\n" \
        " KL(All) = KeeLoq (任意厂商)\n" \
        " KL(Custom) = KeeLoq\n"                                                \
        " (指定厂商) 在'同步遥控器'时设置.\n" \
        "固定码 [SN+按钮]: 要解码的 SN+按钮. (20000000 为\n" \
        " 测试解码).\n"                  \
        "重放攻击: 允许重放\n 攻击.\n" \
        "窗口 [下一个]: 允许向前\n" \
        " 多少个计数?\n"       \
        "窗口 [未来]: 向前\n" \
        " 多少个计数被视为未来?\n" \
        "窗口 [间隔]: 两个\n" \
        " 连续未来计数之间可以相差多远?\n" \
        "SN00 攻击: 允许将解码的 00 作为通配符 SN.\n" \
        "SN 位数 [自定义*]: 比较\n" \
        " 的位数. (仅自定义固件?)\n" \
        "计数 0 视为开启: 计数 0 被\n 视为匹配.\n" \
        "=========\n"                                                           \
        "作者: @codeallnight\n"                            \
        "https://discord.com/invite/NsjCvqwPAd\n"                              \
        "https://youtube.com/@MrDerekJamison")
