#pragma once

#include "stdlib.h"
#include <stdio.h>
#include <iostream>
#include <filesystem>
#include <Windows.h>
#include <chrono>
#include <unordered_map>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#include "ChildWindow.h"
#include "Config.h"
#include "TrayIcon.h"
#include "Config.h"
#include "ThreadStopFlag.h"
#include <filesystem>

using namespace std;
using namespace std::filesystem;	// C++17以


// 再入力が必要な場合のメッセージ
namespace ReEnterInfo {
    PCWSTR NO_SOURCE      = L"バックアップ元フォルダが有効ではありません。";
    PCWSTR NO_DESTINATION = L"バックアップ先フォルダが有効ではありません。";
    PCWSTR ZERO_TIME      = L"バックアップする間隔を1秒以上にしてください。";
    PCWSTR NO_EXTENSION   = L"拡張子が入力されていません。";
}

// 通常のメッセージ
namespace Info {
    PCWSTR START  = L"バックアップを開始しました。";
    PCWSTR CLOSE  = L"バックアップを停止しました。";
    PCWSTR BACKUP = L"[バックアップ]";
    PCWSTR SKIP   = L"[スキップ] 既にバックアップ済み";

    PCWSTR COPY_ERR =
        L"[エラー] ファイルをコピーできませんでした。\r\n"
        L"以下の可能性があります。\r\n"
        L"  ・コピー先フォルダが存在しない\r\n"
        L"  ・空き容量が足りない\r\n"
        L"  ・ファイル名(フルパス)が長い";
};

// バックアップ処理中断時のメッセージ
namespace AbortInfo {
    PCWSTR SOURCE_ERR = L"[エラー] バックアップ元フォルダが存在しない可能性があります。";
    PCWSTR L_TIME_ERR = L"[エラー] タイムスタンプをローカル時間に変換できませんでした。";
    PCWSTR WATCH_WAIT_ERR  = L"[エラー] フォルダ監視の待機に失敗しました。";
    PCWSTR WATCH_START_ERR = L"[エラー] フォルダ監視の開始に失敗しました。";
}

//=============================================================================
// バックアップ処理全体 [クラス]
//=============================================================================
/*
    全体の流れ
        指定時間 待機
        フォルダ自体の最終更新時を確認
        階層内をチェック → 対象ファイルを記録
        前回の記録と比較 → 新規 or 時間更新 ならバックアップ
*/

class Backup {
private:

    // スレッド管理用
    HANDLE threadHandle;
    DWORD  threadID;
    ThreadStopFlag stopFlag;

    // UIへの参照 (View)
    const EditBox& historyUI;
    TrayIcon& trayIcon;

    // バックアップ処理用の設定
    ConfigStruct config;

    // ファイルリスト 2つ
    //		フォルダチェックごとに [前回のリスト, 今回のリスト] を入替え、
    //      新しく[今回] になった方へ記録していく
    using FileMap = unordered_map <path, file_time_type>;
    FileMap  fileMap1;
    FileMap  fileMap2;
    FileMap* fileMapNew = &fileMap1;
    FileMap* fileMapOld = &fileMap2;

public:

    // コンストラクタ
    // ※ 参照型の引数は、このインスタンスよりも長寿命である必要がある
    Backup(TrayIcon& trayIcon, const EditBox& historyUI) :
        trayIcon(trayIcon), historyUI(historyUI), config{},
        threadHandle(nullptr), threadID(0) {}

    // デストラクタで確実にスレッドを停止
    ~Backup() { closeThread(); }


