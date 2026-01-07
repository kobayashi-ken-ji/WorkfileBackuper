#include "../include/Rect.h"
#include <cassert>

//=============================================================================
// 横一行分のフォーマット
//=============================================================================

// constexpr は今回不採用
// constexpr はコンパイル時に値を決定するため、呼出し側から見えている必要がある。
// つまり、ヘッダにコンストラクタ関連を全て書かなければならない。

using Horizontals   = RowFormat::Horizontals;


// コンストラクタ
RowFormat::RowFormat(const Param& param, Ratios& format) :
    horizontals{ getHorizontals(param, format) },
    defaultH(param.defaultH)
{}


// Rectを生成
// @param index 取得したい要素番号 0から開始
// @param yAdd  改行高さ (前回生成時のy + yAdd = 今回のy)
// @param h     高さ (省略時は規定高さを使用)
Rect RowFormat::get(int index, int yAdd, int h) const {
    assert(index < ROW_LENGTH);

    // y (全インスタンス共通の値)
    // 改行高さを加算
    static int y = 0;
    y += yAdd;

    const Horizontal& horiz = horizontals[index];
    if (h == 0) h = defaultH;

    return Rect(horiz.x, y, horiz.w, h);
}


// {x, w} の配列を生成
Horizontals RowFormat::getHorizontals(
    const Param& param, Ratios& ratios) const {

    // 要素数を取得
    int length = 0;
    for (const double& ratio : ratios) {
        if (ratio > 0) length++;
        else break;
    }

    // 内側の幅 (余白を除いた幅)
    const int totalMarginW = (length - 1) * param.autoMarginW;
    const int innerW = param.parentW - (param.paddingW * 2) - totalMarginW;

    // ループ結果を格納
    Horizontals horizontals = { 0 };
    double x = (double)param.paddingW;

    for (int i=0; i<length; i++) {
        Horizontal& horiz = horizontals[i];

        // Xを格納 (四捨五入)
        horiz.x = static_cast<int>(x + 0.5);

        // Wを格納 (% → ピクセル化)
        double w = (ratios[i] * innerW) / 100;
        horiz.w  = static_cast<int>(w + 0.5);

        // 次回のX座標
        x += w + param.autoMarginW;
    }

    return horizontals;
}