#include "fs_tools.h"

static bool fs_tools_FS_is_begin(void)
{
    if (SPIFFS.begin()) return true;
    else return false;
}

int fs_tools_FS_info(void)
{
    if (!fs_tools_FS_is_begin()) return -1;

    FSInfo fs_info;
    SPIFFS.info(fs_info);

    Serial.println();
    Serial.println("/========================== SPIFFS info start ==========================/");
    Serial.println((String)("totalBytes: ") + fs_info.totalBytes + " Bytes");
    Serial.println((String)("usedBytes: ") + fs_info.usedBytes + " Bytes");
    Serial.println((String)("maxPathLength: ") + fs_info.maxPathLength);
    Serial.println((String)("maxOpenFiles: ") + fs_info.maxOpenFiles);
    Serial.println((String)("blockSize: ") + fs_info.blockSize);
    Serial.println((String)("pageSize: ") + fs_info.pageSize);
    Serial.println("/=========================== SPIFFS info end ===========================/");

    return 0;
}

int fs_tools_writeFile(const String& fileRoute, const String& text)
{
    return fs_tools_writeFile(fileRoute.c_str(), text.c_str());
}

int fs_tools_writeFile(const char* fileRoute, const String& text)
{
    return fs_tools_writeFile(fileRoute, text.c_str());
}

int fs_tools_writeFile(const String& fileRoute, const char* text)
{
    return fs_tools_writeFile(fileRoute.c_str(), text);
}

int fs_tools_writeFile(const char* fileRoute, const char* text)
{
    if (!fs_tools_FS_is_begin()) return -1;

    File f = SPIFFS.open(fileRoute, "w");
    f.print(text);
    f.close();

    return 0;
}

int fs_tools_readFile(const String& fileRoute, char* text)
{
    return fs_tools_readFile(fileRoute.c_str(), text);
}

int fs_tools_readFile(const char* fileRoute, char* text)
{
    return fs_tools_readFile(fileRoute, text, 0);
}

int fs_tools_readFile(const char* fileRoute, char* text, size_t length)
{
    if (!fs_tools_FS_is_begin()) return -1;

    if (!SPIFFS.exists(fileRoute)) return -2;

    File f = SPIFFS.open(fileRoute, "r");
    if (length == 0) {
        length = f.size();
    }
    f.close();

    return 0;
}
