/****************************************************************************
* Copyright (C) 2016 Maschell
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <algorithm>
#include <list>
#include <stdarg.h>
#include <gctypes.h>
#include "function_hooks.h"
#include "dynamic_libs/aoc_functions.h"
#include "dynamic_libs/ax_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/padscore_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/acp_functions.h"
#include "dynamic_libs/syshid_functions.h"
#include "system/exception_handler.h"
#include "kernel/kernel_functions.h"
#include "utils/logger.h"
#include "common/common.h"
#include "common/fs_defs.h"
#include "common/game_defs.h"
#include "main.h"
#include "pygecko.h"
#include "main.h"
#include "fs/fs_utils.h"

#include "resources/Resources.h"

#define LIB_CODE_RW_BASE_OFFSET                         0xC1000000
#define CODE_RW_BASE_OFFSET                             0x00000000
#define DEBUG_LOG_DYN                                   0

#define USE_EXTRA_LOG_FUNCTIONS   0

#define DECL(res, name, ...) \
        res (* real_ ## name)(__VA_ARGS__) __attribute__((section(".data"))); \
        res my_ ## name(__VA_ARGS__)


static u8 input_actionStep = 0;
static u8 input_actionID = 0;
static int input_holdDuration = 0;

static u8 input_actionStepKPAD = 0;
static u8 input_actionIDKPAD = 0;
static int input_holdDurationKPAD = 0;

static u8 ExceptionHooksCore1 = 0;

typedef struct _resList
{
	int fileHandle;
	std::string resName;
	void *pClient;
	void *pCmd;
	int lastReadCount;
}resList;

struct Spawn
{
	char Stage[8];
	u8 RoomID;
	u8 SpawnID;
	u8 EntranceType;
	u16 Modifier;
};

typedef std::array<Spawn, 1117> SpawnArray; //1117

#pragma region AllSpawns
static SpawnArray AllSpawns = { "D_MN10", 9, 0x01, 0x00, 0xF01F,
"F_SP121", 0, 0x0A, 0x00, 0xF01F,
"F_SP122", 8, 0x07, 0x00, 0xF01F,
"D_MN09", 1, 0x03, 0xA0, 0xF01F,
"D_MN09A", 51, 0x7A, 0x00, 0xF01F,
"F_SP125", 4, 0x38, 0x00, 0xF03B,
"F_SP108", 1, 0x06, 0x00, 0xF01F,
"R_SP107", 0, 0x00, 0x00, 0xF03F,
"D_MN09", 1, 0x00, 0xA0, 0xF011,
"D_MN07", 13, 0x00, 0x00, 0xF01F,
"D_MN11", 0, 0x02, 0xA0, 0xF01F,
"D_MN10", 2, 0x00, 0x00, 0xF01F,
"D_MN07", 4, 0x00, 0x00, 0xF01F,
"D_MN08C", 52, 0x02, 0x00, 0xF01F,
"F_SP124", 0, 0x05, 0x00, 0xF01F,
"F_SP126", 0, 0x63, 0x00, 0xF01F,
"F_SP111", 0, 0x6F, 0xC0, 0xF01F,
"F_SP115", 0, 0x0A, 0x00, 0xF01F,
"F_SP103", 0, 0x09, 0xA0, 0xF09F,
"D_MN11", 4, 0x09, 0xB0, 0xF01F,
//"F_SP109", 0, 0x0C, 0x00, 0xF01F, //Bomb House Destroyed CS
"F_SP121", 12, 0x00, 0x50, 0xF01F,
"F_SP115", 0, 0x04, 0x90, 0xF01F,
"F_SP103", 1, 0x05, 0x00, 0xF09F, //Save post Goats 1
"D_MN09", 1, 0x02, 0xA0, 0xF01F,
"R_SP116", 5, 0x1E, 0x10, 0xF05F,
//"F_SP121", 0, 0x0E, 0x00, 0xF01F, //Fortune Teller
"D_MN06A", 50, 0x00, 0x60, 0xF01F,
"F_SP122", 16, 0x01, 0x50, 0xF09F,
"R_SP160", 3, 0x01, 0xB0, 0xF09F,
"D_MN05", 4, 0x01, 0x70, 0xF01F,
"F_SP121", 3, 0x01, 0x80, 0xF01F,
"D_SB02", 0, 0x00, 0x00, 0xF01F,
"F_SP121", 3, 0x63, 0x20, 0xF01F,
"D_SB03", 0, 0x00, 0x50, 0xF01F,
"D_MN11A", 50, 0x01, 0x60, 0xF01F,
"F_SP121", 14, 0x01, 0x00, 0xF01F,
//"R_SP109", 0, 0x15, 0x00, 0xF019, //Horsecall CS 2
"R_SP01", 5, 0x02, 0x00, 0xF01F,
"D_MN10", 6, 0x01, 0x00, 0xF01F,
"F_SP121", 0, 0x05, 0x00, 0xF01F,
"F_SP108", 5, 0x01, 0x50, 0xF09F,
"F_SP121", 13, 0x00, 0x20, 0xF01F,
"F_SP108", 0, 0x14, 0x00, 0xF038,
"D_MN01A", 50, 0x02, 0xD0, 0x00FF,
"D_MN08D", 55, 0x01, 0xD0, 0x00FF,
"F_SP111", 00, 0x02, 0xD0, 0xF01F,
"F_SP111", 00, 0x03, 0xD0, 0x00FF,
"F_SP113", 00, 0x07, 0xD0, 0x00FF,
"F_SP113", 00, 0x08, 0xE0, 0x00FF,
"F_SP113", 00, 0x0A, 0xD0, 0xF01F,
"F_SP113", 01, 0x09, 0xD0, 0x00FF,
"F_SP113", 01, 0x03, 0xE0, 0x00FF,
"F_SP113", 01, 0x0B, 0xD0, 0x00FF,
"F_SP113", 01, 0x0C, 0xD0, 0x00FF,
"F_SP113", 01, 0x0D, 0xE0, 0x00FF,
"F_SP115", 00, 0x19, 0xD0, 0x00FF,
"F_SP121", 03, 0x06, 0xE0, 0xF01F,
"F_SP121", 10, 0x02, 0xD0, 0x00FF,
"F_SP126", 00, 0x02, 0xE0, 0x00FF,
"F_SP126", 00, 0x03, 0xD0, 0x00FF,
"F_SP126", 00, 0x06, 0xD0, 0x00FF,
"F_SP126", 00, 0x08, 0xD0, 0x00FF,
"F_SP108", 6, 0x02, 0x00, 0xF09F,
"F_SP121", 6, 0x15, 0x00, 0xF01F,
"F_SP113", 0, 0x00, 0x00, 0xF01F,
"D_SB01", 28, 0x00, 0x00, 0xF01F,
"F_SP103", 1, 0x17, 0x00, 0xF014,
"F_SP114", 0, 0x00, 0x50, 0xF09F,
"D_MN10", 4, 0x02, 0x00, 0xF01F,
"F_SP104", 1, 0x05, 0x00, 0xF01F,
"D_MN11", 4, 0x08, 0xA0, 0xF01F,
"F_SP117", 2, 0x01, 0x50, 0xF01F,
"F_SP128", 0, 0x02, 0x00, 0xF01F,
"F_SP104", 1, 0x19, 0x00, 0xF00A,
"D_MN07", 12, 0x02, 0x70, 0xF01F,
//"F_SP00", 0, 0x03, 0x20, 0xF012, //Goats Spawn
"D_MN05", 4, 0x02, 0x00, 0xF01F,
"D_MN11", 6, 0x01, 0xA0, 0xF01F,
"D_MN08", 9, 0x14, 0x00, 0xF01F,
"F_SP103", 0, 0x67, 0x50, 0x5011,
"D_MN07", 12, 0x00, 0x00, 0xF01F,
"D_MN10", 0, 0x01, 0x00, 0xF01F,
"D_MN07", 15, 0x01, 0x70, 0xF01F,
"F_SP109", 0, 0x09, 0x00, 0xF09F,
"F_SP200", 0, 0x06, 0x00, 0xF010,
"D_SB10", 0, 0x14, 0x50, 0xF01F,
"F_SP122", 8, 0x65, 0x00, 0xF01F,
"F_SP110", 3, 0x05, 0x00, 0xF01F,
"D_MN07", 4, 0x02, 0x70, 0xF01F,
"F_SP115", 0, 0x1E, 0x00, 0xF01F,
"F_SP121", 0, 0x0C, 0x90, 0xF01F,
"D_MN10", 4, 0x01, 0x00, 0xF01F,
"F_SP109", 0, 0x44, 0x00, 0x6014,
"D_MN07", 3, 0x00, 0x00, 0xF01F,
"R_SP300", 0, 0x00, 0x00, 0xF01F,
"D_MN04", 1, 0x00, 0x00, 0xF01F,
"R_SP109", 5, 0x00, 0xB0, 0xF01F,
"D_MN01", 6, 0x02, 0x00, 0xF01F,
"F_SP00", 0, 0x63, 0x20, 0xF01F,
"F_SP104", 1, 0x63, 0x20, 0xF01F,
"F_SP115", 0, 0x37, 0x00, 0xF01F,
"D_MN11", 3, 0x09, 0xA0, 0xF01F,
"F_SP121", 6, 0x00, 0x50, 0xF09F,
"F_SP113", 1, 0x08, 0x50, 0xF09F,
"F_SP109", 0, 0x0A, 0x10, 0xF09F,
"F_SP115", 0, 0x0F, 0xC0, 0xF01F,
"F_SP126", 0, 0x0B, 0xC0, 0xF01F,
"R_SP01", 0, 0x00, 0xA0, 0xF09F,
//"D_MN10", 13, 0x01, 0x00, 0xF01F, //Fortune Teller
"D_MN05", 11, 0x00, 0x00, 0xF01F,
"R_SP107", 2, 0x14, 0x00, 0xF018,
"F_SP115", 0, 0x63, 0x90, 0xF01F,
"D_MN08C", 52, 0x00, 0x60, 0xF01F,
"D_SB10", 0, 0x15, 0x50, 0xF01F,
"F_SP103", 1, 0x63, 0x20, 0xF01F,
"F_SP114", 1, 0x14, 0x00, 0xF01F,
"F_SP103", 0, 0x02, 0x00, 0xF01F,
"R_SP160", 5, 0x04, 0x00, 0xF01F,
"F_SP128", 0, 0x03, 0x00, 0xF01F,
"D_SB01", 0, 0x00, 0x50, 0xF01F,
"F_SP109", 0, 0x0E, 0x00, 0xF01F,
"F_SP126", 0, 0x64, 0x00, 0xF01F,
"F_SP111", 0, 0x06, 0x00, 0xF01F,
"F_SP113", 0, 0x01, 0x50, 0xF09F,
"F_SP118", 2, 0x00, 0x00, 0xF01F,
"D_MN08", 0, 0x02, 0x70, 0xF01F,
"D_MN10", 0, 0x03, 0x00, 0xF01F,
"F_SP124", 0, 0x08, 0x90, 0xF01F,
"F_SP108", 4, 0x02, 0x20, 0xF01F,
"F_SP128", 0, 0x05, 0x00, 0xF011,
"F_SP108", 0, 0x18, 0x00, 0xF017,
"F_SP115", 0, 0x08, 0xB0, 0xF09F,
"F_SP200", 0, 0x04, 0x00, 0xF010,
"F_SP116", 4, 0x05, 0x00, 0xF05F,
"F_SP121", 10, 0x04, 0x50, 0xF09F,
"F_SP125", 4, 0x00, 0x10, 0xF01F,
"F_SP108", 0, 0xFE, 0x00, 0xF01F,
"F_SP109", 0, 0x28, 0xA0, 0xF09F,
"D_MN07", 4, 0x01, 0x70, 0xF01F,
//"D_SB01", 24, 0x00, 0x00, 0xF01F,
"F_SP108", 6, 0x32, 0xC0, 0xF010,
"R_SP109", 0, 0x05, 0xB0, 0xF09F,
"F_SP102", 0, 0x04, 0x00, 0xF01F,
//"F_SP115", 0, 0x10, 0x00, 0xF01F, //Outer Zoras Domain cleared from high water cs
"F_SP115", 0, 0x0B, 0xD0, 0xF01F,
"D_MN08D", 60, 0x00, 0x00, 0xF01F,
"D_MN01", 9, 0x02, 0x70, 0xF01F,
"D_MN01B", 51, 0x02, 0x70, 0xF01F,
"D_MN06", 1, 0x00, 0x00, 0xF01F,
"F_SP103", 1, 0x19, 0x00, 0xF01F,
"F_SP122", 16, 0x00, 0x50, 0xF09F,
"D_MN01", 9, 0x03, 0x00, 0xF01F,
"F_SP109", 0, 0x3B, 0x00, 0xF01F,
"D_MN09A", 50, 0x15, 0x00, 0xF019,
"R_SP109", 4, 0x01, 0x50, 0xF01F,
"F_SP121", 10, 0x05, 0x00, 0xF01F,
"D_MN07", 0, 0x03, 0x00, 0xF01F,
"D_MN11", 4, 0x04, 0x00, 0xF01F,
"D_MN04", 12, 0x00, 0x00, 0xF01F,
"F_SP108", 5, 0x00, 0x50, 0xF09F,
"F_SP109", 0, 0x00, 0x50, 0xF01F,
"D_MN08", 0, 0x15, 0x00, 0xF01F,
"F_SP108", 5, 0x62, 0x50, 0xF09F,
"D_SB11", 36, 0x00, 0x00, 0xF01F,
"D_MN05", 5, 0x00, 0x00, 0xF01F,
//"D_SB11", 24, 0x00, 0x00, 0xF01F,
"F_SP121", 7, 0x02, 0x90, 0xF01F,
"F_SP108", 4, 0x09, 0x00, 0xF05F,
"F_SP112", 1, 0x0D, 0x00, 0xF01F,
"F_SP113", 1, 0x64, 0x50, 0xF01F,
"D_MN11", 3, 0x07, 0xA0, 0xF01F,
"F_SP200", 0, 0x05, 0x00, 0xF010,
//"R_SP160", 3, 0x02, 0x00, 0xF01F, //Fortune Teller
"F_SP115", 0, 0x1F, 0x00, 0xF01F,
"F_SP121", 13, 0x63, 0x20, 0xF012,
"F_SP113", 1, 0x07, 0x00, 0xF01F,
"F_SP109", 0, 0x40, 0x00, 0xF01F,
"R_SP109", 0, 0x08, 0xB0, 0xF09F,
"F_SP125", 4, 0x03, 0x00, 0xF01F,
"F_SP115", 0, 0x22, 0x00, 0xF01F,
"R_SP127", 0, 0x01, 0x00, 0xF05F,
//"R_SP109", 2, 0x03, 0x00, 0x603D, //Escort End
"D_SB00", 0, 0x00, 0x00, 0xF09F,
"D_MN09", 2, 0x03, 0xA0, 0xF01F,
"D_MN07A", 50, 0x01, 0x60, 0xF01F,
"F_SP109", 0, 0x23, 0x20, 0xF031,
"D_MN09A", 51, 0x78, 0x00, 0xF01F,
"F_SP121", 9, 0x02, 0x00, 0xF01F,
"F_SP121", 3, 0x03, 0x00, 0xF01F,
"D_MN08", 0, 0x00, 0x00, 0xF01F,
"D_MN11", 4, 0x05, 0x60, 0xF01F,
//"D_SB01", 34, 0x00, 0x00, 0xF01F,
"D_MN05", 2, 0x00, 0x00, 0xF01F,
"F_SP114", 0, 0x0A, 0x00, 0xF01F,
"F_SP121", 15, 0x16, 0x00, 0xF01F,
"F_SP108", 6, 0x00, 0x00, 0xF014,
"F_SP109", 0, 0x39, 0x00, 0xF01F,
"D_MN07", 3, 0x03, 0x70, 0xF01F,
"F_SP116", 0, 0x03, 0x50, 0xF09F,
"F_SP108", 5, 0x0A, 0x00, 0xF01F,
"F_SP103", 0, 0x00, 0x50, 0xF09F,
"F_SP117", 1, 0x06, 0x50, 0xF01F,
"F_SP117", 3, 0x00, 0x50, 0xF09F,
"D_MN09", 15, 0x03, 0xA0, 0xF011,
"D_SB01", 19, 0x00, 0x00, 0xF012,
"R_SP109", 1, 0x02, 0xB0, 0xF09F,
"R_SP160", 4, 0x02, 0xA0, 0xF09F,
"R_SP01", 2, 0x03, 0x00, 0xF01F,
"F_SP103", 0, 0x0F, 0x40, 0xF03F,
"D_MN01", 0, 0x00, 0x00, 0xF09F,
"D_SB11", 19, 0x00, 0x50, 0xF01F,
"F_SP121", 6, 0x0A, 0x20, 0xF01F,
"F_SP103", 1, 0x07, 0x00, 0xF01F,
"D_SB10", 0, 0x00, 0x50, 0xF09F,
"F_SP109", 0, 0x33, 0xA0, 0xF09F,
"F_SP112", 1, 0x0A, 0x00, 0xF01F,
"F_SP124", 0, 0x09, 0x90, 0xF01F,
"F_SP108", 0, 0x04, 0x00, 0xF01F,
"D_SB11", 7, 0x00, 0x00, 0xF01F,
"D_MN04", 7, 0x00, 0x00, 0xF01F,
//"D_SB01", 18, 0x01, 0x10, 0xF01F,
"R_SP300", 0, 0x78, 0x00, 0xF01F,
"F_SP121", 0, 0x04, 0x00, 0xF01F,
"F_SP113", 1, 0x14, 0x00, 0xF01F,
"F_SP108", 3, 0x00, 0x00, 0xF01F,
"D_MN07", 16, 0x01, 0x70, 0xF01F,
"D_SB11", 25, 0x00, 0x00, 0xF01F,
"F_SP108", 5, 0x3C, 0x00, 0xF01F,
"F_SP124", 0, 0x04, 0x00, 0xF01F,
"F_SP104", 1, 0x1A, 0x00, 0xF031,
"D_MN09A", 51, 0x15, 0x00, 0xF01F,
"F_SP109", 0, 0x25, 0x00, 0x1032,
"F_SP103", 0, 0x04, 0xB0, 0xF09F,
"D_MN04B", 51, 0x01, 0x60, 0xF01F,
"F_SP103", 0, 0x01, 0x50, 0xF09F,
"D_SB11", 30, 0x00, 0x00, 0xF01F,
"D_MN07B", 51, 0x02, 0x00, 0xF03F,
"F_SP126", 0, 0x09, 0x00, 0xF01F,
"D_MN11", 3, 0x0C, 0xB0, 0xF01F,
"F_SP110", 0, 0x64, 0xC0, 0xF01F,
"D_SB03", 0, 0x01, 0x00, 0xF01F,
"D_MN06", 6, 0x00, 0x00, 0xF01F,
"F_SP103", 1, 0x04, 0x00, 0xF09F,
"F_SP121", 4, 0x01, 0x00, 0xF01F,
"F_SP109", 0, 0x15, 0x00, 0xF01F,
"R_SP01", 4, 0x00, 0xB0, 0xF09F,
"D_MN07", 1, 0x00, 0x00, 0xF01F,
//"D_MN11", 0, 0x03, 0x00, 0xF01F, //Fortune Teller
"F_SP00", 0, 0x04, 0x20, 0xF016, //Post Goats Spawn
"F_SP116", 0, 0x0B, 0x50, 0xF01F,
"D_MN04", 13, 0x00, 0x00, 0xF01F,
"F_SP121", 15, 0x63, 0x20, 0xF01F,
"D_MN01", 2, 0x00, 0x00, 0xF01F,
"D_SB09", 4, 0x00, 0x00, 0xF010,
"F_SP103", 1, 0x09, 0x00, 0xF015,
"R_SP160", 1, 0x00, 0x10, 0xF09F,
"D_MN11A", 50, 0x02, 0x00, 0xF03F,
"D_MN04", 3, 0x00, 0x00, 0xF01F,
"F_SP00", 0, 0x02, 0x00, 0xF01F,
"F_SP118", 3, 0x00, 0x50, 0xF01F,
"F_SP115", 0, 0x09, 0xC0, 0xF01F,
"D_MN10", 11, 0x02, 0x70, 0xF01F,
"R_SP160", 3, 0x00, 0x00, 0xF09F,
"F_SP103", 1, 0x0B, 0x00, 0xF01F,
"F_SP124", 0, 0x00, 0x00, 0xF018,
"F_SP109", 0, 0x2E, 0xA0, 0xF09F,
"R_SP107", 2, 0x02, 0x50, 0xF01F,
"R_SP116", 5, 0x02, 0x10, 0xF092,
//"D_SB01", 25, 0x00, 0x00, 0xF01F,
"D_SB11", 48, 0x00, 0x00, 0xF01F,
//"F_SP121", 0, 0x12, 0x00, 0xF01F, //Fortune Teller
"F_SP114", 0, 0x0D, 0x00, 0xF01F,
//"D_SB01", 2, 0x00, 0x00, 0xF01F,
"D_MN01", 3, 0x00, 0x00, 0xF01F,
"D_MN09", 12, 0x00, 0x00, 0xF01F,
"F_SP115", 0, 0x4E, 0x00, 0xF01F,
"F_SP108", 1, 0x01, 0x80, 0xC00F,
"F_SP124", 0, 0x06, 0x50, 0xF09F,
"R_SP116", 5, 0x05, 0x50, 0xF012,
"D_MN10", 11, 0x03, 0x00, 0xF01F,
"R_SP107", 0, 0x16, 0x00, 0xF037,
"F_SP103", 1, 0x1E, 0x00, 0x2019,
"F_SP114", 1, 0x0C, 0x00, 0xF01F,
"F_SP121", 11, 0x01, 0x00, 0xF01F,
"D_MN07", 5, 0x02, 0x00, 0xF01F,
//"F_SP121", 0, 0x08, 0x00, 0xF016, //Hotspring Minigame
"D_MN08", 8, 0x00, 0x70, 0xF01F,
"D_MN07", 10, 0x03, 0x00, 0xF01F,
"R_SP160", 0, 0x00, 0xB0, 0xF09F,
"D_MN04", 5, 0x00, 0x00, 0xF01F,
"D_SB01", 10, 0x00, 0x00, 0xF01F,
"F_SP109", 0, 0x18, 0x00, 0xF01F,
"F_SP109", 0, 0x12, 0x00, 0xF01F,
"D_MN09", 12, 0x04, 0x00, 0xF01F,
//"D_SB01", 31, 0x00, 0x00, 0xF01F,
"F_SP109", 0, 0x45, 0x00, 0xF01F,
"D_MN10", 1, 0x00, 0x00, 0xF01F,
"D_MN07", 2, 0x04, 0x70, 0xF01F,
"R_SP209", 7, 0x02, 0x00, 0xF01F,
//"R_SP160", 1, 0x02, 0x00, 0xF01F, //Fortune Teller Return Warp
//"D_MN09", 12, 0x03, 0x00, 0xF01F, //Fortune Teller
"R_SP109", 4, 0x00, 0xB0, 0xF09F,
"D_MN10", 9, 0x02, 0x50, 0xF01F,
"R_SP107", 2, 0x00, 0x00, 0xF09F,
"D_MN08", 0, 0x04, 0x80, 0xF01F,
"F_SP118", 1, 0x01, 0x00, 0xF01F,
"D_MN11", 3, 0x0B, 0xA0, 0xF01F,
//"F_SP102", 0, 0x00, 0x20, 0xE080, //KB1 Phase 1 Fight Spawn (normal for gameplay)
"F_SP121", 12, 0x14, 0x00, 0xF01F,
"F_SP116", 4, 0x03, 0xA0, 0xF09F,
"D_MN08", 7, 0x01, 0x70, 0xF01F,
//"F_SP121", 7, 0x0E, 0x00, 0xF01F, //Broken Fortune Teller Spawn
//"D_SB11", 45, 0x00, 0x00, 0xF01F,
"F_SP122", 8, 0x01, 0x50, 0xF09F,
"F_SP103", 1, 0x03, 0xA0, 0xF09F,
"F_SP115", 0, 0x00, 0x00, 0xF01F,
"D_MN09", 3, 0x00, 0x00, 0xF01F,
"F_SP108", 14, 0x00, 0x50, 0xF09F,
//"D_SB01", 38, 0x01, 0x10, 0xF01F,
"D_MN10", 4, 0x03, 0x00, 0xF01F,
//"F_SP121", 3, 0x0E, 0x00, 0xF01F,
"D_MN06", 4, 0x00, 0x00, 0xF01F,
"F_SP125", 4, 0x06, 0x10, 0x501F,
"D_SB01", 40, 0x00, 0x00, 0xF01F,
"F_SP103", 0, 0x18, 0x50, 0xF01F,
"R_SP110", 0, 0x00, 0x50, 0xF09F,
//"F_SP121", 2, 0x0A, 0x00, 0xF01E, //Eldin Twilight Barrier
//"D_SB01", 41, 0x00, 0x00, 0xF01F,
//"F_SP126", 0, 0x01, 0x00, 0xF03F, //UZR Post Kagarok Spawn
"D_SB01", 46, 0x00, 0x00, 0xF01F,
"F_SP108", 5, 0x64, 0x00, 0xF01F,
"R_SP107", 0, 0x17, 0x00, 0xF03A,
"D_MN07", 16, 0x00, 0x00, 0xF01F,
"D_SB01", 9, 0x00, 0x00, 0xF011,
"F_SP113", 1, 0x0F, 0x50, 0xF09F,
"D_MN08D", 50, 0x14, 0x00, 0xF01F,
"F_SP114", 2, 0x0C, 0x50, 0xF01F,
"F_SP127", 0, 0x03, 0xB0, 0xF01F,
"F_SP109", 0, 0x1F, 0x00, 0xF01F,
"D_MN11", 4, 0x01, 0xA0, 0xF01F,
"D_SB11", 8, 0x00, 0x00, 0xF01F,
"D_MN01", 1, 0x00, 0x00, 0xF01F,
"F_SP102", 0, 0x05, 0x00, 0xF01F,
"F_SP121", 9, 0x01, 0x00, 0xF01F,
"D_MN08", 11, 0x00, 0x70, 0xF01F,
"D_MN07", 6, 0x03, 0x70, 0xF01F,
"D_MN01", 8, 0x00, 0x00, 0xF01F,
"F_SP121", 2, 0x14, 0x00, 0xF01F,
"F_SP121", 0, 0x01, 0x50, 0xF09F,
"F_SP117", 1, 0x66, 0x00, 0xF01F,
"F_SP103", 0, 0x0B, 0xA0, 0xF09F,
"F_SP121", 0, 0x03, 0x80, 0xF01F,
"F_SP122", 17, 0x04, 0x00, 0xF01F,
"F_SP122", 8, 0x6F, 0xC0, 0xF01F,
"F_SP104", 1, 0xC8, 0x00, 0xF01F,
"F_SP110", 3, 0x01, 0x10, 0xF09F,
"D_MN01A", 50, 0x01, 0x30, 0xF01F,
"R_SP107", 1, 0x00, 0x50, 0xF09F,
"F_SP121", 1, 0x00, 0x20, 0xF01F,
"D_MN06", 8, 0x00, 0x00, 0xF01F,
"D_MN09A", 51, 0x0A, 0x60, 0xF01F,
"R_SP109", 2, 0x02, 0xA0, 0xF09F,
"F_SP109", 0, 0x11, 0x00, 0xF01F,
"F_SP117", 3, 0x03, 0x00, 0xF01F,
"F_SP108", 0, 0x16, 0x00, 0xF01F,
"F_SP112", 1, 0x02, 0x00, 0xF010,
"F_SP121", 15, 0x15, 0x00, 0xF01F,
"D_SB11", 18, 0x00, 0x00, 0xF01F,
"F_SP108", 4, 0x08, 0x50, 0xF09F,
"R_SP107", 1, 0x07, 0x00, 0xF01F,
"D_MN11", 8, 0x00, 0x00, 0xF01F,
"F_SP109", 0, 0x38, 0x00, 0xF01F,
"R_SP161", 7, 0x01, 0x00, 0xF090,
"D_MN09", 6, 0x00, 0xA0, 0xF011,
"D_MN08", 0, 0x01, 0x70, 0xF01F,
//"F_SP121", 10, 0x0E, 0x00, 0xF01F, //Fortune Teller
"F_SP122", 8, 0x4C, 0x00, 0xF01F,
//"D_SB11", 5, 0x00, 0x00, 0xF01F,
"F_SP109", 0, 0x04, 0x90, 0xF03F,
"F_SP115", 0, 0x28, 0x00, 0x9003,
"F_SP116", 3, 0x0D, 0x00, 0xF01F,
"D_MN11A", 50, 0x00, 0x00, 0xF01F,
"F_SP114", 1, 0x0A, 0xB0, 0xF01F,
"D_MN07", 5, 0x00, 0x00, 0xF01F,
"R_SP109", 0, 0x03, 0x00, 0xF01E,
"F_SP121", 7, 0x00, 0x20, 0xF01F,
//"D_SB11", 4, 0x00, 0x00, 0xF01F,
"F_SP103", 0, 0x0E, 0x00, 0xF01F,
"D_MN04B", 51, 0x02, 0x00, 0xF01F,
"F_SP110", 3, 0x03, 0xE0, 0xF01F,
"F_SP108", 0, 0x19, 0x00, 0xE00F,
"F_SP118", 3, 0x04, 0x00, 0xF01F,
"F_SP103", 0, 0x16, 0x00, 0xF01F,
"D_MN01B", 51, 0x00, 0x00, 0xF01F,
"D_MN11B", 49, 0x00, 0x00, 0xF01F,
"F_SP104", 1, 0x15, 0x00, 0xF01B,
"F_SP116", 3, 0x0A, 0x50, 0xF01F,
"D_MN11", 3, 0x04, 0xB0, 0xF01F,
"D_MN05", 22, 0x00, 0x00, 0xF09F,
"F_SP109", 0, 0x41, 0x00, 0xF01F,
"R_SP160", 1, 0x01, 0x00, 0xF01F,
"F_SP104", 1, 0x06, 0x50, 0xF01F,
"D_MN08", 0, 0x16, 0x00, 0xF018,
"D_MN09", 4, 0x02, 0xA0, 0xF01F,
"F_SP117", 3, 0x02, 0x50, 0xF01F,
"D_MN07", 6, 0x06, 0x70, 0xF01F,
"F_SP121", 3, 0x02, 0x00, 0xF01F,
"F_SP121", 6, 0x0B, 0x00, 0xF01F,
"F_SP121", 10, 0x16, 0x00, 0xF01C,
"F_SP116", 2, 0x01, 0x10, 0xF01F,
"D_MN01A", 50, 0x00, 0x00, 0xF01F,
"R_SP209", 7, 0x00, 0x00, 0xF03F,
//"D_MN11", 7, 0x0A, 0x00, 0xF01F, //Fortune Teller
"D_MN07", 3, 0x01, 0x70, 0xF01F,
"D_SB05", 0, 0x01, 0xC0, 0xF013,
"R_SP160", 0, 0x02, 0x00, 0xF01F,
"F_SP114", 0, 0x02, 0x90, 0xF01F,
"F_SP103", 1, 0x01, 0x10, 0x203F,
//"F_SP121", 10, 0x10, 0x00, 0xF01F, //Fortune Teller
"F_SP117", 1, 0x96, 0x00, 0xF010,
"F_SP109", 0, 0x32, 0xB0, 0xF09F,
"F_SP116", 4, 0x06, 0x50, 0xF09F,
"F_SP109", 0, 0x31, 0xB0, 0xF09F,
"F_SP108", 1, 0x02, 0x00, 0xF01F,
"D_MN10", 11, 0x00, 0x00, 0xF01F,
"D_MN08", 10, 0x01, 0x00, 0xF01F,
//"F_SP104", 1, 0x17, 0x00, 0xF03C, //Horsecall CS 1
"F_SP108", 5, 0x04, 0x50, 0xF01F,
//"D_SB11", 1, 0x00, 0x00, 0xF01F,
"D_MN04B", 51, 0x00, 0x60, 0xF01F,
"F_SP103", 0, 0x1E, 0x00, 0xF01F,
"F_SP103", 1, 0x0A, 0x00, 0xF014,
"F_SP121", 6, 0x03, 0x90, 0xF01F,
"R_SP127", 0, 0x00, 0xA0, 0xF01F,
//"D_SB01", 1, 0x00, 0x00, 0xF01F,
"F_SP109", 0, 0x10, 0x00, 0xF01F,
"F_SP121", 0, 0x0D, 0x90, 0xF01F,
"D_MN11", 4, 0x06, 0xA0, 0xF01F,
"F_SP121", 0, 0x09, 0x00, 0xF01F,
//"D_SB11", 32, 0x00, 0x00, 0xF01F,
"D_SB01", 30, 0x00, 0x00, 0xF01F,
"F_SP114", 0, 0x07, 0x50, 0xF01F,
"D_MN07", 13, 0x01, 0x70, 0xF01F,
"F_SP108", 8, 0x03, 0x00, 0xF01F,
"F_SP113", 1, 0x0A, 0x50, 0xF09F,
"D_MN07", 6, 0x08, 0x00, 0xF03F,
"D_SB11", 28, 0x00, 0x00, 0xF01F,
"D_MN04", 16, 0x00, 0x00, 0xF01F,
"F_SP121", 10, 0x0F, 0x50, 0xF01F,
"F_SP00", 0, 0x14, 0x00, 0xF01F,
"D_MN11", 0, 0x01, 0xB0, 0xF01F,
"F_SP112", 1, 0x03, 0x00, 0xF01F,
"R_SP107", 1, 0x02, 0x00, 0xF01F,
"R_SP01", 4, 0x02, 0x00, 0xF01F,
//"D_SB11", 23, 0x00, 0x00, 0xF01F,
"F_SP113", 1, 0x62, 0x00, 0xF018,
"R_SP109", 0, 0x07, 0xA0, 0xF09F,
"F_SP104", 1, 0x00, 0x50, 0xF01F,
"F_SP103", 0, 0x05, 0xA0, 0xF09F,
"D_MN06A", 50, 0x01, 0x10, 0xF01F,
"F_SP121", 3, 0x15, 0x00, 0xF01F,
"F_SP114", 0, 0x05, 0xC0, 0xF01F,
"D_MN01A", 50, 0x03, 0x00, 0xF01F,
"D_SB04", 10, 0x00, 0x00, 0xF09F,
"F_SP113", 1, 0x0E, 0x00, 0xF01F,
"F_SP115", 0, 0x85, 0x00, 0xF01F,
"F_SP109", 0, 0x46, 0x80, 0xC002,
"F_SP121", 0, 0x14, 0x50, 0xF09F,
"F_SP121", 0, 0x07, 0x50, 0xF09F,
//"D_MN04", 4, 0x01, 0x00, 0xF01F, //Fortune Teller
"F_SP111", 0, 0x04, 0x00, 0xF01F,
"F_SP121", 13, 0x17, 0x00, 0xF01F,
"F_SP118", 1, 0x02, 0x50, 0xF01F,
"F_SP121", 3, 0x04, 0x90, 0xF01F,
"D_MN08", 2, 0x00, 0x70, 0xF01F,
"D_MN07", 15, 0x00, 0x00, 0xF01F,
//"D_MN01", 3, 0x02, 0x00, 0xF01F, //Fortune Teller
"D_MN06", 8, 0x02, 0x00, 0xF01F,
"F_SP128", 0, 0x01, 0xA0, 0xF09F,
//"D_SB01", 3, 0x00, 0x00, 0xF01F,
"F_SP111", 0, 0x01, 0x90, 0xF03F,
"D_MN07", 0, 0x02, 0x00, 0xC00F,
"F_SP117", 2, 0x00, 0x00, 0xF012,
"D_MN09", 8, 0x00, 0x00, 0xF01F,
"D_SB06", 1, 0x01, 0xC0, 0xF01F,
"F_SP124", 0, 0x34, 0x00, 0xF01A,
//"D_SB01", 4, 0x00, 0x00, 0xF01F,
//"F_SP127", 0, 0x04, 0x00, 0xF03F, //Henna Boat Game
"R_SP109", 2, 0x01, 0x00, 0xF09F,
"D_MN09A", 50, 0x79, 0x00, 0xF019,
"D_MN11", 7, 0x00, 0x00, 0xF01F,
"F_SP115", 0, 0x4B, 0x00, 0xF01F,
//"F_SP121", 3, 0x11, 0x00, 0xF01F, //Fortune Teller
"D_MN11", 9, 0x00, 0x00, 0xF01F,
"F_SP115", 0, 0x64, 0x00, 0xF01E,
"F_SP121", 3, 0x0F, 0x50, 0xF01F,
"D_MN09", 15, 0x05, 0x00, 0xF01F,
//"F_SP121", 3, 0x10, 0x00, 0xF01F, //Fortune Teller
"D_MN05B", 51, 0x02, 0x00, 0xF03F,
"R_SP109", 6, 0x01, 0x00, 0xF01F,
"R_SP116", 5, 0x14, 0x00, 0xF018,
"F_SP115", 0, 0x86, 0x00, 0xF01F,
"F_SP115", 0, 0x4D, 0x40, 0xC00F,
"R_SP161", 7, 0x03, 0x10, 0xF090,
"D_MN09", 12, 0x02, 0xA0, 0xF011,
"F_SP103", 0, 0x63, 0x20, 0xF01F,
"F_SP127", 0, 0x00, 0x00, 0xF01F,
"D_MN09", 12, 0x06, 0x00, 0xF01F,
"F_SP112", 1, 0x0B, 0x00, 0xF01F,
"D_MN10", 2, 0x01, 0x50, 0xF01F,
//"F_SP112", 1, 0x00, 0x00, 0xF014, //Kagarok Spawn
"R_SP01", 2, 0x02, 0x40, 0xF01F,
"F_SP113", 0, 0x04, 0x50, 0xF09F,
"F_SP122", 8, 0x06, 0x00, 0xF036,
"R_SP107", 1, 0x05, 0x00, 0xF01F,
"F_SP116", 1, 0x32, 0x10, 0xF01F,
"R_SP107", 0, 0x15, 0x00, 0xF01F,
"F_SP121", 10, 0x08, 0x90, 0xF01F,
"D_MN01", 5, 0x01, 0x00, 0xF01F,
"D_MN01", 3, 0x01, 0x00, 0xF01F,
//"D_SB01", 44, 0x00, 0x00, 0xF01F,
"D_MN07", 2, 0x02, 0x70, 0xF01F,
"F_SP121", 13, 0x15, 0x00, 0xF01F,
"D_MN09", 4, 0x01, 0xA0, 0xF011,
"F_SP109", 0, 0x42, 0x00, 0xF01F,
"D_MN01", 7, 0x00, 0x00, 0xF01F,
"F_SP104", 1, 0x03, 0x00, 0xF02F,
"D_MN05", 10, 0x00, 0x00, 0xF01F,
"F_SP109", 0, 0x22, 0x00, 0x009A,
"R_SP109", 3, 0x00, 0x10, 0xF09F,
"F_SP116", 0, 0x0E, 0xA0, 0xF09F,
"F_SP109", 0, 0x20, 0x00, 0xC029,
"R_SP109", 0, 0x00, 0x30, 0xF03F,
"F_SP115", 1, 0x00, 0x50, 0xC000,
"D_MN09A", 51, 0x14, 0x00, 0xF01F,
"F_SP122", 16, 0x03, 0x90, 0xF01F,
"F_SP121", 0, 0x0B, 0x20, 0xF01F,
"F_SP109", 0, 0x24, 0x00, 0x203B,
"F_SP103", 0, 0x0D, 0x00, 0xF01F,
"R_SP160", 2, 0x02, 0x10, 0xF09F,
"D_SB06", 1, 0x00, 0x00, 0xF010,
"F_SP117", 1, 0x14, 0x00, 0xF01F,
"D_MN06", 7, 0x00, 0x00, 0xF01F,
"R_SP01", 0, 0x02, 0x00, 0xF01F,
"F_SP121", 3, 0x16, 0x00, 0xF01F,
"F_SP108", 6, 0x01, 0x50, 0xF01F,
"R_SP109", 2, 0x00, 0xB0, 0xF09F,
"D_MN04A", 50, 0x01, 0x00, 0xF03F,
//"D_SB01", 5, 0x00, 0x00, 0xF01F,
"D_MN11A", 50, 0x03, 0x00, 0xF01F,
"D_MN08", 0, 0x14, 0x00, 0xF019,
"R_SP01", 5, 0x01, 0x50, 0x5010,
"D_MN11", 2, 0x02, 0xB0, 0xF01F,
"D_SB01", 7, 0x00, 0x00, 0xF01F,
"F_SP125", 4, 0x01, 0x00, 0xF01F,
"F_SP116", 1, 0x1E, 0x00, 0xF03F,
"F_SP125", 4, 0x04, 0x00, 0xF01F,
"R_SP209", 7, 0x01, 0x30, 0xF01F,
"D_MN01", 5, 0x00, 0x00, 0xF01F,
"R_SP107", 1, 0x04, 0x30, 0xF01F,
"D_MN10", 8, 0x00, 0x00, 0xF01F,
"F_SP104", 1, 0xFE, 0x00, 0xF01F,
"D_SB11", 38, 0x00, 0x00, 0xF01F,
"D_MN06", 5, 0x00, 0x00, 0xF01F,
"D_MN08D", 50, 0x00, 0x00, 0xF01F,
"F_SP121", 11, 0x00, 0x20, 0xF01F,
"F_SP116", 3, 0x00, 0x10, 0xF09F,
"F_SP113", 1, 0x00, 0x50, 0xF09F,
"R_SP107", 3, 0x16, 0x00, 0xF03A,
"D_MN11", 2, 0x01, 0xA0, 0xF01F,
"F_SP125", 4, 0x33, 0x00, 0xF037,
"F_SP113", 1, 0x22, 0x50, 0xF01F,
"F_SP116", 3, 0x08, 0x50, 0xF09F,
"F_SP126", 0, 0x00, 0x00, 0xF01F,
"F_SP121", 13, 0x01, 0xA0, 0xF09F,
"D_MN11", 2, 0x00, 0x00, 0xF01F,
"F_SP109", 0, 0x07, 0x00, 0xF09F,
"F_SP102", 0, 0x02, 0x00, 0xF01F,
"D_MN10A", 50, 0x03, 0x00, 0xF01F,
"F_SP121", 10, 0x03, 0x00, 0xF09F,
"F_SP121", 13, 0x02, 0x00, 0xF01E,
"F_SP125", 4, 0x34, 0x00, 0xF038,
"F_SP109", 0, 0x34, 0x00, 0xF01F,
"F_SP121", 12, 0x03, 0x00, 0xF01F,
"D_SB11", 47, 0x00, 0x00, 0xF01F,
//"D_SB01", 23, 0x00, 0x00, 0xF01F,
"R_SP161", 7, 0x02, 0x00, 0xF01F,
"D_MN11", 0, 0x00, 0x00, 0xF01F,
"R_SP128", 0, 0x00, 0xB0, 0xF09F,
"F_SP109", 0, 0x29, 0xA0, 0xF09F,
"R_SP300", 0, 0x14, 0x00, 0xF018,
"D_MN07A", 50, 0x00, 0x00, 0xF01F,
"D_SB04", 10, 0x01, 0x50, 0xF09F,
"D_MN11", 5, 0x02, 0xB0, 0xF01F,
"F_SP115", 0, 0x14, 0xC0, 0xF01F,
"D_MN08A", 10, 0x17, 0x00, 0xF01F,
"F_SP115", 0, 0x0D, 0x00, 0xF01F,
"F_SP108", 0, 0x17, 0x00, 0xF07A,
"F_SP109", 0, 0x08, 0x50, 0xF09F,
"D_SB01", 49, 0x00, 0x00, 0xF014, //Final Fairy that softlocks with State 5-8 (use state 4 now)
//"D_SB11", 21, 0x00, 0x00, 0xF01F,
"F_SP108", 0, 0x15, 0x00, 0xF079,
"F_SP115", 0, 0x32, 0xD0, 0xF01F,
"D_MN04", 14, 0x00, 0x00, 0xF01F,
"D_MN10", 0, 0x00, 0x00, 0xF01F,
"D_MN08D", 55, 0x00, 0x00, 0xF01F,
"D_MN04", 11, 0x00, 0x70, 0xF01F,
"F_SP122", 8, 0x04, 0x00, 0xF01F,
"D_MN10", 5, 0x00, 0x00, 0xF01F,
//"D_MN10", 2, 0x03, 0x00, 0xF01F, //Fortune Teller
"D_MN09A", 50, 0x01, 0x00, 0xF010,
"F_SP116", 4, 0x04, 0xB0, 0xF09F,
"R_SP107", 1, 0x03, 0x00, 0xF01F,
"F_SP116", 0, 0x06, 0x50, 0xF09F,
"D_MN11B", 51, 0x02, 0xB0, 0xF01F,
"D_MN06", 2, 0x00, 0x00, 0xF01F,
"R_SP01", 2, 0x00, 0xB0, 0xF09F,
"D_MN11", 5, 0x01, 0xA0, 0xF01F,
//"D_MN07", 12, 0x03, 0x00, 0xF01F, //Fortune Teller
//"D_SB01", 33, 0x00, 0x00, 0xF01F,
"F_SP108", 6, 0x64, 0xC0, 0xF01F,
"F_SP114", 1, 0x01, 0x00, 0xF01F,
//"D_SB11", 41, 0x00, 0x00, 0xF01F,
"D_MN07", 0, 0x01, 0x70, 0xF01F,
//"F_SP115", 0, 0x02, 0x00, 0xF01F, //Lake Hylia Boat Spawn
"D_MN11", 13, 0x00, 0x00, 0xF01F,
"D_MN10", 13, 0x00, 0x00, 0xF01F,
"F_SP118", 3, 0x03, 0x50, 0xF01F,
//"D_MN07", 11, 0x01, 0x00, 0xF01F, //Fortune Teller
"D_MN09C", 0, 0x00, 0x00, 0xF01F,
"D_SB11", 38, 0x01, 0x10, 0xF01F,
"D_MN07", 14, 0x03, 0x70, 0xF01F,
"D_MN05", 7, 0x00, 0x00, 0xF01F,
"F_SP110", 1, 0x00, 0x00, 0xF01F,
"F_SP122", 8, 0x64, 0x00, 0x6009,
"F_SP124", 0, 0x07, 0x00, 0xF01F,
"F_SP102", 0, 0x03, 0x20, 0xF01F,
"F_SP121", 5, 0x00, 0x20, 0xF01F,
"D_MN05", 4, 0x00, 0x00, 0xF01F,
"F_SP109", 0, 0x06, 0x50, 0xF09F,
"D_MN08B", 51, 0x01, 0x00, 0xF01F,
"D_MN05", 3, 0x00, 0x00, 0xF01F,
"D_SB11", 34, 0x00, 0x00, 0xF01F,
"F_SP116", 2, 0x02, 0x50, 0xF01F,
"D_SB08", 3, 0x01, 0xC0, 0xF013,
"D_MN04", 12, 0x01, 0x00, 0xF01F,
"F_SP115", 0, 0x03, 0x90, 0xF03F,
//"D_SB01", 13, 0x00, 0x00, 0xF01F,
"F_SP126", 0, 0x0A, 0xA0, 0xF01F,
"D_SB01", 29, 0x00, 0x00, 0xF013,
"F_SP114", 0, 0x0E, 0x00, 0xF01F,
"F_SP116", 1, 0x28, 0x00, 0xF03F,
"D_MN10", 12, 0x00, 0x00, 0xF01F,
"D_MN06", 2, 0x02, 0x00, 0xF01F,
"D_MN01", 13, 0x00, 0x00, 0xF01F,
"D_MN01", 10, 0x00, 0x00, 0xF01F,
"F_SP115", 0, 0x06, 0x00, 0xF01F,
"F_SP116", 4, 0x02, 0x50, 0xF01F,
"F_SP116", 0, 0x10, 0xB0, 0xF09F,
"D_MN11", 6, 0x00, 0x00, 0xF01F,
//"D_SB01", 42, 0x00, 0x00, 0xF01F,
"F_SP103", 1, 0x14, 0x00, 0xF018,
//"D_SB01", 48, 0x01, 0x10, 0xF01F, //useless final fairy spawn
"D_MN10B", 51, 0x02, 0x00, 0xF01F,
"F_SP115", 0, 0x46, 0x00, 0xF01F,
"D_MN07B", 51, 0x01, 0x60, 0xF01F,
"D_SB11", 29, 0x00, 0x00, 0xF01F,
"D_MN09", 6, 0x01, 0x00, 0xF01F,
"F_SP125", 4, 0x37, 0x00, 0xF01A,
"F_SP00", 0, 0x05, 0x90, 0xF01F,
"D_MN09A", 50, 0x16, 0x00, 0xF01A,
"D_SB01", 47, 0x00, 0x00, 0xF01F,
"R_SP109", 0, 0x06, 0xA0, 0xF09F,
//"D_MN06", 4, 0x01, 0x00, 0xF01F, //Fortune Teller
"F_SP104", 1, 0x14, 0x00, 0xF079,
//"D_SB11", 27, 0x00, 0x00, 0xF01F,
"F_SP108", 5, 0x06, 0x50, 0xF09F,
"F_SP103", 1, 0x1B, 0x00, 0xF01F,
"D_MN09", 12, 0x07, 0x00, 0xF01F,
//"D_SB11", 33, 0x00, 0x00, 0xF01F,
"F_SP121", 14, 0x00, 0x20, 0xF01F,
//"D_SB11", 22, 0x00, 0x00, 0xF01F,
"F_SP125", 4, 0x08, 0x00, 0xF030,
"R_SP109", 0, 0x02, 0x10, 0xF09F,
"F_SP121", 9, 0x00, 0x20, 0xF01F,
"D_MN08", 5, 0x01, 0x00, 0xF01F,
"R_SP109", 6, 0x05, 0xB0, 0xF09F,
"F_SP110", 0, 0x02, 0x00, 0xF01F,
"D_MN09", 9, 0x01, 0x50, 0xF011,
"R_SP160", 5, 0x00, 0x50, 0xF01F,
"D_SB01", 38, 0x00, 0x00, 0xF01F,
"F_SP124", 0, 0x32, 0x00, 0xF03F,
"D_MN05A", 50, 0x01, 0x00, 0xF03F,
"F_SP121", 2, 0x00, 0x20, 0xF01F,
"F_SP00", 0, 0x00, 0x00, 0xF01F,
//"R_SP107", 3, 0x17, 0x00, 0xF039, //Zelda 2 cs, advances Faron State
"F_SP127", 0, 0x01, 0x00, 0xF01F,
"D_MN09", 11, 0x00, 0x00, 0xF01F,
"F_SP112", 1, 0x07, 0x00, 0xF011,
"F_SP109", 0, 0x43, 0x00, 0x601D,
"F_SP110", 3, 0x02, 0x80, 0xF01F,
"F_SP121", 12, 0x01, 0x50, 0xF09F,
"F_SP124", 0, 0x01, 0x00, 0xF01F,
"F_SP117", 3, 0x05, 0x90, 0xF01F,
"F_SP114", 1, 0x16, 0x00, 0xF01F,
"D_MN11", 4, 0x00, 0x00, 0xF01F,
"D_SB01", 6, 0x00, 0x00, 0xF01F,
"D_MN11", 3, 0x00, 0x00, 0xF01F,
"F_SP116", 3, 0x01, 0xA0, 0xF01F,
"D_MN09", 4, 0x00, 0x00, 0xF011,
"R_SP160", 4, 0x00, 0xB0, 0xF09F,
"R_SP116", 6, 0x0A, 0x00, 0xF01F,
"F_SP109", 0, 0x2F, 0xA0, 0xF09F,
"F_SP122", 8, 0x03, 0x00, 0x1038,
"F_SP112", 1, 0x06, 0x00, 0xF01F,
"D_MN08", 4, 0x00, 0x70, 0xF01F,
"F_SP108", 6, 0xC8, 0x00, 0xF01F,
"D_MN09B", 0, 0x00, 0x20, 0xF01F,
"F_SP104", 1, 0x6F, 0xC0, 0xF01F,
"F_SP108", 6, 0x03, 0x50, 0xF01F,
"D_MN10", 4, 0x00, 0x00, 0xF01F,
"D_MN10", 7, 0x00, 0x00, 0xF01F,
"F_SP116", 3, 0x04, 0xA0, 0xF09F,
"F_SP109", 0, 0x05, 0x00, 0xF01F,
//"D_SB01", 35, 0x00, 0x00, 0xF01F,
"D_MN11", 3, 0x03, 0xA0, 0xF01F,
"D_MN06", 7, 0x02, 0x00, 0xF01F,
"F_SP121", 0, 0x10, 0x00, 0xF01F,
"R_SP116", 6, 0x15, 0x00, 0xF01F,
"D_MN09A", 51, 0x79, 0x00, 0xF01F,
"D_MN05A", 50, 0x00, 0x60, 0xF01F,
"F_SP121", 10, 0x15, 0x00, 0xF01F,
"F_SP00", 0, 0x01, 0x50, 0xF09F,
"R_SP01", 4, 0x04, 0x00, 0xB00F,
"F_SP109", 0, 0x14, 0x00, 0xF01F,
"F_SP113", 1, 0x04, 0x50, 0xF09F,
"D_MN04", 7, 0x01, 0x00, 0xF01F,
"F_SP118", 1, 0x00, 0x50, 0xF01F,
"F_SP126", 0, 0x04, 0xA0, 0xF09F,
"F_SP122", 8, 0xFE, 0x00, 0xF01F,
"D_MN07B", 51, 0x00, 0x00, 0xF01F,
//"D_MN04", 6, 0x01, 0x00, 0xF01F, //Fortune Teller
"R_SP01", 1, 0x00, 0x10, 0xF09F,
"F_SP121", 3, 0x58, 0xC0, 0xF01F,
"D_MN05", 1, 0x00, 0x00, 0xF01F,
"F_SP103", 1, 0x08, 0x00, 0xF01F,
"F_SP114", 2, 0x08, 0x50, 0xF01F,
"D_MN11", 3, 0x05, 0xA0, 0xF01F,
"F_SP121", 3, 0x0A, 0x20, 0xF01F,
"D_MN09", 14, 0x00, 0x00, 0xF01F,
"F_SP108", 6, 0x0A, 0x00, 0xF01F,
"F_SP113", 1, 0x1E, 0x50, 0xF01F,
"F_SP121", 4, 0x00, 0x20, 0xF01F,
"R_SP107", 0, 0x02, 0x00, 0xF01F,
"D_MN07", 16, 0x02, 0x00, 0xF01F,
"F_SP108", 0, 0x03, 0x00, 0xF01F,
"D_SB11", 48, 0x01, 0x10, 0xF01F,
"R_SP116", 5, 0x03, 0x00, 0xF01F,
"F_SP121", 6, 0x02, 0x90, 0xF01F,
//"D_MN09", 9, 0x02, 0x00, 0xF01F, //Fortune Teller
"D_MN09", 12, 0x05, 0x00, 0xF01F,
"D_MN04", 9, 0x02, 0x70, 0xF01F,
"F_SP103", 1, 0x1A, 0x00, 0xF01F,
"F_SP121", 7, 0x01, 0x50, 0xF09F,
"D_MN04", 9, 0x01, 0x70, 0xF01F,
"D_MN04A", 50, 0x00, 0x60, 0xF01F,
"F_SP126", 0, 0x65, 0xC0, 0xF01F,
"R_SP107", 1, 0x01, 0x00, 0xF09F,
"D_MN10", 14, 0x00, 0x00, 0xF01F,
"D_MN08A", 10, 0x00, 0x60, 0xF01F,
"D_MN10", 6, 0x00, 0x00, 0xF01F,
"D_MN06", 0, 0x00, 0x50, 0xF01F,
"F_SP109", 0, 0x02, 0x00, 0xF09F,
"D_MN01", 9, 0x01, 0x50, 0xF01F,
"R_SP107", 2, 0x01, 0x00, 0xF01F,
"F_SP116", 0, 0x63, 0x50, 0xF01F,
//"D_SB11", 15, 0x00, 0x00, 0xF01F,
"R_SP109", 0, 0x0A, 0x00, 0xF01F,
"D_MN05B", 51, 0x00, 0x00, 0xF03F,
//"F_SP121", 6, 0x0C, 0x00, 0xF01F, //Fortune Teller Spawn
"F_SP118", 1, 0x06, 0x00, 0xF01F,
"D_MN08", 9, 0x15, 0x00, 0xF018,
"D_MN09A", 51, 0x00, 0x00, 0xF01F,
"F_SP103", 0, 0x1B, 0x00, 0xF01F,
"D_SB09", 4, 0x01, 0xC0, 0xF014,
"D_MN09", 14, 0x05, 0x50, 0xF010,
"D_MN07", 8, 0x00, 0x00, 0xF01F,
"D_SB07", 2, 0x01, 0xC0, 0xF013,
"F_SP115", 0, 0x11, 0xC0, 0xF03F,
"D_MN01", 0, 0x01, 0x00, 0xF01F,
"F_SP115", 0, 0x4C, 0x00, 0xF01F,
"R_SP110", 0, 0x02, 0x00, 0xF01F,
"D_MN11", 4, 0x02, 0xB0, 0xF01F,
"D_MN07", 7, 0x00, 0x00, 0xF01F,
"F_SP108", 1, 0x00, 0x00, 0xF01F,
"F_SP109", 0, 0x3D, 0x00, 0xF01F,
//"F_SP121", 0, 0x11, 0x00, 0xF01F, //Fortune Teller
"F_SP121", 10, 0x01, 0x20, 0xF01F,
"F_SP122", 16, 0x02, 0x00, 0xF01F,
"D_SB11", 2, 0x00, 0x00, 0xF01F,
"F_SP121", 0, 0x15, 0x50, 0xF09F,
"F_SP114", 0, 0x01, 0x90, 0xF01F,
"F_SP108", 8, 0x02, 0x50, 0xF09F,
"F_SP103", 0, 0x66, 0x50, 0xF096,
"D_SB01", 8, 0x00, 0x00, 0xF01F,
"D_MN09", 15, 0x00, 0xA0, 0xF011,
"F_SP112", 1, 0x04, 0x00, 0xF01F,
"R_SP161", 7, 0x00, 0x00, 0xF01F,
"F_SP115", 0, 0xFE, 0x00, 0xF01F,
"D_MN10", 10, 0x00, 0x00, 0xF01F,
"D_MN11", 5, 0x04, 0xB0, 0xF01F,
//"D_SB01", 22, 0x00, 0x00, 0xF01F,
//"D_SB01", 43, 0x00, 0x00, 0xF01F,
"F_SP109", 0, 0x13, 0x00, 0xF01F,
//"F_SP121", 13, 0x0E, 0x00, 0xF01F, //Fortune Teller
"F_SP116", 1, 0x6F, 0xC0, 0xF01F,
"D_MN07", 10, 0x00, 0x00, 0xF01F,
"D_MN07", 15, 0x04, 0x00, 0xF01F,
"F_SP116", 0, 0x0C, 0xA0, 0xF09F,
//"F_SP121", 9, 0x0A, 0x00, 0xF01E, //Lanayru Twilight Barrier
//"F_SP109", 0, 0x26, 0x00, 0xB02C, //Post GM CS
"D_MN07A", 50, 0x03, 0x00, 0xF01F,
"D_MN08", 5, 0x00, 0x70, 0xF01F,
"F_SP121", 0, 0x32, 0x20, 0xF01F,
"F_SP121", 13, 0x62, 0xC0, 0x5012,
"F_SP115", 0, 0x0E, 0x00, 0xF01F,
"F_SP110", 3, 0x00, 0x00, 0xF01F,
"F_SP122", 17, 0x00, 0x50, 0xF09F,
//"D_SB01", 21, 0x00, 0x00, 0xF01F,
"D_MN08D", 57, 0x00, 0x00, 0xF01F,
"F_SP114", 1, 0x0B, 0x80, 0xF01F,
"D_MN07", 14, 0x00, 0x00, 0xF01F,
"D_MN07", 2, 0x01, 0x00, 0xF05F,
"D_MN11", 5, 0x00, 0x00, 0xF01F,
"F_SP111", 0, 0x05, 0x90, 0xF03F,
"D_MN07", 12, 0x01, 0x70, 0xF01F,
"F_SP125", 4, 0x02, 0x00, 0xF030,
"D_SB11", 16, 0x00, 0x00, 0xF01F,
"F_SP114", 0, 0x0F, 0x50, 0xF01F,
"D_MN09A", 50, 0x78, 0x00, 0xF01F,
"D_MN08A", 10, 0x18, 0x00, 0xF01F,
"F_SP108", 5, 0x03, 0x50, 0xF09F,
"F_SP108", 2, 0x00, 0xC0, 0xF01F,
"F_SP103", 0, 0x06, 0xA0, 0xF09F,
"F_SP108", 8, 0x01, 0x00, 0xF01F,
"F_SP109", 0, 0x3E, 0x00, 0xF01F,
"D_MN05", 12, 0x00, 0x00, 0xF01F,
"D_MN09A", 51, 0x01, 0x00, 0xF01F,
//"D_MN05", 3, 0x01, 0x00, 0xF01F, //Fortune Teller
"F_SP115", 0, 0x1D, 0x50, 0xF01F,
"D_MN08", 11, 0x01, 0x70, 0xF01F,
"F_SP108", 6, 0xFE, 0x00, 0xF01F,
"F_SP109", 0, 0x3A, 0x00, 0xF01F,
"D_MN04", 17, 0x00, 0x00, 0xF01F,
"D_MN04", 17, 0x01, 0x00, 0xF01F,
"R_SP109", 6, 0x00, 0x00, 0xF03F,
"F_SP102", 0, 0x35, 0x00, 0xF019,
"F_SP104", 1, 0x16, 0x00, 0xB028,
"F_SP114", 1, 0x06, 0xC0, 0xF01F,
"F_SP113", 1, 0x06, 0x50, 0xF09F,
"F_SP114", 1, 0x09, 0xA0, 0xF01F,
"F_SP108", 5, 0x19, 0x00, 0xF01F,
"F_SP125", 4, 0x05, 0x00, 0xF01F,
//"D_SB01", 8, 0x01, 0x10, 0xF01F,
"F_SP124", 0, 0x03, 0x00, 0xF01F,
"F_SP116", 3, 0x1E, 0x10, 0xF05F,
"F_SP114", 0, 0x04, 0x00, 0xF01F,
"D_MN04", 9, 0x03, 0x00, 0xF01F,
"F_SP124", 0, 0x33, 0x00, 0xF019,
"R_SP107", 0, 0x18, 0x00, 0xF01B,
"D_MN09", 15, 0x02, 0x00, 0xF01F,
"D_MN07", 10, 0x01, 0x70, 0xF01F,
"F_SP121", 0, 0x00, 0x20, 0xF01F,
"D_SB01", 39, 0x00, 0x00, 0xF014,
//"D_MN08", 1, 0x01, 0x00, 0xF01F, //Fortune Teller
"D_MN09C", 0, 0x14, 0x00, 0xF018,
"D_MN09", 12, 0x01, 0x60, 0xF01F,
//"F_SP200", 0, 0x01, 0xC0, 0xF077, //Hidden Skill Learning Spawn
"R_SP116", 6, 0x14, 0x00, 0xF01F,
"D_MN10", 3, 0x00, 0x00, 0xF01F,
"D_MN09A", 51, 0x16, 0x00, 0xF01F,
"R_SP01", 4, 0x01, 0x00, 0xF01F,
"F_SP104", 1, 0x04, 0x50, 0xF01F,
"F_SP118", 3, 0x07, 0x50, 0xF01F,
"D_MN04", 4, 0x00, 0x00, 0xF01F,
"D_MN07", 3, 0x02, 0x70, 0xF01F,
"D_MN11B", 51, 0x01, 0xA0, 0xF01F,
"F_SP117", 2, 0x65, 0x80, 0xF01F,
"D_SB11", 46, 0x00, 0x00, 0xF01F,
"D_MN09", 2, 0x02, 0xA0, 0xF01F,
"D_MN08A", 10, 0x15, 0x00, 0xF018,
"D_MN11", 3, 0x06, 0xB0, 0xF01F,
//"F_SP127", 0, 0x05, 0x00, 0xF01F, //Fortune Teller
//"F_SP103", 1, 0x15, 0x00, 0xF03F, //Links House Window
//"R_SP161", 7, 0x04, 0x00, 0xF01F, //Fortune Teller
"F_SP121", 3, 0x14, 0x00, 0xF01F,
"D_MN11", 11, 0x00, 0x00, 0xF01F,
"D_MN09", 14, 0x04, 0x00, 0xF01F,
"R_SP107", 0, 0x03, 0x00, 0xF03F,
"F_SP121", 13, 0x04, 0x00, 0xF01F,
"D_MN07", 6, 0x05, 0x70, 0xF01F,
"D_MN08", 2, 0x01, 0x00, 0xF01F,
"D_MN10A", 50, 0x00, 0x60, 0xF01F,
"D_SB01", 16, 0x00, 0x00, 0xF01F,
"F_SP108", 5, 0x32, 0xC0, 0xF01F,
"F_SP121", 0, 0x0F, 0x00, 0xF01F,
"D_MN11B", 51, 0x00, 0x00, 0xF01F,
"D_MN09", 5, 0x00, 0x00, 0xF01F,
"D_MN11", 1, 0x00, 0x00, 0xF01F,
"F_SP112", 1, 0x10, 0x00, 0xF01F,
//"D_SB11", 13, 0x00, 0x00, 0xF01F,
"D_MN10", 9, 0x00, 0x00, 0xF01F,
"D_MN07", 2, 0x00, 0x00, 0xF01F,
"F_SP115", 0, 0x65, 0xC0, 0xF01F,
"D_MN10A", 50, 0x02, 0x00, 0xF03F,
"F_SP121", 13, 0x03, 0x90, 0xF01F,
"D_MN08B", 51, 0x02, 0x00, 0xF01F,
"D_MN07", 5, 0x01, 0x70, 0xF01F,
"F_SP125", 4, 0x07, 0x00, 0xF030,
//"R_SP160", 5, 0x03, 0x00, 0xF01F, //Fortune Teller
"D_MN08", 1, 0x00, 0x70, 0xF01F,
"D_MN09A", 50, 0x00, 0x00, 0xF01F,
"D_SB01", 20, 0x00, 0x00, 0xF01F,
"F_SP124", 0, 0x0B, 0x00, 0xF01F,
"F_SP116", 3, 0x09, 0x50, 0xF09F,
"R_SP109", 5, 0x01, 0x50, 0xF09F,
"F_SP113", 0, 0x05, 0x00, 0xF03F,
"D_MN09", 9, 0x00, 0x00, 0xF01F,
"D_MN04", 1, 0x01, 0x00, 0xF01F,
"D_MN10", 15, 0x01, 0x00, 0xF01F,
"F_SP103", 0, 0x1A, 0x00, 0x509F,
"R_SP116", 5, 0x00, 0xB0, 0xF09F,
"F_SP122", 17, 0x01, 0x50, 0xF09F,
"D_MN01", 12, 0x00, 0x00, 0xF01F,
"F_SP117", 1, 0x05, 0x10, 0xF032,
"F_SP116", 3, 0x05, 0xA0, 0xF09F,
"R_SP107", 2, 0x02, 0x50, 0xF01F,
"R_SP110", 0, 0x03, 0xE0, 0xF01F,
"D_MN09A", 50, 0x02, 0x00, 0xF011,
"F_SP117", 3, 0x06, 0x00, 0xF01F,
"D_MN05", 19, 0x00, 0x00, 0xF01F,
"F_SP121", 1, 0x15, 0x00, 0xF01F,
"F_SP115", 0, 0x21, 0x00, 0xF01F,
"D_MN05", 12, 0x01, 0x00, 0xF01F,
//"D_MN01", 8, 0x02, 0x00, 0xF01F, //Fortune Teller
"F_SP112", 1, 0x01, 0xB0, 0xF09F,
"F_SP117", 1, 0x01, 0x00, 0xF01F,
//"D_SB01", 12, 0x00, 0x00, 0xF01F,
"F_SP113", 0, 0x03, 0x50, 0xF09F,
"F_SP109", 0, 0x3F, 0x00, 0xF01F,
"R_SP108", 0, 0x01, 0xB0, 0xF09F,
"D_SB11", 26, 0x00, 0x00, 0xF01F,
"F_SP103", 0, 0x17, 0x00, 0xF01F,
//"D_SB01", 11, 0x00, 0x00, 0xF01F,
"D_MN07", 14, 0x01, 0x70, 0xF01F,
"F_SP109", 0, 0x21, 0x00, 0xC02F,
"F_SP116", 0, 0x32, 0x00, 0xF09F,
"D_MN01", 9, 0x04, 0x00, 0xF01F,
"F_SP128", 0, 0x00, 0x50, 0xF09F,
//"D_SB01", 32, 0x00, 0x00, 0xF01F,
"D_MN01B", 51, 0x03, 0x00, 0xF03F,
"F_SP103", 1, 0x06, 0x00, 0xF01F,
"F_SP108", 4, 0x01, 0xA0, 0xF01F,
"F_SP121", 10, 0x06, 0x90, 0xF01F,
"D_SB11", 49, 0x00, 0x00, 0xF01F,
//"D_SB01", 15, 0x00, 0x00, 0xF01F,
"F_SP00", 0, 0x06, 0x00, 0xF01F,
"F_SP108", 5, 0x07, 0x00, 0xF01F,
"D_SB01", 18, 0x00, 0x00, 0xF01F,
"F_SP121", 13, 0x16, 0x00, 0xF01F,
"R_SP01", 4, 0x03, 0x00, 0xF03F,
"D_MN01", 6, 0x00, 0x00, 0xF01F,
"D_MN07", 0, 0x00, 0x00, 0xF01F,
"D_MN08D", 54, 0x00, 0x00, 0xF01F,
//"D_SB11", 42, 0x00, 0x00, 0xF01F,
"F_SP128", 0, 0x64, 0xC0, 0xF01F,
"D_MN01", 9, 0x00, 0x00, 0xF01F,
//"D_SB11", 35, 0x00, 0x00, 0xF01F,
"F_SP116", 3, 0x03, 0x00, 0xF01F,
"R_SP109", 4, 0x02, 0xB0, 0xF09F,
"D_MN11B", 51, 0x03, 0x00, 0xF03F,
"F_SP117", 3, 0x01, 0x50, 0xF01F,
"R_SP01", 7, 0x00, 0x00, 0xF01F,
"D_MN08B", 51, 0x03, 0x00, 0xF01F,
"F_SP108", 8, 0x00, 0x00, 0xF01F,
"D_MN11", 3, 0x0A, 0xB0, 0xF01F,
"F_SP116", 1, 0x00, 0x50, 0xF01F,
"D_MN05", 9, 0x00, 0x00, 0xF01F,
"F_SP121", 10, 0x07, 0x90, 0xF01F,
"D_MN06", 2, 0x01, 0x00, 0xF01F,
//"D_SB01", 28, 0x01, 0x10, 0xF01F,
"R_SP109", 0, 0x16, 0x00, 0xF01F,
"F_SP116", 0, 0x04, 0x50, 0xF09F,
"D_MN09", 12, 0x08, 0x00, 0xF01F,
"D_MN04", 11, 0x01, 0x00, 0xF01F,
"D_MN08A", 10, 0x19, 0x00, 0xF01F,
"F_SP117", 1, 0x63, 0x00, 0xF019,
"R_SP109", 3, 0x01, 0x00, 0xF09F,
"D_MN07", 6, 0x00, 0x00, 0xF01F,
"D_MN04B", 51, 0x03, 0x00, 0xF03F,
"R_SP01", 0, 0x01, 0xB0, 0xF09F,
"D_MN11", 3, 0x08, 0xB0, 0xF01F,
"D_MN09", 1, 0x01, 0x00, 0xF01F,
"F_SP109", 0, 0x17, 0x20, 0xF01F,
"F_SP108", 4, 0x17, 0x00, 0xF01F,
"F_SP109", 0, 0x0D, 0x40, 0xF03F,
"D_SB08", 3, 0x00, 0x00, 0xF011,
"F_SP110", 3, 0x04, 0x00, 0xF01F,
"D_MN04", 6, 0x00, 0x00, 0xF01F,
"F_SP117", 1, 0xC8, 0x00, 0xF01F,
"F_SP116", 2, 0x03, 0x50, 0xF09F,
"F_SP109", 0, 0x47, 0x00, 0xF01F,
"F_SP103", 1, 0x02, 0x50, 0x3091,
"F_SP110", 2, 0x00, 0x00, 0xF01F,
"F_SP114", 1, 0x15, 0x00, 0xF01F,
"F_SP112", 1, 0x11, 0x00, 0xF01F,
"F_SP122", 16, 0x04, 0x00, 0xF01F,
"F_SP117", 2, 0x03, 0x00, 0xF01F,
"D_MN07", 11, 0x00, 0x00, 0xF01F,
"F_SP108", 5, 0x02, 0x50, 0xF09F,
//"D_MN06", 8, 0x01, 0x60, 0xF01F, //Broken ToT spawn before boss room
//"D_MN06", 3, 0x01, 0x00, 0xF01F, //Fortune Teller
"R_SP109", 1, 0x01, 0x40, 0xF03F,
"F_SP104", 1, 0x1E, 0x00, 0xF01F,
"D_MN09", 2, 0x00, 0x00, 0xF01F,
"F_SP115", 0, 0x01, 0x00, 0xF01F,
"F_SP104", 1, 0x01, 0x50, 0xF09F,
"F_SP114", 2, 0x0D, 0x50, 0xF01F,
"D_MN08", 0, 0x0A, 0x00, 0xF01E,
"D_MN10", 0, 0x02, 0x00, 0xF01F,
"F_SP121", 12, 0x02, 0x20, 0x1053,
"F_SP109", 0, 0x0B, 0x50, 0xF09F,
"R_SP107", 0, 0x01, 0x50, 0xF09F,
"F_SP116", 4, 0x00, 0x10, 0xF09F,
"F_SP121", 13, 0x14, 0x00, 0xF01F,
"F_SP109", 0, 0x01, 0x50, 0xF09F,
"D_SB01", 27, 0x00, 0x00, 0xF01F,
"D_MN10B", 51, 0x01, 0x00, 0xF03F,
"R_SP160", 4, 0x01, 0x00, 0xF09F,
"F_SP126", 0, 0x07, 0x50, 0xF09F,
//"F_SP200", 0, 0x00, 0xC0, 0xF077, //Hidden Skill Learning Spawn
"F_SP121", 0, 0x02, 0x20, 0xF01F,
"D_MN08D", 56, 0x00, 0x00, 0xF01F,
"D_MN07", 6, 0x01, 0x00, 0xF05F,
"D_MN11", 3, 0x02, 0xB0, 0xF01F,
"D_MN08B", 51, 0x00, 0x60, 0xF01F,
"D_MN04", 9, 0x00, 0x00, 0xF01F,
"D_MN10B", 51, 0x00, 0x60, 0xF01F,
"F_SP116", 2, 0x04, 0x00, 0xF01F,
"R_SP110", 0, 0x01, 0x50, 0xF01F,
"D_SB05", 0, 0x00, 0x00, 0xF010,
"D_MN09", 11, 0x01, 0xB0, 0xF010,
"D_MN11", 3, 0x01, 0xA0, 0xF01F,
"D_MN06", 3, 0x00, 0x00, 0xF01F,
"F_SP104", 1, 0x02, 0x00, 0xF01F,
"F_SP116", 3, 0x02, 0x50, 0xF01F,
"D_MN01", 11, 0x00, 0x00, 0xF01F,
"R_SP160", 2, 0x00, 0xB0, 0xF09F,
"D_MN01", 5, 0x02, 0x00, 0xF01F,
"F_SP109", 0, 0x2C, 0xA0, 0xF09F,
"F_SP109", 0, 0x2A, 0xA0, 0xF09F,
"F_SP117", 3, 0x04, 0x00, 0xF01F,
"F_SP121", 6, 0x01, 0x50, 0xF09F,
"D_MN09B", 0, 0x01, 0x00, 0xF01F,
"D_SB11", 17, 0x00, 0x00, 0xF01F,
"F_SP109", 0, 0x0F, 0x00, 0xF01F,
//"D_SB11", 3, 0x00, 0x00, 0xF01F,
"D_MN08", 11, 0x16, 0x00, 0xF018,
"F_SP109", 0, 0x16, 0x00, 0xF01F,
"F_SP108", 3, 0x63, 0x50, 0xF09F,
"F_SP200", 0, 0x07, 0x00, 0xF010,
"F_SP103", 1, 0x18, 0x00, 0xF01F,
"F_SP122", 8, 0x00, 0x50, 0xF09F,
"D_SB11", 10, 0x00, 0x00, 0xF01F,
"F_SP115", 0, 0x0C, 0x00, 0xF01F,
"F_SP114", 1, 0x02, 0x00, 0xF01F,
"D_MN06", 7, 0x01, 0x70, 0xF01F,
"D_MN11", 6, 0x02, 0xB0, 0xF01F,
"D_MN09A", 50, 0x14, 0x00, 0xF018,
"F_SP108", 5, 0x18, 0x00, 0xF01F,
"F_SP108", 4, 0x00, 0x00, 0xF01F,
"F_SP124", 0, 0x6F, 0xC0, 0xF01F,
"F_SP117", 1, 0x15, 0x00, 0xF01F,
"F_SP117", 2, 0x66, 0x00, 0xF01F,
"R_SP01", 2, 0x01, 0x00, 0xF01F,
"F_SP121", 2, 0x01, 0x00, 0xF01F,
"F_SP122", 16, 0x6F, 0xC0, 0xF01F,
"F_SP116", 1, 0x64, 0x00, 0xF019,
"D_SB01", 48, 0x00, 0x00, 0xF01F,
"F_SP113", 0, 0xFE, 0x00, 0xF01F,
"F_SP113", 1, 0x01, 0x00, 0xF01F,
"D_SB01", 26, 0x00, 0x00, 0xF01F,
"D_MN10", 15, 0x00, 0x00, 0xF01F,
"R_SP01", 5, 0x00, 0xB0, 0xF09F,
"F_SP109", 0, 0x2D, 0xA0, 0xF09F,
"D_MN06", 0, 0x01, 0x00, 0xF01F,
"D_MN07A", 50, 0x02, 0x00, 0xF03F,
"D_MN07", 0, 0x05, 0x00, 0xF01F,
//"D_SB01", 14, 0x00, 0x00, 0xF01F,
"D_SB11", 40, 0x00, 0x00, 0xF01F,
"F_SP116", 2, 0x00, 0x50, 0xF09F,
"R_SP107", 0, 0x19, 0x00, 0xF01F,
"D_MN09", 15, 0x06, 0x00, 0xF01F,
"F_SP121", 15, 0x00, 0x20, 0xF01F,
"F_SP121", 3, 0x05, 0x20, 0xF01F,
"F_SP111", 0, 0x00, 0x50, 0xF09F,
"D_MN05", 0, 0x00, 0x50, 0xF01F,
"D_SB11", 37, 0x00, 0x00, 0xF01F,
"D_MN01", 0, 0x02, 0x00, 0xF01F,
"D_MN09", 13, 0x00, 0x00, 0xF01F,
"F_SP117", 1, 0x03, 0x50, 0xF01F,
"D_MN01B", 51, 0x01, 0x50, 0xF01F,
//"D_SB01", 45, 0x00, 0x00, 0xF01F,
"F_SP117", 1, 0x0A, 0x00, 0xF01F,
"R_SP160", 2, 0x01, 0xA0, 0xF09F,
"D_MN09", 1, 0x01, 0x00, 0xF01F,
"R_SP109", 1, 0x03, 0x00, 0xF01F,
"F_SP115", 0, 0x20, 0x00, 0xF01F,
"D_MN07", 6, 0x02, 0x70, 0xF01F,
"D_MN07", 0, 0x04, 0x80, 0xF01F,
"R_SP116", 6, 0x0C, 0x00, 0xF01F,
"R_SP160", 5, 0x02, 0x00, 0xF01F,
"F_SP200", 0, 0x02, 0x00, 0xF010,
"F_SP122", 8, 0x05, 0x90, 0xF01F,
"F_SP200", 0, 0x03, 0x00, 0xF010,
"F_SP103", 0, 0x07, 0xA0, 0xF01F,
"F_SP116", 0, 0x05, 0x50, 0xF09F,
"F_SP117", 1, 0x04, 0x00, 0xF01F,
"D_MN08", 7, 0x00, 0x70, 0xF01F,
"F_SP00", 0, 0x07, 0x50, 0xF01F,
"F_SP121", 3, 0x00, 0x50, 0xF09F,
"F_SP121", 1, 0x16, 0x00, 0xF01F,
"D_MN08", 0, 0x03, 0x70, 0xF01F,
"D_MN07", 2, 0x03, 0x70, 0xF01F,
"D_MN09", 15, 0x07, 0x00, 0xF01F,
"D_SB10", 0, 0x01, 0x10, 0xF09F,
"D_MN04", 14, 0x01, 0x00, 0xF01F,
"D_MN07", 6, 0x07, 0x00, 0xF033,
"F_SP109", 0, 0x37, 0x20, 0xF03F,
"D_MN08D", 53, 0x00, 0x00, 0xF01F,
"F_SP114", 0, 0x64, 0xC0, 0xF01F,
//"D_SB11", 44, 0x00, 0x00, 0xF01F,
"D_SB01", 37, 0x00, 0x00, 0xF01F,
"F_SP116", 3, 0x0C, 0x10, 0xF01F,
"F_SP104", 1, 0x0A, 0x00, 0xF01F,
//"D_MN06B", 51, 0x00, 0x60, 0xF01F, //Darknut Miniboss crashed
"D_MN10", 16, 0x00, 0x00, 0xF01F,
"R_SP116", 5, 0x04, 0x10, 0xF01F,
"F_SP110", 0, 0x01, 0x00, 0xF09F,
"D_MN10A", 50, 0x01, 0x00, 0xF03F,
"F_SP108", 11, 0x00, 0x50, 0xF09F,
"F_SP113", 0, 0x61, 0x00, 0xF019,
"F_SP116", 0, 0x0F, 0x10, 0xF095,
"D_SB01", 17, 0x00, 0x00, 0xF01F,
"F_SP108", 0, 0x00, 0x50, 0xF01F,
"D_MN09A", 50, 0x7A, 0x00, 0xF01F,
"F_SP113", 0, 0x63, 0x00, 0xF01F,
"D_MN08", 11, 0x14, 0x00, 0xF01F,
"D_MN11", 5, 0x03, 0xA0, 0xF01F,
//"D_SB11", 43, 0x00, 0x00, 0xF01F,
"F_SP121", 10, 0x00, 0x20, 0xF01F,
"D_MN08A", 10, 0x01, 0x00, 0xF01F,
"F_SP121", 12, 0x15, 0x00, 0xF01F,
"F_SP126", 0, 0x05, 0x00, 0xF01F,
"F_SP110", 0, 0x00, 0x50, 0xF09F,
"F_SP109", 0, 0x36, 0x00, 0xF03F,
"F_SP115", 0, 0x05, 0x50, 0xF01F,
"F_SP121", 1, 0x14, 0x00, 0xF01F,
"R_SP108", 0, 0x00, 0x00, 0xF05F,
//"D_MN08", 4, 0x01, 0x00, 0xF01F, //Fortune Teller
"F_SP128", 0, 0x04, 0x00, 0xF012,
"F_SP127", 0, 0x02, 0xB0, 0xF01F,
"D_MN08", 9, 0x00, 0x00, 0xF01F,
"D_MN08", 11, 0x15, 0x00, 0xF019,
//"D_MN05", 5, 0x01, 0x00, 0xF01F, //Fortune Teller
"F_SP103", 1, 0x00, 0x10, 0xF09F,
"F_SP117", 1, 0xFE, 0x00, 0xF01F,
"D_MN11", 4, 0x07, 0xB0, 0xF01F,
"R_SP160", 5, 0x01, 0x10, 0x5011,
"D_MN09", 11, 0x05, 0x00, 0xF01F,
"F_SP109", 0, 0x19, 0x00, 0xF01F,
"F_SP109", 0, 0x03, 0x50, 0xF01F,
"F_SP125", 4, 0x39, 0x00, 0xF01C,
"D_SB01", 36, 0x00, 0x00, 0xF01F,
"R_SP110", 0, 0x04, 0x00, 0xF01F,
"F_SP124", 0, 0x0A, 0x90, 0xF01F,
"F_SP116", 1, 0x01, 0x00, 0xF01F,
"F_SP114", 0, 0x06, 0x50, 0xF01F,
"F_SP103", 0, 0x19, 0x20, 0xF078,
"R_SP01", 0, 0x03, 0x00, 0xF01F,
"F_SP114", 1, 0x03, 0x00, 0xF01F,
"D_MN07", 6, 0x04, 0x70, 0xF01F,
"F_SP109", 0, 0x27, 0x00, 0xF032,
"D_SB11", 28, 0x01, 0x10, 0xF01F,
"F_SP109", 0, 0x35, 0x50, 0xF03F,
"D_SB07", 2, 0x00, 0x00, 0xF011,
"F_SP108", 4, 0x07, 0x00, 0xF01F,
"R_SP107", 1, 0x06, 0x50, 0xF09F,
"F_SP114", 1, 0x05, 0x00, 0xF01F,
"D_MN08", 9, 0x01, 0x60, 0xF01F,
"F_SP116", 3, 0x07, 0xC0, 0xF01F,
"D_MN11", 4, 0x0A, 0x00, 0xF01F,
"F_SP124", 0, 0x0C, 0x00, 0xF01F,
"F_SP104", 1, 0x18, 0x00, 0x101D,
"F_SP112", 1, 0x0E, 0x00, 0xF01F,
"R_SP109", 1, 0x00, 0x00, 0xF09F,
"D_MN09A", 51, 0x02, 0x00, 0xF01F,
"F_SP109", 0, 0x3C, 0x50, 0xF09F,
"D_MN05B", 51, 0x01, 0x60, 0xF01F,
"F_SP121", 15, 0x14, 0x00, 0xF01F,
"R_SP109", 0, 0x14, 0x00, 0xF038,
//"D_SB11", 31, 0x00, 0x00, 0xF01F,
"D_MN09", 15, 0x04, 0xA0, 0xF011,
"F_SP109", 0, 0x30, 0xB0, 0xF09F,
"F_SP124", 0, 0x02, 0x50, 0xF01F,
"D_MN11", 4, 0x03, 0x00, 0xF01F,
"D_MN08", 5, 0x02, 0x00, 0xF01F,
"F_SP125", 4, 0x36, 0x00, 0xF019,
"F_SP122", 8, 0x02, 0x50, 0xF01F,
"F_SP115", 0, 0x07, 0x50, 0xF09F,
"R_SP301", 0, 0x00, 0x00, 0xF01F
};
#pragma endregion AllSpawns

static SpawnArray AllSpawnsRandom;


static std::list<resList> *loadedArcFiles = NULL;
static int handledFileSize = 0;
static int handledDecompressedFileSize = 0;

//Beta Quest Flags
static u8 isGameOver = 0;
static u8 isGameStart = 0;
static u8 isFileLoad = 0;
static u8 isNewGame = 0;
static u8 isCheckingDeathMusic = 0;

static u8 forceHuman = 0;

static u8 msgQuery_checkEventFlag = 0;
static u8 useRubyFont = 0;

static char LastUsedName[8];
static char LastStage[8];
static int LastRoom = 0;
static u8 LastSpawn = 0;
static u8 LastState = 0;

static u8 warpSet = 0;
static u8 listRandomized = 0;

static char GameOverStage[8];
static int GameOverRoom = 0;
static u8 GameOverSpawn = 0;
static u8 GameOverState = 0;

static char SaveLocationStage[8];
static u8 SaveLocationRoom = 0;
static u8 SaveLocationSpawn = 0;

static int IsBitSet(u8 b, int pos)
{
	return (b & (1 << pos)) != 0;
}


//TP HD Functions
DECL(fopAcM_prm_class*, TP_fopAcM_CreateAppend, void)
{
	fopAcM_prm_class* actorMemoryPtr = real_TP_fopAcM_CreateAppend();

	return actorMemoryPtr;
}
DECL(void, TP_Stage_ActorCreate, void* ActorTemplate, fopAcM_prm_class* ActorMemory)
{
	real_TP_Stage_ActorCreate(ActorTemplate, ActorMemory);
}

void SpawnLoadingZone(char roomID, float xPos, float yPos, float zPos, unsigned short yRot, char ScaleX, char ScaleY, char ScaleZ, char autoWalkDirection, bool isAirTrigger, char exitID)
{
	/*
	FF = Identifier
	00 = Direction of the auto walk (00=Backward ; 01=Right ; 02=Left ; 03=Forward)
	FF = Type of loading zone (00=Air ; FF=Ground)
	00 = Exit ID (only valid for the active room. Under the ID the room stores information like "area to load", "fade type" and "fade duration")
	*/

	stage_SCOB_data_class LoadingZone;

	char type = 0xFF;

	if (isAirTrigger)
		type = 0x00;

	sprintf((char*)LoadingZone.name, "scnChg");
	LoadingZone.params = (0xFF << 24) + (autoWalkDirection << 16) + (type << 8) + exitID;
	LoadingZone.xPos = xPos;
	LoadingZone.yPos = yPos;
	LoadingZone.zPos = zPos;
	LoadingZone.unk1 = 0x0FFF;
	LoadingZone.yRot = yRot;
	LoadingZone.unk2 = 0x0FFF;
	LoadingZone.unk3 = 0xFFFF;
	LoadingZone.scaleX = ScaleX;
	LoadingZone.scaleY = ScaleY;
	LoadingZone.scaleZ = ScaleZ;
	LoadingZone.padding = 0xFF;

	fopAcM_prm_class* actorMemoryPtr = real_TP_fopAcM_CreateAppend();

	actorMemoryPtr->params = LoadingZone.params;

	actorMemoryPtr->xPos = LoadingZone.xPos;
	actorMemoryPtr->yPos = LoadingZone.yPos;
	actorMemoryPtr->zPos = LoadingZone.zPos;

	actorMemoryPtr->xRot = LoadingZone.unk1;
	actorMemoryPtr->yRot = LoadingZone.yRot;
	actorMemoryPtr->flag = LoadingZone.unk2;
	actorMemoryPtr->enemy_id = LoadingZone.unk3;

	actorMemoryPtr->flags[0] = LoadingZone.scaleX;
	actorMemoryPtr->flags[1] = LoadingZone.scaleY;
	actorMemoryPtr->flags[2] = LoadingZone.scaleZ;

	actorMemoryPtr->room_id = roomID;

	real_TP_Stage_ActorCreate(&LoadingZone, actorMemoryPtr);
}
void SpawnActor(char roomID, const char *name, unsigned int Params, float xPos, float yPos, float zPos, unsigned short xRot, unsigned short yRot, unsigned short flag, signed short enemy_id)
{
	stage_ACTR_data_class Actor;

	sprintf((char*)Actor.name, "%s", name);
	Actor.params = Params;
	Actor.xPos = xPos;
	Actor.yPos = yPos;
	Actor.zPos = zPos;
	Actor.xRot = xRot;
	Actor.yRot = yRot;
	Actor.flag = flag;
	Actor.enemy_id = enemy_id;

	fopAcM_prm_class* actorMemoryPtr = real_TP_fopAcM_CreateAppend();

	actorMemoryPtr->params = Actor.params;

	actorMemoryPtr->xPos = Actor.xPos;
	actorMemoryPtr->yPos = Actor.yPos;
	actorMemoryPtr->zPos = Actor.zPos;

	actorMemoryPtr->xRot = Actor.xRot;
	actorMemoryPtr->yRot = Actor.yRot;

	actorMemoryPtr->flag = Actor.flag;
	actorMemoryPtr->enemy_id = Actor.enemy_id;
	actorMemoryPtr->room_id = roomID;

	real_TP_Stage_ActorCreate(&Actor, actorMemoryPtr);
}



