#include "menu.h"
#include "confirmation_view.h"
#include "ghost_esp_ep.h"
#include "settings_def.h"
#include "settings_storage.h"
#include "uart_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>

struct View {
    ViewDrawCallback draw_callback;
    ViewInputCallback input_callback;
    ViewCustomCallback custom_callback;

    ViewModelType model_type;
    ViewNavigationCallback previous_callback;
    ViewCallback enter_callback;
    ViewCallback exit_callback;
    ViewOrientation orientation;

    ViewUpdateCallback update_callback;
    void* update_callback_context;

    void* model;
    void* context;
};

typedef struct {
    const char* label; // Display label in menu
    const char* command; // UART command to send
    const char* capture_prefix; // Prefix for capture files (NULL if none)
    const char* file_ext; // File extension for captures (NULL if none)
    const char* folder; // Folder for captures (NULL if none)
    bool needs_input; // Whether command requires text input
    const char* input_text; // Text to show in input box (NULL if none)
    bool needs_confirmation; // Whether command needs confirmation
    const char* confirm_header; // Confirmation dialog header
    const char* confirm_text; // Confirmation dialog text
    const char* details_header; // Header for details view
    const char* details_text; // Detailed description/info text
} MenuCommand;

typedef struct {
    const char* label;
    const char* command;
    const char* capture_prefix;
} SniffCommandDef;

typedef struct {
    const char* label;
    const char* command;
} BeaconSpamDef;

typedef struct {
    AppState* state;
    const MenuCommand* command;
} MenuCommandContext;

typedef struct {
    const char* label;
    const char* command;
    const char* details_header;
    const char* details_text;
    bool needs_input;
    const char* input_text;
} CyclingMenuDef;

// Forward declarations of static functions
static void show_menu(
    AppState* state,
    const MenuCommand* commands,
    size_t command_count,
    const char* header,
    Submenu* menu,
    uint8_t view_id);
static void show_command_details(AppState* state, const MenuCommand* command);
static bool menu_input_handler(InputEvent* event, void* context);
static void text_input_result_callback(void* context);
static void confirmation_ok_callback(void* context);
static void confirmation_cancel_callback(void* context);
static void app_info_ok_callback(void* context);
static void ir_sweep_stop_callback(void* context);
static void execute_menu_command(AppState* state, const MenuCommand* command);
static void error_callback(void* context);
static bool ir_query_and_parse_list(AppState* state);
static bool ir_query_and_parse_show(AppState* state, uint32_t remote_index);
static bool ir_query_and_parse_universals(AppState* state);
static bool ir_query_and_parse_universal_buttons(AppState* state, const char* filename);
static void ir_show_remotes_menu(AppState* state);
static void ir_show_buttons_menu(AppState* state);
static void ir_show_universals_menu(AppState* state);
static void ir_show_error(AppState* state, const char* text);
static void show_result_dialog(AppState* state, const char* header, const char* text);
static bool handle_ir_command_feedback_ex(
    AppState* state,
    const char* cmd,
    bool send_cmd,
    bool reset_buffers);
static bool handle_ir_command_feedback(AppState* state, const char* cmd);
static bool ir_index_buttons_from_file(AppState* state);
static void ir_send_button_from_file(AppState* state, uint32_t button_index);

// Sniff command definitions
static const SniffCommandDef sniff_commands[] = {
    {GHOST_ESP_UI_TEXT("< Sniff WPS >", "< 嗅探 WPS >"), "capture -wps\n", "wps_capture"},
    {GHOST_ESP_UI_TEXT("< Sniff Raw Packets >", "< 嗅探原始数据包 >"), "capture -raw\n", "raw_capture"},
    {GHOST_ESP_UI_TEXT("< Sniff Probes >", "< 嗅探探测帧 >"), "capture -p\n", "probe_capture"},
    {GHOST_ESP_UI_TEXT("< Sniff Deauth >", "< 嗅探断连帧 >"), "capture -deauth\n", "deauth_capture"},
    {GHOST_ESP_UI_TEXT("< Sniff Beacons >", "< 嗅探信标帧 >"), "capture -beacon\n", "beacon_capture"},
    {GHOST_ESP_UI_TEXT("< Sniff EAPOL >", "< 嗅探 EAPOL >"), "capture -eapol\n", "eapol_capture"},
    {GHOST_ESP_UI_TEXT("< Sniff Pwn >", "< 嗅探 Pwn >"), "capture -pwn\n", "pwn_capture"},
};

// Beacon spam command definitions
static const CyclingMenuDef beacon_spam_commands[] = {
    {GHOST_ESP_UI_TEXT("< Beacon Spam (List) >", "< 信标洪流 (列表) >"),
     "beaconspam -l\n",
     GHOST_ESP_UI_TEXT("Beacon Spam (List)", "信标洪流 (列表)"),
     GHOST_ESP_UI_TEXT("Spam SSIDs from list.", "使用列表中的 SSID 发送。"),
     false,
     NULL},
    {GHOST_ESP_UI_TEXT("< Beacon Spam (Random) >", "< 信标洪流 (随机) >"),
     "beaconspam -r\n",
     GHOST_ESP_UI_TEXT("Beacon Spam (Random)", "信标洪流 (随机)"),
     GHOST_ESP_UI_TEXT("Spam random SSIDs.", "发送随机 SSID。"),
     false,
     NULL},
    {GHOST_ESP_UI_TEXT("< Beacon Spam (Rickroll) >", "< 信标洪流 (Rickroll) >"),
     "beaconspam -rr\n",
     GHOST_ESP_UI_TEXT("Beacon Spam (Rickroll)", "信标洪流 (Rickroll)"),
     GHOST_ESP_UI_TEXT("Spam Rickroll SSIDs.", "发送 Rickroll SSID。"),
     false,
     NULL},
    {GHOST_ESP_UI_TEXT("< Beacon Spam (Custom) >", "< 信标洪流 (自定义) >"),
     "beaconspam",
     GHOST_ESP_UI_TEXT("Beacon Spam (Custom)", "信标洪流 (自定义)"),
     GHOST_ESP_UI_TEXT("Spam custom SSID.", "发送自定义 SSID。"),
     true,
     GHOST_ESP_UI_TEXT("SSID Name", "SSID 名称")},
};

// BLE spam command definitions
static const CyclingMenuDef ble_spam_commands[] = {
    {GHOST_ESP_UI_TEXT("< BLE Spam (Apple) >", "< BLE 洪流 (Apple) >"),
     "blespam -apple\n",
     GHOST_ESP_UI_TEXT("BLE Spam (Apple)", "BLE 洪流 (Apple)"),
     GHOST_ESP_UI_TEXT("Spam Apple BLE devices.", "发送 Apple BLE 设备广播。"),
     false,
     NULL},
    {GHOST_ESP_UI_TEXT("< BLE Spam (Microsoft) >", "< BLE 洪流 (Microsoft) >"),
     "blespam -ms\n",
     GHOST_ESP_UI_TEXT("BLE Spam (Microsoft)", "BLE 洪流 (Microsoft)"),
     GHOST_ESP_UI_TEXT("Spam Microsoft BLE devices.", "发送 Microsoft BLE 设备广播。"),
     false,
     NULL},
    {GHOST_ESP_UI_TEXT("< BLE Spam (Samsung) >", "< BLE 洪流 (Samsung) >"),
     "blespam -samsung\n",
     GHOST_ESP_UI_TEXT("BLE Spam (Samsung)", "BLE 洪流 (Samsung)"),
     GHOST_ESP_UI_TEXT("Spam Samsung BLE devices.", "发送 Samsung BLE 设备广播。"),
     false,
     NULL},
    {GHOST_ESP_UI_TEXT("< BLE Spam (Google) >", "< BLE 洪流 (Google) >"),
     "blespam -google\n",
     GHOST_ESP_UI_TEXT("BLE Spam (Google)", "BLE 洪流 (Google)"),
     GHOST_ESP_UI_TEXT("Spam Google BLE devices.", "发送 Google BLE 设备广播。"),
     false,
     NULL},
    {GHOST_ESP_UI_TEXT("< BLE Spam (Random) >", "< BLE 洪流 (随机) >"),
     "blespam -random\n",
     GHOST_ESP_UI_TEXT("BLE Spam (Random)", "BLE 洪流 (随机)"),
     GHOST_ESP_UI_TEXT("Spam random BLE devices.", "发送随机 BLE 设备广播。"),
     false,
     NULL},
};

static size_t current_rgb_index = 0;

static const CyclingMenuDef rgbmode_commands[] = {
    {GHOST_ESP_UI_TEXT("< LED: Rainbow >", "< LED: 彩虹 >"), "rgbmode rainbow\n", GHOST_ESP_UI_TEXT("LED: Rainbow", "LED: 彩虹"), GHOST_ESP_UI_TEXT("Cycle rainbow colors.", "循环彩虹色。"), false, NULL},
    {GHOST_ESP_UI_TEXT("< LED: Police >", "< LED: 警灯 >"), "rgbmode police\n", GHOST_ESP_UI_TEXT("LED: Police", "LED: 警灯"), GHOST_ESP_UI_TEXT("Police light effect.", "警灯闪烁效果。"), false, NULL},
    {GHOST_ESP_UI_TEXT("< LED: Strobe >", "< LED: 频闪 >"), "rgbmode strobe\n", GHOST_ESP_UI_TEXT("LED: Strobe", "LED: 频闪"), GHOST_ESP_UI_TEXT("Strobe light effect.", "频闪灯效果。"), false, NULL},
    {GHOST_ESP_UI_TEXT("< LED: Off >", "< LED: 关闭 >"), "rgbmode off\n", GHOST_ESP_UI_TEXT("LED: Off", "LED: 关闭"), GHOST_ESP_UI_TEXT("Turn off LED.", "关闭 LED。"), false, NULL},
    {GHOST_ESP_UI_TEXT("< LED: Red >", "< LED: 红色 >"), "rgbmode red\n", GHOST_ESP_UI_TEXT("LED: Red", "LED: 红色"), GHOST_ESP_UI_TEXT("Set LED to red.", "设置 LED 为红色。"), false, NULL},
    {GHOST_ESP_UI_TEXT("< LED: Green >", "< LED: 绿色 >"), "rgbmode green\n", GHOST_ESP_UI_TEXT("LED: Green", "LED: 绿色"), GHOST_ESP_UI_TEXT("Set LED to green.", "设置 LED 为绿色。"), false, NULL},
    {GHOST_ESP_UI_TEXT("< LED: Blue >", "< LED: 蓝色 >"), "rgbmode blue\n", GHOST_ESP_UI_TEXT("LED: Blue", "LED: 蓝色"), GHOST_ESP_UI_TEXT("Set LED to blue.", "设置 LED 为蓝色。"), false, NULL},
    {GHOST_ESP_UI_TEXT("< LED: Yellow >", "< LED: 黄色 >"), "rgbmode yellow\n", GHOST_ESP_UI_TEXT("LED: Yellow", "LED: 黄色"), GHOST_ESP_UI_TEXT("Set LED to yellow.", "设置 LED 为黄色。"), false, NULL},
    {GHOST_ESP_UI_TEXT("< LED: Purple >", "< LED: 紫色 >"), "rgbmode purple\n", GHOST_ESP_UI_TEXT("LED: Purple", "LED: 紫色"), GHOST_ESP_UI_TEXT("Set LED to purple.", "设置 LED 为紫色。"), false, NULL},
    {GHOST_ESP_UI_TEXT("< LED: Cyan >", "< LED: 青色 >"), "rgbmode cyan\n", GHOST_ESP_UI_TEXT("LED: Cyan", "LED: 青色"), GHOST_ESP_UI_TEXT("Set LED to cyan.", "设置 LED 为青色。"), false, NULL},
    {GHOST_ESP_UI_TEXT("< LED: Orange >", "< LED: 橙色 >"), "rgbmode orange\n", GHOST_ESP_UI_TEXT("LED: Orange", "LED: 橙色"), GHOST_ESP_UI_TEXT("Set LED to orange.", "设置 LED 为橙色。"), false, NULL},
    {GHOST_ESP_UI_TEXT("< LED: White >", "< LED: 白色 >"), "rgbmode white\n", GHOST_ESP_UI_TEXT("LED: White", "LED: 白色"), GHOST_ESP_UI_TEXT("Set LED to white.", "设置 LED 为白色。"), false, NULL},
    {GHOST_ESP_UI_TEXT("< LED: Pink >", "< LED: 粉色 >"), "rgbmode pink\n", GHOST_ESP_UI_TEXT("LED: Pink", "LED: 粉色"), GHOST_ESP_UI_TEXT("Set LED to pink.", "设置 LED 为粉色。"), false, NULL},
};

static const CyclingMenuDef wifi_scan_modes[] = {
    {GHOST_ESP_UI_TEXT("< Scan: (APs) >", "< 扫描: (AP) >"), "scanap\n", GHOST_ESP_UI_TEXT("WiFi AP Scanner", "WiFi AP 扫描"), GHOST_ESP_UI_TEXT("Scans for WiFi APs...", "扫描 WiFi AP..."), false, NULL},
    {GHOST_ESP_UI_TEXT("< Scan: (APs Live) >", "< 扫描: (AP 实时) >"),
     "scanap -live\n",
     GHOST_ESP_UI_TEXT("Live AP Scanner", "实时 AP 扫描"),
     GHOST_ESP_UI_TEXT(
         "Continuously updates as APs are found\n- SSID names\n- Signal levels\n- "
         "Security type\n- Channel info\n",
         "发现 AP 时持续更新\n- SSID 名称\n- 信号强度\n- "
         "安全类型\n- 信道信息\n"),
     false,
     NULL},
    {GHOST_ESP_UI_TEXT("< Scan: (Stations) >", "< 扫描: (站点) >"), "scansta\n", GHOST_ESP_UI_TEXT("Station Scanner", "站点扫描"), GHOST_ESP_UI_TEXT("Scans for clients...", "扫描客户端..."), false, NULL},
    {GHOST_ESP_UI_TEXT("< Scan: (AP+STA) >", "< 扫描: (AP+STA) >"), "scanall\n", GHOST_ESP_UI_TEXT("Scan All", "全部扫描"), GHOST_ESP_UI_TEXT("Combined AP/Station scan...", "AP/站点联合扫描..."), false, NULL},
};

static const CyclingMenuDef wifi_list_modes[] = {
    {GHOST_ESP_UI_TEXT("< List: (APs) >", "< 列表: (AP) >"),
     "list -a\n",
     GHOST_ESP_UI_TEXT("List Access Points", "AP 列表"),
     GHOST_ESP_UI_TEXT("Shows list of APs found during last scan.", "显示上次扫描到的 AP 列表。"),
     false,
     NULL},
    {GHOST_ESP_UI_TEXT("< List: (Stations) >", "< 列表: (站点) >"),
     "list -s\n",
     GHOST_ESP_UI_TEXT("List Stations", "站点列表"),
     GHOST_ESP_UI_TEXT("Shows list of clients found during last scan.", "显示上次扫描到的客户端列表。"),
     false,
     NULL},
};

static const CyclingMenuDef wifi_select_modes[] = {
    {GHOST_ESP_UI_TEXT("< Select: (AP) >", "< 选择: (AP) >"),
     "select -a",
     GHOST_ESP_UI_TEXT("Select Access Point", "选择 AP"),
     GHOST_ESP_UI_TEXT("Select an AP by number from the scanned list.", "从扫描列表中按编号选择 AP。"),
     true,
     GHOST_ESP_UI_TEXT("AP Number", "AP 编号")},
    {GHOST_ESP_UI_TEXT("< Select: (Station) >", "< 选择: (站点) >"),
     "select -s",
     GHOST_ESP_UI_TEXT("Select Station", "选择站点"),
     GHOST_ESP_UI_TEXT("Target a station by number from the scan list for attacks.", "从扫描列表中按编号选择站点进行攻击。"),
     true,
     GHOST_ESP_UI_TEXT("Station Number", "站点编号")},
};

static const CyclingMenuDef wifi_listen_modes[] = {
    {GHOST_ESP_UI_TEXT("< Listen Probes (Hop) >", "< 监听探测 (跳频) >"),
     "listenprobes\n",
     GHOST_ESP_UI_TEXT("Listen for Probes", "监听探测帧"),
     GHOST_ESP_UI_TEXT("Listen for and log probe requests\nwhile hopping channels.", "跳频监听并记录探测请求。"),
     false,
     NULL},
    {GHOST_ESP_UI_TEXT("< Listen Probes (Chan) >", "< 监听探测 (定频) >"),
     "listenprobes",
     GHOST_ESP_UI_TEXT("Listen on Channel", "定频监听"),
     GHOST_ESP_UI_TEXT("Listen for probe requests on a\nspecific channel.", "在指定信道上监听探测请求。"),
     true,
     GHOST_ESP_UI_TEXT("Channel (1-165)", "信道 (1-165)")},
};

static size_t current_sniff_index = 0;
static size_t current_beacon_index = 0;
static size_t current_ble_spam_index = 0;
static size_t current_wifi_scan_index = 0;
static size_t current_wifi_list_index = 0;
static size_t current_wifi_select_index = 0;
static size_t current_wifi_listen_index = 0;

