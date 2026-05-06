#include <ctype.h>
#include <furi.h>
#include <furi/core/timer.h>
#include <furi/core/log.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/canvas.h>
#include <gui/elements.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/number_input.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include <gui/modules/widget.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include <lib/nfc/nfc.h>
#include <nfc/nfc_poller.h>
#include <nfc/nfc_listener.h>
#include <nfc/nfc_device.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight_poller.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight_listener.h>
#include <nfc/helpers/nfc_data_generator.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flip_crypt_icons.h"

#include "ciphers/aes.h"
#include "ciphers/affine.h"
#include "ciphers/atbash.h"
#include "ciphers/baconian.h"
#include "ciphers/beaufort.h"
#include "ciphers/caesar.h"
#include "ciphers/playfair.h"
#include "ciphers/polybius.h"
#include "ciphers/porta.h"
#include "ciphers/railfence.h"
#include "ciphers/rc4.h"
#include "ciphers/rot13.h"
#include "ciphers/scytale.h"
#include "ciphers/vigenere.h"

#include "hashes/blake2.h"
#include "hashes/fnv.h"
#include "hashes/md2.h"
#include "hashes/md5.h"
#include "hashes/murmur3.h"
#include "hashes/sha1.h"
#include "hashes/sha2.h"
#include "hashes/siphash.h"
#include "hashes/xxhash.h"

#include "encoders/base32.h"
#include "encoders/base58.h"
#include "encoders/base64.h"

#include "qrcode/qrcodegen.h"
#include "storage.h"

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define FLIP_CRYPT_UI_TEXT(en, zh) (zh)
#else
#define FLIP_CRYPT_UI_TEXT(en, zh) (en)
#endif

// Scene declarations
typedef enum {
    // Main menu
    FlipCryptMainMenuScene,
    // Main Menu Submenus
    FlipCryptCipherSubmenuScene,
    FlipCryptHashSubmenuScene,
    FlipCryptOtherSubmenuScene,
    FlipCryptAboutScene,
    // AES scenes
    FlipCryptAESSubmenuScene,
    FlipCryptAESInputScene,
    FlipCryptAESKeyInputScene,
    FlipCryptAESDecryptKeyInputScene,
    FlipCryptAESOutputScene,
    FlipCryptAESDecryptInputScene,
    FlipCryptAESDecryptOutputScene,
    FlipCryptAESLearnScene,
    // Affine scenes
    FlipCryptAffineSubmenuScene,
    FlipCryptAffineInputScene,
    FlipCryptAffineKeyAInputScene,
    FlipCryptAffineDecryptKeyAInputScene,
    FlipCryptAffineKeyBInputScene,
    FlipCryptAffineDecryptKeyBInputScene,
    FlipCryptAffineOutputScene,
    FlipCryptAffineDecryptInputScene,
    FlipCryptAffineDecryptOutputScene,
    FlipCryptAffineLearnScene,
    // Atbash scenes
    FlipCryptAtbashSubmenuScene,
    FlipCryptAtbashInputScene,
    FlipCryptAtbashOutputScene,
    FlipCryptAtbashDecryptInputScene,
    FlipCryptAtbashDecryptOutputScene,
    FlipCryptAtbashLearnScene,
    // Baconian scenes
    FlipCryptBaconianSubmenuScene,
    FlipCryptBaconianInputScene,
    FlipCryptBaconianOutputScene,
    FlipCryptBaconianDecryptInputScene,
    FlipCryptBaconianDecryptOutputScene,
    FlipCryptBaconianLearnScene,
    // Beaufort scenes
    FlipCryptBeaufortSubmenuScene,
    FlipCryptBeaufortInputScene,
    FlipCryptBeaufortKeyInputScene,
    FlipCryptBeaufortDecryptKeyInputScene,
    FlipCryptBeaufortOutputScene,
    FlipCryptBeaufortDecryptInputScene,
    FlipCryptBeaufortDecryptOutputScene,
    FlipCryptBeaufortLearnScene,
    // Caesar scenes
    FlipCryptCaesarSubmenuScene,
    FlipCryptCaesarInputScene,
    FlipCryptCaesarKeyInputScene,
    FlipCryptCaesarDecryptKeyInputScene,
    FlipCryptCaesarOutputScene,
    FlipCryptCaesarDecryptInputScene,
    FlipCryptCaesarDecryptOutputScene,
    FlipCryptCaesarLearnScene,
    // Playfair scenes
    FlipCryptPlayfairSubmenuScene,
    FlipCryptPlayfairInputScene,
    FlipCryptPlayfairKeywordInputScene,
    FlipCryptPlayfairDecryptKeywordInputScene,
    FlipCryptPlayfairOutputScene,
    FlipCryptPlayfairDecryptInputScene,
    FlipCryptPlayfairDecryptOutputScene,
    FlipCryptPlayfairLearnScene,
    // Polybius square scenes
    FlipCryptPolybiusSubmenuScene,
    FlipCryptPolybiusInputScene,
    FlipCryptPolybiusOutputScene,
    FlipCryptPolybiusDecryptInputScene,
    FlipCryptPolybiusDecryptOutputScene,
    FlipCryptPolybiusLearnScene,
    // Porta scenes
    FlipCryptPortaSubmenuScene,
    FlipCryptPortaInputScene,
    FlipCryptPortaKeywordInputScene,
    FlipCryptPortaDecryptKeywordInputScene,
    FlipCryptPortaOutputScene,
    FlipCryptPortaDecryptInputScene,
    FlipCryptPortaDecryptOutputScene,
    FlipCryptPortaLearnScene,
    // Railfence scenes
    FlipCryptRailfenceSubmenuScene,
    FlipCryptRailfenceInputScene,
    FlipCryptRailfenceKeyInputScene,
    FlipCryptRailfenceDecryptKeyInputScene,
    FlipCryptRailfenceOutputScene,
    FlipCryptRailfenceDecryptInputScene,
    FlipCryptRailfenceDecryptOutputScene,
    FlipCryptRailfenceLearnScene,
    // RC4 Cipher
    FlipCryptRC4SubmenuScene,
    FlipCryptRC4InputScene,
    FlipCryptRC4KeywordInputScene,
    FlipCryptRC4DecryptKeywordInputScene,
    FlipCryptRC4OutputScene,
    FlipCryptRC4DecryptInputScene,
    FlipCryptRC4DecryptOutputScene,
    FlipCryptRC4LearnScene,
    // ROT13 scenes
    FlipCryptROT13SubmenuScene,
    FlipCryptROT13InputScene,
    FlipCryptROT13OutputScene,
    FlipCryptROT13DecryptInputScene,
    FlipCryptROT13DecryptOutputScene,
    FlipCryptROT13LearnScene,
    // Scytale Cipher
    FlipCryptScytaleSubmenuScene,
    FlipCryptScytaleInputScene,
    FlipCryptScytaleKeywordInputScene,
    FlipCryptScytaleDecryptKeywordInputScene,
    FlipCryptScytaleOutputScene,
    FlipCryptScytaleDecryptInputScene,
    FlipCryptScytaleDecryptOutputScene,
    FlipCryptScytaleLearnScene,
    // Vigenere Cipher
    FlipCryptVigenereSubmenuScene,
    FlipCryptVigenereInputScene,
    FlipCryptVigenereKeywordInputScene,
    FlipCryptVigenereDecryptKeywordInputScene,
    FlipCryptVigenereOutputScene,
    FlipCryptVigenereDecryptInputScene,
    FlipCryptVigenereDecryptOutputScene,
    FlipCryptVigenereLearnScene,
    // BLAKE2 Hash
    FlipCryptBlake2SubmenuScene,
    FlipCryptBlake2InputScene,
    FlipCryptBlake2OutputScene,
    FlipCryptBlake2LearnScene,
    // FNV-1A Hash
    FlipCryptFNV1ASubmenuScene,
    FlipCryptFNV1AInputScene,
    FlipCryptFNV1AOutputScene,
    FlipCryptFNV1ALearnScene,
    // MD2 Hash
    FlipCryptMD2SubmenuScene,
    FlipCryptMD2InputScene,
    FlipCryptMD2OutputScene,
    FlipCryptMD2LearnScene,
    // MD5 Hash
    FlipCryptMD5SubmenuScene,
    FlipCryptMD5InputScene,
    FlipCryptMD5OutputScene,
    FlipCryptMD5LearnScene,
    // Murmur3 Hash
    FlipCryptMurmur3SubmenuScene,
    FlipCryptMurmur3InputScene,
    FlipCryptMurmur3OutputScene,
    FlipCryptMurmur3LearnScene,
    // SipHash
    FlipCryptSipSubmenuScene,
    FlipCryptSipInputScene,
    FlipCryptSipKeywordInputScene,
    FlipCryptSipOutputScene,
    FlipCryptSipLearnScene,
    // SHA1 Hash
    FlipCryptSHA1SubmenuScene,
    FlipCryptSHA1InputScene,
    FlipCryptSHA1OutputScene,
    FlipCryptSHA1LearnScene,
    // SHA224 Hash
    FlipCryptSHA224SubmenuScene,
    FlipCryptSHA224InputScene,
    FlipCryptSHA224OutputScene,
    FlipCryptSHA224LearnScene,
    // SHA256 Hash
    FlipCryptSHA256SubmenuScene,
    FlipCryptSHA256InputScene,
    FlipCryptSHA256OutputScene,
    FlipCryptSHA256LearnScene,
    // SHA384 Hash
    FlipCryptSHA384SubmenuScene,
    FlipCryptSHA384InputScene,
    FlipCryptSHA384OutputScene,
    FlipCryptSHA384LearnScene,
    // SHA512 Hash
    FlipCryptSHA512SubmenuScene,
    FlipCryptSHA512InputScene,
    FlipCryptSHA512OutputScene,
    FlipCryptSHA512LearnScene,
    // XX Hash
    FlipCryptXXSubmenuScene,
    FlipCryptXXInputScene,
    FlipCryptXXOutputScene,
    FlipCryptXXLearnScene,
    // Base32 scenes
    FlipCryptBase32SubmenuScene,
    FlipCryptBase32InputScene,
    FlipCryptBase32OutputScene,
    FlipCryptBase32DecryptInputScene,
    FlipCryptBase32DecryptOutputScene,
    FlipCryptBase32LearnScene,
    // Base58 scenes
    FlipCryptBase58SubmenuScene,
    FlipCryptBase58InputScene,
    FlipCryptBase58OutputScene,
    FlipCryptBase58DecryptInputScene,
    FlipCryptBase58DecryptOutputScene,
    FlipCryptBase58LearnScene,
    // Base64 scenes
    FlipCryptBase64SubmenuScene,
    FlipCryptBase64InputScene,
    FlipCryptBase64OutputScene,
    FlipCryptBase64DecryptInputScene,
    FlipCryptBase64DecryptOutputScene,
    FlipCryptBase64LearnScene,
    // Extra actions scenes
    FlipCryptNFCScene,
    FlipCryptSaveScene,
    FlipCryptSaveTextInputScene,
    FlipCryptQRScene,
    FlipCryptSceneCount,
} FlipCryptScene;

// View type declarations
typedef enum {
    FlipCryptSubmenuView,
    FlipCryptWidgetView,
    FlipCryptTextInputView,
    FlipCryptNumberInputView,
    FlipCryptDialogExView,
    FlipCryptCanvasView,
} FlipCryptView;

// View and variable inits
typedef struct App {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    Widget* widget;
    TextInput* text_input;
    NumberInput* number_input;
    DialogEx* dialog_ex;
    Nfc* nfc;
    NfcListener* listener;
    NfcDevice* nfc_device;
    char* universal_input;
    uint8_t universal_input_size;
    char* save_name_input;
    char* last_output_scene;
    char* aes_key_input;
    int32_t affine_keya_input;
    int32_t affine_keyb_input;
    char* beaufort_key_input;
    int32_t caesar_key_input;
    char* playfair_keyword_input;
    char* porta_keyword_input;
    int32_t railfence_key_input;
    char* rc4_keyword_input;
    int32_t scytale_keyword_input;
    char* vigenere_keyword_input;
    char* sip_keyword_input;
    uint8_t save_name_input_size;
    uint8_t last_output_scene_size;
    uint8_t aes_key_input_size;
    uint8_t beaufort_key_input_size;
    uint8_t playfair_keyword_input_size;
    uint8_t porta_keyword_input_size;
    uint8_t rc4_keyword_input_size;
    uint8_t vigenere_keyword_input_size;
    uint8_t sip_keyword_input_size;
    uint8_t* qr_buffer;
    uint8_t* qrcode;
} App;

// Menu item indicies
typedef enum {
    MainMenuIndexCiphers,
    MainMenuIndexHashes,
    MainMenuIndexOther,
    MainMenuIndexAbout,
    MenuIndexAES,
    MenuIndexAffine,
    MenuIndexAtbash,
    MenuIndexBaconian,
    MenuIndexBeaufort,
    MenuIndexCaesar,
    MenuIndexPlayfair,
    MenuIndexPolybius,
    MenuIndexPorta,
    MenuIndexRailfence,
    MenuIndexRC4,
    MenuIndexROT13,
    MenuIndexScytale,
    MenuIndexVigenere,
    MenuIndexBlake2,
    MenuIndexFNV1A,
    MenuIndexMD2,
    MenuIndexMD5,
    MenuIndexMurmur3,
    MenuIndexSHA1,
    MenuIndexSHA224,
    MenuIndexSHA256,
    MenuIndexSHA384,
    MenuIndexSHA512,
    MenuIndexSip,
    MenuIndexXX,
    MenuIndexBase32,
    MenuIndexBase58,
    MenuIndexBase64,
} FlipCryptMenuIndices;

// Menu item events
typedef enum {
    EventCiphers,
    EventHashes,
    EventOther,
    EventAbout,
    EventAES,
    EventAffine,
    EventAtbash,
    EventBaconian,
    EventBeaufort,
    EventCaesar,
    EventPlayfair,
    EventPolybius,
    EventPorta,
    EventRailfence,
    EventRC4,
    EventROT13,
    EventScytale,
    EventVigenere,
    EventBlake2,
    EventFNV1A,
    EventMD2,
    EventMD5,
    EventMurmur3,
    EventSHA1,
    EventSHA224,
    EventSHA256,
    EventSHA384,
    EventSHA512,
    EventSip,
    EventXX,
    EventBase32,
    EventBase58,
    EventBase64,
} FlipCryptMenuCustomEvents;

// QR Code vars
typedef struct {
    uint8_t* qr_buffer;
    uint8_t* qrcode;
} QrCodeModel;

// Main menu functionality
void flip_crypt_menu_callback(void* context, uint32_t index) {
    App* app = context;
    switch(index) { 
        case MainMenuIndexCiphers:
            scene_manager_handle_custom_event(app->scene_manager, EventCiphers);
            break;
        case MainMenuIndexHashes:
            scene_manager_handle_custom_event(app->scene_manager, EventHashes);
            break;
        case MainMenuIndexOther:
            scene_manager_handle_custom_event(app->scene_manager, EventOther);
            break;
        case MainMenuIndexAbout:
            scene_manager_handle_custom_event(app->scene_manager, EventAbout);
            break;
        case MenuIndexAES:
            scene_manager_handle_custom_event(app->scene_manager, EventAES);
            break;
        case MenuIndexAffine:
            scene_manager_handle_custom_event(app->scene_manager, EventAffine);
            break;
        case MenuIndexAtbash:
            scene_manager_handle_custom_event(app->scene_manager, EventAtbash);
            break;
        case MenuIndexBaconian:
            scene_manager_handle_custom_event(app->scene_manager, EventBaconian);
            break;
        case MenuIndexBeaufort:
            scene_manager_handle_custom_event(app->scene_manager, EventBeaufort);
            break;
        case MenuIndexCaesar:
            scene_manager_handle_custom_event(app->scene_manager, EventCaesar);
            break;
        case MenuIndexPlayfair:
            scene_manager_handle_custom_event(app->scene_manager, EventPlayfair);
            break;
        case MenuIndexPolybius:
            scene_manager_handle_custom_event(app->scene_manager, EventPolybius);
            break;
        case MenuIndexPorta:
            scene_manager_handle_custom_event(app->scene_manager, EventPorta);
            break;
        case MenuIndexRailfence:
            scene_manager_handle_custom_event(app->scene_manager, EventRailfence);
            break;
        case MenuIndexRC4:
            scene_manager_handle_custom_event(app->scene_manager, EventRC4);
            break;
        case MenuIndexROT13:
            scene_manager_handle_custom_event(app->scene_manager, EventROT13);
            break;
        case MenuIndexScytale:
            scene_manager_handle_custom_event(app->scene_manager, EventScytale);
            break;
        case MenuIndexVigenere:
            scene_manager_handle_custom_event(app->scene_manager, EventVigenere);
            break;
        case MenuIndexBlake2:
            scene_manager_handle_custom_event(app->scene_manager, EventBlake2);
            break;
        case MenuIndexFNV1A:
            scene_manager_handle_custom_event(app->scene_manager, EventFNV1A);
            break;
        case MenuIndexMD2:
            scene_manager_handle_custom_event(app->scene_manager, EventMD2);
            break;
        case MenuIndexMD5:
            scene_manager_handle_custom_event(app->scene_manager, EventMD5);
            break;
        case MenuIndexMurmur3:
            scene_manager_handle_custom_event(app->scene_manager, EventMurmur3);
            break;
        case MenuIndexSip:
            scene_manager_handle_custom_event(app->scene_manager, EventSip);
            break;
        case MenuIndexSHA1:
            scene_manager_handle_custom_event(app->scene_manager, EventSHA1);
            break;
        case MenuIndexSHA224:
            scene_manager_handle_custom_event(app->scene_manager, EventSHA224);
            break;
        case MenuIndexSHA256:
            scene_manager_handle_custom_event(app->scene_manager, EventSHA256);
            break;
        case MenuIndexSHA384:
            scene_manager_handle_custom_event(app->scene_manager, EventSHA384);
            break;
        case MenuIndexSHA512:
            scene_manager_handle_custom_event(app->scene_manager, EventSHA512);
            break;
        case MenuIndexXX:
            scene_manager_handle_custom_event(app->scene_manager, EventXX);
            break;
        case MenuIndexBase32:
            scene_manager_handle_custom_event(app->scene_manager, EventBase32);
            break;
        case MenuIndexBase58:
            scene_manager_handle_custom_event(app->scene_manager, EventBase58);
            break;
        case MenuIndexBase64:
            scene_manager_handle_custom_event(app->scene_manager, EventBase64);
            break;
    }
}

// Main menu initialization
void flip_crypt_main_menu_scene_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "FlipCrypt");
    submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Ciphers", "密码算法"), MainMenuIndexCiphers, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Hashes", "哈希算法"), MainMenuIndexHashes, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Other", "其他"), MainMenuIndexOther, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("About", "关于"), MainMenuIndexAbout, flip_crypt_menu_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptSubmenuView);
}

void flip_crypt_cipher_submenu_scene_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, FLIP_CRYPT_UI_TEXT("Ciphers", "密码算法"));
    submenu_add_item(app->submenu, "AES-128", MenuIndexAES, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Affine Cipher", "仿射密码"), MenuIndexAffine, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Atbash Cipher", "Atbash密码"), MenuIndexAtbash, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Baconian Cipher", "培根密码"), MenuIndexBaconian, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Beaufort Cipher", "Beaufort密码"), MenuIndexBeaufort, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Caesar Cipher", "凯撒密码"), MenuIndexCaesar, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Playfair Cipher", "Playfair密码"), MenuIndexPlayfair, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Polybius Square", "Polybius方阵"), MenuIndexPolybius, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Porta Cipher", "Porta密码"), MenuIndexPorta, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Railfence Cipher", "栅栏密码"), MenuIndexRailfence, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, "RC4", MenuIndexRC4, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, "ROT-13", MenuIndexROT13, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Scytale Cipher", "斯巴达密码棒"), MenuIndexScytale, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Vigenere Cipher", "维吉尼亚密码"), MenuIndexVigenere, flip_crypt_menu_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptSubmenuView);
}

void flip_crypt_hash_submenu_scene_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, FLIP_CRYPT_UI_TEXT("Hashes", "哈希算法"));
    submenu_add_item(app->submenu, "BLAKE-2s", MenuIndexBlake2, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, "FNV-1A", MenuIndexFNV1A, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, "MD2", MenuIndexMD2, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, "MD5", MenuIndexMD5, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, "MurmurHash3", MenuIndexMurmur3, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, "SHA-1", MenuIndexSHA1, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, "SHA-224", MenuIndexSHA224, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, "SHA-256", MenuIndexSHA256, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, "SHA-384", MenuIndexSHA384, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, "SHA-512", MenuIndexSHA512, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, "SipHash", MenuIndexSip, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, "XXHash64", MenuIndexXX, flip_crypt_menu_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptSubmenuView);
}

