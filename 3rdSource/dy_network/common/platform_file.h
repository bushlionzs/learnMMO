#ifndef __PLATFORM_FILE_H__
#define __PLATFORM_FILE_H__
#include <string>
#include <stdint.h>
#include <vector>
#include <string>

bool get_file_content(const char* filename, std::string& content, uint32_t part = 0);
bool get_file_content(const char* filename, std::wstring& content, uint32_t part = 0);
bool save_file_content(const char* filename, const std::string& content);
bool save_file_content(const char* filename, const char* data, uint32_t size);

bool delete_file(const char* filename);
void traverse_dir(const char* path, std::vector<std::string>& filelist, uint32_t day);
void traverse_dir(const char* path, std::vector<std::string>& filelist);

bool platform_copy_file(const char* srcname, const char* dstname);

#endif //__PLATFORM_FILE_H__