    // スレッドを作成する (バックアップ処理を開始する)
    // @returns コンフィグ値が不適切な場合のメッセージ、正常時はnullptr
    const WCHAR* createThread(const ConfigStruct &config) {
        
        // 整合性をチェック
        // メッセージボックスの使用をメインウィンドウに任せるため、情報を返す
        if (!exists(config.sourceFolder))      return ReEnterInfo::NO_SOURCE;
        if (!exists(config.destinationFolder)) return ReEnterInfo::NO_DESTINATION;
        if (config.checkInterval < 1)          return ReEnterInfo::ZERO_TIME;
        if (config.extensionCount == 0)        return ReEnterInfo::NO_EXTENSION;

        // 設定を複製 (この時点の値で固定)
        this->config = config;

        // スレッドを安全に停止させるためのイベントハンドル
        stopFlag.init();

        // this を lpParameter に渡す → スレッド関数(静的)で受取り
        threadHandle = CreateThread(
            NULL,	            // セキュリティ属性構造体へのポインタ	9x系はNULL
            0, 		            // 新スレッドのスタックサイズ 		0でデフォルト
            thredFunc,          // 新スレッドの実行開始アドレス	関数型へのポインタ
            (LPVOID)this,       // スレッドに渡す追加情報
            0,	                // スレッドの状態	    0 実行可能, CREATE_SUSPENDED 待機
            &threadID	        // スレッド識別子を受取用ポインタ	失敗はNULLが入る
        );

        // 起動通知
        updateHistory(Info::START);
        trayIcon.notify(Info::START);
        return nullptr;
    }
        

    // スレッドを終了
    // @returns ハンドル開放の成否
    bool closeThread(bool notificationFlag = false) {

        // スレッドが停止中 → return
        DWORD result = 0;
        GetExitCodeThread(threadHandle, &result);
        if (result != STILL_ACTIVE) return true;

        // 停止フラグを立てて、止まるまで待機
        stopFlag.stop();
        WaitForSingleObject(threadHandle, INFINITE);

        // 履歴更新、通知
        updateHistory(Info::CLOSE);
        if (notificationFlag)
            trayIcon.notify(Info::CLOSE);

        return CloseHandle(threadHandle);
    }

private:

    // スレッド関数 [静的]
    static DWORD WINAPI thredFunc(LPVOID lpParamete) {

        // 自クラスのインスタンスを受取る
        Backup* self = (Backup*)lpParamete;   

        // バックアップ方法で分岐
        PCWSTR abortInfo = (self->config.interval)
            ? self->intervalFunc()
            : self->immediateFunc();

        // 処理途中で終了した場合、情報を出力
        if (abortInfo) {
            self->updateHistory(Info::CLOSE, abortInfo);
            self->trayIcon.notify(Info::CLOSE, abortInfo);
        }

        return 0;
    }