void flip_crypt_other_submenu_scene_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, FLIP_CRYPT_UI_TEXT("Other", "其他"));
    submenu_add_item(app->submenu, "Base32", MenuIndexBase32, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, "Base58", MenuIndexBase58, flip_crypt_menu_callback, app);
    submenu_add_item(app->submenu, "Base64", MenuIndexBase64, flip_crypt_menu_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptSubmenuView);
}

// More main menu functionality
bool flip_crypt_main_menu_scene_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
            case EventCiphers:
                scene_manager_next_scene(app->scene_manager, FlipCryptCipherSubmenuScene);
                consumed = true;
                break;
            case EventHashes:
                scene_manager_next_scene(app->scene_manager, FlipCryptHashSubmenuScene);
                consumed = true;
                break;
            case EventOther:
                scene_manager_next_scene(app->scene_manager, FlipCryptOtherSubmenuScene);
                consumed = true;
                break;
            case EventAbout:
                scene_manager_next_scene(app->scene_manager, FlipCryptAboutScene);
                consumed = true;
                break;
            case EventAES:
                scene_manager_next_scene(app->scene_manager, FlipCryptAESSubmenuScene);
                consumed = true;
                break;
            case EventAffine:
                scene_manager_next_scene(app->scene_manager, FlipCryptAffineSubmenuScene);
                consumed = true;
                break;
            case EventAtbash:
                scene_manager_next_scene(app->scene_manager, FlipCryptAtbashSubmenuScene);
                consumed = true;
                break;
            case EventBaconian:
                scene_manager_next_scene(app->scene_manager, FlipCryptBaconianSubmenuScene);
                consumed = true;
                break;
            case EventBeaufort:
                scene_manager_next_scene(app->scene_manager, FlipCryptBeaufortSubmenuScene);
                consumed = true;
                break;
            case EventCaesar:
                scene_manager_next_scene(app->scene_manager, FlipCryptCaesarSubmenuScene);
                consumed = true;
                break;
            case EventPlayfair:
                scene_manager_next_scene(app->scene_manager, FlipCryptPlayfairSubmenuScene);
                consumed = true;
                break;
            case EventPolybius:
                scene_manager_next_scene(app->scene_manager, FlipCryptPolybiusSubmenuScene);
                consumed = true;
                break;
            case EventPorta:
                scene_manager_next_scene(app->scene_manager, FlipCryptPortaSubmenuScene);
                consumed = true;
                break;
            case EventRailfence:
                scene_manager_next_scene(app->scene_manager, FlipCryptRailfenceSubmenuScene);
                consumed = true;
                break;
            case EventRC4:
                scene_manager_next_scene(app->scene_manager, FlipCryptRC4SubmenuScene);
                consumed = true;
                break;
            case EventROT13:
                scene_manager_next_scene(app->scene_manager, FlipCryptROT13SubmenuScene);
                consumed = true;
                break;
            case EventScytale:
                scene_manager_next_scene(app->scene_manager, FlipCryptScytaleSubmenuScene);
                consumed = true;
                break;
            case EventVigenere:
                scene_manager_next_scene(app->scene_manager, FlipCryptVigenereSubmenuScene);
                consumed = true;
                break;
            case EventBlake2:
                scene_manager_next_scene(app->scene_manager, FlipCryptBlake2SubmenuScene);
                consumed = true;
                break;
            case EventFNV1A:
                scene_manager_next_scene(app->scene_manager, FlipCryptFNV1ASubmenuScene);
                consumed = true;
                break;
            case EventMD2:
                scene_manager_next_scene(app->scene_manager, FlipCryptMD2SubmenuScene);
                consumed = true;
                break;
            case EventMD5:
                scene_manager_next_scene(app->scene_manager, FlipCryptMD5SubmenuScene);
                consumed = true;
                break;
            case EventMurmur3:
                scene_manager_next_scene(app->scene_manager, FlipCryptMurmur3SubmenuScene);
                consumed = true;
                break;
            case EventSip:
                scene_manager_next_scene(app->scene_manager, FlipCryptSipSubmenuScene);
                consumed = true;
                break;
            case EventSHA1:
                scene_manager_next_scene(app->scene_manager, FlipCryptSHA1SubmenuScene);
                consumed = true;
                break;
            case EventSHA224:
                scene_manager_next_scene(app->scene_manager, FlipCryptSHA224SubmenuScene);
                consumed = true;
                break;
            case EventSHA256:
                scene_manager_next_scene(app->scene_manager, FlipCryptSHA256SubmenuScene);
                consumed = true;
                break;
            case EventSHA384:
                scene_manager_next_scene(app->scene_manager, FlipCryptSHA384SubmenuScene);
                consumed = true;
                break;
            case EventSHA512:
                scene_manager_next_scene(app->scene_manager, FlipCryptSHA512SubmenuScene);
                consumed = true;
                break;
            case EventXX:
                scene_manager_next_scene(app->scene_manager, FlipCryptXXSubmenuScene);
                consumed = true;
                break;
            case EventBase32:
                scene_manager_next_scene(app->scene_manager, FlipCryptBase32SubmenuScene);
                consumed = true;
                break;
            case EventBase58:
                scene_manager_next_scene(app->scene_manager, FlipCryptBase58SubmenuScene);
                consumed = true;
                break;
            case EventBase64:
                scene_manager_next_scene(app->scene_manager, FlipCryptBase64SubmenuScene);
                consumed = true;
                break;
        }
    }
    return consumed;
}

void flip_crypt_main_menu_scene_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}

// Cipher / Hash 'Encrypt' submenu option functionality
void cipher_encrypt_submenu_callback(void* context, uint32_t index) {
    UNUSED(index);
    UNUSED(context);
    App* app = context;
    FlipCryptScene current = scene_manager_get_current_scene(app->scene_manager);
    switch(current) {
        case FlipCryptAESSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAESKeyInputScene);
            break;
        case FlipCryptAffineSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAffineKeyAInputScene);
            break;
        case FlipCryptAtbashSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAtbashInputScene);
            break;
        case FlipCryptBaconianSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBaconianInputScene);
            break;
        case FlipCryptBeaufortSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBeaufortKeyInputScene);
            break;
        case FlipCryptCaesarSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptCaesarKeyInputScene);
            break;
        case FlipCryptPlayfairSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPlayfairKeywordInputScene);
            break;
        case FlipCryptPolybiusSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPolybiusInputScene);
            break;
        case FlipCryptPortaSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPortaKeywordInputScene);
            break;
        case FlipCryptRailfenceSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptRailfenceKeyInputScene);
            break;
        case FlipCryptRC4SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptRC4KeywordInputScene);
            break;
        case FlipCryptROT13SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptROT13InputScene);
            break;
        case FlipCryptScytaleSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptScytaleKeywordInputScene);
            break;
        case FlipCryptVigenereSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptVigenereKeywordInputScene);
            break;
        case FlipCryptBlake2SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBlake2InputScene);
            break;
        case FlipCryptFNV1ASubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptFNV1AInputScene);
            break;
        case FlipCryptMD2SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptMD2InputScene);
            break;
        case FlipCryptMD5SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptMD5InputScene);
            break;
        case FlipCryptMurmur3SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptMurmur3InputScene);
            break;
        case FlipCryptSipSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSipKeywordInputScene);
            break;
        case FlipCryptSHA1SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSHA1InputScene);
            break;
        case FlipCryptSHA224SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSHA224InputScene);
            break;
        case FlipCryptSHA256SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSHA256InputScene);
            break;
        case FlipCryptSHA384SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSHA384InputScene);
            break;
        case FlipCryptSHA512SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSHA512InputScene);
            break;
        case FlipCryptXXSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptXXInputScene);
            break;
        case FlipCryptBase32SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBase32InputScene);
            break;
        case FlipCryptBase58SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBase58InputScene);
            break;
        case FlipCryptBase64SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBase64InputScene);
            break;
        default:
            break;
    }
}

// Cipher / Hash 'Decrypt' submenu option functionality
void cipher_decrypt_submenu_callback(void* context, uint32_t index) {
    UNUSED(index);
    UNUSED(context);
    App* app = context;
    FlipCryptScene current = scene_manager_get_current_scene(app->scene_manager);
    switch(current) {
        case FlipCryptAESSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAESDecryptKeyInputScene);
            break;
        case FlipCryptAffineSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAffineDecryptKeyAInputScene);
            break;
        case FlipCryptAtbashSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAtbashDecryptInputScene);
            break;
        case FlipCryptBaconianSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBaconianDecryptInputScene);
            break;
        case FlipCryptBeaufortSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBeaufortDecryptKeyInputScene);
            break;
        case FlipCryptCaesarSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptCaesarDecryptKeyInputScene);
            break;
        case FlipCryptPlayfairSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPlayfairDecryptKeywordInputScene);
            break;
        case FlipCryptPolybiusSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPolybiusDecryptInputScene);
            break;
        case FlipCryptPortaSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPortaDecryptKeywordInputScene);
            break;
        case FlipCryptRailfenceSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptRailfenceDecryptKeyInputScene);
            break;
        case FlipCryptRC4SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptRC4DecryptKeywordInputScene);
            break;
        case FlipCryptROT13SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptROT13DecryptInputScene);
            break;
        case FlipCryptScytaleSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptScytaleDecryptKeywordInputScene);
            break;
        case FlipCryptVigenereSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptVigenereDecryptKeywordInputScene);
            break;
        // Can't decrypt hashes so they are absent
        case FlipCryptBase32SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBase32DecryptInputScene);
            break;
        case FlipCryptBase58SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBase58DecryptInputScene);
            break;
        case FlipCryptBase64SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBase64DecryptInputScene);
            break;
        default:
            break;
    }
}

// Cipher / Hash 'Learn' submenu option functionality
void cipher_learn_submenu_callback(void* context, uint32_t index) {
    UNUSED(index);
    UNUSED(context);
    App* app = context;
    FlipCryptScene current = scene_manager_get_current_scene(app->scene_manager);
    switch(current) {
        case FlipCryptAESSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAESLearnScene);
            break;
        case FlipCryptAffineSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAffineLearnScene);
            break;
        case FlipCryptAtbashSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAtbashLearnScene);
            break;
        case FlipCryptBaconianSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBaconianLearnScene);
            break;
        case FlipCryptBeaufortSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBeaufortLearnScene);
            break;
        case FlipCryptCaesarSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptCaesarLearnScene);
            break;
        case FlipCryptPlayfairSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPlayfairLearnScene);
            break;
        case FlipCryptPolybiusSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPolybiusLearnScene);
            break;
        case FlipCryptPortaSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPortaLearnScene);
            break;
        case FlipCryptRailfenceSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptRailfenceLearnScene);
            break;
        case FlipCryptRC4SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptRC4LearnScene);
            break;
        case FlipCryptROT13SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptROT13LearnScene);
            break;
        case FlipCryptScytaleSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptScytaleLearnScene);
            break;
        case FlipCryptVigenereSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptVigenereLearnScene);
            break;
        case FlipCryptBlake2SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBlake2LearnScene);
            break;
        case FlipCryptFNV1ASubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptFNV1ALearnScene);
            break;
        case FlipCryptMD2SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptMD2LearnScene);
            break;
        case FlipCryptMD5SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptMD5LearnScene);
            break;
        case FlipCryptMurmur3SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptMurmur3LearnScene);
            break;
        case FlipCryptSipSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSipLearnScene);
            break;
        case FlipCryptSHA1SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSHA1LearnScene);
            break;
        case FlipCryptSHA224SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSHA224LearnScene);
            break;
        case FlipCryptSHA256SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSHA256LearnScene);
            break;
        case FlipCryptSHA384SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSHA384LearnScene);
            break;
        case FlipCryptSHA512SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSHA512LearnScene);
            break;
        case FlipCryptXXSubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptXXLearnScene);
            break;
        case FlipCryptBase32SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBase32LearnScene);
            break;
        case FlipCryptBase58SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBase58LearnScene);
            break;
        case FlipCryptBase64SubmenuScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBase64LearnScene);
            break;
        default:
            break;
    }
}

// Number input stuff
void number_input_scene_callback(void* context, int32_t number) {
    App* app = context;
    FlipCryptScene current = scene_manager_get_current_scene(app->scene_manager);
    switch(current) {
        case FlipCryptAffineKeyAInputScene:
            app->affine_keya_input = number;
            scene_manager_next_scene(app->scene_manager, FlipCryptAffineKeyBInputScene);
            break;
        case FlipCryptAffineKeyBInputScene:
            app->affine_keyb_input = number;
            scene_manager_next_scene(app->scene_manager, FlipCryptAffineInputScene);
            break;
        case FlipCryptAffineDecryptKeyAInputScene:
            app->affine_keya_input = number;
            scene_manager_next_scene(app->scene_manager, FlipCryptAffineDecryptKeyBInputScene);
            break;
        case FlipCryptAffineDecryptKeyBInputScene:
            app->affine_keyb_input = number;
            scene_manager_next_scene(app->scene_manager, FlipCryptAffineDecryptInputScene);
            break;
        case FlipCryptCaesarKeyInputScene:
            app->caesar_key_input = number;
            scene_manager_next_scene(app->scene_manager, FlipCryptCaesarInputScene);
            break;
        case FlipCryptCaesarDecryptKeyInputScene:
            app->caesar_key_input = number;
            scene_manager_next_scene(app->scene_manager, FlipCryptCaesarDecryptInputScene);
            break;
        case FlipCryptRailfenceKeyInputScene:
            app->railfence_key_input = number;
            scene_manager_next_scene(app->scene_manager, FlipCryptRailfenceInputScene);
            break;
        case FlipCryptRailfenceDecryptKeyInputScene:
            app->railfence_key_input = number;
            scene_manager_next_scene(app->scene_manager, FlipCryptRailfenceDecryptInputScene);
            break;
        case FlipCryptScytaleKeywordInputScene:
            app->scytale_keyword_input = number;
            scene_manager_next_scene(app->scene_manager, FlipCryptScytaleInputScene);
            break;
        case FlipCryptScytaleDecryptKeywordInputScene: 
            app->scytale_keyword_input = number;
            scene_manager_next_scene(app->scene_manager, FlipCryptScytaleDecryptInputScene);
            break;
        default: 
            break;
    }
}

void number_input_scene_on_enter(void* context) {
    furi_assert(context);
    App* app = context;
    NumberInput* number_input = app->number_input;
    FlipCryptScene current = scene_manager_get_current_scene(app->scene_manager);
    switch(current) {
        case FlipCryptAffineKeyAInputScene:
            char affine_encrypt_keya_input_str[40];
            snprintf(affine_encrypt_keya_input_str, sizeof(affine_encrypt_keya_input_str), FLIP_CRYPT_UI_TEXT("Odd # between 1-25, not 13", "输入1-25的奇数,非13"));
            number_input_set_header_text(number_input, affine_encrypt_keya_input_str);
            number_input_set_result_callback(number_input, number_input_scene_callback, context, app->affine_keya_input, 1, 25);
            break;
        case FlipCryptAffineKeyBInputScene:
            char affine_encrypt_keyb_input_str[40];
            snprintf(affine_encrypt_keyb_input_str, sizeof(affine_encrypt_keyb_input_str), FLIP_CRYPT_UI_TEXT("Enter key (%d - %d)", "输入密钥(%d-%d)"), 1, 30);
            number_input_set_header_text(number_input, affine_encrypt_keyb_input_str);
            number_input_set_result_callback(number_input, number_input_scene_callback, context, app->affine_keyb_input, 1, 30);
            break;
        case FlipCryptAffineDecryptKeyAInputScene:
            char affine_decrypt_keya_input_str[40];
            snprintf(affine_decrypt_keya_input_str, sizeof(affine_decrypt_keya_input_str), FLIP_CRYPT_UI_TEXT("Odd # between 1-25, not 13", "输入1-25的奇数,非13"));
            number_input_set_header_text(number_input, affine_decrypt_keya_input_str);
            number_input_set_result_callback(number_input, number_input_scene_callback, context, app->affine_keya_input, 1, 25);
            break;
        case FlipCryptAffineDecryptKeyBInputScene:
            char affine_decrypt_keyb_input_str[40];
            snprintf(affine_decrypt_keyb_input_str, sizeof(affine_decrypt_keyb_input_str), FLIP_CRYPT_UI_TEXT("Enter key (%d - %d)", "输入密钥(%d-%d)"), 1, 30);
            number_input_set_header_text(number_input, affine_decrypt_keyb_input_str);
            number_input_set_result_callback(number_input, number_input_scene_callback, context, app->affine_keyb_input, 1, 30);
            break;
        case FlipCryptCaesarKeyInputScene:
            char caesar_key_input_str[40];
            snprintf(caesar_key_input_str, sizeof(caesar_key_input_str), FLIP_CRYPT_UI_TEXT("Enter key (%d - %d)", "输入密钥(%d-%d)"), 1, 26);
            number_input_set_header_text(number_input, caesar_key_input_str);
            number_input_set_result_callback(number_input, number_input_scene_callback, context, app->caesar_key_input, 1, 26);
            break;
        case FlipCryptCaesarDecryptKeyInputScene:
            char caesar_decrypt_key_input_str[40];
            snprintf(caesar_decrypt_key_input_str, sizeof(caesar_decrypt_key_input_str), FLIP_CRYPT_UI_TEXT("Enter key (%d - %d)", "输入密钥(%d-%d)"), 1, 26);
            number_input_set_header_text(number_input, caesar_decrypt_key_input_str);
            number_input_set_result_callback(number_input, number_input_scene_callback, context, app->caesar_key_input, 1, 26);
            break;
        case FlipCryptRailfenceKeyInputScene:
            char railfence_key_input_str[40];
            snprintf(railfence_key_input_str, sizeof(railfence_key_input_str), FLIP_CRYPT_UI_TEXT("Enter key (%d - %d)", "输入密钥(%d-%d)"), 1, 8);
            number_input_set_header_text(number_input, railfence_key_input_str);
            number_input_set_result_callback(number_input, number_input_scene_callback, context, app->railfence_key_input, 1, 8);
            break;
        case FlipCryptRailfenceDecryptKeyInputScene:
            char railfence_decrypt_key_input_str[40];
            snprintf(railfence_decrypt_key_input_str, sizeof(railfence_decrypt_key_input_str), FLIP_CRYPT_UI_TEXT("Enter key (%d - %d)", "输入密钥(%d-%d)"), 1, 26);
            number_input_set_header_text(number_input, railfence_decrypt_key_input_str);
            number_input_set_result_callback(number_input, number_input_scene_callback, context, app->railfence_key_input, 1, 26);
            break;
        case FlipCryptScytaleKeywordInputScene:
            char scytale_keyword_input_str[40];
            snprintf(scytale_keyword_input_str, sizeof(scytale_keyword_input_str), FLIP_CRYPT_UI_TEXT("Enter key (%d - %d)", "输入密钥(%d-%d)"), 1, 9);
            number_input_set_header_text(number_input, scytale_keyword_input_str);
            number_input_set_result_callback(number_input, number_input_scene_callback, context, app->scytale_keyword_input, 1, 9);
            break;
        case FlipCryptScytaleDecryptKeywordInputScene:
            char scytale_decrypt_keyword_input_str[40];
            snprintf(scytale_decrypt_keyword_input_str, sizeof(scytale_decrypt_keyword_input_str), FLIP_CRYPT_UI_TEXT("Enter key (%d - %d)", "输入密钥(%d-%d)"), 1, 9);
            number_input_set_header_text(number_input, scytale_decrypt_keyword_input_str);
            number_input_set_result_callback(number_input, number_input_scene_callback, context, app->scytale_keyword_input, 1, 9);
            break;
        default:
            break;
    }
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptNumberInputView);
}

