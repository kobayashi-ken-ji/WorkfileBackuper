#pragma once
#include <Windows.h>
#include "Constants.h"
#include "ChildWindow.h"
#include "Config.h"

// メインウィンドウのサイズ
namespace WindowSize {
    constexpr int W = 460;
    constexpr int H = 709;
}

// ※透明なサイズ調整用境界線（Invisible Borders）が片側8pxずつある
constexpr int
    USABLE_W    = WindowSize::W - (8 * 2),  // 実際に描画できる幅
    MARGIN_H    = 5,                        // 要素同士の間隔 (縦)
    ROW_H       = 22,                       // 1行分 高さ
    CRLF        = ROW_H + MARGIN_H,         // 改行高さ 通常
    CRLF2       = CRLF + 10,                // 改行高さ 大
    HISTORY_H   = 175;


namespace RowFormats {

    // 親ウィンドウ幅、左右のパディング、要素間のマージン、既定の高さ
    namespace Params {
        constexpr RowFormat::Param
            OUTER  { USABLE_W, 10, 8, 22 },  // パネルなど用
            INNER  { USABLE_W, 25, 8, 22 };  // パネルの内側用
    }

    using namespace Params;
    constexpr RowFormat
        PANEL               (OUTER, { 100 }),
        FULLWIDTH           (INNER, { 100 }),
        SHORT_LONG          (INNER, { 30, 70 }),
        LONG_SHORT          (INNER, { 70, 30 }),
        DIVIDE3             (INNER, { 33.3, 33.4, 33.3 }),
        DIVIDE4             (INNER, { 25, 25, 25, 25 }),
        TIME                (INNER, { 5, 67, 9, 5, 9, 5 }),
        INDENT_SHORT_LONG   (INNER, { 5, 25, 70 });
}

// ウィンドウスタイル
constexpr DWORD FOLDER_STYLE  = WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER | ES_AUTOHSCROLL;
constexpr DWORD TIME_STYLE    = WS_VISIBLE | WS_CHILD | ES_RIGHT | WS_BORDER | ES_NUMBER;
constexpr DWORD HISTORY_STYLE = WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER | WS_HSCROLL |
                                ES_READONLY | ES_MULTILINE | ES_WANTRETURN; // 読取りのみ

// メインウィンドウ内の要素をまとめたクラス
class UiElements {
public:

	// バックアップ元 / 先
	RectDC	folderPanel;
	TextDC	sourceFolderText;
	EditBox sourceFolder;
	TextDC	destinationFolderText;
	EditBox destinationFolder;

	// バックアップのタイミング
	RectDC		timingPanel;
	TextDC		timingText;
	RadioButton immediate;
	RadioButton interval;

	// バックアップ間隔
	TextDC	 intervalMinText;
	EditBox  intervalMin;
	TextDC	 intervalSecText;
	EditBox  intervalSec;
	CheckBox folderCheck;
	Button	 notesButton;

    // バックアップするファイルの種類 ( 拡張子 )
    RectDC	extensionsPanel;
    TextDC  extensionsText;
    EditBox extensions[Config::EXTENSIONS_LENGTH];

    // 起動時に画面を表示、通知する
    RectDC   optionPanel;
    CheckBox bootWindow;
    CheckBox notify;

    // バージョン、開始、停止、終了 ボタン
    Button helpButton;
    Button applyButton;
    Button stopButton;
    Button closeButton;

    // 履歴表示
    EditBox history;

    UiElements();
    void createAll(const CreateWindowArgs& args);
    void paintAll(HWND hWnd, HFONT hFont) const;
    void intervalGroupEnable(bool enable) const;
};


