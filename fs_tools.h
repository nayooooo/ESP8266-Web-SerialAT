#ifndef __FS_TOOLS_H___
#define __FS_TOOLS_H___

#include <Arduino.h>
#include <FS.h>

bool fs_tools_FS_begin(void);
bool fs_tools_FS_end(void);

int fs_tools_FS_info(void);

int fs_tools_print_directory(const String& path);
int fs_tools_print_directory(const char* path);

int fs_tools_writeFile(const String& filePath, const String& text);
int fs_tools_writeFile(const char* filePath, const String& text);
int fs_tools_writeFile(const String& filePath, const char* text);
int fs_tools_writeFile(const char* filePath, const char* text);

int fs_tools_readFile(const char* filePath, String& text);
int fs_tools_readFile(const String& filePath, String& text, size_t length = 0);
int fs_tools_readFile(const String& filePath, char* text);
int fs_tools_readFile(const char* filePath, char* text, size_t length = 0);

int fs_tools_openFile(const String& filePath, File& f, const char* mode = "r");
int fs_tools_openFile(const char* filePath, File& f, const char* mode = "r");
int fs_tools_closeFile(File& f);

#endif  // !__FS_TOOLS_H___
