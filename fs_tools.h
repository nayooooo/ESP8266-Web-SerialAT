#ifndef __FS_TOOLS_H___
#define __FS_TOOLS_H___

#include <Arduino.h>
#include <FS.h>

int fs_tools_FS_info(void);

int fs_tools_print_directory(const String& path, int depth = 2);
int fs_tools_print_directory(const char* path, int depth = 2);

int fs_tools_writeFile(const String& filePath, const String& text);
int fs_tools_writeFile(const char* filePath, const String& text);
int fs_tools_writeFile(const String& filePath, const char* text);
int fs_tools_writeFile(const char* filePath, const char* text);

int fs_tools_readFile(const char* filePath, String& text);
int fs_tools_readFile(const String& filePath, String& text, size_t length = 0);
int fs_tools_readFile(const String& filePath, char* text);
int fs_tools_readFile(const char* filePath, char* text, size_t length = 0);

#endif  // !__FS_TOOLS_H___