// WiFi menu command definitions
static const MenuCommand wifi_scanning_commands[] = {
    {
        .label = GHOST_ESP_UI_TEXT("< Scan: (APs) >", "< 扫描: (AP) >"),
        .command = wifi_scan_modes[0].command,
        .details_header = wifi_scan_modes[0].details_header,
        .details_text = wifi_scan_modes[0].details_text,
    },
    {
        .label = GHOST_ESP_UI_TEXT("< List: (APs) >", "< 列表: (AP) >"),
        .command = wifi_list_modes[0].command,
        .details_header = wifi_list_modes[0].details_header,
        .details_text = wifi_list_modes[0].details_text,
    },
    {
        .label = GHOST_ESP_UI_TEXT("< Select: (AP) >", "< 选择: (AP) >"),
        .command = wifi_select_modes[0].command,
        .needs_input = wifi_select_modes[0].needs_input,
        .input_text = wifi_select_modes[0].input_text,
        .details_header = wifi_select_modes[0].details_header,
        .details_text = wifi_select_modes[0].details_text,
    },
    {
        .label = GHOST_ESP_UI_TEXT("< Listen Probes (Hop) >", "< 监听探测 (跳频) >"),
        .command = wifi_listen_modes[0].command,
        .needs_input = wifi_listen_modes[0].needs_input,
        .input_text = wifi_listen_modes[0].input_text,
        .details_header = wifi_listen_modes[0].details_header,
        .details_text = wifi_listen_modes[0].details_text,
    },
    {
        .label = GHOST_ESP_UI_TEXT("Pineapple Detect", "Pineapple 检测"),
        .command = "pineap\n",
        .details_header = GHOST_ESP_UI_TEXT("Pineapple Detection", "Pineapple 检测"),
        .details_text = GHOST_ESP_UI_TEXT("Detects WiFi Pineapple devices\n", "检测 WiFi Pineapple 设备\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop Pineapple Detect", "停止 Pineapple 检测"),
        .command = "pineap -s\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop Pineapple Detect", "停止 Pineapple 检测"),
        .details_text = GHOST_ESP_UI_TEXT("Stops Pineapple detection mode.", "停止 Pineapple 检测模式。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Flock Detection", "Flock 检测"),
        .command = "flockscan\n",
        .details_header = GHOST_ESP_UI_TEXT("Flock Detection", "Flock 检测"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Detect Flock Safety cameras,\n"
            "extended battery units, and\n"
            "Penguin surveillance devices\n"
            "on 2.4 GHz Wi-Fi.\n"
            "Uses OUI matching, wildcard\n"
            "probe detection, and SSID\n"
            "keyword matching.\n",
            "检测 2.4GHz WiFi 上的\n"
            "Flock Safety 摄像头、\n"
            "扩展电池单元和 Penguin\n"
            "监控设备。\n"
            "使用 OUI 匹配、通配\n"
            "探测检测和 SSID\n"
            "关键词匹配。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Flock List", "Flock 列表"),
        .command = "flocklist\n",
        .details_header = GHOST_ESP_UI_TEXT("Flock List", "Flock 列表"),
        .details_text = GHOST_ESP_UI_TEXT(
            "List detected Flock Safety\n"
            "surveillance devices.\n"
            "Run after Flock Detection.\n",
            "列出已检测到的 Flock Safety\n"
            "监控设备。\n"
            "请先运行 Flock 检测。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop Flock Detection", "停止 Flock 检测"),
        .command = "flockstop\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop Flock Detection", "停止 Flock 检测"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Stops Flock Safety camera\n"
            "detection scanning.\n",
            "停止 Flock Safety 摄像头\n"
            "检测扫描。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Channel Congestion", "信道拥塞"),
        .command = "congestion\n",
        .details_header = GHOST_ESP_UI_TEXT("Channel Congestion", "信道拥塞"),
        .details_text = GHOST_ESP_UI_TEXT("Display Wi-Fi channel\ncongestion chart.\n", "显示 WiFi 信道\n拥塞图表。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Scan Ports", "端口扫描"),
        .command = "scanports",
        .needs_input = true,
        .input_text = GHOST_ESP_UI_TEXT("local or IP [options]", "local 或 IP [选项]"),
        .details_header = GHOST_ESP_UI_TEXT("Port Scanner", "端口扫描"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Scan ports on local net\n"
            "or specific IP.\n"
            "Options: -C, -A, range\n"
            "Ex: local -C\n"
            "Ex: 192.168.1.1 80-1000",
            "扫描本地网络或指定 IP 的端口。\n"
            "选项: -C, -A, 范围\n"
            "例: local -C\n"
            "例: 192.168.1.1 80-1000"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("ARP Scan", "ARP 扫描"),
        .command = "scanarp\n",
        .details_header = GHOST_ESP_UI_TEXT("ARP Scan", "ARP 扫描"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Initiates an ARP scan on the local network to discover hosts:\n"
            "- Sends ARP requests across the subnet\n"
            "- Shows responding IP/MAC pairs\n"
            "Requires WiFi connection.\n",
            "在本地网络发起 ARP 扫描发现主机:\n"
            "- 发送 ARP 请求到子网\n"
            "- 显示响应的 IP/MAC 对\n"
            "需要 WiFi 连接。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Scan SSH", "SSH 扫描"),
        .command = "scanssh",
        .needs_input = true,
        .input_text = "IP",
        .details_header = GHOST_ESP_UI_TEXT("SSH Scan", "SSH 扫描"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Initiate an SSH port/service scan against the target IP:\n"
            "- Provide an IP address (e.g., 192.168.1.10)\n"
            "- Scans common SSH ports and reports responses\n"
            "- Requires network connectivity\n\n",
            "对目标 IP 进行 SSH 端口/服务扫描:\n"
            "- 输入 IP 地址 (如 192.168.1.10)\n"
            "- 扫描常用 SSH 端口并报告响应\n"
            "- 需要网络连接\n\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Full Environment Sweep", "全面环境扫描"),
        .command = "sweep\n",
        .details_header = GHOST_ESP_UI_TEXT("Environment Sweep", "环境扫描"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Full sweep: WiFi APs, stations,\n"
            "and BLE devices.\n"
            "Saves CSV report to SD.\n"
            "Uses default timing for scan.\n",
            "全面扫描: WiFi AP、站点\n"
            "和 BLE 设备。\n"
            "保存 CSV 报告到 SD 卡。\n"
            "使用默认扫描间隔。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Combined AP+STA Scan", "AP+STA 联合扫描"),
        .command = "scanall",
        .needs_input = true,
        .input_text = GHOST_ESP_UI_TEXT("Seconds", "秒数"),
        .details_header = GHOST_ESP_UI_TEXT("Scan All", "全部扫描"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Combined AP and station scan\n"
            "with summary report.\n"
            "Optionally specify duration.\n",
            "AP 和站点联合扫描\n"
            "并生成汇总报告。\n"
            "可指定扫描时长。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Track Selected AP", "追踪选定 AP"),
        .command = "trackap\n",
        .details_header = GHOST_ESP_UI_TEXT("Track AP Signal", "追踪 AP 信号"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Track selected AP signal\n"
            "strength (RSSI) in real-time.\n"
            "Select an AP first.\n",
            "实时追踪选定 AP 的\n"
            "信号强度 (RSSI)。\n"
            "请先选择一个 AP。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Track Selected Station", "追踪选定站点"),
        .command = "tracksta\n",
        .details_header = GHOST_ESP_UI_TEXT("Track Station Signal", "追踪站点信号"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Track selected station signal\n"
            "strength (RSSI) in real-time.\n"
            "Select a station first.\n",
            "实时追踪选定站点的\n"
            "信号强度 (RSSI)。\n"
            "请先选择一个站点。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop Listen Probes", "停止监听探测"),
        .command = "listenprobes stop\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop Listening", "停止监听"),
        .details_text = GHOST_ESP_UI_TEXT("Stops the probe listener.", "停止探测帧监听器。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop Scan", "停止扫描"),
        .command = "stopscan\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop Scan", "停止扫描"),
        .details_text = GHOST_ESP_UI_TEXT("Stops AP or Station scan.", "停止 AP 或站点扫描。"),
    },
};

static const MenuCommand wifi_capture_commands[] = {
    {
        .label = GHOST_ESP_UI_TEXT("< Sniff WPS >", "< 嗅探 WPS >"),
        .command = "capture -wps\n",
        .capture_prefix = "wps_capture",
        .file_ext = "pcap",
        .folder = GHOST_ESP_APP_FOLDER_PCAPS,
        .details_header = GHOST_ESP_UI_TEXT("Variable Sniff", "切换嗅探模式"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Use Left/Right to change:\n"
            "- WPS traffic\n"
            "- Raw packets\n"
            "- Probe requests\n"
            "- Deauth frames\n"
            "- Beacon frames\n"
            "- EAPOL/Handshakes\n",
            "左右键切换模式:\n"
            "- WPS 流量\n"
            "- 原始数据包\n"
            "- 探测请求\n"
            "- 断连帧\n"
            "- 信标帧\n"
            "- EAPOL/握手\n"),
    },
};

static const MenuCommand wifi_attack_commands[] = {
    {
        .label = GHOST_ESP_UI_TEXT("< Beacon Spam (List) >", "< 信标洪流 (列表) >"),
        .command = "beaconspam -l\n",
        .needs_input = false,
        .input_text = GHOST_ESP_UI_TEXT("SSID Name", "SSID 名称"),
        .details_header = GHOST_ESP_UI_TEXT("Variable Beacon Spam", "切换信标洪流模式"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Use Left/Right to change:\n"
            "- List mode\n"
            "- Random names\n"
            "- Rickroll mode\n"
            "- Custom SSID\n"
            "Range: ~50-100m\n",
            "左右键切换模式:\n"
            "- 列表模式\n"
            "- 随机名称\n"
            "- Rickroll 模式\n"
            "- 自定义 SSID\n"
            "范围: ~50-100m\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Deauth", "断连攻击"),
        .command = "attack -d\n",
        .details_header = GHOST_ESP_UI_TEXT("Deauth Attack", "断连攻击"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Sends deauth frames to\n"
            "disconnect clients from\n"
            "selected network.\n"
            "Range: ~50-100m\n",
            "发送断连帧使客户端\n"
            "从选定网络断开。\n"
            "范围: ~50-100m\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("EAPOL Logoff", "EAPOL 注销"),
        .command = "attack -e\n",
        .details_header = GHOST_ESP_UI_TEXT("EAPOL Logoff Attack", "EAPOL 注销攻击"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Sends EAPOL logoff frames to\n"
            "disconnect clients.",
            "发送 EAPOL 注销帧\n"
            "断开客户端连接。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("SAE Handshake Flood", "SAE 握手洪流"),
        .command = "saeflood",
        .needs_input = true,
        .input_text = "SAE PSK",
        .details_header = GHOST_ESP_UI_TEXT("SAE Flood Attack", "SAE 洪流攻击"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Floods WPA3 networks with\nSAE handshakes using the\n"
            "provided PSK. Select a\nWPA3 AP first.",
            "使用提供的 PSK 向 WPA3\n网络发送 SAE 握手洪流。\n请先选择一个 WPA3 AP。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop SAE Flood", "停止 SAE 洪流"),
        .command = "stopsaeflood\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop SAE Flood", "停止 SAE 洪流"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Stops active SAE flood and\n"
            "password spray attacks.",
            "停止 SAE 洪流和\n"
            "密码喷射攻击。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("SAE Flood Help", "SAE 洪流帮助"),
        .command = "saefloodhelp\n",
        .details_header = GHOST_ESP_UI_TEXT("SAE Flood Help", "SAE 洪流帮助"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Shows usage guidance for\n"
            "SAE flood operations.",
            "显示 SAE 洪流操作\n"
            "使用指南。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Karma Start", "Karma 启动"),
        .command = "karma start\n",
        .details_header = GHOST_ESP_UI_TEXT("Karma Rogue AP", "Karma 伪 AP"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Replies to probe requests\n"
            "with saved SSIDs.",
            "使用已保存的 SSID\n"
            "响应探测请求。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Karma Start (Custom)", "Karma 启动 (自定义)"),
        .command = "karma start",
        .needs_input = true,
        .input_text = "SSID [SSID...]",
        .details_header = GHOST_ESP_UI_TEXT("Karma Rogue AP (Custom)", "Karma 伪 AP (自定义)"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Replies to probe requests\n"
            "using SSIDs you provide\n"
            "or saved entries.",
            "使用你提供的 SSID\n"
            "或已保存的条目\n"
            "响应探测请求。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Karma Stop", "Karma 停止"),
        .command = "karma stop\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop Karma Rogue AP", "停止 Karma 伪 AP"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Stops the active Karma\n"
            "rogue AP responder.",
            "停止 Karma 伪 AP\n"
            "响应器。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("DHCP Starve Start", "DHCP 耗尽启动"),
        .command = "dhcpstarve",
        .needs_input = true,
        .input_text = "start [threads]",
        .details_header = GHOST_ESP_UI_TEXT("DHCP Starve Attack", "DHCP 耗尽攻击"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Exhausts DHCP server's IP pool.\n"
            "Input: start [threads]\n"
            "e.g., 'start' or 'start 5'",
            "耗尽 DHCP 服务器的 IP 池。\n"
            "输入: start [线程数]\n"
            "例: 'start' 或 'start 5'"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("DHCP Starve Stop", "DHCP 耗尽停止"),
        .command = "dhcpstarve stop\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop DHCP Starve", "停止 DHCP 耗尽"),
        .details_text = GHOST_ESP_UI_TEXT("Stops the DHCP starvation attack.", "停止 DHCP 耗尽攻击。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop Deauth/SAE/EAPOL", "停止断连/SAE/EAPOL"),
        .command = "stopdeauth\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop Attacks", "停止攻击"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Stops Deauth, SAE Flood,\n"
            "and EAPOL Logoff attacks.",
            "停止断连、SAE 洪流\n"
            "和 EAPOL 注销攻击。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Add SSID to Beacon List", "添加 SSID 到信标列表"),
        .command = "beaconadd",
        .needs_input = true,
        .input_text = "SSID",
        .details_header = GHOST_ESP_UI_TEXT("Add to Beacon List", "添加到信标列表"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Add an SSID to the list used\n"
            "by Beacon List Spam.",
            "添加 SSID 到信标洪流列表。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Remove SSID from Beacon List", "从信标列表移除 SSID"),
        .command = "beaconremove",
        .needs_input = true,
        .input_text = "SSID",
        .details_header = GHOST_ESP_UI_TEXT("Remove from Beacon List", "从信标列表移除"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Remove an SSID from the\n"
            "beacon spam list.",
            "从信标洪流列表中移除 SSID。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Clear Beacon List", "清空信标列表"),
        .command = "beaconclear\n",
        .details_header = GHOST_ESP_UI_TEXT("Clear Beacon List", "清空信标列表"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Clears all SSIDs from the\n"
            "beacon spam list.",
            "清空信标洪流列表中的所有 SSID。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Show Beacon List", "显示信标列表"),
        .command = "beaconshow\n",
        .details_header = GHOST_ESP_UI_TEXT("Show Beacon List", "显示信标列表"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Displays all SSIDs in the\n"
            "beacon spam list.",
            "显示信标洪流列表中的所有 SSID。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Start Beacon List Spam", "启动信标列表洪流"),
        .command = "beaconspamlist\n",
        .details_header = GHOST_ESP_UI_TEXT("Beacon List Spam", "信标列表洪流"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Starts beacon spam using the\n"
            "custom list of SSIDs.",
            "使用自定义 SSID 列表\n"
            "启动信标洪流。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop Beacon Spam", "停止信标洪流"),
        .command = "stopspam\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop Beacon Spam", "停止信标洪流"),
        .details_text = GHOST_ESP_UI_TEXT("Stops any active beacon spam.", "停止所有信标洪流。"),
    },
};

