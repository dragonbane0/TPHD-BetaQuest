#include <gctypes.h>
#include <malloc.h>
#include <string.h>
#include "resources/Resources.h"
#include "filelist.h"
#include "fs/fs_utils.h"

Resources * Resources::instance = NULL;

void Resources::Clear()
{
	for(int i = 0; RecourceList[i].filename != NULL; ++i)
	{
		if(RecourceList[i].CustomFile)
		{
			free(RecourceList[i].CustomFile);
			RecourceList[i].CustomFile = NULL;
		}

		if(RecourceList[i].CustomFileSize != 0)
			RecourceList[i].CustomFileSize = 0;
	}

	if(instance)
        delete instance;

    instance = NULL;
}

bool Resources::LoadFiles(const char * path)
{
	if(!path)
		return false;

	bool result = false;
	Clear();

	for(int i = 0; RecourceList[i].filename != NULL; ++i)
	{
        std::string fullpath(path);
        fullpath += "/";
        fullpath += RecourceList[i].filename;

        u8 * buffer = NULL;
        u32 filesize = 0;

        LoadFileToMem(fullpath.c_str(), &buffer, &filesize);

        RecourceList[i].CustomFile = buffer;
        RecourceList[i].CustomFileSize = (u32) filesize;
        result |= (buffer != 0);
	}

	return result;
}

const u8 * Resources::GetFile(const char * filename)
{
	for(int i = 0; RecourceList[i].filename != NULL; ++i)
	{
		if(strcasecmp(filename, RecourceList[i].filename) == 0)
		{
			return (RecourceList[i].CustomFile ? RecourceList[i].CustomFile : RecourceList[i].DefaultFile);
		}
	}

	return NULL;
}

u32 Resources::GetFileSize(const char * filename)
{
	for(int i = 0; RecourceList[i].filename != NULL; ++i)
	{
		if(strcasecmp(filename, RecourceList[i].filename) == 0)
		{
			return (RecourceList[i].CustomFile ? RecourceList[i].CustomFileSize : RecourceList[i].DefaultFileSize);
		}
	}
	return 0;
}