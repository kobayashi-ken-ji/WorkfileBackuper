#pragma once
#include <Windows.h>

// メッセージボックスのショートハンド
#define MsgBox(a,...) (MessageBoxCenter::create(a,__VA_ARGS__))

/*   
    フック処理の流れ
        (1) フックを仕込む 
            → 本来のプロシージャの前に、hookProc が実行されるようになる

        (2) MessageBox() を実行
        (3) メッセージがフックに掛かる

        (4) hookProcが実行される (サブクラス化処理)
            → MessageBoxのプロシージャの前に subclassProc が実行されるようになる

        (5) subclassProc内でダイアログの初期化時を捉え、ウインドウの位置を調整
*/

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
    static int create(LPCWSTR text, LPCWSTR caption = L"", UINT type = MB_OK) {

        // フック（メッセージボックスをサブクラス化する処理) を仕込む
        hHook = SetWindowsHookExW(
            WH_CALLWNDPROC,			// メッセージを宛先プロシージャに送信する 前 に処理を行う
            hookProc,		        // サブクラス化処理を指定
            NULL,					// DLL へのハンドル
            GetCurrentThreadId()	// 対象スレッドの識別子	*今回はメインスレッドを監視
        );

        return MessageBoxW(hParent, text, caption, type);
    }

private:

    // メッセージボックスをサブクラス化 (フックプロシージャ)
    static LRESULT CALLBACK hookProc(int nCode, WPARAM wParam, LPARAM lParam) {

        if (nCode == HC_ACTION) {

            // CallWindowProc構造体 
            CWPSTRUCT* pcwp = (CWPSTRUCT*)lParam;
            switch (pcwp->message) {

            // ダイアログが初期化されるとき
            case WM_INITDIALOG:{

                // サブクラス化
                // 生成されたメッセージウィンドウのハンドルを使用する
                SetWindowSubclass(      
                    pcwp->hwnd,         // サブクラス化するウィンドウのハンドル
                    subclassProc,       // プロシージャ を指定
                    ID,                 // サブクラスのID を指定
                    0                   // 参照するデータ  呼出元アプリによって決定
                );

                // このフックプロシージャ自体を 解除
                UnhookWindowsHookEx(hHook);
                break;
            }
            } // swicth
        }
        return CallNextHookEx(hHook, nCode, wParam, lParam);
    }


    // ウィンドウ位置を、親の中心に移動  (サブクラス化プロシージャ)
    static LRESULT CALLBACK subclassProc(
        HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
        UINT_PTR uIdSubclass, DWORD_PTR dwRefData
    ) {
        switch (uMsg) {

        // 初期化時
        case WM_INITDIALOG: {
            RECT rect;

            // メインウィンドウ 座標
            GetWindowRect(hParent, &rect);
            LONG mainX = rect.left;
            LONG mainY = rect.top;
            LONG mainW = rect.right - rect.left;
            LONG mainH = rect.bottom - rect.top;

            // メッセージボックス 座標 (親ウィンドウの中心へ)
            GetWindowRect(hWnd, &rect);
            LONG w = rect.right - rect.left;
            LONG h = rect.bottom - rect.top;
            LONG x = mainX + ((mainW - w) / 2);
            LONG y = mainY + ((mainH - h) / 2);

            // ウィンドウを移動
            MoveWindow(hWnd, x, y, w, h, false);  // 再描画なし
            break;
        }

        // ウィンドウ終了時に サブクラスを削除
        case WM_DESTROY:
            RemoveWindowSubclass(hWnd, subclassProc, ID);
            break;

        } // switch
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }
};

// 静的メンバ 初期化
HWND  MessageBoxCenter::hParent;
HHOOK MessageBoxCenter::hHook;
