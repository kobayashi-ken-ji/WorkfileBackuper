#pragma once
#include <Windows.h>
#include <commctrl.h>
#include "MainFont.h"
#include "Rect.h"

//=============================================================================
// デバイスコンテキストを使用
//=============================================================================

// 文字をウィンドウに描画 (デバイスコンテキストを使用)
class TextDC {
private:
    const WCHAR* text;  // 描画する文字列
    const Rect rect;    // 座標、サイズ

public:
    TextDC(const WCHAR* text, Rect rect);

    // テキストを描画
    bool textOut(HDC hdc) const;
};


// 矩形をウィンドウに描画 (デバイスコンテキストを使用)
class RectDC {
private:
    const Rect rect;

public:
    RectDC(Rect rect);

    // 矩形で塗りつぶし
    bool rectangle(HDC hdc) const;
};

//=============================================================================
// 子ウインドウ
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
    ChildWindowBase(int64_t id, const WCHAR* text, Rect wndSize);

    // デストラクタ (ハンドルを解放)
    virtual ~ChildWindowBase();

    // 有効/無効 を切替え
    bool enable(bool enable) const;

protected:
    // ウィンドウ作成 (子クラスの create() から呼び出される)
    // @param args      「子クラス共通の引数」をまとめた構造体
    // @param className WC_BUTTON など
    // @param style     WS_CHILD | WS_VISIBLE など
    void createBase(const CreateWindowArgs& args, LPCWSTR className, DWORD style);
};

//=============================================================================
// ボタン
//=============================================================================

class Button : public ChildWindowBase {
public:
    // 親のコンストラクタを使用
    using ChildWindowBase::ChildWindowBase;

    //  ボタン作成
    void create(CreateWindowArgs args);
};

//=============================================================================
// チェックボックス
//=============================================================================

class CheckBox : public ChildWindowBase {
public:
    // 親のコンストラクタを使用
    using ChildWindowBase::ChildWindowBase;

    // チェックに値を設定
    void set(bool value) const;

    // チェックの値を取得
    bool get() const;

    //  チェックボックス作成
    void create(const CreateWindowArgs& args);
};

//=============================================================================
// ラジオボタン
//=============================================================================

// チェックボックス + グループ要素
class RadioButton : public CheckBox {
public:
    // グループの1つ目かどうか（排他ボタン用）
    const bool isNewGroup;

    RadioButton(int64_t id, const WCHAR* text, Rect rect, bool isNewGroup);

    // ラジオボタン作成
    void create(const CreateWindowArgs& args);
};

//=============================================================================
// 編集ボックス
//=============================================================================

class EditBox : public ChildWindowBase {
private:
    // 文字列 ←→ 数値 を行うためのバッファサイズ
    static constexpr int TEXT_LENGTH = 256;

    // styleのデフォルト値
    static constexpr DWORD STYLE = WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER;
    const DWORD style;

public:
    EditBox(int64_t id, Rect rect, DWORD style = STYLE);

    // ボックス内容を変更
    bool set(const WCHAR* text) const;

    // ボックス内容を取得
    int get(WCHAR* buffer, int size) const;

    // ボックス内容を変更 (数値入力)
    bool set(int number) const;

    // ボックス内容を取得 (数値出力)
    int get() const;

    // 編集ボックス作成
    void create(const CreateWindowArgs& args);
};