DECL(int, TP_getLayerNo, const char* stageName, int roomID, int layerOverwrite)
{
	int result = real_TP_getLayerNo(stageName, roomID, layerOverwrite);
	u8 layerOverwriteByte = (u8)layerOverwrite;

	if (layerOverwriteByte == 0xFF)
	{
		if (!strcmp(stageName, "F_SP108") && result != 0x0E && (roomID == 3 || roomID == 4)) //Coro
		{
			const unsigned char *LanternOwnedPtr = (const unsigned char *)0x10647BE5;
			char *LanternOwned;

			LanternOwned = ((char *)LanternOwnedPtr);

			if (*LanternOwned != 72) //Lantern not owned
			{
				return 0x01;
			}
		}
		else if (!strcmp(stageName, "F_SP103") && roomID == 0) //Ordon Village
		{
			const unsigned char *RodOwnedPtr = (const unsigned char *)0x10647BF8;
			char *RodOwned;

			RodOwned = ((char *)RodOwnedPtr);

			if (*RodOwned != 74 && *RodOwned != 92) //Rod nor upgraded Rod owned
			{
				return 0x00;
			}
		}
		else if (!strcmp(stageName, "R_SP01") && roomID == 2) //Shield House
		{
			return 0x01;
		}
		else if (!strcmp(stageName, "R_SP01") && roomID == 5) //Sword House
		{
			return 0x00;
		}
	}

	return result;
}