// Cipher / Hash submenu initialization
void cipher_submenu_scene_on_enter(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
    FlipCryptScene current = scene_manager_get_current_scene(app->scene_manager);
    switch(current) {
        case FlipCryptAESSubmenuScene:
            submenu_set_header(app->submenu, "AES-128");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "加密文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解密文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 2, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptAffineSubmenuScene:
            submenu_set_header(app->submenu, FLIP_CRYPT_UI_TEXT("Affine Cipher", "仿射密码"));
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "加密文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解密文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 2, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptAtbashSubmenuScene:
            submenu_set_header(app->submenu, FLIP_CRYPT_UI_TEXT("Atbash Cipher", "Atbash密码"));
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "加密文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解密文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 2, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptBaconianSubmenuScene:
            submenu_set_header(app->submenu, FLIP_CRYPT_UI_TEXT("Baconian Cipher", "培根密码"));
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "加密文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解密文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 2, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptBeaufortSubmenuScene:
            submenu_set_header(app->submenu, FLIP_CRYPT_UI_TEXT("Beaufort Cipher", "Beaufort密码"));
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "加密文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解密文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 2, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptCaesarSubmenuScene:
            submenu_set_header(app->submenu, FLIP_CRYPT_UI_TEXT("Caesar Cipher", "凯撒密码"));
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "加密文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解密文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 2, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptPlayfairSubmenuScene:
            submenu_set_header(app->submenu, FLIP_CRYPT_UI_TEXT("Playfair Cipher", "Playfair密码"));
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "加密文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解密文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 2, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptPolybiusSubmenuScene:
            submenu_set_header(app->submenu, FLIP_CRYPT_UI_TEXT("Polybius Square", "Polybius方阵"));
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "加密文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解密文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 2, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptPortaSubmenuScene:
            submenu_set_header(app->submenu, FLIP_CRYPT_UI_TEXT("Porta Cipher", "Porta密码"));
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "加密文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解密文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 3, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptRailfenceSubmenuScene:
            submenu_set_header(app->submenu, FLIP_CRYPT_UI_TEXT("Railfence Cipher", "栅栏密码"));
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "加密文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解密文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 3, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptRC4SubmenuScene:
            submenu_set_header(app->submenu, "RC4");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "加密文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解密文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 2, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptROT13SubmenuScene:
            submenu_set_header(app->submenu, "ROT-13");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "加密文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解密文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 2, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptScytaleSubmenuScene:
            submenu_set_header(app->submenu, FLIP_CRYPT_UI_TEXT("Scytale Cipher", "斯巴达密码棒"));
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "加密文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解密文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 2, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptVigenereSubmenuScene:
            submenu_set_header(app->submenu, FLIP_CRYPT_UI_TEXT("Vigenere Cipher", "维吉尼亚密码"));
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "加密文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解密文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 2, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptBlake2SubmenuScene:
            submenu_set_header(app->submenu, "BLAKE-2s");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Hash Text", "哈希文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 1, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptFNV1ASubmenuScene:
            submenu_set_header(app->submenu, "FNV-1A");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Hash Text", "哈希文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 1, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptMD2SubmenuScene:
            submenu_set_header(app->submenu, "MD2");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Hash Text", "哈希文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 1, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptMD5SubmenuScene:
            submenu_set_header(app->submenu, "MD5");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Hash Text", "哈希文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 1, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptMurmur3SubmenuScene:
            submenu_set_header(app->submenu, "MurmurHash3");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Hash Text", "哈希文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 1, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptSipSubmenuScene:
            submenu_set_header(app->submenu, "SipHash");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Hash Text", "哈希文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 1, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptSHA1SubmenuScene:
            submenu_set_header(app->submenu, "SHA-1");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Hash Text", "哈希文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 1, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptSHA224SubmenuScene:
            submenu_set_header(app->submenu, "SHA-224");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Hash Text", "哈希文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 1, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptSHA256SubmenuScene:
            submenu_set_header(app->submenu, "SHA-256");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Hash Text", "哈希文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 1, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptSHA384SubmenuScene:
            submenu_set_header(app->submenu, "SHA-384");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Hash Text", "哈希文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 1, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptSHA512SubmenuScene:
            submenu_set_header(app->submenu, "SHA-512");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Hash Text", "哈希文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 1, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptXXSubmenuScene:
            submenu_set_header(app->submenu, "XXHash64");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Hash Text", "哈希文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 1, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptBase32SubmenuScene:
            submenu_set_header(app->submenu, "Base32");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "编码文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解码文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 2, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptBase58SubmenuScene:
            submenu_set_header(app->submenu, "Base58");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "编码文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解码文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 2, cipher_learn_submenu_callback, app);
            break;
        case FlipCryptBase64SubmenuScene:
            submenu_set_header(app->submenu, "Base64");
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Encode Text", "编码文本"), 0, cipher_encrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Decode Text", "解码文本"), 1, cipher_decrypt_submenu_callback, app);
            submenu_add_item(app->submenu, FLIP_CRYPT_UI_TEXT("Learn", "学习"), 2, cipher_learn_submenu_callback, app);
            break;
        default:
            submenu_set_header(app->submenu, FLIP_CRYPT_UI_TEXT("Error!", "错误!"));
            break;
    }
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptSubmenuView);
}

// Moves view forward from input to output scenes
void flip_crypt_text_input_callback(void* context) {
    App* app = context;
    FlipCryptScene current = scene_manager_get_current_scene(app->scene_manager);
    switch(current) {
        case FlipCryptSaveTextInputScene: 
            scene_manager_next_scene(app->scene_manager, FlipCryptSaveScene);
            break;
        case FlipCryptAESKeyInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAESInputScene);
            break;
        case FlipCryptAESInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAESOutputScene);
            break;
        case FlipCryptAESDecryptKeyInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAESDecryptInputScene);
            break;
        case FlipCryptAESDecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAESDecryptOutputScene);
            break;
        case FlipCryptAffineInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAffineOutputScene);
            break;
        case FlipCryptAffineDecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAffineDecryptOutputScene);
            break;
        case FlipCryptAtbashInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAtbashOutputScene);
            break;
        case FlipCryptAtbashDecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptAtbashDecryptOutputScene);
            break;
        case FlipCryptBaconianInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBaconianOutputScene);
            break;
        case FlipCryptBaconianDecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBaconianDecryptOutputScene);
            break;
        case FlipCryptBeaufortKeyInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBeaufortInputScene);
            break;
        case FlipCryptBeaufortInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBeaufortOutputScene);
            break;
        case FlipCryptBeaufortDecryptKeyInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBeaufortDecryptInputScene);
            break;
        case FlipCryptBeaufortDecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBeaufortDecryptOutputScene);
            break;
        case FlipCryptCaesarKeyInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptCaesarInputScene);
            break;
        case FlipCryptCaesarInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptCaesarOutputScene);
            break;
        case FlipCryptCaesarDecryptKeyInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptCaesarDecryptInputScene);
            break;
        case FlipCryptCaesarDecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptCaesarDecryptOutputScene);
            break;
        case FlipCryptPlayfairKeywordInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPlayfairInputScene);
            break;
        case FlipCryptPlayfairInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPlayfairOutputScene);
            break;
        case FlipCryptPlayfairDecryptKeywordInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPlayfairDecryptInputScene);
            break;
        case FlipCryptPlayfairDecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPlayfairDecryptOutputScene);
            break;
        case FlipCryptPolybiusInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPolybiusOutputScene);
            break;
        case FlipCryptPolybiusDecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPolybiusDecryptOutputScene);
            break;
        case FlipCryptPortaInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPortaOutputScene);
            break;
        case FlipCryptPortaDecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPortaDecryptOutputScene);
            break;
        case FlipCryptPortaKeywordInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPortaInputScene);
            break;
        case FlipCryptPortaDecryptKeywordInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptPortaDecryptInputScene);
            break;
        case FlipCryptRailfenceInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptRailfenceOutputScene);
            break;
        case FlipCryptRailfenceDecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptRailfenceDecryptOutputScene);
            break;
        case FlipCryptRC4InputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptRC4OutputScene);
            break;
        case FlipCryptRC4KeywordInputScene: 
            scene_manager_next_scene(app->scene_manager, FlipCryptRC4InputScene);
            break;
        case FlipCryptRC4DecryptKeywordInputScene: 
            scene_manager_next_scene(app->scene_manager, FlipCryptRC4DecryptInputScene);
            break;
        case FlipCryptRC4DecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptRC4DecryptOutputScene);
            break;
        case FlipCryptROT13InputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptROT13OutputScene);
            break;
        case FlipCryptROT13DecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptROT13DecryptOutputScene);
            break;
        case FlipCryptScytaleInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptScytaleOutputScene);
            break;
        case FlipCryptScytaleDecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptScytaleDecryptOutputScene);
            break;
        case FlipCryptVigenereInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptVigenereOutputScene);
            break;
        case FlipCryptVigenereKeywordInputScene: 
            scene_manager_next_scene(app->scene_manager, FlipCryptVigenereInputScene);
            break;
        case FlipCryptVigenereDecryptKeywordInputScene: 
            scene_manager_next_scene(app->scene_manager, FlipCryptVigenereDecryptInputScene);
            break;
        case FlipCryptVigenereDecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptVigenereDecryptOutputScene);
            break;
        case FlipCryptBlake2InputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBlake2OutputScene);
            break;
        case FlipCryptFNV1AInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptFNV1AOutputScene);
            break;
        case FlipCryptMD2InputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptMD2OutputScene);
            break;
        case FlipCryptMD5InputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptMD5OutputScene);
            break;
        case FlipCryptMurmur3InputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptMurmur3OutputScene);
            break;
        case FlipCryptSipInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSipOutputScene);
            break;
        case FlipCryptSipKeywordInputScene: 
            scene_manager_next_scene(app->scene_manager, FlipCryptSipInputScene);
            break;
        case FlipCryptSHA1InputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSHA1OutputScene);
            break;
        case FlipCryptSHA224InputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSHA224OutputScene);
            break;
        case FlipCryptSHA256InputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSHA256OutputScene);
            break;
        case FlipCryptSHA384InputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSHA384OutputScene);
            break;
        case FlipCryptSHA512InputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptSHA512OutputScene);
            break;
        case FlipCryptXXInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptXXOutputScene);
            break;
        case FlipCryptBase32InputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBase32OutputScene);
            break;
        case FlipCryptBase32DecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBase32DecryptOutputScene);
            break;
        case FlipCryptBase58InputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBase58OutputScene);
            break;
        case FlipCryptBase58DecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBase58DecryptOutputScene);
            break;
        case FlipCryptBase64InputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBase64OutputScene);
            break;
        case FlipCryptBase64DecryptInputScene:
            scene_manager_next_scene(app->scene_manager, FlipCryptBase64DecryptOutputScene);
            break;
        default:
            break;
    }
}

// Text input views
void cipher_input_scene_on_enter(void* context) {
    App* app = context;
    text_input_reset(app->text_input);
    text_input_set_header_text(app->text_input, FLIP_CRYPT_UI_TEXT("Enter text", "输入文本"));

    FlipCryptScene current = scene_manager_get_current_scene(app->scene_manager);
    switch(current) {
        case FlipCryptSaveTextInputScene: 
            text_input_reset(app->text_input);
            text_input_set_header_text(app->text_input, FLIP_CRYPT_UI_TEXT("Enter file name", "输入文件名"));
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->save_name_input, app->save_name_input_size, true);
            break;
        case FlipCryptAESInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptAESKeyInputScene:
            text_input_reset(app->text_input);
            text_input_set_header_text(app->text_input, FLIP_CRYPT_UI_TEXT("Enter key (sixteen chars)", "输入密钥(16字符)"));
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->aes_key_input, app->aes_key_input_size, true);
            break;
        case FlipCryptAESDecryptKeyInputScene:
            text_input_reset(app->text_input);
            text_input_set_header_text(app->text_input, FLIP_CRYPT_UI_TEXT("Enter key (sixteen chars)", "输入密钥(16字符)"));
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->aes_key_input, app->aes_key_input_size, true);
            break;
        case FlipCryptAESDecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptAffineInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptAffineDecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptAtbashInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptAtbashDecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptBaconianInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptBaconianDecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptBeaufortInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptBeaufortKeyInputScene:
            text_input_reset(app->text_input);
            text_input_set_header_text(app->text_input, FLIP_CRYPT_UI_TEXT("Enter key", "输入密钥"));
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->beaufort_key_input, app->beaufort_key_input_size, true);
            break;
        case FlipCryptBeaufortDecryptKeyInputScene:
            text_input_reset(app->text_input);
            text_input_set_header_text(app->text_input, FLIP_CRYPT_UI_TEXT("Enter key", "输入密钥"));
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->beaufort_key_input, app->beaufort_key_input_size, true);
            break;
        case FlipCryptBeaufortDecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptCaesarInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptCaesarDecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptPlayfairInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptPlayfairKeywordInputScene:
            text_input_reset(app->text_input);
            text_input_set_header_text(app->text_input, FLIP_CRYPT_UI_TEXT("Enter key", "输入密钥"));
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->playfair_keyword_input, app->playfair_keyword_input_size, true);
            break;
        case FlipCryptPlayfairDecryptKeywordInputScene:
            text_input_reset(app->text_input);
            text_input_set_header_text(app->text_input, FLIP_CRYPT_UI_TEXT("Enter key", "输入密钥"));
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->playfair_keyword_input, app->playfair_keyword_input_size, true);
            break;
        case FlipCryptPlayfairDecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptPolybiusInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptPolybiusDecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptPortaInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptPortaDecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptPortaKeywordInputScene:
            text_input_reset(app->text_input);
            text_input_set_header_text(app->text_input, FLIP_CRYPT_UI_TEXT("Enter key", "输入密钥"));
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->porta_keyword_input, app->porta_keyword_input_size, true);
            break;
        case FlipCryptPortaDecryptKeywordInputScene:
            text_input_reset(app->text_input);
            text_input_set_header_text(app->text_input, FLIP_CRYPT_UI_TEXT("Enter key", "输入密钥"));
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->porta_keyword_input, app->porta_keyword_input_size, true);
            break;
        case FlipCryptRailfenceInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptRailfenceDecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptRC4KeywordInputScene:
            text_input_reset(app->text_input);
            text_input_set_header_text(app->text_input, FLIP_CRYPT_UI_TEXT("Enter key", "输入密钥"));
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->rc4_keyword_input, app->rc4_keyword_input_size, true);
            break;
        case FlipCryptRC4DecryptKeywordInputScene:
            text_input_reset(app->text_input);
            text_input_set_header_text(app->text_input, FLIP_CRYPT_UI_TEXT("Enter key", "输入密钥"));
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->rc4_keyword_input, app->rc4_keyword_input_size, true);
            break;
        case FlipCryptRC4InputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptRC4DecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptROT13InputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptROT13DecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptScytaleInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptScytaleDecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptVigenereKeywordInputScene:
            text_input_reset(app->text_input);
            text_input_set_header_text(app->text_input, FLIP_CRYPT_UI_TEXT("Enter key", "输入密钥"));
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->vigenere_keyword_input, app->vigenere_keyword_input_size, true);
            break;
        case FlipCryptVigenereDecryptKeywordInputScene:
            text_input_reset(app->text_input);
            text_input_set_header_text(app->text_input, FLIP_CRYPT_UI_TEXT("Enter key", "输入密钥"));
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->vigenere_keyword_input, app->vigenere_keyword_input_size, true);
            break;
        case FlipCryptVigenereInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptVigenereDecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptBlake2InputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptFNV1AInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptMD2InputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptMD5InputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptMurmur3InputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptSipKeywordInputScene:
            text_input_reset(app->text_input);
            text_input_set_header_text(app->text_input, FLIP_CRYPT_UI_TEXT("Enter 16 char keyword", "输入16字符密钥"));
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->sip_keyword_input, app->sip_keyword_input_size, true);
            break;
        case FlipCryptSipInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptSHA1InputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptSHA224InputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptSHA256InputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptSHA384InputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptSHA512InputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptXXInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptBase32InputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptBase32DecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptBase58InputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptBase58DecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptBase64InputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        case FlipCryptBase64DecryptInputScene:
            text_input_set_result_callback(app->text_input, flip_crypt_text_input_callback, app, app->universal_input, app->universal_input_size, true);
            break;
        default:
            break;
    }
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptTextInputView);
}

// Output dialog
static void dialog_ex_callback(DialogExResult result, void* context) {
    App* app = context;

    switch(result) {
        case DialogExResultLeft:
        case DialogExPressLeft:
            scene_manager_next_scene(app->scene_manager, FlipCryptNFCScene);
            break;

        case DialogExResultRight:
        case DialogExPressRight:
            scene_manager_next_scene(app->scene_manager, FlipCryptQRScene);
            break;

        case DialogExResultCenter:
        case DialogExPressCenter:
            scene_manager_next_scene(app->scene_manager, FlipCryptSaveTextInputScene);
            break;

        case DialogExReleaseLeft:
        case DialogExReleaseRight:
        case DialogExReleaseCenter:
            break;
    }
}

