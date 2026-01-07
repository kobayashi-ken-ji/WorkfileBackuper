#pragma once
#include <Windows.h>


// フォントの生成、解放を管理するクラス
class MainFont {
public:
    const HFONT handle;

    // コンストラクタ : フォントを生成
    MainFont();

    // デストラクタ : フォントを解放
    ~MainFont();
};