DECL(void, TP_Save_onSwitch, unsigned int* gameInfoPtr, int flagID, int roomID)
{
	real_TP_Save_onSwitch(gameInfoPtr, flagID, roomID);

	if (flagID < 0x80)
		log_printf("[%p]Switched Flag: %X in Room: %i ON!", gameInfoPtr, flagID, roomID);
}

DECL(void, TP_Save_offSwitch, unsigned int* gameInfoPtr, int flagID, int roomID)
{
	real_TP_Save_offSwitch(gameInfoPtr, flagID, roomID);

	//log_printf("[%p]Switched Flag: %X in Room: %i ON!", gameInfoPtr, flagID, roomID);
}

DECL(int, TP_Save_isSwitch, unsigned int* gameInfoPtr, int flagID, int roomID)
{
	//log_printf("[0x%p]Check Flag: %X in Room: %i!", gameInfoPtr, flagID, roomID);

	int isSwitchedOn = real_TP_Save_isSwitch(gameInfoPtr, flagID, roomID);

	//log_printf("Flag Status: %i", isSwitchedOn);

	return isSwitchedOn;
}

DECL(void, TP_Stage_Loader, void* ptr, u32* dStage_dt_c) //Add dynamic modded contents to Stage
{
	g_isLoading = 0;
	isNewGame = 0;
	isGameStart = 0;
	isGameOver = 0;
	warpSet = 0;
	isCheckingDeathMusic = 0;

	g_hasGameOvered = 0;
	g_checkIntroItemReload = 0;

	//Loading has finished, make sure CurrentLayer and NextLayer are identical to fix Layer 0 glitch after a GameOver/Void
	const unsigned char *CurrentLayerPtr = (const unsigned char *)0x1064CDF3;
	u8 *CurrentLayer;
	CurrentLayer = ((u8 *)CurrentLayerPtr);

	const unsigned char *NextLayerPtr = (const unsigned char *)0x1064CE01;
	u8 *NextLayer;
	NextLayer = ((u8 *)NextLayerPtr);

	*CurrentLayer = *NextLayer;
	DCFlushRange(CurrentLayer, 1);

	if (isFileLoad == 1) //Restore Save Location when loading finished
	{
		const unsigned char *SaveStagePtr = (const unsigned char *)0x10647BA0;
		char *SaveStage;

		const unsigned char *SaveRoomPtr = (const unsigned char *)0x10647BA9;
		char *SaveRoom;

		const unsigned char *SaveSpawnPtr = (const unsigned char *)0x10647BA8;
		char *SaveSpawn;

		SaveStage = ((char *)SaveStagePtr);
		SaveRoom = ((char *)SaveRoomPtr);
		SaveSpawn = ((char *)SaveSpawnPtr);

		memcpy(SaveStage, &SaveLocationStage, 8);
		DCFlushRange(SaveStage, 8);

		*SaveRoom = SaveLocationRoom;
		DCFlushRange(SaveRoom, 1);

		*SaveSpawn = SaveLocationSpawn;
		DCFlushRange(SaveSpawn, 1);
	}

	isFileLoad = 0;

	//Check for Ooccoo in Forest and refresh her if gotten
	const unsigned char *CurrentStagePtr = (const unsigned char *)0x1064CDE8;
	const char *CurrentStage;

	CurrentStage = ((const char *)CurrentStagePtr);

	if (!strcmp(CurrentStage, "D_MN05"))
	{
		const unsigned char *OoccooGrabbedPtr = (const unsigned char *)0x1064895D;
		char *OoccooGrabbed;

		OoccooGrabbed = ((char *)OoccooGrabbedPtr);

		if (IsBitSet(*OoccooGrabbed, 6) == 1)
		{
			*OoccooGrabbed = *OoccooGrabbed - 64;
			DCFlushRange(OoccooGrabbed, 1);
		}
	}
	else if (!strcmp(CurrentStage, "D_MN01")) //Check for Ooccoo in Lakebed and refresh her if gotten to avoid crash on return
	{
		const unsigned char *OoccooGrabbedPtr = (const unsigned char *)0x1064895D;
		char *OoccooGrabbed;

		OoccooGrabbed = ((char *)OoccooGrabbedPtr);

		if (IsBitSet(*OoccooGrabbed, 6) == 1)
		{
			*OoccooGrabbed = *OoccooGrabbed - 64;
			DCFlushRange(OoccooGrabbed, 1);
		}
	}

	real_TP_Stage_Loader(ptr, dStage_dt_c);

	//Spawn Epona on every Stage
	SpawnActor(0, "Horse", 0x00000F06, 0, 500, 0, 0, 0, 0, 0xFFFF);
}
DECL(void, TP_Room_Loader, void* ptr, u32* dStage_dt_c, int roomID)
{
	real_TP_Room_Loader(ptr, dStage_dt_c, roomID);
}
DECL(void, TP_Room_ReLoader, void* ptr, u32* dStage_dt_c, int roomID) //Add dynamic modded contents to Rooms
{
	real_TP_Room_ReLoader(ptr, dStage_dt_c, roomID);

	//Grab current Map Info Stuff
	const unsigned char *CurrentStagePtr_TP = (const unsigned char *)0x1064CDE8;
	unsigned char CurrentStage_TP[8];

	const unsigned char *CurrentStatePtr_TP = (const unsigned char *)0x1062915B;
	unsigned char CurrentState_TP[2];

	memcpy(CurrentStage_TP, CurrentStagePtr_TP, 8);
	memcpy(CurrentState_TP, CurrentStatePtr_TP, 2);

	//Spawn special exit zones in City/Lakebed/Palace
	if (!strcmp((const char*)CurrentStage_TP, "D_MN07") && roomID == 0) //City Exit Zone
		SpawnLoadingZone(roomID, -16.655250, -200.00, 11700.000, 0, 20, 30, 25, 0xFF, false, 0x01);
	else if (!strcmp((const char*)CurrentStage_TP, "D_MN07") && roomID == 16) //City Ooccoo Shop Exit Zone
		SpawnLoadingZone(roomID, 0, 0, 650, 0, 25, 20, 15, 0xFF, false, 0x00);
	else if (!strcmp((const char*)CurrentStage_TP, "D_MN01") && roomID == 0) //Lakebed Exit Zone
		SpawnLoadingZone(roomID, -10, 650, 25000, 0, 62, 51, 25, 0xFF, false, 0x00);
	else if (!strcmp((const char*)CurrentStage_TP, "D_MN08") && roomID == 0) //PoT Exit Zone
		SpawnLoadingZone(roomID, 2700, 146, 14870, 0, 20, 10, 5, 0xFF, false, 0x05);


	//Spawn additional bugs in Faron Twilight (ID: 01 = Normal; 03 = Flying by default)
	if (!strcmp((const char*)CurrentStage_TP, "F_SP108") && roomID == 4 && !strcmp((const char*)CurrentState_TP, "e")) //Coro Area
	{
		//Bug near Coro
		SpawnActor(roomID, "E_ym", 0x66FFFF01, -12797, 437, -14501, 0xFF04, 0, 0xFF01, 0xFFFF);
		SpawnActor(roomID, "E_ymt", 0x00000001, -12797, 537, -14872, 0, 0, 0, 0xFFFF);
		SpawnActor(roomID, "Drop", 0x00006604, -12914, 100, -14872, 0, 0, 0, 0xFFFF);
	}
	else if (!strcmp((const char*)CurrentStage_TP, "F_SP108") && roomID == 6 && !strcmp((const char*)CurrentState_TP, "e")) //North Faron
	{
		//Bug after EMS
		SpawnActor(roomID, "E_ym", 0x67FFFF03, -40500, -700, -17750, 0xFF03, 0, 0xFFFF, 0xFFFF);
		SpawnActor(roomID, "Drop", 0x00006709, -41000, -900, -17050, 0, 0, 0, 0xFFFF);
	}
	else if (!strcmp((const char*)CurrentStage_TP, "F_SP108") && roomID == 1 && !strcmp((const char*)CurrentState_TP, "e")) //Spring
	{
		//Tears to Faron Spring
		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x67, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00006709, -18650, 0, -5650, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x71, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x0000710C, -18769, 0, -6597, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x73, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x0000730E, -18769, 0, -6397, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x77, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00007712, -18769, 0, -6197, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x79, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00007914, -18669, 0, -6797, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x7A, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00007A15, -18469, 0, -6897, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x7C, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00007C17, -18369, 0, -6997, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x7E, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00007E08, -18169, 0, -6997, 0, 0, 0, 0xFFFF);
	}

	//Spawn additional bugs in Eldin Twilight (ID: 00 = Normal;  01 = Can stick to walls; 03 = Flying by default; 04 = Can dig)
	if (!strcmp((const char*)CurrentStage_TP, "F_SP109") && roomID == 0 && !strcmp((const char*)CurrentState_TP, "e")) //Kakariko
	{
		//Sanctuary Top
		SpawnActor(roomID, "E_ym", 0x0100FF00, 1220, 1270, 5650, 0xFF00, 0, 0xFFFF, 0xFFFF);
		SpawnActor(roomID, "Drop", 0x00000101, 1350, 1400, 5100, 0, 0, 0, 0xFFFF);

		//Hotspring Water
		SpawnActor(roomID, "E_ym", 0x02FFFF03, -5194, 3033, -2061, 0xFF03, 0, 0xFFFF, 0xFFFF);
		SpawnActor(roomID, "Drop", 0x00000202, -5194, 2933, -2061, 0, 0, 0, 0xFFFF);

		//Top Tower
		SpawnActor(roomID, "E_ym", 0x0300FF00, -2100, 3830, -6200, 0xFF03, 0, 0xFF01, 0xFFFF);
		SpawnActor(roomID, "E_ymt", 0x00000001, -2650, 1477, 2071, 0, 0, 0, 0xFFFF);
		SpawnActor(roomID, "Drop", 0x00000303, -1500, 3930, -6400, 0, 0, 0, 0xFFFF);

		//Tears to Eldin Spring
		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x07, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00000707, 1268, 0, 11918, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x08, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00000808, 376, 0, 11963, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x0C, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00000C0C, 188, 0, 11740, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x0E, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00000E0E, 858, 0, 11294, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x0F, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00000F0F, 1277, 0, 11615, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x10, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00001010, 1733, 0, 11776, 0, 0, 0, 0xFFFF);
	}
	else if (!strcmp((const char*)CurrentStage_TP, "F_SP110") && roomID == 0 && !strcmp((const char*)CurrentState_TP, "e")) //Death Mountain Entrance
	{
		//DM Entrance Bug
		SpawnActor(roomID, "E_ym", 0x07FFFF01, -2400, -4800, 26610, 0xFF03, 0, 0xFFFF, 0xFFFF);
		SpawnActor(roomID, "Drop", 0x00000707, -2400, -5100, 27000, 0, 0, 0, 0xFFFF);
	}
	else if (!strcmp((const char*)CurrentStage_TP, "F_SP110") && roomID == 3 && !strcmp((const char*)CurrentState_TP, "e")) //Death Mountain Main
	{
		//Front Shortcut Bug
		SpawnActor(roomID, "E_ym", 0x0800FF00, -300, -870, -4180, 0xFF00, 0, 0xFFFF, 0xFFFF);
		SpawnActor(roomID, "Drop", 0x00000808, 1100, -600, -4560, 0, 0, 0, 0xFFFF);

		//Back Shortcut Bug
		SpawnActor(roomID, "E_ym", 0x0C00FF00, -3700, 100, -4570, 0xFF00, 0, 0xFFFF, 0xFFFF);
		SpawnActor(roomID, "Drop", 0x00000C0C, -3700, 200, -4570, 0, 0, 0, 0xFFFF);
	}

	//Spawn additional bugs in Lanayru Twilight (ID: 00 = Normal;  01 = Can stick to walls; 03 = Flying by default; 04 = Can dig)
	if (!strcmp((const char*)CurrentStage_TP, "F_SP115") && roomID == 1 && !strcmp((const char*)CurrentState_TP, "d")) //Spring
	{
		//Tears to Lanayru Spring
		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x6C, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00006C2F, -1513, 100, 63, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x6D, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00006D30, 1676, 100, -151, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x6E, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00006E2E, 1410, 100, -689, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x71, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x0000712D, -1134, 100, -641, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x74, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00007431, 1643, 100, -529, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x77, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00007737, -1011, 100, -183, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x79, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00007939, -1985, 100, 569, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x7A, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00007A3A, 1747, 100, 815, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x7D, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00007D3D, 1812, 100, -673, 0, 0, 0, 0xFFFF);

		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x7E, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00007E3E, 0, 100, -300, 0, 0, 0, 0xFFFF);
	}
	else if (!strcmp((const char*)CurrentStage_TP, "F_SP116") && roomID == 0 && !strcmp((const char*)CurrentState_TP, "d")) //Spawn Castle Town Tear in Center as well since Flag Region is different
	{
		if (real_TP_Save_isSwitch((unsigned int*)0x10647b48, 0x6F, 0xFF) == 1)
			SpawnActor(roomID, "Drop", 0x00006F2C, 0, 100, -2231, 0, 0, 0, 0xFFFF);
	}

	//Spawn Bomb Bag and Horse Call chest in Links House
	if (!strcmp((const char*)CurrentStage_TP, "R_SP01") && roomID == 4)
	{
		SpawnActor(roomID, "tboxB0", 0xFF2FF3C0, -100.0f, 0.0f, -200.0f, 0, 0, 0x84FF, 0xFFFF);
		SpawnActor(roomID, "tboxB0", 0xFF1FF3C0, 100.0f, 0.0f, -200.0f, 0, 0, 0x50FF, 0xFFFF);

		const unsigned char *BombBagOwnedPtr = (const unsigned char *)0x10647BF3;
		char *BombBagOwned;

		BombBagOwned = ((char *)BombBagOwnedPtr);

		const unsigned char *HorseCallOwnedPtr = (const unsigned char *)0x10647BF9;
		char *HorseCallOwned;

		HorseCallOwned = ((char *)HorseCallOwnedPtr);

		if (*BombBagOwned == 0xFF && *HorseCallOwned == 0xFF) //Bombbag and Horsecall not owned
		{
			g_checkIntroItemReload = 1;
		}
	}
}
DECL(void, TP_Link_CheckWarpStart, void* classPtr) //Re-add Map Glitch
{
	u32* warpFlagPtr = (u32*)0x1066A740; //+0x502800
	u8 warpFlag = 0;

	memcpy(&warpFlag, warpFlagPtr, 1);

	if (warpFlag == 3)
	{
		log_printf("TP Start Warp");

		u8* LinkPointerPtr = (u8*)0x10681E40;
		u32 LinkPointer = 0;
		u8 LinkStatus = 0;

		//Set new Status
		memcpy(&LinkPointer, LinkPointerPtr, 4);

		if (LinkPointer > 0x10000000)
		{
			u8 *linkStatusPtr = (u8*)LinkPointer + 0xA6;

			memcpy(&LinkStatus, linkStatusPtr, 1);

			if ((LinkStatus & (1 << 6)) == 0)
			{
				*linkStatusPtr = (u8)(LinkStatus + 64);
				DCFlushRange(linkStatusPtr, 1);
			}
		}
	}

	real_TP_Link_CheckWarpStart(classPtr);
}

