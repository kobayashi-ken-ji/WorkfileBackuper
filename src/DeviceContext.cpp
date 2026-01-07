#pragma once
#include "../include/DeviceContext.h"

//=============================================================================
// デバイスコンテキストをラッピング
// ハンドルのライフサイクルを管理
//=============================================================================

// コンストラクタ / 描画開始
DeviceContext::DeviceContext(HWND hwnd, HFONT hFont) :
    hwnd(hwnd),
    hdc(BeginPaint(hwnd, &ps))
{
    // 背景モード、フォントを指定
    SetBkMode(hdc, TRANSPARENT);
    SelectObject(hdc, hFont);
}

// デストラクタ / リソースを解放
DeviceContext::~DeviceContext() {
    EndPaint(hwnd, &ps);
}


// 塗りつぶしの色設定 (透明)
void DeviceContext::setBrush() {
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
}

// 塗りつぶしの色設定
void DeviceContext::setBrush(COLORREF color) {
    SelectObject(hdc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(hdc, color);
}


// 輪郭線の色設定 (透明)
void DeviceContext::setPen() {
    SelectObject(hdc, GetStockObject(NULL_PEN));
}

// 輪郭線の色設定
void DeviceContext::setPen(COLORREF color) {
    SelectObject(hdc, GetStockObject(DC_PEN));
    SetDCPenColor(hdc, color);
}