static const MenuCommand wifi_network_commands[] = {
    {
        .label = GHOST_ESP_UI_TEXT("Evil Portal", "Evil Portal"),
        .command = "startportal",
        .needs_input = true,
        .input_text = GHOST_ESP_UI_TEXT("<filepath> <SSID> <PSK (leave blank for open)>", "<路径> <SSID> <PSK (留空为开放)>"),
        .details_header = GHOST_ESP_UI_TEXT("Evil Portal", "Evil Portal"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Captive portal for\n"
            "credential harvest.\n"
            "Configure in WebUI:\n"
            "- Portal settings\n"
            "- Landing page\n",
            "用于凭证捕获的\n"
            "强制门户。\n"
            "在 WebUI 中配置:\n"
            "- 门户设置\n"
            "- 落地页\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("List Portals", "门户列表"),
        .command = "listportals\n",
        .details_header = GHOST_ESP_UI_TEXT("List Portals", "门户列表"),
        .details_text = GHOST_ESP_UI_TEXT("Show all available HTML portals\non the SD card.", "显示 SD 卡上所有可用的 HTML 门户。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Set Evil Portal HTML", "设置 Evil Portal HTML"),
        .command = "set_evil_portal_html",
        .needs_input = true,
        .input_text = GHOST_ESP_UI_TEXT("HTML File", "HTML 文件"),
        .details_header = GHOST_ESP_UI_TEXT("Set Evil Portal HTML", "设置 Evil Portal HTML"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Select and send an HTML\n"
            "file to the ESP32 for\n"
            "the evil portal.\n\n",
            "选择并发送 HTML 文件\n"
            "到 ESP32 用于\n"
            "Evil Portal。\n\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Clear Evil Portal HTML", "清除 Evil Portal HTML"),
        .command = "evilportal -c clear\n",
        .details_header = GHOST_ESP_UI_TEXT("Clear Evil Portal", "清除 Evil Portal"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Restores the default portal\n"
            "landing page on the ESP.",
            "恢复 ESP 上的默认\n"
            "门户落地页。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Connect To WiFi", "连接 WiFi"),
        .command = "connect",
        .needs_input = true,
        .input_text = "SSID",
        .details_header = GHOST_ESP_UI_TEXT("WiFi Connect", "WiFi 连接"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Connect ESP to WiFi:\n"
            "Enter SSID followed by password.\n\n",
            "连接 ESP 到 WiFi:\n"
            "输入 SSID 和密码。\n\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Connect to Saved WiFi", "连接已保存 WiFi"),
        .command = "connect\n",
        .details_header = GHOST_ESP_UI_TEXT("Connect (Saved)", "连接 (已保存)"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Connect to the previously saved WiFi credentials on the ESP.\n"
            "No input required.\n\n",
            "使用 ESP 上已保存的 WiFi 凭据连接。\n"
            "无需输入。\n\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Disconnect WiFi", "断开 WiFi"),
        .command = "disconnect\n",
        .details_header = GHOST_ESP_UI_TEXT("Disconnect", "断开连接"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Disconnects from the current WiFi network on the ESP.\n"
            "No input required.\n",
            "断开 ESP 当前的 WiFi 连接。\n"
            "无需输入。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Cast Random Video", "投送随机视频"),
        .command = "dialconnect\n",
        .needs_confirmation = true,
        .confirm_header = GHOST_ESP_UI_TEXT("Cast Video", "投送视频"),
        .confirm_text = GHOST_ESP_UI_TEXT(
            "Make sure you've connected\nto WiFi first via "
            "the\n'Connect to WiFi' option.\n",
            "请确保已通过\n'连接 WiFi' 选项\n连接到 WiFi。\n"),
        .details_header = GHOST_ESP_UI_TEXT("Video Cast", "视频投送"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Casts random videos\n"
            "to nearby Cast/DIAL\n"
            "enabled devices.\n"
            "Range: ~50m\n\n",
            "向附近的 Cast/DIAL\n"
            "设备投送随机视频。\n"
            "范围: ~50m\n\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Printer Power", "打印机电源"),
        .command = "powerprinter\n",
        .needs_confirmation = true,
        .confirm_header = GHOST_ESP_UI_TEXT("Printer Power", "打印机电源"),
        .confirm_text = GHOST_ESP_UI_TEXT(
            "You need to configure\n settings in the WebUI\n for "
            "this command.\n",
            "需要在 WebUI 中\n配置设置才能\n使用此命令。\n"),
        .details_header = GHOST_ESP_UI_TEXT("WiFi Printer", "WiFi 打印机"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Control power state\n"
            "of network printers.\n"
            "Configure in WebUI:\n"
            "- Printer IP/Port\n"
            "- Protocol type\n\n",
            "控制网络打印机的\n"
            "电源状态。\n"
            "在 WebUI 中配置:\n"
            "- 打印机 IP/端口\n"
            "- 协议类型\n\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Scan Local Network", "扫描本地网络"),
        .command = "scanlocal\n",
        .needs_confirmation = true,
        .confirm_header = GHOST_ESP_UI_TEXT("Local Network Scan", "本地网络扫描"),
        .confirm_text = GHOST_ESP_UI_TEXT(
            "Make sure you've connected\nto WiFi first via "
            "the\n'Connect to WiFi' option.\n",
            "请确保已通过\n'连接 WiFi' 选项\n连接到 WiFi。\n"),
        .details_header = GHOST_ESP_UI_TEXT("Network Scanner", "网络扫描"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Scans local network for:\n"
            "- Printers\n"
            "- Smart devices\n"
            "- Cast devices\n"
            "- Requires WiFi connection\n\n",
            "扫描本地网络中的:\n"
            "- 打印机\n"
            "- 智能设备\n"
            "- Cast 设备\n"
            "- 需要 WiFi 连接\n\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Set WebUI Creds", "设置 WebUI 凭据"),
        .command = "apcred",
        .needs_input = true,
        .input_text = "MySSID MyPassword",
        .details_header = GHOST_ESP_UI_TEXT("Set AP Credentials", "设置 AP 凭据"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set custom WebUI AP:\n"
            "Format:\nMySSID MyPassword\n"
            "Example: GhostNet,spooky123\n",
            "设置自定义 WebUI AP:\n"
            "格式:\nMySSID MyPassword\n"
            "例: GhostNet,spooky123\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Reset WebUI Creds", "重置 WebUI 凭据"),
        .command = "apcred -r\n",
        .needs_confirmation = true,
        .confirm_header = GHOST_ESP_UI_TEXT("Reset AP Credentials", "重置 AP 凭据"),
        .confirm_text = GHOST_ESP_UI_TEXT(
            "Reset WebUI AP to\n"
            "default credentials?\n"
            "SSID: GhostNet\n"
            "Password: GhostNet\n",
            "将 WebUI AP 恢复为\n"
            "默认凭据？\n"
            "SSID: GhostNet\n"
            "密码: GhostNet\n"),
        .details_header = GHOST_ESP_UI_TEXT("Reset AP Credentials", "重置 AP 凭据"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Restores default WebUI AP:\n"
            "SSID: GhostNet\n"
            "Password: GhostNet\n"
            "Requires ESP reboot\n",
            "恢复默认 WebUI AP:\n"
            "SSID: GhostNet\n"
            "密码: GhostNet\n"
            "需要重启 ESP\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop Evil Portal", "停止 Evil Portal"),
        .command = "stopportal\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop Evil Portal", "停止 Evil Portal"),
        .details_text = GHOST_ESP_UI_TEXT("Stops the Evil Portal.", "停止 Evil Portal。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("TP-Link Smart Plug", "TP-Link 智能插座"),
        .command = "tplinktest",
        .needs_input = true,
        .input_text = "on | off | loop",
        .details_header = GHOST_ESP_UI_TEXT("TP-Link Control", "TP-Link 控制"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Control TP-Link smart plugs\n"
            "on the local network.",
            "控制本地网络上的\n"
            "TP-Link 智能插座。"),
    },
};

static const MenuCommand wifi_settings_commands[] = {
    {
        .label = GHOST_ESP_UI_TEXT("< LED: Rainbow >", "< LED: 彩虹 >"),
        .command = "rgbmode rainbow\n",
        .confirm_header = GHOST_ESP_UI_TEXT("LED Effects", "LED 效果"),
        .details_header = GHOST_ESP_UI_TEXT("LED Effects", "LED 效果"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Control LED effects:\n"
            "- rainbow, police, strobe, off, or fixed colors\n"
            "Cycle with Left/Right to select an effect\n",
            "控制 LED 效果:\n"
            "- 彩虹、警灯、频闪、关闭或固定颜色\n"
            "左右键切换效果\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Set RGB Pins", "设置 RGB 引脚"),
        .command = "setrgbpins",
        .needs_input = true,
        .input_text = "<red> <green> <blue>",
        .details_header = GHOST_ESP_UI_TEXT("Set RGB Pins", "设置 RGB 引脚"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Change RGB LED pins.\n"
            "Requires restart.\n"
            "Use same value for all\n"
            "pins for single-pin LED.",
            "更改 RGB LED 引脚。\n"
            "需要重启。\n"
            "单引脚 LED 使用\n"
            "相同值。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Chip Info", "芯片信息"),
        .command = "chipinfo\n",
        .details_header = GHOST_ESP_UI_TEXT("Chip Info", "芯片信息"),
        .details_text = GHOST_ESP_UI_TEXT("Displays chip information from the ESP\n", "显示 ESP 芯片信息\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Show SD Pin Config", "显示 SD 引脚配置"),
        .command = "sd_config",
        .details_header = GHOST_ESP_UI_TEXT("SD Pin Config", "SD 引脚配置"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Show current SD GPIO\n"
            "pin configuration for\n"
            "MMC and SPI modes.",
            "显示当前 SD GPIO\n"
            "引脚配置 (MMC 和 SPI 模式)。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Set SD Pins (MMC)", "设置 SD 引脚 (MMC)"),
        .command = "sd_pins_mmc",
        .needs_input = true,
        .input_text = "<clk> <cmd> <d0..d3>",
        .details_header = GHOST_ESP_UI_TEXT("Set SD Pins (MMC)", "设置 SD 引脚 (MMC)"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set GPIO pins for SDMMC.\n"
            "Requires restart.\n"
            "Only if firmware built\n"
            "for SDMMC mode.",
            "设置 SDMMC 的 GPIO 引脚。\n"
            "需要重启。\n"
            "仅适用于 SDMMC 模式\n"
            "编译的固件。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Set SD Pins (SPI)", "设置 SD 引脚 (SPI)"),
        .command = "sd_pins_spi",
        .needs_input = true,
        .input_text = "<cs> <clk> <miso> <mosi>",
        .details_header = GHOST_ESP_UI_TEXT("Set SD Pins (SPI)", "设置 SD 引脚 (SPI)"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set GPIO pins for SPI.\n"
            "Requires restart.\n"
            "Only if firmware built\n"
            "for SPI mode.",
            "设置 SPI 的 GPIO 引脚。\n"
            "需要重启。\n"
            "仅适用于 SPI 模式\n"
            "编译的固件。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Save SD Pin Config", "保存 SD 引脚配置"),
        .command = "sd_save_config",
        .needs_confirmation = true,
        .confirm_header = GHOST_ESP_UI_TEXT("Save SD Config", "保存 SD 配置"),
        .confirm_text = GHOST_ESP_UI_TEXT(
            "Save current SD pin\n"
            "config to SD card?\n"
            "Requires SD mounted.",
            "保存当前 SD 引脚\n"
            "配置到 SD 卡？\n"
            "需要 SD 卡已挂载。"),
        .details_header = GHOST_ESP_UI_TEXT("Save SD Pin Config", "保存 SD 引脚配置"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Save current SD pin\n"
            "config (both modes) to\n"
            "SD card (sd_config.conf).",
            "保存当前 SD 引脚配置\n"
            "(两种模式) 到 SD 卡\n"
            "(sd_config.conf)。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Set Timezone", "设置时区"),
        .command = "timezone",
        .needs_input = true,
        .input_text = "TZ String",
        .details_header = GHOST_ESP_UI_TEXT("Set Timezone", "设置时区"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set timezone for the clock.\n"
            "e.g. 'EST5EDT,M3.2.0,M11.1.0'",
            "设置时钟时区。\n"
            "例: 'EST5EDT,M3.2.0,M11.1.0'"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Set Web Auth", "设置 Web 认证"),
        .command = "webauth",
        .needs_input = true,
        .input_text = "on | off",
        .details_header = GHOST_ESP_UI_TEXT("Set Web Auth", "设置 Web 认证"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Enable or disable Web\n"
            "UI authentication.",
            "启用或禁用 Web UI 认证。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Set WiFi Country", "设置 WiFi 国家"),
        .command = "setcountry",
        .needs_input = true,
        .input_text = GHOST_ESP_UI_TEXT("Country Code (e.g. US)", "国家代码 (如 CN)"),
        .details_header = GHOST_ESP_UI_TEXT("Set WiFi Country", "设置 WiFi 国家"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set the WiFi country code.\n"
            "May require ESP32-C5.",
            "设置 WiFi 国家代码。\n"
            "可能需要 ESP32-C5。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Set RGB Profile", "设置 RGB 配置"),
        .command = "setrgbmode",
        .needs_input = true,
        .input_text = "normal|rainbow|stealth",
        .details_header = GHOST_ESP_UI_TEXT("Set RGB Profile", "设置 RGB 配置"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Save the default LED mode\n"
            "used after reboot.",
            "保存重启后使用的\n"
            "默认 LED 模式。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Set NeoPixel Brightness", "设置 NeoPixel 亮度"),
        .command = "setneopixelbrightness",
        .needs_input = true,
        .input_text = "0-100",
        .details_header = GHOST_ESP_UI_TEXT("NeoPixel Brightness", "NeoPixel 亮度"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Adjust NeoPixel brightness\n"
            "from 0 to 100%.",
            "调整 NeoPixel 亮度\n"
            "范围 0 到 100%。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Get NeoPixel Brightness", "获取 NeoPixel 亮度"),
        .command = "getneopixelbrightness\n",
        .details_header = GHOST_ESP_UI_TEXT("NeoPixel Brightness", "NeoPixel 亮度"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Displays the current NeoPixel\n"
            "brightness level.",
            "显示当前 NeoPixel 亮度。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Set RGB LED Count", "设置 RGB LED 数量"),
        .command = "setrgbcount",
        .needs_input = true,
        .input_text = "1-512",
        .details_header = GHOST_ESP_UI_TEXT("Set RGB LED Count", "设置 RGB LED 数量"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set the number of RGB LEDs\n"
            "connected (1-512).\n"
            "Effects will span the correct\n"
            "length. Reinitializes if pins\n"
            "are already configured.\n",
            "设置连接的 RGB LED 数量 (1-512)。\n"
            "效果将覆盖正确长度。\n"
            "如引脚已配置则重新初始化。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Settings List", "设置列表"),
        .command = "settings list\n",
        .details_header = GHOST_ESP_UI_TEXT("List Settings", "设置列表"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Shows available configuration\n"
            "keys and descriptions.",
            "显示可用的配置键和说明。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Settings Help", "设置帮助"),
        .command = "settings help\n",
        .details_header = GHOST_ESP_UI_TEXT("Settings Help", "设置帮助"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Displays CLI usage for\n"
            "settings commands.",
            "显示设置命令的 CLI 用法。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Settings Get", "获取设置"),
        .command = "settings get",
        .needs_input = true,
        .input_text = "Key",
        .details_header = GHOST_ESP_UI_TEXT("Get Setting", "获取设置"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Read the current value for\n"
            "a configuration key.",
            "读取配置键的当前值。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Settings Set", "设置值"),
        .command = "settings set",
        .needs_input = true,
        .input_text = "Key Value",
        .details_header = GHOST_ESP_UI_TEXT("Set Setting", "设置值"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Update a configuration key\n"
            "with a new value.",
            "更新配置键的值。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Settings Reset (Key)", "重置设置 (键)"),
        .command = "settings reset",
        .needs_input = true,
        .input_text = "Key",
        .details_header = GHOST_ESP_UI_TEXT("Reset Setting", "重置设置"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Reset a specific configuration\n"
            "key to defaults.",
            "将指定配置键恢复默认值。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Settings Reset (All)", "重置所有设置"),
        .command = "settings reset\n",
        .details_header = GHOST_ESP_UI_TEXT("Reset Settings", "重置设置"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Restore all configuration\n"
            "keys to defaults.",
            "将所有配置键恢复默认值。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Show Help", "显示帮助"),
        .command = "help\n",
        .details_header = GHOST_ESP_UI_TEXT("Help", "帮助"),
        .details_text = GHOST_ESP_UI_TEXT("Show complete command list.", "显示完整命令列表。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Reboot Device", "重启设备"),
        .command = "reboot\n",
        .needs_confirmation = true,
        .confirm_header = GHOST_ESP_UI_TEXT("Reboot Device", "重启设备"),
        .confirm_text = GHOST_ESP_UI_TEXT("Are you sure you want to reboot?", "确定要重启吗？"),
        .details_header = GHOST_ESP_UI_TEXT("Reboot", "重启"),
        .details_text = GHOST_ESP_UI_TEXT("Restart the ESP device.", "重启 ESP 设备。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Enable/Disable AP", "启用/禁用 AP"),
        .command = "apenable",
        .needs_input = true,
        .input_text = "on | off",
        .details_header = GHOST_ESP_UI_TEXT("AP Enable/Disable", "AP 启用/禁用"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Enable or disable the Access Point\nacross reboots.",
            "启用或禁用 AP (重启后保持)。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Show Chip Info", "显示芯片信息"),
        .command = "chipinfo\n",
        .details_header = GHOST_ESP_UI_TEXT("Chip Info", "芯片信息"),
        .details_text = GHOST_ESP_UI_TEXT("Show chip and memory info.", "显示芯片和内存信息。"),
    },
};

static const MenuCommand wifi_stop_command = {
    .label = GHOST_ESP_UI_TEXT("Stop All WiFi", "停止所有 WiFi"),
    .command = "stop\n",
    .details_header = GHOST_ESP_UI_TEXT("Stop WiFi Operations", "停止 WiFi 操作"),
    .details_text = GHOST_ESP_UI_TEXT(
        "Stops all active WiFi\n"
        "operations including:\n"
        "- Scanning\n"
        "- Beacon Spam\n"
        "- Deauth Attacks\n"
        "- Packet Captures\n"
        "- Evil Portal\n",
        "停止所有 WiFi 操作:\n"
        "- 扫描\n"
        "- 信标洪流\n"
        "- 断连攻击\n"
        "- 数据包捕获\n"
        "- Evil Portal\n"),
};

static const MenuCommand status_idle_commands[] = {
    {
        .label = GHOST_ESP_UI_TEXT("Life (Game of Life)", "生命游戏"),
        .command = "statusidle set life\n",
        .details_header = GHOST_ESP_UI_TEXT("Life Animation", "生命游戏动画"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set idle status display to\n"
            "Game of Life animation.",
            "设置待机显示为\n"
            "生命游戏动画。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Ghost (Sprite)", "幽灵"),
        .command = "statusidle set ghost\n",
        .details_header = GHOST_ESP_UI_TEXT("Ghost Animation", "幽灵动画"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set idle status display to\n"
            "ghost sprite animation.",
            "设置待机显示为\n"
            "幽灵精灵动画。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Starfield", "星空"),
        .command = "statusidle set starfield\n",
        .details_header = GHOST_ESP_UI_TEXT("Starfield Animation", "星空动画"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set idle status display to\n"
            "starfield effect.",
            "设置待机显示为\n"
            "星空效果。"),
    },
    {
        .label = "HUD",
        .command = "statusidle set hud\n",
        .details_header = GHOST_ESP_UI_TEXT("HUD Animation", "HUD 动画"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set idle status display to\n"
            "HUD-style overlay.",
            "设置待机显示为\n"
            "HUD 风格叠加。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Matrix", "矩阵"),
        .command = "statusidle set matrix\n",
        .details_header = GHOST_ESP_UI_TEXT("Matrix Animation", "矩阵动画"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set idle status display to\n"
            "Matrix-style rain effect.",
            "设置待机显示为\n"
            "矩阵雨效果。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Multiple Ghosts", "多幽灵"),
        .command = "statusidle set ghosts\n",
        .details_header = GHOST_ESP_UI_TEXT("Ghosts Animation", "幽灵动画"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set idle status display to\n"
            "floating ghosts effect.",
            "设置待机显示为\n"
            "漂浮幽灵效果。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Spiral", "螺旋"),
        .command = "statusidle set spiral\n",
        .details_header = GHOST_ESP_UI_TEXT("Spiral Animation", "螺旋动画"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set idle status display to\n"
            "spiral pattern effect.",
            "设置待机显示为\n"
            "螺旋图案效果。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Falling Leaves", "落叶"),
        .command = "statusidle set leaves\n",
        .details_header = GHOST_ESP_UI_TEXT("Falling Leaves Animation", "落叶动画"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set idle status display to\n"
            "falling leaves effect.",
            "设置待机显示为\n"
            "落叶效果。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Bouncing Text", "弹跳文字"),
        .command = "statusidle set bouncing\n",
        .details_header = GHOST_ESP_UI_TEXT("Bouncing Text Animation", "弹跳文字动画"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set idle status display to\n"
            "bouncing text effect.",
            "设置待机显示为\n"
            "弹跳文字效果。"),
    },
};

// BLE menu command definitions
static const MenuCommand ble_scanning_commands[] = {
    {
        .label = GHOST_ESP_UI_TEXT("Skimmer Detection", "刷卡器检测"),
        .command = "capture -skimmer\n",
        .capture_prefix = "skimmer_scan",
        .file_ext = "pcap",
        .folder = GHOST_ESP_APP_FOLDER_PCAPS,
        .details_header = GHOST_ESP_UI_TEXT("Skimmer Scanner", "刷卡器扫描"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Detects potential\n"
            "card skimmers by\n"
            "analyzing BLE\n"
            "signatures and\n"
            "known patterns.\n",
            "通过分析 BLE 签名\n"
            "和已知模式检测\n"
            "潜在的银行卡\n"
            "刷卡器。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Find the Flippers", "查找 Flipper"),
        .command = "blescan -f\n",
        .details_header = GHOST_ESP_UI_TEXT("Flipper Scanner", "Flipper 扫描"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Scans for Flippers:\n"
            "- Device name\n"
            "- BT address\n"
            "- Signal level\n"
            "Range: ~50m\n",
            "扫描 Flipper 设备:\n"
            "- 设备名称\n"
            "- 蓝牙地址\n"
            "- 信号强度\n"
            "范围: ~50m\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("AirTag Scanner", "AirTag 扫描"),
        .command = "blescan -a\n",
        .details_header = GHOST_ESP_UI_TEXT("AirTag Scanner", "AirTag 扫描"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Detects nearby Apple\n"
            "AirTags and shows:\n"
            "- Device ID\n"
            "- Signal strength\n"
            "- Last seen time\n",
            "检测附近的 Apple\n"
            "AirTag 并显示:\n"
            "- 设备 ID\n"
            "- 信号强度\n"
            "- 最后发现时间\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("List AirTags", "AirTag 列表"),
        .command = "listairtags\n",
        .details_header = GHOST_ESP_UI_TEXT("List AirTags", "AirTag 列表"),
        .details_text = GHOST_ESP_UI_TEXT("List discovered AirTags.", "列出已发现的 AirTag。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Select AirTag", "选择 AirTag"),
        .command = "select -airtag",
        .needs_input = true,
        .input_text = GHOST_ESP_UI_TEXT("AirTag Number", "AirTag 编号"),
        .details_header = GHOST_ESP_UI_TEXT("Select AirTag", "选择 AirTag"),
        .details_text = GHOST_ESP_UI_TEXT("Target an AirTag by number\nfrom the scan list.", "从扫描列表中按编号选择 AirTag。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("List Flippers", "Flipper 列表"),
        .command = "listflippers\n",
        .details_header = GHOST_ESP_UI_TEXT("List Flippers", "Flipper 列表"),
        .details_text = GHOST_ESP_UI_TEXT(
            "List discovered Flipper Devices\n"
            "in range.",
            "列出范围内发现的 Flipper 设备。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Select Flipper to Track", "选择 Flipper 追踪"),
        .command = "selectflipper",
        .needs_input = true,
        .input_text = GHOST_ESP_UI_TEXT("Flipper Number", "Flipper 编号"),
        .details_header = GHOST_ESP_UI_TEXT("Select Flipper to Track", "选择 Flipper 追踪"),
        .details_text = GHOST_ESP_UI_TEXT("Select a Flipper by number to track RSSI strength.", "按编号选择 Flipper 追踪 RSSI 信号强度。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Scan GATT Devices", "扫描 GATT 设备"),
        .command = "blescan -g\n",
        .details_header = GHOST_ESP_UI_TEXT("GATT Device Scanner", "GATT 设备扫描"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Scan for connectable BLE\n"
            "devices for GATT enumeration.\n"
            "Shows device addresses and\n"
            "connection capability.\n",
            "扫描可连接的 BLE 设备\n"
            "进行 GATT 枚举。\n"
            "显示设备地址和\n"
            "连接能力。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("List GATT Devices", "GATT 设备列表"),
        .command = "listgatt\n",
        .details_header = GHOST_ESP_UI_TEXT("List GATT Devices", "GATT 设备列表"),
        .details_text = GHOST_ESP_UI_TEXT(
            "List discovered GATT devices\n"
            "with tracker type detection.\n",
            "列出已发现的 GATT 设备\n"
            "并检测追踪器类型。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Select GATT Device", "选择 GATT 设备"),
        .command = "selectgatt",
        .needs_input = true,
        .input_text = GHOST_ESP_UI_TEXT("Device Index", "设备索引"),
        .details_header = GHOST_ESP_UI_TEXT("Select GATT Device", "选择 GATT 设备"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Select a GATT device by index\n"
            "for enumeration or tracking.\n",
            "按索引选择 GATT 设备\n"
            "进行枚举或追踪。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Enumerate GATT Services", "枚举 GATT 服务"),
        .command = "enumgatt\n",
        .details_header = GHOST_ESP_UI_TEXT("Enumerate GATT", "枚举 GATT"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Connect to selected device\n"
            "and enumerate its GATT\n"
            "services, characteristics,\n"
            "and descriptors.\n",
            "连接到选定设备并\n"
            "枚举其 GATT 服务、\n"
            "特征和描述符。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Track GATT Device", "追踪 GATT 设备"),
        .command = "trackgatt\n",
        .details_header = GHOST_ESP_UI_TEXT("Track GATT Device", "追踪 GATT 设备"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Track selected GATT device\n"
            "using real-time RSSI signal\n"
            "strength monitoring.\n",
            "使用实时 RSSI 信号\n"
            "强度监测追踪选定的\n"
            "GATT 设备。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("View All BLE Traffic", "查看所有 BLE 流量"),
        .command = "blescan -r\n",
        .details_header = GHOST_ESP_UI_TEXT("BLE Raw Traffic", "BLE 原始流量"),
        .details_text = GHOST_ESP_UI_TEXT("View all Bluetooth Low Energy\ntraffic in range.", "查看范围内所有 BLE 流量。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop BLE Scanning", "停止 BLE 扫描"),
        .command = "blescan -s\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop BLE Scan", "停止 BLE 扫描"),
        .details_text = GHOST_ESP_UI_TEXT("Stops any active BLE scanning.", "停止所有 BLE 扫描。"),
    },
};

static const MenuCommand ble_capture_commands[] = {
    {
        .label = GHOST_ESP_UI_TEXT("BLE Raw Capture", "BLE 原始捕获"),
        .command = "capture -ble\n",
        .capture_prefix = "ble_raw_capture",
        .file_ext = "pcap",
        .folder = GHOST_ESP_APP_FOLDER_PCAPS,
        .details_header = GHOST_ESP_UI_TEXT("BLE Raw Capture", "BLE 原始捕获"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Captures raw BLE\n"
            "traffic and data.\n"
            "Range: ~10-30m\n",
            "捕获 BLE 原始\n"
            "流量和数据。\n"
            "范围: ~10-30m\n"),
    },
};

static const MenuCommand ble_attack_commands[] = {
    {
        .label = GHOST_ESP_UI_TEXT("< BLE Spam (Apple) >", "< BLE 洪流 (Apple) >"),
        .command = "blespam -apple\n",
        .details_header = GHOST_ESP_UI_TEXT("Variable BLE Spam", "切换 BLE 洪流模式"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Use Left/Right to change:\n"
            "- Apple device spam\n"
            "- Microsoft Swift Pair\n"
            "- Samsung Galaxy Watch\n"
            "- Google Fast Pair\n"
            "- Random spam (all types)\n"
            "Range: ~50m\n",
            "左右键切换模式:\n"
            "- Apple 设备广播\n"
            "- Microsoft Swift Pair\n"
            "- Samsung Galaxy Watch\n"
            "- Google Fast Pair\n"
            "- 随机广播 (全部类型)\n"
            "范围: ~50m\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Spoof Selected AirTag", "伪装选定 AirTag"),
        .command = "spoofairtag\n",
        .details_header = GHOST_ESP_UI_TEXT("Spoof AirTag", "伪装 AirTag"),
        .details_text = GHOST_ESP_UI_TEXT("Spoof the selected AirTag.", "伪装选定的 AirTag。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop BLE Spam", "停止 BLE 洪流"),
        .command = "blespam -s\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop BLE Spam", "停止 BLE 洪流"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Stops BLE advertisement\n"
            "spam attacks.",
            "停止 BLE 广播洪流攻击。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop AirTag Spoof", "停止 AirTag 伪装"),
        .command = "stopspoof\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop Spoofing", "停止伪装"),
        .details_text = GHOST_ESP_UI_TEXT("Stops AirTag spoofing.", "停止 AirTag 伪装。"),
    },
};

static const MenuCommand ble_stop_command = {
    .label = GHOST_ESP_UI_TEXT("Stop All BLE", "停止所有 BLE"),
    .command = "stop\n",
    .details_header = GHOST_ESP_UI_TEXT("Stop BLE Operations", "停止 BLE 操作"),
    .details_text = GHOST_ESP_UI_TEXT(
        "Stops all active BLE\n"
        "operations including:\n"
        "- BLE Scanning\n"
        "- Skimmer Detection\n"
        "- Packet Captures\n"
        "- Device Detection\n",
        "停止所有 BLE 操作:\n"
        "- BLE 扫描\n"
        "- 刷卡器检测\n"
        "- 数据包捕获\n"
        "- 设备检测\n"),
};

// GPS menu command definitions
static const MenuCommand gps_commands[] = {
    {
        .label = GHOST_ESP_UI_TEXT("GPS Info", "GPS 信息"),
        .command = "gpsinfo\n",
        .details_header = GHOST_ESP_UI_TEXT("GPS Information", "GPS 信息"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Shows GPS details:\n"
            "- Position (Lat/Long)\n"
            "- Altitude & Speed\n"
            "- Direction & Quality\n"
            "- Satellite Status\n",
            "显示 GPS 详情:\n"
            "- 位置 (经纬度)\n"
            "- 海拔和速度\n"
            "- 方向和质量\n"
            "- 卫星状态\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Set GPS Pin", "设置 GPS 引脚"),
        .command = "gpspin",
        .needs_input = true,
        .input_text = GHOST_ESP_UI_TEXT("Pin Number", "引脚编号"),
        .details_header = GHOST_ESP_UI_TEXT("Set GPS RX Pin", "设置 GPS RX 引脚"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Set the GPS RX pin for\n"
            "external GPS modules.\n"
            "Setting persists to NVS.\n"
            "Restart GPS commands to apply.\n",
            "设置外部 GPS 模块的\n"
            "GPS RX 引脚。\n"
            "设置保存到 NVS。\n"
            "重启 GPS 命令生效。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("View GPS Pin", "查看 GPS 引脚"),
        .command = "gpspin\n",
        .details_header = GHOST_ESP_UI_TEXT("View GPS RX Pin", "查看 GPS RX 引脚"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Shows current GPS RX pin\n"
            "configuration for external\n"
            "GPS modules.\n",
            "显示外部 GPS 模块的\n"
            "当前 GPS RX 引脚配置。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Start Wardriving", "开始 WiFi Wardrive"),
        .command = "startwd\n",
        .capture_prefix = "wardrive_wifi",
        .file_ext = "csv",
        .folder = GHOST_ESP_APP_FOLDER_WARDRIVE,
        .details_header = GHOST_ESP_UI_TEXT("Wardrive Mode", "Wardrive 模式"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Maps WiFi networks:\n"
            "- Network info\n"
            "- GPS location\n"
            "- Signal levels\n"
            "Saves as CSV\n",
            "绘制 WiFi 网络地图:\n"
            "- 网络信息\n"
            "- GPS 位置\n"
            "- 信号强度\n"
            "保存为 CSV\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("BLE Wardriving", "BLE Wardrive"),
        .command = "blewardriving\n",
        .capture_prefix = "wardrive_ble",
        .file_ext = "csv",
        .folder = GHOST_ESP_APP_FOLDER_WARDRIVE,
        .details_header = GHOST_ESP_UI_TEXT("BLE Wardriving", "BLE Wardrive"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Maps BLE devices:\n"
            "- Device info\n"
            "- GPS location\n"
            "- Signal levels\n"
            "Saves as CSV\n",
            "绘制 BLE 设备地图:\n"
            "- 设备信息\n"
            "- GPS 位置\n"
            "- 信号强度\n"
            "保存为 CSV\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop BLE Wardriving", "停止 BLE Wardrive"),
        .command = "blewardriving -s\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop BLE Wardrive", "停止 BLE Wardrive"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Stops BLE wardriving capture\n"
            "and logging.",
            "停止 BLE Wardrive 捕获和记录。"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop All GPS", "停止所有 GPS"),
        .command = "stop\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop GPS Operations", "停止 GPS 操作"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Stops all active GPS\n"
            "operations including:\n"
            "- GPS Info Updates\n"
            "- WiFi Wardriving\n"
            "- BLE Wardriving\n",
            "停止所有 GPS 操作:\n"
            "- GPS 信息更新\n"
            "- WiFi Wardrive\n"
            "- BLE Wardrive\n"),
    },
};

// Aerial Detector menu command definitions - all in one menu
static const MenuCommand aerial_commands[] = {
    {
        .label = GHOST_ESP_UI_TEXT("Start Scan (30s)", "开始扫描 (30秒)"),
        .command = "aerialscan 30\n",
        .details_header = GHOST_ESP_UI_TEXT("Scan for Drones", "扫描无人机"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Scans for aerial devices:\n"
            "- OpenDroneID (WiFi/BLE)\n"
            "- DJI drones\n"
            "- Drone networks\n"
            "Phase 1: WiFi (all channels)\n"
            "Phase 2: BLE\n"
            "Duration: 30 seconds\n",
            "扫描飞行设备:\n"
            "- OpenDroneID (WiFi/BLE)\n"
            "- DJI 无人机\n"
            "- 无人机网络\n"
            "阶段1: WiFi (全信道)\n"
            "阶段2: BLE\n"
            "持续时间: 30 秒\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Quick Scan (15s)", "快速扫描 (15秒)"),
        .command = "aerialscan 15\n",
        .details_header = GHOST_ESP_UI_TEXT("Quick Scan", "快速扫描"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Fast 15 second scan for\n"
            "nearby aerial devices.\n",
            "快速扫描 15 秒\n"
            "查找附近飞行设备。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Extended Scan (60s)", "扩展扫描 (60秒)"),
        .command = "aerialscan 60\n",
        .details_header = GHOST_ESP_UI_TEXT("Extended Scan", "扩展扫描"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Extended 60 second scan\n"
            "for maximum coverage.\n",
            "扩展扫描 60 秒\n"
            "最大覆盖范围。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("List Detected Drones", "列出检测到的无人机"),
        .command = "aeriallist\n",
        .details_header = GHOST_ESP_UI_TEXT("Detected Devices", "检测到的设备"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Lists all detected aerial\n"
            "devices with:\n"
            "- Device ID & Type\n"
            "- GPS coordinates\n"
            "- Altitude & Speed\n"
            "- Operator location\n"
            "- RSSI signal\n",
            "列出所有检测到的飞行设备:\n"
            "- 设备 ID 和类型\n"
            "- GPS 坐标\n"
            "- 海拔和速度\n"
            "- 操作者位置\n"
            "- RSSI 信号\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Track Drone by Index", "按索引追踪无人机"),
        .command = "aerialtrack",
        .needs_input = true,
        .input_text = GHOST_ESP_UI_TEXT("Device Index", "设备索引"),
        .details_header = GHOST_ESP_UI_TEXT("Track Drone", "追踪无人机"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Track specific drone by\n"
            "index from aeriallist.\n"
            "Shows real-time updates\n"
            "for selected device.\n",
            "按索引追踪特定无人机。\n"
            "显示选定设备的\n"
            "实时更新。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Track Drone by MAC", "按 MAC 追踪无人机"),
        .command = "aerialtrack",
        .needs_input = true,
        .input_text = "MAC Address",
        .details_header = GHOST_ESP_UI_TEXT("Track by MAC", "按 MAC 追踪"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Track specific drone by\n"
            "MAC address.\n"
            "Format: aa:bb:cc:dd:ee:ff\n",
            "按 MAC 地址追踪\n"
            "特定无人机。\n"
            "格式: aa:bb:cc:dd:ee:ff\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Spoof Test Drone", "测试伪装无人机"),
        .command = "aerialspoof\n",
        .details_header = GHOST_ESP_UI_TEXT("Test Spoof", "测试伪装"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Broadcasts test RemoteID:\n"
            "ID: GHOST-TEST\n"
            "Location: San Francisco\n"
            "Altitude: 100m\n"
            "Status: Airborne\n\n"
            "Note: WiFi suspended\n"
            "during BLE broadcast\n",
            "广播测试 RemoteID:\n"
            "ID: GHOST-TEST\n"
            "位置: San Francisco\n"
            "海拔: 100m\n"
            "状态: 飞行中\n\n"
            "注意: BLE 广播期间\n"
            "WiFi 暂停\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Custom Spoof", "自定义伪装"),
        .command = "aerialspoof",
        .needs_input = true,
        .input_text = "ID Lat Lon Alt",
        .details_header = GHOST_ESP_UI_TEXT("Custom Spoof", "自定义伪装"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Broadcast custom RemoteID.\n"
            "Format:\n"
            "DRONE-ID lat lon alt\n\n"
            "Example:\n"
            "GHOST-1 40.7128 -74.0060 100\n",
            "广播自定义 RemoteID。\n"
            "格式:\n"
            "DRONE-ID 纬度 经度 海拔\n\n"
            "例:\n"
            "GHOST-1 40.7128 -74.0060 100\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop Spoofing", "停止伪装"),
        .command = "aerialspoofstop\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop Spoofing", "停止伪装"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Stops RemoteID broadcast\n"
            "and restores WiFi.\n",
            "停止 RemoteID 广播\n"
            "并恢复 WiFi。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop All", "全部停止"),
        .command = "aerialstop\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop All Operations", "停止所有操作"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Stops all active aerial\n"
            "operations including:\n"
            "- Scanning\n"
            "- Tracking\n"
            "- Spoofing\n",
            "停止所有飞行操作:\n"
            "- 扫描\n"
            "- 追踪\n"
            "- 伪装\n"),
    },
};

// IR menu command definitions
static const MenuCommand ir_commands[] = {
    {
        .label = GHOST_ESP_UI_TEXT("Browse IR Remotes", "浏览红外遥控器"),
        .command = "ir list\n",
        .details_header = GHOST_ESP_UI_TEXT("Browse IR Remotes", "浏览红外遥控器"),
        .details_text = GHOST_ESP_UI_TEXT("Browse IR remotes on ESP\n", "浏览 ESP 上的红外遥控器\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Browse Universals", "浏览万能遥控"),
        .command = "ir universals list\n",
        .details_header = GHOST_ESP_UI_TEXT("Browse Universals", "浏览万能遥控"),
        .details_text = GHOST_ESP_UI_TEXT("Browse built-in universal IR\n", "浏览内置万能红外遥控\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Send from Flipper", "从 Flipper 发送"),
        .command = "send_ir_file",
        .details_header = GHOST_ESP_UI_TEXT("Send from Flipper", "从 Flipper 发送"),
        .details_text = GHOST_ESP_UI_TEXT("Browse Flipper IR files and\nsend signals to ESP.\n", "浏览 Flipper 红外文件\n并发送信号到 ESP。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("IR Learn (Auto File)", "红外学习 (自动文件)"),
        .command = "ir learn\n",
        .details_header = GHOST_ESP_UI_TEXT("Learn IR (Auto)", "红外学习 (自动)"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Capture IR signal (10s wait).\n"
            "Auto-create a new IR file.\n",
            "捕获红外信号 (等待10秒)。\n"
            "自动创建新的红外文件。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("IR Learn (Path)", "红外学习 (指定路径)"),
        .command = "ir learn",
        .needs_input = true,
        .input_text = GHOST_ESP_UI_TEXT("Path (optional)", "路径 (可选)"),
        .details_header = GHOST_ESP_UI_TEXT("Learn IR (Path)", "红外学习 (路径)"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Capture IR signal (10s wait).\n"
            "Leave blank to auto-create,\n"
            "or specify path to append.\n",
            "捕获红外信号 (等待10秒)。\n"
            "留空自动创建，\n"
            "或指定路径追加。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("IR Receive", "红外接收"),
        .command = "ir rx",
        .needs_input = true,
        .input_text = GHOST_ESP_UI_TEXT("Timeout (default 60)", "超时 (默认60)"),
        .details_header = GHOST_ESP_UI_TEXT("Receive IR", "红外接收"),
        .details_text = GHOST_ESP_UI_TEXT(
            "Wait for single IR signal.\n"
            "Prints decoded or RAW data.\n",
            "等待单个红外信号。\n"
            "打印解码或原始数据。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("IR List Files (Raw)", "红外文件列表 (原始)"),
        .command = "ir list\n",
        .details_header = GHOST_ESP_UI_TEXT("List IR Files", "红外文件列表"),
        .details_text = GHOST_ESP_UI_TEXT(
            "List all .ir/.json files in\n"
            "remote directories.\n",
            "列出遥控器目录中的\n"
            "所有 .ir/.json 文件。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("IR Show File (Raw)", "红外文件内容 (原始)"),
        .command = "ir show",
        .needs_input = true,
        .input_text = GHOST_ESP_UI_TEXT("Path or Index", "路径或索引"),
        .details_header = GHOST_ESP_UI_TEXT("Show IR File", "显示红外文件"),
        .details_text = GHOST_ESP_UI_TEXT("Display signals from an IR file.\n", "显示红外文件中的信号。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("IR Send (Raw)", "红外发送 (原始)"),
        .command = "ir send",
        .needs_input = true,
        .input_text = GHOST_ESP_UI_TEXT("Index [Button]", "索引 [按键]"),
        .details_header = GHOST_ESP_UI_TEXT("Send IR (Raw)", "红外发送 (原始)"),
        .details_text = GHOST_ESP_UI_TEXT("Transmit using raw indices.\n", "使用原始索引发送。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("IR Dazzler Start", "红外眩目器启动"),
        .command = "ir dazzler\n",
        .details_header = GHOST_ESP_UI_TEXT("IR Dazzler Start", "红外眩目器启动"),
        .details_text = GHOST_ESP_UI_TEXT("Start continuous IR dazzler flood.\n", "启动持续红外眩目器。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("IR Dazzler Stop", "红外眩目器停止"),
        .command = "ir dazzler stop\n",
        .details_header = GHOST_ESP_UI_TEXT("IR Dazzler Stop", "红外眩目器停止"),
        .details_text = GHOST_ESP_UI_TEXT("Stop continuous IR dazzler flood.\n", "停止持续红外眩目器。\n"),
    },
    {
        .label = GHOST_ESP_UI_TEXT("Stop IR", "停止红外"),
        .command = "stop\n",
        .details_header = GHOST_ESP_UI_TEXT("Stop IR", "停止红外"),
        .details_text = GHOST_ESP_UI_TEXT("Stop all active IR operations.\n", "停止所有红外操作。\n"),
    },
};

#define IR_UART_PARSE_BUF_SIZE 1024

static char* next_line(char* buf, size_t* offset) {
    if(!buf || !offset) return NULL;
    char* p = buf + *offset;
    while(*p == '\r' || *p == '\n')
        p++;
    if(*p == '\0') return NULL;
    char* start = p;
    while(*p && *p != '\r' && *p != '\n')
        p++;
    if(*p) {
        *p++ = '\0';
    }
    *offset = (size_t)(p - buf);
    return start;
}

static bool ir_query_and_parse_list(AppState* state) {
    if(!state || !state->uart_context) return false;

    uart_reset_text_buffers(state->uart_context);
    send_uart_command("ir list\n", state);

    char buffer[IR_UART_PARSE_BUF_SIZE];

    size_t len = 0;
    uint32_t start = furi_get_tick();
    const uint32_t timeout_ms = 2000;
    while(furi_get_tick() - start < timeout_ms) {
        furi_delay_ms(100);
        if(uart_copy_text_buffer_tail(state->uart_context, buffer, IR_UART_PARSE_BUF_SIZE, &len) &&
           len > 0) {
            if(strstr(buffer, "IR files in ") || strstr(buffer, "(none)") ||
               strstr(buffer, "(none).") || strchr(buffer, '[')) {
                break;
            }
        }
    }

    if(len == 0) {
        return false;
    }

    state->ir_remote_count = 0;

    size_t pos = 0;
    char* line = NULL;
    while((line = next_line(buffer, &pos))) {
        while(*line == ' ' || *line == '\t')
            line++;
        if(strncmp(line, "IR files in ", 12) == 0) {
            continue;
        }
        if(strncmp(line, "(none).", 7) == 0 || strncmp(line, "(none)", 6) == 0) {
            continue;
        }
        if(line[0] == '[') {
            unsigned int idx = 0;
            char name[64] = {0};
            if(sscanf(line, "[%u] %63s", &idx, name) == 2) {
                if(state->ir_remote_count < COUNT_OF(state->ir_remotes)) {
                    IrRemoteEntry* e = &state->ir_remotes[state->ir_remote_count++];
                    e->index = idx;
                    strncpy(e->name, name, sizeof(e->name) - 1);
                    e->name[sizeof(e->name) - 1] = '\0';
                }
            }
        }
    }

    return state->ir_remote_count > 0;
}

static bool ir_query_and_parse_show(AppState* state, uint32_t remote_index) {
    if(!state || !state->uart_context) return false;

    uart_reset_text_buffers(state->uart_context);

    char cmd[32];
    snprintf(cmd, sizeof(cmd), "ir show %lu\n", (unsigned long)remote_index);
    send_uart_command(cmd, state);

    char buffer[IR_UART_PARSE_BUF_SIZE];

    size_t len = 0;
    uint32_t start = furi_get_tick();
    const uint32_t timeout_ms = 3000;
    while(furi_get_tick() - start < timeout_ms) {
        furi_delay_ms(100);
        if(uart_copy_text_buffer_tail(state->uart_context, buffer, IR_UART_PARSE_BUF_SIZE, &len) &&
           len > 0) {
            if(strstr(buffer, "Signals in ") || strstr(buffer, "Unique buttons in ") ||
               strchr(buffer, '[')) {
                break;
            }
        }
    }

    if(len == 0) {
        return false;
    }

    state->ir_signal_count = 0;

    size_t pos = 0;
    char* line = NULL;
    while((line = next_line(buffer, &pos))) {
        while(*line == ' ' || *line == '\t')
            line++;
        if(strncmp(line, "Signals in ", 11) == 0) {
            continue;
        }
        if(strncmp(line, "IR: ", 4) == 0) {
            continue;
        }
        if(line[0] == '[') {
            unsigned int idx = 0;
            char name[32] = {0};
            char proto[16] = {0};
            int n = sscanf(line, "[%u] %31s (%15[^)])", &idx, name, proto);
            if(n >= 2) {
                if(state->ir_signal_count < COUNT_OF(state->ir_signals)) {
                    IrSignalEntry* e = &state->ir_signals[state->ir_signal_count++];
                    e->index = idx;
                    strncpy(e->name, name, sizeof(e->name) - 1);
                    e->name[sizeof(e->name) - 1] = '\0';
                    if(n == 3 && proto[0]) {
                        strncpy(e->proto, proto, sizeof(e->proto) - 1);
                        e->proto[sizeof(e->proto) - 1] = '\0';
                    } else {
                        e->proto[0] = '\0';
                    }
                }
            }
        }
    }

    return state->ir_signal_count > 0;
}

// Stream/index .ir file without holding entire file in RAM
static bool ir_index_buttons_from_file(AppState* state) {
    if(!state || !state->ir_file_path[0]) return false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    bool ok = false;

    do {
        if(!storage_file_open(file, state->ir_file_path, FSAM_READ, FSOM_OPEN_EXISTING)) break;

        const size_t buf_size = 512;
        uint8_t buf[buf_size];
        size_t global_offset = 0;
        bool in_block = false;
        size_t block_start = 0;

        state->ir_signal_count = 0;

        while(true) {
            uint16_t read = storage_file_read(file, buf, buf_size);
            if(read == 0) break;

            size_t pos = 0;
            while(pos < read && state->ir_signal_count < COUNT_OF(state->ir_signals)) {
                // Consume whitespace
                while(pos < read && (buf[pos] == '\r' || buf[pos] == '\n' || buf[pos] == ' ' ||
                                     buf[pos] == '\t')) {
                    if(buf[pos] == '\n' || buf[pos] == '\r') {
                        if(in_block) {
                            // Potential end of block handled when we see next header
                        }
                    }
                    pos++;
                    global_offset++;
                }
                if(pos >= read) break;

                // Skip comments
                if(buf[pos] == '#') {
                    while(pos < read && buf[pos] != '\n' && buf[pos] != '\r') {
                        pos++;
                        global_offset++;
                    }
                    continue;
                }

                // Detect "name:" start
                const char name_hdr[] = "name:";
                if(read - pos >= sizeof(name_hdr) - 1 &&
                   memcmp(buf + pos, name_hdr, sizeof(name_hdr) - 1) == 0) {
                    // If we were already in a block, close it at current global_offset
                    if(in_block && state->ir_signal_count > 0) {
                        state->ir_signal_block_lengths[state->ir_signal_count - 1] =
                            (global_offset)-state
                                ->ir_signal_block_offsets[state->ir_signal_count - 1];
                    }

                    in_block = true;
                    block_start = global_offset;

                    // Parse name on this line to populate label
                    size_t line_end = pos;
                    while(line_end < read && buf[line_end] != '\n' && buf[line_end] != '\r')
                        line_end++;

                    size_t val_start = pos + (sizeof(name_hdr) - 1);
                    while(val_start < line_end &&
                          (buf[val_start] == ' ' || buf[val_start] == '\t')) {
                        val_start++;
                    }
                    size_t val_end = line_end;
                    while(val_end > val_start &&
                          (buf[val_end - 1] == ' ' || buf[val_end - 1] == '\t')) {
                        val_end--;
                    }

                    if(state->ir_signal_count < COUNT_OF(state->ir_signals)) {
                        IrSignalEntry* e = &state->ir_signals[state->ir_signal_count];
                        size_t name_len = (val_end > val_start) ? (val_end - val_start) : 0;
                        if(name_len >= sizeof(e->name)) name_len = sizeof(e->name) - 1;
                        if(name_len > 0) {
                            memcpy(e->name, buf + val_start, name_len);
                            e->name[name_len] = '\0';
                        } else {
                            e->name[0] = '\0';
                        }
                        e->index = state->ir_signal_count; // use slot index
                        e->proto[0] = '\0';

                        state->ir_signal_block_offsets[state->ir_signal_count] = block_start;
                        state->ir_signal_block_lengths[state->ir_signal_count] = 0; // temp
                        state->ir_signal_count++;
                    }

                    global_offset += (line_end - pos);
                    pos = line_end;
                    continue;
                }

                // Detect end of block by seeing next header in subsequent iterations
                // Consume rest of line
                while(pos < read && buf[pos] != '\n' && buf[pos] != '\r') {
                    pos++;
                    global_offset++;
                }
            }
        }

        // Close last block length if open
        if(in_block && state->ir_signal_count > 0) {
            uint64_t file_size = storage_file_size(file);
            state->ir_signal_block_lengths[state->ir_signal_count - 1] =
                (size_t)file_size - state->ir_signal_block_offsets[state->ir_signal_count - 1];
        }

        ok = state->ir_signal_count > 0;
    } while(false);

    if(file) {
        storage_file_close(file);
        storage_file_free(file);
    }
    if(storage) {
        furi_record_close(RECORD_STORAGE);
    }

    return ok;
}

static bool ir_query_and_parse_universals(AppState* state) {
    if(!state || !state->uart_context) return false;

    uart_reset_text_buffers(state->uart_context);
    send_uart_command("ir universals list\n", state);

    char buffer[IR_UART_PARSE_BUF_SIZE];

    size_t len = 0;
    uint32_t start = furi_get_tick();
    const uint32_t timeout_ms = 3000;
    while(furi_get_tick() - start < timeout_ms) {
        furi_delay_ms(100);
        if(uart_copy_text_buffer_tail(state->uart_context, buffer, IR_UART_PARSE_BUF_SIZE, &len) &&
           len > 0) {
            if(strstr(buffer, "Universal Files in ") || strstr(buffer, "Built-in") ||
               strstr(buffer, "(none)") || strchr(buffer, '.')) {
                break;
            }
        }
    }

    if(len == 0) {
        return false;
    }

    state->ir_universal_count = 0;
    bool in_files_section = false;

    size_t pos = 0;
    char* line = NULL;
    while((line = next_line(buffer, &pos))) {
        while(*line == ' ' || *line == '\t')
            line++;

        if(strncmp(line, "IR: ", 4) == 0) {
            line += 4;
            while(*line == ' ' || *line == '\t')
                line++;
        }

        if(strncmp(line, "Universal Files in ", 19) == 0) {
            in_files_section = true;
            continue;
        }

        if(in_files_section) {
            if(line[0] == '\0') {
                continue;
            }

            if(strncmp(line, "Built-in Universal Signals", 26) == 0 ||
               strncmp(line, "Use 'ir universals list", 23) == 0) {
                in_files_section = false;
                continue;
            }

            if(strncmp(line, "(none)", 6) == 0) {
                continue;
            }

            if(state->ir_universal_count < COUNT_OF(state->ir_universals)) {
                IrUniversalEntry* e = &state->ir_universals[state->ir_universal_count];
                e->index = state->ir_universal_count;
                strncpy(e->name, line, sizeof(e->name) - 1);
                e->name[sizeof(e->name) - 1] = '\0';
                e->proto[0] = '\0';
                state->ir_universal_count++;
            }
        }
    }

    return state->ir_universal_count > 0;
}

static bool ir_query_and_parse_universal_buttons(AppState* state, const char* filename) {
    if(!state || !state->uart_context || !filename || !filename[0]) return false;

    uart_reset_text_buffers(state->uart_context);

    char path[128];
    snprintf(path, sizeof(path), "/mnt/ghostesp/infrared/universals/%s", filename);

    char cmd[192];
    snprintf(cmd, sizeof(cmd), "ir show %s\n", path);
    send_uart_command(cmd, state);

    char buffer[IR_UART_PARSE_BUF_SIZE];

    size_t len = 0;

    uint32_t start = furi_get_tick();
    const uint32_t timeout_ms = 5000;
    while(furi_get_tick() - start < timeout_ms) {
        furi_delay_ms(100);
        if(uart_copy_text_buffer(state->uart_context, buffer, IR_UART_PARSE_BUF_SIZE, &len) &&
           len > 0) {
            if(strstr(buffer, "Unique buttons in ") || strstr(buffer, "Signals in ")) {
                break;
            }
        }
    }

    if(len == 0) {
        return false;
    }

    state->ir_signal_count = 0;

    size_t pos = 0;
    char* line = NULL;
    while((line = next_line(buffer, &pos))) {
        while(*line == ' ' || *line == '\t')
            line++;

        if(strncmp(line, "Signals in ", 11) == 0) {
            continue;
        }
        if(strncmp(line, "IR: ", 4) == 0) {
            continue;
        }

        if(line[0] == '[') {
            unsigned int idx = 0;
            char name[32] = {0};
            char proto[16] = {0};
            int n = sscanf(line, "[%u] %31s (%15[^)])", &idx, name, proto);
            if(n < 2) {
                proto[0] = '\0';
                n = sscanf(line, "[%u] %31s", &idx, name);
            }
            if(n >= 2) {
                bool exists = false;
                for(size_t i = 0; i < state->ir_signal_count; i++) {
                    if(strcmp(state->ir_signals[i].name, name) == 0) {
                        exists = true;
                        break;
                    }
                }
                if(!exists && state->ir_signal_count < COUNT_OF(state->ir_signals)) {
                    IrSignalEntry* e = &state->ir_signals[state->ir_signal_count++];
                    e->index = idx;
                    strncpy(e->name, name, sizeof(e->name) - 1);
                    e->name[sizeof(e->name) - 1] = '\0';
                    if(n == 3) {
                        strncpy(e->proto, proto, sizeof(e->proto) - 1);
                        e->proto[sizeof(e->proto) - 1] = '\0';
                    } else {
                        e->proto[0] = '\0';
                    }
                }
            }
        }
    }

    bool result = state->ir_signal_count > 0;
    return result;
}

static void ir_show_remotes_menu(AppState* state) {
    if(!state || !state->ir_remotes_menu) return;

    submenu_reset(state->ir_remotes_menu);
    submenu_set_header(state->ir_remotes_menu, GHOST_ESP_UI_TEXT("IR Remotes", "红外遥控器"));

    uint32_t selected = 0;
    for(size_t i = 0; i < state->ir_remote_count; i++) {
        submenu_add_item(
            state->ir_remotes_menu, state->ir_remotes[i].name, i, submenu_callback, state);
        if(state->ir_remotes[i].index == state->ir_current_remote_index) {
            selected = i;
        }
    }

    if(state->ir_remote_count > 0) {
        submenu_set_selected_item(state->ir_remotes_menu, selected);
    }

    view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_IR_REMOTES);
    state->current_view = VIEW_IR_REMOTES;
}

static void ir_show_buttons_menu(AppState* state) {
    if(!state || !state->ir_buttons_menu) return;

    submenu_reset(state->ir_buttons_menu);
    if(state->ir_universal_buttons_mode) {
        submenu_set_header(state->ir_buttons_menu, GHOST_ESP_UI_TEXT("Universal Buttons", "万能遥控按键"));
    } else {
        submenu_set_header(state->ir_buttons_menu, GHOST_ESP_UI_TEXT("IR Buttons", "红外按键"));
    }

    for(size_t i = 0; i < state->ir_signal_count; i++) {
        const char* label = state->ir_signals[i].name;
        submenu_add_item(state->ir_buttons_menu, label, i, submenu_callback, state);
    }

    if(state->ir_signal_count > 0) {
        submenu_set_selected_item(state->ir_buttons_menu, 0);
    }

    view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_IR_BUTTONS);
    state->current_view = VIEW_IR_BUTTONS;
}

static void ir_show_universals_menu(AppState* state) {
    if(!state || !state->ir_universals_menu) return;

    submenu_reset(state->ir_universals_menu);
    submenu_set_header(state->ir_universals_menu, GHOST_ESP_UI_TEXT("IR Universals", "红外万能遥控"));

    for(size_t i = 0; i < state->ir_universal_count; i++) {
        const char* label = state->ir_universals[i].name;
        submenu_add_item(state->ir_universals_menu, label, i, submenu_callback, state);
    }

    if(state->ir_universal_count > 0) {
        submenu_set_selected_item(state->ir_universals_menu, 0);
    }

    view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_IR_UNIVERSALS);
    state->current_view = VIEW_IR_UNIVERSALS;
}

static void ir_show_error(AppState* state, const char* text) {
    if(!state || !state->confirmation_view) return;

    state->previous_view = state->current_view;
    confirmation_view_set_header(state->confirmation_view, GHOST_ESP_UI_TEXT("IR Error", "红外错误"));
    confirmation_view_set_text(state->confirmation_view, text ? text : GHOST_ESP_UI_TEXT("IR error", "红外错误"));
    confirmation_view_set_ok_callback(state->confirmation_view, app_info_ok_callback, state);
    confirmation_view_set_cancel_callback(state->confirmation_view, app_info_ok_callback, state);
    view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_CONFIRMATION);
    state->current_view = VIEW_CONFIRMATION;
}

static bool cycle_menu_item(
    CyclingMenuDef* cycling_array,
    size_t cycling_count,
    size_t* current_index,
    MenuCommand* menu_commands,
    size_t menu_index,
    Submenu* menu,
    InputEvent* event) {
    if(event->key == InputKeyRight) {
        *current_index = (*current_index + 1) % cycling_count;
    } else {
        *current_index = (*current_index == 0) ? (cycling_count - 1) : (*current_index - 1);
    }
    submenu_change_item_label(menu, menu_index, cycling_array[*current_index].label);

    // Update menu command fields
    MenuCommand* cmd = &menu_commands[menu_index];
    cmd->command = cycling_array[*current_index].command;
    cmd->needs_input = cycling_array[*current_index].needs_input;
    cmd->input_text = cycling_array[*current_index].input_text;
    cmd->details_header = cycling_array[*current_index].details_header;
    cmd->details_text = cycling_array[*current_index].details_text;

    return true;
}

void send_uart_command(const char* command, void* state) {
    AppState* app_state = (AppState*)state;
    uart_send(app_state->uart_context, (uint8_t*)command, strlen(command));
}

void send_uart_command_with_text(const char* command, char* text, AppState* state) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s %s\n", command, text);
    uart_send(state->uart_context, (uint8_t*)buffer, strlen(buffer));
}

void send_uart_command_with_bytes(
    const char* command,
    const uint8_t* bytes,
    size_t length,
    AppState* state) {
    send_uart_command(command, state);
    uart_send(state->uart_context, bytes, length);
}

static void confirmation_ok_callback(void* context) {
    MenuCommandContext* cmd_ctx = context;
    if(cmd_ctx && cmd_ctx->state && cmd_ctx->command) {
        bool file_opened = false;

        // Handle capture commands
        if(cmd_ctx->command->capture_prefix || cmd_ctx->command->file_ext ||
           cmd_ctx->command->folder) {
            FURI_LOG_I("Capture", "Attempting to open PCAP file before sending capture command.");
            file_opened = uart_receive_data(
                cmd_ctx->state->uart_context,
                cmd_ctx->state->view_dispatcher,
                cmd_ctx->state,
                cmd_ctx->command->capture_prefix ? cmd_ctx->command->capture_prefix : "",
                cmd_ctx->command->file_ext ? cmd_ctx->command->file_ext : "",
                cmd_ctx->command->folder ? cmd_ctx->command->folder : "");

            if(!file_opened) {
                FURI_LOG_E("Capture", "Failed to open PCAP file. Aborting capture command.");
                confirmation_cancel_callback(cmd_ctx);
                return;
            }

            // Send capture command
            send_uart_command(cmd_ctx->command->command, cmd_ctx->state);
            FURI_LOG_I("Capture", "Capture command sent to firmware.");
        } else {
            // For non-capture confirmation commands, send command and switch to text
            // view
            send_uart_command(cmd_ctx->command->command, cmd_ctx->state);
            uart_receive_data(
                cmd_ctx->state->uart_context,
                cmd_ctx->state->view_dispatcher,
                cmd_ctx->state,
                "",
                "",
                ""); // No capture files needed
        }
    }
    if(cmd_ctx->state) cmd_ctx->state->active_confirm_context = NULL;
    free(cmd_ctx);
}

void navigate_to_view(AppState* state, uint8_t view_id) {
    if(!state || !state->view_dispatcher) return;
    switch(view_id) {
    case VIEW_MAIN:
        show_main_menu(state);
        break;
    case VIEW_WIFI:
        show_wifi_menu(state);
        break;
    case VIEW_WIFI_SCANNING:
        show_wifi_scanning_menu(state);
        break;
    case VIEW_WIFI_CAPTURE:
        show_wifi_capture_menu(state);
        break;
    case VIEW_WIFI_ATTACK:
        show_wifi_attack_menu(state);
        break;
    case VIEW_WIFI_NETWORK:
        show_wifi_network_menu(state);
        break;
    case VIEW_WIFI_SETTINGS:
        show_wifi_settings_menu(state);
        break;
    case VIEW_AERIAL:
        show_aerial_menu(state);
        break;
    case VIEW_BLE:
        show_ble_menu(state);
        break;
    case VIEW_BLE_SCANNING:
        show_ble_scanning_menu(state);
        break;
    case VIEW_BLE_CAPTURE:
        show_ble_capture_menu(state);
        break;
    case VIEW_BLE_ATTACK:
        show_ble_attack_menu(state);
        break;
    case VIEW_GPS:
        show_gps_menu(state);
        break;
    case VIEW_IR:
        show_ir_menu(state);
        break;
    case VIEW_IR_REMOTES:
        ir_show_remotes_menu(state);
        break;
    case VIEW_IR_BUTTONS:
        ir_show_buttons_menu(state);
        break;
    case VIEW_IR_UNIVERSALS:
        ir_show_universals_menu(state);
        break;
    case VIEW_STATUS_IDLE:
        show_status_idle_menu(state);
        break;
    case VIEW_SETTINGS_CONFIG:
    case VIEW_SETTINGS_ACTIONS:
        view_dispatcher_switch_to_view(state->view_dispatcher, view_id);
        state->current_view = view_id;
        break;
    default:
        show_main_menu(state);
        break;
    }
}

static void confirmation_cancel_callback(void* context) {
    MenuCommandContext* cmd_ctx = context;
    if(cmd_ctx && cmd_ctx->state) {
        cmd_ctx->state->active_confirm_context = NULL;
        navigate_to_view(cmd_ctx->state, cmd_ctx->state->previous_view);
    }
    free(cmd_ctx);
}

static void app_info_ok_callback(void* context) {
    AppState* state = context;
    if(!state) return;

    view_dispatcher_switch_to_view(state->view_dispatcher, state->previous_view);
    state->current_view = state->previous_view;
}

static void show_command_details(AppState* state, const MenuCommand* command) {
    if(!command->details_header || !command->details_text) return;

    // Save current view before switching
    state->previous_view = state->current_view;

    // Setup confirmation view to show details
    confirmation_view_set_header(state->confirmation_view, command->details_header);
    confirmation_view_set_text(state->confirmation_view, command->details_text);

    // Set up callbacks for OK/Cancel to return to previous view
    confirmation_view_set_ok_callback(
        state->confirmation_view,
        app_info_ok_callback, // Reuse app info callback since it does the same
        // thing
        state);
    confirmation_view_set_cancel_callback(state->confirmation_view, app_info_ok_callback, state);

    // Switch to confirmation view
    view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_CONFIRMATION);
    state->current_view = VIEW_CONFIRMATION;
}

static void error_callback(void* context) {
    AppState* state = (AppState*)context;
    if(!state) return;
    view_dispatcher_switch_to_view(state->view_dispatcher, state->previous_view);
    state->current_view = state->previous_view;
}

static void show_result_dialog(AppState* state, const char* header, const char* text) {
    if(!state || !state->confirmation_view) return;

    state->previous_view = state->current_view;
    confirmation_view_set_header(state->confirmation_view, header ? header : GHOST_ESP_UI_TEXT("Result", "结果"));
    confirmation_view_set_text(state->confirmation_view, text ? text : "");
    confirmation_view_set_ok_callback(state->confirmation_view, app_info_ok_callback, state);
    confirmation_view_set_cancel_callback(state->confirmation_view, app_info_ok_callback, state);
    view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_CONFIRMATION);
    state->current_view = VIEW_CONFIRMATION;
}

static void ir_sweep_stop_callback(void* context) {
    AppState* state = (AppState*)context;
    if(!state) return;
    send_uart_command("stop\n", state);
    app_info_ok_callback(state);
}

static bool handle_ir_command_feedback_ex(
    AppState* state,
    const char* cmd,
    bool send_cmd,
    bool reset_buffers) {
    if(!state || !state->uart_context || !cmd) return false;

    bool is_send = strncmp(cmd, "ir send", 7) == 0;
    bool is_uni_send = strncmp(cmd, "ir universals send ", 20) == 0;
    bool is_uni_sendall = strncmp(cmd, "ir universals sendall", 21) == 0;
    bool is_inline = strncmp(cmd, "ir inline", 9) == 0;
    bool is_dazzler = strncmp(cmd, "ir dazzler", 10) == 0;

    if(!is_send && !is_uni_send && !is_uni_sendall && !is_inline && !is_dazzler) return false;

    if(reset_buffers) {
        uart_reset_text_buffers(state->uart_context);
    }
    if(send_cmd) {
        send_uart_command(cmd, state);
    }

    if(is_uni_sendall) {
        state->previous_view = state->current_view;
        confirmation_view_set_header(state->confirmation_view, GHOST_ESP_UI_TEXT("Universal send", "万能遥控发送"));
        confirmation_view_set_text(state->confirmation_view, GHOST_ESP_UI_TEXT("Universal sending...\nOK = Stop", "万能遥控发送中...\n按OK停止"));
        confirmation_view_set_ok_callback(state->confirmation_view, ir_sweep_stop_callback, state);
        confirmation_view_set_cancel_callback(
            state->confirmation_view, app_info_ok_callback, state);
        view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_CONFIRMATION);
        state->current_view = VIEW_CONFIRMATION;
    } else if(is_dazzler) {
        show_result_dialog(state, GHOST_ESP_UI_TEXT("IR Dazzler", "红外眩目器"), GHOST_ESP_UI_TEXT("Working...", "工作中..."));
    } else {
        show_result_dialog(state, GHOST_ESP_UI_TEXT("IR", "红外"), GHOST_ESP_UI_TEXT("Transmitting...", "发送中..."));
    }

    char buffer[512];
    char raw_buffer[512];
    size_t len = 0;
    char* message = state->confirmation_message;
    char summary[128];
    message[0] = '\0';
    summary[0] = '\0';
    raw_buffer[0] = '\0';
    bool have_output = false;
    bool saw_ok = false;

    uint32_t start = furi_get_tick();
    const uint32_t timeout_ms = is_uni_sendall ? 60000 : 5000;

    while(furi_get_tick() - start < timeout_ms) {
        furi_delay_ms(100);

        if(!uart_copy_text_buffer_tail(state->uart_context, buffer, sizeof(buffer), &len) ||
           len == 0) {
            continue;
        }

        have_output = true;
        memcpy(raw_buffer, buffer, len < sizeof(raw_buffer) ? len : sizeof(raw_buffer) - 1);
        raw_buffer[len < sizeof(raw_buffer) ? len : sizeof(raw_buffer) - 1] = '\0';

        size_t pos = 0;
        char* line = NULL;
        while((line = next_line(buffer, &pos))) {
            while(*line == ' ' || *line == '\t')
                line++;

            if(is_dazzler) {
                char* tag = strstr(line, "IR_DAZZLER:");
                if(tag) {
                    const char* code = tag + 11; // skip "IR_DAZZLER:"
                    while(*code == ' ' || *code == '\t')
                        code++;

                    if(strncmp(code, "STARTED", 7) == 0) {
                        strncpy(
                            message,
                            GHOST_ESP_UI_TEXT("Dazzler started successfully", "眩目器启动成功"),
                            sizeof(state->confirmation_message) - 1);
                    } else if(strncmp(code, "FAILED", 6) == 0) {
                        strncpy(
                            message, GHOST_ESP_UI_TEXT("Dazzler failed", "眩目器启动失败"), sizeof(state->confirmation_message) - 1);
                    } else if(strncmp(code, "ALREADY_RUNNING", 15) == 0) {
                        strncpy(
                            message,
                            GHOST_ESP_UI_TEXT("Dazzler is already running", "眩目器已在运行"),
                            sizeof(state->confirmation_message) - 1);
                    } else if(strncmp(code, "STOPPING", 8) == 0) {
                        strncpy(
                            message, GHOST_ESP_UI_TEXT("Stopped dazzler.", "眩目器已停止。"), sizeof(state->confirmation_message) - 1);
                    } else if(strncmp(code, "NOT_RUNNING", 11) == 0) {
                        strncpy(
                            message,
                            GHOST_ESP_UI_TEXT("Dazzler is not running", "眩目器未运行"),
                            sizeof(state->confirmation_message) - 1);
                    } else {
                        snprintf(
                            message, sizeof(state->confirmation_message), "Dazzler: %.64s", code);
                    }
                    message[sizeof(state->confirmation_message) - 1] = '\0';
                    start = timeout_ms + start;
                    break;
                }
            }

            if(is_send || is_uni_send || is_inline) {
                if(strncmp(line, "IR: signal ", 11) == 0) {
                    const char* p = line + 11;
                    char name[16] = {0};
                    char proto[16] = {0};
                    char addr[16] = {0};
                    char cmd[16] = {0};
                    char len_str[8] = {0};
                    char freq_str[24] = {0};
                    char duty_str[16] = {0};

                    if(strncmp(p, "raw ", 4) == 0 || strncmp(p, "raw len=", 8) == 0 ||
                       strstr(p, " raw ") || strstr(p, " raw len=")) {
                        if(sscanf(
                               line,
                               "IR: signal raw len=%15s freq=%31s duty=%15s",
                               len_str,
                               freq_str,
                               duty_str) == 3) {
                            snprintf(
                                summary,
                                sizeof(summary),
                                "Raw len=%s\nFreq: %s\nDuty: %s",
                                len_str,
                                freq_str,
                                duty_str);
                        }
                    } else {
                        if(strchr(p, '[')) {
                            if(sscanf(
                                   line,
                                   "IR: signal [%15[^]]] protocol=%15s addr=%15s cmd=%15s",
                                   name,
                                   proto,
                                   addr,
                                   cmd) >= 4) {
                                if(cmd[0] == '\0') {
                                    cmd[0] = '-';
                                    cmd[1] = '\0';
                                }
                                snprintf(
                                    summary,
                                    sizeof(summary),
                                    "%s (%s)\nAddr: %s\nCmd: %s",
                                    name,
                                    proto,
                                    addr,
                                    cmd);
                            }
                        } else {
                            if(sscanf(
                                   line,
                                   "IR: signal protocol=%15s addr=%15s cmd=%15s",
                                   proto,
                                   addr,
                                   cmd) >= 3) {
                                snprintf(
                                    summary,
                                    sizeof(summary),
                                    "Proto: %s\nAddr: %s\nCmd: %s",
                                    proto,
                                    addr,
                                    cmd);
                            }
                        }
                    }

                    if(saw_ok && summary[0] && !message[0]) {
                        // Truncate summary if needed to prevent buffer overflow when combining
                        size_t max_len = sizeof(state->confirmation_message) -
                                         10; // Reserve space for "Send OK\n"
                        if(strlen(summary) > max_len) {
                            summary[max_len] = '\0';
                        }
                        snprintf(
                            message,
                            sizeof(state->confirmation_message),
                            "Send OK%s%s",
                            "\n",
                            summary);
                        start = timeout_ms + start;
                        break;
                    }
                    continue;
                }
            }

            if(is_inline && strstr(line, "IR inline parse failed")) {
                strncpy(message, GHOST_ESP_UI_TEXT("Inline parse failed", "内联解析失败"), sizeof(state->confirmation_message) - 1);
                message[sizeof(state->confirmation_message) - 1] = '\0';
                start = timeout_ms + start;
                break;
            }

            if(is_send || is_uni_send || is_inline) {
                if(strstr(line, "send OK") || strstr(line, "status: OK") ||
                   strstr(line, "status OK") || strstr(line, "ir signal transmission complete")) {
                    saw_ok = true;
                    if(summary[0]) {
                        // Truncate summary if needed
                        size_t max_len = sizeof(state->confirmation_message) - 10;
                        if(strlen(summary) > max_len) {
                            summary[max_len] = '\0';
                        }
                        snprintf(
                            message,
                            sizeof(state->confirmation_message),
                            "%s%s%s",
                            GHOST_ESP_UI_TEXT("Send OK", "发送成功"),
                            "\n",
                            summary);
                        start = timeout_ms + start;
                        break;
                    }
                }
                if(strstr(line, "send FAIL") || strstr(line, "status: FAIL") ||
                   strstr(line, "status FAIL") || strstr(line, "status: ERROR")) {
                    strncpy(message, GHOST_ESP_UI_TEXT("Send failed", "发送失败"), sizeof(state->confirmation_message) - 1);
                    message[sizeof(state->confirmation_message) - 1] = '\0';
                    start = timeout_ms + start;
                    break;
                }
                if(strstr(line, "failed to read list")) {
                    strncpy(
                        message, GHOST_ESP_UI_TEXT("Failed to read list", "读取列表失败"), sizeof(state->confirmation_message) - 1);
                    message[sizeof(state->confirmation_message) - 1] = '\0';
                    start = timeout_ms + start;
                    break;
                }
                if(strstr(line, "no signals in")) {
                    strncpy(
                        message, GHOST_ESP_UI_TEXT("No signals in list", "列表中无信号"), sizeof(state->confirmation_message) - 1);
                    message[sizeof(state->confirmation_message) - 1] = '\0';
                    start = timeout_ms + start;
                    break;
                }
                if(strstr(line, "remote index out of range")) {
                    strncpy(
                        message,
                        GHOST_ESP_UI_TEXT("Remote index out of range", "遥控器索引超出范围"),
                        sizeof(state->confirmation_message) - 1);
                    message[sizeof(state->confirmation_message) - 1] = '\0';
                    start = timeout_ms + start;
                    break;
                }
                if(strstr(line, "index out of range")) {
                    strncpy(
                        message,
                        GHOST_ESP_UI_TEXT("Button index out of range", "按键索引超出范围"),
                        sizeof(state->confirmation_message) - 1);
                    message[sizeof(state->confirmation_message) - 1] = '\0';
                    start = timeout_ms + start;
                    break;
                }
                if(strstr(line, "invalid universal index")) {
                    strncpy(
                        message,
                        GHOST_ESP_UI_TEXT("Invalid universal index", "无效的万能遥控索引"),
                        sizeof(state->confirmation_message) - 1);
                    message[sizeof(state->confirmation_message) - 1] = '\0';
                    start = timeout_ms + start;
                    break;
                }
            } else if(is_uni_sendall) {
                if(strstr(line, "universal sendall already running")) {
                    strncpy(
                        message,
                        GHOST_ESP_UI_TEXT("Universal send already running; use 'stop' to cancel.", "万能遥控发送已在运行; 使用 'stop' 取消。"),
                        sizeof(state->confirmation_message) - 1);
                    message[sizeof(state->confirmation_message) - 1] = '\0';
                    start = timeout_ms + start;
                    break;
                }
                if(strstr(line, "universal sendall started")) {
                }
                if(strstr(line, "no builtin signals named")) {
                    strncpy(
                        message,
                        GHOST_ESP_UI_TEXT("No builtin signals with that name.", "没有该名称的内置信号。"),
                        sizeof(state->confirmation_message) - 1);
                    message[sizeof(state->confirmation_message) - 1] = '\0';
                    start = timeout_ms + start;
                    break;
                }
                if(strstr(line, "no signals named")) {
                    strncpy(
                        message,
                        GHOST_ESP_UI_TEXT("No file signals with that name.", "没有该名称的文件信号。"),
                        sizeof(state->confirmation_message) - 1);
                    message[sizeof(state->confirmation_message) - 1] = '\0';
                    start = timeout_ms + start;
                    break;
                }
                if(strstr(line, "universal sendall finished")) {
                    strncpy(
                        message,
                        GHOST_ESP_UI_TEXT("Universal send finished.", "万能遥控发送完成。"),
                        sizeof(state->confirmation_message) - 1);
                    message[sizeof(state->confirmation_message) - 1] = '\0';
                    confirmation_view_set_header(state->confirmation_view, GHOST_ESP_UI_TEXT("Universal send", "万能遥控发送"));
                    confirmation_view_set_text(state->confirmation_view, message);
                    confirmation_view_set_ok_callback(
                        state->confirmation_view, app_info_ok_callback, state);
                    confirmation_view_set_cancel_callback(
                        state->confirmation_view, app_info_ok_callback, state);
                    start = timeout_ms + start;
                    break;
                }
                if(strstr(line, "universal sendall stopped")) {
                    strncpy(
                        message,
                        GHOST_ESP_UI_TEXT("Universal send stopped.", "万能遥控发送已停止。"),
                        sizeof(state->confirmation_message) - 1);
                    message[sizeof(state->confirmation_message) - 1] = '\0';
                    confirmation_view_set_header(state->confirmation_view, GHOST_ESP_UI_TEXT("Universal send", "万能遥控发送"));
                    confirmation_view_set_text(state->confirmation_view, message);
                    confirmation_view_set_ok_callback(
                        state->confirmation_view, app_info_ok_callback, state);
                    confirmation_view_set_cancel_callback(
                        state->confirmation_view, app_info_ok_callback, state);
                    start = timeout_ms + start;
                    break;
                }
            }
        }

        if(message[0]) break;
    }

    if(!message[0] && saw_ok) {
        strncpy(message, GHOST_ESP_UI_TEXT("Send OK", "发送成功"), sizeof(state->confirmation_message) - 1);
        message[sizeof(state->confirmation_message) - 1] = '\0';
    }

    if(message[0]) {
        if(strncmp(message, GHOST_ESP_UI_TEXT("Send OK", "发送成功"), strlen(GHOST_ESP_UI_TEXT("Send OK", "发送成功"))) == 0) {
            const char* body = message + strlen(GHOST_ESP_UI_TEXT("Send OK", "发送成功"));
            if(*body == '\n') body++;
            confirmation_view_set_header(state->confirmation_view, GHOST_ESP_UI_TEXT("Sent Successfully", "发送成功"));
            confirmation_view_set_text(state->confirmation_view, body);
        } else {
            confirmation_view_set_text(state->confirmation_view, message);
        }
    } else if(have_output) {
        char display[256];
        snprintf(display, sizeof(display), "%s\nRaw:\n%.180s", GHOST_ESP_UI_TEXT("No match.", "无匹配。"), raw_buffer);
        confirmation_view_set_text(state->confirmation_view, display);
    } else {
        confirmation_view_set_text(state->confirmation_view, GHOST_ESP_UI_TEXT("No response from ESP.", "ESP 无响应。"));
    }

    return true;
}

static bool handle_ir_command_feedback(AppState* state, const char* cmd) {
    return handle_ir_command_feedback_ex(state, cmd, true, true);
}

// Text input callback implementation
static void text_input_result_callback(void* context) {
    AppState* input_state = (AppState*)context;
    if(input_state->connect_input_stage == 1) {
        size_t len = strlen(input_state->input_buffer);
        if(len >= sizeof(input_state->connect_ssid)) len = sizeof(input_state->connect_ssid) - 1;
        memcpy(input_state->connect_ssid, input_state->input_buffer, len);
        input_state->connect_ssid[len] = '\0';
        input_state->connect_input_stage = 2;
        if(input_state->input_buffer) memset(input_state->input_buffer, 0, INPUT_BUFFER_SIZE);
        text_input_reset(input_state->text_input);
        text_input_set_header_text(input_state->text_input, GHOST_ESP_UI_TEXT("PASSWORD", "密码"));
        text_input_set_result_callback(
            input_state->text_input,
            text_input_result_callback,
            input_state,
            input_state->input_buffer,
            INPUT_BUFFER_SIZE,
            true);
#ifdef HAS_MOMENTUM_SUPPORT
        text_input_show_illegal_symbols(input_state->text_input, true);
#endif
        view_dispatcher_switch_to_view(input_state->view_dispatcher, VIEW_TEXT_INPUT);
        return;
    }
    if(input_state->connect_input_stage == 2) {
        char buffer[256];
        snprintf(
            buffer,
            sizeof(buffer),
            "connect \"%s\" \"%s\"\n",
            input_state->connect_ssid,
            input_state->input_buffer);
        uart_send(input_state->uart_context, (uint8_t*)buffer, strlen(buffer));
        input_state->connect_input_stage = 0;
        input_state->connect_ssid[0] = '\0';
    } else {
        if(input_state->uart_command && strcmp(input_state->uart_command, "ir send") == 0) {
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "ir send %s\n", input_state->input_buffer);
            handle_ir_command_feedback_ex(input_state, cmd, true, true);
        } else {
            send_uart_command_with_text(
                input_state->uart_command, input_state->input_buffer, input_state);
            uart_receive_data(
                input_state->uart_context, input_state->view_dispatcher, input_state, "", "", "");
        }
    }
    if(input_state->input_buffer) memset(input_state->input_buffer, 0, INPUT_BUFFER_SIZE);
}

static void send_ir_file(AppState* state) {
    uint8_t* ir_data = NULL;
    size_t ir_size = 0;

    if(!ghost_esp_ep_read_ir_file(state, &ir_data, &ir_size)) {
        return;
    }

    // Clear any cached buffer; we stream now
    if(state->ir_file_buffer) {
        free(state->ir_file_buffer);
        state->ir_file_buffer = NULL;
        state->ir_file_buffer_size = 0;
    }

    state->ir_universal_buttons_mode = false;
    state->ir_file_buttons_mode = true;

    if(!ir_index_buttons_from_file(state)) {
        state->ir_file_buttons_mode = false;
        ir_show_error(state, GHOST_ESP_UI_TEXT("No IR buttons found.", "未找到红外按键。"));
        return;
    }

    ir_show_buttons_menu(state);
}

static void ir_send_button_from_file(AppState* state, uint32_t button_index) {
    if(!state || !state->uart_context) return;
    if(button_index >= state->ir_signal_count) return;
    if(!state->ir_file_path[0]) return;

    size_t start = state->ir_signal_block_offsets[button_index];
    size_t payload_len = state->ir_signal_block_lengths[button_index];
    if(payload_len == 0) return;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    if(!storage_file_open(file, state->ir_file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return;
    }

    storage_file_seek(file, start, true);

    const size_t chunk = 512;
    uint8_t buf[chunk];
    const char* ir_begin_marker = "[IR/BEGIN]";
    const char* ir_close_marker = "[IR/CLOSE]";

    uart_reset_text_buffers(state->uart_context);
    uart_send(state->uart_context, (const uint8_t*)ir_begin_marker, 10);
    uart_send(state->uart_context, (const uint8_t*)"\n", 1);
    size_t remaining = payload_len;
    while(remaining > 0) {
        size_t to_read = (remaining > chunk) ? chunk : remaining;
        uint16_t read = storage_file_read(file, buf, (uint16_t)to_read);
        if(read == 0) break;
        uart_send(state->uart_context, buf, read);
        remaining -= read;
    }
    uart_send(state->uart_context, (const uint8_t*)ir_close_marker, 10);
    uart_send(state->uart_context, (const uint8_t*)"\n", 1);

    handle_ir_command_feedback_ex(state, "ir inline", false, false);

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

static void send_evil_portal_html(AppState* state) {
    uint8_t* the_html = NULL;
    size_t html_size = 0;

    if(ghost_esp_ep_read_html_file(state, &the_html, &html_size)) {
        if(the_html != NULL) {
            // Send the command first
            const char* command_str = "evilportal -c sethtmlstr\n";
            uart_send(state->uart_context, (const uint8_t*)command_str, strlen(command_str));

            // Begin HTML block
            const char* html_begin_marker = "[HTML/BEGIN]";
            uart_send(state->uart_context, (const uint8_t*)html_begin_marker, 12);

            // Send HTML content
            uart_send(state->uart_context, the_html, html_size);

            // End HTML block
            const char* html_close_marker = "[HTML/CLOSE]";
            uart_send(state->uart_context, (const uint8_t*)html_close_marker, 12);
            uart_send(state->uart_context, (const uint8_t*)"\n", 1);

            free(the_html);
        }
    } else {
        // Only free if read failed but buffer was allocated (unlikely but safe)
        if(the_html) free(the_html);
    }
}

static void execute_menu_command(AppState* state, const MenuCommand* command) {
    if(strcmp(command->command, "set_evil_portal_html") == 0) {
        send_evil_portal_html(state);
        return;
    }
    if(strcmp(command->command, "send_ir_file") == 0) {
        // Ensure capture streams are cleaned up before opening file browser
        if(state->uart_context) {
            uart_cleanup_capture_streams(state->uart_context);
        }
        send_ir_file(state);
        return;
    }
    if(!uart_is_esp_connected(state->uart_context)) {
        state->previous_view = state->current_view;
        confirmation_view_set_header(state->confirmation_view, GHOST_ESP_UI_TEXT("Connection Error", "连接错误"));
        confirmation_view_set_text(
            state->confirmation_view,
            GHOST_ESP_UI_TEXT(
                "No response from ESP!\nIs a command running?\nRestart the "
                "app.\nRestart ESP.\nCheck UART Pins.\nReflash if issues persist.\nYou "
                "can disable this check in the settings menu.\n\n",
                "ESP 无响应！\n是否有命令在运行？\n请重启应用。\n重启 ESP。\n检查 UART 引脚。\n如仍有问题请重新刷写。\n可在设置菜单中禁用此检测。\n\n"));
        confirmation_view_set_ok_callback(state->confirmation_view, error_callback, state);
        confirmation_view_set_cancel_callback(state->confirmation_view, error_callback, state);

        view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_CONFIRMATION);
        state->current_view = VIEW_CONFIRMATION;
        return;
    }

    if(!command->needs_input && !command->needs_confirmation && !command->capture_prefix &&
       !command->file_ext && !command->folder) {
        if(handle_ir_command_feedback(state, command->command)) {
            return;
        }
    }

    if(command->needs_input && strcmp(command->command, "connect") == 0) {
        state->connect_input_stage = 1;
        state->uart_command = command->command;
        state->previous_view = state->current_view;
        text_input_reset(state->text_input);
        if(state->input_buffer) memset(state->input_buffer, 0, INPUT_BUFFER_SIZE);
        text_input_set_header_text(state->text_input, GHOST_ESP_UI_TEXT("SSID", "SSID"));
        text_input_set_result_callback(
            state->text_input,
            text_input_result_callback,
            state,
            state->input_buffer,
            INPUT_BUFFER_SIZE,
            true);
#ifdef HAS_MOMENTUM_SUPPORT
        text_input_show_illegal_symbols(state->text_input, true);
#endif
        view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_TEXT_INPUT);
        state->current_view = VIEW_TEXT_INPUT;
        return;
    }

    // For commands needing input
    if(command->needs_input) {
        state->uart_command = command->command;
        state->previous_view = state->current_view;
        text_input_reset(state->text_input);
        if(state->input_buffer) memset(state->input_buffer, 0, INPUT_BUFFER_SIZE);
        text_input_set_header_text(state->text_input, command->input_text);
        text_input_set_result_callback(
            state->text_input,
            text_input_result_callback,
            state,
            state->input_buffer,
            INPUT_BUFFER_SIZE,
            true);
#ifdef HAS_MOMENTUM_SUPPORT
        text_input_show_illegal_symbols(state->text_input, true);
#endif
        view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_TEXT_INPUT);
        state->current_view = VIEW_TEXT_INPUT;
        return;
    }

    // For commands needing confirmation
    if(command->needs_confirmation) {
        MenuCommandContext* cmd_ctx = malloc(sizeof(MenuCommandContext));
        cmd_ctx->state = state;
        cmd_ctx->command = command;
        state->active_confirm_context = cmd_ctx;
        confirmation_view_set_header(state->confirmation_view, command->confirm_header);
        confirmation_view_set_text(state->confirmation_view, command->confirm_text);
        confirmation_view_set_ok_callback(
            state->confirmation_view, confirmation_ok_callback, cmd_ctx);
        confirmation_view_set_cancel_callback(
            state->confirmation_view, confirmation_cancel_callback, cmd_ctx);

        view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_CONFIRMATION);
        state->current_view = VIEW_CONFIRMATION;
        return;
    }

    // Handle variable sniff command
    if(state->current_view == VIEW_WIFI_CAPTURE && state->current_index == 0) {
        const SniffCommandDef* current_sniff = &sniff_commands[current_sniff_index];
        // Handle capture commands
        if(current_sniff->capture_prefix) {
            // Save current view for proper back navigation
            state->previous_view = state->current_view;
            bool file_opened = uart_receive_data(
                state->uart_context,
                state->view_dispatcher,
                state,
                current_sniff->capture_prefix,
                "pcap",
                GHOST_ESP_APP_FOLDER_PCAPS);

            if(!file_opened) {
                FURI_LOG_E("Capture", "Failed to open capture file");
                return;
            }

            furi_delay_ms(10);
            send_uart_command(current_sniff->command, state);
            state->current_view = VIEW_TEXT_BOX;
            return;
        }

        // Save view and show terminal log
        state->previous_view = state->current_view;
        uart_receive_data(state->uart_context, state->view_dispatcher, state, "", "", "");
        state->current_view = VIEW_TEXT_BOX;

        furi_delay_ms(5);
        send_uart_command(current_sniff->command, state);
        return;
    }

    // Handle variable beacon spam command
    if(state->current_view == VIEW_WIFI_ATTACK && state->current_index == 0) {
        const CyclingMenuDef* current_beacon = &beacon_spam_commands[current_beacon_index];

        // If it's custom mode (last index), handle text input
        if(current_beacon_index == COUNT_OF(beacon_spam_commands) - 1) {
            state->uart_command = current_beacon->command;
            // Save current view for proper back navigation
            state->previous_view = state->current_view;
            text_input_reset(state->text_input);
            text_input_set_header_text(state->text_input, GHOST_ESP_UI_TEXT("SSID Name", "SSID 名称"));
            text_input_set_result_callback(
                state->text_input,
                text_input_result_callback,
                state,
                state->input_buffer,
                INPUT_BUFFER_SIZE,
                true);
#ifdef HAS_MOMENTUM_SUPPORT
            text_input_show_illegal_symbols(state->text_input, true);
#endif
            view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_TEXT_INPUT);
            state->current_view = VIEW_TEXT_INPUT;
            return;
        }

        // Save view and show terminal log
        state->previous_view = state->current_view;
        uart_receive_data(state->uart_context, state->view_dispatcher, state, "", "", "");
        state->current_view = VIEW_TEXT_BOX;
        furi_delay_ms(5);
        send_uart_command(current_beacon->command, state);
        return;
    }

    // Handle variable rgbmode command (new branch for index 17)
    if(state->current_view == VIEW_WIFI_SETTINGS && state->current_index == 0) {
        const CyclingMenuDef* current_rgb = &rgbmode_commands[current_rgb_index];
        // Save view and show terminal log
        state->previous_view = state->current_view;
        uart_receive_data(state->uart_context, state->view_dispatcher, state, "", "", "");
        state->current_view = VIEW_TEXT_BOX;
        furi_delay_ms(5);
        send_uart_command(current_rgb->command, state);
        return;
    }

    // Handle variable WiFi scan command (scan modes like APs / APs Live / Stations / All)
    if(state->current_view == VIEW_WIFI_SCANNING && state->current_index == 0) {
        const CyclingMenuDef* current_scan = &wifi_scan_modes[current_wifi_scan_index];
        // Save view and show terminal log
        state->previous_view = state->current_view;
        uart_receive_data(state->uart_context, state->view_dispatcher, state, "", "", "");
        state->current_view = VIEW_TEXT_BOX;
        furi_delay_ms(5);
        send_uart_command(current_scan->command, state);
        return;
    }

    // Handle variable BLE spam command
    if(state->current_view == VIEW_BLE_ATTACK && state->current_index == 0) {
        const CyclingMenuDef* current_ble_spam = &ble_spam_commands[current_ble_spam_index];
        // Save view and show terminal log
        state->previous_view = state->current_view;
        uart_receive_data(state->uart_context, state->view_dispatcher, state, "", "", "");
        state->current_view = VIEW_TEXT_BOX;
        furi_delay_ms(5);
        send_uart_command(current_ble_spam->command, state);
        return;
    }

    // Handle capture commands
    if(command->capture_prefix || command->file_ext || command->folder) {
        // Save view and show terminal log
        state->previous_view = state->current_view;
        bool file_opened = uart_receive_data(
            state->uart_context,
            state->view_dispatcher,
            state,
            command->capture_prefix ? command->capture_prefix : "",
            command->file_ext ? command->file_ext : "",
            command->folder ? command->folder : "");

        if(!file_opened) {
            FURI_LOG_E("Capture", "Failed to open capture file");
            return;
        }

        furi_delay_ms(10);
        send_uart_command(command->command, state);
        state->current_view = VIEW_TEXT_BOX;
        return;
    }

    // Save view and show terminal log
    state->previous_view = state->current_view;
    uart_receive_data(state->uart_context, state->view_dispatcher, state, "", "", "");

    furi_delay_ms(5);
    send_uart_command(command->command, state);
}

