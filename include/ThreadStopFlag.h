#pragma once
#include <handleapi.h>
#include <synchapi.h>
#include <cassert>


// スレッド停止用のフラグ
// 使用例
// (1) インスタンス化 : ThreadStopFlag flag;
// (2) スレッドを待機 : WaitForSingleObject(flag.handle, INFINITE);
// (3) 外部からの停止命令 : flag.stop();
// (4) 1の関数の待機が終了する
class ThreadStopFlag {
public:
    // イベントハンドル
    const HANDLE handle;

    // イベントを生成
    // (セキュリティ属性なし、手動リセット, 初期状態は非シグナル、イベント名なし)
    ThreadStopFlag() : handle{ CreateEventW(NULL, TRUE, FALSE, NULL) } {
        assert(handle != NULL);
    }

    // デストラクタ (イベントを解放)
    ~ThreadStopFlag() { CloseHandle(handle); }

    // 停止フラグを立てる (シグナル化)
    // WaitForSingleObject側は待機が終了する
    void stop() const { SetEvent(handle); }

    // 停止フラグを初期化 (非シグナル化)
    void init() const { ResetEvent(handle); }
};
