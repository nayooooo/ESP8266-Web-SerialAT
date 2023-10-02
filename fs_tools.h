#ifndef __FS_TOOLS_H___
#define __FS_TOOLS_H___

#include <Arduino.h>
#include <FS.h>

int fs_tools_FS_info(void);

int fs_tools_writeFile(const String& fileRoute, const String& text);
int fs_tools_writeFile(const char* fileRoute, const String& text);
int fs_tools_writeFile(const String& fileRoute, const char* text);
int fs_tools_writeFile(const char* fileRoute, const char* text);

int fs_tools_readFile(const String& fileRoute, char* text);
int fs_tools_readFile(const char* fileRoute, char* text);
int fs_tools_readFile(const char* fileRoute, char* text, size_t length);

#endif  // !__FS_TOOLS_H___
