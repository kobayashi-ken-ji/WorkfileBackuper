#include "../include/UiElements.h"
#include "../include/DeviceContext.h"
#include "../include/Constants.h"

//=============================================================================
// メインウィンドウ内の要素をまとめたクラス
//=============================================================================

// ※透明なサイズ調整用境界線（Invisible Borders）が片側8pxずつある
constexpr int
    USABLE_W    = WindowSize::W - (8 * 2),  // 実際に描画できる幅
    MARGIN_H    = 5,                        // 要素同士の間隔 (縦)
    ROW_H       = 22,                       // 1行分 高さ
    CRLF        = ROW_H + MARGIN_H,         // 改行高さ 通常
    CRLF2       = CRLF + 10,                // 改行高さ 大
    HISTORY_H   = 175;


// 横一列のフォーマット (ウィンドウ幅の分割指定)
namespace RowFormats {

    // 親ウィンドウ幅、左右のパディング、要素間のマージン、既定の高さ
    namespace Params {
        constexpr RowFormat::Param
            OUTER  { USABLE_W, 10, 8, 22 },  // パネルなど用
            INNER  { USABLE_W, 25, 8, 22 };  // パネルの内側用
    }

    using namespace Params;
    const RowFormat
        PANEL               (OUTER, { 100 }),
        FULLWIDTH           (INNER, { 100 }),
        SHORT_LONG          (INNER, { 30, 70 }),
        LONG_SHORT          (INNER, { 70, 30 }),
        DIVIDE3             (INNER, { 33.3, 33.4, 33.3 }),
        DIVIDE4             (INNER, { 25, 25, 25, 25 }),
        TIME                (INNER, { 70, 25, 5 }),     // [空白][00][秒]
        INDENT_SHORT_LONG   (INNER, { 5, 25, 70 });
}

// ウィンドウスタイル
constexpr DWORD FOLDER_STYLE  = WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER | ES_AUTOHSCROLL;
constexpr DWORD TIME_STYLE    = WS_VISIBLE | WS_CHILD | ES_RIGHT | WS_BORDER | ES_NUMBER;
constexpr DWORD HISTORY_STYLE = WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER | WS_HSCROLL |
                                ES_READONLY | ES_MULTILINE | ES_WANTRETURN; // 読取りのみ


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
    waitTimePanel (PANEL.get(0, CRLF2, CRLF*2 + MARGIN_H + 2)),
    waitTimeText  (L"ファイルが変更されてから、バックアップを開始するまでの時間", SHORT_LONG.get(0, MARGIN_H)),
    waitTime      (ID_WAIT_TIME, TIME.get(1, CRLF), TIME_STYLE),
    waitTimeSecText (L"秒", TIME.get(2, 2)),

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
    windowLaunch(ID_WINDOW_LAUNCH, L"アプリ起動時にこの画面を開く", FULLWIDTH.get(0, MARGIN_H)),
    notification(ID_NOTIFICATION, L"デスクトップ通知をする", FULLWIDTH.get(0, CRLF)),

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
    waitTime    .create(args);

    for (EditBox& extension : extensions)
        extension.create(args);

    windowLaunch.create(args);
    notification.create(args);
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
    waitTimePanel   .rectangle(hdc);
    waitTimeText    .textOut(hdc);
    waitTimeSecText .textOut(hdc);
    extensionsPanel .rectangle(hdc);
    extensionsText  .textOut(hdc);
    optionPanel     .rectangle(hdc);
}
