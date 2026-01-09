#pragma once
#include <Windows.h>


// コンフィグ構造体
struct ConfigStruct {
public:
    // 定数
    static constexpr size_t EXTENSIONS_LENGTH = 5;	// 登録可能数
    static constexpr size_t EXTENSION_LENGTH = 32;	// 拡張子の最大文字数
    static constexpr size_t TIME_INPUT_LENGTH = 8;  // 秒 受取用文字サイズ

    // 型
    using Extension = WCHAR[EXTENSION_LENGTH];

    // メンバ (ファイルに保存される部分)
    WCHAR sourceFolder[MAX_PATH];               // コピー元
    WCHAR destinationFolder[MAX_PATH];	        // コピー先
    DWORD waitTime;                             // ファイル変更されてから、バックアップ開始までの時間 (秒)
    bool  windowLaunch;                         // 起動時に画面を開くか
    bool  notification;                         // 通知をするか
    Extension extensions[EXTENSIONS_LENGTH];    // 登録拡張子 [配列]
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