DECL(int, TP_Link_ProcDead, void* classPtr)
{
	int success = real_TP_Link_ProcDead(classPtr);

	if (success == 1) //Link has died/is currently dying, set GameOver Flag
	{
		isGameOver = 1;

		if (g_hasGameOvered == 1) //keep overwriting the stage/room/spawn address for game over loads
		{
			const unsigned char *StagePtr = (const unsigned char *)0x1064CDF6;
			char *Stage;

			const unsigned char *roomIDPtr = (const unsigned char *)0x1064CE00;
			u8 *roomID;

			const unsigned char *spawnIDPtr = (const unsigned char *)0x1064CDFF;
			u8 *spawnID;

			const unsigned char *layerPtr = (const unsigned char *)0x1064CE01;
			u8 *layer;

			const unsigned char *IsVoidFlagPtr = (const unsigned char *)0x1064CDFE;
			u8 *IsVoidFlag;

			const unsigned char *AnimationIDPtr = (const unsigned char *)0x10648DBB;
			u8 *AnimationID;

			Stage = ((char *)StagePtr);

			roomID = ((u8 *)roomIDPtr);
			spawnID = ((u8 *)spawnIDPtr);
			layer = ((u8 *)layerPtr);

			IsVoidFlag = ((u8 *)IsVoidFlagPtr);
			AnimationID = ((u8 *)AnimationIDPtr);

			memcpy(Stage, &GameOverStage, 8);
			DCFlushRange(Stage, 8);

			*roomID = (u8)GameOverRoom;
			DCFlushRange(roomID, 1);

			*spawnID = GameOverSpawn;
			DCFlushRange(spawnID, 1);

			*layer = GameOverState;
			DCFlushRange(layer, 1);

			if (*IsVoidFlag > 0)
			{
				log_printf("Game Over load - %s, %i, %X, %X", Stage, *roomID, *spawnID, *layer);
			}

			*IsVoidFlag = 0;
			DCFlushRange(IsVoidFlag, 1);
		}
	}

	return success;
}

