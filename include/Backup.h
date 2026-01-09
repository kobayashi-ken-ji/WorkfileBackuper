#pragma once

#include <Windows.h>
#include <unordered_map>
#include <filesystem>   // C++17以降

#include "ThreadStopFlag.h"
#include "ChildWindow.h"
#include "Config.h"
#include "TrayIcon.h"


//  バックアップ処理の流れ
//      (1) フォルダを監視 / 変更通知まで待機
//      (2) フォルダ内をチェック / ファイル名・最終更新時を記録
//      (3) ファイルが「新規 or 時間更新」ならファイルをコピー
//      (4) 1へ戻る
// 
//  外部からの停止
//      stopFlag.stop() を実行すると
//      1の待機中に処理を抜け、スレッドを停止する
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

    // ファイルリストの型 <ファイルパス, 最終更新時>
    using FileMap = std::unordered_map <
        std::filesystem::path,
        std::filesystem::file_time_type
    >;

    // ファイルリスト 2つ
    //		フォルダチェックごとに [前回のリスト, 今回のリスト] を入替え、
    //      新しく[今回] になった方へ記録していく
    FileMap  fileMap1;
    FileMap  fileMap2;
    FileMap* pFileMapNew = &fileMap1;
    FileMap* pFileMapOld = &fileMap2;

public:

    // コンストラクタ
    // ※ 引数は、このインスタンスよりも長寿命である必要がある
    Backup(TrayIcon& trayIcon, const EditBox& historyUI);

    // デストラクタ
    ~Backup();

    // スレッドを作成する (バックアップ処理を開始する)
    // @returns コンフィグ値が不適切な場合のメッセージ、正常時はnullptr
    const WCHAR* createThread(const ConfigStruct& config);

    // スレッドを終了
    // @param notificationFlag デスクトップ通知に「終了した」と表示するか否か
    // @returns ハンドル開放の成否
    bool closeThread(bool notificationFlag = false);

private:

    // スレッド関数 [静的]
    static DWORD WINAPI thredFunc(LPVOID lpParamete);
    
    // バックアップの一連の処理
    // @returns エラー情報 (正常終了時はnullptr)
    PCWSTR main();

    // 履歴UIの内容を更新
    // 表示形式
    //      No.○○
    //      message1
    //      message2
    void updateHistory(const WCHAR* message1, const WCHAR* message2 = L"\0") const;

    // フォルダ内のファイル一覧を取得
    // @returns エラー情報 (正常終了時はnullptr)
    PCWSTR getFileList();

    // ファイルの新旧を比較・バックアップ処理
    void fileBackup();
};