#pragma once
#include <Windows.h>


// デバイスコンテキストをラッピング
// ハンドルのライフサイクルを管理
class DeviceContext {
private:

    PAINTSTRUCT ps;
    const HWND hwnd;

public:
    const HDC hdc;

    // コンストラクタ
    DeviceContext(HWND hwnd, HFONT hFont) :
        hwnd(hwnd),
        hdc(BeginPaint(hwnd, &ps))  // 描画開始
    {
        // 背景モード、フォントを指定
        SetBkMode(hdc, TRANSPARENT);
        SelectObject(hdc, hFont);
    }

    // デストラクタ (リソースを解放)
    ~DeviceContext() {
        EndPaint(hwnd, &ps);
    }


    // 塗りつぶしの色設定 (透明)
    void setBrush() {
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
    }

    // 塗りつぶしの色設定
    void setBrush(COLORREF color) {
        SelectObject(hdc, GetStockObject(DC_BRUSH));
        SetDCBrushColor(hdc, color);
    }


    // 輪郭線の色設定 (透明)
    void setPen() {
        SelectObject(hdc, GetStockObject(NULL_PEN));
    }

    // 輪郭線の色設定
    void setPen(COLORREF color) {
        SelectObject(hdc, GetStockObject(DC_PEN));
        SetDCPenColor(hdc, color);
    }
};