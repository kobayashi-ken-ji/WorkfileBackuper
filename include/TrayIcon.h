#pragma once
#include <Windows.h>

// タスクトレイアイコン
// アイコン追加・削除、デスクトップ通知 を行う
class TrayIcon {
private:
    // 構造体のどのメンバが有効か
    static constexpr UINT FLAGS = NIF_ICON | NIF_TIP | NIF_MESSAGE;

    // トレイアイコン構造体
    NOTIFYICONDATA iconData;

public:
    // 通知の有効/無効
    bool enable;

    /**
    * コンストラクタ
    * @param tipText           マウスオーバー時の表示テキスト
    * @param iconID            アイコンの識別ID
    * @param callbackMessage   クリック時などに送られるメッセージ
    */
    TrayIcon(const WCHAR* tipText, int iconID, UINT callbackMessage);

    /**
    * ハンドルを設定 (WM_CREATE時に実行)
    * @param hWnd トレイアイコンのクリックメッセージの送信先
    * @param hIcon アプリのアイコン
    */
    void setHandle(HWND hWnd, HICON hIcon);

    // タスクトレイアイコン 追加
    void add();

    // タスクトレイアイコン 削除
    void deleteIcon();

    /**
    * デスクトップ通知を表示
    * @param message    メッセージテキスト
    * @param title      タイトルテキスト
    * @returns          通知が成功したか否か
    */
    void notify(const WCHAR* message, const WCHAR* title = L"\0");
};
