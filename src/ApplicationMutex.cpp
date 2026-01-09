#include "../include/ApplicationMutex.h"

//=============================================================================
// 多重起動防止クラス
// ハンドルのライフサイクルを管理
//=============================================================================

// コンストラクタ
ApplicationMutex::ApplicationMutex(const WCHAR* name)
    : name(name), handle(nullptr) {}

// デストラクタ (ハンドルの解放)
ApplicationMutex::~ApplicationMutex() { 
    close();
}


// 既にアプリが起動しているか (重複起動したか)
bool ApplicationMutex::isAlreadyStarted() {

    // ハンドル取得
    handle = CreateMutexW(
        NULL,       // セキュリティ記述子
        TRUE,       // 最初の所有者
        name        // オブジェクトの名前
    );

    // 起動しているか確認
    const bool isStarted = (GetLastError() == ERROR_ALREADY_EXISTS);
    return isStarted;
}


// 所有権、ハンドルを解放
void ApplicationMutex::close() {
    if (handle == nullptr) return;

    // 所有権の破棄
    bool success = ReleaseMutex(handle);
    if (!success) return;

    // ハンドルの解放
    CloseHandle(handle);
}