#ifndef __GAME_DEFS_H_
#define __GAME_DEFS_H_

#include "types.h"
#include "assert.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _fopAcM_prm_class fopAcM_prm_class;
typedef struct _stage_ACTR_data_class stage_ACTR_data_class;
typedef struct _stage_SCOB_data_class stage_SCOB_data_class;

struct _fopAcM_prm_class
{
	unsigned int params;

	float xPos;
	float yPos;
	float zPos;

	unsigned short xRot;
	unsigned short yRot;

	unsigned short flag;
	signed short enemy_id;

	unsigned char flags[9];
	unsigned char room_id;

	unsigned char padding[2];
};

struct _stage_ACTR_data_class
{
	unsigned char name[8];
	unsigned int params;

	float xPos;
	float yPos;
	float zPos;

	unsigned short xRot;
	unsigned short yRot;

	unsigned short flag;
	signed short enemy_id;
};

struct _stage_SCOB_data_class
{
	unsigned char name[8];
	unsigned int params;

	float xPos;
	float yPos;
	float zPos;

	unsigned short unk1;
	unsigned short yRot;
	unsigned short unk2;
	unsigned short unk3;

	unsigned char scaleX;
	unsigned char scaleY;
	unsigned char scaleZ;
	unsigned char padding;
};

#ifdef __cplusplus
}
#endif

#endif // __GAME_DEFS_H_