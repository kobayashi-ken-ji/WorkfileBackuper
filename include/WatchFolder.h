#pragma once
#include <Windows.h>

// WatchFolder::waitChangeOrStopsignal() の戻り値
namespace WatchFolderWaitResult {
    constexpr DWORD
        CHANGED   = WAIT_OBJECT_0 + 0,
        STOPED    = WAIT_OBJECT_0 + 1,
        TIMEOUT   = WAIT_TIMEOUT,
        FAILED    = WAIT_FAILED,
        ABANDONED = WAIT_ABANDONED;  // 解放されなかったミューテックスオブジェクト
}


// フォルダ監視処理をまとめたクラス
// ハンドルのライフサイクルを管理
class WatchFolder {
private:
    const HANDLE handle;        // フォルダ監視ハンドル
    const HANDLE handles[2];    // 待機する要素 [フォルダ監視、停止シグナル]

public:
    /**
    * コンストラクタ / フォルダ監視を開始
    * @param pathName       監視するフォルダ
    * @param notifyFilter   通知条件 (FILE_NOTIFY_CHANGE~)
    * @param hStopSignal    停止用イベント (シグナル化することで待機が終了)
    */
    WatchFolder(LPCWSTR pathName, DWORD notifyFilter, HANDLE hStopSignal);

    // デストラクタ / ハンドルを解放
    ~WatchFolder();

    // ハンドルが無効か否か (コンストラクタ直後に使用)
    bool isInvalidHandle() const;

    // フォルダ監視の終了 を待機
    DWORD waitChange(DWORD milliSeconds = INFINITE) const;

    // フォルダ監視の終了 or 停止信号 を待機
    DWORD waitChangeOrStopsignal(DWORD milliSeconds = INFINITE) const;

    // フォルダ監視を再開
    bool reStart() const;
};