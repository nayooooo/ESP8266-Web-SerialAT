#include "fs_tools.h"

bool fs_tools_FS_begin(void)
{
    if (SPIFFS.begin()) return true;
    else return false;
}

bool fs_tools_FS_end(void)
{
    SPIFFS.end();
    return true;
}

bool fs_tools_FS_format(void)
{
    SPIFFS.format();
    return true;
}

int fs_tools_FS_info(void)
{
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

int fs_tools_print_directory(const String& path)
{
    return fs_tools_print_directory(path.c_str());
}

int fs_tools_print_directory(const char* path)
{
    Dir dir = SPIFFS.openDir(path);
    Serial.println();
    if (path == "") {
        Serial.println("all file: ");
    } else {
        Serial.println(String(path) + ": ");
    }
    uint32_t fileNum = 0;
    size_t total_f_size = 0;
    while (dir.next()) {
        File f = dir.openFile("r");
        size_t f_size = f.size();
        total_f_size += f_size;
        Serial.println(String("--") + dir.fileName() + "\t\t" + String((float)f_size / 1024) + "KB");
        f.close();
        fileNum++;
    }
    if (fileNum == 0) {
        Serial.println("There are no files here!");
    } else {
        Serial.println(String("Total files number: ") + String(fileNum));
        Serial.println(String("Total files size: ") + String((float)total_f_size / 1024) + "KB");
    }

    return 0;
}

int fs_tools_writeFile(const String& filePath, const String& text)
{
    return fs_tools_writeFile(filePath.c_str(), text.c_str());
}

int fs_tools_writeFile(const char* filePath, const String& text)
{
    return fs_tools_writeFile(filePath, text.c_str());
}

int fs_tools_writeFile(const String& filePath, const char* text)
{
    return fs_tools_writeFile(filePath.c_str(), text);
}

int fs_tools_writeFile(const char* filePath, const char* text)
{
    File f = SPIFFS.open(filePath, "w");
    f.print(text);
    f.close();

    return 0;
}

int fs_tools_readFile(const char* filePath, String& text)
{
    return fs_tools_readFile(String(filePath), text, 0);
}

int fs_tools_readFile(const String& filePath, String& text, size_t length)
{
    if (!SPIFFS.exists(filePath)) return -2;

    File f = SPIFFS.open(filePath, "r");
    if (length == 0) {
        length = f.size();
    }
    text = f.readString();
    f.close();

    return 0;
}

int fs_tools_readFile(const String& filePath, char* text)
{
    return fs_tools_readFile(filePath.c_str(), text, 0);
}

int fs_tools_readFile(const char* filePath, char* text, size_t length)
{
    if (!SPIFFS.exists(filePath)) return -2;

    File f = SPIFFS.open(filePath, "r");
    if (length == 0) {
        length = f.size();
    }
    f.readBytes(text, length);
    f.close();

    return 0;
}

int fs_tools_openFile(const String& filePath, File& f, const char* mode)
{
    return fs_tools_openFile(filePath.c_str(), f, mode);
}

int fs_tools_openFile(const char* filePath, File& f, const char* mode)
{
    if (!SPIFFS.exists(filePath)) return -2;

    f = SPIFFS.open(filePath, mode);

    return 0;
}

int fs_tools_closeFile(File& f)
{
    f.close();
    return 0;
}
