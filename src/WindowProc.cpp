#include "../include/WindowProc.h"
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

//=============================================================================
// ウィンドウプロシージャ関連をまとめたクラス (Controller)
// プロシージャ内部でインスタンス化される
//=============================================================================

// アプリのデフォルト設定
// コンフィグファイルが読み込まれなかった場合に使用される
constexpr Config defaultConfig = {
    L"D:\\作業フォルダ",	            // コピー元
    L"E:\\バックアップフォルダ",      // コピー先
    0,                              // ファイル変更から、バックアップまでの時間 (秒)
    true,						    // 起動時に画面を開くか
    true,						    // 通知をするか
    { L".txt" }	                    // 拡張子リスト
};


// コンストラクタ (proc内で実行される)
WindowProc::WindowProc() :
    config {defaultConfig},
    font(),
    ui(),
    trayIcon (AppInfo::NAME_JP, ID_TRAYICON, WM_TASKTRAY),
    backup (trayIcon, ui.history)
{}


// ウィンドウプロシージャ (WNDCLASS構造体のlpfnWndProcメンバ)
LRESULT CALLBACK WindowProc::proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    // 自クラスをインスタンス化
    static WindowProc self;

    switch (uMsg) {

    // タスクアイコン、タスクバー、最小化 の処理
    case WM_SIZE    :
    case WM_TASKTRAY:
    case WM_SHOWINIT:
        if (self.show(hWnd, uMsg, wParam, lParam)) return 0;
        break;

    // ボタンが押された時
    case WM_COMMAND:
        if (self.wmCommand(hWnd, wParam)) return 0;
        break;
        
    // 描画時
    case WM_PAINT:
        self.ui.paintAll(hWnd, self.font.handle);
        return 0;

    // 作成時
    case WM_CREATE:
        self.wmCreate(hWnd, lParam);
        return 0;

    // アプリの終了
    case WM_CLOSE:{

        // 確認ダイアログ  
        const int result = MsgBox(
            L"終了しますか？", L"アプリケーションの終了",
            MB_YESNO | MB_ICONQUESTION
        );

        // WM_DESTROY を発行
        if(result == IDYES) DestroyWindow(hWnd);
        return 0;
    }

    // ウィンドウ破棄
    case WM_DESTROY:{

        // バックアップ停止、トレイアイコン削除、WM_QUITを送信
        self.backup.closeThread();
        self.trayIcon.deleteIcon();
        PostQuitMessage(0);
        return 0;
    }
    } // switch

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}


