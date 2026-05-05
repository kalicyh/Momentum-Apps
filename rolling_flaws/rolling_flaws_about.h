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
        "\xe6\xbb\x9a\xe5\x8a\xa8\xe7\xa0\x81\xe6\x8e\xa5\xe6\x94\xb6\xe5\x99\xa8\n \xe7\x89\x88\xe6\x9c\xac 1.5\n" \
        "---\n"                                                                 \
        "\xe7\xbb\x83\xe4\xb9\xa0\xe6\xbb\x9a\xe5\x8a\xa8\xe7\xa0\x81\xe6\x94\xbb\xe5\x87\xbb\xe8\x80\x8c\xe4\xb8\x8d\xe4\xbc\x9a\xe5\xa4\xb1\xe5\x8e\xbb\xe5\x90\x8c\xe6\xad\xa5!\n" \
        "\xe6\xad\xa4\xe5\xba\x94\xe7\x94\xa8\xe4\xbb\x85\xe7\x94\xa8\xe4\xba\x8e\xe6\x95\x99\xe8\x82\xb2\n" \
        "\xe7\x9b\xae\xe7\x9a\x84.\n---\n"                                      \
        "\xe5\xbd\x93\xe5\x89\x8d\xe6\x94\xaf\xe6\x8c\x81 KeeLoq (DoorHan) \xe5\x8d\x8f\xe8\xae\xae.\n" \
        "\xe6\x9c\xaa\xe6\x9d\xa5\xe5\xb0\x86\xe6\xb7\xbb\xe5\x8a\xa0\xe6\x9b\xb4\xe5\xa4\x9a\n" \
        "\xe5\x8d\x8f\xe8\xae\xae.\n\n"                                        \
        ":::\xe6\x94\xaf\xe6\x8c\x81\xe7\x9a\x84\xe9\x85\x8d\xe7\xbd\xae:::\n" \
        "\xe9\xa2\x91\xe7\x8e\x87: \xe5\x8f\x91\xe5\xb0\x84/\xe6\x8e\xa5\xe6\x94\xb6\xe9\xa2\x91\xe7\x8e\x87.\n" \
        "\xe5\x8d\x8f\xe8\xae\xae: KL(DH) = KeeLoq\n \xe5\x8e\x82\xe5\x95\x86=DoorHan.\n" \
        " KL(All) = KeeLoq (\xe4\xbb\xbb\xe6\x84\x8f\xe5\x8e\x82\xe5\x95\x86)\n" \
        " KL(Custom) = KeeLoq\n"                                                \
        " (\xe6\x8c\x87\xe5\xae\x9a\xe5\x8e\x82\xe5\x95\x86) \xe5\x9c\xa8'\xe5\x90\x8c\xe6\xad\xa5\xe9\x81\xa5\xe6\x8e\xa7\xe5\x99\xa8'\xe6\x97\xb6\xe8\xae\xbe\xe7\xbd\xae.\n" \
        "\xe5\x9b\xba\xe5\xae\x9a\xe7\xa0\x81 [SN+\xe6\x8c\x89\xe9\x92\xae]: \xe8\xa6\x81\xe8\xa7\xa3\xe7\xa0\x81\xe7\x9a\x84 SN+\xe6\x8c\x89\xe9\x92\xae. (20000000 \xe4\xb8\xba\n" \
        " \xe6\xb5\x8b\xe8\xaf\x95\xe8\xa7\xa3\xe7\xa0\x81).\n"                  \
        "\xe9\x87\x8d\xe6\x94\xbe\xe6\x94\xbb\xe5\x87\xbb: \xe5\x85\x81\xe8\xae\xb8\xe9\x87\x8d\xe6\x94\xbe\n \xe6\x94\xbb\xe5\x87\xbb.\n" \
        "\xe7\xaa\x97\xe5\x8f\xa3 [\xe4\xb8\x8b\xe4\xb8\x80\xe4\xb8\xaa]: \xe5\x85\x81\xe8\xae\xb8\xe5\x90\x91\xe5\x89\x8d\n" \
        " \xe5\xa4\x9a\xe5\xb0\x91\xe4\xb8\xaa\xe8\xae\xa1\xe6\x95\xb0?\n"       \
        "\xe7\xaa\x97\xe5\x8f\xa3 [\xe6\x9c\xaa\xe6\x9d\xa5]: \xe5\x90\x91\xe5\x89\x8d\n" \
        " \xe5\xa4\x9a\xe5\xb0\x91\xe4\xb8\xaa\xe8\xae\xa1\xe6\x95\xb0\xe8\xa2\xab\xe8\xa7\x86\xe4\xb8\xba\xe6\x9c\xaa\xe6\x9d\xa5?\n" \
        "\xe7\xaa\x97\xe5\x8f\xa3 [\xe9\x97\xb4\xe9\x9a\x94]: \xe4\xb8\xa4\xe4\xb8\xaa\n" \
        " \xe8\xbf\x9e\xe7\xbb\xad\xe6\x9c\xaa\xe6\x9d\xa5\xe8\xae\xa1\xe6\x95\xb0\xe4\xb9\x8b\xe9\x97\xb4\xe5\x8f\xaf\xe4\xbb\xa5\xe7\x9b\xb8\xe5\xb7\xae\xe5\xa4\x9a\xe8\xbf\x9c?\n" \
        "SN00 \xe6\x94\xbb\xe5\x87\xbb: \xe5\x85\x81\xe8\xae\xb8\xe5\xb0\x86\xe8\xa7\xa3\xe7\xa0\x81\xe7\x9a\x84 00 \xe4\xbd\x9c\xe4\xb8\xba\xe9\x80\x9a\xe9\x85\x8d\xe7\xac\xa6 SN.\n" \
        "SN \xe4\xbd\x8d\xe6\x95\xb0 [\xe8\x87\xaa\xe5\xae\x9a\xe4\xb9\x89*]: \xe6\xaf\x94\xe8\xbe\x83\n" \
        " \xe7\x9a\x84\xe4\xbd\x8d\xe6\x95\xb0. (\xe4\xbb\x85\xe8\x87\xaa\xe5\xae\x9a\xe4\xb9\x89\xe5\x9b\xba\xe4\xbb\xb6?)\n" \
        "\xe8\xae\xa1\xe6\x95\xb0 0 \xe8\xa7\x86\xe4\xb8\xba\xe5\xbc\x80\xe5\x90\xaf: \xe8\xae\xa1\xe6\x95\xb0 0 \xe8\xa2\xab\n \xe8\xa7\x86\xe4\xb8\xba\xe5\x8c\xb9\xe9\x85\x8d.\n" \
        "=========\n"                                                           \
        "\xe4\xbd\x9c\xe8\x80\x85: @codeallnight\n"                            \
        "https://discord.com/invite/NsjCvqwPAd\n"                              \
        "https://youtube.com/@MrDerekJamison")
