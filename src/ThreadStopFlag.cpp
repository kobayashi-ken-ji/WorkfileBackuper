#include "../include/ThreadStopFlag.h"
#include <synchapi.h>
#include <handleapi.h>

//=============================================================================
// スレッド停止用のフラグ
//=============================================================================

// コンストラクタ (イベントを生成)
ThreadStopFlag::ThreadStopFlag()
    : handle{ CreateEventW(
        NULL,   // セキュリティ属性なし
        TRUE,   // 手動リセット
        FALSE,  // 初期状態は非シグナル
        NULL    // イベント名なし
    ) } {}

// デストラクタ (イベントを解放)
ThreadStopFlag::~ThreadStopFlag() { CloseHandle(handle); }


// 停止フラグを立てる (シグナル化)
// WaitForSingleObject側は待機が終了する
void ThreadStopFlag::stop() const { SetEvent(handle); }


// 停止フラグを初期化 (非シグナル化)
void ThreadStopFlag::init() const { ResetEvent(handle); }
