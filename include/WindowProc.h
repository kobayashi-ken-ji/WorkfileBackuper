#pragma once

#include "stdlib.h"
#include <stdio.h>
#include <Windows.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#include "Constants.h"
//#include "MenuID.h"

// view
#include "MessageBoxCenter.h"
#include "DeviceContext.h"
#include "ChildWindow.h"
#include "UiElements.h"
#include "TrayIcon.h"
#include "MainFont.h"

// model
#include "Config.h"
#include "Backup.h"

//=============================================================================
// ウインドウプロシージャ
//=============================================================================

namespace Text {
    const WCHAR* NOTES_TITLE = L"負荷を軽減";
    const WCHAR* NOTES =
        L"【 注意事項 】\r\n"
        L"    ○ 有効  ：  一時ファイルを作る上書き  (Photoshop など)\r\n"
        L"    × 無効  ：  単純な上書き  (メモ帳 など)\r\n"
        L"   バックアップされるか試してから、ご利用ください。\r\n\r\n"
        L"【 機能 】\r\n"
        L"    フォルダ日時が更新されている場合のみ、上書き確認を行います。\r\n"
        L"    フォルダ内をスキャン・比較する回数を減らすことができます。\r\n\r\n"
        L"【 Windowsの仕様 】\r\n"
        L"    フォルダの更新日時が変更されるのは\r\n"
        L"    フォルダ直下での 『作成・削除・名前変更』 をした場合のようです。\r\n"
        L"    単純な上書きでは変更されません。\r\n";
}


constexpr Config defaultConfig = {
    L"D:\\一時作業ファイル",	        // コピー元
    L"E:\\一時作業バックアップ",      // コピー先
    5,                              // フォルダをチェックする間隔 (秒)
    L"0",
    L"5",

    true,
    false,

    true,						    // 起動時に画面を開くか
    true,						    // 通知をするか
    false,                          // フォルダの更新日時をチェック（高速化）するか
    { L".txt" }	                    // 拡張子リスト
};


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


    // コンストラクタ (proc()内で実行される)
    WindowProc() :
        config {defaultConfig},
        font(),
        ui(),
        trayIcon (AppInfo::NAME_JP, ID_TRAYICON, WM_TASKTRAY),
        backup (trayIcon, ui.history)
    {}

public:

    // ウィンドウプロシージャ (WNDCLASS の lpfnWndProc に渡す)
    static LRESULT CALLBACK proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

        // 自クラスをインスタンス化
        static WindowProc self;

        switch (uMsg) {

        // タスクアイコン、タスクバー の処理
        case WM_SIZE    :
        case WM_TASKTRAY:
        case WM_SHOWINIT:
            if (self.show(hwnd, uMsg, wParam, lParam)) return 0;
            break;

        // ボタンが押された時
        case WM_COMMAND:
            if (self.wmCommand(hwnd, wParam)) return 0;
            break;
        
        // 描画時
        case WM_PAINT:
            self.ui.paintAll(hwnd, self.font.handle);
            return 0;

        // 作成時
        case WM_CREATE:
            self.wm_create(hwnd, lParam);
            return 0;

        // アプリの終了
        case WM_CLOSE:{

            // 確認ダイアログ  
            const int result = MsgBox(
                L"終了しますか？", L"アプリケーションの終了",
                MB_YESNO | MB_ICONQUESTION
            );

            // WM_DESTROY を発行
            if(result == IDYES) DestroyWindow(hwnd);
            return 0;
        }

        // ウィンドウ破棄
        case WM_DESTROY:{

            // バックアップ停止、トレイアイコン削除、WM_QUITをポスト
            self.backup.closeThread();
            self.trayIcon.deleteIcon();
            PostQuitMessage(0);
            return 0;
        }
        } // switch

        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