DECL(void, TP_setNextStage, const char *StageName, short SpawnPoint, signed char roomID, u8 LayerOverride, float currentSpeed, unsigned long param5, int param6, signed char faderType, short param8, int param9, int param10)
{
	g_isLoading = 1;

	//Always set fader type to instant black (13) for speed
	faderType = 0;

	char StageNameLoc[8];
	signed char roomIDLoc;
	short SpawnPointLoc;
	u8 LayerOverrideLoc;

	char finalStageName[8];
	signed char finalRoomID;
	short finalSpawnPoint;
	u8 finalLayerOverride;

	u8 overrideDestination = 0;

	u8 randomizeList = 0;

	if (isGameStart == 1)
	{
		if (isNewGame == 0 && roomID == 1 && SpawnPoint == 0x15 && LayerOverride == 0x0D) //Check destination for Ordon Spring Intro
		{
			isNewGame = 1;

			log_printf("New Game triggered");

			//Set Beta Quest Story Flags Once (Savefile Flag region: 10647B48-10648000)
			const unsigned char *SensePtr = (const unsigned char *)0x10647D7B;
			char *Sense;

			const unsigned char *MidnaZPtr = (const unsigned char *)0x10647D44;
			char *MidnaZ;

			const unsigned char *LinkClothingPtr = (const unsigned char *)0x10647B5D;
			char *LinkClothing;

			const unsigned char *LinkWeaponPtr = (const unsigned char *)0x10647B5E;
			char *LinkWeapon;

			const unsigned char *WolfChainPtr = (const unsigned char *)0x10647D3D;
			char *WolfChain;

			const unsigned char *EponaTamedPtr = (const unsigned char *)0x10647D3E;
			char *EponaTamed;

			const unsigned char *YetaTalkPtr = (const unsigned char *)0x10647D43;
			char *YetaTalk;

			const unsigned char *YetoTalkPtr = (const unsigned char *)0x10647D39;
			char *YetoTalk;

			const unsigned char *HennaTalkPtr = (const unsigned char *)0x10647D70;
			char *HennaTalk;

			const unsigned char *DesertStatePtr = (const unsigned char *)0x10647D78;
			char *DesertState;

			Sense = ((char *)SensePtr);
			*Sense += 8;
			DCFlushRange(Sense, 1);

			MidnaZ = ((char *)MidnaZPtr);
			*MidnaZ += 16;
			DCFlushRange(MidnaZ, 1);

			LinkClothing = ((char *)LinkClothingPtr);
			*LinkClothing = 47;
			DCFlushRange(LinkClothing, 1);

			LinkWeapon = ((char *)LinkWeaponPtr);
			*LinkWeapon = 63;
			DCFlushRange(LinkWeapon, 1);

			WolfChain = ((char *)WolfChainPtr);
			*WolfChain += 16;
			DCFlushRange(WolfChain, 1);

			EponaTamed = ((char *)EponaTamedPtr);
			*EponaTamed += 1;
			DCFlushRange(EponaTamed, 1);

			YetaTalk = ((char *)YetaTalkPtr);
			*YetaTalk += 32;
			DCFlushRange(YetaTalk, 1);

			YetoTalk = ((char *)YetoTalkPtr);
			*YetoTalk += 64;
			DCFlushRange(YetoTalk, 1);

			HennaTalk = ((char *)HennaTalkPtr);
			*HennaTalk += 6;
			DCFlushRange(HennaTalk, 1);

			DesertState = ((char *)DesertStatePtr);
			*DesertState += 8;
			DCFlushRange(DesertState, 1);

			randomizeList = 1;
			warpSet = 0;
		}
		else if (isNewGame == 0 && isFileLoad == 0) //any file load
		{
			isFileLoad = 1;

			//Reset Rupee Collection Flag on every file load
			const unsigned char *RupeesCollectedPtr = (const unsigned char *)0x10647C17;
			char *RupeesCollected;

			RupeesCollected = ((char *)RupeesCollectedPtr);

			if (IsBitSet(*RupeesCollected, 0) == 1)
				*RupeesCollected = 3;
			else
				*RupeesCollected = 2;

			DCFlushRange(RupeesCollected, 1);

			//Check for Prison Savewarp Position and normalize it
			if (!strcmp(StageName, "R_SP107") && roomID == 0 && (u8)(SpawnPoint) == 0x18)
			{
				memcpy(&LastStage, StageName, 8);
				LastRoom = roomID;
				LastSpawn = (u8)SpawnPoint;
				LastState = LayerOverride;

				warpSet = 1;
			}

			//Check for Sewers entrance spawn and normalize it
			if (!strcmp(StageName, "R_SP107") && roomID == 1 && (u8)(SpawnPoint) == 0x00)
			{
				memcpy(&LastStage, StageName, 8);
				LastRoom = roomID;
				LastSpawn = (u8)SpawnPoint;
				LastState = LayerOverride;

				warpSet = 1;
			}

			//Check for MDH Savewarp Position and normalize it
			if (!strcmp(StageName, "R_SP107") && roomID == 1 && (u8)(SpawnPoint) == 0x07)
			{
				memcpy(&LastStage, StageName, 8);
				LastRoom = roomID;
				LastSpawn = (u8)SpawnPoint;
				LastState = LayerOverride;

				warpSet = 1;
			}

			//Check for Sewers rooftops spawn and normalize it
			if (!strcmp(StageName, "R_SP107") && roomID == 2 && (u8)(SpawnPoint) == 0x00)
			{
				memcpy(&LastStage, StageName, 8);
				LastRoom = roomID;
				LastSpawn = (u8)SpawnPoint;
				LastState = LayerOverride;

				warpSet = 1;
			}


			//Store Save Location for restoration later (prevents corruption)
			memcpy(&SaveLocationStage, StageName, 8);
			SaveLocationRoom = roomID;
			SaveLocationSpawn = (u8)SpawnPoint;

			randomizeList = 1;
		}
	}

	if (randomizeList == 1) //Randomize List Code
	{
		randomizeList = 0;

		const unsigned char *NamePtr = (const unsigned char *)0x10647CFC;
		const char *Name = ((const char *)NamePtr);

		u64 fileSeed = 0;
		u8 sign = 0;
		u8 signsToCopy = 0;

		if (strcmp(Name, LastUsedName)) //Only create list whenever the file name changes
		{
			memcpy(&LastUsedName, Name, 8);

			if (!strcmp(Name, "Link"))
			{
				listRandomized = 0; //Dont randomize list if file name is Link
			}
			else
			{
				//Get File Seed
				for (int n = 0; n < 7; n++)
				{
					memcpy(&sign, NamePtr + n, 1);
	
					if (sign == 0)
						break;

					signsToCopy++;
				}

				signsToCopy = 8 - signsToCopy;

				int z = 0;
				u32 ptr = (u32)&fileSeed;

				for (int n = signsToCopy; n < 8; n++)
				{
					memcpy((u8*)ptr + n, NamePtr + z, 1);

					z++;		
				}

				log_printf("New Filename Seed is: %llu", fileSeed);

				//Init Random List
				AllSpawnsRandom = AllSpawns;

				int sizeList = AllSpawns.size();

				for (int n = sizeList - 1; n >= 0; n--)
				{
					u64 newIndex = fileSeed * (n + 1); //OLD: (u64)((fileSeed * (n + 1)) / 1.12345);

					//Debug Code: Verify Spawn List
					if (AllSpawns[n].Stage[0] == 0)
					{
						log_printf("ERROR: Entry %i in Spawn List is empty!!!", n);
					}

					newIndex = newIndex % sizeList;

					Spawn tempValue = AllSpawnsRandom[n];
					AllSpawnsRandom[n] = AllSpawnsRandom[newIndex];

					AllSpawnsRandom[newIndex] = tempValue;
				}

				listRandomized = 1;

				//Zone Lookup Code
				/*
				for (int n = sizeList - 1; n >= 0; n--)
				{
					Spawn tempValue = AllSpawnsRandom[n];

					if (!strcmp(tempValue.Stage, "D_MN06"))
						log_printf("D_MN06 is reached by: %s->%i->%X", AllSpawns[n].Stage, AllSpawns[n].RoomID, AllSpawns[n].SpawnID);
				}
				*/
			}
		}
	}

	log_printf("Original target load is: %s-%i-%X-%X ; warpSet: %i", StageName, roomID, SpawnPoint, LayerOverride, warpSet);

	u8 doRandomWarp = 1;
	u8 doGameOverWarp = 0;

	//Trying to load Title Screen, reset warp to normal
	if ((!strcmp(StageName, "OPENING")) || (!strcmp(StageName, "F_SP102") && roomID == 0 && (u8)SpawnPoint == 0x64 && LayerOverride == 0x0A))
	{
		strcpy(LastStage, "F_SP102");
		LastRoom = 0;
		LastSpawn = 0x64;
		LastState = 0x0A;

		strcpy(finalStageName, "F_SP102");
		finalRoomID = 0;
		finalSpawnPoint = 0x64;
		finalLayerOverride = 0x0A;

		doRandomWarp = 0;
		warpSet = 1;
		overrideDestination = 1;

		log_printf("Load Title Screen!");
	}

	if (listRandomized == 1) //Random Warp Code
	{
		if (warpSet == 0)
		{
			memcpy(&StageNameLoc, StageName, 8);
			roomIDLoc = roomID;
			SpawnPointLoc = SpawnPoint;
			LayerOverrideLoc = LayerOverride;

			if (isNewGame == 1) //Set warp to Links House
			{
				strcpy(finalStageName, "R_SP01");
				finalRoomID = 7;
				finalSpawnPoint = 0x00;
				finalLayerOverride = 0xFF;

				log_printf("Start new game");

				doRandomWarp = 0;
				warpSet = 1;

				overrideDestination = 1;
			}
			else if ((isGameOver == 1) && ((u8)(SpawnPoint) == 0xFF || !strcmp(LastStage, "D_SB01") || !strcmp(LastStage, "D_SB11"))) //normal game over is randomized based on last entrance ; CoS/CoO as an exception since it's buggy on death
			{
				memcpy((char*)StageNameLoc, &LastStage, 8);
				roomIDLoc = LastRoom;
				SpawnPointLoc = LastSpawn;
				LayerOverrideLoc = LastState;

				warpSet = 1;
				doGameOverWarp = 1;

				log_printf("Game Over");
			}
			else
			{
				//Check for dungeon warp
				const unsigned char *CurrentStagePtr = (const unsigned char *)0x1064CDE8;
				const char *CurrentStage;

				CurrentStage = ((const char *)CurrentStagePtr);

				if (strstr(CurrentStage, "D_MN") != 0) //Current Stage is dungeon
				{
					if (strstr(CurrentStage, "A") != 0 || strstr(CurrentStage, "D_MN08D") != 0 || strstr(CurrentStage, "D_MN09B") != 0 || strstr(CurrentStage, "D_MN09C") != 0) //Current Stage is boss, normalize
					{
						doRandomWarp = 0;
						warpSet = 1;
					}
					else if (strstr(StageName, "D_MN") != 0) //Next Stage is also dungeon
					{
						if (strstr(StageName, "A") == 0) //Next Stage is not a boss, thus normalize
						{
							doRandomWarp = 0;
							warpSet = 1;
						}
					}
				}

				if (!strcmp(CurrentStage, "R_SP107") && !strcmp(StageName, "R_SP107")) //Current Stage and next Stage is Sewers, normalize
				{
					doRandomWarp = 0;
					warpSet = 1;
				}

				if (!strcmp(CurrentStage, "F_SP126") && !strcmp(StageName, "F_SP112") && roomID == 1 && SpawnPoint == 0x02) //If spawn is Izas hut from UZR Portal CS normalize it
				{
					doRandomWarp = 0;
					warpSet = 1;
				}
			}

			//Locate destination in the normal warp list for lookup purposes
			/*
			int sizeList = AllSpawns.size();

			for (int n = sizeList - 1; n >= 0; n--)
			{
			Spawn spawn = AllSpawnsRandom[n];

			if (!strcmp(spawn.Stage, "F_SP113") && spawn.RoomID == 1 && spawn.SpawnID == 0x0B) //F_SP200
			{
			Spawn source = AllSpawns[n];

			log_printf("%s:%i:%X links to %s:%i:%X", source.Stage, source.RoomID, source.SpawnID, spawn.Stage, spawn.RoomID, spawn.SpawnID);

			memcpy((char*)StageNameLoc, &source.Stage, 8);
			roomIDLoc = source.RoomID;
			SpawnPointLoc = source.SpawnID;
			LayerOverrideLoc = 0xFF;
			}
			}
			*/

			if (doRandomWarp == 1) //Perform random warp
			{
				int sizeList = AllSpawns.size();

				for (int n = sizeList - 1; n >= 0; n--)
				{
					Spawn spawn = AllSpawns[n];

					if (!strcmp(spawn.Stage, StageNameLoc) && spawn.RoomID == roomIDLoc && spawn.SpawnID == SpawnPointLoc)
					{
						Spawn destination = AllSpawnsRandom[n];

						int countDown = 1;

						while (destination.EntranceType == 0xD0 || destination.EntranceType == 0xE0) //Find other spawn if its a potentially crashy one
						{
							log_printf("Invalid destination at id: %i! Crash risk, advance...", n);

							if (n < 2)
								countDown = 0;

							if (countDown == 1)
								n--;
							else
								n++;

							destination = AllSpawnsRandom[n];
						}

						if (doGameOverWarp == 0)
						{
							memcpy(&finalStageName, &destination.Stage, 8);
							finalRoomID = destination.RoomID;
							finalSpawnPoint = destination.SpawnID;
							finalLayerOverride = (u8)(destination.Modifier);
							finalLayerOverride = finalLayerOverride << 4;
							finalLayerOverride = finalLayerOverride >> 4;

							if (finalLayerOverride == 0xF)
								finalLayerOverride = 0xFF;

							log_printf("Found random spawn normal: %s, %i, %X, %X", &finalStageName, finalRoomID, finalSpawnPoint, finalLayerOverride);
							warpSet = 1;

							overrideDestination = 1;
						}
						else
						{
							memcpy((char*)GameOverStage, &destination.Stage, 8);
							GameOverRoom = destination.RoomID;
							GameOverSpawn = destination.SpawnID;
							GameOverState = (u8)(destination.Modifier);
							GameOverState = GameOverState << 4;
							GameOverState = GameOverState >> 4;

							if (GameOverState == 0xF)
								GameOverState = 0xFF;

							log_printf("Found random spawn game over: %s, %i, %X, %X", GameOverStage, GameOverRoom, GameOverSpawn, GameOverState);

							memcpy(&LastStage, &destination.Stage, 8);
							LastRoom = destination.RoomID;
							LastSpawn = destination.SpawnID;
							LastState = GameOverState;

							g_hasGameOvered = 1;
						}

						break;
					}
				}
			}
		}
		else //use prior warp if its already set
		{
			memcpy(&finalStageName, &LastStage, 8);
			finalRoomID = LastRoom;
			finalSpawnPoint = LastSpawn;
			finalLayerOverride = LastState;

			overrideDestination = 1;
		}
	}

	if (overrideDestination == 1)
	{
		log_printf("Load random Area: %s->%i->%04X->%02X ; Fader: %X", &finalStageName, finalRoomID, finalSpawnPoint, finalLayerOverride, faderType);
	}
	else
	{
		log_printf("Load default Area: %s->%i->%04X->%02X ; Fader: %X", StageName, roomID, SpawnPoint, LayerOverride, faderType);

		memcpy(&finalStageName, StageName, 8);
		finalRoomID = roomID;
		finalSpawnPoint = (u8)SpawnPoint;
		finalLayerOverride = LayerOverride;
	}

	//Save warp as last valid warp if not void/game over
	if ((u8)(finalSpawnPoint) != 0xFF && g_hasGameOvered == 0)
	{
		memcpy(&LastStage, &finalStageName, 8);
		LastRoom = finalRoomID;
		LastSpawn = (u8)finalSpawnPoint;
		LastState = finalLayerOverride;
	}


	//log_printf("Param5: %16X, Param6: %X, Param8: %04X, Param9: %X, Param10: %X", param5, param6, param8, param9, param10);


	//If spawn is Morpheel 00 or 01 do exception spawn to avoid softlock
	if (g_hasGameOvered == 1)
	{
		if (!strcmp(GameOverStage, "D_MN01A") && GameOverRoom == 50)
		{
			if ((u8)(GameOverSpawn) == 0x00 || (u8)(GameOverSpawn) == 0x01)
				GameOverSpawn = 3;
		}
	}
	else if ((u8)(finalSpawnPoint) != 0xFF)
	{
		if (!strcmp(finalStageName, "D_MN01A") && finalRoomID == 50)
		{
			if ((u8)(finalSpawnPoint) == 0x00 || (u8)(finalSpawnPoint) == 0x01)
				finalSpawnPoint = 3;

		}
	}

	//If spawn is Telmas Bar do exception spawns to avoid softlocks
	if (g_hasGameOvered == 1)
	{
		if (!strcmp(GameOverStage, "R_SP116") && GameOverRoom == 5)
		{
			u8 targetLayer = 0xFF;

			if (GameOverState == 0xFF)
				targetLayer = real_TP_getLayerNo("R_SP116", 5, -1);
			else
				targetLayer = real_TP_getLayerNo("R_SP116", 5, GameOverState);

			if (targetLayer == 2) //MDH State, avoid bottom spawns because of Goron crash
			{
				GameOverSpawn = 2;
			}
			else if (targetLayer == 0xE || targetLayer == 0xD) //Twilight States, avoid door because it doesn't exist
			{
				if ((u8)(GameOverSpawn) == 0x00)
				{
					GameOverSpawn = 0x1E;
				}
			}
		}
	}
	else if ((u8)(finalSpawnPoint) != 0xFF)
	{
		if (!strcmp(finalStageName, "R_SP116") && finalRoomID == 5)
		{
			u8 targetLayer = 0xFF;

			if (finalLayerOverride == 0xFF)
				targetLayer = real_TP_getLayerNo("R_SP116", 5, -1);
			else
				targetLayer = real_TP_getLayerNo("R_SP116", 5, finalLayerOverride);

			if (targetLayer == 2)
			{
				finalSpawnPoint = 2;
			}
			else if (targetLayer == 0xE || targetLayer == 0xD) //Twilight States, avoid door because it doesn't exist
			{
				if ((u8)(finalSpawnPoint) == 0x00)
				{
					finalSpawnPoint = 0x1E;
				}
			}
		}
	}


	//If spawn is South Castle Town do exception spawn to avoid softlock
	if (g_hasGameOvered == 1)
	{
		if (!strcmp(GameOverStage, "F_SP116") && GameOverRoom == 3)
		{
			u8 targetLayer = real_TP_getLayerNo("F_SP116", 3, -1);

			if (targetLayer == 0xE || targetLayer == 0xD || targetLayer == 0x2) //Twilight or MDH States, avoid door because it doesn't exist
			{
				if ((u8)(GameOverSpawn) == 1)
					GameOverSpawn = 0x1E;
			}
		}
	}
	else if ((u8)(finalSpawnPoint) != 0xFF)
	{
		if (!strcmp(finalStageName, "F_SP116") && finalRoomID == 3)
		{
			u8 targetLayer = real_TP_getLayerNo("F_SP116", 3, -1);

			if (targetLayer == 0xE || targetLayer == 0xD || targetLayer == 0x2) //Twilight or MDH States, avoid door because it doesn't exist
			{
				if ((u8)(finalSpawnPoint) == 1)
					finalSpawnPoint = 0x1E;
			}
		}
	}

	//If spawn is broken door in Kak do exception spawn to avoid softlock
	if (g_hasGameOvered == 1)
	{
		if (!strcmp(GameOverStage, "F_SP109") && GameOverRoom == 0)
		{
			if ((u8)(GameOverSpawn) == 0x2B)
				GameOverSpawn = 3;
		}
	}
	else if ((u8)(finalSpawnPoint) != 0xFF)
	{
		if (!strcmp(finalStageName, "F_SP109") && finalRoomID == 0)
		{
			if ((u8)(finalSpawnPoint) == 0x2B)
				finalSpawnPoint = 3;
		}
	}

	//If spawn is Hennas Boat Ride, Lake Hylia Boat, Fortune Teller or Hidden Skill Learning force Human to avoid issues
	if (!strcmp(finalStageName, "F_SP127") && finalRoomID == 0 && finalSpawnPoint == 0x04)
	{
		const unsigned char *CurrentFormPtr = (const unsigned char *)0x10647B68;
		char *CurrentForm;

		CurrentForm = ((char *)CurrentFormPtr);

		const unsigned char *MSOwnedPtr = (const unsigned char *)0x10647D45;
		char *MSOwned;

		MSOwned = ((char *)MSOwnedPtr);

		if (IsBitSet(*MSOwned, 2) == 1) //MS was obtained, just force human form once
		{
			if (*CurrentForm == 1)
			{
				*CurrentForm = 0;
				DCFlushRange(CurrentForm, 1);
			}
		}
		else
		{
			g_forceHuman = 1; //force human for longer for non MS cases

			*MSOwned = *MSOwned + 4;
			DCFlushRange(MSOwned, 1);

			if (*CurrentForm == 1)
			{
				*CurrentForm = 0;
				DCFlushRange(CurrentForm, 1);
			}
		}
	}
	else if (!strcmp(finalStageName, "F_SP115") && finalRoomID == 0 && finalSpawnPoint == 0x02)
	{
		const unsigned char *CurrentFormPtr = (const unsigned char *)0x10647B68;
		char *CurrentForm;

		CurrentForm = ((char *)CurrentFormPtr);

		const unsigned char *MSOwnedPtr = (const unsigned char *)0x10647D45;
		char *MSOwned;

		MSOwned = ((char *)MSOwnedPtr);

		if (IsBitSet(*MSOwned, 2) == 0) //If MS was obtained, don't do anything since the player should know what he is doing
		{
			g_forceHuman = 1; //force human for longer for non MS cases

			*MSOwned = *MSOwned + 4;
			DCFlushRange(MSOwned, 1);

			if (*CurrentForm == 1)
			{
				*CurrentForm = 0;
				DCFlushRange(CurrentForm, 1);
			}
		}
	}
	else if (!strcmp(finalStageName, "F_SP200") && finalRoomID == 0 && (finalSpawnPoint == 0x00 || finalSpawnPoint == 0x01))
	{
		const unsigned char *CurrentFormPtr = (const unsigned char *)0x10647B68;
		char *CurrentForm;

		CurrentForm = ((char *)CurrentFormPtr);

		const unsigned char *MSOwnedPtr = (const unsigned char *)0x10647D45;
		char *MSOwned;

		MSOwned = ((char *)MSOwnedPtr);

		if (IsBitSet(*MSOwned, 2) == 0) //If MS was obtained, don't do anything since you can't enter the golden wolf as wolf link
		{
			g_forceHuman = 1; //force human for longer for non MS cases

			*MSOwned = *MSOwned + 4;
			DCFlushRange(MSOwned, 1);

			if (*CurrentForm == 1)
			{
				*CurrentForm = 0;
				DCFlushRange(CurrentForm, 1);
			}
		}
	}
	else if (!strcmp(finalStageName, "R_SP160") && finalRoomID == 1)
	{
		const unsigned char *CurrentFormPtr = (const unsigned char *)0x10647B68;
		char *CurrentForm;

		CurrentForm = ((char *)CurrentFormPtr);

		const unsigned char *MSOwnedPtr = (const unsigned char *)0x10647D45;
		char *MSOwned;

		MSOwned = ((char *)MSOwnedPtr);

		if (IsBitSet(*MSOwned, 2) == 1) //MS was obtained, just force human form once
		{
			if (*CurrentForm == 1)
			{
				*CurrentForm = 0;
				DCFlushRange(CurrentForm, 1);
			}
		}
		else
		{
			g_forceHuman = 1; //force human for longer for non MS cases

			*MSOwned = *MSOwned + 4;
			DCFlushRange(MSOwned, 1);

			if (*CurrentForm == 1)
			{
				*CurrentForm = 0;
				DCFlushRange(CurrentForm, 1);
			}
		}
	}

	//Locate destination in list again to apply time of day override and form changes
	int sizeList = AllSpawns.size();

	char lookUpStage[8];
	int lookUpRoom = 0;
	u8 lookUpSpawn = 0;

	if (g_hasGameOvered == 1)
	{
		memcpy(&lookUpStage, &GameOverStage, 8);
		lookUpRoom = GameOverRoom;
		lookUpSpawn = GameOverSpawn;
	}
	else
	{
		memcpy(&lookUpStage, &finalStageName, 8);
		lookUpRoom = finalRoomID;
		lookUpSpawn = (u8)(finalSpawnPoint);
	}

	for (int n = sizeList - 1; n >= 0; n--)
	{
		Spawn spawn = AllSpawns[n];

		if (!strcmp(spawn.Stage, lookUpStage) && spawn.RoomID == lookUpRoom && spawn.SpawnID == lookUpSpawn)
		{
			//ToD Override Handling
			const unsigned char *NextToDPtr = (const unsigned char *)0x10668314;
			float *NextToD;

			NextToD = ((float *)NextToDPtr);

			u16 modifier = spawn.Modifier;

			u8 timeToAdd = modifier >> 12;

			u8 baseTime = (u8)(modifier);
			baseTime = baseTime >> 4;

			int currentHours = 0;

			if (IsBitSet(baseTime, 0) == 1)
			{
				if (timeToAdd == 0xF)
				{
					currentHours = -1;
				}
				else
				{
					currentHours = 16;
				}
			}
			else
			{
				currentHours = 0;
			}

			if (currentHours == -1)
			{
				*NextToD = -1.0f;
				DCFlushRange(NextToD, 4);

				log_printf("Next time is -1");
			}
			else
			{
				currentHours = currentHours + timeToAdd;

				if (currentHours > 23)
					currentHours = 0;

				int currentMinutes = currentHours * 60;

				*NextToD = currentMinutes / 4.0f;
				DCFlushRange(NextToD, 4);

				log_printf("Next time is custom");
			}

			//Door Handling as Wolf
			u8 spawnType = spawn.EntranceType;

			if (spawnType == 0xA0 || spawnType == 0xB0 || spawnType == 0x80) //Spawn leads to a door or a post dungeon warp
			{
				//If spawn is a door in Kak Twilight, do exception spawn to avoid possible softlocks
				if (spawnType == 0xA0 || spawnType == 0xB0)
				{
					if (g_hasGameOvered == 0)
					{
						if (!strcmp(finalStageName, "F_SP109") && finalRoomID == 0)
						{
							u8 targetLayer = 0xFF;

							if (finalLayerOverride == 0xFF)
								targetLayer = real_TP_getLayerNo("F_SP109", 0, -1);
							else
								targetLayer = real_TP_getLayerNo("F_SP109", 0, finalLayerOverride);

							if (targetLayer == 0xE)
							{
								finalSpawnPoint = 0x2E;
							}
						}
					}
					else
					{
						if (!strcmp(GameOverStage, "F_SP109") && GameOverRoom == 0)
						{
							u8 targetLayer = 0xFF;

							if (GameOverState == 0xFF)
								targetLayer = real_TP_getLayerNo("F_SP109", 0, -1);
							else
								targetLayer = real_TP_getLayerNo("F_SP109", 0, GameOverState);

							if (targetLayer == 0xE)
							{
								GameOverSpawn = 0x2E;
							}
						}
					}
				}

				const unsigned char *CurrentFormPtr = (const unsigned char *)0x10647B68;
				char *CurrentForm;

				CurrentForm = ((char *)CurrentFormPtr);

				const unsigned char *MSOwnedPtr = (const unsigned char *)0x10647D45;
				char *MSOwned;

				MSOwned = ((char *)MSOwnedPtr);

				if (IsBitSet(*MSOwned, 2) == 1) //MS was obtained, just force human form once
				{
					if (*CurrentForm == 1)
					{
						*CurrentForm = 0;
						DCFlushRange(CurrentForm, 1);
					}
				}
				else
				{
					g_forceHuman = 1; //force human for longer for non MS cases

					*MSOwned = *MSOwned + 4;
					DCFlushRange(MSOwned, 1);

					if (*CurrentForm == 1)
					{
						*CurrentForm = 0;
						DCFlushRange(CurrentForm, 1);
					}
				}
			}

			break;
		}
	}

	real_TP_setNextStage(StageName, SpawnPoint, roomID, LayerOverride, currentSpeed, param5, param6, faderType, param8, param9, param10);


	//Overwrite Destination with random or normal warp	
	const unsigned char *StagePtr = (const unsigned char *)0x1064CDF6;
	char *NewStage;

	const unsigned char *roomIDPtr = (const unsigned char *)0x1064CE00;
	u8 *NewRoomID;

	const unsigned char *spawnIDPtr = (const unsigned char *)0x1064CDFF;
	u8 *NewSpawnID;

	const unsigned char *spawnIDModifierPtr = (const unsigned char *)0x10648DA1;
	u8 *NewSpawnIDModifier;

	const unsigned char *layerPtr = (const unsigned char *)0x1064CE01;
	u8 *NewLayer;

	NewStage = ((char *)StagePtr);

	NewRoomID = ((u8 *)roomIDPtr);
	NewSpawnID = ((u8 *)spawnIDPtr);
	NewSpawnIDModifier = ((u8 *)spawnIDModifierPtr);
	NewLayer = ((u8 *)layerPtr);

	memcpy(NewStage, &finalStageName, 8);
	DCFlushRange(NewStage, 8);

	*NewRoomID = (u8)finalRoomID;
	DCFlushRange(NewRoomID, 1);

	*NewSpawnID = (u8)finalSpawnPoint;
	DCFlushRange(NewSpawnID, 1);

	*NewSpawnIDModifier = (u8)finalSpawnPoint;
	DCFlushRange(NewSpawnIDModifier, 1);

	*NewLayer = finalLayerOverride;
	DCFlushRange(NewLayer, 1);


	//Check for Epona/Ooccoo spawns to avoid parallel universe glitch as Wolf
	const unsigned char *AnimationIDPtr = (const unsigned char *)0x10648DBB;
	u8 *AnimationID;

	AnimationID = ((u8 *)AnimationIDPtr);

	if (*AnimationID == 1 || *AnimationID == 8 || *AnimationID == 72 || *AnimationID == 12) //1 (Epona in load zone) or 8 (Game Over on Epona) or 72 (Void on Epona) or 12 (Ooccoo Warp)
	{
		if (!strcmp(finalStageName, "D_SB11")) //Possible Epona spawn in Cave of Shadows, remove Epona to avoid parallel universe glitch! Usually only a concern for Game Overs
		{
			if (*AnimationID == 1 || *AnimationID == 8)
			{
				*AnimationID = 0;
				DCFlushRange(AnimationID, 1);
			}
		}
		else
		{
			const unsigned char *CurrentFormPtr = (const unsigned char *)0x10647B68;
			char *CurrentForm;

			CurrentForm = ((char *)CurrentFormPtr);

			const unsigned char *MSOwnedPtr = (const unsigned char *)0x10647D45;
			char *MSOwned;

			MSOwned = ((char *)MSOwnedPtr);

			if (IsBitSet(*MSOwned, 2) == 1) //MS was obtained, just force human form once
			{
				if (*CurrentForm == 1)
				{
					*CurrentForm = 0;
					DCFlushRange(CurrentForm, 1);
				}
			}
			else
			{
				g_forceHuman = 1; //force human for longer for non MS cases

				*MSOwned = *MSOwned + 4;
				DCFlushRange(MSOwned, 1);

				if (*CurrentForm == 1)
				{
					*CurrentForm = 0;
					DCFlushRange(CurrentForm, 1);
				}

				if (*AnimationID == 12) //Force a reload after Ooccoo so player can become Wolf again
				{
					g_forceReload = 1;
				}
			}
		}
	}
}