    // [廃止予定]
    // 一定間隔で上書きをチェック (ポーリング)
    PCWSTR intervalFunc() {

        // 現在の最終更新時を取得
        checkFolderUpdate();

        // 開始時のファイルの一覧を作成
        getFileList();

        // タイムアウトするまでの間隔 (ミリ秒)
        const int interval = config.checkInterval * 1000;

        while (true) {

            // 待機 (タイムアウト or stopFlag.stop() がされるまで)
            const DWORD result = WaitForSingleObject(stopFlag.handle, interval);
            switch (result) {
                case WAIT_OBJECT_0  : return 0;             // 停止ボタンが押された
                case WAIT_TIMEOUT   : break;                // タイムアウト → 次の処理へ
                default: return 0;  // 失敗 → メッセージ表示
            }

            // フォルダの更新時をチェック
            if (!checkFolderUpdate()) continue;	

            // ファイル一覧を取得
            PCWSTR errInfo = getFileList();
            if (errInfo) return errInfo;

            // バックアップ処理
            fileBackup();
        }
        return 0;
    }

    
    // 上書きされた直後にバックアップ
    PCWSTR immediateFunc() {

        // 開始時のファイルの一覧を作成
        getFileList();

        // 通知条件
        constexpr DWORD notifyFilter =
            FILE_NOTIFY_CHANGE_FILE_NAME |  // ファイル名の変更、作成、削除
            FILE_NOTIFY_CHANGE_LAST_WRITE;  // 最終書き込み日時の変更

        // フォルダ内の監視 を開始
        HANDLE watch = FindFirstChangeNotificationW(
            config.sourceFolder,            // フォルダ
            false,                          // サブディレクトリを監視するか
            notifyFilter                    // 通知条件
        );

        // 開始に失敗
        if (watch == INVALID_HANDLE_VALUE)
            return AbortInfo::SOURCE_ERR;

        // 監視のハンドルを解放 する関数
        // return の前に必ず実行する
        auto close = [=] () {
            FindCloseChangeNotification(watch);
        };

        // 処理待ちをする要素 [フォルダ監視、スレッド停止フラグ監視]
        const HANDLE hEvents[] = { watch, stopFlag.handle };
        
        // 監視ループ
        while (true) {

            // 待ち処理 (フォルダ内変更 or 停止ボタンが押される)
            DWORD result = WaitForMultipleObjects(
                2,          // ハンドル数
                hEvents,    // ハンドルの配列
                false,      // 全てのハンドルが終了するまで待つか
                INFINITE    // タイムアウト間隔 (ミリ秒)
            );

            // 結果判定
            switch (result) {

                // フォルダ内に変更あり → 次の処理へ
                case WAIT_OBJECT_0 + 0:
                    break;

                // 停止ボタンが押された → return
                case WAIT_OBJECT_0 + 1: 
                    close();
                    return nullptr;

                // 待ち処理に失敗
                default:
                    close();
                    return AbortInfo::WATCH_WAIT_ERR;
            }

            // 変更通知が途切れるまでチェックを続ける
            while (true) {

                // 監視を開始
                if (!FindNextChangeNotification(watch)) {
                    close();
                    return AbortInfo::WATCH_START_ERR;
                }

                // 監視終了を待つ
                DWORD result = WaitForSingleObject(watch, 2000);

                // 結果判定
                if (result == WAIT_TIMEOUT) break;      // タイムアウト → 次の処理
                if (result == WAIT_OBJECT_0) continue;  // 変更あり → 再チェック

                // それ以外 → 待ち処理に失敗
                close();
                return AbortInfo::WATCH_WAIT_ERR;
            }

            // ファイル一覧を取得
            PCWSTR errInfo = getFileList();
            if (errInfo) {
                close();
                return errInfo;
            }

            // バックアップ処理
            fileBackup();

            // 次の監視を開始
            if (!FindNextChangeNotification(watch)) {
                close();
                return AbortInfo::WATCH_START_ERR;
            }
        }
        return 0;
    }


    // 履歴UIの内容を更新
    // 表示形式
    //      No.○○
    //      message1
    //      message2
    void updateHistory(const WCHAR* message1, const WCHAR* message2 = L"\0") const {

        // 通知数をカウント
        static UINT count = 0;
        count++;

        // 文字列を作成
        constexpr int LENGTH = 512;
        WCHAR text[LENGTH] = L"";
        const WCHAR* format = L"No.%d\r\n%s\r\n%s";
        swprintf_s(text, LENGTH, format, count, message1, message2);

        // UIに表示
        historyUI.set(text);
    }


    // [廃止予定]
    // フォルダの更新日時をチェック （処理軽量化）
    //      フォルダ更新時が変わるのは  直下ファイルが [名前変更、作成、削除] された時
    //      → 単純上書きの場合は 更新時は変更されないため注意
    bool checkFolderUpdate() {

        if (!config.folderCheck) return true;

        static file_time_type folderTime;	    // チェックするフォルダの最終更新時
        error_code err;
        file_time_type timeNew = last_write_time(config.sourceFolder, err);

        // フォルダー取得エラー を表示
        if (err) {
            updateHistory(AbortInfo::SOURCE_ERR, config.sourceFolder);
            trayIcon.notify(AbortInfo::SOURCE_ERR);
            return false;
        }

        // 更新されていれば時間上書き
        bool isUpdated = (folderTime != timeNew);
        if (isUpdated) folderTime = timeNew;
        return isUpdated;
    }