private:

    // タスクアイコン、タスクバー 関連の処理
    // @returns 処理が行われたか否か
    bool show(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const {
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
            (config.bootWindow)
                ? ShowWindow(hWnd, nCmdShow)
                : ShowWindow(hWnd, SW_SHOWMINNOACTIVE);

            return true;
        }
        } // switch
        return false;
    }


    // ボタンが押されたとき の処理
    // @returns 処理が行われたか否か
    bool wmCommand(HWND hWnd, WPARAM wParam) {

        const UINT id = LOWORD(wParam);
        switch (id) {

        // ラジオボタン に関連するウィンドウの 有効/無効 を切替え
        case ID_IMMEDIATE :
        case ID_INTERVAL  :
            ui.intervalGroupEnable(id == ID_INTERVAL);
            return true;

        // 注意事項 ボタン
        case ID_NOTES: {
            MsgBox(Text::NOTES, Text::NOTES_TITLE, MB_OK);
            return true;
        }

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
    void applyAndSart() {

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
    void stop() {
        // 停止ボタンを無効化、バックアップスレッドを停止
        ui.stopButton.enable(false);
        backup.closeThread(true);
    }


    // UI値を取得、configに格納
    void getUiValues() {

        // フォルダパス
        ui.sourceFolder     .get(config.sourceFolder     , MAX_PATH);
        ui.destinationFolder.get(config.destinationFolder, MAX_PATH);

        // タイミング
        config.immediate   = ui.immediate.get();
        config.interval    = ui.interval.get();
        config.folderCheck = ui.folderCheck.get();

        //-----------------------------------------------------
        // バックアップ間隔
        //-----------------------------------------------------

        // 文字列格納 → 数値変換
        const auto timeFunc = [](const EditBox &edit, WCHAR* buffer, int size, int maxTime = 9999999) {

            // ウィンドウ値を取得、数値化
            int time = 0;
            edit.get(buffer, size);
            time = _wtoi(buffer);

            // 上限反映
            if (time > maxTime) time = maxTime;                 

            // 文字で再格納、表示を更新
            _itow_s(time, buffer, Config::TIME_INPUT_LENGTH, 10);
            edit.set(buffer);
            return time;
        };

        // 分  秒
        int minutes = timeFunc(ui.intervalMin, config.intervalMin, Config::TIME_INPUT_LENGTH);
        int seconds = timeFunc(ui.intervalSec, config.intervalSec, Config::TIME_INPUT_LENGTH, 59);

        // 合計秒を格納
        config.checkInterval = (minutes * 60) + seconds;

        //-----------------------------------------------------
        // 拡張子 (リストの上から隙間なく埋める)
        //-----------------------------------------------------

        int extensionCount = 0;
        for (int i = 0; i < Config::EXTENSIONS_LENGTH; i++) {

            // ウィンドウから拡張子を取得
            ui.extensions[i].get( config.extensions[i], Config::EXTENSION_LENGTH);
            WCHAR* pExtension = config.extensions[i];   // 1つ分

            // 頭文字 取得
            WCHAR head = pExtension[0];
            if (head == L'\0') continue;

            // 書込先の取得
            bool sellMove = (extensionCount < i);           // セルを移動するか
            int writeIdx = (sellMove) ? extensionCount : i; // 書込先インデックス
            WCHAR
                * read  = config.extensions[i],             // 読込先
                * write = config.extensions[writeIdx];      // 書込先

            // 先頭に . がない場合は 付与
            WCHAR tmp[Config::EXTENSION_LENGTH] = L".";              // 新規 ドットのみ
            if (head != L'.') {
                wcscat_s(tmp, Config::EXTENSION_LENGTH, pExtension);   // ↑に文字を追加
                read = tmp;                                     // 読込先にする
            }

            // 文字列を戻す
            if(read != write)
                wcscpy_s(write, Config::EXTENSION_LENGTH, read);

            if (sellMove) pExtension[0] = L'\0';     // 空いた部分の処理
            extensionCount++;                           // 登録数カウントアップ
        }
        config.extensionCount = extensionCount;         // 拡張子の有効登録数

        // 表示更新
        for (int i = 0; i < Config::EXTENSIONS_LENGTH; i++)
            ui.extensions[i].set( config.extensions[i] );

        //-----------------------------------------------------

        // チェックボックス
        config.bootWindow = ui.bootWindow.get();
        config.notify     = ui.notify.get();

        // トレイアイコンに設定を反映
        trayIcon.enable = config.notify;
    }


    // 子ウインドウ作成
    void wm_create(HWND hwnd, LPARAM lParam) {

        const HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
        const HICON hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(MENU_ICON));

        // メッセージボックス 初期化
        MessageBoxCenter::hParent = hwnd;

        // コンフィグファイルを読込
        const bool configLoadFlag = config.loadFile();

        // 子ウィンドウを全て描画
        const CreateWindowArgs args = { hwnd, hInstance, font.handle };
        ui.createAll(args);

        const WCHAR* initialHistory = 
            L"各項目に入力し、[適用・開始] ボタンを押してください。";

        // コンフィグ値を子ウィンドウに反映 
        // set()にはハンドルが必要なため、create() 後に実行
        ui.sourceFolder     .set(config.sourceFolder);
        ui.destinationFolder.set(config.destinationFolder);
        ui.immediate        .set(config.immediate);
        ui.interval         .set(config.interval);
        ui.intervalMin      .set(config.intervalMin);
        ui.intervalSec      .set(config.intervalSec);
        ui.folderCheck      .set(config.folderCheck);
        ui.bootWindow       .set(config.bootWindow);
        ui.notify           .set(config.notify);
        ui.history          .set(initialHistory);

        for (int64_t i = 0; i < Config::EXTENSIONS_LENGTH; i++)
            ui.extensions[i].set(config.extensions[i]);

        // ラジオボタン関連の 有効/無効 切り替え
        ui.intervalGroupEnable(config.interval == true);

        // アイコンをトレイに追加
        trayIcon.setHandle(hwnd, hIcon);
        trayIcon.add();
        trayIcon.enable = config.notify;
  
        // 設定 読込済み → バックアップ開始
        if (configLoadFlag) applyAndSart();
        else ui.stopButton.enable(false);   // 停止ボタン 無効
    }
};