#pragma once
#include <Windows.h>
#include "Constants.h"

// View
#include "MessageBoxCenter.h"
#include "DeviceContext.h"
#include "ChildWindow.h"
#include "UiElements.h"
#include "TrayIcon.h"
#include "MainFont.h"

// Model
#include "Config.h"
#include "Backup.h"


// ウィンドウプロシージャ関連をまとめたクラス (Controller)
// プロシージャ内部でインスタンス化される
class WindowProc {
private:
    // Model
    Config      config;
    Backup      backup;

    // View
    MainFont    font;
    UiElements  ui;
    TrayIcon    trayIcon;

    // コンストラクタ (proc内で実行される)
    WindowProc();

public:
    // ウィンドウプロシージャ (WNDCLASS構造体のlpfnWndProcメンバ)
    static LRESULT CALLBACK proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    // タスクアイコン、タスクバー、最小化 関連の処理
    // @returns 処理が行われたか否か
    bool show(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const;

    // ボタンが押されたとき の処理
    // @returns 処理が行われたか否か
    bool wmCommand(HWND hWnd, WPARAM wParam);

    // 設定適用・開始 ボタン
    void applyAndSart();

    // 停止 ボタン
    void stop();

    // UI値を取得、configに格納
    void getUiValues();

    // 子ウインドウ作成
    void wmCreate(HWND hWnd, LPARAM lParam);
};