// コンストラクタ
// 初期化子を使用
using namespace RowFormats;
UiElements::UiElements() :

    folderPanel(PANEL.get(0, MARGIN_H * 2, CRLF * 2 + MARGIN_H)),

    // バックアップ元 / 先
    sourceFolderText(L"バックアップ元", SHORT_LONG.get(0, MARGIN_H)),
    sourceFolder (ID_SOURCE_FOLDER, SHORT_LONG.get(1), FOLDER_STYLE),

    destinationFolderText (L"バックアップ先", SHORT_LONG.get(0, CRLF)),
    destinationFolder (ID_DESTINATION_FOLDER, SHORT_LONG.get(1), FOLDER_STYLE),


    // バックアップのタイミング
    timingPanel (PANEL.get(0, CRLF2, CRLF*4 + MARGIN_H)),
    timingText  (L"バックアップのタイミング", SHORT_LONG.get(0, MARGIN_H)),

    immediate  (ID_IMMEDIATE, L"上書き直後にバックアップ", TIME.get(1, CRLF), true),
    interval   (ID_INTERVAL, L"一定間隔で上書きを確認", TIME.get(1, CRLF), false),

    // バックアップ間隔
    intervalMin     (ID_INTERVAL_MIN, TIME.get(2), TIME_STYLE),
    intervalMinText (L"分", TIME.get(3)),

    intervalSec     (ID_INTERVAL_SEC, TIME.get(4), TIME_STYLE),
    intervalSecText (L"秒", TIME.get(5)),

    // 負荷を軽減
    folderCheck (ID_FOLDER_CHECK, L"負荷を軽減", TIME.get(1, CRLF)),
    notesButton (ID_NOTES, L"[！] 軽減 注意事項", LONG_SHORT.get(1)),


    // 拡張子
    extensionsPanel (PANEL.get(0,  CRLF2, CRLF * 6 + MARGIN_H)),
    extensionsText (L"バックアップするファイルの種類 ( 拡張子 )", FULLWIDTH.get(0, MARGIN_H)),
    extensions {
        { ID_EXTENSION0, DIVIDE3.get(2, CRLF) },
        { ID_EXTENSION1, DIVIDE3.get(2, CRLF) },
        { ID_EXTENSION2, DIVIDE3.get(2, CRLF) },
        { ID_EXTENSION3, DIVIDE3.get(2, CRLF) },
        { ID_EXTENSION4, DIVIDE3.get(2, CRLF) },
    },

    
    // 起動時に画面を表示、通知する
    optionPanel (PANEL.get(0,  CRLF2, CRLF * 2 + MARGIN_H)),
    bootWindow  (ID_BOOT_WINDOW, L"アプリ起動時にこの画面を開く", FULLWIDTH.get(0, MARGIN_H)),
    notify      (ID_NOTIFY, L"デスクトップ通知をする", FULLWIDTH.get(0, CRLF)),

    // ボタン
    helpButton  (ID_HELP_BUTTON , L"バージョン"  , DIVIDE4.get(0, CRLF2)),
    applyButton (ID_APPLY_BUTTON, L"適用・開始"  , DIVIDE4.get(1)),
    stopButton  (ID_STOP_BUTTON , L"停止"       , DIVIDE4.get(2)),
    closeButton (ID_CLOSE_BUTTON, L"終了"       , DIVIDE4.get(3)),

    // 履歴表示
    history (ID_HISTORY, PANEL.get(0, CRLF2, HISTORY_H), HISTORY_STYLE)
{}


// 子ウィンドウを生成
void UiElements::createAll(const CreateWindowArgs &args) {

    InitCommonControls();

    sourceFolder.create(args);
    destinationFolder.create(args);
    immediate   .create(args);
    interval    .create(args);
    intervalMin .create(args);
    intervalSec .create(args);
    folderCheck .create(args);
    notesButton .create(args);

    for (EditBox& extension : extensions)
        extension.create(args);

    bootWindow  .create(args);
    notify      .create(args);
    helpButton  .create(args);
    applyButton .create(args);
    stopButton  .create(args);
    closeButton .create(args);
    history     .create(args);
};


// 文字列、パネルを描画 (WM_PAINT の処理)
void UiElements::paintAll(HWND hWnd, HFONT hFont) const {

    DeviceContext dc = { hWnd, hFont };
    HDC hdc = dc.hdc;

    // システムカラー取得
    // const COLORREF menuColor = GetSysColor(COLOR_MENU);  

    // 塗りつぶし(ブラシ)  輪郭線(ペン) の色設定
    dc.setBrush(RGB(240, 240, 240));
    dc.setPen  (RGB(210, 210, 210));

    // 各要素を描画
    folderPanel     .rectangle(hdc);
    sourceFolderText.textOut(hdc);
    destinationFolderText.textOut(hdc);
    timingPanel     .rectangle(hdc);
    timingText      .textOut(hdc);
    intervalMinText .textOut(hdc);
    intervalSecText .textOut(hdc);
    extensionsPanel .rectangle(hdc);
    extensionsText  .textOut(hdc);
    optionPanel     .rectangle(hdc);
}

// ラジオボタン に関連するウィンドウの 有効/無効 を切替え
void UiElements::intervalGroupEnable(bool enable) const {

    intervalMin.enable(enable);
    intervalSec.enable(enable);
    folderCheck.enable(enable);
    notesButton.enable(enable);
}
