#pragma once
#include <Windows.h>
#include <cassert>
#include <array>


// 座標・サイズ (子ウィンドウ用)
struct Rect {
    int x, y, w, h;
};


// Rect生成ユーティリティー
// 横一行分のフォーマット
//      コンストラクタで {20%幅, 10%幅, 70%幅} のように分割を指定
//      生成後は get(index) でRect型で取得できる
class RowFormat {
public:
    //-------------------------------------------
    // 型定義
    //-------------------------------------------

    // 1行内の要素数 (最大値)
    static constexpr int ROW_LENGTH = 6;

    // 1行の内訳 (%)
    // 合計を100%にする
    // 例: {25, 25, 25, 25} → 等分幅の要素が4つ
    using Ratios = const double[ROW_LENGTH];
    
    // コンストラクタ引数をまとめた型
    struct Param {
        const int parentW;      // 親ウィンドウの幅 (Invisible Borders を除く)
        const int paddingW;     // 左右の余白
        const int autoMarginW;  // 要素同士の隙間幅 (0も可)
        const int defaultH;     // 既定高さ  ※get()で h を指定しない場合に使用
    };

private:
    // {X座標、横幅} の配列型
    struct Horizontal { int x, w; };
    using Horizontals = std::array<Horizontal, ROW_LENGTH>;

    //-------------------------------------------
    // メンバ
    //-------------------------------------------

    const Horizontals horizontals;
    const int defaultH;

public:
    //-------------------------------------------
    // メソッド
    //-------------------------------------------

    // コンストラクタ
    constexpr RowFormat(const Param& param, Ratios& format) :
        horizontals{ getHorizontals(param, format) },
        defaultH(param.defaultH)
    {}


    // Rectを生成
    // @param index 取得したい要素番号 0から開始
    // @param yAdd  改行高さ (前回生成時のy + yAdd = 今回のy)
    // @param h     高さ (省略時は規定高さを使用)
    Rect get(int index, int yAdd = 0, int h = 0) const {
        assert(index < ROW_LENGTH);

        // y (全インスタンス共通の値)
        // 改行高さを加算
        static int y = 0;
        y += yAdd;

        const Horizontal& horiz = horizontals[index];
        if (h == 0) h = defaultH;

        return Rect(horiz.x, y, horiz.w, h);
    }

private:

    // {x, w} の配列を生成
    constexpr Horizontals getHorizontals(const Param& param, Ratios& ratios) const {

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
};
