#ifndef RECOURCES_H_
#define RECOURCES_H_


#include <map>

class Resources
{
public:
    static void Clear();
    static bool LoadFiles(const char * path);
    static const u8 * GetFile(const char * filename);
    static u32 GetFileSize(const char * filename);

private:
    static Resources *instance;

    Resources() {}
    ~Resources() {}
};

#endif
