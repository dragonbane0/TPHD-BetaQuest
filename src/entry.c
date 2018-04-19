#include <string.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/padscore_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/ax_functions.h"
#include "patcher/function_hooks.h"
#include "fs/fs_utils.h"
#include "fs/sd_fat_devoptab.h"
#include "kernel/kernel_functions.h"
#include "system/exception_handler.h"
#include "system/memory.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "common/common.h"
#include "pygecko.h"
#include "main.h"


int __entry_menu(int argc, char **argv)
{
	//Main Menu is loading, do nothing
	if (OSGetTitleID != 0 && (
		OSGetTitleID() == 0x0005001010040200 //Wii U Menu PAL
		|| OSGetTitleID() == 0x0005001010040100 //Wii U Menu NTSC
		|| OSGetTitleID() == 0x0005001010040000)) //Wii U Menu JPN
	{
		InitOSFunctionPointers();
		InitSocketFunctionPointers();
		InitVPadFunctionPointers(); //for restoring VPAD Read
		InitGX2FunctionPointers(); //Graphics e.g. GX2WaitForVsync
		InitFSFunctionPointers();
		InitPadScoreFunctionPointers();

		//Init twice is needed so logging works properly
		log_init(HOST_IP);
		log_deinit();
		log_init(HOST_IP);

		log_printf("Loading Wii U Main Menu\n");

		RestoreGameInstructions(); //Restore custom game hooks
		RestoreInstructions(); //Restore original VPAD Read function and socket lib finish (and others)

		PatchMethodHooks();
		g_fileRedirectActive = 1;

		log_deinit();

		return EXIT_RELAUNCH_ON_LOAD; 	//EXIT_RELAUNCH_ON_LOAD restarts this app ONCE every time another app loads (menu, game, HBL, etc.)
	}
	
	//Another app is starting (not HBL nor Main Menu), assuming it's a game
	if (OSGetTitleID != 0 &&
		OSGetTitleID() != 0x000500101004A200 && // mii maker eur
		OSGetTitleID() != 0x000500101004A100 && // mii maker usa
		OSGetTitleID() != 0x000500101004A000)   // mii maker jpn)
	{
		InitOSFunctionPointers();
		InitSocketFunctionPointers();
		InitVPadFunctionPointers(); //for patching VPAD Read
		InitGX2FunctionPointers(); //for patching GX2 functions
		InitFSFunctionPointers();
		InitPadScoreFunctionPointers();

		SetupKernelCallback(); //Needed for PatchMethodHooks()

		//Init twice is needed so logging works properly
		log_init(HOST_IP);
		log_deinit();
		log_init(HOST_IP);

		//Patch VPAD Read,socket lib finish and others to inject custom code into them
		PatchMethodHooks();

		//Check for TP HD
		if (OSGetTitleID() != 0 && (OSGetTitleID() == 0x000500001019E500 || OSGetTitleID() == 0x000500001019E600 || OSGetTitleID() == 0x000500001019C800)) //TP HD is running
		{
			g_isTPHD = 1;
			g_fileRedirectActive = 1;

			log_printf("Launching a game - Title ID: 0x%16X\n", OSGetTitleID());

			start_pygecko();

			log_printf("PyGecko thread started!\n");
		}

		log_deinit();

		return EXIT_RELAUNCH_ON_LOAD;
	}


    //! *******************************************************************
    //! *                 Jump to our application                    *
    //! *******************************************************************
    return Menu_Main();
}