// Menu display function implementation
static void show_menu(
    AppState* state,
    const MenuCommand* commands,
    size_t command_count,
    const char* header,
    Submenu* menu,
    uint8_t view_id) {
    submenu_reset(menu);
    submenu_set_header(menu, header);

    for(size_t i = 0; i < command_count; i++) {
        submenu_add_item(menu, commands[i].label, i, submenu_callback, state);
    }

    // Set up view with input handler
    View* menu_view = submenu_get_view(menu);
    view_set_context(menu_view, state);
    view_set_input_callback(menu_view, menu_input_handler);

    // Restore last selection based on menu type
    uint32_t last_index = 0;
    switch(view_id) {
    case VIEW_WIFI: // WiFi categories
        last_index = state->last_wifi_category_index;
        break;
    case VIEW_WIFI_SCANNING: // WiFi Scanning
        last_index = state->last_wifi_scanning_index;
        break;
    case VIEW_WIFI_CAPTURE: // WiFi Capture
        last_index = state->last_wifi_capture_index;
        break;
    case VIEW_WIFI_ATTACK: // WiFi Attack
        last_index = state->last_wifi_attack_index;
        break;
    case VIEW_WIFI_NETWORK: // WiFi Network
        last_index = state->last_wifi_network_index;
        break;
    case VIEW_WIFI_SETTINGS: // WiFi Settings
        last_index = state->last_wifi_settings_index;
        break;
    case VIEW_BLE: // BLE categories
        last_index = state->last_ble_category_index;
        break;
    case VIEW_BLE_SCANNING: // BLE Scanning
        last_index = state->last_ble_scanning_index;
        break;
    case VIEW_BLE_CAPTURE: // BLE Capture
        last_index = state->last_ble_capture_index;
        break;
    case VIEW_BLE_ATTACK: // BLE Attack
        last_index = state->last_ble_attack_index;
        break;
    case VIEW_GPS: // GPS
        last_index = state->last_gps_index;
        break;
    case VIEW_AERIAL: // Aerial
        last_index = state->last_aerial_category_index;
        break;
    }
    if(last_index < command_count) {
        submenu_set_selected_item(menu, last_index);
    }

    state->previous_view = state->current_view;
    view_dispatcher_switch_to_view(state->view_dispatcher, view_id);
    state->current_view = view_id;
}

