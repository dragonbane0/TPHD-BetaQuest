#ifndef _MAIN_H_
#define _MAIN_H_

#include "common/types.h"
#include "common/game_defs.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/padscore_functions.h"

/* Main */
#ifdef __cplusplus
extern "C" {
#endif

//! C wrapper for our C++ functions
int Menu_Main(void);

//Global Vars
extern const char *HOST_IP;

//For the Input Viewer Thread
extern VPADData g_currentInputData;
extern KPADData g_currentInputDataKPAD;

//For Function Hooks.cpp
extern u8 g_assertion_counter;
extern u8 g_gameRunning;
extern u8 g_fontInit;
extern int g_isTPHD;
extern u32 g_renderWidth;
extern u32 g_renderHeight;
extern f32 g_widthScaleFactor;
extern f32 g_heightScaleFactor;
extern f32 g_depthScaleFactor;

extern u8 g_continueVPADRead;
extern u8 g_frameStepActive;
extern u8 g_frameStepFPS;
extern u8 g_frameStepCounter;
extern u8 g_fileRedirectActive;

extern u8 g_hasGameOvered;
extern u8 g_isLoading;
extern u8 g_forceHuman;
extern u8 g_forceReload;

extern u8 g_checkIntroItemReload;
extern u8 g_doGorgeFallback;

#ifdef __cplusplus
}
#endif

#endif
