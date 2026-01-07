#include "../include/Config.h"
#include <cstdio>

//=============================================================================
// Configクラス
//=============================================================================

bool Config::loadFile() { return saveLoadFile(false); }
bool Config::saveFile() { return saveLoadFile(true);  }


bool Config::saveLoadFile(bool saveMode) {

    // fread() で this に書込むため、メソッドをconstにはできない

    const char* openMode = (saveMode) ? "wb" : "rb";
    const size_t size = sizeof(Config);

    // バイナリモード でファイルを開く
    FILE* fp = NULL;
    if (fopen_s(&fp, FILE_PATH, openMode) != 0)
        return false;

    // 読み書きできた構造体の数
    size_t successCount = (saveMode)
        ? fwrite(this, size, 1, fp)   // 書込
        : fread (this, size, 1, fp);  // 読込

    fclose(fp);

    // 読み書き数が0 → 失敗
    const bool success = (successCount > 0);
    return success;
}