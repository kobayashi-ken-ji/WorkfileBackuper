#pragma once
#include <Windows.h>
#include "ChildWindow.h"
#include "Config.h"

// メインウィンドウのサイズ
namespace WindowSize {
    constexpr int W = 460;
    constexpr int H = 655;
}

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
	RectDC	waitTimePanel;
	TextDC	waitTimeText;
    EditBox waitTime;
    TextDC	waitTimeSecText;

    // バックアップするファイルの種類 ( 拡張子 )
    RectDC	extensionsPanel;
    TextDC  extensionsText;
    EditBox extensions[Config::EXTENSIONS_LENGTH];

    // 起動時に画面を表示、通知する
    RectDC   optionPanel;
    CheckBox windowLaunch;
    CheckBox notification;

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
};
