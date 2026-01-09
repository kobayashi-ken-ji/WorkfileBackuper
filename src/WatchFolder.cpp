#include "../include/WatchFolder.h"

//=============================================================================
// フォルダ監視処理をまとめたクラス
// ハンドルのライフサイクルを管理
//=============================================================================

/**
* コンストラクタ / フォルダ監視を開始
* @param pathName       監視するフォルダのパス
* @param notifyFilter   通知条件 (FILE_NOTIFY_CHANGE~)
* @param hStopSignal    停止用イベント (シグナル化することで待機が終了)
*/
WatchFolder::WatchFolder(LPCWSTR pathName, DWORD notifyFilter, HANDLE hStopSignal) :
    handle(FindFirstChangeNotificationW(
        pathName,       // フォルダ
        false,          // サブディレクトリを監視するか
        notifyFilter    // 通知条件
    )),
    handles{ handle, hStopSignal }
{}


// デストラクタ / ハンドルを解放
WatchFolder::~WatchFolder() {
    FindCloseChangeNotification(handle);
}


// ハンドルが無効か否か (コンストラクタ直後に使用)
bool WatchFolder::isInvalidHandle() const {
    return (handle == INVALID_HANDLE_VALUE);
}


// フォルダ監視の終了 or 停止フラグのシグナル化 を待機
// @param milliSeconds タイムアウトまでのミリ秒
// @returns WatchFolderWaitResultのいずれか
DWORD WatchFolder::waitChangeOrStopsignal(DWORD milliSeconds) const {
    return WaitForMultipleObjects(
        2,              // ハンドル数
        handles,        // ハンドルの配列
        false,          // 全てのハンドルが終了するまで待つか
        milliSeconds    // タイムアウト間隔 (ミリ秒)
    );
}


// フォルダ監視を再開
bool WatchFolder::reStart() const {
    return FindNextChangeNotification(handle);
}
