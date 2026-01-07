#pragma once
#include "windows.h"

//=============================================================================
// 定数定義ファイル
//=============================================================================

// アプリ名 / バージョン / コピーライト
namespace AppInfo {
    constexpr PCWSTR NAME      = L"Workfile Backuper";
    constexpr PCWSTR NAME_JP   = L"作業ファイル自動バックアップ";
    constexpr PCWSTR VERSION   = L"0.01";
    constexpr PCWSTR COPYRIGHT = L"Copyright (C) 2023 Kobayashi Kenji";
}

// プロシージャ用
constexpr int ID_TRAYICON = WM_USER + 100;	// トレイアイコンの識別ID
constexpr int WM_TASKTRAY = WM_APP + 1;     // トレイクリック時などに送られるメッセージ
constexpr int WM_SHOWINIT = WM_APP + 2;     // 「起動時に画面を開く」に関する処理用メッセージ

#define MENU_ICON 2000

// 子ウィンドウ識別子
enum {
    ID_SOURCE_FOLDER = 3000,
    ID_DESTINATION_FOLDER,
    ID_WAIT_TIME,

    ID_EXTENSION0,
    ID_EXTENSION1,
    ID_EXTENSION2,
    ID_EXTENSION3,
    ID_EXTENSION4,

    ID_WINDOW_LAUNCH,
    ID_NOTIFICATION,

    ID_HELP_BUTTON,
    ID_APPLY_BUTTON,
    ID_STOP_BUTTON,
    ID_CLOSE_BUTTON,

    ID_HISTORY,
};