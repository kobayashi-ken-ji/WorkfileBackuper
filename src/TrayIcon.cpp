#include "../include/TrayIcon.h"
#include <cassert>

//=============================================================================
// タスクトレイアイコン
// アイコン追加・削除、デスクトップ通知 を行う
//=============================================================================

/**
* コンストラクタ
* @param tipText           マウスオーバー時の表示テキスト
* @param iconID            アイコンの識別ID
* @param callbackMessage   クリック時などに送られるメッセージ
*/
TrayIcon::TrayIcon(const WCHAR* tipText, int iconID, UINT callbackMessage)
    : enable(true), iconData{} {

    iconData.cbSize = sizeof(NOTIFYICONDATA);   // この構造体のサイズ
    iconData.uID = iconID;
    iconData.dwInfoFlags = NIIF_NOSOUND;        // サウンドなし
    iconData.uCallbackMessage = callbackMessage;
    lstrcpyW(iconData.szTip, tipText);
};


/**
* ハンドルを設定 (WM_CREATE時に実行)
* @param hWnd トレイアイコンのクリックメッセージの送信先
* @param hIcon アプリのアイコン
*/
void TrayIcon::setHandle(HWND hWnd, HICON hIcon) {
    iconData.hWnd  = hWnd;
    iconData.hIcon = hIcon;
}


// タスクトレイアイコン 追加
void TrayIcon::add() {
    deleteIcon();
    iconData.uFlags = FLAGS;
    Shell_NotifyIconW(NIM_ADD, &iconData);
}


// タスクトレイアイコン 削除
void TrayIcon::deleteIcon() {
    assert(iconData.hWnd && iconData.hIcon);
    Shell_NotifyIconW(NIM_DELETE, &iconData);
}


/**
* デスクトップ通知を表示
* @param message    メッセージテキスト
* @param title      タイトルテキスト
* @returns          通知が成功したか否か
*/
void TrayIcon::notify(const WCHAR* message, const WCHAR* title) {

    assert(iconData.hWnd && iconData.hIcon);

    // 通知が有効かチェック
    if (!enable) return;

    // 文字列を構造体へコピー
    wcscpy_s(iconData.szInfo, message);
    wcscpy_s(iconData.szInfoTitle, title);

    // アイコンの変更
    iconData.uFlags = FLAGS | NIF_INFO;
    Shell_NotifyIconW(NIM_MODIFY, &iconData);
}