DECL(void, TP_gameStart, void)
{
	if (ExceptionHooksCore1 == 0)
	{
		SetupOSExceptions(); //re-direct Exceptions in Core 1 as well
		ExceptionHooksCore1 = 1;
	}

	isGameStart = 1;

	real_TP_gameStart();
}

DECL(int, TP_isEventBit, unsigned int* eventSectionPtr, u16 flagBitMask)
{
	u32 linkRegister = 0;
	asm volatile("mflr %0" : "=r" (linkRegister));

	int result = real_TP_isEventBit(eventSectionPtr, flagBitMask);

	u8 flagBit = (u8)flagBitMask;
	u8 flagID = flagBitMask >> 8;

	unsigned int flagAddress = (unsigned int)eventSectionPtr + flagID;

	if (msgQuery_checkEventFlag == 1) //Msg Query is in Progress and the Player is Wolf without MS
	{
		msgQuery_checkEventFlag = 0;

		//log_printf("Checking Flag at Address: 0x%X for bit: %i! Result is: %i", flagAddress, flagBit, result);

		if (flagAddress == 0x10647D45 && flagBit == 4) //MS owned Flag is checked, return 1
		{
			return 1;
		}
	}

	//Game checks if the Mirror needs to be raised still before allowing warping, remove this block
	if (linkRegister == 0x2A633A4 || linkRegister == 0x29D78D4)
	{
		if (flagAddress == 0x10647D64 && flagBit == 16)
		{
			log_printf("Mirror was pretended to be raised to allow warping!");
			return 1;
		}
	}

	return result;
}

DECL(void, TP_onEventBit, unsigned int* eventSectionPtr, u16 flagBitMask)
{
	u8 flagBit = (u8)flagBitMask;
	u8 flagID = flagBitMask >> 8;

	unsigned int flagAddress = (unsigned int)eventSectionPtr + flagID;

	//log_printf("Set flag: %X with bit: %i!", flagAddress, flagBit);

	//Check for Map Warp flag being set at the Gorge
	if (flagAddress == 0x10647D3E && flagBit == 4)
	{
		//Check if player has a Faron Woods Warp unlocked
		const unsigned char *NFaronPtr = (const unsigned char *)0x10647E83; //bit 4 set 
		char *NFaron;

		NFaron = ((char *)NFaronPtr);

		const unsigned char *SFaronPtr = (const unsigned char *)0x10647E8B; //bit 128 set 
		char *SFaron;

		SFaron = ((char *)SFaronPtr);

		const unsigned char *GrovePtr = (const unsigned char *)0x10647F2F; //bit 16 set 
		char *Grove;

		Grove = ((char *)GrovePtr);

		if (IsBitSet(*NFaron, 2) == 0 && IsBitSet(*SFaron, 7) == 0 && IsBitSet(*Grove, 4) == 0)
		{
			//Player has no Faron Warp unlocked, unset map warping flags and safety load to prevent softlock on map screen
			real_TP_Save_offSwitch((unsigned int*)0x10647B48, 0x17, 3);
			real_TP_Save_offSwitch((unsigned int*)0x10647B48, 0x18, 3);

			g_doGorgeFallback = 1;

			log_printf("Attempt to set map warp flag without a Faron Woods Warp unlocked! Fallback...");

			return;
		}
	}

	real_TP_onEventBit(eventSectionPtr, flagBitMask);
}

DECL(int, TP_Query_checkAcceptTransform, void* classPtr, void* mesg_flow_node_branch, void* fopAc_ac_c, int param1) //param1 is usually 1
{
	int result = real_TP_Query_checkAcceptTransform(classPtr, mesg_flow_node_branch, fopAc_ac_c, param1);

	const unsigned char *CurrentFormPtr = (const unsigned char *)0x10647B68;
	unsigned char CurrentForm[1];

	memcpy(CurrentForm, CurrentFormPtr, 1);

	//log_printf("Transform was checked. Result is: %i", result); //0 = accepted, 1 = normal block , 2 = normal block, 3 = fog block, 4 = castle town uproar block

	if (CurrentForm[0] == 1 && result != 0) //Allow Transforms regardless when Wolf-Human
	{
		log_printf("Wolf-Human Transform granted despite block!");
		return 0;
	}

	return result;
}

DECL(int, TP_Query_checkEventFlag, unsigned int* classPtr, unsigned int* mesg_flow_node_branch, unsigned int* fopAc_ac_c, int param1)
{
	//Check
	const unsigned char *CurrentFormPtr = (const unsigned char *)0x10647B68;
	unsigned char CurrentForm[1];

	memcpy(CurrentForm, CurrentFormPtr, 1);

	if (real_TP_isEventBit((unsigned int*)0x10647D38, 0x0D04) == 0) //MS not Owned
	{
		if (CurrentForm[0] == 1) //is Wolf
		{
			msgQuery_checkEventFlag = 1;
		}
	}

	int result = real_TP_Query_checkEventFlag(classPtr, mesg_flow_node_branch, fopAc_ac_c, param1);

	return result;
}

DECL(int, TP_Query_checkAcceptDungeonWarp, unsigned int* classPtr, unsigned int* mesg_flow_node_branch, unsigned int* fopAc_ac_c, int param1)
{
	int result = real_TP_Query_checkAcceptDungeonWarp(classPtr, mesg_flow_node_branch, fopAc_ac_c, param1);

	if (result == 0) //Warp is allowed
	{
		const unsigned char *CurrentFormPtr = (const unsigned char *)0x10647B68;
		unsigned char CurrentForm[1];

		memcpy(CurrentForm, CurrentFormPtr, 1);

		if (real_TP_isEventBit((unsigned int*)0x10647D38, 0x0D04) == 0) //MS not Owned
		{
			if (CurrentForm[0] == 1) //is Wolf
			{
				if (real_TP_isEventBit((unsigned int*)0x10647D38, 0x0604) == 0 && real_TP_isEventBit((unsigned int*)0x10647D38, 0x1E08) == 0) //Midna/Z Warping hasnt been unlocked yet, remove Warp Option
				{
					return 1; //Warp not allowed
				}
			}
		}
	}

	return result;
}

DECL(unsigned int*, TP_getRubyFont, void)
{
	unsigned int* result = real_TP_getRubyFont();

	return result;
}

DECL(unsigned int*, TP_getMesgFont, void)
{
	unsigned int* result = 0;

	if (useRubyFont == 1)
	{
		result = real_TP_getRubyFont();
	}
	else
	{
		result = real_TP_getMesgFont();
	}

	return result;
}

DECL(void, TP_Title_loadWait_proc, void* classPtr)
{
	useRubyFont = 1;

	real_TP_Title_loadWait_proc(classPtr);

	useRubyFont = 0;
}

DECL(void, TP_Menu_dataWrite, unsigned int* classPtr)
{
	log_printf("Save in Progress");

	const unsigned char *CurrentStagePtr = (const unsigned char *)0x1064CDE8;
	const char *CurrentStage;

	const unsigned char *CurrentRoomPtr = (const unsigned char *)0x106813D4;
	char *CurrentRoom;

	const unsigned char *CurrentStatePtr = (const unsigned char *)0x1062915B;
	const char *CurrentState;

	const unsigned char *SaveStagePtr = (const unsigned char *)0x10647BA0;
	char *SaveStage;

	const unsigned char *SaveRoomPtr = (const unsigned char *)0x10647BA9;
	char *SaveRoom;

	const unsigned char *SaveSpawnPtr = (const unsigned char *)0x10647BA8;
	char *SaveSpawn;

	const unsigned char *MSOwnedPtr = (const unsigned char *)0x10647D45;
	char *MSOwned;

	CurrentStage = ((const char *)CurrentStagePtr);
	CurrentRoom = ((char *)CurrentRoomPtr);
	CurrentState = ((const char *)CurrentStatePtr);
	SaveStage = ((char *)SaveStagePtr);
	SaveRoom = ((char *)SaveRoomPtr);
	SaveSpawn = ((char *)SaveSpawnPtr);

	MSOwned = ((char *)MSOwnedPtr);

	if (strstr(CurrentStage, "D_MN01") != 0) //Lakebed
	{
		if (strstr(SaveStage, "D_MN01") == 0)
		{
			strcpy(SaveStage, "D_MN01");
			DCFlushRange(SaveStage, 8);

			*SaveRoom = 0;
			DCFlushRange(SaveRoom, 1);

			if (IsBitSet(*MSOwned, 2) == 1) //MS was obtained
			{
				*SaveSpawn = 2;
				DCFlushRange(SaveSpawn, 1);
			}
			else
			{
				*SaveSpawn = 0;
				DCFlushRange(SaveSpawn, 1);
			}
		}
	}
	else if (strstr(CurrentStage, "D_MN04") != 0) //Goron Mines
	{
		if (strstr(SaveStage, "D_MN04") == 0)
		{
			strcpy(SaveStage, "D_MN04");
			DCFlushRange(SaveStage, 8);

			*SaveRoom = 1;
			DCFlushRange(SaveRoom, 1);

			*SaveSpawn = 0;
			DCFlushRange(SaveSpawn, 1);
		}
	}
	else if (strstr(CurrentStage, "D_MN05") != 0) //Forest Temple
	{
		if (strstr(SaveStage, "D_MN05") == 0)
		{
			strcpy(SaveStage, "D_MN05");
			DCFlushRange(SaveStage, 8);

			*SaveRoom = 22;
			DCFlushRange(SaveRoom, 1);

			*SaveSpawn = 0;
			DCFlushRange(SaveSpawn, 1);
		}
	}
	else if (strstr(CurrentStage, "D_MN06") != 0) //Temple of Time
	{
		if (strstr(SaveStage, "D_MN06") == 0)
		{
			strcpy(SaveStage, "D_MN06");
			DCFlushRange(SaveStage, 8);

			*SaveRoom = 0;
			DCFlushRange(SaveRoom, 1);

			*SaveSpawn = 0;
			DCFlushRange(SaveSpawn, 1);
		}
	}
	else if (strstr(CurrentStage, "D_MN07") != 0) //City
	{
		if (strstr(SaveStage, "D_MN07") == 0)
		{
			strcpy(SaveStage, "D_MN07");
			DCFlushRange(SaveStage, 8);

			*SaveRoom = 0;
			DCFlushRange(SaveRoom, 1);

			*SaveSpawn = 3;
			DCFlushRange(SaveSpawn, 1);
		}
	}
	else if (strstr(CurrentStage, "D_MN08") != 0) //PoT
	{
		if (strstr(SaveStage, "D_MN08") == 0)
		{
			strcpy(SaveStage, "D_MN08");
			DCFlushRange(SaveStage, 8);

			*SaveRoom = 0;
			DCFlushRange(SaveRoom, 1);

			*SaveSpawn = 0;
			DCFlushRange(SaveSpawn, 1);
		}
	}
	else if (strstr(CurrentStage, "D_MN09") != 0) //HC
	{
		if (strstr(SaveStage, "D_MN09") == 0)
		{
			strcpy(SaveStage, "D_MN09");
			DCFlushRange(SaveStage, 8);

			*SaveRoom = 11;
			DCFlushRange(SaveRoom, 1);

			*SaveSpawn = 0;
			DCFlushRange(SaveSpawn, 1);
		}
	}
	else if (strstr(CurrentStage, "D_MN10") != 0) //AG
	{
		if (strstr(SaveStage, "D_MN10") == 0)
		{
			strcpy(SaveStage, "D_MN10");
			DCFlushRange(SaveStage, 8);

			*SaveRoom = 0;
			DCFlushRange(SaveRoom, 1);

			*SaveSpawn = 0;
			DCFlushRange(SaveSpawn, 1);
		}
	}
	else if (strstr(CurrentStage, "D_MN11") != 0) //SPR
	{
		if (strstr(SaveStage, "D_MN11") == 0)
		{
			strcpy(SaveStage, "D_MN11");
			DCFlushRange(SaveStage, 8);

			*SaveRoom = 0;
			DCFlushRange(SaveRoom, 1);

			*SaveSpawn = 0;
			DCFlushRange(SaveSpawn, 1);
		}
	}
	else if (!strcmp(CurrentStage, "R_SP107") && !strcmp(CurrentState, "d")) //Sewers in MDH State
	{
		if (strstr(SaveStage, "R_SP107") == 0) //If not already Sewers go to dig spot in MDH
		{
			strcpy(SaveStage, "R_SP107");
			DCFlushRange(SaveStage, 8);

			*SaveRoom = 1;
			DCFlushRange(SaveRoom, 1);

			*SaveSpawn = 0x07;
			DCFlushRange(SaveSpawn, 1);
		}
	}
	else if ((!strcmp(CurrentStage, "R_SP107") && !strcmp(SaveStage, "R_SP107") && *SaveRoom == 0 && *SaveSpawn == 0x18) && (!strcmp(CurrentState, "7") || !strcmp(CurrentState, "a") || !strcmp(CurrentState, "b"))) //Sewers in Twilight and save location is in Prison don't change it. Only for initial and cs states!
	{
	}
	else if (!strcmp(CurrentStage, "R_SP107") && *CurrentRoom == 1 && !strcmp(CurrentState, "e")) //Sewers in Twilight and player is in main sewers, force save location at the start
	{
		strcpy(SaveStage, "R_SP107");
		DCFlushRange(SaveStage, 8);

		*SaveRoom = 1;
		DCFlushRange(SaveRoom, 1);

		*SaveSpawn = 0x00;
		DCFlushRange(SaveSpawn, 1);
	}
	else if (!strcmp(CurrentStage, "R_SP107") && *CurrentRoom == 2 && !strcmp(CurrentState, "e")) //Sewers in Twilight and player is outside, don't touch save location
	{
	}
	else //Everything else goes to Castle Town Central as a failsave
	{
		strcpy(SaveStage, "F_SP116");
		DCFlushRange(SaveStage, 8);

		*SaveRoom = 0;
		DCFlushRange(SaveRoom, 1);

		*SaveSpawn = 0;
		DCFlushRange(SaveSpawn, 1);
	}

	real_TP_Menu_dataWrite(classPtr);

	log_printf("Save Done");
}

