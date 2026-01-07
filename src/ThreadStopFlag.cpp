#include "../include/ThreadStopFlag.h"
#include <synchapi.h>
#include <handleapi.h>
#include <cassert>

//=============================================================================
// スレッド停止用のフラグ
//=============================================================================

// イベントを生成
// (セキュリティ属性なし、手動リセット, 初期状態は非シグナル、イベント名なし)
ThreadStopFlag::ThreadStopFlag() : handle{ CreateEventW(NULL, TRUE, FALSE, NULL) } {
    assert(handle != NULL);
}

// デストラクタ (イベントを解放)
ThreadStopFlag::~ThreadStopFlag() { CloseHandle(handle); }


// 停止フラグを立てる (シグナル化)
// WaitForSingleObject側は待機が終了する
void ThreadStopFlag::stop() const { SetEvent(handle); }


// 停止フラグを初期化 (非シグナル化)
void ThreadStopFlag::init() const { ResetEvent(handle); }
