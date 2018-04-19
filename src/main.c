#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include <unistd.h>
#include <fcntl.h>
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
#include "common/game_defs.h"
#include "pygecko.h"
#include "ios_exploit.h"

#define BUFFER_SIZE 40000

char *buffer[BUFFER_SIZE] __attribute__((section(".data")));

#define PRINT_TEXT2(x, y, ...) { snprintf(msg, 80, __VA_ARGS__); OSScreenPutFontEx(0, x, y, msg); OSScreenPutFontEx(1, x, y, msg); }

static int app_launched = 0;

//Global Extern Vars
const char *HOST_IP = "192.168.2.104";

VPADData g_currentInputData;
KPADData g_currentInputDataKPAD;

u8 g_assertion_counter = 0;
u8 g_gameRunning = 0;
u8 g_fontInit = 0;
int g_isTPHD = -1;
u32 g_renderWidth = 0;
u32 g_renderHeight = 0;
f32 g_widthScaleFactor = 0;
f32 g_heightScaleFactor = 0;
f32 g_depthScaleFactor = 0;

u8 g_continueVPADRead = 0;
u8 g_frameStepActive = 0;
u8 g_frameStepFPS = 0;
u8 g_frameStepCounter;
u8 g_fileRedirectActive = 0;

u8 g_hasGameOvered = 0;
u8 g_isLoading = 0;
u8 g_forceHuman = 0;
u8 g_forceReload = 0;

u8 g_checkIntroItemReload = 0;
u8 g_doGorgeFallback = 0;


/* Entry point */
int Menu_Main(void)
{
        //!*******************************************************************
        //!                   Initialize function pointers                   *
        //!*******************************************************************
        //! do OS (for acquire) and sockets first so we got logging
		InitOSFunctionPointers(); //various uses
		InitSocketFunctionPointers(); //for logging
		InitSysFunctionPointers(); //for SYSLaunchMenu
		InitVPadFunctionPointers(); //for restoring VPAD Read
		InitGX2FunctionPointers(); //Graphics e.g. GX2WaitForVsync
		InitFSFunctionPointers();
		InitPadScoreFunctionPointers();

		SetupKernelCallback(); //for RestoreInstructions() and printing RPX Name

		//Init twice is needed so logging works properly
        log_init(HOST_IP);
		log_deinit();
		log_init(HOST_IP);

		log_printf("Beta Quest App was launched\n");

        log_printf("Current RPX Name: %s\n", cosAppXmlInfoStruct.rpx_name);
	    log_printf("App Launched Value: %i\n", app_launched);
	

		//Return to HBL if app is launched a second time
		if (app_launched == 1)
		{
			RestoreGameInstructions(); //Restore custom game hooks
			RestoreInstructions(); //Restore original VPAD Read function and socket lib finish (and others)

			log_printf("Returning to HBL\n");
			log_deinit();
			return EXIT_SUCCESS; //Returns to HBL
		}

		//Run Mocha Fast to patch SD access
		log_printf("Run IOSExploit\n");

		cfw_config_t config;
		default_config(&config);

		config.directLaunch = 1;
		config.launchSysMenu = 0;
		config.noIosReload = 1;
		config.redNAND = 0;
		config.launchImage = 0;

		int res = ExecuteIOSExploit(&config);

		if (res == 0)
		{
			log_printf("IOSExploit had success!\n");
		}
		else
		{
			log_printf("IOSExploit critical error!\n");
		}

		log_printf("Beta Quest prepared for launch\n");
	
		app_launched = 1;

		int error;
		VPADData inputs;
		
		VPADRead(0, &inputs, 1, &error);

		if (((inputs.btns_d & VPAD_BUTTON_R) != VPAD_BUTTON_R) && ((inputs.btns_h & VPAD_BUTTON_R) != VPAD_BUTTON_R))
		{
			log_printf("Booting TP HD\n");

			PatchMethodHooks();
			g_fileRedirectActive = 1;

			log_deinit();

			if (SYSCheckTitleExists(0x000500001019E500))
			{
				SYSLaunchTitle(0x000500001019E500);
				return EXIT_RELAUNCH_ON_LOAD;
			}
			else if (SYSCheckTitleExists(0x000500001019E600))
			{
				SYSLaunchTitle(0x000500001019E600);
				return EXIT_RELAUNCH_ON_LOAD;
			}
			else if (SYSCheckTitleExists(0x000500001019C800))
			{
				SYSLaunchTitle(0x000500001019C800);
				return EXIT_RELAUNCH_ON_LOAD;
			}
		}

		log_deinit();

		SYSLaunchMenu(); //Launches the Wii U Main Menu

        return EXIT_RELAUNCH_ON_LOAD;
}
