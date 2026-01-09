#pragma once
#include <Windows.h>


// デバイスコンテキストをラッピング
// ハンドルのライフサイクルを管理
class DeviceContext {
private:

    PAINTSTRUCT ps;
    const HWND hWnd;
public:
    const HDC hdc;

    // コンストラクタ / 描画開始
    DeviceContext(HWND hWnd, HFONT hFont);

    // デストラクタ / リソースを解放
    ~DeviceContext();


    // 塗りつぶしの色設定 (透明)
    void setBrush();

    // 塗りつぶしの色設定
    void setBrush(COLORREF color);


    // 輪郭線の色設定 (透明)
    void setPen();

    // 輪郭線の色設定
    void setPen(COLORREF color);
};
