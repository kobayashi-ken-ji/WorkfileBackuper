#pragma once
#include <Windows.h>

// WatchFolder::waitChangeOrStopsignal() の戻り値
namespace WatchFolderWaitResult {
    constexpr DWORD
        CHANGED   = WAIT_OBJECT_0 + 0,  // フォルダ内に変更があった
        STOPED    = WAIT_OBJECT_0 + 1,  // 停止フラグが立った
        TIMEOUT   = WAIT_TIMEOUT,       // なにも起きず、タイムアウト
        FAILED    = WAIT_FAILED,        // 待ち処理に失敗
        ABANDONED = WAIT_ABANDONED;     // 放棄されたミューテックスオブジェクト
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
    * @param pathName       監視するフォルダのパス
    * @param notifyFilter   通知条件 (FILE_NOTIFY_CHANGE~)
    * @param hStopSignal    停止用フラグ (シグナル化することで待機が終了)
    */
    WatchFolder(LPCWSTR pathName, DWORD notifyFilter, HANDLE hStopSignal);

    // デストラクタ / ハンドルを解放
    ~WatchFolder();

    // ハンドルが無効か否か (コンストラクタ直後に使用)
    bool isInvalidHandle() const;

    // フォルダ監視の終了 or 停止フラグのシグナル化 を待機
    // @param milliSeconds タイムアウトまでのミリ秒
    // @returns WatchFolderWaitResultのいずれか
    DWORD waitChangeOrStopsignal(DWORD milliSeconds = INFINITE) const;

    // フォルダ監視を再開
    bool reStart() const;
};