void dialog_cipher_output_scene_on_enter(void* context) {
    App* app = context;
    FlipCryptScene current = scene_manager_get_current_scene(app->scene_manager);
    static const char sha_hex_chars[] = "0123456789abcdef";
    switch(current) {
        case FlipCryptAESOutputScene:
            if(strlen(app->aes_key_input) != 16) {
                dialog_ex_set_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Key must be 16 bytes", "密钥须为16字节"), 64, 18, AlignCenter, AlignCenter);
                break;
            }
            size_t aes_encrypt_len = strlen(app->universal_input);
            if(aes_encrypt_len > 128) aes_encrypt_len = 128;
            uint8_t key[16];
            memcpy(key, app->aes_key_input, 16);
            uint8_t iv[16] = {0};
            uint8_t aes_encrypt_encrypted[128];
            memcpy(aes_encrypt_encrypted, app->universal_input, aes_encrypt_len);
            struct AES_ctx aes_ctx;
            AES_init_ctx_iv(&aes_ctx, key, iv);
            AES_CTR_xcrypt_buffer(&aes_ctx, aes_encrypt_encrypted, aes_encrypt_len);
            char aes_output_text[2 * 128 + 1];
            aes_bytes_to_hex(aes_encrypt_encrypted, aes_encrypt_len, aes_output_text);
            dialog_ex_set_text(app->dialog_ex, aes_output_text, 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("aes.txt"), aes_output_text);
            app->last_output_scene = "AES";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;

        case FlipCryptAESDecryptOutputScene:
            if(strlen(app->aes_key_input) != 16) {
                dialog_ex_set_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Key must be 16 bytes", "密钥须为16字节"), 64, 18, AlignCenter, AlignCenter);
                break;
            }
            size_t aes_decrypt_input_len = strlen(app->universal_input);
            if(aes_decrypt_input_len > 256) aes_decrypt_input_len = 256;
            if(aes_decrypt_input_len % 2 != 0) {
                dialog_ex_set_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Invalid hex length", "十六进制长度无效"), 64, 18, AlignCenter, AlignCenter);
                break;
            }
            size_t aes_decrypt_len = aes_decrypt_input_len / 2;
            uint8_t aes_decrypt_key[16];
            memcpy(aes_decrypt_key, app->aes_key_input, 16);
            uint8_t aes_iv[16] = {0};
            uint8_t aes_decrypt_encrypted[128];
            aes_hex_to_bytes(app->universal_input, aes_decrypt_encrypted, aes_decrypt_input_len);
            struct AES_ctx aes_decrypt_ctx;
            AES_init_ctx_iv(&aes_decrypt_ctx, aes_decrypt_key, aes_iv);
            AES_CTR_xcrypt_buffer(&aes_decrypt_ctx, aes_decrypt_encrypted, aes_decrypt_len);
            char aes_decrypt_output_text[129];
            memcpy(aes_decrypt_output_text, aes_decrypt_encrypted, aes_decrypt_len);
            aes_decrypt_output_text[aes_decrypt_len] = '\0';
            dialog_ex_set_text(app->dialog_ex, aes_decrypt_output_text, 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("aes_decrypt.txt"), aes_decrypt_output_text);
            app->last_output_scene = "AESDecrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptAffineOutputScene:
            dialog_ex_set_text(app->dialog_ex, encode_affine(app->universal_input, app->affine_keya_input, app->affine_keyb_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("affine.txt"), encode_affine(app->universal_input, app->affine_keya_input, app->affine_keyb_input));
            app->last_output_scene = "Affine";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptAffineDecryptOutputScene:
            dialog_ex_set_text(app->dialog_ex, decode_affine(app->universal_input, app->affine_keya_input, app->affine_keyb_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("affine_decrypt.txt"), decode_affine(app->universal_input, app->affine_keya_input, app->affine_keyb_input));
            app->last_output_scene = "AffineDecrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptAtbashOutputScene:
            dialog_ex_set_text(app->dialog_ex, atbash_encrypt_or_decrypt(app->universal_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("atbash.txt"), atbash_encrypt_or_decrypt(app->universal_input));
            app->last_output_scene = "Atbash";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptAtbashDecryptOutputScene:
            dialog_ex_set_text(app->dialog_ex, atbash_encrypt_or_decrypt(app->universal_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("atbash_decrypt.txt"), atbash_encrypt_or_decrypt(app->universal_input));
            app->last_output_scene = "AtbashDecrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptBaconianOutputScene:
            dialog_ex_set_text(app->dialog_ex, baconian_encrypt(app->universal_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("baconian.txt"), baconian_encrypt(app->universal_input));
            app->last_output_scene = "Baconian";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptBaconianDecryptOutputScene:
            dialog_ex_set_text(app->dialog_ex, baconian_decrypt(app->universal_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("baconian_decrypt.txt"), baconian_decrypt(app->universal_input));
            app->last_output_scene = "BaconianDecrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptBeaufortOutputScene:
            dialog_ex_set_text(app->dialog_ex, beaufort_cipher_encrypt_and_decrypt(app->universal_input, app->beaufort_key_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("beaufort.txt"), beaufort_cipher_encrypt_and_decrypt(app->universal_input, app->beaufort_key_input));
            app->last_output_scene = "Beaufort";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptBeaufortDecryptOutputScene:
            dialog_ex_set_text(app->dialog_ex, beaufort_cipher_encrypt_and_decrypt(app->universal_input, app->beaufort_key_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("beaufort_decrypt.txt"), beaufort_cipher_encrypt_and_decrypt(app->universal_input, app->beaufort_key_input));
            app->last_output_scene = "BeaufortDecrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptCaesarOutputScene:
            dialog_ex_set_text(app->dialog_ex, encode_caesar(app->universal_input, app->caesar_key_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("caesar.txt"), encode_caesar(app->universal_input, app->caesar_key_input));
            app->last_output_scene = "Caesar";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptCaesarDecryptOutputScene:
            dialog_ex_set_text(app->dialog_ex, decode_caesar(app->universal_input, app->caesar_key_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("caesar_decrypt.txt"), decode_caesar(app->universal_input, app->caesar_key_input));
            app->last_output_scene = "CaesarDecrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptPlayfairOutputScene:
            dialog_ex_set_text(app->dialog_ex, playfair_encrypt(app->universal_input, playfair_make_table(app->playfair_keyword_input)), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("playfair.txt"), playfair_encrypt(app->universal_input, playfair_make_table(app->playfair_keyword_input)));
            app->last_output_scene = "Playfair";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptPlayfairDecryptOutputScene:
            dialog_ex_set_text(app->dialog_ex, playfair_decrypt(app->universal_input, playfair_make_table(app->playfair_keyword_input)), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("playfair_decrypt.txt"), playfair_decrypt(app->universal_input, playfair_make_table(app->playfair_keyword_input)));
            app->last_output_scene = "PlayfairDecrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptPolybiusOutputScene:
            dialog_ex_set_text(app->dialog_ex, encrypt_polybius(app->universal_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("polybius.txt"), encrypt_polybius(app->universal_input));
            app->last_output_scene = "Polybius";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptPolybiusDecryptOutputScene:
            dialog_ex_set_text(app->dialog_ex, decrypt_polybius(app->universal_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("polybius_decrypt.txt"), decrypt_polybius(app->universal_input));
            app->last_output_scene = "PolybiusDecrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptPortaOutputScene:
            dialog_ex_set_text(app->dialog_ex, porta_encrypt_and_decrypt(app->universal_input, app->porta_keyword_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("porta.txt"), porta_encrypt_and_decrypt(app->universal_input, app->porta_keyword_input));
            app->last_output_scene = "Porta";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptPortaDecryptOutputScene:
            dialog_ex_set_text(app->dialog_ex, porta_encrypt_and_decrypt(app->universal_input, app->porta_keyword_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("porta_decrypt.txt"), porta_encrypt_and_decrypt(app->universal_input, app->porta_keyword_input));
            app->last_output_scene = "PortaDecrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptRailfenceOutputScene:
            dialog_ex_set_text(app->dialog_ex, rail_fence_encrypt(app->universal_input, app->railfence_key_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("railfence.txt"), rail_fence_encrypt(app->universal_input, app->railfence_key_input));
            app->last_output_scene = "Railfence";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptRailfenceDecryptOutputScene:
            dialog_ex_set_text(app->dialog_ex, rail_fence_decrypt(app->universal_input, app->railfence_key_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("railfence_decrypt.txt"), rail_fence_decrypt(app->universal_input, app->railfence_key_input));
            app->last_output_scene = "RailfenceDecrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptRC4OutputScene:
            size_t rc4_input_len = strlen(app->universal_input);
            unsigned char* rc4_encrypted = rc4_encrypt_and_decrypt(app->rc4_keyword_input, (const unsigned char*)app->universal_input, rc4_input_len);
            if (!rc4_encrypted) {
                dialog_ex_set_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Encryption failed", "加密失败"), 64, 18, AlignCenter, AlignCenter);
                break;
            }
            char* rc4_encrypt_hex_output = rc4_to_hex((const char*)rc4_encrypted, rc4_input_len);
            dialog_ex_set_text(app->dialog_ex, rc4_encrypt_hex_output, 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("rc4.txt"), rc4_encrypt_hex_output);
            app->last_output_scene = "RC4";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            free(rc4_encrypt_hex_output);
            free(rc4_encrypted);
            break;
        case FlipCryptRC4DecryptOutputScene:
            size_t rc4_encrypted_len;
            unsigned char* rc4_encrypted_bytes = rc4_hex_to_bytes(app->universal_input, &rc4_encrypted_len);
            if (!rc4_encrypted_bytes) {
                dialog_ex_set_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Invalid hex input", "十六进制输入无效"), 64, 18, AlignCenter, AlignCenter);
                break;
            }
            unsigned char* rc4_decrypted = rc4_encrypt_and_decrypt(app->rc4_keyword_input, rc4_encrypted_bytes, rc4_encrypted_len);
            if (!rc4_decrypted) {
                dialog_ex_set_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Decryption failed", "解密失败"), 64, 18, AlignCenter, AlignCenter);
                free(rc4_encrypted_bytes);
                break;
            }
            char* rc4_decrypted_str = malloc(rc4_encrypted_len + 1);
            if (rc4_decrypted_str) {
                memcpy(rc4_decrypted_str, rc4_decrypted, rc4_encrypted_len);
                rc4_decrypted_str[rc4_encrypted_len] = '\0';
                dialog_ex_set_text(app->dialog_ex, rc4_decrypted_str, 64, 18, AlignCenter, AlignCenter);
                free(rc4_decrypted_str);
            }
            save_result_generic(APP_DATA_PATH("rc4_decrypt.txt"), rc4_decrypted_str);
            app->last_output_scene = "RC4Decrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            free(rc4_encrypted_bytes);
            free(rc4_decrypted);
            break;
        case FlipCryptROT13OutputScene:
            dialog_ex_set_text(app->dialog_ex, encrypt_rot13(app->universal_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("rot13.txt"), encrypt_rot13(app->universal_input));
            app->last_output_scene = "ROT13";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptROT13DecryptOutputScene:
            dialog_ex_set_text(app->dialog_ex, decrypt_rot13(app->universal_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("rot13_decrypt.txt"), decrypt_rot13(app->universal_input));
            app->last_output_scene = "ROT13Decrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptScytaleOutputScene:
            dialog_ex_set_text(app->dialog_ex, scytale_encrypt(app->universal_input, app->scytale_keyword_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("scytale.txt"), scytale_encrypt(app->universal_input, app->scytale_keyword_input));
            app->last_output_scene = "Scytale";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptScytaleDecryptOutputScene:
            dialog_ex_set_text(app->dialog_ex, scytale_decrypt(app->universal_input, app->scytale_keyword_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("scytale_decrypt.txt"), scytale_decrypt(app->universal_input, app->scytale_keyword_input));
            app->last_output_scene = "ScytaleDecrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptVigenereOutputScene:
            dialog_ex_set_text(app->dialog_ex, vigenere_encrypt(app->universal_input, app->vigenere_keyword_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("vigenere.txt"), vigenere_encrypt(app->universal_input, app->vigenere_keyword_input));
            app->last_output_scene = "Vigenere";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptVigenereDecryptOutputScene:
            dialog_ex_set_text(app->dialog_ex, vigenere_decrypt(app->universal_input, app->vigenere_keyword_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("vigenere_decrypt.txt"), vigenere_decrypt(app->universal_input, app->vigenere_keyword_input));
            app->last_output_scene = "VigenereDecrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptBlake2OutputScene:
            Blake2sContext blake2_ctx;
            uint8_t blake2_hash[BLAKE2S_OUTLEN];
            char blake2_hex_output[BLAKE2S_OUTLEN * 2 + 1] = {0};
            blake2s_init(&blake2_ctx);
            blake2s_update(&blake2_ctx, (const uint8_t*)app->universal_input, strlen(app->universal_input));
            blake2s_finalize(&blake2_ctx, blake2_hash);
            blake2s_to_hex(blake2_hash, blake2_hex_output);
            dialog_ex_set_text(app->dialog_ex, blake2_hex_output, 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("blake2.txt"), blake2_hex_output);
            app->last_output_scene = "Blake2";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptFNV1AOutputScene:
            char fnv1a_hash_str[11];
            Fnv32_t fnv1a_hash = fnv_32a_str(app->universal_input, FNV1_32A_INIT);
            snprintf(fnv1a_hash_str, sizeof(fnv1a_hash_str), "0x%08lx", fnv1a_hash);
            dialog_ex_set_text(app->dialog_ex, fnv1a_hash_str, 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("fnv1a.txt"), fnv1a_hash_str);
            app->last_output_scene = "FNV1A";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptMD2OutputScene:
            BYTE hash[MD2_BLOCK_SIZE];
            MD2_CTX ctx;
            md2_init(&ctx);
            md2_update(&ctx, (const BYTE*)app->universal_input, strlen(app->universal_input));
            md2_final(&ctx, hash);
            char md2_hash_str[MD2_BLOCK_SIZE * 2 + 1];
            for (int i = 0; i < MD2_BLOCK_SIZE; ++i) {
                snprintf(&md2_hash_str[i * 2], 3, "%02x", hash[i]);
            }
            dialog_ex_set_text(app->dialog_ex, md2_hash_str, 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("md2.txt"), md2_hash_str);
            app->last_output_scene = "MD2";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptMD5OutputScene:
            uint8_t md5_hash[16];
            char md5_hex_output[33] = {0};
            MD5Context md5_ctx;
            md5_init(&md5_ctx);
            md5_update(&md5_ctx, (const uint8_t*)app->universal_input, strlen(app->universal_input));
            md5_finalize(&md5_ctx, md5_hash);
            md5_to_hex(md5_hash, md5_hex_output);
            dialog_ex_set_text(app->dialog_ex, md5_hex_output, 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("md5.txt"), md5_hex_output);
            app->last_output_scene = "MD5";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptMurmur3OutputScene:
            dialog_ex_set_text(app->dialog_ex, MurmurHash3_x86_32(app->universal_input, strlen(app->universal_input), 0), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("murmur3.txt"), MurmurHash3_x86_32(app->universal_input, strlen(app->universal_input), 0));
            app->last_output_scene = "Murmur3";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptSipOutputScene:
            uint8_t siphash_output[8];
            siphash(app->universal_input, strlen(app->universal_input), app->sip_keyword_input, siphash_output, 8);
            char siphash_str[17];
            for (int i = 0; i < 8; ++i) {
                snprintf(&siphash_str[i * 2], 3, "%02x", siphash_output[i]);
            }
            siphash_str[16] = '\0';
            dialog_ex_set_text(app->dialog_ex, siphash_str, 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("sip.txt"), siphash_str);
            app->last_output_scene = "Sip";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptSHA1OutputScene:
            Sha1Context sha1_ctx;
            uint8_t sha1_hash[20];
            char sha1_hex_output[41] = {0};
            sha1_init(&sha1_ctx);
            sha1_update(&sha1_ctx, (const uint8_t*)app->universal_input, strlen(app->universal_input));
            sha1_finalize(&sha1_ctx, sha1_hash);
            sha1_to_hex(sha1_hash, sha1_hex_output);
            dialog_ex_set_text(app->dialog_ex, sha1_hex_output, 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("sha1.txt"), sha1_hex_output);
            app->last_output_scene = "SHA1";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptSHA224OutputScene: 
            uint8_t sha224_hash[28];
            char sha224_hex_output[57] = {0};
            sha224((const uint8_t *)app->universal_input, (uint64)strlen(app->universal_input), sha224_hash);
            for (int i = 0; i < 28; i++) {
                sha224_hex_output[i * 2] = sha_hex_chars[(sha224_hash[i] >> 4) & 0xF];
                sha224_hex_output[i * 2 + 1] = sha_hex_chars[sha224_hash[i] & 0xF];
            }
            sha224_hex_output[56] = '\0';
            dialog_ex_set_text(app->dialog_ex, sha224_hex_output, 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("sha224.txt"), sha224_hex_output);
            app->last_output_scene = "SHA224";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptSHA256OutputScene: 
            uint8_t sha256_hash[32];
            char sha256_hex_output[65] = {0};
            sha256((const uint8_t *)app->universal_input, (uint64)strlen(app->universal_input), sha256_hash);
            for (int i = 0; i < 32; i++) {
                sha256_hex_output[i * 2] = sha_hex_chars[(sha256_hash[i] >> 4) & 0xF];
                sha256_hex_output[i * 2 + 1] = sha_hex_chars[sha256_hash[i] & 0xF];
            }
            sha256_hex_output[64] = '\0';
            dialog_ex_set_text(app->dialog_ex, sha256_hex_output, 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("sha256.txt"), sha256_hex_output);
            app->last_output_scene = "SHA256";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptSHA384OutputScene: 
            uint8_t sha384_hash[48];
            char sha384_hex_output[97] = {0};
            sha384((const uint8_t *)app->universal_input, (uint64)strlen(app->universal_input), sha384_hash);
            for (int i = 0; i < 48; i++) {
                sha384_hex_output[i * 2] = sha_hex_chars[(sha384_hash[i] >> 4) & 0xF];
                sha384_hex_output[i * 2 + 1] = sha_hex_chars[sha384_hash[i] & 0xF];
            }
            sha384_hex_output[96] = '\0';
            dialog_ex_set_text(app->dialog_ex, sha384_hex_output, 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("sha384.txt"), sha384_hex_output);
            app->last_output_scene = "SHA384";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptSHA512OutputScene:
            uint8_t sha512_hash[64];
            char sha512_hex_output[129] = {0};
            sha512((const uint8_t *)app->universal_input, (uint64)strlen(app->universal_input), sha512_hash);
            for (int i = 0; i < 64; i++) {
                sha512_hex_output[i * 2] = sha_hex_chars[(sha512_hash[i] >> 4) & 0xF];
                sha512_hex_output[i * 2 + 1] = sha_hex_chars[sha512_hash[i] & 0xF];
            }
            sha512_hex_output[128] = '\0';
            dialog_ex_set_text(app->dialog_ex, sha512_hex_output, 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("sha512.txt"), sha512_hex_output);
            app->last_output_scene = "SHA512";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptXXOutputScene:
            uint64_t xxhash = XXH64(app->universal_input, strlen(app->universal_input), 0);
            char xxhash_str[17];
            snprintf(xxhash_str, sizeof(xxhash_str), "%016llX", xxhash);
            dialog_ex_set_text(app->dialog_ex, xxhash_str, 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("xx.txt"), xxhash_str);
            app->last_output_scene = "XX";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptBase32OutputScene:
            dialog_ex_set_text(app->dialog_ex, base32_encode((const uint8_t*)app->universal_input, strlen(app->universal_input)), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("base32.txt"), base32_encode((const uint8_t*)app->universal_input, strlen(app->universal_input)));
            app->last_output_scene = "Base32";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptBase32DecryptOutputScene:
            size_t base32_decoded_len;
            dialog_ex_set_text(app->dialog_ex, (const char*)base32_decode(app->universal_input, &base32_decoded_len), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("base32_decrypt.txt"), (const char*)base32_decode(app->universal_input, &base32_decoded_len));
            app->last_output_scene = "Base32Decrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptBase58OutputScene:
            dialog_ex_set_text(app->dialog_ex, base58_encode(app->universal_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("base58.txt"), base58_encode(app->universal_input));
            app->last_output_scene = "Base58";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptBase58DecryptOutputScene:
            dialog_ex_set_text(app->dialog_ex, base58_decode(app->universal_input), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("base58_decrypt.txt"), base58_decode(app->universal_input));
            app->last_output_scene = "Base58Decrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptBase64OutputScene:
            dialog_ex_set_text(app->dialog_ex, base64_encode((const unsigned char*)app->universal_input, strlen(app->universal_input)), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("base64.txt"), base64_encode((const unsigned char*)app->universal_input, strlen(app->universal_input)));
            app->last_output_scene = "Base64";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        case FlipCryptBase64DecryptOutputScene:
            size_t base64_decoded_len;
            dialog_ex_set_text(app->dialog_ex, (const char*)base64_decode(app->universal_input, &base64_decoded_len), 64, 18, AlignCenter, AlignCenter);
            save_result_generic(APP_DATA_PATH("base64_decrypt.txt"), (const char*)base64_decode(app->universal_input, &base64_decoded_len));
            app->last_output_scene = "Base64Decrypt";
            dialog_ex_set_left_button_text(app->dialog_ex, "NFC");
            dialog_ex_set_center_button_text(app->dialog_ex, FLIP_CRYPT_UI_TEXT("Save", "保存"));
            dialog_ex_set_right_button_text(app->dialog_ex, "QR");
            break;
        default:
            break;
    }
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptDialogExView);
}

// Learn screen views
void cipher_learn_scene_on_enter(void* context) {
    App* app = context;
    widget_reset(app->widget);
    FlipCryptScene current = scene_manager_get_current_scene(app->scene_manager);
    switch(current) {
        case FlipCryptAESLearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "AES-128 (Advanced Encryption Standard with a 128-bit key) is a symmetric block cipher widely used for data encryption. It encrypts data in fixed blocks of 128 bits using a 128-bit key and operates through 10 rounds of transformations, including substitution, permutation, mixing, and key addition. AES-128 is known for its strong security and efficiency, and is a standard for protecting sensitive data in everything from government communications to online banking. Unlike classical ciphers, AES relies on complex mathematical operations and is resistant to all known practical cryptographic attacks when implemented properly.",
                "AES-128(高级加密标准,128位密钥)是一种广泛使用的对称分组密码。它使用128位密钥对128位固定块数据加密,经过10轮变换(替换、置换、混合和密钥加法)。AES-128以安全性和效率著称,是保护敏感数据的标准,应用于政府通信到网上银行等领域。与经典密码不同,AES依赖复杂的数学运算,正确实现可抵御所有已知的实际攻击。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptAffineLearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "The Affine cipher is a type of monoalphabetic substitution cipher that uses a mathematical formula to encrypt each letter: E(x) = (a x x + b) mod 26, where x is the position of the plaintext letter in the alphabet (A = 0, B = 1, etc.), and a and b are keys. The value of a must be coprime with 26 to ensure that each letter maps uniquely. Decryption uses the inverse of a with the formula D(x) = a^-1 x (x - b) mod 26. The Affine cipher combines multiplicative and additive shifts, making it slightly more secure than a Caesar cipher, but still vulnerable to frequency analysis.",
                "仿射密码是一种单表替换密码,使用数学公式加密每个字母:E(x)=(ax+b) mod 26,其中x是明文字母在字母表中的位置(A=0,B=1等),a和b是密钥。a必须与26互素以确保唯一映射。解密使用a的逆元,公式为D(x)=a^-1 x(x-b) mod 26。仿射密码结合乘法和加法移位,比凯撒密码更安全,但仍易受频率分析攻击。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptAtbashLearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "The Atbash cipher is a classical substitution cipher originally used for the Hebrew alphabet. It works by reversing the alphabet so that the first letter becomes the last, the second becomes the second-to-last, and so on. For example, in the Latin alphabet, \'A\' becomes \'Z\', \'B\' becomes \'Y\', and \'C\' becomes \'X\'. It is a simple and symmetric cipher, meaning that the same algorithm is used for both encryption and decryption. Though not secure by modern standards, the Atbash cipher is often studied for its historical significance.",
                "Atbash密码是一种经典替换密码,最初用于希伯来字母表。它通过反转字母表工作,首字母变末字母,第二字母变倒数第二字母,依此类推。例如拉丁字母表中,A变为Z,B变为Y,C变为X。它是简单的对称密码,加密和解密使用相同算法。虽不符合现代安全标准,但因历史意义常被研究。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptBaconianLearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "The Baconian cipher, developed by Francis Bacon in the early 17th century, is a steganographic cipher that encodes each letter of the alphabet into a series of five binary characters, typically using combinations of \'A\' and \'B\'. For example, the letter \'A\' is represented as \'AAAAA\', \'B\' as \'AAAAB\', and so on. This binary code can then be hidden within text, images, or formatting, making it a method of concealed rather than encrypted communication. The Baconian cipher is notable for being an early example of steganography and is often used in historical or educational contexts.",
                "培根密码由Francis Bacon于17世纪初发明,是一种隐写密码,将每个字母编码为五个二进制字符,通常用A和B的组合。例如A表示为AAAAA,B表示为AAAAB,以此类推。二进制码可隐藏在文本、图像或格式中,是隐蔽通信而非加密通信。培根密码是隐写术的早期典范,常用于历史和教育场景。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptBeaufortLearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "The Beaufort cipher is a polyalphabetic substitution cipher that is similar to the Vigenere cipher but uses a slightly different encryption algorithm. Instead of adding the key to the plaintext, it subtracts the plaintext letter from the key letter using a tabula recta, meaning that the same process is used for both encryption and decryption. The cipher was named after Sir Francis Beaufort and was historically used in applications like encrypting naval signals. While more secure than simple ciphers like Caesar, it is still vulnerable to modern cryptanalysis techniques.",
                "Beaufort密码是一种多表替换密码,类似于维吉尼亚密码但算法略有不同。它不是将密钥加到明文上,而是用密钥字母减去明文字母(使用tabula recta),加密和解密过程相同。该密码以Francis Beaufort命名,历史上用于加密海军信号。虽比凯撒密码等简单密码更安全,但仍易受现代密码分析攻击。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptCaesarLearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "The Caesar cipher is a simple and well-known substitution cipher named after Julius Caesar, who reportedly used it to protect military messages. It works by shifting each letter in the plaintext a fixed number of positions down the alphabet. For example, with a shift of 3, \'A\' becomes \'D\', \'B\' becomes \'E\', and so on. After \'Z\', the cipher wraps around to the beginning of the alphabet. While easy to understand and implement, the Caesar cipher is also extremely easy to break.",
                "凯撒密码是一种简单著名的替换密码,以尤利尤斯·凯撒命名,据传他用其保护军事信息。它将明文中每个字母在字母表中向后移动固定位数。例如偏移3位时,A变为D,B变为E,以此类推。Z之后回到字母表开头。凯撒密码易于理解和实现,但也极易被破解。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptPlayfairLearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "The Playfair cipher is a manual symmetric encryption technique invented in 1854 by Charles Wheatstone but popularized by Lord Playfair. It encrypts pairs of letters (digraphs) instead of single letters, making it more secure than simple substitution ciphers. The cipher uses a 5x5 grid of letters constructed from a keyword, combining \'I\' and \'J\' to fit the alphabet into 25 cells. To encrypt, each pair of letters is located in the grid, and various rules are applied based on their positions like same row, same column, or rectangle to substitute them with new letters. The Playfair cipher was used historically for military communication due to its relative ease of use and stronger encryption for its time.",
                "Playfair密码是1854年由Charles Wheatstone发明、Lord Playfair推广的手动对称加密技术。它加密字母对(双字母组合)而非单个字母,比简单替换密码更安全。该密码使用关键词构建的5x5字母网格,I和J合并以适应25格。加密时在网格中定位每对字母,根据同行、同列或矩形等位置规则替换为新字母。Playfair密码因易于使用且加密强度较高,历史上用于军事通信。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptPolybiusLearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "The Polybius square is a classical cipher that uses a 5x5 grid filled with letters of the alphabet to convert plaintext into pairs of numbers. Each letter is identified by its row and column numbers in the grid. For example, 'A' might be encoded as '11', 'B' as '12', and so on. Since the Latin alphabet has 26 letters, 'I' and 'J' are typically combined to fit into the 25-cell grid. The Polybius square is easy to implement and was historically used for signaling and cryptography in wartime. It is simple and easy to decode, and therefore offers minimal security by modern standards.",
                "Polybius方阵是一种经典密码,使用填满字母的5x5网格将明文转换为数字对。每个字母由其在网格中的行列号标识。例如A编码为11,B编码为12,以此类推。拉丁字母表有26个字母,I和J通常合并以适应25格。Polybius方阵易于实现,历史上用于战时通信和密码学。它简单易解码,按现代标准安全性极低。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptPortaLearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "The Porta cipher is a classical polyalphabetic substitution cipher invented by Giovanni Battista della Porta in the 16th century. Unlike simple monoalphabetic ciphers, it uses a repeating key to select from a set of 13 reciprocal substitution alphabets, where each pair of key letters (A/B, C/D, etc.) corresponds to one alphabet. Because the substitution is reciprocal, the same process is used for both encryption and decryption, making it relatively easy to implement. While more secure than a Caesar cipher, it is still vulnerable to frequency analysis and modern cryptanalysis techniques.",
                "Porta密码是16世纪Giovanni Battista della Porta发明的经典多表替换密码。与简单单表密码不同,它使用重复密钥从13个互逆替换字母表中选择,每对密钥字母(A/B、C/D等)对应一个字母表。由于替换是互逆的,加密和解密使用相同过程,实现相对简单。虽比凯撒密码更安全,但仍易受频率分析和现代密码分析攻击。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptRailfenceLearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "The Rail Fence cipher is a form of transposition cipher that rearranges the letters of the plaintext in a zigzag pattern across multiple \'rails\' (or rows), and then reads them off row by row to create the ciphertext. For example, using 3 rails, the message \'HELLO WORLD\' would be written in a zigzag across three lines and then read horizontally to produce the encrypted message. It\'s a simple method that relies on obscuring the letter order rather than substituting characters, and it\'s relatively easy to decrypt with enough trial and error.",
                "栅栏密码是一种换位密码,将明文字母按之字形排列在多条轨道(行)上,然后逐行读取生成密文。例如使用3条轨道,HELLO WORLD将按之字形写在三行上,然后水平读取产生加密信息。这是一种通过打乱字母顺序而非替换字符的简单方法,通过足够的尝试和错误可以相对容易地解密。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptRC4LearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "RC4 is a stream cipher designed by Ron Rivest in 1987 and widely used for its speed and simplicity. It generates a pseudorandom keystream that is XORed with the plaintext to produce ciphertext. RC4\'s internal state consists of a 256-byte array and a pair of index pointers, updated in a key-dependent manner. While once popular in protocols like SSL and WEP, RC4 has been found to have significant vulnerabilities, especially related to key scheduling, and is now considered insecure for modern uses.",
                "RC4是Ron Rivest于1987年设计的流密码,因速度快和实现简单而被广泛使用。它生成伪随机密钥流,与明文异或产生密文。RC4的内部状态由256字节数组和一对索引指针组成,按密钥相关方式更新。虽然曾在SSL和WEP等协议中流行,但RC4已被发现存在重大漏洞(尤其与密钥调度相关),现被认为不安全。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptROT13LearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "ROT13 (short for 'rotate by 13 places') is a simple letter substitution cipher used primarily to obscure text rather than securely encrypt it. It works by shifting each letter of the alphabet 13 positions forward, wrapping around from Z back to A if necessary. Because the alphabet has 26 letters, applying ROT13 twice returns the original text, making it a symmetric cipher. ROT13 is commonly used in online forums to hide spoilers, puzzles, or offensive content, but it offers no real security and can be easily reversed without a key.",
                "ROT13(rotate by 13 places的缩写)是一种简单的字母替换密码,主要用于混淆文本而非安全加密。它将字母表中每个字母向前移动13位,Z之后回到A。由于字母表有26个字母,应用两次ROT13会返回原文,是对称密码。ROT13常用于在线论坛隐藏剧透、谜题或不当内容,但不提供真正安全性,无需密钥即可轻松还原。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptScytaleLearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "The Scytale cipher is an ancient transposition cipher used by the Spartans. It involves wrapping a strip of parchment around a rod (scytale) of a fixed diameter and writing the message along the rod's surface. When unwrapped, the text appears scrambled unless it is rewrapped around a rod of the same size. The security relies on the secrecy of the rod's diameter. Although simple and easy to use, the Scytale cipher offers almost no security by modern standards and just of historical interest.",
                "斯巴达密码棒是斯巴达人使用的古老换位密码。将羊皮纸条缠绕在固定直径的木棒上,沿棒面书写信息。展开后文本看似杂乱,除非用相同尺寸的木棒重新缠绕。安全性依赖于木棒直径的保密性。虽简单易用,但按现代标准几乎无安全性,仅具历史研究价值。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptVigenereLearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "The Vigenere cipher is a classical polyalphabetic substitution cipher that uses a keyword to determine the shift for each letter of the plaintext. Each letter of the keyword corresponds to a Caesar cipher shift, which is applied cyclically over the plaintext. For example, with the keyword \'KEY\', the first letter of the plaintext is shifted by the position of \'K\', the second by \'E\', and so on. This method makes frequency analysis more difficult than in simple substitution ciphers, and it was considered unbreakable for centuries until modern cryptanalysis techniques were developed.",
                "维吉尼亚密码是一种经典多表替换密码,使用关键词确定明文每个字母的偏移量。关键词的每个字母对应一个凯撒密码偏移,循环应用于明文。例如关键词KEY,明文第一个字母按K的位置偏移,第二个按E偏移,以此类推。这种方法使频率分析比简单替换密码更困难,数百年来被认为是不可破解的,直到现代密码分析技术发展。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptBlake2LearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "BLAKE-2s is a high performance cryptographic hash function designed as an improved alternative to MD5, SHA-1, and even SHA-2, offering strong security with faster hashing speeds. Developed by Jean-Philippe Aumasson and others, it builds on the cryptographic foundations of the BLAKE algorithm (a finalist in the SHA-3 competition) but is optimized for practical use. BLAKE2 comes in two main variants: BLAKE2b (optimized for 64-bit platforms) and BLAKE2s (for 8- to 32-bit platforms). It provides features like keyed hashing, salting, and personalization, making it suitable for applications like password hashing, digital signatures, and message authentication. BLAKE2 is widely adopted due to its balance of speed, simplicity, and security, and is used in software like Argon2 (a password hashing algorithm) and various blockchain projects.",
                "BLAKE-2s是高性能密码哈希函数,作为MD5、SHA-1甚至SHA-2的改进替代方案,提供强安全性且哈希速度更快。由Jean-Philippe Aumasson等人开发,基于BLAKE算法(SHA-3竞赛决赛选手)的密码学基础,针对实际使用优化。BLAKE2有两个主要变体:BLAKE2b(针对64位平台优化)和BLAKE2s(针对8-32位平台)。它提供密钥哈希、加盐和个性化等功能,适用于密码哈希、数字签名和消息认证。BLAKE2因速度、简洁和安全性的平衡被广泛采用,用于Argon2等密码哈希算法和各种区块链项目。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptFNV1ALearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "FNV-1a (Fowler-Noll-Vo hash, variant 1a) is a simple, fast, and widely used non-cryptographic hash function designed for use in hash tables and data indexing. It works by starting with a fixed offset basis, then for each byte of input, it XORs the byte with the hash and multiplies the result by a prime number (commonly 16777619 for 32-bit or 1099511628211 for 64-bit). FNV-1a is known for its good distribution and performance on small inputs, but it's not cryptographically secure and should not be used for security-sensitive applications. Its simplicity and efficiency make it a favorite in performance-critical systems and embedded environments.",
                "FNV-1a(Fowler-Noll-Vo哈希,变体1a)是简单、快速、广泛使用的非密码学哈希函数,设计用于哈希表和数据索引。它以固定偏移基数开始,对每个输入字节与哈希值异或,然后乘以素数(32位常用16777619,64位常用1099511628211)。FNV-1a以良好的分布性和小输入性能著称,但不具备密码学安全性,不应用于安全敏感场景。其简洁高效使其成为性能关键系统和嵌入式环境的首选。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptMD2LearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "MD2 (Message Digest 2) is a cryptographic hash function designed by Ronald Rivest in 1989. It produces a 128-bit (16-byte) hash value from an input of any length, typically used to verify data integrity. Although it was once widely used, MD2 is now considered obsolete due to its slow performance and vulnerabilities to collision attacks. As a result, more secure and efficient hash functions like SHA-2 or SHA-3 are recommended for modern applications. Despite its weaknesses, MD2 remains an important part of cryptographic history and legacy systems.",
                "MD2(消息摘要2)是Ronald Rivest于1989年设计的密码哈希函数。它从任意长度输入产生128位(16字节)哈希值,通常用于验证数据完整性。虽曾广泛使用,但MD2因性能慢和碰撞攻击漏洞现已被淘汰。建议现代应用使用更安全高效的SHA-2或SHA-3。尽管存在弱点,MD2仍是密码学历史和遗留系统的重要组成部分。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptMD5LearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "MD5 (Message Digest Algorithm 5) is a widely used cryptographic hash function that produces a 128-bit (16 byte) hash value, typically rendered as a 32-character hexadecimal number. Originally designed for digital signatures and file integrity verification, MD5 is now considered cryptographically broken due to known collision vulnerabilities. While still used in some non-security-critical contexts, it is not recommended for new cryptographic applications.",
                "MD5(消息摘要算法5)是广泛使用的密码哈希函数,产生128位(16字节)哈希值,通常表示为32字符十六进制数。最初设计用于数字签名和文件完整性验证,但因已知碰撞漏洞现已被认为密码学上不安全。虽仍在非安全关键场景使用,但不建议用于新的密码学应用。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptMurmur3LearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "MurmurHash3 is a non-cryptographic hash function designed for fast hashing performance, primarily used in hash-based data structures like hash tables and bloom filters. It was developed by Austin Appleby and is the third and final version of the MurmurHash family. MurmurHash3 offers excellent distribution and low collision rates for general-purpose use, with versions optimized for both 32-bit and 128-bit outputs. Its speed and simplicity make it a popular choice in software like databases, compilers, and networking tools. However, it is not suitable for cryptographic purposes because it lacks the security properties needed to resist things like collision or preimage attacks.",
                "MurmurHash3是专为快速哈希性能设计的非密码学哈希函数,主要用于哈希表和布隆过滤器等哈希数据结构。由Austin Appleby开发,是MurmurHash系列的第三个也是最终版本。MurmurHash3提供优秀的分布性和低碰撞率,有针对32位和128位输出的优化版本。其速度和简洁性使其广泛用于数据库、编译器和网络工具等软件。但不适合密码学用途,因缺乏抵御碰撞或原像攻击所需的安全属性。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptSipLearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "SipHash is a fast, cryptographically secure hash function designed specifically to protect hash tables from DoS attacks caused by hash collisions. Developed by Jean-Philippe Aumasson and Daniel J. Bernstein, SipHash uses a secret key to produce a 64-bit (or 128-bit) hash, making it resistant to hash-flooding attacks where an attacker intentionally causes many collisions. While not as fast as non-cryptographic hashes like MurmurHash, it strikes a balance between speed and security, making it ideal for situations where untrusted input needs to be safely hashed, such as in web servers and language runtimes.",
                "SipHash是快速的密码学安全哈希函数,专为保护哈希表免受哈希碰撞引起的DoS攻击而设计。由Jean-Philippe Aumasson和Daniel J. Bernstein开发,SipHash使用密钥产生64位(或128位)哈希,可抵御攻击者故意制造大量碰撞的哈希泛洪攻击。虽不如MurmurHash等非密码学哈希快,但在速度和安全之间取得平衡,适合需要安全哈希不可信输入的场景,如Web服务器和语言运行时。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptSHA1LearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "SHA-1 (Secure Hash Algorithm 1) is a cryptographic hash function that produces a 160-bit (20-byte) hash value. Once widely used in SSL certificates and digital signatures, SHA-1 has been deprecated due to demonstrated collision attacks, where two different inputs produce the same hash. As a result, it\'s no longer considered secure for cryptographic purposes and has largely been replaced by stronger alternatives.",
                "SHA-1(安全哈希算法1)是密码哈希函数,产生160位(20字节)哈希值。曾广泛用于SSL证书和数字签名,但因已证实的碰撞攻击(两个不同输入产生相同哈希)已被弃用。因此不再被认为具有密码学安全性,已基本被更强的替代方案取代。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptSHA224LearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "SHA-224 is a cryptographic hash function from the SHA-2 family that produces a 224-bit (28-byte) hash value. It is a truncated version of SHA-256, using the same algorithm but outputting a shorter digest. SHA-224 is used when a smaller hash size is preferred while maintaining strong security, commonly in digital signatures and certificate generation.",
                "SHA-224是SHA-2系列的密码哈希函数,产生224位(28字节)哈希值。它是SHA-256的截断版本,使用相同算法但输出更短的摘要。SHA-224适用于需要较小哈希大小同时保持强安全性的场景,常用于数字签名和证书生成。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptSHA256LearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "SHA-256 is part of the SHA-2 family of cryptographic hash functions and generates a 256-bit (32-byte) hash value. It is currently considered secure and is widely used in blockchain, password hashing, digital signatures, and data integrity verification. SHA-256 offers strong resistance against collision and preimage attacks, making it a trusted standard today.",
                "SHA-256是SHA-2系列密码哈希函数的一部分,产生256位(32字节)哈希值。目前被认为安全,广泛用于区块链、密码哈希、数字签名和数据完整性验证。SHA-256对碰撞和原像攻击具有强抵抗力,是当今可信的标准。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptSHA384LearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "SHA-384 is a variant of the SHA-2 hash family that generates a 384-bit (48-byte) hash. It is a truncated version of SHA-512, optimized for 64-bit processors. SHA-384 provides a higher security margin than SHA-256 and SHA-224 due to its longer output and 64-bit internal operations, making it suitable for applications requiring robust collision resistance, such as secure communication protocols and cryptographic signatures.",
                "SHA-384是SHA-2哈希系列的变体,产生384位(48字节)哈希。它是SHA-512的截断版本,针对64位处理器优化。由于更长的输出和64位内部运算,SHA-384提供比SHA-256和SHA-224更高的安全裕度,适用于需要强碰撞抗性的场景,如安全通信协议和密码学签名。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptSHA512LearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "SHA-512 (Secure Hash Algorithm 512) is a member of the SHA-2 family of cryptographic hash functions, developed by the NSA and standardized by NIST. It produces a 512-bit (64-byte) hash from any input, making it very resistant to collision and preimage attacks. SHA-512 operates on 1024-bit blocks and performs 80 rounds of complex mathematical operations involving bitwise logic, modular addition, and message expansion. It's widely used in digital signatures, certificates, and data integrity checks where strong cryptographic security is required. Although slower on 32-bit systems due to its large word size, SHA-512 is very efficient on 64-bit processors and remains a trusted standard in secure applications.",
                "SHA-512(安全哈希算法512)是SHA-2系列密码哈希函数的成员,由NSA开发、NIST标准化。它从任意输入产生512位(64字节)哈希,对碰撞和原像攻击具有极强抵抗力。SHA-512处理1024位块,执行80轮涉及位逻辑、模加法和消息扩展的复杂运算。广泛用于需要强密码安全性的数字签名、证书和数据完整性检查。虽因大字长在32位系统上较慢,但在64位处理器上非常高效,仍是安全应用的可信标准。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptXXLearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "XXHash is a non-cryptographic hash function designed for high-performance hashing. Developed by Yann Collet, XXHash focuses on speed and efficiency, outperforming many traditional hash functions while maintaining a low collision rate. It comes in several versions, including XXH32, XXH64, and the newer XXH3, which offers improved performance and adaptability to modern CPUs. While XXHash is not suitable for cryptographic purposes due to its lack of security guarantees, it is widely used in performance-critical applications like databases, file systems, and compression tools.",
                "XXHash是为高性能哈希设计的非密码学哈希函数。由Yann Collet开发,XXHash专注于速度和效率,超越许多传统哈希函数同时保持低碰撞率。有多个版本,包括XXH32、XXH64和更新的XXH3(提供改进性能和对现代CPU的适应性)。虽因缺乏安全保证不适合密码学用途,但广泛用于数据库、文件系统和压缩工具等性能关键应用。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptBase32LearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "Base32 is an encoding scheme that converts binary data into a set of 32 ASCII characters, using the characters A-Z and 2-7. It is commonly used for representing binary data in a human-readable and URL-safe format, especially when Base64 is not ideal due to case sensitivity or special characters. Each Base32 character represents 5 bits of data, making it slightly less space-efficient than Base64 but easier to handle in contexts like QR codes, file names, or secret keys (e.g., in two-factor authentication). Unlike encryption or hashing, Base32 is not secure-it's simply a reversible way to encode data.",
                "Base32是将二进制数据转换为32个ASCII字符(A-Z和2-7)的编码方案。常用于以人类可读和URL安全的格式表示二进制数据,尤其当Base64因大小写敏感或特殊字符不理想时。每个Base32字符表示5位数据,空间效率略低于Base64,但在QR码、文件名或密钥(如双因素认证)等场景更易处理。与加密或哈希不同,Base32不安全,只是可逆的数据编码方式。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptBase58LearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "Base-58 is a binary-to-text encoding scheme used to represent large numbers or binary data in a more human-friendly format. It's most commonly used in cryptocurrencies like Bitcoin to encode addresses. Unlike Base-64, Base-58 removes easily confused characters such as zero, capital o, capital i, and lowercase L, to reduce human error when copying or typing. This makes Base-58 more suitable for user-facing strings while still being compact and efficient for encoding data.",
                "Base-58是将大数字或二进制数据以更友好格式表示的二进制转文本编码方案。最常用于比特币等加密货币的地址编码。与Base-64不同,Base-58移除了容易混淆的字符(如0、O、I和l),减少复制或输入时的人为错误。这使Base-58更适合面向用户的字符串,同时保持编码数据的紧凑高效。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        case FlipCryptBase64LearnScene:
            widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
                "Base64 is a binary-to-text encoding scheme that represents binary data using 64 ASCII characters: A-Z, a-z, 0-9, +, and /. It works by dividing the input into 6-bit chunks and mapping each chunk to a character from the Base64 alphabet, often adding = as padding at the end to align the output. Base64 is commonly used to encode data for transmission over media that are designed to handle text, such as embedding images in HTML or safely transmitting binary data in email or JSON. Like Base-32 and Base-58, Base-64 is not secure as it is fully reversible.",
                "Base64是使用64个ASCII字符(A-Z、a-z、0-9、+和/)表示二进制数据的二进制转文本编码方案。它将输入分为6位块,将每块映射到Base64字母表的字符,通常在末尾添加=作为填充以对齐输出。Base64常用于在设计处理文本的媒体上传输数据,如在HTML中嵌入图像或在邮件和JSON中安全传输二进制数据。与Base-32和Base-58一样,Base-64不安全,完全可逆。"));
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
            break;
        default:
            break;
    }
}

// NFC functionality on output screen
void create_nfc_tag(App* app, const char* message) {
    app->nfc_device = nfc_device_alloc();
    nfc_data_generator_fill_data(NfcDataGeneratorTypeNTAG215, app->nfc_device);

    const MfUltralightData* data_original = nfc_device_get_data(app->nfc_device, NfcProtocolMfUltralight);
    // Create a mutable copy of the data
    MfUltralightData* data = malloc(sizeof(MfUltralightData));
    furi_assert(data); // Check for successful allocation
    memcpy(data, data_original, sizeof(MfUltralightData));

    // Ensure iso14443_3a_data is also a copy if it's not directly embedded
    Iso14443_3aData* isodata = malloc(sizeof(Iso14443_3aData));
    furi_assert(isodata); // Check for successful allocation
    memcpy(isodata, data_original->iso14443_3a_data, sizeof(Iso14443_3aData));
    data->iso14443_3a_data = isodata;

    const char* lang = "en";
    uint8_t lang_len = strlen(lang);
    size_t msg_len = strlen(message);

    // NDEF Text Record Payload: Status Byte + Language Code + Message
    size_t text_payload_len = 1 + lang_len + msg_len;

    // NDEF Record Header: TNF_Flags (1) + Type_Length (1) + Payload_Length (1) + Type (1)
    size_t ndef_record_header_len = 4; // D1, 01, Payload_Len, T

    // Total NDEF Message Length (excluding TLV 0x03 and its length byte)
    // This is the length that goes into data->page[4].data[1]
    size_t ndef_message_total_len = ndef_record_header_len + text_payload_len;

    // Set up TLV Header (starting at Page 4, Byte 0)
    data->page[4].data[0] = 0x03; // TLV: NDEF Message
    data->page[4].data[1] = ndef_message_total_len; // Length of NDEF Message (excluding TLV itself)

    // NDEF Record Header (starting at Page 4, Byte 2)
    data->page[4].data[2] = 0xD1; // MB, ME, SR, TNF=1 (Well-Known Type)
    data->page[4].data[3] = 0x01; // Type Length = 1 ('T')

    // NDEF Record Payload Start (starting at Page 5, Byte 0)
    data->page[5].data[0] = text_payload_len; // Payload Length
    data->page[5].data[1] = 'T'; // Type = text
    uint8_t status_byte = (0 << 7) | (lang_len & 0x3F); // UTF-8 (bit 7 = 0), Language Length
    data->page[5].data[2] = status_byte; // Status byte

    // Begin writing language and message
    size_t current_byte_idx = 3; // Starting at Page 5, Byte 3 for language code
    size_t current_page_idx = 5;

    // Write language code
    for (size_t i = 0; i < lang_len; ++i) {
        data->page[current_page_idx].data[current_byte_idx++] = lang[i];
        if (current_byte_idx > 3) { // Moved to next byte in page, if overflowed
            current_byte_idx = 0;
            current_page_idx++;
        }
    }

    // Write actual message
    for (size_t i = 0; i < msg_len; ++i) {
        data->page[current_page_idx].data[current_byte_idx++] = message[i];
        if (current_byte_idx > 3) { // Moved to next byte in page, if overflowed
            current_byte_idx = 0;
            current_page_idx++;
        }
    }

    // NDEF TLV Terminator (0xFE) - placed after the entire NDEF message
    // It should be at the next available byte after the message payload.
    data->page[current_page_idx].data[current_byte_idx++] = 0xFE;

    // Finalize and store
    nfc_device_set_data(app->nfc_device, NfcProtocolMfUltralight, data);

    // Free the allocated memory
    free(data);
    free(isodata);
}

void flip_crypt_nfc_scene_on_enter(void* context) {
    App* app = context;
    widget_reset(app->widget);
    widget_add_icon_element(app->widget, 0, 3, &I_NFC_dolphin_emulation_51x64);
    widget_add_string_element(app->widget, 90, 25, AlignCenter, AlignTop, FontPrimary, FLIP_CRYPT_UI_TEXT("Emulating...", "模拟中..."));
    if (strcmp(app->last_output_scene, "AES") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("aes.txt")));    
    } else if (strcmp(app->last_output_scene, "AESDecrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("aes_decrypt.txt")));    
    } else if (strcmp(app->last_output_scene, "Atbash") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("atbash.txt")));
    } else if (strcmp(app->last_output_scene, "AtbashDecrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("atbash_decrypt.txt")));
    } else if (strcmp(app->last_output_scene, "Affine") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("affine.txt")));
    } else if (strcmp(app->last_output_scene, "AffineDecrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("affine_decrypt.txt")));
    } else if (strcmp(app->last_output_scene, "Baconian") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("baconian.txt")));
    } else if (strcmp(app->last_output_scene, "BaconianDecrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("baconian_decrypt.txt")));
    } else if (strcmp(app->last_output_scene, "Beaufort") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("beaufort.txt")));
    } else if (strcmp(app->last_output_scene, "BeaufortDecrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("beaufort_decrypt.txt")));
    } else if (strcmp(app->last_output_scene, "Caesar") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("caesar.txt")));
    } else if (strcmp(app->last_output_scene, "CaesarDecrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("caesar_decrypt.txt")));
    } else if (strcmp(app->last_output_scene, "Playfair") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("playfair.txt")));
    } else if (strcmp(app->last_output_scene, "PlayfairDecrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("playfair_decrypt.txt")));
    } else if (strcmp(app->last_output_scene, "Polybius") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("polybius.txt")));
    } else if (strcmp(app->last_output_scene, "PolybiusDecrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("polybius_decrypt.txt")));
    } else if (strcmp(app->last_output_scene, "Porta") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("porta.txt")));
    } else if (strcmp(app->last_output_scene, "PortaDecrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("porta_decrypt.txt")));
    }  else if (strcmp(app->last_output_scene, "Railfence") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("railfence.txt")));
    } else if (strcmp(app->last_output_scene, "RailfenceDecrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("railfence_decrypt.txt")));
    } else if (strcmp(app->last_output_scene, "RC4") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("rc4.txt")));
    } else if (strcmp(app->last_output_scene, "RC4Decrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("rc4_decrypt.txt")));
    } else if (strcmp(app->last_output_scene, "ROT13") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("rot13.txt")));
    } else if (strcmp(app->last_output_scene, "ROT13Decrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("rot13_decrypt.txt")));
    } else if (strcmp(app->last_output_scene, "Scytale") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("scytale.txt")));
    } else if (strcmp(app->last_output_scene, "ScytaleDecrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("scytale_decrypt.txt")));
    } else if (strcmp(app->last_output_scene, "Vigenere") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("vigenere.txt")));
    } else if (strcmp(app->last_output_scene, "VigenereDecrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("vigenere_decrypt.txt")));
    } else if (strcmp(app->last_output_scene, "Blake2") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("blake2.txt")));
    } else if (strcmp(app->last_output_scene, "FNV1A") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("fnv1a.txt")));
    } else if (strcmp(app->last_output_scene, "MD2") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("md2.txt")));
    } else if (strcmp(app->last_output_scene, "MD5") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("md5.txt")));
    } else if (strcmp(app->last_output_scene, "Murmur3") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("murmur3.txt")));
    } else if (strcmp(app->last_output_scene, "Sip") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("sip.txt")));
    } else if (strcmp(app->last_output_scene, "SHA1") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("sha1.txt")));
    } else if (strcmp(app->last_output_scene, "SHA224") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("sha224.txt")));
    } else if (strcmp(app->last_output_scene, "SHA256") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("sha256.txt")));
    } else if (strcmp(app->last_output_scene, "SHA384") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("sha384.txt")));
    } else if (strcmp(app->last_output_scene, "SHA512") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("sha512.txt")));
    } else if (strcmp(app->last_output_scene, "XX") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("xx.txt")));
    } else if (strcmp(app->last_output_scene, "Base32") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("base32.txt")));
    } else if (strcmp(app->last_output_scene, "Base32Decrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("base32_decrypt.txt")));
    } else if (strcmp(app->last_output_scene, "Base58") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("base58.txt")));
    } else if (strcmp(app->last_output_scene, "Base58Decrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("base58_decrypt.txt")));
    } else if (strcmp(app->last_output_scene, "Base64") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("base64.txt")));
    } else if (strcmp(app->last_output_scene, "Base64Decrypt") == 0) {
        create_nfc_tag(context, load_result_generic(APP_DATA_PATH("base64_decrypt.txt")));
    } else {
        create_nfc_tag(context, "ERROR");
    }
    const MfUltralightData* data = nfc_device_get_data(app->nfc_device, NfcProtocolMfUltralight);
    app->listener = nfc_listener_alloc(app->nfc, NfcProtocolMfUltralight, data);
    nfc_listener_start(app->listener, NULL, NULL);
    notification_message(furi_record_open(RECORD_NOTIFICATION), &sequence_blink_start_magenta);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
}

