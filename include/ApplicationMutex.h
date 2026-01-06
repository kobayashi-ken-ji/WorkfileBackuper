#pragma once
#include <Windows.h>


// 多重起動防止クラス (ハンドルはデストラクタで自動解放)
class ApplicationMutex {
private:

    const WCHAR* name;  // オブジェクト名
    HANDLE handle;      // ハンドル

public:

    // コンストラクタ
    ApplicationMutex(const WCHAR* name) : name(name), handle(nullptr) {}

    // デストラクタ (ハンドルの解放)
    ~ApplicationMutex() { close(); }


    // 既にアプリが起動しているか (重複起動したか)
    // @param showMessageBox  起動済みの場合、メッセージボックスを表示する
    bool isAlreadyStarted(bool showMessageBox) {

        // ハンドル取得
        handle = CreateMutexW(
            NULL,       // セキュリティ記述子
            TRUE,       // 最初の所有者
            name        // オブジェクトの名前
        );

        // 起動しているか確認
        const bool isStarted = (GetLastError() == ERROR_ALREADY_EXISTS);

        // メッセージボックス
        if (showMessageBox && isStarted)
            MessageBoxW(nullptr, L"既に起動しています。", AppInfo::NAME, MB_OK);

        return isStarted;
    }

    // 所有権、ハンドルを解放
    void close() {
        if (handle == nullptr) return;

        // 所有権の破棄
        bool success = ReleaseMutex(handle);
        if (!success) return;
        
        // ハンドルの解放
        CloseHandle(handle);
    }
};