    // フォルダ内のファイル一覧を取得
    // @param エラー情報 (正常終了時はnullptr)
    PCWSTR getFileList() {

        // ディレクトリを取得 (イテレータ)
        error_code err;
        directory_iterator directory(config.sourceFolder, err);
        if (err) return AbortInfo::SOURCE_ERR;

        // ファイル一覧 新旧入替
        FileMap* fileMapTmp = fileMapNew;
        fileMapNew = fileMapOld;
        fileMapOld = fileMapTmp;

        // 新マップをクリア
        (*fileMapNew).clear();

        // ディレクトリ内を走査
        for (const auto& entry : directory) {

            // ファイルパス、拡張子を取得
            const path filePath = entry.path();
            path fileExtension = filePath.extension();

            // フォルダの場合 → スキップ
            if (fileExtension == L"") continue;


            // コンフィグ内の拡張子を走査
            for (const auto& configExtension : config.extensions) {

                // 拡張子が未登録 (= 番兵)
                if (configExtension[0] == L'\0') break;

                // 拡張子がマッチ → マップに追加
                if (configExtension == fileExtension) {
                    (*fileMapNew)[filePath] = last_write_time(filePath);
                    break;
                }
            }
        }
        return nullptr;
    }


    // ファイルの新旧を比較・バックアップ処理
    //		戻り値 エラーの真偽
    bool fileBackup() {
        using namespace std::chrono;

        // 新mapの要素 を取出す
        for (const auto& [filePath, lastTime] : (*fileMapNew)) {

            // 新旧比較
            // 同じファイル → バックアップしない
            if ((*fileMapOld).count(filePath) &&        // 同名ファイル
                (*fileMapOld)[filePath] == lastTime)    // 最終更新時が同じ
                continue;

            //----------------------------------
            // 最終更新時を文字列化
            //-----------------------------------

            // Windows(1601~) → UNIX(1970~) へ変換  [C++20]
            auto timeUnix = clock_cast<system_clock>(lastTime);

            // 秒に変換
            seconds	sec = duration_cast<seconds>(timeUnix.time_since_epoch());
            time_t	t = sec.count();

            // ローカル時間に変換
            tm timeStruct;
            errno_t timeErr = localtime_s(&timeStruct, &t);

            // 変換失敗通知
            if (timeErr) {
                const WCHAR* path = filePath.c_str();
                updateHistory(AbortInfo::L_TIME_ERR, path);
                trayIcon.notify(path, AbortInfo::L_TIME_ERR);
                continue;
            }

            // タイムスタンプを文字列化 [yyyymmdd_hhmmss]
            char timeText[32] = "";
            strftime(
                timeText,			// 格納先
                sizeof(timeText),	// 格納先サイズ
                "[%Y%m%d_%H%M%S]",	// フォーマット
                &timeStruct			// tm構造体
            );

            //-----------------------------------
            // コピー先のファイルパスを生成
            //-----------------------------------

            // ファイル名  "ベース名[yyyymmdd_hhmmss].拡張子"
            path fileName = filePath.stem();
            string extension = filePath.extension().string();
            fileName.concat(timeText).concat(extension);

            // コピー先ディレクトリ + ファイル名
            path savePath = config.destinationFolder;
            savePath /= fileName;
            
            // string → WCHAR 変換
            const WCHAR* fileNameW = fileName.c_str();

            // 同名ファイルがある (バックアップ済み) → スキップ
            if (exists(savePath)) {
                updateHistory(Info::SKIP, fileNameW);
                trayIcon.notify(fileNameW, Info::SKIP);
                continue;
            }

            //-----------------------------------
            // ファイルをコピー
            //-----------------------------------

            // コピー
            error_code copyErr;
            copy(filePath, savePath, copyErr);

            // コピー失敗通知
            if (copyErr) {
                updateHistory(Info::COPY_ERR, fileNameW);
                trayIcon.notify(Info::COPY_ERR);
                continue;
            }

            // 保存ファイル名表示
            updateHistory(Info::BACKUP, fileNameW);
            trayIcon.notify(fileNameW, Info::BACKUP);
        }
        return true;
    }
};