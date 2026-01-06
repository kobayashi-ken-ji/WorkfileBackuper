#pragma once
#include <Windows.h>

//=============================================================================
// タスクトレイアイコン [クラス]
//=============================================================================

class TrayIcon {
private:
    // どのメンバが有効か
    static constexpr UINT FLAGS = NIF_ICON | NIF_TIP | NIF_MESSAGE;

    // トレイアイコン構造体
    NOTIFYICONDATA iconData;

public:
    // 通知の有効/無効
    bool enable;

    // コンストラクタ
    TrayIcon(
        const   WCHAR* tipText,	    // マウスオーバー時の表示テキスト
        int     iconID,             // アイコンの識別ID
        UINT    callbackMessage     // クリック時などに送られるメッセージ

    ) : enable(true), iconData{} {

        iconData.cbSize = sizeof(NOTIFYICONDATA);   // この構造体のサイズ
        iconData.uID = iconID;
        iconData.dwInfoFlags = NIIF_NOSOUND;        // サウンドなし
        iconData.uCallbackMessage = callbackMessage;
        lstrcpyW(iconData.szTip, tipText);
    };


    // ハンドルを設定 (WM_CREATE時に実行)
    // @param hWnd トレイアイコンのクリックメッセージの送信先
    // @param hIcon アプリのアイコン
    void setHandle(HWND hWnd, HICON hIcon) {
        iconData.hWnd  = hWnd;
        iconData.hIcon = hIcon;
    }

    // タスクトレイアイコン 追加
    void add() {
        deleteIcon();
        iconData.uFlags = FLAGS;
        Shell_NotifyIconW(NIM_ADD, &iconData);
    }

    // タスクトレイアイコン 削除
    void deleteIcon() {
        //assert(iconData.hWnd && iconData.hIcon);
        Shell_NotifyIconW(NIM_DELETE, &iconData);
    }

    /**
    * デスクトップ通知を表示
    * @param message    メッセージテキスト
    * @param title      タイトルテキスト
    * @returns          通知が成功したか否か
    */
    void notify(const WCHAR* message, const WCHAR* title = L"\0") {

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
};
