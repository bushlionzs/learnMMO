#include "platform_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time_util.h>
#include <string_util.h>
#include <errno.h>
#ifdef _WIN32
#include <tchar.h>
#endif
bool get_file_content(const char* filename, std::string& content, uint32_t part)
{
    content.clear();
    FILE* fp = fopen(filename, "rb");
    if (NULL == fp)
    {
        return false;
    }

    fseek(fp, 0, SEEK_END);
    uint32_t size = ftell(fp);
    if (size > 1024 * 1024 * 100)
    {
        fclose(fp);
        return false;
    }

    if (part > 0)
    {
        size = std::min(size, part);
    }
    fseek(fp, 0, SEEK_SET);
    char* mem = (char*)malloc(size);
    fread(mem, 1, size, fp);
    content.assign(mem, size);
    free(mem);
    fclose(fp);
    return true;
}

bool save_file_content(const char* filename, const std::string& content)
{
    FILE* fp = fopen(filename, "wb");
    if (NULL == fp)
    {
        WARNING_LOG("[name:%s, err:%d]fail to save content to file", filename, errno);
        return false;
    }

    fwrite(content.c_str(), content.size(), 1, fp);
    fclose(fp);
    return true;
}

bool delete_file(const char* filename)
{
    int ret = remove(filename);
    return ret == 0;
}

bool save_file_content(const char* filename, const char* data, uint32_t size)
{
    FILE* fp = fopen(filename, "wb");
    if (NULL == fp)
    {
        WARNING_LOG("[name:%s, err:%d]fail to save content to file", filename, errno);
        return false;
    }

    fwrite(data, 1, size, fp);
    fclose(fp);
    return true;
}

#ifdef _WIN32

typedef struct _dirdesc {
    int     dd_fd;      /** file descriptor associated with directory */
    long    dd_loc;     /** offset in current buffer */
    long    dd_size;    /** amount of data returned by getdirentries */
    char    *dd_buf;    /** data buffer */
    int     dd_len;     /** size of data buffer */
    long    dd_seek;    /** magic cookie returned by getdirentries */
    HANDLE  w_handle;
} DIR;

# define __dirfd(dp)    ((dp)->dd_fd)

DIR *opendir(const wchar_t *);
struct dirent *readdir(DIR *);
void rewinddir(DIR *);
int closedir(DIR *);
struct dirent
{
    long d_ino;              /* inode number*/
    off_t d_off;             /* offset to this dirent*/
    unsigned short d_reclen; /* length of this d_name*/
    unsigned char d_type;    /* the type of d_name*/
    wchar_t d_name[MAX_PATH];          /* file name (null-terminated)*/
};


DIR *opendir(const char *name)
{
    DIR *dir;
    WIN32_FIND_DATAA FindData;
    char namebuf[1024];

    dir = (DIR *)malloc(sizeof(DIR));
    if (!dir)
    {
        return 0;
    }
    memset(dir, 0, sizeof(DIR));
    dir->dd_fd = 0;

    dir->w_handle = INVALID_HANDLE_VALUE;
    sprintf(namebuf, "%s\\*.*", name);

    dir->w_handle = FindFirstFileA(namebuf, &FindData);
    if (dir->w_handle == INVALID_HANDLE_VALUE)
    {
        free(dir);
        return 0;
    }

    return dir;
}

struct dirent *readdir(DIR *d)
{
    BOOL bf;
    WIN32_FIND_DATAW FileData;
    if (!d)
    {
        return 0;
    }

    bf = FindNextFileW(d->w_handle, &FileData);
    //fail or end  
    if (!bf)
    {
        return 0;
    }

    struct dirent *dir = (struct dirent *)malloc(sizeof(struct dirent));

    wcscpy(dir->d_name, FileData.cFileName);

    dir->d_reclen = wcslen(dir->d_name);

    //check there is file or directory  
    if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        dir->d_type = 2;
    }
    else
    {
        dir->d_type = 1;
    }

    return dir;
}
int closedir(DIR *d)
{
    if (!d)
        return -1;
    ::FindClose(d->w_handle);
    free(d);
    return 0;
}

#define S_ISDIR(mode) (mode & _S_IFDIR)
#endif


void traverse_dir(const char* path, std::vector<std::string>& filelist, uint32_t day)
{
    DIR *d = NULL;
    struct dirent *dp = NULL;
    struct stat st;
    char p[1024] = { 0 };


    uint32_t size = strlen(path);
#ifdef _WIN32
    std::string apath = dy::utf8_to_acsi(path);
#else
    std::string apath = path;
#endif
    if (stat(apath.c_str(), &st) < 0 )
    {
        return;
    }

    if (!(d = opendir(apath.c_str())))
    {
        return;
    }
    std::wstring fullpath;
    while ((dp = readdir(d)) != NULL)
    {
        if (dp->d_name[0] != '.')
      {

        #ifdef _WIN32
        snprintf(p, sizeof(p), "%s%s", apath.c_str(), dy::unicode_to_acsi(dp->d_name).c_str());
        #else
        snprintf(p, sizeof(p), "%s%s", apath.c_str(), dp->d_name);
        #endif
        stat(p, &st);
        if (!S_ISDIR(st.st_mode))
        {
            uint32_t lasttime = get_seconds() - day * 3600 * 24;
            if (st.st_mtime < lasttime)
            {
                filelist.push_back(p);
            }
        }
      }
#ifdef _WIN32
      free(dp);
#endif
    }
    closedir(d);

    return;
}

void traverse_dir(const char* path, std::vector<std::string>& filelist)
{
    DIR* d = NULL;
    struct dirent* dp = NULL;
    struct stat st;
    char p[1024] = { 0 };


    uint32_t size = strlen(path);
#ifdef _WIN32
    std::string apath = dy::utf8_to_acsi(path);
#else
    std::string apath = path;
#endif
    if (stat(apath.c_str(), &st) < 0)
    {
        return;
    }

    if (!(d = opendir(apath.c_str())))
    {
        return;
    }
    std::wstring fullpath;
    while ((dp = readdir(d)) != NULL)
    {
        if (dp->d_name[0] != '.')
        {

#ifdef _WIN32
            snprintf(p, sizeof(p), "%s%s", apath.c_str(), dy::unicode_to_acsi(dp->d_name).c_str());
#else
            snprintf(p, sizeof(p), "%s%s", apath.c_str(), dp->d_name);
#endif
            stat(p, &st);
            if (!S_ISDIR(st.st_mode))
            {
                 filelist.push_back(p);
            }
        }
#ifdef _WIN32
        free(dp);
#endif
    }
    closedir(d);

    return;
}

bool platform_copy_file(const char* srcname, const char* dstname)
{
#ifdef _WIN32
    return ::CopyFile(srcname, dstname, FALSE);
#else
    std::string content;
    if (!get_file_content(srcname, content))
    {
        return false;
    }

    return save_file_content(dstname, content);
#endif
}