DECL(void, TP_gameOverRetryQuestion, unsigned int* classPtr)
{
	if (isGameOver == 1)
		isCheckingDeathMusic = 1;

	real_TP_gameOverRetryQuestion(classPtr);
}

DECL(void, TP_dispWait, unsigned int* classPtr)
{
	if (isGameOver == 1)
	{
		const unsigned char *WaitTimerPtr = (const unsigned char *)classPtr + 0x114;
		u16 *WaitTimer;

		WaitTimer = ((u16 *)WaitTimerPtr);

		*WaitTimer = 0;
		DCFlushRange(WaitTimer, 2);
	}

	real_TP_dispWait(classPtr);
}

DECL(int, TP_checkBgmIDPlaying, unsigned int* classPtr, unsigned long ID)
{
	int result = real_TP_checkBgmIDPlaying(classPtr, ID);

	if (isCheckingDeathMusic == 1)
	{
		isCheckingDeathMusic = 0;
		return 0;
	}

	return result;
}

DECL(int, TP_procCoHorseCallWaitInit, unsigned int* classPtr, int val) //on successfull Epona call
{
	//Move Epona to Link
	u8* EponaPointerPtr = (u8*)0x1064DDBC;
	u32 EponaPointer = 0;

	memcpy(&EponaPointer, EponaPointerPtr, 4);

	if (EponaPointer > 0x10000000)
	{
		u8* LinkPointerPtr = (u8*)0x10681E40;
		u32 LinkPointer = 0;

		memcpy(&LinkPointer, LinkPointerPtr, 4);

		if (LinkPointer > 0x10000000)
		{
			memcpy((u32*)(EponaPointer + 0x4D0), (u32*)(LinkPointer + 0x0), 4);
			memcpy((u32*)(EponaPointer + 0x4D4), (u32*)(LinkPointer + 0x4), 4);
			memcpy((u32*)(EponaPointer + 0x4D8), (u32*)(LinkPointer + 0x8), 4);
		}
	}

	int status = real_TP_procCoHorseCallWaitInit(classPtr, val);

	return status;
}



#define MAKE_MAGIC(x, lib,functionType) { (unsigned int) my_ ## x, (unsigned int) &real_ ## x, lib, # x,0,0,functionType,0}

static struct hooks_magic_t
{
	const unsigned int replaceAddr;
	const unsigned int replaceCall;
	const unsigned int library;
	const char functionName[50];
	unsigned int realAddr;
	unsigned int restoreInstruction;
	unsigned char functionType;
	unsigned char alreadyPatched;
}tp_hooks[] = //TP HD
{
	MAKE_MAGIC(TP_Stage_Loader,						LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_Room_Loader,						LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_Room_ReLoader,					LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_fopAcM_CreateAppend,				LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_Stage_ActorCreate,				LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_Link_CheckWarpStart,				LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_Save_onSwitch,					LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_Save_offSwitch,					LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_Save_isSwitch,					LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_Link_ProcDead,					LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_setNextStage,						LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_gameStart,						LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_Query_checkAcceptTransform,		LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_Query_checkEventFlag,				LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_Query_checkAcceptDungeonWarp,		LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_onEventBit,						LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_isEventBit,						LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_getMesgFont,						LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_getRubyFont,						LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_Title_loadWait_proc,				LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_Menu_dataWrite,					LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_getLayerNo,						LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_gameOverRetryQuestion,			LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_checkBgmIDPlaying,				LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_dispWait,							LIB_GAME,DYNAMIC_FUNCTION),
	MAKE_MAGIC(TP_procCoHorseCallWaitInit,			LIB_GAME,DYNAMIC_FUNCTION),
};

volatile unsigned int dynamic_tp_calls[sizeof(tp_hooks) / sizeof(struct hooks_magic_t) * 7] __attribute__((section(".data")));


static const int totalGameHookArrays = 1;

/*
*Patches a game function
*/
void PatchGameHooks(void)
{
	hooks_magic_t *currentHooks = 0;
	int sizeHookArray = 0;
	volatile unsigned int *space = 0; //Patch branches to it

	if (OSGetTitleID() != 0 && (OSGetTitleID() == 0x000500001019E500 || OSGetTitleID() == 0x000500001019E600 || OSGetTitleID() == 0x000500001019C800)) //TP HD
	{
		currentHooks = &tp_hooks[0];
		sizeHookArray = sizeof(tp_hooks);
		space = &dynamic_tp_calls[0];

		log_printf("Patch game functions for TP HD!\n");
	}
	else
	{
		log_printf("Game doesn't need function patching!\n");
		return;
	}

	int method_hooks_count = sizeHookArray / sizeof(struct hooks_magic_t);

	u32 skip_instr = 1;
	u32 my_instr_len = 6;
	u32 instr_len = my_instr_len + skip_instr;
	u32 flush_len = 4 * instr_len;
	for (int i = 0; i < method_hooks_count; i++)
	{
		log_printf("Patching %s ...", currentHooks[i].functionName);

		u32 physical = 0;
		unsigned int repl_addr = (unsigned int)currentHooks[i].replaceAddr;
		unsigned int call_addr = (unsigned int)currentHooks[i].replaceCall;

		unsigned int real_addr = GetGameAddressOfFunction(currentHooks[i].functionName);

		if (!real_addr) {
			log_printf("Error. Didnt find address for %s\n", currentHooks[i].functionName);
			space += instr_len;
			continue;
		}

		if (DEBUG_LOG_DYN)log_printf("%s is located at %08X!\n", currentHooks[i].functionName, real_addr);

		physical = (u32)OSEffectiveToPhysical((void*)real_addr);
		if (!physical) {
			log_printf("Error. Something is wrong with the physical address\n");
			space += instr_len;
			continue;
		}

		if (DEBUG_LOG_DYN)log_printf("%s physical is located at %08X!\n", currentHooks[i].functionName, physical);

		bat_table_t my_dbat_table;
		if (DEBUG_LOG_DYN)log_printf("Setting up DBAT\n");
		KernelSetDBATsForDynamicFuction(&my_dbat_table, physical);

		//log_printf("Setting call_addr to %08X\n",(unsigned int)(space) - CODE_RW_BASE_OFFSET);
		*(volatile unsigned int *)(call_addr) = (unsigned int)(space)-CODE_RW_BASE_OFFSET;

		// copy instructions from real function.
		u32 offset_ptr = 0;
		for (offset_ptr = 0; offset_ptr<skip_instr * 4; offset_ptr += 4) {
			if (DEBUG_LOG_DYN)log_printf("(real_)%08X = %08X\n", space, *(volatile unsigned int*)(physical + offset_ptr));
			*space = *(volatile unsigned int*)(physical + offset_ptr);
			space++;
		}

		//Only works if skip_instr == 1
		if (skip_instr == 1) {
			// fill the restore instruction section
			currentHooks[i].realAddr = real_addr;
			currentHooks[i].restoreInstruction = *(volatile unsigned int*)(physical);
		}
		else {
			log_printf("Error. Can't save %s for restoring!\n", currentHooks[i].functionName);
		}

		//adding jump to real function
		/*
		90 61 ff e0     stw     r3,-32(r1)
		3c 60 12 34     lis     r3,4660
		60 63 56 78     ori     r3,r3,22136
		7c 69 03 a6     mtctr   r3
		80 61 ff e0     lwz     r3,-32(r1)
		4e 80 04 20     bctr*/
		*space = 0x9061FFE0;
		space++;
		*space = 0x3C600000 | (((real_addr + (skip_instr * 4)) >> 16) & 0x0000FFFF); // lis r3, real_addr@h
		space++;
		*space = 0x60630000 | ((real_addr + (skip_instr * 4)) & 0x0000ffff); // ori r3, r3, real_addr@l
		space++;
		*space = 0x7C6903A6; // mtctr   r3
		space++;
		*space = 0x8061FFE0; // lwz     r3,-32(r1)
		space++;
		*space = 0x4E800420; // bctr
		space++;
		DCFlushRange((void*)(space - instr_len), flush_len);
		ICInvalidateRange((unsigned char*)(space - instr_len), flush_len);

		//setting jump back
		unsigned int replace_instr = 0x48000002 | (repl_addr & 0x03fffffc);
		*(volatile unsigned int *)(physical) = replace_instr;
		ICInvalidateRange((void*)(real_addr), 4);

		//restore my dbat stuff
		KernelRestoreDBATs(&my_dbat_table);

		currentHooks[i].alreadyPatched = 1;

		log_printf("done!\n");
	}

	log_print("Done with patching all game functions!\n");
}

void RestoreGameInstructions(void)
{
	log_printf("Restore all game functions!\n");

	for (int n = 0; n < totalGameHookArrays; n++)
	{
		hooks_magic_t *currentHooks = 0;
		int sizeHookArray = 0;

		if (n == 0) //TP HD
		{
			currentHooks = &tp_hooks[0];
			sizeHookArray = sizeof(tp_hooks);
		}

		int method_hooks_count = sizeHookArray / sizeof(struct hooks_magic_t);
		for (int i = 0; i < method_hooks_count; i++)
		{
			log_printf("Restoring %s ...", currentHooks[i].functionName);
			if (currentHooks[i].restoreInstruction == 0 || currentHooks[i].realAddr == 0) {
				log_printf("Error. I dont have the information for the restore =( skip\n");
				continue;
			}

			currentHooks[i].alreadyPatched = 0;
		}
	}

	log_print("Done with restoring all game functions!\n");
}