void flip_crypt_nfc_scene_on_exit(void* context) {
    App* app = context;
    if(app->listener) {
        nfc_listener_stop(app->listener);
        nfc_listener_free(app->listener);
        app->listener = NULL;
    }
    widget_reset(app->widget);
    notification_message(furi_record_open(RECORD_NOTIFICATION), &sequence_blink_stop);
}

// QR Code functionality on output screen
void flip_crypt_qr_scene_on_enter(void* context) {
    App* app = context;
    bool isTooLong = false;
    widget_reset(app->widget);
    if (strcmp(app->last_output_scene, "AES") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("aes.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true ); 
    } else if (strcmp(app->last_output_scene, "AESDecrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("aes_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);  
    } else if (strcmp(app->last_output_scene, "Atbash") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("atbash.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "AtbashDecrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("atbash_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Affine") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("affine.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "AffineDecrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("affine_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Baconian") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("baconian.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "BaconianDecrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("baconian_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Beaufort") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("beaufort.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "BeaufortDecrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("beaufort_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Caesar") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("caesar.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "CaesarDecrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("caesar_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Playfair") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("playfair.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "PlayfairDecrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("playfair_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Polybius") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("polybius.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "PolybiusDecrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("polybius_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Porta") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("porta.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "PortaDecrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("porta_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Railfence") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("railfence.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "RailfenceDecrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("railfence_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "RC4") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("rc4.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "RC4Decrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("rc4_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "ROT13") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("rot13.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "ROT13Decrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("rot13_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Scytale") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("scytale.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "ScytaleDecrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("scytale_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Vigenere") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("vigenere.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "VigenereDecrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("vigenere_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Blake2") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("blake2.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "FNV1A") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("fnv1a.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "MD2") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("md2.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "MD5") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("md5.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Murmur3") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("murmur3.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Sip") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("sip.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "SHA1") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("sha1.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "SHA224") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("sha224.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "SHA256") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("sha256.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "SHA384") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("sha384.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "SHA512") == 0) {
        isTooLong = true;
    } else if (strcmp(app->last_output_scene, "XX") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("xx.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Base32") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("base32.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Base32Decrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("base32_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Base58") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("base58.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Base58Decrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("base58_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Base64") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("base64.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else if (strcmp(app->last_output_scene, "Base64Decrypt") == 0) {
        qrcodegen_encodeText(load_result_generic(APP_DATA_PATH("base64_decrypt.txt")), app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    } else {
        qrcodegen_encodeText("ERROR", app->qr_buffer, app->qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 5, qrcodegen_Mask_AUTO, true);
    }
    if (!isTooLong) {
        int size = qrcodegen_getSize(app->qrcode);
        const int scale = 1;
        const int offset_x = 64 - (size * scale) / 2;
        const int offset_y = 32 - (size * scale) / 2;
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (qrcodegen_getModule(app->qrcode, x, y)) {
                    // widget_add_rect_element(app->widget, offset_x + x * scale, offset_y + y * scale, scale, scale, 0, true);
                    widget_add_rect_element(app->widget, offset_x + x, offset_y + y, scale, scale, 0, true);
                }
            }
        }
    } else {
        widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, "Output too long");
    }
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
}

void flip_crypt_qr_scene_on_exit(void* context) {
    App* app = context;
    widget_reset(app->widget);
}

void flip_crypt_save_scene_on_enter(void* context) {
    App* app = context;
    widget_reset(app->widget);
    if (strcmp(app->last_output_scene, "AES") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("aes.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "AESDecrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("aes_decrypt.txt")), app->save_name_input);  
    } else if (strcmp(app->last_output_scene, "Atbash") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("atbash.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "AtbashDecrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("atbash_decrypt.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Affine") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("affine.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "AffineDecrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("affine_decrypt.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Baconian") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("baconian.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "BaconianDecrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("baconian_decrypt.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Beaufort") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("beaufort.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "BeaufortDecrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("beaufort_decrypt.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Caesar") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("caesar.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "CaesarDecrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("caesar_decrypt.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Playfair") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("playfair.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "PlayfairDecrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("playfair_decrypt.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Polybius") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("polybius.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "PolybiusDecrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("polybius_decrypt.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Porta") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("porta.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "PortaDecrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("porta_decrypt.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Railfence") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("railfence.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "RailfenceDecrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("railfence_decrypt.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "RC4") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("rc4.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "RC4Decrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("rc4_decrypt.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "ROT13") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("rot13.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "ROT13Decrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("rot13_decrypt.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Scytale") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("scytale.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "ScytaleDecrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("scytale_decrypt.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Vigenere") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("vigenere.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "VigenereDecrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("vigenere_decrypt.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Blake2") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("blake2.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "FNV1A") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("fnv1a.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "MD2") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("md2.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "MD5") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("md5.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Murmur3") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("murmur3.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Sip") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("sip.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "SHA1") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("sha1.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "SHA224") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("sha224.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "SHA256") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("sha256.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "SHA384") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("sha384.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "SHA512") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("sha512.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "XX") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("xx.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Base32") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("base32.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Base32Decrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("base32_decrypt.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Base58") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("base58.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Base58Decrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("base58_decrypt.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Base64") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("base64.txt")), app->save_name_input);
    } else if (strcmp(app->last_output_scene, "Base64Decrypt") == 0) {
        save_result(load_result_generic(APP_DATA_PATH("base64_decrypt.txt")), app->save_name_input);
    } else {
        save_result("ERROR", app->save_name_input);
    }
    widget_add_icon_element(app->widget, 36, 6, &I_DolphinSaved_92x58);
    widget_add_string_element(app->widget, 25, 15, AlignCenter, AlignCenter, FontPrimary, FLIP_CRYPT_UI_TEXT("Saved!", "已保存!"));
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
}

// About screen from main menu option
void flip_crypt_about_scene_on_enter(void* context) {
    App* app = context;
    widget_reset(app->widget);
    widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, FLIP_CRYPT_UI_TEXT(
        "FlipCrypt\n"
        "v0.6\n"
        "Explore and learn about various cryptographic and text encoding methods.\n\n"
        "Usage:\n"
        "Select the method you want to use for encoding / decoding text and fill in the necessary inputs.\n"
        "On the output screen, there are up to three options for actions you can do with the output - Save, NFC, and QR. The save button saves the output to a text file in the folder located at /ext/flip_crypt_saved/. The NFC button emulates the output using NTAG215. The QR button generates and displays a QR code of your output. Not all three options will be available on every output screen due to memory limitations - for instance the flipper just can't handle the QR code for a SHA-512 output.\n\n"
        "Feel free to leave any issues / PRs on the repo with new feature ideas!\n\n"
        "Author: @Tyl3rA\n"
        "Source Code: https://github.com/Tyl3rA/FlipCrypt\n\n\n"
        "SHA 224-512 LICENSE INFO:"
        "FIPS 180-2 SHA-224/256/384/512 implementation\n"
        "\n"
        "Copyright (C) 2005-2023 Olivier Gay <olivier.gay@a3.epfl.ch>\n"
        "All rights reserved.\n"
        "\n"
        "Redistribution and use in source and binary forms, with or without\n"
        "modification, are permitted provided that the following conditions\n"
        "are met:\n"
        "1. Redistributions of source code must retain the above copyright\n"
        "notice, this list of conditions and the following disclaimer.\n"
        "2. Redistributions in binary form must reproduce the above copyright\n"
        "notice, this list of conditions and the following disclaimer in the\n"
        "documentation and/or other materials provided with the distribution.\n"
        "3. Neither the name of the project nor the names of its contributors\n"
        "may be used to endorse or promote products derived from this software\n"
        "without specific prior written permission.\n"
        "\n"
        "THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND\n"
        "ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n"
        "IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n"
        "ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE\n"
        "FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n"
        "DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS\n"
        "OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n"
        "HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\n"
        "LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\n"
        "OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n"
        "SUCH DAMAGE.",
        "FlipCrypt\n"
        "v0.6\n"
        "探索和学习各种密码学及文本编码方法。\n\n"
        "使用方法:\n"
        "选择编码/解码方式并填入所需输入。\n"
        "输出界面最多有三个操作选项:保存、NFC和QR。保存按钮将输出保存至 /ext/flip_crypt_saved/ 目录。NFC按钮通过NTAG215模拟输出。QR按钮生成并显示输出的二维码。由于内存限制,并非所有输出界面都支持全部三个选项,例如SHA-512输出无法生成QR码。\n\n"
        "欢迎在仓库提交Issue或PR提出新功能建议!\n\n"
        "作者: @Tyl3rA\n"
        "源代码: https://github.com/Tyl3rA/FlipCrypt\n\n\n"
        "SHA 224-512 许可证信息:"
        "FIPS 180-2 SHA-224/256/384/512 实现\n"
        "\n"
        "Copyright (C) 2005-2023 Olivier Gay <olivier.gay@a3.epfl.ch>\n"
        "All rights reserved.\n"
        "\n"
        "Redistribution and use in source and binary forms, with or without\n"
        "modification, are permitted provided that the following conditions\n"
        "are met:\n"
        "1. Redistributions of source code must retain the above copyright\n"
        "notice, this list of conditions and the following disclaimer.\n"
        "2. Redistributions in binary form must reproduce the above copyright\n"
        "notice, this list of conditions and the following disclaimer in the\n"
        "documentation and/or other materials provided with the distribution.\n"
        "3. Neither the name of the project nor the names of its contributors\n"
        "may be used to endorse or promote products derived from this software\n"
        "without specific prior written permission.\n"
        "\n"
        "THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND\n"
        "ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n"
        "IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n"
        "ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE\n"
        "FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n"
        "DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS\n"
        "OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n"
        "HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\n"
        "LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\n"
        "OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n"
        "SUCH DAMAGE."));
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipCryptWidgetView);
}

// Generic event handlers
bool flip_crypt_generic_event_handler(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void flip_crypt_generic_on_exit(void* context) {
    UNUSED(context);
}

void (*const flip_crypt_scene_on_enter_handlers[])(void*) = {
    flip_crypt_main_menu_scene_on_enter, // Main menu
    flip_crypt_cipher_submenu_scene_on_enter, // Cipher submenu
    flip_crypt_hash_submenu_scene_on_enter, // Hash submenu
    flip_crypt_other_submenu_scene_on_enter, // Other submenu
    flip_crypt_about_scene_on_enter, // About scene
    cipher_submenu_scene_on_enter, // AES128Submenu
    cipher_input_scene_on_enter,   // AES128Input
    cipher_input_scene_on_enter,   // AES128KeyInput
    cipher_input_scene_on_enter,   // AES128DecryptKeyInput
    dialog_cipher_output_scene_on_enter,  // AES128Output
    cipher_input_scene_on_enter,   // AES128DecryptInput
    dialog_cipher_output_scene_on_enter,  // AES128DecryptOutput
    cipher_learn_scene_on_enter,   // AES128Learn
    cipher_submenu_scene_on_enter, // AffineSubmenu
    cipher_input_scene_on_enter,   // AffineInput
    number_input_scene_on_enter,   // AffineKeyAInput
    number_input_scene_on_enter,   // AffineDecryptKeyAInput
    number_input_scene_on_enter,   // AffineKeyBInput
    number_input_scene_on_enter,   // AffineDecryptKeyBInput
    dialog_cipher_output_scene_on_enter,  // AffineOutput
    cipher_input_scene_on_enter,   // AffineDecryptInput
    dialog_cipher_output_scene_on_enter,  // AffineDecryptOutput
    cipher_learn_scene_on_enter,   // AffineLearn
    cipher_submenu_scene_on_enter, // AtbashSubmenu
    cipher_input_scene_on_enter,   // AtbashInput
    dialog_cipher_output_scene_on_enter,  // AtbashOutput
    cipher_input_scene_on_enter,   // AtbashDecryptInput
    dialog_cipher_output_scene_on_enter,  // AtbashDecryptOutput
    cipher_learn_scene_on_enter,   // AtbashLearn
    cipher_submenu_scene_on_enter, // BaconianSubmenu
    cipher_input_scene_on_enter,   // BaconianInput
    dialog_cipher_output_scene_on_enter,  // BaconianOutput
    cipher_input_scene_on_enter,   // BaconianDecryptInput
    dialog_cipher_output_scene_on_enter,  // BaconianDecryptOutput
    cipher_learn_scene_on_enter,   // BaconianLearn
    cipher_submenu_scene_on_enter, // BeaufortSubmenu
    cipher_input_scene_on_enter,   // BeaufortInput
    cipher_input_scene_on_enter,   // BeaufortKeyInput
    cipher_input_scene_on_enter,   // BeaufortDecryptKeyInput
    dialog_cipher_output_scene_on_enter,  // BeaufortOutput
    cipher_input_scene_on_enter,   // BeaufortDecryptInput
    dialog_cipher_output_scene_on_enter,  // BeaufortDecryptOutput
    cipher_learn_scene_on_enter,   // BeaufortLearn
    cipher_submenu_scene_on_enter, // CaesarSubmenu
    cipher_input_scene_on_enter,   // CaesarInput
    number_input_scene_on_enter,   // CaesarKeyInput
    number_input_scene_on_enter,   // CaesarDecryptKeyInput
    dialog_cipher_output_scene_on_enter,  // CaesarOutput
    cipher_input_scene_on_enter,   // CaesarDecryptInput
    dialog_cipher_output_scene_on_enter,  // CaesarDecryptOutput
    cipher_learn_scene_on_enter,   // CaesarLearn
    cipher_submenu_scene_on_enter, // PlayfairSubmenu
    cipher_input_scene_on_enter,   // PlayfairInput
    cipher_input_scene_on_enter,   // PlayfairKeywordInput
    cipher_input_scene_on_enter,   // PlayfairDecryptKeywordInput
    dialog_cipher_output_scene_on_enter,  // PlayfairOutput
    cipher_input_scene_on_enter,   // PlayfairDecryptInput
    dialog_cipher_output_scene_on_enter,  // PlayfairDecryptOutput
    cipher_learn_scene_on_enter,   // PlayfairLearn
    cipher_submenu_scene_on_enter, // PolybiusSubmenu
    cipher_input_scene_on_enter,   // PolybiusInput
    dialog_cipher_output_scene_on_enter,  // PolybiusOutput
    cipher_input_scene_on_enter,   // PolybiusDecryptInput
    dialog_cipher_output_scene_on_enter,  // PolybiusDecryptOutput
    cipher_learn_scene_on_enter,   // PolybiusLearn
    cipher_submenu_scene_on_enter, // PortaSubmenu
    cipher_input_scene_on_enter,   // PortaInput
    cipher_input_scene_on_enter,   // PortaKeywordInput
    cipher_input_scene_on_enter,   // PortaDecryptKeywordInput
    dialog_cipher_output_scene_on_enter,  // PortaOutput
    cipher_input_scene_on_enter,   // PortaDecryptInput
    dialog_cipher_output_scene_on_enter,  // PortaDecryptOutput
    cipher_learn_scene_on_enter,   // PortaLearn
    cipher_submenu_scene_on_enter, // RailfenceSubmenu
    cipher_input_scene_on_enter,   // RailfenceInput
    number_input_scene_on_enter,   // RailfenceKeyInput
    number_input_scene_on_enter,   // RailfenceDecryptKeyInput
    dialog_cipher_output_scene_on_enter,  // RailfenceOutput
    cipher_input_scene_on_enter,   // RailfenceDecryptInput
    dialog_cipher_output_scene_on_enter,  // RailfenceDecryptOutput
    cipher_learn_scene_on_enter,   // RailfenceLearn
    cipher_submenu_scene_on_enter, // RC4Submenu
    cipher_input_scene_on_enter,   // RC4Input
    cipher_input_scene_on_enter,   // RC4KeywordInput
    cipher_input_scene_on_enter,   // RC4DecryptKeywordInput
    dialog_cipher_output_scene_on_enter,  // RC4Output
    cipher_input_scene_on_enter,   // RC4DecryptInput
    dialog_cipher_output_scene_on_enter,  // RC4DecryptOutput
    cipher_learn_scene_on_enter,   // RC4Learn
    cipher_submenu_scene_on_enter, // ROT13Submenu
    cipher_input_scene_on_enter,   // ROT13Input
    dialog_cipher_output_scene_on_enter,  // ROT13Output
    cipher_input_scene_on_enter,   // ROT13DecryptInput
    dialog_cipher_output_scene_on_enter,  // ROT13DecryptOutput
    cipher_learn_scene_on_enter,   // ROT13Learn
    cipher_submenu_scene_on_enter, // ScytaleSubmenu
    cipher_input_scene_on_enter,   // ScytaleInput
    number_input_scene_on_enter,   // ScytaleKeywordInput
    number_input_scene_on_enter,   // ScytaleDecryptKeywordInput
    dialog_cipher_output_scene_on_enter,  // ScytaleOutput
    cipher_input_scene_on_enter,   // ScytaleDecryptInput
    dialog_cipher_output_scene_on_enter,  // ScytaleDecryptOutput
    cipher_learn_scene_on_enter,   // ScytaleLearn
    cipher_submenu_scene_on_enter, // VigenereSubmenu
    cipher_input_scene_on_enter,   // VigenereInput
    cipher_input_scene_on_enter,   // VigenereKeywordInput
    cipher_input_scene_on_enter,   // VigenereDecryptKeywordInput
    dialog_cipher_output_scene_on_enter,  // VigenereOutput
    cipher_input_scene_on_enter,   // VigenereDecryptInput
    dialog_cipher_output_scene_on_enter,  // VigenereDecryptOutput
    cipher_learn_scene_on_enter,   // VigenereLearn
    cipher_submenu_scene_on_enter, // Blake2Submenu
    cipher_input_scene_on_enter,   // Blake2Input
    dialog_cipher_output_scene_on_enter,  // Blake2Output
    cipher_learn_scene_on_enter,   // Blake2Learn
    cipher_submenu_scene_on_enter, // FNV1ASubmenu
    cipher_input_scene_on_enter,   // FNV1AInput
    dialog_cipher_output_scene_on_enter,  // FNV1AOutput
    cipher_learn_scene_on_enter,   // FNV1ALearn
    cipher_submenu_scene_on_enter, // MD2Submenu
    cipher_input_scene_on_enter,   // MD2Input
    dialog_cipher_output_scene_on_enter,  // MD2Output
    cipher_learn_scene_on_enter,   // MD2Learn
    cipher_submenu_scene_on_enter, // MD5Submenu
    cipher_input_scene_on_enter,   // MD5Input
    dialog_cipher_output_scene_on_enter,  // MD5Output
    cipher_learn_scene_on_enter,   // MD5Learn
    cipher_submenu_scene_on_enter, // Murmur3Submenu
    cipher_input_scene_on_enter,   // Murmur3Input
    dialog_cipher_output_scene_on_enter,  // Murmur3Output
    cipher_learn_scene_on_enter,   // Murmur3Learn
    cipher_submenu_scene_on_enter, // SipSubmenu
    cipher_input_scene_on_enter,   // SipInput
    cipher_input_scene_on_enter,   // SipKeywordInput
    dialog_cipher_output_scene_on_enter,  // SipOutput
    cipher_learn_scene_on_enter,   // SipLearn
    cipher_submenu_scene_on_enter, // SHA1Submenu
    cipher_input_scene_on_enter,   // SHA1Input
    dialog_cipher_output_scene_on_enter,  // SHA1Output
    cipher_learn_scene_on_enter,   // SHA1Learn
    cipher_submenu_scene_on_enter, // SHA224Submenu
    cipher_input_scene_on_enter,   // SHA224Input
    dialog_cipher_output_scene_on_enter,  // SHA224Output
    cipher_learn_scene_on_enter,   // SHA224Learn
    cipher_submenu_scene_on_enter, // SHA256Submenu
    cipher_input_scene_on_enter,   // SHA256Input
    dialog_cipher_output_scene_on_enter,  // SHA256Output
    cipher_learn_scene_on_enter,   // SHA256Learn
    cipher_submenu_scene_on_enter, // SHA384Submenu
    cipher_input_scene_on_enter,   // SHA384Input
    dialog_cipher_output_scene_on_enter,  // SHA384Output
    cipher_learn_scene_on_enter,   // SHA384Learn
    cipher_submenu_scene_on_enter, // SHA512Submenu
    cipher_input_scene_on_enter,   // SHA512Input
    dialog_cipher_output_scene_on_enter,  // SHA512Output
    cipher_learn_scene_on_enter,   // SHA512Learn
    cipher_submenu_scene_on_enter, // XXSubmenu
    cipher_input_scene_on_enter,   // XXInput
    dialog_cipher_output_scene_on_enter,  // XXOutput
    cipher_learn_scene_on_enter,   // XXLearn
    cipher_submenu_scene_on_enter, // Base32Submenu
    cipher_input_scene_on_enter,   // Base32Input
    dialog_cipher_output_scene_on_enter,  // Base32Output
    cipher_input_scene_on_enter,   // Base32DecryptInput
    dialog_cipher_output_scene_on_enter,  // Base32DecryptOutput
    cipher_learn_scene_on_enter,   // Base32Learn
    cipher_submenu_scene_on_enter, // Base58Submenu
    cipher_input_scene_on_enter,   // Base58Input
    dialog_cipher_output_scene_on_enter,  // Base58Output
    cipher_input_scene_on_enter,   // Base58DecryptInput
    dialog_cipher_output_scene_on_enter,  // Base58DecryptOutput
    cipher_learn_scene_on_enter,   // Base58Learn
    cipher_submenu_scene_on_enter, // Base64Submenu
    cipher_input_scene_on_enter,   // Base64Input
    dialog_cipher_output_scene_on_enter,  // Base64Output
    cipher_input_scene_on_enter,   // Base64DecryptInput
    dialog_cipher_output_scene_on_enter,  // Base64DecryptOutput
    cipher_learn_scene_on_enter,   // Base64Learn
    flip_crypt_nfc_scene_on_enter, // NFC
    flip_crypt_save_scene_on_enter, // Save
    cipher_input_scene_on_enter, // Save Text input for file name
    flip_crypt_qr_scene_on_enter, // QR Code
};

bool (*const flip_crypt_scene_on_event_handlers[])(void*, SceneManagerEvent) = {
    flip_crypt_main_menu_scene_on_event, // Main menu
    flip_crypt_main_menu_scene_on_event, // Cipher submenu
    flip_crypt_main_menu_scene_on_event, // Hash submenu
    flip_crypt_main_menu_scene_on_event, // Other submenu
    flip_crypt_generic_event_handler, // About
    flip_crypt_generic_event_handler, // AES128Submenu
    flip_crypt_generic_event_handler, // AES128Input
    flip_crypt_generic_event_handler, // AES128KeyInput
    flip_crypt_generic_event_handler, // AES128DecryptKeyInput
    flip_crypt_generic_event_handler, // AES128Output
    flip_crypt_generic_event_handler, // AES128DecryptInput
    flip_crypt_generic_event_handler, // AES128DecryptOutput
    flip_crypt_generic_event_handler, // AES128Learn
    flip_crypt_generic_event_handler, // AffineSubmenu
    flip_crypt_generic_event_handler, // AffineInput
    flip_crypt_generic_event_handler, // AffineKeyAInput
    flip_crypt_generic_event_handler, // AffineDecryptKeyAInput
    flip_crypt_generic_event_handler, // AffineKeyBInput
    flip_crypt_generic_event_handler, // AffineDecryptKeyBInput
    flip_crypt_generic_event_handler, // AffineOutput
    flip_crypt_generic_event_handler, // AffineDecryptInput
    flip_crypt_generic_event_handler, // AffineDecryptOutput
    flip_crypt_generic_event_handler, // AffineLearn
    flip_crypt_generic_event_handler, // AtbashSubmenu
    flip_crypt_generic_event_handler, // AtbashInput
    flip_crypt_generic_event_handler, // AtbashOutput
    flip_crypt_generic_event_handler, // AtbashDecryptInput
    flip_crypt_generic_event_handler, // AtbashDecryptOutput
    flip_crypt_generic_event_handler, // AtbashLearn
    flip_crypt_generic_event_handler, // BaconianSubmenu
    flip_crypt_generic_event_handler, // BaconianInput
    flip_crypt_generic_event_handler, // BaconianOutput
    flip_crypt_generic_event_handler, // BaconianDecryptInput
    flip_crypt_generic_event_handler, // BaconianDecryptOutput
    flip_crypt_generic_event_handler, // BaconianLearn
    flip_crypt_generic_event_handler, // BeaufortSubmenu
    flip_crypt_generic_event_handler, // BeaufortInput
    flip_crypt_generic_event_handler, // BeaufortKeyInput
    flip_crypt_generic_event_handler, // BeaufortDecryptKeyInput
    flip_crypt_generic_event_handler, // BeaufortOutput
    flip_crypt_generic_event_handler, // BeaufortDecryptInput
    flip_crypt_generic_event_handler, // BeaufortDecryptOutput
    flip_crypt_generic_event_handler, // BeaufortLearn
    flip_crypt_generic_event_handler, // CaesarSubmenu
    flip_crypt_generic_event_handler, // CaesarInput
    flip_crypt_generic_event_handler, // CaesarKeyInput
    flip_crypt_generic_event_handler, // CaesarDecryptKeyInput
    flip_crypt_generic_event_handler, // CaesarOutput
    flip_crypt_generic_event_handler, // CaesarDecryptInput
    flip_crypt_generic_event_handler, // CaesarDecryptOutput
    flip_crypt_generic_event_handler, // CaesarLearn
    flip_crypt_generic_event_handler, // PlayfairSubmenu
    flip_crypt_generic_event_handler, // PlayfairInput
    flip_crypt_generic_event_handler, // PlayfairKeywordInput
    flip_crypt_generic_event_handler, // PlayfairDecryptKeywordInput
    flip_crypt_generic_event_handler, // PlayfairOutput
    flip_crypt_generic_event_handler, // PlayfairDecryptInput
    flip_crypt_generic_event_handler, // PlayfairDecryptOutput
    flip_crypt_generic_event_handler, // PlayfairLearn
    flip_crypt_generic_event_handler, // PolybiusSubmenu
    flip_crypt_generic_event_handler, // PolybiusInput
    flip_crypt_generic_event_handler, // PolybiusOutput
    flip_crypt_generic_event_handler, // PolybiusDecryptInput
    flip_crypt_generic_event_handler, // PolybiusDecryptOutput
    flip_crypt_generic_event_handler, // PolybiusLearn
    flip_crypt_generic_event_handler, // PortaSubmenu
    flip_crypt_generic_event_handler, // PortaInput
    flip_crypt_generic_event_handler, // PortaKeywordInput
    flip_crypt_generic_event_handler, // PortaDecryptKeywordInput
    flip_crypt_generic_event_handler, // PortaOutput
    flip_crypt_generic_event_handler, // PortaDecryptInput
    flip_crypt_generic_event_handler, // PortaDecryptOutput
    flip_crypt_generic_event_handler, // PortaLearn
    flip_crypt_generic_event_handler, // RailfenceSubmenu
    flip_crypt_generic_event_handler, // RailfenceInput
    flip_crypt_generic_event_handler, // RailfenceKeyInput
    flip_crypt_generic_event_handler, // RailfenceDecryptKeyInput
    flip_crypt_generic_event_handler, // RailfenceOutput
    flip_crypt_generic_event_handler, // RailfenceDecryptInput
    flip_crypt_generic_event_handler, // RailfenceDecryptOutput
    flip_crypt_generic_event_handler, // RailfenceLearn
    flip_crypt_generic_event_handler, // RC4Submenu
    flip_crypt_generic_event_handler, // RC4Input
    flip_crypt_generic_event_handler, // RC4KeywordInput
    flip_crypt_generic_event_handler, // RC4DecryptKeywordInput
    flip_crypt_generic_event_handler, // RC4Output
    flip_crypt_generic_event_handler, // RC4DecryptInput
    flip_crypt_generic_event_handler, // RC4DecryptOutput
    flip_crypt_generic_event_handler, // RC4Learn
    flip_crypt_generic_event_handler, // ROT13Submenu
    flip_crypt_generic_event_handler, // ROT13Input
    flip_crypt_generic_event_handler, // ROT13Output
    flip_crypt_generic_event_handler, // ROT13DecryptInput
    flip_crypt_generic_event_handler, // ROT13DecryptOutput
    flip_crypt_generic_event_handler, // ROT13Learn
    flip_crypt_generic_event_handler, // ScytaleSubmenu
    flip_crypt_generic_event_handler, // ScytaleInput
    flip_crypt_generic_event_handler, // ScytaleKeywordInput
    flip_crypt_generic_event_handler, // ScytaleDecryptKeywordInput
    flip_crypt_generic_event_handler, // ScytaleOutput
    flip_crypt_generic_event_handler, // ScytaleDecryptInput
    flip_crypt_generic_event_handler, // ScytaleDecryptOutput
    flip_crypt_generic_event_handler, // ScytaleLearn
    flip_crypt_generic_event_handler, // VigenereSubmenu
    flip_crypt_generic_event_handler, // VigenereInput
    flip_crypt_generic_event_handler, // VigenereKeywordInput
    flip_crypt_generic_event_handler, // VigenereDecryptKeywordInput
    flip_crypt_generic_event_handler, // VigenereOutput
    flip_crypt_generic_event_handler, // VigenereDecryptInput
    flip_crypt_generic_event_handler, // VigenereDecryptOutput
    flip_crypt_generic_event_handler, // VigenereLearn
    flip_crypt_generic_event_handler, // Blake2Submenu
    flip_crypt_generic_event_handler, // Blake2Input
    flip_crypt_generic_event_handler, // Blake2Output
    flip_crypt_generic_event_handler, // Blake2Learn
    flip_crypt_generic_event_handler, // FNV1ASubmenu
    flip_crypt_generic_event_handler, // FNV1AInput
    flip_crypt_generic_event_handler, // FNV1AOutput
    flip_crypt_generic_event_handler, // FNV1ALearn
    flip_crypt_generic_event_handler, // MD2Submenu
    flip_crypt_generic_event_handler, // MD2Input
    flip_crypt_generic_event_handler, // MD2Output
    flip_crypt_generic_event_handler, // MD2Learn
    flip_crypt_generic_event_handler, // MD5Submenu
    flip_crypt_generic_event_handler, // MD5Input
    flip_crypt_generic_event_handler, // MD5Output
    flip_crypt_generic_event_handler, // MD5Learn
    flip_crypt_generic_event_handler, // Murmur3Submenu
    flip_crypt_generic_event_handler, // Murmur3Input
    flip_crypt_generic_event_handler, // Murmur3Output
    flip_crypt_generic_event_handler, // Murmur3Learn
    flip_crypt_generic_event_handler, // SipSubmenu
    flip_crypt_generic_event_handler, // SipInput
    flip_crypt_generic_event_handler, // SipKeywordInput
    flip_crypt_generic_event_handler, // SipOutput
    flip_crypt_generic_event_handler, // SipLearn
    flip_crypt_generic_event_handler, // SHA1Submenu
    flip_crypt_generic_event_handler, // SHA1Input
    flip_crypt_generic_event_handler, // SHA1Output
    flip_crypt_generic_event_handler, // SHA1Learn
    flip_crypt_generic_event_handler, // SHA224Submenu
    flip_crypt_generic_event_handler, // SHA224Input
    flip_crypt_generic_event_handler, // SHA224Output
    flip_crypt_generic_event_handler, // SHA224Learn
    flip_crypt_generic_event_handler, // SHA256Submenu
    flip_crypt_generic_event_handler, // SHA256Input
    flip_crypt_generic_event_handler, // SHA256Output
    flip_crypt_generic_event_handler, // SHA256Learn
    flip_crypt_generic_event_handler, // SHA384Submenu
    flip_crypt_generic_event_handler, // SHA384Input
    flip_crypt_generic_event_handler, // SHA384Output
    flip_crypt_generic_event_handler, // SHA384Learn
    flip_crypt_generic_event_handler, // SHA512Submenu
    flip_crypt_generic_event_handler, // SHA512Input
    flip_crypt_generic_event_handler, // SHA512Output
    flip_crypt_generic_event_handler, // SHA512Learn
    flip_crypt_generic_event_handler, // XXSubmenu
    flip_crypt_generic_event_handler, // XXInput
    flip_crypt_generic_event_handler, // XXOutput
    flip_crypt_generic_event_handler, // XXLearn
    flip_crypt_generic_event_handler, // Base32Submenu
    flip_crypt_generic_event_handler, // Base32Input
    flip_crypt_generic_event_handler, // Base32Output
    flip_crypt_generic_event_handler, // Base32DecryptInput
    flip_crypt_generic_event_handler, // Base32DecryptOutput
    flip_crypt_generic_event_handler, // Base32Learn
    flip_crypt_generic_event_handler, // Base58Submenu
    flip_crypt_generic_event_handler, // Base58Input
    flip_crypt_generic_event_handler, // Base58Output
    flip_crypt_generic_event_handler, // Base58DecryptInput
    flip_crypt_generic_event_handler, // Base58DecryptOutput
    flip_crypt_generic_event_handler, // Base58Learn
    flip_crypt_generic_event_handler, // Base64Submenu
    flip_crypt_generic_event_handler, // Base64Input
    flip_crypt_generic_event_handler, // Base64Output
    flip_crypt_generic_event_handler, // Base64DecryptInput
    flip_crypt_generic_event_handler, // Base64DecryptOutput
    flip_crypt_generic_event_handler, // Base64Learn
    flip_crypt_generic_event_handler, // NFC
    flip_crypt_generic_event_handler, // Save
    flip_crypt_generic_event_handler, // Save input
    flip_crypt_generic_event_handler, // QR
};

void (*const flip_crypt_scene_on_exit_handlers[])(void*) = {
    flip_crypt_main_menu_scene_on_exit, // Main menu
    flip_crypt_main_menu_scene_on_exit, // Cipher submenu
    flip_crypt_main_menu_scene_on_exit, // Hash submenu
    flip_crypt_main_menu_scene_on_exit, // Other submenu
    flip_crypt_generic_on_exit, // About
    flip_crypt_generic_on_exit, // AES128Submenu
    flip_crypt_generic_on_exit, // AES128Input
    flip_crypt_generic_on_exit, // AES128KeyInput
    flip_crypt_generic_on_exit, // AES128DecryptKeyInput
    flip_crypt_generic_on_exit, // AES128Output
    flip_crypt_generic_on_exit, // AES128DecryptInput
    flip_crypt_generic_on_exit, // AES128DecryptOutput
    flip_crypt_generic_on_exit, // AES128Learn
    flip_crypt_generic_on_exit, // AffineSubmenu
    flip_crypt_generic_on_exit, // AffineInput
    flip_crypt_generic_on_exit, // AffineKeyAInput
    flip_crypt_generic_on_exit, // AffineDecryptKeyAInput
    flip_crypt_generic_on_exit, // AffineKeyBInput
    flip_crypt_generic_on_exit, // AffineDecryptKeyBInput
    flip_crypt_generic_on_exit, // AffineOutput
    flip_crypt_generic_on_exit, // AffineDecryptInput
    flip_crypt_generic_on_exit, // AffineDecryptOutput
    flip_crypt_generic_on_exit, // AffineLearn
    flip_crypt_generic_on_exit, // AtbashSubmenu
    flip_crypt_generic_on_exit, // AtbashInput
    flip_crypt_generic_on_exit, // AtbashOutput
    flip_crypt_generic_on_exit, // AtbashDecryptInput
    flip_crypt_generic_on_exit, // AtbashDecryptOutput
    flip_crypt_generic_on_exit, // AtbashLearn
    flip_crypt_generic_on_exit, // BaconianSubmenu
    flip_crypt_generic_on_exit, // BaconianInput
    flip_crypt_generic_on_exit, // BaconianOutput
    flip_crypt_generic_on_exit, // BaconianDecryptInput
    flip_crypt_generic_on_exit, // BaconianDecryptOutput
    flip_crypt_generic_on_exit, // BaconianLearn
    flip_crypt_generic_on_exit, // BeaufortSubmenu
    flip_crypt_generic_on_exit, // BeaufortInput
    flip_crypt_generic_on_exit, // BeaufortKeyInput
    flip_crypt_generic_on_exit, // BeaufortDecryptKeyInput
    flip_crypt_generic_on_exit, // BeaufortOutput
    flip_crypt_generic_on_exit, // BeaufortDecryptInput
    flip_crypt_generic_on_exit, // BeaufortDecryptOutput
    flip_crypt_generic_on_exit, // BeaufortLearn
    flip_crypt_generic_on_exit, // CaesarSubmenu
    flip_crypt_generic_on_exit, // CaesarInput
    flip_crypt_generic_on_exit, // CaesarKeyInput
    flip_crypt_generic_on_exit, // CaesarDecryptKeyInput
    flip_crypt_generic_on_exit, // CaesarOutput
    flip_crypt_generic_on_exit, // CaesarDecryptInput
    flip_crypt_generic_on_exit, // CaesarDecryptOutput
    flip_crypt_generic_on_exit, // CaesarLearn
    flip_crypt_generic_on_exit, // PlayfairSubmenu
    flip_crypt_generic_on_exit, // PlayfairInput
    flip_crypt_generic_on_exit, // PlayfairKeywordInput
    flip_crypt_generic_on_exit, // PlayfairDecryptKeywordInput
    flip_crypt_generic_on_exit, // PlayfairOutput
    flip_crypt_generic_on_exit, // PlayfairDecryptInput
    flip_crypt_generic_on_exit, // PlayfairDecryptOutput
    flip_crypt_generic_on_exit, // PlayfairLearn
    flip_crypt_generic_on_exit, // PolybiusSubmenu
    flip_crypt_generic_on_exit, // PolybiusInput
    flip_crypt_generic_on_exit, // PolybiusOutput
    flip_crypt_generic_on_exit, // PolybiusDecryptInput
    flip_crypt_generic_on_exit, // PolybiusDecryptOutput
    flip_crypt_generic_on_exit, // PolybiusLearn
    flip_crypt_generic_on_exit, // PortaSubmenu
    flip_crypt_generic_on_exit, // PortaInput
    flip_crypt_generic_on_exit, // PortaKeywordInput
    flip_crypt_generic_on_exit, // PortaDecryptKeywordInput
    flip_crypt_generic_on_exit, // PortaOutput
    flip_crypt_generic_on_exit, // PortaDecryptInput
    flip_crypt_generic_on_exit, // PortaDecryptOutput
    flip_crypt_generic_on_exit, // PortaLearn
    flip_crypt_generic_on_exit, // RailfenceSubmenu
    flip_crypt_generic_on_exit, // RailfenceInput
    flip_crypt_generic_on_exit, // RailfenceKeyInput
    flip_crypt_generic_on_exit, // RailfenceDecryptKeyInput
    flip_crypt_generic_on_exit, // RailfenceOutput
    flip_crypt_generic_on_exit, // RailfenceDecryptInput
    flip_crypt_generic_on_exit, // RailfenceDecryptOutput
    flip_crypt_generic_on_exit, // RailfenceLearn
    flip_crypt_generic_on_exit, // RC4Submenu
    flip_crypt_generic_on_exit, // RC4Input
    flip_crypt_generic_on_exit, // RC4KeywordInput
    flip_crypt_generic_on_exit, // RC4DecryptKeywordInput
    flip_crypt_generic_on_exit, // RC4Output
    flip_crypt_generic_on_exit, // RC4DecryptInput
    flip_crypt_generic_on_exit, // RC4DecryptOutput
    flip_crypt_generic_on_exit, // RC4Learn
    flip_crypt_generic_on_exit, // ROT13Submenu
    flip_crypt_generic_on_exit, // ROT13Input
    flip_crypt_generic_on_exit, // ROT13Output
    flip_crypt_generic_on_exit, // ROT13DecryptInput
    flip_crypt_generic_on_exit, // ROT13DecryptOutput
    flip_crypt_generic_on_exit, // ROT13Learn
    flip_crypt_generic_on_exit, // ScytaleSubmenu
    flip_crypt_generic_on_exit, // ScytaleInput
    flip_crypt_generic_on_exit, // ScytaleKeywordInput
    flip_crypt_generic_on_exit, // ScytaleDecryptKeywordInput
    flip_crypt_generic_on_exit, // ScytaleOutput
    flip_crypt_generic_on_exit, // ScytaleDecryptInput
    flip_crypt_generic_on_exit, // ScytaleDecryptOutput
    flip_crypt_generic_on_exit, // ScytaleLearn
    flip_crypt_generic_on_exit, // VigenereSubmenu
    flip_crypt_generic_on_exit, // VigenereInput
    flip_crypt_generic_on_exit, // VigenereKeywordInput
    flip_crypt_generic_on_exit, // VigenereDecryptKeywordInput
    flip_crypt_generic_on_exit, // VigenereOutput
    flip_crypt_generic_on_exit, // VigenereDecryptInput
    flip_crypt_generic_on_exit, // VigenereDecryptOutput
    flip_crypt_generic_on_exit, // VigenereLearn
    flip_crypt_generic_on_exit, // Blake2Submenu
    flip_crypt_generic_on_exit, // Blake2Input
    flip_crypt_generic_on_exit, // Blake2Output
    flip_crypt_generic_on_exit, // Blake2Learn
    flip_crypt_generic_on_exit, // FNV1ASubmenu
    flip_crypt_generic_on_exit, // FNV1AInput
    flip_crypt_generic_on_exit, // FNV1AOutput
    flip_crypt_generic_on_exit, // FNV1ALearn
    flip_crypt_generic_on_exit, // MD2Submenu
    flip_crypt_generic_on_exit, // MD2Input
    flip_crypt_generic_on_exit, // MD2Output
    flip_crypt_generic_on_exit, // MD2Learn
    flip_crypt_generic_on_exit, // MD5Submenu
    flip_crypt_generic_on_exit, // MD5Input
    flip_crypt_generic_on_exit, // MD5Output
    flip_crypt_generic_on_exit, // MD5Learn
    flip_crypt_generic_on_exit, // Murmur3Submenu
    flip_crypt_generic_on_exit, // Murmur3Input
    flip_crypt_generic_on_exit, // Murmur3Output
    flip_crypt_generic_on_exit, // Murmur3Learn
    flip_crypt_generic_on_exit, // SipSubmenu
    flip_crypt_generic_on_exit, // SipInput
    flip_crypt_generic_on_exit, // SipKeywordInput
    flip_crypt_generic_on_exit, // SipOutput
    flip_crypt_generic_on_exit, // SipLearn
    flip_crypt_generic_on_exit, // SHA1Submenu
    flip_crypt_generic_on_exit, // SHA1Input
    flip_crypt_generic_on_exit, // SHA1Output
    flip_crypt_generic_on_exit, // SHA1Learn
    flip_crypt_generic_on_exit, // SHA224Submenu
    flip_crypt_generic_on_exit, // SHA224Input
    flip_crypt_generic_on_exit, // SHA224Output
    flip_crypt_generic_on_exit, // SHA224Learn
    flip_crypt_generic_on_exit, // SHA256Submenu
    flip_crypt_generic_on_exit, // SHA256Input
    flip_crypt_generic_on_exit, // SHA256Output
    flip_crypt_generic_on_exit, // SHA256Learn
    flip_crypt_generic_on_exit, // SHA384Submenu
    flip_crypt_generic_on_exit, // SHA384Input
    flip_crypt_generic_on_exit, // SHA384Output
    flip_crypt_generic_on_exit, // SHA384Learn
    flip_crypt_generic_on_exit, // SHA512Submenu
    flip_crypt_generic_on_exit, // SHA512Input
    flip_crypt_generic_on_exit, // SHA512Output
    flip_crypt_generic_on_exit, // SHA512Learn
    flip_crypt_generic_on_exit, // XXSubmenu
    flip_crypt_generic_on_exit, // XXInput
    flip_crypt_generic_on_exit, // XXOutput
    flip_crypt_generic_on_exit, // XXLearn
    flip_crypt_generic_on_exit, // Base32Submenu
    flip_crypt_generic_on_exit, // Base32Input
    flip_crypt_generic_on_exit, // Base32Output
    flip_crypt_generic_on_exit, // Base32DecryptInput
    flip_crypt_generic_on_exit, // Base32DecryptOutput
    flip_crypt_generic_on_exit, // Base32Learn
    flip_crypt_generic_on_exit, // Base58Submenu
    flip_crypt_generic_on_exit, // Base58Input
    flip_crypt_generic_on_exit, // Base58Output
    flip_crypt_generic_on_exit, // Base58DecryptInput
    flip_crypt_generic_on_exit, // Base58DecryptOutput
    flip_crypt_generic_on_exit, // Base58Learn
    flip_crypt_generic_on_exit, // Base64Submenu
    flip_crypt_generic_on_exit, // Base64Input
    flip_crypt_generic_on_exit, // Base64Output
    flip_crypt_generic_on_exit, // Base64DecryptInput
    flip_crypt_generic_on_exit, // Base64DecryptOutput
    flip_crypt_generic_on_exit, // Base64Learn
    flip_crypt_nfc_scene_on_exit, // NFC
    flip_crypt_generic_on_exit, // Save
    flip_crypt_generic_on_exit, // Save input
    flip_crypt_qr_scene_on_exit, // QR
};

static const SceneManagerHandlers flip_crypt_scene_manager_handlers = {
    .on_enter_handlers = flip_crypt_scene_on_enter_handlers,
    .on_event_handlers = flip_crypt_scene_on_event_handlers,
    .on_exit_handlers = flip_crypt_scene_on_exit_handlers,
    .scene_num = FlipCryptSceneCount,
};

// Custom event callbacks
static bool basic_scene_custom_callback(void* context, uint32_t custom_event) {
    furi_assert(context);
    App* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, custom_event);
}

bool basic_scene_back_event_callback(void* context) {
    furi_assert(context);
    App* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

// Memory allocation
static App* app_alloc() {
    // App
    App* app = malloc(sizeof(App));
    // Vars
    app->last_output_scene_size = 64;
    app->save_name_input_size = 64;
    // universal input
    app->universal_input_size = 255;
    app->aes_key_input_size = 17;
    app->beaufort_key_input_size = 64;
    app->playfair_keyword_input_size = 26;
    app->porta_keyword_input_size = 64;
    app->rc4_keyword_input_size = 64;
    app->vigenere_keyword_input_size = 64;
    app->universal_input = malloc(app->universal_input_size);
    app->save_name_input = malloc(app->save_name_input_size);
    app->last_output_scene = malloc(app->last_output_scene_size);
    app->aes_key_input = malloc(app->aes_key_input_size);
    app->affine_keya_input = 1;
    app->affine_keyb_input = 1;
    app->beaufort_key_input = malloc(app->beaufort_key_input_size);
    app->caesar_key_input = 0;
    app->playfair_keyword_input = malloc(app->playfair_keyword_input_size);
    app->porta_keyword_input = malloc(app->porta_keyword_input_size);
    app->railfence_key_input = 1;
    app->rc4_keyword_input = malloc(app->rc4_keyword_input_size);
    app->scytale_keyword_input = 0;
    app->vigenere_keyword_input = malloc(app->vigenere_keyword_input_size);
    // Other
    app->scene_manager = scene_manager_alloc(&flip_crypt_scene_manager_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, basic_scene_custom_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, basic_scene_back_event_callback);
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(app->view_dispatcher, FlipCryptSubmenuView, submenu_get_view(app->submenu));
    app->widget = widget_alloc();
    view_dispatcher_add_view(app->view_dispatcher, FlipCryptWidgetView, widget_get_view(app->widget));
    app->text_input = text_input_alloc();
    view_dispatcher_add_view(app->view_dispatcher, FlipCryptTextInputView, text_input_get_view(app->text_input));
    app->number_input = number_input_alloc();
    view_dispatcher_add_view(app->view_dispatcher, FlipCryptNumberInputView, number_input_get_view(app->number_input));
    app->dialog_ex = dialog_ex_alloc();
    dialog_ex_set_context(app->dialog_ex, app);
    dialog_ex_set_result_callback(app->dialog_ex, dialog_ex_callback);
    view_dispatcher_add_view(app->view_dispatcher, FlipCryptDialogExView, dialog_ex_get_view(app->dialog_ex));
    app->nfc = nfc_alloc();
    app->nfc_device = nfc_device_alloc();
    app->qr_buffer = malloc(qrcodegen_BUFFER_LEN_MAX);
    app->qrcode = malloc(qrcodegen_BUFFER_LEN_MAX);
    return app;
}

// Free memory on app termination
static void app_free(App* app) {
    furi_assert(app);
    view_dispatcher_remove_view(app->view_dispatcher, FlipCryptSubmenuView);
    view_dispatcher_remove_view(app->view_dispatcher, FlipCryptWidgetView);
    view_dispatcher_remove_view(app->view_dispatcher, FlipCryptTextInputView);
    view_dispatcher_remove_view(app->view_dispatcher, FlipCryptDialogExView);
    view_dispatcher_remove_view(app->view_dispatcher, FlipCryptNumberInputView);
    dialog_ex_free(app->dialog_ex);
    number_input_free(app->number_input);
    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);
    submenu_free(app->submenu);
    widget_free(app->widget);
    text_input_free(app->text_input);
    furi_record_close(RECORD_NOTIFICATION);
    nfc_free(app->nfc);
    nfc_device_free(app->nfc_device);
    free(app->qr_buffer);
    free(app->qrcode);
    free(app->last_output_scene);
    free(app->universal_input);
    free(app->aes_key_input);
    free(app->beaufort_key_input);
    free(app->playfair_keyword_input);
    free(app->porta_keyword_input);
    free(app->rc4_keyword_input);
    free(app->vigenere_keyword_input);
    free(app->sip_keyword_input);
    free(app);
}

// Main function
int32_t flip_crypt_app(void* p) {
    UNUSED(p);
    App* app = app_alloc();
    Gui* gui = furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    scene_manager_next_scene(app->scene_manager, FlipCryptMainMenuScene);
    view_dispatcher_run(app->view_dispatcher);
    app_free(app);
    return 0; // return of 0 means success, return of 1 means failure
}
