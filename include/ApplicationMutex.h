#pragma once
#include <Windows.h>


// 多重起動防止クラス
// ハンドルのライフサイクルを管理
class ApplicationMutex {
private:
    const WCHAR* name;  // オブジェクト名
    HANDLE handle;      // ハンドル

public:
    // コンストラクタ
    ApplicationMutex(const WCHAR* name);

    // デストラクタ (ハンドルの解放)
    ~ApplicationMutex();

    // 既にアプリが起動しているか (重複起動したか)
    bool isAlreadyStarted();

    // 所有権、ハンドルを解放
    void close();
};