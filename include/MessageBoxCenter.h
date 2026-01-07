#pragma once
#include <Windows.h>

// メッセージボックスのショートハンド
#define MsgBox(a,...) (MessageBoxCenter::create(a,__VA_ARGS__))


// メッセージボックス (親ウィンドウの中央に表示)
// 使用方法
// (1) 親ハンドルを指定: HWND MessageBoxCenter::hParent = hWnd;
// (2) メソッドを実行: MsgBox(text [, caption, type]);
class MessageBoxCenter {
public:
    static const int ID = 20;   // 識別ID
    static HWND  hParent;       // 親ウィンドウのハンドル
    static HHOOK hHook;         // フックハンドル


    // メッセージボックス 作成
    static int create(LPCWSTR text, LPCWSTR caption = L"", UINT type = MB_OK);

private:

    // メッセージボックスをサブクラス化 (フックプロシージャ)
    static LRESULT CALLBACK hookProc(int nCode, WPARAM wParam, LPARAM lParam);


    // ウィンドウ位置を、親の中心に移動  (サブクラス化プロシージャ)
    static LRESULT CALLBACK subclassProc(
        HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
        UINT_PTR uIdSubclass, DWORD_PTR dwRefData
    );
};

