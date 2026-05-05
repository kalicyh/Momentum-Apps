#include "../nfc_playlist.h"

typedef enum {
   NfcPlaylistPlaylistEdit_CreatePlaylist,
   NfcPlaylistPlaylistEdit_DeletePlaylist,
   NfcPlaylistPlaylistEdit_RenamePlaylist,
   NfcPlaylistPlaylistEdit_AddNfcItem,
   NfcPlaylistPlaylistEdit_RemoveNfcItem,
   NfcPlaylistPlaylistEdit_MoveNfcItem,
   NfcPlaylistPlaylistEdit_ViewPlaylistContent
} NfcPlaylistPlaylistEditMenuSelection;

static void nfc_playlist_playlist_edit_scene_menu_callback(void* context, uint32_t index) {
   furi_assert(context);
   NfcPlaylist* nfc_playlist = context;
   scene_manager_handle_custom_event(nfc_playlist->scene_manager, index);
}

void nfc_playlist_playlist_edit_scene_on_enter(void* context) {
   NfcPlaylist* nfc_playlist = context;

   submenu_set_header(nfc_playlist->views.submenu, NFC_PLAYLIST_UI_TEXT("Edit Playlist", "编辑播放列表"));

   bool playlist_path_empty = furi_string_empty(nfc_playlist->worker_info.settings->playlist_path);

   submenu_add_item(
      nfc_playlist->views.submenu,
      NFC_PLAYLIST_UI_TEXT("Create Playlist", "创建播放列表"),
      NfcPlaylistPlaylistEdit_CreatePlaylist,
      nfc_playlist_playlist_edit_scene_menu_callback,
      nfc_playlist);

   submenu_add_lockable_item(
      nfc_playlist->views.submenu,
      NFC_PLAYLIST_UI_TEXT("Delete Playlist", "删除播放列表"),
      NfcPlaylistPlaylistEdit_DeletePlaylist,
      nfc_playlist_playlist_edit_scene_menu_callback,
      nfc_playlist,
      playlist_path_empty,
      NFC_PLAYLIST_UI_TEXT("No\nplaylist\nselected", "未选择\n播放列表"));

   submenu_add_lockable_item(
      nfc_playlist->views.submenu,
      NFC_PLAYLIST_UI_TEXT("Rename Playlist", "重命名播放列表"),
      NfcPlaylistPlaylistEdit_RenamePlaylist,
      nfc_playlist_playlist_edit_scene_menu_callback,
      nfc_playlist,
      playlist_path_empty,
      NFC_PLAYLIST_UI_TEXT("No\nplaylist\nselected", "未选择\n播放列表"));

   submenu_add_lockable_item(
      nfc_playlist->views.submenu,
      NFC_PLAYLIST_UI_TEXT("Add NFC Item", "添加 NFC 项目"),
      NfcPlaylistPlaylistEdit_AddNfcItem,
      nfc_playlist_playlist_edit_scene_menu_callback,
      nfc_playlist,
      playlist_path_empty,
      NFC_PLAYLIST_UI_TEXT("No\nplaylist\nselected", "未选择\n播放列表"));

   submenu_add_lockable_item(
      nfc_playlist->views.submenu,
      NFC_PLAYLIST_UI_TEXT("Remove NFC Item", "移除 NFC 项目"),
      NfcPlaylistPlaylistEdit_RemoveNfcItem,
      nfc_playlist_playlist_edit_scene_menu_callback,
      nfc_playlist,
      playlist_path_empty,
      NFC_PLAYLIST_UI_TEXT("No\nplaylist\nselected", "未选择\n播放列表"));

   submenu_add_lockable_item(
      nfc_playlist->views.submenu,
      NFC_PLAYLIST_UI_TEXT("Move NFC Item", "移动 NFC 项目"),
      NfcPlaylistPlaylistEdit_MoveNfcItem,
      nfc_playlist_playlist_edit_scene_menu_callback,
      nfc_playlist,
      playlist_path_empty,
      NFC_PLAYLIST_UI_TEXT("No\nplaylist\nselected", "未选择\n播放列表"));

   submenu_add_lockable_item(
      nfc_playlist->views.submenu,
      NFC_PLAYLIST_UI_TEXT("View Playlist Content", "查看播放列表内容"),
      NfcPlaylistPlaylistEdit_ViewPlaylistContent,
      nfc_playlist_playlist_edit_scene_menu_callback,
      nfc_playlist,
      playlist_path_empty,
      NFC_PLAYLIST_UI_TEXT("No\nplaylist\nselected", "未选择\n播放列表"));

   view_dispatcher_switch_to_view(nfc_playlist->view_dispatcher, NfcPlaylistView_Submenu);
}

bool nfc_playlist_playlist_edit_scene_on_event(void* context, SceneManagerEvent event) {
   NfcPlaylist* nfc_playlist = context;
   bool consumed = false;
   if(event.type == SceneManagerEventTypeCustom) {
      switch(event.event) {
      case NfcPlaylistPlaylistEdit_CreatePlaylist:
         scene_manager_next_scene(nfc_playlist->scene_manager, NfcPlaylistScene_NameNewPlaylist);
         consumed = true;
         break;
      case NfcPlaylistPlaylistEdit_DeletePlaylist:
         scene_manager_next_scene(nfc_playlist->scene_manager, NfcPlaylistScene_ConfirmDelete);
         consumed = true;
         break;
      case NfcPlaylistPlaylistEdit_RenamePlaylist:
         scene_manager_next_scene(nfc_playlist->scene_manager, NfcPlaylistScene_PlaylistRename);
         consumed = true;
         break;
      case NfcPlaylistPlaylistEdit_AddNfcItem:
         scene_manager_next_scene(nfc_playlist->scene_manager, NfcPlaylistScene_NfcAdd);
         consumed = true;
         break;
      case NfcPlaylistPlaylistEdit_RemoveNfcItem:
         scene_manager_next_scene(nfc_playlist->scene_manager, NfcPlaylistScene_NfcRemove);
         consumed = true;
         break;
      case NfcPlaylistPlaylistEdit_MoveNfcItem:
         scene_manager_next_scene(nfc_playlist->scene_manager, NfcPlaylistScene_NfcMoveItem);
         consumed = true;
         break;
      case NfcPlaylistPlaylistEdit_ViewPlaylistContent:
         scene_manager_next_scene(
            nfc_playlist->scene_manager, NfcPlaylistScene_ViewPlaylistContent);
         consumed = true;
         break;
      default:
         break;
      }
   }
   return consumed;
}

void nfc_playlist_playlist_edit_scene_on_exit(void* context) {
   NfcPlaylist* nfc_playlist = context;
   submenu_reset(nfc_playlist->views.submenu);
}