// Menu display functions
void show_wifi_scanning_menu(AppState* state) {
    show_menu(
        state,
        wifi_scanning_commands,
        COUNT_OF(wifi_scanning_commands),
        GHOST_ESP_UI_TEXT("Scanning & Probing", "扫描与探测"),
        state->wifi_scanning_menu,
        10);

    // Ensure the first item label reflects the currently selected scan mode
    // (so the menu shows "Scan: (APs Live)" etc. after cycling)
    submenu_change_item_label(
        state->wifi_scanning_menu, 0, wifi_scan_modes[current_wifi_scan_index].label);

    // Also persist labels for list/select/listen cycling entries
    submenu_change_item_label(
        state->wifi_scanning_menu, 1, wifi_list_modes[current_wifi_list_index].label);
    submenu_change_item_label(
        state->wifi_scanning_menu, 2, wifi_select_modes[current_wifi_select_index].label);
    submenu_change_item_label(
        state->wifi_scanning_menu, 3, wifi_listen_modes[current_wifi_listen_index].label);
}

void show_wifi_capture_menu(AppState* state) {
    show_menu(
        state,
        wifi_capture_commands,
        COUNT_OF(wifi_capture_commands),
        GHOST_ESP_UI_TEXT("Packet Capture", "数据包捕获"),
        state->wifi_capture_menu,
        11);
}

