#pragma once
#include <Windows.h>


// フォントの生成、解放を管理するクラス
class MainFont {
public:
    const HFONT handle;

    // コンストラクタ : フォントを生成
    MainFont() :
        handle(::CreateFontW(
            18, 0,                  // フォントの高さ,幅    old 18  /  15
            0, 0,
            FW_NORMAL,              // 文字太さ
            FALSE, FALSE, FALSE,    // 斜体 下線 打消し線
            DEFAULT_CHARSET,        // キャラセット シフトJSなど
            OUT_DEFAULT_PRECIS,     // 物理フォント検索方法
            CLIP_DEFAULT_PRECIS,    // クリッピング領域からのはみ出し時処理
            DEFAULT_QUALITY,        // 論理と物理のフォント属性を どの程度一致させるか
            0,                      // ピッチとファミリ
            (LPCWSTR)TEXT("Meiryo") // フォントの識別子  /  NULL
        ))
    {}


    // デストラクタ : フォントを解放
    ~MainFont() {
        if (handle != nullptr) {
            ::DeleteObject(handle);
        }
    }
};
