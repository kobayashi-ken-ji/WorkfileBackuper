#pragma once
#include <Windows.h>
#include "MainFont.h"
#include "Rect.h"
#include <cassert>
#include <cstdint>

//=============================================================================
// デバイスコンテキストを使用
//=============================================================================

// 文字をウィンドウに描画 (デバイスコンテキストを使用)
class TextDC {
private:
    const WCHAR* text;  // 描画する文字列
    const Rect rect;

public:
    TextDC(const WCHAR* text, Rect rect) : text(text), rect(rect) {}

    // テキストを描画
    bool textOut(HDC hdc) const {
        const int length = lstrlenW(text);
        return TextOutW(hdc, rect.x, rect.y, text, length);
    }
};


// 矩形をウィンドウに描画 (デバイスコンテキストを使用)
class RectDC {
private:
    const Rect rect;

public:
    RectDC(Rect rect) : rect(rect) {}

    // 矩形で塗りつぶし
    bool rectangle(HDC hdc) const {

        const int right = rect.x + rect.w;
        const int bottom = rect.y + rect.h;
        return Rectangle(hdc, rect.x, rect.y, right, bottom);
    }
};

//=============================================================================
// 子ウインドウ クラス
//      View として再構築
//      データとの連携はController側へ移植
//=============================================================================

// 子ウィンドウ生成時の引数
struct CreateWindowArgs {
    HWND	  hParent;    // 親ウィンドウ
    HINSTANCE hInstance;  // アプリインスタンス
    HFONT     hFont;      // フォント
};


// 子ウィンドウの共通部分 (親クラス)
// ハンドルのライフサイクルを管理
class ChildWindowBase {
private:

    const int64_t id;   // プロシージャでキャッチするID
    const WCHAR* text;  // ボタン名、チェックボックス文、ラジオボタン文
    const Rect rect;
protected:
    HWND hSelf;         // createBase()の実行前は nullptr
    
public:

    // コンストラクタ
    ChildWindowBase(int64_t id, const WCHAR* text, Rect wndSize)
        : id(id), text(text), rect(wndSize), hSelf(nullptr) {}

    // デストラクタ (ハンドルを解放)
    virtual ~ChildWindowBase() {
        DestroyWindow(hSelf);
    }

    // 有効/無効 を切替え
    bool enable(bool enable) const {
        assert(hSelf != nullptr);
        return EnableWindow(hSelf, enable);
    }

protected:

    // ウィンドウ作成 (子クラスの create() から呼び出される)
    void createBase(const CreateWindowArgs &args, LPCWSTR className, DWORD style) {

        // 同じウィンドウの重複を避ける
        assert(hSelf == nullptr);

        hSelf = CreateWindowW(
            className,          // クラス名 (WC_BUTTON など)
            text,               // ウィンドウ名
            style,              // ウィンドウスタイル (WS_CHILD など)
            rect.x, rect.y,     // 横, 縦の位置
            rect.w, rect.h,     // ウィンドウの幅, 高さ  ※ ドロップダウンのサイズを含む
            args.hParent,       // 親ウィンドウ
            (HMENU)id,          // 子ウィンドウ識別子 (プロシージャでキャッチするID)
            args.hInstance,     // インスタンス
            NULL                // ウィンドウ作成データ
        );

        // フォントを設定
        SendMessageW(hSelf, WM_SETFONT, (WPARAM)args.hFont, FALSE);
    }
};

//=============================================================================
// ボタン
//=============================================================================

class Button : public ChildWindowBase {
public:

    // 親のコンストラクタを使用
    using ChildWindowBase::ChildWindowBase;

    //  ボタン作成
    void create(CreateWindowArgs args) {
        const DWORD style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
        createBase(args, WC_BUTTON, style);
    }
};

//=============================================================================
// チェックボックス
//=============================================================================

class CheckBox : public ChildWindowBase {
public:

    // 親のコンストラクタを使用
    using ChildWindowBase::ChildWindowBase;

    // チェックに値を設定
    void set(bool value) const {
        assert(hSelf != nullptr);
        const WPARAM wParam = value ? BST_CHECKED : BST_UNCHECKED;
        SendMessageW(hSelf, BM_SETCHECK, wParam, 0);     // 戻り値は常に0
    }

    // チェックの値を取得
    bool get() const {
        assert(hSelf != nullptr);
        return SendMessageW(hSelf, BM_GETCHECK, 0, 0);
    }

    //  チェックボックス作成
    void create(const CreateWindowArgs& args) {
        const DWORD dwStyle = WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX;
        createBase(args, WC_BUTTON, dwStyle);
    }
};

//=============================================================================
// ラジオボタン
//=============================================================================

// チェックボックス + グループ要素
class RadioButton : public CheckBox {
public:

    // グループの1つ目かどうか（排他ボタン用）
    const bool isNewGroup;

    RadioButton(int64_t id, const WCHAR* text, Rect rect, bool isNewGroup)
        : CheckBox(id, text, rect), isNewGroup(isNewGroup) {}


    // ラジオボタン作成
    void create(const CreateWindowArgs& args) {
        DWORD style = WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON;   
        if (isNewGroup) style |= WS_GROUP;    
        createBase(args, WC_BUTTON, style);
    }
};

//=============================================================================
// テキストボックス
//=============================================================================

class EditBox : public ChildWindowBase {
public:
    // 文字列 ←→ 数値 を行うためのバッファサイズ
    static constexpr int TEXT_LENGTH = 256;

    // styleを指定可能 / 指定しなければ定数が使用される
    static constexpr DWORD STYLE = WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER;
    const DWORD style;
    
    EditBox(int64_t id, Rect rect, DWORD style = STYLE)
        : ChildWindowBase(id, nullptr, rect), style(style) {}


    // ボックス内容を変更
    bool set(const WCHAR* text) const {
        assert(hSelf != nullptr);
        return SetWindowTextW(hSelf, text);
    }

    // ボックス内容を取得
    int get(WCHAR* buffer, int size) const {
        assert(hSelf != nullptr);
        return GetWindowTextW(hSelf, buffer, size);
    }

    // ボックス内容を変更 (数値 → 文字列)
    bool set(int number) const {
        assert(hSelf != nullptr);

        WCHAR buffer[TEXT_LENGTH] = { 0 };
        _itow_s(number, buffer, TEXT_LENGTH, 10);
        return SetWindowTextW(hSelf, buffer);
    }

    // ボックス内容を取得 (文字列 → 数値)
    int get() const {
        assert(hSelf != nullptr);

        WCHAR buffer[TEXT_LENGTH] = { 0 };
        GetWindowTextW(hSelf, buffer, TEXT_LENGTH);
        return _wtoi(buffer);
    }

    //  テキストボックス作成
    void create(const CreateWindowArgs& args) {
        createBase(args, WC_EDIT, style);
    }
};