void show_wifi_attack_menu(AppState* state) {
    show_menu(
        state,
        wifi_attack_commands,
        COUNT_OF(wifi_attack_commands),
        GHOST_ESP_UI_TEXT("Attacks", "攻击"),
        state->wifi_attack_menu,
        12);

    // Ensure beacon spam cycling label persists
    submenu_change_item_label(
        state->wifi_attack_menu, 0, beacon_spam_commands[current_beacon_index].label);
}

void show_wifi_network_menu(AppState* state) {
    show_menu(
        state,
        wifi_network_commands,
        COUNT_OF(wifi_network_commands),
        GHOST_ESP_UI_TEXT("Portal & Network", "门户与网络"),
        state->wifi_network_menu,
        13);
}

void show_wifi_settings_menu(AppState* state) {
    show_menu(
        state,
        wifi_settings_commands,
        COUNT_OF(wifi_settings_commands),
        GHOST_ESP_UI_TEXT("Settings & Hardware", "设置与硬件"),
        state->wifi_settings_menu,
        14);

    // Ensure rgbmode cycling label persists
    submenu_change_item_label(
        state->wifi_settings_menu, 0, rgbmode_commands[current_rgb_index].label);
}

void show_status_idle_menu(AppState* state) {
    show_menu(
        state,
        status_idle_commands,
        COUNT_OF(status_idle_commands),
        GHOST_ESP_UI_TEXT("Select an animation", "选择动画"),
        state->status_idle_menu,
        40);
}