unsigned int GetGameAddressOfFunction(const char *functionName)
{
	unsigned int real_addr = 0;

	//TP HD Section
	if (!strcmp(functionName, "TP_Stage_Loader"))
	{
		real_addr = 0x02AB9A00;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_Room_Loader"))
	{
		real_addr = 0x02AB9A64;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_Room_ReLoader"))
	{
		real_addr = 0x02AB9AE4;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_fopAcM_CreateAppend"))
	{
		real_addr = 0x02ACE4AC;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_Stage_ActorCreate"))
	{
		real_addr = 0x02AB6BCC;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_Link_CheckWarpStart"))
	{
		real_addr = 0x02064424;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_Save_onSwitch"))
	{
		real_addr = 0x02AA8714;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_Save_offSwitch"))
	{
		real_addr = 0x02AA8780;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_Save_isSwitch"))
	{
		real_addr = 0x02AA87EC;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_setNextStage"))
	{
		real_addr = 0x029049F4;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_gameStart"))
	{
		real_addr = 0x029076E4;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_Link_ProcDead"))
	{
		real_addr = 0x020B9A90;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_Query_checkAcceptTransform"))
	{
		real_addr = 0x02A6419C;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_Query_checkEventFlag"))
	{
		real_addr = 0x02A6337C;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_Query_checkAcceptDungeonWarp"))
	{
		real_addr = 0x02A643C8;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_onEventBit"))
	{
		real_addr = 0x02AA55E8;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_isEventBit"))
	{
		real_addr = 0x02AA5614;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_getMesgFont"))
	{
		real_addr = 0x02ADDD44;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_getRubyFont"))
	{
		real_addr = 0x02ADDE80;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_Title_loadWait_proc"))
	{
		real_addr = 0x028BDC24;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_Menu_dataWrite"))
	{
		real_addr = 0x02A08494;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_getLayerNo"))
	{
		real_addr = 0x02907940;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_checkFileLoad"))
	{
		real_addr = 0x028BD67C;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_triggerStart"))
	{
		real_addr = 0x02ABC630;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_gameOverMove"))
	{
		real_addr = 0x02A03060;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_gameOverRetryQuestion"))
	{
		real_addr = 0x02A0A924;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_checkBgmIDPlaying"))
	{
		real_addr = 0x02C2525C;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_dispWait"))
	{
		real_addr = 0x0294FAE4;
		return real_addr;
	}
	if (!strcmp(functionName, "TP_procCoHorseCallWaitInit"))
	{
		real_addr = 0x020B6358;
		return real_addr;
	}

	return 0;
}

static void ClearGlobalVars()
{
	memset(&g_currentInputData, 0, sizeof(VPADData));
	memset(&g_currentInputDataKPAD, 0, sizeof(KPADData));
	g_assertion_counter = 0;
	g_gameRunning = 0;
	g_fontInit = 0;
	g_fileRedirectActive = 0;
	g_isTPHD = -1;
	g_renderWidth = 0;
	g_renderHeight = 0;
	g_widthScaleFactor = 0;
	g_heightScaleFactor = 0;
	g_depthScaleFactor = 0;
	g_hasGameOvered = 0;

	g_continueVPADRead = 0;
	g_frameStepActive = 0;
	g_frameStepFPS = 0;
	g_frameStepCounter = 0;

	g_checkIntroItemReload = 0;
	g_doGorgeFallback = 0;

	ExceptionHooksCore1 = 0;
}

//Gets called on process exit
DECL(void, _Exit, void)
{
	//Cleanup
	ClearGlobalVars();

	if (loadedArcFiles)
	{
		delete(loadedArcFiles);
		loadedArcFiles = NULL;
	}

	real__Exit();
}


//Gets called whenever the system polls the WiiU Gamepad
DECL(int, VPADRead, int chan, VPADData *buffer, u32 buffer_size, s32 *error)
{
	int result = real_VPADRead(chan, buffer, buffer_size, error); //Read the actual inputs from the real function

	if (chan == 0) //Only read inputs from Controller Port 0 for now
	{
		g_currentInputData = *buffer;

		if (buffer->tpdata1.touched)
		{
			input_actionID = 0;
		}
		else if (((buffer->btns_h & VPAD_BUTTON_STICK_L) == VPAD_BUTTON_STICK_L) && ((buffer->btns_h & VPAD_BUTTON_STICK_R) == VPAD_BUTTON_STICK_R))
		{
			input_actionID = 2;
		}
		else if (((buffer->btns_h & VPAD_BUTTON_Y) == VPAD_BUTTON_Y) && ((buffer->btns_h & VPAD_BUTTON_B) == VPAD_BUTTON_B) && ((buffer->btns_h & VPAD_BUTTON_DOWN) == VPAD_BUTTON_DOWN))
		{
			input_actionID = 3;
		}
		else
		{
			input_actionStep = 0;
			input_actionID = 0;
			input_holdDuration = 0;
		}

		if (g_gameRunning)
		{
			if (input_actionID == 1) //Check for touchpad swipe left-right
			{
				//log_printf("Touchpad X: %i, Y: %i", buffer->tpdata1.x, buffer->tpdata1.y);

				u16 x = buffer->tpdata1.x;

				switch (input_actionStep)
				{

				case 0:

					if (x > 0 && x < 200)
						input_actionStep++;

					break;

				case 1:

					if (x > 800 && x < 1600)
						input_actionStep++;

					break;

				case 2:

					if (x > 1600 && x < 2400)
						input_actionStep++;

					break;

				case 3:

					if (x > 2400 && x < 3200)
						input_actionStep++;

					break;

				case 4:

					if (x > 3400) //Go to Mii Maker if touchpad swipe left to right
					{
						log_printf("Touchpad command triggered");

						if (sysapp_handle == 0)
							InitSysFunctionPointers();

						SYSLaunchMenu();
						_SYSLaunchMiiStudio();

						ClearGlobalVars();

						input_actionStep = 0;
						input_actionID = 0;
						input_holdDuration = 0;
					}

					break;

				default:

					input_actionStep = 0;
					input_actionID = 0;
					input_holdDuration = 0;

					break;
				}
			}
			else if (input_actionID == 2)
			{
				if (coreinit_handle == 0)
					InitOSFunctionPointers();

				if (input_holdDuration < 90)
					input_holdDuration++;
				else //Restart Game if both sticks clicked for more than 3 seconds (1.5 seconds in 60 fps games)
				{
					log_printf("Normal click button command triggered");

					OSRestartGame(0, NULL);

					ClearGlobalVars();

					input_actionStep = 0;
					input_actionID = 0;
					input_holdDuration = 0;
				}
			}
			else if (input_actionID == 3) //Softreset in 0.67 seconds
			{
				if (coreinit_handle == 0)
					InitOSFunctionPointers();

				if (input_holdDuration < 20)
					input_holdDuration++;
				else
				{
					u32 startResetPtr = 0;
					int *startReset;

					memcpy(&startResetPtr, (u32*)0x1062DDDC, 4);

					if (startResetPtr > 0x10000000)
					{
						startReset = ((int *)startResetPtr);

						if (*startReset == 0)
						{
							log_printf("Softreset command triggered");

							*startReset = 1;
							DCFlushRange(startReset, 4);
						}
					}

					input_actionStep = 0;
					input_actionID = 0;
					input_holdDuration = 0;
				}
			}
		}
	}

	return result;
}

DECL(s32, KPADReadEx, s32 chan, KPADData *buffer, u32 count, s32 *error)
{
	u8 readController = 1;

	s32 result = real_KPADReadEx(chan, buffer, count, error); //Read the actual inputs from the real function

	if (error)
	{
		if (*error == -2) //Controller not connected
		{
			readController = 0;
		}
	}

	if (readController) //Only read inputs from Controllers that are connected
	{
		g_currentInputDataKPAD = *buffer;

		if (((buffer->classic.btns_h & WPAD_CLASSIC_BUTTON_STICK_L) == WPAD_CLASSIC_BUTTON_STICK_L) && ((buffer->classic.btns_h & WPAD_CLASSIC_BUTTON_STICK_R) == WPAD_CLASSIC_BUTTON_STICK_R))
		{
			input_actionIDKPAD = 2;
		}
		else if (((buffer->classic.btns_h & WPAD_CLASSIC_BUTTON_Y) == WPAD_CLASSIC_BUTTON_Y) && ((buffer->classic.btns_h & WPAD_CLASSIC_BUTTON_B) == WPAD_CLASSIC_BUTTON_B) && ((buffer->classic.btns_h & WPAD_CLASSIC_BUTTON_DOWN) == WPAD_CLASSIC_BUTTON_DOWN))
		{
			input_actionIDKPAD = 3;
		}
		else
		{
			input_actionStepKPAD = 0;
			input_actionIDKPAD = 0;
			input_holdDurationKPAD = 0;
		}

		if (g_gameRunning)
		{
			if (input_actionIDKPAD == 2)
			{
				if (coreinit_handle == 0)
					InitOSFunctionPointers();

				if (input_holdDurationKPAD < 90)
					input_holdDurationKPAD++;
				else //Restart Game if both sticks clicked for more than 3 seconds (1.5 seconds in 60 fps games)
				{
					log_printf("Normal click button command triggered");

					OSRestartGame(0, NULL);

					ClearGlobalVars();

					input_actionStepKPAD = 0;
					input_actionIDKPAD = 0;
					input_holdDurationKPAD = 0;
				}
			}
			else if (input_actionIDKPAD == 3) //Softreset in 0.67 seconds
			{
				if (coreinit_handle == 0)
					InitOSFunctionPointers();

				if (input_holdDurationKPAD < 20)
					input_holdDurationKPAD++;
				else
				{
					u32 startResetPtr = 0;
					int *startReset;

					memcpy(&startResetPtr, (u32*)0x1062DDDC, 4);

					if (startResetPtr > 0x10000000)
					{
						startReset = ((int *)startResetPtr);

						if (*startReset == 0)
						{
							log_printf("Softreset command triggered");

							*startReset = 1;
							DCFlushRange(startReset, 4);
						}
					}

					input_actionStepKPAD = 0;
					input_actionIDKPAD = 0;
					input_holdDurationKPAD = 0;
				}
			}
		}
	}

	return result;
}

//OSPanic
DECL(void, OSPanic, const char *file, u32 line, const char *fmt, ...)
{
	if (coreinit_handle == 0)
		InitOSFunctionPointers();

	if (!strcmp(file, "OSFatal.c")) //Coming from exception handler or serious game crash
	{
		ClearGlobalVars();

		real_OSPanic(file, line, fmt); //in this case we might not have logging available, thus crash asap

		return;
	}

	if (sysapp_handle == 0)
		InitSysFunctionPointers();

	if (nsysnet_handle == 0)
	{
		InitSocketFunctionPointers();

		log_init(HOST_IP);
		log_deinit();
		log_init(HOST_IP);
	}

	char * tmp = NULL;

	va_list va;
	va_start(va, fmt);
	if ((vasprintf(&tmp, fmt, va) >= 0) && tmp)
	{
		log_printf("OSPanic call from file: %s on line: %i with message: %s", file, line, tmp);
	}
	else
	{
		log_printf("OSPanic call from file: %s on line: %i", file, line);
	}
	va_end(va);

	if (tmp)
		free(tmp);
}

//Re-direct socket lib finish into nothing so games can't kill the socket library and our connection by accident
DECL(int, socket_lib_finish, void)
{
	return 0;
}

//Realize when the home button menu is about to come up to prevent crashing
DECL(int, OSIsHomeButtonMenuEnabled, void)
{
	int ret = real_OSIsHomeButtonMenuEnabled();

	g_gameRunning = 0;
	g_fileRedirectActive = 0;

	return ret;
}

DECL(int, FSGetStat, void *pClient, void *pCmd, const char *path, FSStat *stats, int errHandling)
{
	std::string newPath = path;

	//Load modded files from SD
	if (g_fileRedirectActive)
	{
		int status = 0;

		char mountSrc[FS_MOUNT_SOURCE_SIZE];
		char mountPath[FS_MAX_MOUNTPATH_SIZE];

		if (strstr(path, "/content/") != 0)
		{
			std::string localSDPath = newPath.substr(newPath.find_first_of("content") + 3);
			localSDPath = "/vol/external01/wiiu/apps/TP_BetaQuest/" + localSDPath;

			//log_printf("FSGetStat Res File: %s", localSDPath.c_str());

			//Mount SD
			if ((status = FSGetMountSource(pClient, pCmd, FS_SOURCETYPE_EXTERNAL, &mountSrc, FS_RET_NO_ERROR)) == FS_STATUS_OK)
			{
				if ((status = FSMount(pClient, pCmd, &mountSrc, mountPath, FS_MAX_MOUNTPATH_SIZE, FS_RET_UNSUPPORTED_CMD)) == FS_STATUS_OK)
				{
					FSStat stats;

					if ((status = real_FSGetStat(pClient, pCmd, localSDPath.c_str(), &stats, -1)) == FS_STATUS_OK)
					{
						log_printf("FSGetStat: File %s found in MOD!", localSDPath.c_str());

						newPath = localSDPath;

						if (strstr(path, "FileSizeList.txt") != 0)
						{
							log_printf("FSGetStat: ResName exists: FileSizeList.txt - Redirect size to: %i!", stats.size);
						}
						else if (strstr(path, "DecompressedSizeList.txt") != 0)
						{
							log_printf("FSGetStat: ResName exists: DecompressedSizeList.txt - Redirect size to: %i!", stats.size);
						}
					}
				}
				else
				{
					log_printf("FSMount failed %d\n", status);
				}
			}
			else
			{
				log_printf("FSGetMountSource failed %d\n", status);
			}
		}
	}


	int result = real_FSGetStat(pClient, pCmd, newPath.c_str(), stats, errHandling);

	//Legacy Resource System
	if (g_fileRedirectActive)
	{
		if (loadedArcFiles == NULL)
			loadedArcFiles = new std::list<resList>;

		if (result == 0 && loadedArcFiles)
		{
			if (strstr(path, "res/Msg") != 0)
			{
				std::string pathStr = path;

				std::string arcName = pathStr.substr(pathStr.find_last_of('/') + 1);

				if (Resources::GetFile(arcName.c_str()) != NULL)
				{
					stats->size = Resources::GetFileSize(arcName.c_str());

					log_printf("FSGetStat: ResName exists: %s - Redirect size to: %i!", arcName.c_str(), stats->size);
				}
			}
		}
	}

	return result;
}

DECL(int, FSGetStatFile, void *pClient, void *pCmd, int fd, FSStat *stats, int error)
{
	int result = real_FSGetStatFile(pClient, pCmd, fd, stats, error);

	if (g_fileRedirectActive)
	{
		if (loadedArcFiles)
		{
			if (loadedArcFiles->size() > 0)
			{
				for (std::list<resList>::iterator it = loadedArcFiles->begin(); it != loadedArcFiles->end(); it++)
				{
					if (it->fileHandle == fd)
					{
						stats->size = Resources::GetFileSize(it->resName.c_str());

						log_printf("FSGetStatFile: Get size for file handle from file: %s - Redirect to: %i!", it->resName.c_str(), stats->size);

						break;
					}
				}
			}
		}
	}

	return result;
}

DECL(int, FSOpenFile, void *pClient, void *pCmd, const char *path, const char *mode, int *fd, int errHandling)
{
	std::string newPath = path;

	//Load modded files from SD
	if (g_fileRedirectActive)
	{
		int status = 0;

		char mountSrc[FS_MOUNT_SOURCE_SIZE];
		char mountPath[FS_MAX_MOUNTPATH_SIZE];

		if (strstr(path, "/content/") != 0)
		{
			std::string localSDPath = newPath.substr(newPath.find_first_of("content") + 3);
			localSDPath = "/vol/external01/wiiu/apps/TP_BetaQuest/" + localSDPath;

			//log_printf("FSOpen Res File: %s", localSDPath.c_str());

			//Mount SD
			if ((status = FSGetMountSource(pClient, pCmd, FS_SOURCETYPE_EXTERNAL, &mountSrc, FS_RET_NO_ERROR)) == FS_STATUS_OK)
			{
				if ((status = FSMount(pClient, pCmd, &mountSrc, mountPath, FS_MAX_MOUNTPATH_SIZE, FS_RET_UNSUPPORTED_CMD)) == FS_STATUS_OK)
				{
					FSStat stats;

					if ((status = real_FSGetStat(pClient, pCmd, localSDPath.c_str(), &stats, -1)) == FS_STATUS_OK)
					{
						log_printf("FSOpenFile: File %s found in MOD!", localSDPath.c_str());

						newPath = localSDPath;

						if (strstr(path, "FileSizeList.txt") != 0)
						{
							log_printf("FSOpenFile: FileSizeList.txt exists - add to the list");

							handledFileSize = 1;
						}
						else if (strstr(path, "DecompressedSizeList.txt") != 0)
						{
							log_printf("FSOpenFile: DecompressedSizeList.txt exists - add to the list");

							handledDecompressedFileSize = 1;
						}
					}
				}
				else
				{
					log_printf("FSMount failed %d\n", status);
				}
			}
			else
			{
				log_printf("FSGetMountSource failed %d\n", status);
			}
		}
	}

	int result = real_FSOpenFile(pClient, pCmd, newPath.c_str(), mode, fd, errHandling);

	//Report logs once game has booted fully
	if (g_gameRunning && handledFileSize == 1)
	{
		log_printf("FSOpenFile: Handled FileSizeList.txt!");
		handledFileSize = 0;
	}

	if (g_gameRunning && handledDecompressedFileSize == 1)
	{
		log_printf("FSOpenFile: Handled DecompressedSizeList.txt!");
		handledDecompressedFileSize = 0;
	}

	//Legacy Resource System
	if (g_fileRedirectActive)
	{
		if (loadedArcFiles == NULL)
			loadedArcFiles = new std::list<resList>;

		if (result == 0 && loadedArcFiles)
		{
			if (strstr(path, "res/Msg") != 0)
			{
				std::string pathStr = path;

				std::string arcName = pathStr.substr(pathStr.find_last_of('/') + 1);

				if (Resources::GetFile(arcName.c_str()) != NULL)
				{
					log_printf("FSOpenFile: ResName exists: %s - add to the list", arcName.c_str());

					resList newRes;

					newRes.fileHandle = *fd;
					newRes.resName = arcName;
					newRes.pClient = pClient;
					newRes.pCmd = pCmd;

					loadedArcFiles->push_back(newRes);
				}
			}	
		}
	}

	return result;
}

DECL(int, FSCloseFile, void *pClient, void *pCmd, int fd, int errHandling)
{
	int result = real_FSCloseFile(pClient, pCmd, fd, errHandling);

	if (g_fileRedirectActive)
	{
		if (loadedArcFiles)
		{
			if (loadedArcFiles->size() > 0)
			{
				for (std::list<resList>::iterator it = loadedArcFiles->begin(); it != loadedArcFiles->end(); it++)
				{
					if (it->fileHandle == fd)
					{
						log_printf("FSCloseFile: Deleted %s from the list. Filehandle: %i was closed!", it->resName.c_str(), fd);

						loadedArcFiles->erase(it);
						break;
					}
				}
			}
		}
	}

	return result;
}

DECL(int, FSReadFile, void *pClient, void *pCmd, void *buffer, int size, int count, int fd, int flag, int errHandling)
{
	int result = 0;

	if (g_fileRedirectActive)
	{
		if (loadedArcFiles)
		{
			if (loadedArcFiles->size() > 0)
			{
				for (std::list<resList>::iterator it = loadedArcFiles->begin(); it != loadedArcFiles->end(); it++)
				{
					if (it->fileHandle == fd)
					{
						u32 totalSize = size * count;
						u32 resSize = Resources::GetFileSize(it->resName.c_str());

						if (resSize < totalSize)
							totalSize = resSize;

						//log_printf("FSReadFile: Read issued for file: %s - Redirect!", it->resName.c_str());

						const u8* sourcePtr = Resources::GetFile(it->resName.c_str());

						memcpy(buffer, sourcePtr, totalSize);

						return totalSize;
					}
				}
			}
		}
	}

	result = real_FSReadFile(pClient, pCmd, buffer, size, count, fd, flag, errHandling);

	return result;
}

DECL(int, FSReadFileWithPos, void *pClient, void *pCmd, void *buffer, int size, int count, int pos, int fd, int flag, int errHandling)
{
	int result = 0;

	if (g_fileRedirectActive)
	{
		if (loadedArcFiles)
		{
			if (loadedArcFiles->size() > 0)
			{
				for (std::list<resList>::iterator it = loadedArcFiles->begin(); it != loadedArcFiles->end(); it++)
				{
					if (it->fileHandle == fd)
					{
						u32 totalSize = size * count;
						u32 resSize = Resources::GetFileSize(it->resName.c_str());
						resSize = resSize - pos;

						if (resSize == 0)
						{
							log_printf("FSReadFileWithPos: End of file: %s reached!", it->resName.c_str());
							return -2; //End of file
						}

						//log_printf("FSReadFileWithPos: Read issued for file: %s, Pos: %i, size: %i, count: %i!", it->resName.c_str(), pos, size, count);

						if (resSize < totalSize)
							totalSize = resSize;

						const u8* sourcePtr = Resources::GetFile(it->resName.c_str());
						sourcePtr = sourcePtr + pos;

						memcpy(buffer, sourcePtr, totalSize);

						//result = real_FSReadFileWithPos(pClient, pCmd, buffer, size, count, pos, fd, flag, errHandling);

						//log_printf("FSReadFileWithPos: Result: %u", totalSize);

						return (int)totalSize;
					}
				}
			}
		}
	}

	result = real_FSReadFileWithPos(pClient, pCmd, buffer, size, count, pos, fd, flag, errHandling);

	return result;
}

DECL(int, FSReadFileWithPosAsync, void *pClient, void *pCmd, void *buffer, int size, int count, int pos, int fd, int flag, int errHandling, const FSAsyncData *asyncData)
{
	int result = 0;
	int readZero = 0;

	if (g_fileRedirectActive)
	{
		if (loadedArcFiles)
		{
			if (loadedArcFiles->size() > 0)
			{
				for (std::list<resList>::iterator it = loadedArcFiles->begin(); it != loadedArcFiles->end(); it++)
				{
					if (it->fileHandle == fd)
					{
						u32 totalSize = size * count;
						u32 resSize = Resources::GetFileSize(it->resName.c_str());
						resSize = resSize - pos;

						if (resSize == 0)
						{
							log_printf("FSReadFileWithPosAsync: End of file: %s reached!", it->resName.c_str());
							return -2; //End of file
						}

						//log_printf("FSReadFileWithPosAsync: Read issued for file: %s, Pos: %i, size: %i, count: %i!", it->resName.c_str(), pos, size, count);

						if (resSize < totalSize)
							totalSize = resSize;

						const u8* sourcePtr = Resources::GetFile(it->resName.c_str());
						sourcePtr = sourcePtr + pos;

						memcpy(buffer, sourcePtr, totalSize);

						//result = real_FSReadFileWithPos(pClient, pCmd, buffer, size, count, pos, fd, flag, errHandling);

						//log_printf("FSReadFileWithPos: Result: %u", totalSize);

						it->lastReadCount = (int)totalSize;

						readZero = 1;

						break;
					}
				}
			}
		}
	}

	if (readZero == 0)
	{
		result = real_FSReadFileWithPosAsync(pClient, pCmd, buffer, size, count, pos, fd, flag, errHandling, asyncData);
	}
	else
	{
		result = real_FSReadFileWithPosAsync(pClient, pCmd, buffer, size, 0, 0, fd, flag, errHandling, asyncData);
	}

	return result;
}

DECL(FSAsyncResult*, FSGetAsyncResult, OSMessage *ioMsg)
{
	FSAsyncResult* result = 0;

	result = real_FSGetAsyncResult(ioMsg);

	if (g_fileRedirectActive)
	{
		if (loadedArcFiles)
		{
			if (loadedArcFiles->size() > 0)
			{
				if (result)
				{
					for (std::list<resList>::iterator it = loadedArcFiles->begin(); it != loadedArcFiles->end(); it++)
					{
						if (it->pClient == result->pClient && it->pCmd == result->pCmd)
						{
							//log_printf("Async result for handle: %i - read count: %i, change to: %i", it->fileHandle, result->fileStatus, it->lastReadCount);

							result->fileStatus = it->lastReadCount;

							break;
						}
					}
				}
			}
		}
	}

	return result;
}

/* *****************************************************************************
* Creates function pointer array
* ****************************************************************************/

hooks_magic_t method_hooks[] =
{
	MAKE_MAGIC(VPADRead,                             LIB_VPAD,STATIC_FUNCTION),
	MAKE_MAGIC(KPADReadEx,							 LIB_PADSCORE,DYNAMIC_FUNCTION),
	MAKE_MAGIC(socket_lib_finish,                    LIB_NSYSNET,STATIC_FUNCTION),
	MAKE_MAGIC(OSPanic,								 LIB_CORE_INIT,STATIC_FUNCTION),
	MAKE_MAGIC(OSIsHomeButtonMenuEnabled,			 LIB_CORE_INIT,STATIC_FUNCTION),
	MAKE_MAGIC(_Exit,								 LIB_CORE_INIT,STATIC_FUNCTION),
	MAKE_MAGIC(FSGetStat,							 LIB_FS,STATIC_FUNCTION),
	MAKE_MAGIC(FSGetStatFile,						 LIB_FS,STATIC_FUNCTION),
	MAKE_MAGIC(FSOpenFile,							 LIB_FS,STATIC_FUNCTION),
	MAKE_MAGIC(FSCloseFile,							 LIB_FS,STATIC_FUNCTION),
	MAKE_MAGIC(FSReadFile,							 LIB_FS,STATIC_FUNCTION),
	MAKE_MAGIC(FSReadFileWithPos,					 LIB_FS,STATIC_FUNCTION),
	MAKE_MAGIC(FSReadFileWithPosAsync,				 LIB_FS,STATIC_FUNCTION),
	MAKE_MAGIC(FSGetAsyncResult,					LIB_FS,STATIC_FUNCTION),
};

//TP HD uses FSOpenFile (and async) FSCloseFile (and async), FSGetStat, FSGetStatFile, FSReadFile, FSReadFileWithPos (and async), (FSGetAsyncResult)

//! buffer to store our 7 instructions needed for our replacements
//! the code will be placed in the address of that buffer - CODE_RW_BASE_OFFSET
//! avoid this buffer to be placed in BSS and reset on start up
volatile unsigned int dynamic_method_calls[sizeof(method_hooks) / sizeof(struct hooks_magic_t) * 7] __attribute__((section(".data")));

/*
*Patches a function that is loaded at the start of each application. Its not required to restore, at least when they are really dynamic.
* "normal" functions should be patch with the normal patcher. Current Code by Maschell with the help of dimok.
*/
void PatchMethodHooks(void)
{
	/* Patch branches to it.  */
	volatile unsigned int *space = &dynamic_method_calls[0];

	int method_hooks_count = sizeof(method_hooks) / sizeof(struct hooks_magic_t);

	u32 skip_instr = 1;
	u32 my_instr_len = 6;
	u32 instr_len = my_instr_len + skip_instr;
	u32 flush_len = 4 * instr_len;
	for (int i = 0; i < method_hooks_count; i++)
	{
		log_printf("Patching %s ...", method_hooks[i].functionName);
		if (method_hooks[i].functionType == STATIC_FUNCTION && method_hooks[i].alreadyPatched == 1) {
			if (isDynamicFunction((u32)OSEffectiveToPhysical((void*)method_hooks[i].realAddr))) {
				log_printf(" The function %s is a dynamic function. Please fix that <3 ... ", method_hooks[i].functionName);
				method_hooks[i].functionType = DYNAMIC_FUNCTION;
			}
			else {
				log_printf(" skipped. Its already patched\n", method_hooks[i].functionName);
				space += instr_len;
				continue;
			}
		}

		u32 physical = 0;
		unsigned int repl_addr = (unsigned int)method_hooks[i].replaceAddr;
		unsigned int call_addr = (unsigned int)method_hooks[i].replaceCall;

		unsigned int real_addr = GetAddressOfFunction(method_hooks[i].functionName, method_hooks[i].library);

		if (!real_addr) {
			log_printf("Error. OSDynLoad_FindExport failed for %s\n", method_hooks[i].functionName);
			space += instr_len;
			continue;
		}

		if (DEBUG_LOG_DYN)log_printf("%s is located at %08X!\n", method_hooks[i].functionName, real_addr);

		physical = (u32)OSEffectiveToPhysical((void*)real_addr);
		if (!physical) {
			log_printf("Error. Something is wrong with the physical address\n");
			space += instr_len;
			continue;
		}

		if (DEBUG_LOG_DYN)log_printf("%s physical is located at %08X!\n", method_hooks[i].functionName, physical);

		bat_table_t my_dbat_table;
		if (DEBUG_LOG_DYN)log_printf("Setting up DBAT\n");
		KernelSetDBATsForDynamicFuction(&my_dbat_table, physical);

		//log_printf("Setting call_addr to %08X\n",(unsigned int)(space) - CODE_RW_BASE_OFFSET);
		*(volatile unsigned int *)(call_addr) = (unsigned int)(space)-CODE_RW_BASE_OFFSET;

		// copy instructions from real function.
		u32 offset_ptr = 0;
		for (offset_ptr = 0; offset_ptr<skip_instr * 4; offset_ptr += 4) {
			if (DEBUG_LOG_DYN)log_printf("(real_)%08X = %08X\n", space, *(volatile unsigned int*)(physical + offset_ptr));
			*space = *(volatile unsigned int*)(physical + offset_ptr);
			space++;
		}

		//Only works if skip_instr == 1
		if (skip_instr == 1) {
			// fill the restore instruction section
			method_hooks[i].realAddr = real_addr;
			method_hooks[i].restoreInstruction = *(volatile unsigned int*)(physical);
		}
		else {
			log_printf("Error. Can't save %s for restoring!\n", method_hooks[i].functionName);
		}

		//adding jump to real function
		/*
		90 61 ff e0     stw     r3,-32(r1)
		3c 60 12 34     lis     r3,4660
		60 63 56 78     ori     r3,r3,22136
		7c 69 03 a6     mtctr   r3
		80 61 ff e0     lwz     r3,-32(r1)
		4e 80 04 20     bctr*/
		*space = 0x9061FFE0;
		space++;
		*space = 0x3C600000 | (((real_addr + (skip_instr * 4)) >> 16) & 0x0000FFFF); // lis r3, real_addr@h
		space++;
		*space = 0x60630000 | ((real_addr + (skip_instr * 4)) & 0x0000ffff); // ori r3, r3, real_addr@l
		space++;
		*space = 0x7C6903A6; // mtctr   r3
		space++;
		*space = 0x8061FFE0; // lwz     r3,-32(r1)
		space++;
		*space = 0x4E800420; // bctr
		space++;
		DCFlushRange((void*)(space - instr_len), flush_len);
		ICInvalidateRange((unsigned char*)(space - instr_len), flush_len);

		//setting jump back
		unsigned int replace_instr = 0x48000002 | (repl_addr & 0x03fffffc);
		*(volatile unsigned int *)(physical) = replace_instr;
		ICInvalidateRange((void*)(real_addr), 4);

		//restore my dbat stuff
		KernelRestoreDBATs(&my_dbat_table);

		method_hooks[i].alreadyPatched = 1;

		log_printf("done!\n");
	}
	log_print("Done with patching all functions!\n");
}

/* ****************************************************************** */
/*                  RESTORE ORIGINAL INSTRUCTIONS                     */
/* ****************************************************************** */
void RestoreInstructions(void)
{
	bat_table_t table;
	log_printf("Restore functions!\n");
	int method_hooks_count = sizeof(method_hooks) / sizeof(struct hooks_magic_t);
	for (int i = 0; i < method_hooks_count; i++)
	{
		log_printf("Restoring %s ...", method_hooks[i].functionName);
		if (method_hooks[i].restoreInstruction == 0 || method_hooks[i].realAddr == 0) {
			log_printf("Error. I dont have the information for the restore =( skip\n");
			continue;
		}

		unsigned int real_addr = GetAddressOfFunction(method_hooks[i].functionName, method_hooks[i].library);

		if (!real_addr) {
			//log_printf("Error. OSDynLoad_FindExport failed for %s\n", method_hooks[i].functionName);
			continue;
		}

		u32 physical = (u32)OSEffectiveToPhysical((void*)real_addr);
		if (!physical) {
			log_printf("Error. Something is wrong with the physical address\n");
			continue;
		}

		if (isDynamicFunction(physical)) {
			log_printf("Error. Its a dynamic function. We don't need to restore it! %s\n", method_hooks[i].functionName);
		}
		else {
			KernelSetDBATs(&table);

			*(volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + method_hooks[i].realAddr) = method_hooks[i].restoreInstruction;
			DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + method_hooks[i].realAddr), 4);
			ICInvalidateRange((void*)method_hooks[i].realAddr, 4);
			log_printf(" done\n");
			KernelRestoreDBATs(&table);
		}
		method_hooks[i].alreadyPatched = 0; // In case a
	}
	KernelRestoreInstructions();
	log_print("Done with restoring all functions!\n");
}

int isDynamicFunction(unsigned int physicalAddress) {
	if ((physicalAddress & 0x80000000) == 0x80000000) {
		return 1;
	}    return 0;
}

unsigned int GetAddressOfFunction(const char * functionName, unsigned int library) {
	unsigned int real_addr = 0;

	unsigned int rpl_handle = 0;
	if (library == LIB_CORE_INIT) {
		if (DEBUG_LOG_DYN)log_printf("FindExport of %s! From LIB_CORE_INIT\n", functionName);
		if (coreinit_handle == 0) { log_print("LIB_CORE_INIT not aquired\n"); return 0; }
		rpl_handle = coreinit_handle;
	}
	else if (library == LIB_NSYSNET) {
		if (DEBUG_LOG_DYN)log_printf("FindExport of %s! From LIB_NSYSNET\n", functionName);
		if (nsysnet_handle == 0) { log_print("LIB_NSYSNET not aquired\n"); return 0; }
		rpl_handle = nsysnet_handle;
	}
	else if (library == LIB_GX2) {
		if (DEBUG_LOG_DYN)log_printf("FindExport of %s! From LIB_GX2\n", functionName);
		if (gx2_handle == 0) { log_print("LIB_GX2 not aquired\n"); return 0; }
		rpl_handle = gx2_handle;
	}
	else if (library == LIB_AOC) {
		if (DEBUG_LOG_DYN)log_printf("FindExport of %s! From LIB_AOC\n", functionName);
		if (aoc_handle == 0) { log_print("LIB_AOC not aquired\n"); return 0; }
		rpl_handle = aoc_handle;
	}
	else if (library == LIB_AX) {
		if (DEBUG_LOG_DYN)log_printf("FindExport of %s! From LIB_AX\n", functionName);
		if (sound_handle == 0) { log_print("LIB_AX not aquired\n"); return 0; }
		rpl_handle = sound_handle;
	}
	else if (library == LIB_FS) {
		if (DEBUG_LOG_DYN)log_printf("FindExport of %s! From LIB_FS\n", functionName);
		if (coreinit_handle == 0) { log_print("LIB_FS not aquired\n"); return 0; }
		rpl_handle = coreinit_handle;
	}
	else if (library == LIB_OS) {
		if (DEBUG_LOG_DYN)log_printf("FindExport of %s! From LIB_OS\n", functionName);
		if (coreinit_handle == 0) { log_print("LIB_OS not aquired\n"); return 0; }
		rpl_handle = coreinit_handle;
	}
	else if (library == LIB_PADSCORE) {
		if (DEBUG_LOG_DYN)log_printf("FindExport of %s! From LIB_PADSCORE\n", functionName);
		if (padscore_handle == 0) { log_print("LIB_PADSCORE not aquired\n"); return 0; }
		rpl_handle = padscore_handle;
	}
	else if (library == LIB_SOCKET) {
		if (DEBUG_LOG_DYN)log_printf("FindExport of %s! From LIB_SOCKET\n", functionName);
		if (nsysnet_handle == 0) { log_print("LIB_SOCKET not aquired\n"); return 0; }
		rpl_handle = nsysnet_handle;
	}
	else if (library == LIB_SYS) {
		if (DEBUG_LOG_DYN)log_printf("FindExport of %s! From LIB_SYS\n", functionName);
		if (sysapp_handle == 0) { log_print("LIB_SYS not aquired\n"); return 0; }
		rpl_handle = sysapp_handle;
	}
	else if (library == LIB_VPAD) {
		if (DEBUG_LOG_DYN)log_printf("FindExport of %s! From LIB_VPAD\n", functionName);
		if (vpad_handle == 0) { log_print("LIB_VPAD not aquired\n"); return 0; }
		rpl_handle = vpad_handle;
	}
	else if (library == LIB_NN_ACP) {
		if (DEBUG_LOG_DYN)log_printf("FindExport of %s! From LIB_NN_ACP\n", functionName);
		if (acp_handle == 0) { log_print("LIB_NN_ACP not aquired\n"); return 0; }
		rpl_handle = acp_handle;
	}
	else if (library == LIB_SYSHID) {
		if (DEBUG_LOG_DYN)log_printf("FindExport of %s! From LIB_SYSHID\n", functionName);
		if (syshid_handle == 0) { log_print("LIB_SYSHID not aquired\n"); return 0; }
		rpl_handle = syshid_handle;
	}
	else if (library == LIB_VPADBASE) {
		if (DEBUG_LOG_DYN)log_printf("FindExport of %s! From LIB_VPADBASE\n", functionName);
		if (vpadbase_handle == 0) { log_print("LIB_VPADBASE not aquired\n"); return 0; }
		rpl_handle = vpadbase_handle;
	}

	if (!rpl_handle) {
		log_printf("Failed to find the RPL handle for %s\n", functionName);
		return 0;
	}

	OSDynLoad_FindExport(rpl_handle, 0, functionName, &real_addr);

	if (!real_addr) {
		log_printf("OSDynLoad_FindExport failed for %s\n", functionName);
		return 0;
	}

	if ((u32)(*(volatile unsigned int*)(real_addr) & 0xFF000000) == 0x48000000) {
		real_addr += (u32)(*(volatile unsigned int*)(real_addr) & 0x0000FFFF);
		if ((u32)(*(volatile unsigned int*)(real_addr) & 0xFF000000) == 0x48000000) {
			return 0;
		}
	}

	return real_addr;
}