// タスクアイコン、タスクバー、最小化 関連の処理
// @returns 処理が行われたか否か
bool WindowProc::show(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const {
    switch (uMsg) {

    // ウィンドウサイズ 変更時
    case WM_SIZE: {
        const int id = LOWORD(wParam);

        // 最小化時 → タスクバーから非表示
        if (id == SIZE_MINIMIZED) {
            ShowWindow(hWnd, SW_HIDE);
            return true;
        }
        break;
    }

    // トレイアイコン
    case WM_TASKTRAY: {

        // 左クリック時 → 最小化/通常 を切替え
        if (lParam == WM_LBUTTONDOWN) {
            const bool isMinimized = IsIconic(hWnd);

            // 最小化時 → 通常
            if (isMinimized) {
                ShowWindow(hWnd, SW_SHOW);          // タスクバーに表示
                ShowWindow(hWnd, SW_SHOWNORMAL);    // ウィンドウの表示
            }

            // 通常時 → 最小化
            else ShowWindow(hWnd, SW_SHOWMINIMIZED);  // → WM_SIZE へ
            return true;
        }
        break;
    }

    // 「起動時に画面を開く」に関する処理
    // config を WinMain() から参照できない為、こちら側で行う
    case WM_SHOWINIT: {
        const int nCmdShow = (int)lParam;

        // 通常表示 or 最小化
        (config.windowLaunch)
            ? ShowWindow(hWnd, nCmdShow)
            : ShowWindow(hWnd, SW_SHOWMINNOACTIVE);

        return true;
    }
    } // switch
    return false;
}


// ボタンが押されたとき の処理
// @returns 処理が行われたか否か
bool WindowProc::wmCommand(HWND hWnd, WPARAM wParam) {

    const UINT id = LOWORD(wParam);
    switch (id) {

    // バージョン ボタン
    case ID_HELP_BUTTON: {
        using namespace AppInfo;

        // アプリ情報を文字列化
        WCHAR text[128];
        swprintf_s(
            text, L"%s\n%s%s\n%s",
            NAME_JP, L"バージョン : ", VERSION, COPYRIGHT
        );

        // メッセージボックスで表示
        MsgBox(text, L"バージョン情報", MB_OK);
        return true;
    }

    // 設定適用 ボタン
    case ID_APPLY_BUTTON :
        applyAndSart();
        return true;

    // 停止 ボタン
    case ID_STOP_BUTTON  : 
        stop();
        return true;

    // 終了 ボタン
    case ID_CLOSE_BUTTON: {
        SendMessageW(hWnd, WM_CLOSE, 0, 0);
        return true;
    }
    } // switch
    return false;
}


// 設定適用・開始 ボタン
void WindowProc::applyAndSart() {

    // ボタンを無効化、スレッドを停止
    ui.applyButton.enable(false);
    ui.stopButton.enable(false);
    backup.closeThread();

    // UI値を取得 → コンフィグに格納
    getUiValues();

    // バックアップスレッド起動
    const WCHAR* reEnterInfo = backup.createThread(config);

    // コンフィグ値が無効 → 再入力させる
    if (reEnterInfo) {
        MsgBox(reEnterInfo, L"再入力", MB_OK);
        ui.applyButton.enable(true);
        return;
    }

    // コンフィグをファイルに保存
    config.saveFile();

    // ボタンを有効化
    ui.applyButton.enable(true);
    ui.stopButton.enable(true);
}


// 停止 ボタン
void WindowProc::stop() {
    // 停止ボタンを無効化、バックアップスレッドを停止
    ui.stopButton.enable(false);
    backup.closeThread(true);
}


// UI値を取得、configに格納
void WindowProc::getUiValues() {

    // フォルダパス
    ui.sourceFolder     .get(config.sourceFolder     , MAX_PATH);
    ui.destinationFolder.get(config.destinationFolder, MAX_PATH);

    // バックアップまでの待機時間 (文字列 → 数値)
    config.waitTime = ui.waitTime.get();

    //-----------------------------------------------------
    // 拡張子 (リストの上から隙間なく埋める)
    //-----------------------------------------------------

    // 拡張子の最大文字数、拡張子型
    const int length = Config::EXTENSION_LENGTH;
    using Extension = Config::Extension;

    // コンフィグを初期化
    for (Extension& extension : config.extensions)
        extension[0] = L'\0';

    // UI値を取得 & 整理 (UI → コンフィグ)
    int configIndex = 0;
    for (EditBox& uiExtension : ui.extensions) {

        // UIから拡張子を取得
        Extension tmp;
        uiExtension.get(tmp, length);

        // 拡張子が未設定 → スキップ
        const WCHAR uiHead = tmp[0];
        if (uiHead == L'\0') continue;

        // コンフィグを取得 (書込先)
        Extension& configExtension = config.extensions[configIndex];
        configIndex++;

        // 先頭に.を書込む
        if (uiHead != L'.') {
            configExtension[0] = L'.';
            configExtension[1] = L'\0';
        }

        // その後ろに文字列を追加
        wcscat_s(configExtension, length, tmp);
    }

    // UIに整理した値を反映 (コンフィグ → UI)
    for (int i = 0; i < Config::EXTENSIONS_LENGTH; i++)
        ui.extensions[i].set( config.extensions[i] );

    //-----------------------------------------------------

    // チェックボックス
    config.windowLaunch = ui.windowLaunch.get();
    config.notification = ui.notification.get();

    // トレイアイコンに設定を反映
    trayIcon.enable = config.notification;
}


// 子ウインドウ作成
void WindowProc::wmCreate(HWND hWnd, LPARAM lParam) {

    const HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
    const HICON hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(MENU_ICON));

    // メッセージボックス 初期化
    MessageBoxCenter::hParent = hWnd;

    // コンフィグファイルを読込
    const bool configLoadFlag = config.loadFile();

    // 子ウィンドウを全て描画
    const CreateWindowArgs args = { hWnd, hInstance, font.handle };
    ui.createAll(args);

    // バックアップ開始前なので、停止ボタンを無効化
    ui.stopButton.enable(false);

    const WCHAR* initialHistory = 
        L"各項目に入力し、[適用・開始] ボタンを押してください。";

    // コンフィグ値を子ウィンドウに反映 
    // set()にはハンドルが必要なため、create() 後に実行
    ui.sourceFolder     .set(config.sourceFolder);
    ui.destinationFolder.set(config.destinationFolder);
    ui.waitTime         .set(config.waitTime);
    ui.windowLaunch     .set(config.windowLaunch);
    ui.notification     .set(config.notification);
    ui.history          .set(initialHistory);

    for (int64_t i = 0; i < Config::EXTENSIONS_LENGTH; i++)
        ui.extensions[i].set(config.extensions[i]);

    // アイコンをトレイに追加
    trayIcon.setHandle(hWnd, hIcon);
    trayIcon.add();
    trayIcon.enable = config.notification;
  
    // コンフィグ読込済み → バックアップ開始
    if (configLoadFlag) applyAndSart();
}
