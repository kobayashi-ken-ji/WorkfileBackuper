#pragma once
#include <Windows.h>


// コンフィグ構造体
struct ConfigStruct {
public:
    // 定数
    static constexpr size_t EXTENSIONS_LENGTH = 5;	// 登録可能数
    static constexpr size_t EXTENSION_LENGTH = 32;	// 拡張子の最大文字数
    static constexpr size_t TIME_INPUT_LENGTH = 8;  // 分 秒 受取用 文字サイズ

    // 型
    using Extension = WCHAR[EXTENSION_LENGTH];

    // メンバ (ファイルに保存される部分)
    WCHAR sourceFolder[MAX_PATH];           // コピー元
    WCHAR destinationFolder[MAX_PATH];	    // コピー先
    DWORD checkInterval;                    // フォルダをチェックする間隔 (秒)
    WCHAR intervalMin[TIME_INPUT_LENGTH];   // 分  入力受付用
    WCHAR intervalSec[TIME_INPUT_LENGTH];   // 秒  入力受付用

    bool  immediate;                        // 上書き直後にバックアップ
    bool  interval;                         // 一定間隔で上書き確認

    bool  bootWindow;                       // 起動時に画面を開くか
    bool  notify;                           // 通知をするか
    bool  folderCheck;                      // フォルダの更新日時をチェック（高速化）するか
    Extension extensions[EXTENSIONS_LENGTH];  // 登録拡張子 [配列]
    BYTE  extensionCount;                   // 拡張子の有効登録数
};


// コンフィグ構造体 + ファイル保存機能
class Config : public ConfigStruct {
public:

    // 保存先
    static constexpr char FILE_PATH[] = "WorkfileBackuper_config.dat";

    // 設定ファイルを読込
    bool loadFile();

    // 設定ファイルに書込
    bool saveFile();

private:

    // 設定ファイルを読み書き
    bool saveLoadFile(bool saveMode);
};
