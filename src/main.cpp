//=============================================================================
// 作業ファイルを自動バックアップ
//		Windows 10 64bit で動作チェック
// 
//		指定フォルダを監視し、
//		変更があったファイルをバックアップする
//=============================================================================

#include <Windows.h>
#include "../include/main.h"
#include "../include/Constants.h"
#include "../include/ApplicationMutex.h"
#include "../include/WindowProc.h"

// 新UIデザインを使用
#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


// エントリポイント
int WINAPI WinMain(
    _In_        HINSTANCE   hInstance,
    _In_opt_    HINSTANCE   hPrevInstance,
    _In_        LPSTR       lpCmdLine,
    _In_        int         nCmdShow
){
    // 多重起動防止
    ApplicationMutex mutex(AppInfo::NAME);
    if (mutex.isAlreadyStarted()) {
        MessageBoxW(nullptr, L"既に起動しています。", AppInfo::NAME, MB_OK);
        return 0;
    }

    //-----------------------------------------------------
    // ウィンドウクラスを登録
    //-----------------------------------------------------

    // 背景用 カラーブラシ
    //(HBRUSH)(COLOR_MENU + 1);
    HBRUSH bgBrush = CreateSolidBrush(RGB(240, 240, 240));

    // GIMPで作成したマルチアイコン 
    const HICON hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(MENU_ICON));

    // ウィンドウクラスの属性を設定
    const WNDCLASS wc = {
        CS_HREDRAW | CS_VREDRAW,        // スタイル
        WindowProc::proc,               // ウィンドウプロシージャ
        0,                              // 追加領域
        0,                              // 追加領域
        hInstance,                      // インスタンスハンドル
        hIcon,                          // アイコン
        LoadCursorW(NULL, IDC_ARROW),   // カーソル
        bgBrush,                        // 背景ブラシ
        NULL,                           // メニュー名
        AppInfo::NAME                   // クラス名
    };

    // ウィンドウクラスを登録
    if (!RegisterClassW(&wc)) return 0;

    //-----------------------------------------------------
    // ウィンドウを生成、表示
    //-----------------------------------------------------

    // メインウィンドウ位置
    const int
        displayX = GetSystemMetrics(SM_CXSCREEN) - WindowSize::W - 100,
        displayY = GetSystemMetrics(SM_CYSCREEN) - WindowSize::H - 150;

    // ウィンドウを作成
    const DWORD style = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX;   // 枠可変、最大化 を解除
    const HWND hwnd = CreateWindowW(
        AppInfo::NAME,          // ウィンドウクラス名
        AppInfo::NAME_JP,       // ウィンドウの名称
        style,                  // ウィンドウスタイル
        displayX, displayY,     // 横, 縦の位置  CW_USEDEFAULT
        WindowSize::W,          // ウィンドウの幅, 高さ
        WindowSize::H,
        NULL,                   // 親ウィンドウのハンドル
        NULL,                   // メニューハンドル
        hInstance,              // インスタンスハンドル
        NULL                    // ウィンドウ作成データ
    );

    if (!hwnd) return 0;

    // ウィンドウを表示 (メッセージを送信し、プロシージャ内で処理)
    SendMessageW(hwnd, WM_SHOWINIT, NULL, nCmdShow);
    UpdateWindow(hwnd);

    //-----------------------------------------------------

    // メッセージループ
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) { // メッセージを取出
        TranslateMessage(&msg);                 // 文字化
        DispatchMessageW(&msg);                 // プロシージャへ発信
    }

    // クリーンアップ
    DestroyWindow(hwnd);
    return (int)msg.wParam;
}
