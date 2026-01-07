#include "../include/ChildWindow.h"
#include <cassert>

//=============================================================================
// 文字をウィンドウに描画 (デバイスコンテキストを使用)
//=============================================================================

// コンストラクタ
TextDC::TextDC(const WCHAR* text, Rect rect) : text(text), rect(rect) {}

// テキストを描画
bool TextDC::textOut(HDC hdc) const {
    const int length = lstrlenW(text);
    return TextOutW(hdc, rect.x, rect.y, text, length);
}

//=============================================================================
// 矩形をウィンドウに描画 (デバイスコンテキストを使用)
//=============================================================================

// コンストラクタ
RectDC::RectDC(Rect rect) : rect(rect) {}

// 矩形で塗りつぶし
bool RectDC::rectangle(HDC hdc) const {

    const int right  = rect.x + rect.w;
    const int bottom = rect.y + rect.h;
    return Rectangle(hdc, rect.x, rect.y, right, bottom);
}

//=============================================================================
// 子ウィンドウの共通部分 (親クラス)
// ハンドルのライフサイクルを管理
//=============================================================================

// コンストラクタ
ChildWindowBase::ChildWindowBase(int64_t id, const WCHAR* text, Rect wndSize)
    : id(id), text(text), rect(wndSize), hSelf(nullptr) {}


// デストラクタ (ハンドルを解放)
ChildWindowBase::~ChildWindowBase() {
    DestroyWindow(hSelf);
}


// 有効/無効 を切替え
bool ChildWindowBase::enable(bool enable) const {
    assert(hSelf != nullptr);
    return EnableWindow(hSelf, enable);
}


// ウィンドウ作成 (子クラスの create() から呼び出される)
void ChildWindowBase::createBase(const CreateWindowArgs &args, LPCWSTR className, DWORD style) {

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

//=============================================================================
// ボタン
//=============================================================================

//  ボタン作成
void Button::create(CreateWindowArgs args) {
    const DWORD style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
    createBase(args, WC_BUTTON, style);
}

//=============================================================================
// チェックボックス
//=============================================================================

// チェックに値を設定
void CheckBox::set(bool value) const {
    assert(hSelf != nullptr);
    const WPARAM wParam = value ? BST_CHECKED : BST_UNCHECKED;
    SendMessageW(hSelf, BM_SETCHECK, wParam, 0);     // 戻り値は常に0
}


// チェックの値を取得
bool CheckBox::get() const {
    assert(hSelf != nullptr);
    return SendMessageW(hSelf, BM_GETCHECK, 0, 0);
}


//  チェックボックス作成
void CheckBox::create(const CreateWindowArgs& args) {
    const DWORD dwStyle = WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX;
    createBase(args, WC_BUTTON, dwStyle);
}

//=============================================================================
// ラジオボタン
//=============================================================================

// コンストラクタ
RadioButton::RadioButton(int64_t id, const WCHAR* text, Rect rect, bool isNewGroup)
    : CheckBox(id, text, rect), isNewGroup(isNewGroup) {}


// ラジオボタン作成
void RadioButton::create(const CreateWindowArgs& args) {
    DWORD style = WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON;   
    if (isNewGroup) style |= WS_GROUP;    
    createBase(args, WC_BUTTON, style);
}

//=============================================================================
// 編集ボックス
//=============================================================================

// コンストラクタ    
EditBox::EditBox(int64_t id, Rect rect, DWORD style)
    : ChildWindowBase(id, nullptr, rect), style(style) {}


// ボックス内容を変更
bool EditBox::set(const WCHAR* text) const {
    assert(hSelf != nullptr);
    return SetWindowTextW(hSelf, text);
}

// ボックス内容を取得
int EditBox::get(WCHAR* buffer, int size) const {
    assert(hSelf != nullptr);
    return GetWindowTextW(hSelf, buffer, size);
}

// ボックス内容を変更 (数値 → 文字列)
bool EditBox::set(int number) const {
    assert(hSelf != nullptr);

    WCHAR buffer[TEXT_LENGTH] = { 0 };
    _itow_s(number, buffer, TEXT_LENGTH, 10);
    return SetWindowTextW(hSelf, buffer);
}

// ボックス内容を取得 (文字列 → 数値)
int EditBox::get() const {
    assert(hSelf != nullptr);

    WCHAR buffer[TEXT_LENGTH] = { 0 };
    GetWindowTextW(hSelf, buffer, TEXT_LENGTH);
    return _wtoi(buffer);
}

//  編集ボックス作成
void EditBox::create(const CreateWindowArgs& args) {
    createBase(args, WC_EDIT, style);
}