void show_ble_scanning_menu(AppState* state) {
    show_menu(
        state,
        ble_scanning_commands,
        COUNT_OF(ble_scanning_commands),
        GHOST_ESP_UI_TEXT("Scanning & Detection", "扫描与检测"),
        state->ble_scanning_menu,
        20);
}

void show_ble_capture_menu(AppState* state) {
    show_menu(
        state,
        ble_capture_commands,
        COUNT_OF(ble_capture_commands),
        GHOST_ESP_UI_TEXT("Packet Capture", "数据包捕获"),
        state->ble_capture_menu,
        21);
}

void show_ble_attack_menu(AppState* state) {
    show_menu(
        state,
        ble_attack_commands,
        COUNT_OF(ble_attack_commands),
        GHOST_ESP_UI_TEXT("Attacks & Spoofing", "攻击与欺骗"),
        state->ble_attack_menu,
        22);

    // Ensure BLE spam cycling label persists
    submenu_change_item_label(
        state->ble_attack_menu, 0, ble_spam_commands[current_ble_spam_index].label);
}

void show_wifi_menu(AppState* state) {
    submenu_reset(state->wifi_menu);
    submenu_set_header(state->wifi_menu, GHOST_ESP_UI_TEXT("WiFi Commands", "WiFi 命令"));
    submenu_add_item(state->wifi_menu, GHOST_ESP_UI_TEXT("Scanning & Probing > ", "扫描与探测 > "), 0, submenu_callback, state);
    submenu_add_item(state->wifi_menu, GHOST_ESP_UI_TEXT("Packet Capture > ", "数据包捕获 > "), 1, submenu_callback, state);
    submenu_add_item(state->wifi_menu, GHOST_ESP_UI_TEXT("Attacks > ", "攻击 > "), 2, submenu_callback, state);
    submenu_add_item(state->wifi_menu, GHOST_ESP_UI_TEXT("Evil Portal & Network >", "邪恶门户与网络 >"), 3, submenu_callback, state);
    submenu_add_item(state->wifi_menu, GHOST_ESP_UI_TEXT("Aerial Detector >", "信号检测器 >"), 4, submenu_callback, state);
    submenu_add_item(state->wifi_menu, wifi_stop_command.label, 5, submenu_callback, state);
    // Restore last selected WiFi category
    submenu_set_selected_item(state->wifi_menu, state->last_wifi_category_index);

    view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_WIFI);
    state->current_view = VIEW_WIFI;
}

void show_aerial_menu(AppState* state) {
    show_menu(
        state,
        aerial_commands,
        COUNT_OF(aerial_commands),
        GHOST_ESP_UI_TEXT("Aerial Detector:", "信号检测器:"),
        state->aerial_menu,
        15);
}

void show_ble_menu(AppState* state) {
    submenu_reset(state->ble_menu);
    submenu_set_header(state->ble_menu, GHOST_ESP_UI_TEXT("BLE Commands", "蓝牙命令"));
    submenu_add_item(state->ble_menu, GHOST_ESP_UI_TEXT("Scanning & Detection >", "扫描与检测 >"), 0, submenu_callback, state);
    submenu_add_item(state->ble_menu, GHOST_ESP_UI_TEXT("Packet Capture >", "数据包捕获 >"), 1, submenu_callback, state);
    submenu_add_item(state->ble_menu, GHOST_ESP_UI_TEXT("Attacks & Spoofing >", "攻击与欺骗 >"), 2, submenu_callback, state);
    submenu_add_item(state->ble_menu, ble_stop_command.label, 3, submenu_callback, state);
    // Restore last selected BLE category
    submenu_set_selected_item(state->ble_menu, state->last_ble_category_index);

    view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_BLE);
    state->current_view = VIEW_BLE;
}

void show_gps_menu(AppState* state) {
    state->came_from_settings = false;
    show_menu(state, gps_commands, COUNT_OF(gps_commands), GHOST_ESP_UI_TEXT("GPS Commands:", "GPS 命令:"), state->gps_menu, 3);
}

void show_ir_menu(AppState* state) {
    show_menu(state, ir_commands, COUNT_OF(ir_commands), GHOST_ESP_UI_TEXT("IR Commands:", "红外命令:"), state->ir_menu, 30);
}

// Menu command handlers
void handle_wifi_menu(AppState* state, uint32_t index) {
    // This function is now for sub-category menus
    const MenuCommand* command = NULL;
    switch(state->current_view) {
    case VIEW_WIFI_SCANNING: // Scanning
        if(index < COUNT_OF(wifi_scanning_commands)) {
            command = &wifi_scanning_commands[index];
            state->last_wifi_scanning_index = index;
        }
        break;
    case VIEW_WIFI_CAPTURE: // Capture
        if(index < COUNT_OF(wifi_capture_commands)) {
            command = &wifi_capture_commands[index];
            state->last_wifi_capture_index = index;
        }
        break;
    case VIEW_WIFI_ATTACK: // Attack
        if(index < COUNT_OF(wifi_attack_commands)) {
            command = &wifi_attack_commands[index];
            state->last_wifi_attack_index = index;
        }
        break;
    case VIEW_WIFI_NETWORK: // Network
        if(index < COUNT_OF(wifi_network_commands)) {
            command = &wifi_network_commands[index];
            state->last_wifi_network_index = index;
        }
        break;
    case VIEW_WIFI_SETTINGS: // Settings
        if(index < COUNT_OF(wifi_settings_commands)) {
            command = &wifi_settings_commands[index];
            state->last_wifi_settings_index = index;
        }
        break;
    }

    if(command) {
        execute_menu_command(state, command);
    }
}

void handle_ble_menu(AppState* state, uint32_t index) {
    // This function is now for sub-category menus
    const MenuCommand* command = NULL;
    switch(state->current_view) {
    case VIEW_BLE_SCANNING: // Scanning
        if(index < COUNT_OF(ble_scanning_commands)) {
            command = &ble_scanning_commands[index];
            state->last_ble_scanning_index = index;
        }
        break;
    case VIEW_BLE_CAPTURE: // Capture
        if(index < COUNT_OF(ble_capture_commands)) {
            command = &ble_capture_commands[index];
            state->last_ble_capture_index = index;
        }
        break;
    case VIEW_BLE_ATTACK: // Attack
        if(index < COUNT_OF(ble_attack_commands)) {
            command = &ble_attack_commands[index];
            state->last_ble_attack_index = index;
        }
        break;
    }

    if(command) {
        execute_menu_command(state, command);
    }
}

void handle_aerial_menu(AppState* state, uint32_t index) {
    if(index < COUNT_OF(aerial_commands)) {
        state->last_aerial_category_index = index;
        execute_menu_command(state, &aerial_commands[index]);
    }
}

void handle_gps_menu(AppState* state, uint32_t index) {
    if(index < COUNT_OF(gps_commands)) {
        state->last_gps_index = index; // Save the selection
        execute_menu_command(state, &gps_commands[index]);
    }
}

void handle_ir_menu(AppState* state, uint32_t index) {
    if(index >= COUNT_OF(ir_commands)) return;

    state->last_ir_index = index;

    switch(index) {
    case 0:
        if(ir_query_and_parse_list(state)) {
            ir_show_remotes_menu(state);
        } else {
            ir_show_error(state, GHOST_ESP_UI_TEXT("No IR remotes found.", "未找到红外遥控器。"));
        }
        break;
    case 1:
        if(ir_query_and_parse_universals(state)) {
            ir_show_universals_menu(state);
        } else {
            ir_show_error(state, GHOST_ESP_UI_TEXT("No universal signals found.", "未找到万能遥控信号。"));
        }
        break;
    default:
        execute_menu_command(state, &ir_commands[index]);
        break;
    }
}

void submenu_callback(void* context, uint32_t index) {
    AppState* state = (AppState*)context;
    state->current_index = index; // Track current selection

    switch(state->current_view) {
    case VIEW_MAIN:
        switch(index) {
        case 0:
            show_wifi_menu(state);
            state->last_wifi_category_index = 0;
            break;
        case 1:
            show_ble_menu(state);
            state->last_ble_category_index = 0;
            break;
        case 2:
            show_gps_menu(state);
            break;
        case 3:
            show_ir_menu(state);
            break;
        case 4:
            view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_SETTINGS_ACTIONS);
            state->current_view = VIEW_SETTINGS_ACTIONS;
            break;
        }
        break;
    case VIEW_WIFI:
        state->last_wifi_category_index = index;
        switch(index) {
        case 0:
            show_wifi_scanning_menu(state);
            break;
        case 1:
            show_wifi_capture_menu(state);
            break;
        case 2:
            show_wifi_attack_menu(state);
            break;
        case 3:
            show_wifi_network_menu(state);
            break;
        case 4:
            show_aerial_menu(state);
            state->last_aerial_category_index = 0;
            break;
        case 5:
            execute_menu_command(state, &wifi_stop_command);
            break;
        }
        break;
    case VIEW_BLE:
        state->last_ble_category_index = index;
        switch(index) {
        case 0:
            show_ble_scanning_menu(state);
            break;
        case 1:
            show_ble_capture_menu(state);
            break;
        case 2:
            show_ble_attack_menu(state);
            break;
        case 3:
            execute_menu_command(state, &ble_stop_command);
            break;
        }
        break;
    case VIEW_GPS:
        handle_gps_menu(state, index);
        break;
    case VIEW_BLE_SCANNING:
    case VIEW_BLE_CAPTURE:
    case VIEW_BLE_ATTACK:
        handle_ble_menu(state, index);
        break;
    case VIEW_AERIAL:
        handle_aerial_menu(state, index);
        break;
    case VIEW_IR_REMOTES:
        if(index < state->ir_remote_count) {
            state->ir_universal_buttons_mode = false;
            state->ir_file_buttons_mode = false;
            state->ir_current_remote_index = state->ir_remotes[index].index;
            if(ir_query_and_parse_show(state, state->ir_current_remote_index)) {
                ir_show_buttons_menu(state);
            } else {
                ir_show_error(state, GHOST_ESP_UI_TEXT("No IR buttons found.", "未找到红外按键。"));
            }
        }
        break;
    case VIEW_IR_BUTTONS:
        if(index < state->ir_signal_count) {
            if(state->ir_file_buttons_mode) {
                ir_send_button_from_file(state, index);
            } else {
                IrSignalEntry* sig = &state->ir_signals[index];
                char cmd[128];
                if(state->ir_universal_buttons_mode) {
                    snprintf(
                        cmd,
                        sizeof(cmd),
                        "ir universals sendall %s %s\n",
                        state->ir_current_universal_file,
                        sig->name);
                } else {
                    snprintf(
                        cmd,
                        sizeof(cmd),
                        "ir send %lu %lu\n",
                        (unsigned long)state->ir_current_remote_index,
                        (unsigned long)sig->index);
                }
                MenuCommand dyn = {0};
                dyn.command = cmd;
                execute_menu_command(state, &dyn);
            }
        }
        break;
    case VIEW_IR_UNIVERSALS:
        if(index < state->ir_universal_count) {
            IrUniversalEntry* uni = &state->ir_universals[index];
            strncpy(
                state->ir_current_universal_file,
                uni->name,
                sizeof(state->ir_current_universal_file) - 1);
            state->ir_current_universal_file[sizeof(state->ir_current_universal_file) - 1] = '\0';
            state->ir_universal_buttons_mode = true;
            state->ir_file_buttons_mode = false;
            if(ir_query_and_parse_universal_buttons(state, state->ir_current_universal_file)) {
                ir_show_buttons_menu(state);
            } else {
                ir_show_error(state, GHOST_ESP_UI_TEXT("No universal buttons found.", "未找到万能遥控按键。"));
            }
        }
        break;
    case VIEW_IR:
        handle_ir_menu(state, index);
        break;
    }
}

