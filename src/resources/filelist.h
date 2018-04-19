/****************************************************************************
 * Loadiine resource files.
 * This file is generated automatically.
 * Includes 3 files.
 *
 * NOTE:
 * Any manual modification of this file will be overwriten by the generation.
 ****************************************************************************/
#ifndef _FILELIST_H_
#define _FILELIST_H_

#include <gctypes.h>

typedef struct _RecourceFile
{
	const char *filename;
	const u8   *DefaultFile;
	const u32  &DefaultFileSize;
	u8		   *CustomFile;
	u32		    CustomFileSize;
} RecourceFile;

extern const u8 bmgres_arc[];
extern const u32 bmgres_arc_size;

extern const u8 bmgres1_arc[];
extern const u32 bmgres1_arc_size;

extern const u8 bmgres2_arc[];
extern const u32 bmgres2_arc_size;

static RecourceFile RecourceList[] =
{
	{"bmgres.arc", bmgres_arc, bmgres_arc_size, NULL, 0},
	{"bmgres1.arc", bmgres1_arc, bmgres1_arc_size, NULL, 0},
	{"bmgres2.arc", bmgres2_arc, bmgres2_arc_size, NULL, 0},
	{NULL, NULL, 0, NULL, 0}
};

#endif