static void show_menu_help(void* context, uint32_t index) {
    UNUSED(index);
    AppState* state = context;

    // Save current view
    state->previous_view = state->current_view;

    // Define help text with essential actions only
    const char* help_text =
        GHOST_ESP_UI_TEXT("=== Controls ===\n"
                          "Hold [Ok]\n"
                          "    Show command details\n"
                          "Back button returns to\n"
                          "previous menu\n"
                          "\n"
                          "=== File Locations ===\n"
                          "/apps_data/ghost_esp/\n"
                          "\n"
                          "\n"
                          "=== Tips ===\n"
                          "- One capture at a time\n"
                          "- Hold OK on any command\n"
                          "  to see range & details\n"
                          "\n"
                          "=== Settings ===\n"
                          "Configure options in\n"
                          "SET menu including:\n"
                          "- Auto-stop behavior\n"
                          "- LED settings\n"
                          "\n"
                          "Join the Discord\n"
                          "for support and\n"
                          "to stay updated!\n",
                          "=== 操作 ===\n"
                          "长按 [Ok]\n"
                          "    显示命令详情\n"
                          "返回键回到\n"
                          "上一级菜单\n"
                          "\n"
                          "=== 文件位置 ===\n"
                          "/apps_data/ghost_esp/\n"
                          "\n"
                          "\n"
                          "=== 提示 ===\n"
                          "- 同时只能捕获一项\n"
                          "- 长按OK查看\n"
                          "  命令范围与详情\n"
                          "\n"
                          "=== 设置 ===\n"
                          "在SET菜单中配置:\n"
                          "- 自动停止行为\n"
                          "- LED 设置\n"
                          "\n"
                          "加入 Discord\n"
                          "获取支持和\n"
                          "最新更新!\n");

    // Set header and help text in the confirmation view
    confirmation_view_set_header(state->confirmation_view, GHOST_ESP_UI_TEXT("Quick Help", "快速帮助"));
    confirmation_view_set_text(state->confirmation_view, help_text);

    // Set callbacks for user actions
    confirmation_view_set_ok_callback(state->confirmation_view, app_info_ok_callback, state);
    confirmation_view_set_cancel_callback(state->confirmation_view, app_info_ok_callback, state);

    // Switch to confirmation view to display help
    view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_CONFIRMATION);
    state->current_view = VIEW_CONFIRMATION;
}

bool back_event_callback(void* context) {
    AppState* state = (AppState*)context;
    if(!state) return false;

    uint8_t cv = state->current_view;

    if(cv == VIEW_CONFIRMATION) return false;

    if(cv == VIEW_TEXT_BOX) {
        if(state->settings.stop_on_back_index) {
            send_uart_command(wifi_stop_command.command, state);
        }
        if(state->uart_context) {
            uart_reset_text_buffers(state->uart_context);
            uart_cleanup_capture_streams(state->uart_context);
        }
        if(state->textBoxBuffer) state->buffer_length = 0;
        navigate_to_view(state, state->previous_view);
    } else if(cv == VIEW_TEXT_INPUT) {
        if(state->settings.stop_on_back_index) {
            send_uart_command(wifi_stop_command.command, state);
        }
        state->uart_command = NULL;
        state->connect_input_stage = 0;
        state->connect_ssid[0] = '\0';
        if(state->text_input) text_input_reset(state->text_input);
        if(state->input_buffer) memset(state->input_buffer, 0, INPUT_BUFFER_SIZE);
        navigate_to_view(state, state->previous_view);
    } else if(cv == VIEW_SETTINGS_ACTIONS) {
        navigate_to_view(state, VIEW_MAIN);
    } else if(cv == VIEW_SETTINGS_CONFIG) {
        navigate_to_view(state, VIEW_SETTINGS_ACTIONS);
    } else if(cv >= VIEW_WIFI && cv <= VIEW_GPS) {
        navigate_to_view(state, VIEW_MAIN);
    } else if(cv >= VIEW_IR_REMOTES && cv <= VIEW_IR_UNIVERSALS) {
        if(state->ir_file_buffer) {
            free(state->ir_file_buffer);
            state->ir_file_buffer = NULL;
            state->ir_file_buffer_size = 0;
        }
        state->ir_file_buttons_mode = false;
        state->ir_universal_buttons_mode = false;
        navigate_to_view(state, VIEW_IR);
    } else if(cv == VIEW_IR) {
        navigate_to_view(state, VIEW_MAIN);
    } else if(cv >= VIEW_WIFI_SCANNING && cv <= VIEW_AERIAL) {
        if(state->came_from_settings && cv >= VIEW_WIFI_SCANNING && cv <= VIEW_WIFI_SETTINGS) {
            navigate_to_view(state, VIEW_SETTINGS_ACTIONS);
        } else {
            navigate_to_view(state, VIEW_WIFI);
        }
    } else if(cv == VIEW_STATUS_IDLE) {
        navigate_to_view(state, VIEW_SETTINGS_ACTIONS);
    } else if(cv >= VIEW_BLE_SCANNING && cv <= VIEW_BLE_ATTACK) {
        navigate_to_view(state, VIEW_BLE);
    } else if(cv == VIEW_MAIN) {
        view_dispatcher_stop(state->view_dispatcher);
    }

    return true;
}

void show_main_menu(AppState* state) {
    main_menu_reset(state->main_menu);
    main_menu_set_header(state->main_menu, "");
    main_menu_add_item(state->main_menu, "WiFi", 0, submenu_callback, state);
    main_menu_add_item(state->main_menu, "BLE", 1, submenu_callback, state);
    main_menu_add_item(state->main_menu, "GPS", 2, submenu_callback, state);
    main_menu_add_item(state->main_menu, "  IR", 3, submenu_callback, state);
    main_menu_add_item(state->main_menu, " SET", 4, submenu_callback, state);

    // Set up help callback
    main_menu_set_help_callback(state->main_menu, show_menu_help, state);

    state->came_from_settings = false;
    view_dispatcher_switch_to_view(state->view_dispatcher, VIEW_MAIN);
    state->current_view = VIEW_MAIN;
}

bool text_view_input_handler(InputEvent* event, void* context) {
    AppState* state = (AppState*)context;
    if(!state || !event) return false;

    bool consumed = false;

    if(event->type == InputTypeShort && event->key == InputKeyOk) {
        send_uart_command(wifi_stop_command.command, state);
        consumed = true;
    } else if(event->type == InputTypeShort && event->key == InputKeyRight) {
        state->text_box_user_scrolled = false;
        update_text_box_view(state);
        consumed = true;
    } else {
        if((event->type == InputTypeShort || event->type == InputTypeRepeat) &&
           (event->key == InputKeyUp || event->key == InputKeyDown)) {
            state->text_box_user_scrolled = true;

            if(!state->text_box_pause_hint_shown) {
                state->text_box_pause_hint_shown = true;
                show_result_dialog(
                    state,
                    GHOST_ESP_UI_TEXT("Tip", "提示"),
                    GHOST_ESP_UI_TEXT(
                        "Scroll paused.\nPress Right arrow to resume.\nPress OK to send stop.",
                        "滚动已暂停。\n按右键恢复。\n按OK发送停止。"));
            }
        }

        if(state->text_box_original_input && state->text_box_original_context) {
            consumed = state->text_box_original_input(event, state->text_box_original_context);
        }
    }

    return consumed;
}

void text_view_attach_input_handler(AppState* state) {
    if(!state || !state->text_box) return;

    View* text_view = text_box_get_view(state->text_box);
    if(!text_view) return;

    state->text_box_original_input = text_view->input_callback;
    state->text_box_original_context = text_view->context;

    view_set_context(text_view, state);
    view_set_input_callback(text_view, text_view_input_handler);

    state->text_box_user_scrolled = false;
}

static bool menu_input_handler(InputEvent* event, void* context) {
    AppState* state = (AppState*)context;
    bool consumed = false;

    if(!state || !event) return false;

    const MenuCommand* commands = NULL;
    size_t commands_count = 0;
    Submenu* current_menu = NULL;

    // Determine current menu context
    switch(state->current_view) {
    case VIEW_WIFI:
        current_menu = state->wifi_menu;
        return false;
    case VIEW_BLE:
        current_menu = state->ble_menu;
        return false;
    case VIEW_BLE_SCANNING:
        current_menu = state->ble_scanning_menu;
        commands = ble_scanning_commands;
        commands_count = COUNT_OF(ble_scanning_commands);
        break;
    case VIEW_BLE_CAPTURE:
        current_menu = state->ble_capture_menu;
        commands = ble_capture_commands;
        commands_count = COUNT_OF(ble_capture_commands);
        break;
    case VIEW_BLE_ATTACK:
        current_menu = state->ble_attack_menu;
        commands = ble_attack_commands;
        commands_count = COUNT_OF(ble_attack_commands);
        break;
    case VIEW_GPS:
        current_menu = state->gps_menu;
        commands = gps_commands;
        commands_count = COUNT_OF(gps_commands);
        break;
    case VIEW_IR:
        current_menu = state->ir_menu;
        commands = ir_commands;
        commands_count = COUNT_OF(ir_commands);
        break;
    case VIEW_WIFI_SCANNING:
        current_menu = state->wifi_scanning_menu;
        commands = wifi_scanning_commands;
        commands_count = COUNT_OF(wifi_scanning_commands);
        break;
    case VIEW_WIFI_CAPTURE:
        current_menu = state->wifi_capture_menu;
        commands = wifi_capture_commands;
        commands_count = COUNT_OF(wifi_capture_commands);
        break;
    case VIEW_WIFI_ATTACK:
        current_menu = state->wifi_attack_menu;
        commands = wifi_attack_commands;
        commands_count = COUNT_OF(wifi_attack_commands);
        break;
    case VIEW_WIFI_NETWORK:
        current_menu = state->wifi_network_menu;
        commands = wifi_network_commands;
        commands_count = COUNT_OF(wifi_network_commands);
        break;
    case VIEW_WIFI_SETTINGS:
        current_menu = state->wifi_settings_menu;
        commands = wifi_settings_commands;
        commands_count = COUNT_OF(wifi_settings_commands);
        break;
    case VIEW_AERIAL:
        current_menu = state->aerial_menu;
        commands = aerial_commands;
        commands_count = COUNT_OF(aerial_commands);
        break;
    case VIEW_STATUS_IDLE:
        current_menu = state->status_idle_menu;
        commands = status_idle_commands;
        commands_count = COUNT_OF(status_idle_commands);
        break;
    default:
        return false;
    }

    if(!current_menu || !commands) return false;

    uint32_t current_index = submenu_get_selected_item(current_menu);

    switch(event->type) {
    case InputTypeShort:
        switch(event->key) {
        case InputKeyUp:
            if(current_index > 0) {
                submenu_set_selected_item(current_menu, current_index - 1);
            } else {
                // Wrap to bottom
                submenu_set_selected_item(current_menu, commands_count - 1);
            }
            consumed = true;
            break;

        case InputKeyDown:
            if(current_index < commands_count - 1) {
                submenu_set_selected_item(current_menu, current_index + 1);
            } else {
                // Wrap to top
                submenu_set_selected_item(current_menu, 0);
            }
            consumed = true;
            break;

        case InputKeyOk:
            if(current_index < commands_count) {
                if(state->current_view == VIEW_IR) {
                    submenu_callback(state, current_index);
                } else {
                    state->current_index = current_index;
                    // Save last selection for proper restore on exit
                    if(state->current_view >= VIEW_WIFI_SCANNING &&
                       state->current_view <= VIEW_WIFI_SETTINGS) {
                        switch(state->current_view) {
                        case VIEW_WIFI_SCANNING:
                            state->last_wifi_scanning_index = current_index;
                            break;
                        case VIEW_WIFI_CAPTURE:
                            state->last_wifi_capture_index = current_index;
                            break;
                        case VIEW_WIFI_ATTACK:
                            state->last_wifi_attack_index = current_index;
                            break;
                        case VIEW_WIFI_NETWORK:
                            state->last_wifi_network_index = current_index;
                            break;
                        case VIEW_WIFI_SETTINGS:
                            state->last_wifi_settings_index = current_index;
                            break;
                        }
                    } else if(
                        state->current_view >= VIEW_BLE_SCANNING &&
                        state->current_view <= VIEW_BLE_ATTACK) {
                        switch(state->current_view) {
                        case VIEW_BLE_SCANNING:
                            state->last_ble_scanning_index = current_index;
                            break;
                        case VIEW_BLE_CAPTURE:
                            state->last_ble_capture_index = current_index;
                            break;
                        case VIEW_BLE_ATTACK:
                            state->last_ble_attack_index = current_index;
                            break;
                        }
                    } else if(state->current_view == VIEW_GPS) {
                        state->last_gps_index = current_index;
                    } else if(state->current_view == VIEW_AERIAL) {
                        state->last_aerial_category_index = current_index;
                    }
                    execute_menu_command(state, &commands[current_index]);
                }
                consumed = true;
            }
            break;

        case InputKeyBack:
            if(state->current_view == VIEW_STATUS_IDLE) {
                navigate_to_view(state, VIEW_SETTINGS_ACTIONS);
            } else if(state->current_view >= VIEW_WIFI_SCANNING && state->current_view <= VIEW_AERIAL) {
                if(state->came_from_settings && state->current_view <= VIEW_WIFI_SETTINGS) {
                    navigate_to_view(state, VIEW_SETTINGS_ACTIONS);
                } else {
                    navigate_to_view(state, VIEW_WIFI);
                }
            } else if(
                state->current_view >= VIEW_BLE_SCANNING &&
                state->current_view <= VIEW_BLE_ATTACK) {
                navigate_to_view(state, VIEW_BLE);
            } else if(
                (state->current_view >= VIEW_WIFI && state->current_view <= VIEW_GPS) ||
                state->current_view == VIEW_IR) {
                navigate_to_view(state, VIEW_MAIN);
            }
            consumed = true;
            break;

        case InputKeyRight:
        case InputKeyLeft:
            // Handle sniff command cycling
            if(state->current_view == VIEW_WIFI_CAPTURE && current_index == 0) {
                // sniff_commands is not CyclingMenuDef, so keep legacy logic for now
                if(event->key == InputKeyRight) {
                    current_sniff_index = (current_sniff_index + 1) % COUNT_OF(sniff_commands);
                } else {
                    current_sniff_index = (current_sniff_index == 0) ?
                                              (size_t)(COUNT_OF(sniff_commands) - 1) :
                                              (current_sniff_index - 1);
                }
                submenu_change_item_label(
                    current_menu, current_index, sniff_commands[current_sniff_index].label);
                consumed = true;
            }
            // Handle beacon spam command cycling
            else if(state->current_view == VIEW_WIFI_ATTACK && current_index == 0) {
                consumed = cycle_menu_item(
                    (CyclingMenuDef*)beacon_spam_commands,
                    COUNT_OF(beacon_spam_commands),
                    &current_beacon_index,
                    (MenuCommand*)wifi_attack_commands,
                    0,
                    current_menu,
                    event);
            }
            // Handle rgbmode command cycling (new branch for index 17)
            else if(state->current_view == VIEW_WIFI_SETTINGS && current_index == 0) {
                consumed = cycle_menu_item(
                    (CyclingMenuDef*)rgbmode_commands,
                    COUNT_OF(rgbmode_commands),
                    &current_rgb_index,
                    (MenuCommand*)wifi_settings_commands,
                    0,
                    current_menu,
                    event);
            }
            // Handle BLE spam command cycling
            else if(state->current_view == VIEW_BLE_ATTACK && current_index == 0) {
                consumed = cycle_menu_item(
                    (CyclingMenuDef*)ble_spam_commands,
                    COUNT_OF(ble_spam_commands),
                    &current_ble_spam_index,
                    (MenuCommand*)ble_attack_commands,
                    0,
                    current_menu,
                    event);
            }
            // Handle WiFi scan mode cycling
            else if(state->current_view == VIEW_WIFI_SCANNING && current_index == 0) {
                consumed = cycle_menu_item(
                    (CyclingMenuDef*)wifi_scan_modes,
                    COUNT_OF(wifi_scan_modes),
                    &current_wifi_scan_index,
                    (MenuCommand*)wifi_scanning_commands,
                    0,
                    current_menu,
                    event);
            }
            // List mode cycling
            else if(state->current_view == VIEW_WIFI_SCANNING && current_index == 1) {
                consumed = cycle_menu_item(
                    (CyclingMenuDef*)wifi_list_modes,
                    COUNT_OF(wifi_list_modes),
                    &current_wifi_list_index,
                    (MenuCommand*)wifi_scanning_commands,
                    1,
                    current_menu,
                    event);
            }
            // Select mode cycling
            else if(state->current_view == VIEW_WIFI_SCANNING && current_index == 2) {
                consumed = cycle_menu_item(
                    (CyclingMenuDef*)wifi_select_modes,
                    COUNT_OF(wifi_select_modes),
                    &current_wifi_select_index,
                    (MenuCommand*)wifi_scanning_commands,
                    2,
                    current_menu,
                    event);
            }
            // Handle listen mode cycling
            else if(state->current_view == VIEW_WIFI_SCANNING && current_index == 3) {
                consumed = cycle_menu_item(
                    (CyclingMenuDef*)wifi_listen_modes,
                    COUNT_OF(wifi_listen_modes),
                    &current_wifi_listen_index,
                    (MenuCommand*)wifi_scanning_commands,
                    3,
                    current_menu,
                    event);
            }
            break;
        case InputKeyMAX:
            break;
        }
        break;

    case InputTypeLong:
        switch(event->key) {
        case InputKeyUp:
        case InputKeyDown:
        case InputKeyRight:
        case InputKeyLeft:
        case InputKeyBack:
        case InputKeyMAX:
            break;

        case InputKeyOk:
            if(current_index < commands_count) {
                const MenuCommand* command = &commands[current_index];
                if(command->details_header && command->details_text) {
                    show_command_details(state, command);
                    consumed = true;
                }
            }
            break;
        }
        break;

    case InputTypeRepeat:
        switch(event->key) {
        case InputKeyUp:
            if(current_index > 0) {
                submenu_set_selected_item(current_menu, current_index - 1);
            } else {
                // Wrap to bottom
                submenu_set_selected_item(current_menu, commands_count - 1);
            }
            consumed = true;
            break;

        case InputKeyDown:
            if(current_index < commands_count - 1) {
                submenu_set_selected_item(current_menu, current_index + 1);
            } else {
                // Wrap to top
                submenu_set_selected_item(current_menu, 0);
            }
            consumed = true;
            break;

        case InputKeyRight:
        case InputKeyLeft:
        case InputKeyOk:
        case InputKeyBack:
        case InputKeyMAX:
            break;
        }
        break;

    case InputTypePress:
    case InputTypeRelease:
    case InputTypeMAX:
        break;
    }

    return consumed;
}

// 6675636B796F7564656B69
