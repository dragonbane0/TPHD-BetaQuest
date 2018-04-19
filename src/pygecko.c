#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include "common/common.h"
#include "common/thread_defs.h"
#include "main.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/ax_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/padscore_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "kernel/syscalls.h"
#include "system/exception_handler.h"
#include "utils/logger.h"
#include "saviine.h"
#include "patcher/function_hooks.h"

struct pygecko_bss_t 
{
	int error, line;
	OSThread thread;
	unsigned char stack[0x5000];
};


#define CHECK_ERROR(cond) if (cond) { bss->line = __LINE__; goto error; }
#define errno (*__gh_errno_ptr())
#define MSG_DONTWAIT 32
#define EWOULDBLOCK 6

static int IsBitSet(u8 b, int pos)
{
	return (b & (1 << pos)) != 0;
}

static unsigned int CheckTWWCompatibility(unsigned int *ptr, int isTWWHD, int compatibilityModeTWW)
{
	unsigned int address = (unsigned int)ptr;

	if (isTWWHD == 1 && compatibilityModeTWW == 1 && address > 0x15000000) //TWW HD Trainer Compatibility
	{
		const unsigned char *MemoryCardPointerPtr = (const unsigned char *)0x1096346C;
		unsigned int MemoryCardPointer;

		//Get Memory Card Pointer
		memcpy(&MemoryCardPointer, MemoryCardPointerPtr, 4);

		if (MemoryCardPointer > 0x10000000)
		{
			unsigned int offset = address - 0x14FC49B0; //Value back then
			unsigned int newAddress = MemoryCardPointer + offset;

			return newAddress;
		}
	}

	return address;
}

static int recvwait(struct pygecko_bss_t *bss, int sock, void *buffer, int len) {
	int ret;
	while (len > 0) {
		ret = recv(sock, buffer, len, 0);
		CHECK_ERROR(ret < 0);
		len -= ret;
		buffer += ret;
	}
	return 0;
error:
	bss->error = ret;
	return ret;
}

static int recvbyte(struct pygecko_bss_t *bss, int sock) {
	unsigned char buffer[1];
	int ret;

	ret = recvwait(bss, sock, buffer, 1);
	if (ret < 0) return ret;
	return buffer[0];
}

static int checkbyte(struct pygecko_bss_t *bss, int sock) {
	unsigned char buffer[1];
	int ret;

	ret = recv(sock, buffer, 1, MSG_DONTWAIT);
	if (ret < 0) return ret;
	if (ret == 0) return ret; //-1
	return buffer[0];
}

static int sendwait(struct pygecko_bss_t *bss, int sock, const void *buffer, int len) {
	int ret;
	while (len > 0) {
		ret = send(sock, buffer, len, 0);
		CHECK_ERROR(ret < 0);
		len -= ret;
		buffer += ret;
	}
	return 0;
error:
	bss->error = ret;
	return ret;
}

static int sendbyte(struct pygecko_bss_t *bss, int sock, unsigned char byte) {
	unsigned char buffer[1];

	buffer[0] = byte;
	return sendwait(bss, sock, buffer, 1);
}

//Send Inputs Function
static int SendInputs(struct pygecko_bss_t *bss, int clientfd, int sendDirections)
{
	int ret = 0;

	unsigned char Directions1[1];
	unsigned char Directions2[1];

	Directions1[0] = 0;
	Directions2[0] = 0;

	unsigned char Buttons1[1];
	unsigned char Buttons2[1];
	unsigned char Buttons3[1];

	Buttons1[0] = 0;
	Buttons2[0] = 0;
	Buttons3[0] = 0;

	//Use Pro Controller if it is in use
	if (g_currentInputDataKPAD.classic.btns_d != 0 || g_currentInputDataKPAD.classic.btns_h != 0 || g_currentInputDataKPAD.classic.btns_r != 0 ||
		g_currentInputDataKPAD.classic.lstick_x > 0.05f || g_currentInputDataKPAD.classic.lstick_y > 0.05f ||
		g_currentInputDataKPAD.classic.rstick_x > 0.05f || g_currentInputDataKPAD.classic.rstick_y > 0.05f ||
		g_currentInputDataKPAD.classic.lstick_x < -0.05f || g_currentInputDataKPAD.classic.lstick_y < -0.05f ||
		g_currentInputDataKPAD.classic.rstick_x < -0.05f || g_currentInputDataKPAD.classic.rstick_y < -0.05f)
	{
		//Sticks
		ret = sendwait(bss, clientfd, &g_currentInputDataKPAD.classic.lstick_x, 4);
		CHECK_ERROR(ret < 0);

		ret = sendwait(bss, clientfd, &g_currentInputDataKPAD.classic.lstick_y, 4);
		CHECK_ERROR(ret < 0);

		ret = sendwait(bss, clientfd, &g_currentInputDataKPAD.classic.rstick_x, 4);
		CHECK_ERROR(ret < 0);

		ret = sendwait(bss, clientfd, &g_currentInputDataKPAD.classic.rstick_y, 4);
		CHECK_ERROR(ret < 0);

		//Directions Left Stick
		if (g_currentInputDataKPAD.classic.lstick_y > 0.05f)
			Directions1[0] += 1;
		else if (g_currentInputDataKPAD.classic.lstick_y < -0.05f)
			Directions1[0] += 4;

		if (g_currentInputDataKPAD.classic.lstick_x > 0.05f)
			Directions1[0] += 2;
		else if (g_currentInputDataKPAD.classic.lstick_x < -0.05f)
			Directions1[0] += 8;

		//Directions Right Stick
		if (g_currentInputDataKPAD.classic.rstick_y > 0.05f)
			Directions2[0] += 1;
		else if (g_currentInputDataKPAD.classic.rstick_y < -0.05f)
			Directions2[0] += 4;

		if (g_currentInputDataKPAD.classic.rstick_x > 0.05f)
			Directions2[0] += 2;
		else if (g_currentInputDataKPAD.classic.rstick_x < -0.05f)
			Directions2[0] += 8;

		if (sendDirections == 1) //Data Viewer Apps Only
		{
			ret = sendwait(bss, clientfd, Directions1, 1);
			CHECK_ERROR(ret < 0);

			ret = sendwait(bss, clientfd, Directions2, 1);
			CHECK_ERROR(ret < 0);
		}

		//Button 1
		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_A) == WPAD_CLASSIC_BUTTON_A) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_A) == WPAD_CLASSIC_BUTTON_A)
			Buttons1[0] += 128;

		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_B) == WPAD_CLASSIC_BUTTON_B) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_B) == WPAD_CLASSIC_BUTTON_B)
			Buttons1[0] += 64;

		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_X) == WPAD_CLASSIC_BUTTON_X) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_X) == WPAD_CLASSIC_BUTTON_X)
			Buttons1[0] += 32;

		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_Y) == WPAD_CLASSIC_BUTTON_Y) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_Y) == WPAD_CLASSIC_BUTTON_Y)
			Buttons1[0] += 16;

		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_LEFT) == WPAD_CLASSIC_BUTTON_LEFT) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_LEFT) == WPAD_CLASSIC_BUTTON_LEFT)
			Buttons1[0] += 8;

		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_RIGHT) == WPAD_CLASSIC_BUTTON_RIGHT) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_RIGHT) == WPAD_CLASSIC_BUTTON_RIGHT)
			Buttons1[0] += 4;

		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_UP) == WPAD_CLASSIC_BUTTON_UP) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_UP) == WPAD_CLASSIC_BUTTON_UP)
			Buttons1[0] += 2;

		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_DOWN) == WPAD_CLASSIC_BUTTON_DOWN) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_DOWN) == WPAD_CLASSIC_BUTTON_DOWN)
			Buttons1[0] += 1;


		//Button 2
		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_ZL) == WPAD_CLASSIC_BUTTON_ZL) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_ZL) == WPAD_CLASSIC_BUTTON_ZL)
			Buttons2[0] += 128;

		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_ZR) == WPAD_CLASSIC_BUTTON_ZR) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_ZR) == WPAD_CLASSIC_BUTTON_ZR)
			Buttons2[0] += 64;

		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_L) == WPAD_CLASSIC_BUTTON_L) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_L) == WPAD_CLASSIC_BUTTON_L)
			Buttons2[0] += 32;

		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_R) == WPAD_CLASSIC_BUTTON_R) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_R) == WPAD_CLASSIC_BUTTON_R)
			Buttons2[0] += 16;

		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_PLUS) == WPAD_CLASSIC_BUTTON_PLUS) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_PLUS) == WPAD_CLASSIC_BUTTON_PLUS)
			Buttons2[0] += 8;

		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_MINUS) == WPAD_CLASSIC_BUTTON_MINUS) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_MINUS) == WPAD_CLASSIC_BUTTON_MINUS)
			Buttons2[0] += 4;

		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_STICK_L) == WPAD_CLASSIC_BUTTON_STICK_L) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_STICK_L) == WPAD_CLASSIC_BUTTON_STICK_L)
			Buttons2[0] += 2;

		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_STICK_R) == WPAD_CLASSIC_BUTTON_STICK_R) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_STICK_R) == WPAD_CLASSIC_BUTTON_STICK_R)
			Buttons2[0] += 1;


		//Button 3
		if (((g_currentInputDataKPAD.classic.btns_d & WPAD_CLASSIC_BUTTON_HOME) == WPAD_CLASSIC_BUTTON_HOME) || (g_currentInputDataKPAD.classic.btns_h & WPAD_CLASSIC_BUTTON_HOME) == WPAD_CLASSIC_BUTTON_HOME)
			Buttons3[0] += 2;
	}
	else //Fall back to Gamepad by default
	{
		//Sticks
		ret = sendwait(bss, clientfd, &g_currentInputData.lstick.x, 4);
		CHECK_ERROR(ret < 0);

		ret = sendwait(bss, clientfd, &g_currentInputData.lstick.y, 4);
		CHECK_ERROR(ret < 0);

		ret = sendwait(bss, clientfd, &g_currentInputData.rstick.x, 4);
		CHECK_ERROR(ret < 0);

		ret = sendwait(bss, clientfd, &g_currentInputData.rstick.y, 4);
		CHECK_ERROR(ret < 0);

		//Directions Left Stick
		if (g_currentInputData.lstick.y > 0.05f)
			Directions1[0] += 1;
		else if (g_currentInputData.lstick.y < -0.05f)
			Directions1[0] += 4;

		if (g_currentInputData.lstick.x > 0.05f)
			Directions1[0] += 2;
		else if (g_currentInputData.lstick.x < -0.05f)
			Directions1[0] += 8;

		//Directions Right Stick
		if (g_currentInputData.rstick.y > 0.05f)
			Directions2[0] += 1;
		else if (g_currentInputData.rstick.y < -0.05f)
			Directions2[0] += 4;

		if (g_currentInputData.rstick.x > 0.05f)
			Directions2[0] += 2;
		else if (g_currentInputData.rstick.x < -0.05f)
			Directions2[0] += 8;

		if (sendDirections == 1) //Data Viewer Apps Only
		{
			ret = sendwait(bss, clientfd, Directions1, 1);
			CHECK_ERROR(ret < 0);

			ret = sendwait(bss, clientfd, Directions2, 1);
			CHECK_ERROR(ret < 0);
		}

		//Button 1
		if (((g_currentInputData.btns_d & VPAD_BUTTON_A) == VPAD_BUTTON_A) || (g_currentInputData.btns_h & VPAD_BUTTON_A) == VPAD_BUTTON_A)
			Buttons1[0] += 128;

		if (((g_currentInputData.btns_d & VPAD_BUTTON_B) == VPAD_BUTTON_B) || (g_currentInputData.btns_h & VPAD_BUTTON_B) == VPAD_BUTTON_B)
			Buttons1[0] += 64;

		if (((g_currentInputData.btns_d & VPAD_BUTTON_X) == VPAD_BUTTON_X) || (g_currentInputData.btns_h & VPAD_BUTTON_X) == VPAD_BUTTON_X)
			Buttons1[0] += 32;

		if (((g_currentInputData.btns_d & VPAD_BUTTON_Y) == VPAD_BUTTON_Y) || (g_currentInputData.btns_h & VPAD_BUTTON_Y) == VPAD_BUTTON_Y)
			Buttons1[0] += 16;

		if (((g_currentInputData.btns_d & VPAD_BUTTON_LEFT) == VPAD_BUTTON_LEFT) || (g_currentInputData.btns_h & VPAD_BUTTON_LEFT) == VPAD_BUTTON_LEFT)
			Buttons1[0] += 8;

		if (((g_currentInputData.btns_d & VPAD_BUTTON_RIGHT) == VPAD_BUTTON_RIGHT) || (g_currentInputData.btns_h & VPAD_BUTTON_RIGHT) == VPAD_BUTTON_RIGHT)
			Buttons1[0] += 4;

		if (((g_currentInputData.btns_d & VPAD_BUTTON_UP) == VPAD_BUTTON_UP) || (g_currentInputData.btns_h & VPAD_BUTTON_UP) == VPAD_BUTTON_UP)
			Buttons1[0] += 2;

		if (((g_currentInputData.btns_d & VPAD_BUTTON_DOWN) == VPAD_BUTTON_DOWN) || (g_currentInputData.btns_h & VPAD_BUTTON_DOWN) == VPAD_BUTTON_DOWN)
			Buttons1[0] += 1;


		//Button 2
		if (((g_currentInputData.btns_d & VPAD_BUTTON_ZL) == VPAD_BUTTON_ZL) || (g_currentInputData.btns_h & VPAD_BUTTON_ZL) == VPAD_BUTTON_ZL)
			Buttons2[0] += 128;

		if (((g_currentInputData.btns_d & VPAD_BUTTON_ZR) == VPAD_BUTTON_ZR) || (g_currentInputData.btns_h & VPAD_BUTTON_ZR) == VPAD_BUTTON_ZR)
			Buttons2[0] += 64;

		if (((g_currentInputData.btns_d & VPAD_BUTTON_L) == VPAD_BUTTON_L) || (g_currentInputData.btns_h & VPAD_BUTTON_L) == VPAD_BUTTON_L)
			Buttons2[0] += 32;

		if (((g_currentInputData.btns_d & VPAD_BUTTON_R) == VPAD_BUTTON_R) || (g_currentInputData.btns_h & VPAD_BUTTON_R) == VPAD_BUTTON_R)
			Buttons2[0] += 16;

		if (((g_currentInputData.btns_d & VPAD_BUTTON_PLUS) == VPAD_BUTTON_PLUS) || (g_currentInputData.btns_h & VPAD_BUTTON_PLUS) == VPAD_BUTTON_PLUS)
			Buttons2[0] += 8;

		if (((g_currentInputData.btns_d & VPAD_BUTTON_MINUS) == VPAD_BUTTON_MINUS) || (g_currentInputData.btns_h & VPAD_BUTTON_MINUS) == VPAD_BUTTON_MINUS)
			Buttons2[0] += 4;

		if (((g_currentInputData.btns_d & VPAD_BUTTON_STICK_L) == VPAD_BUTTON_STICK_L) || (g_currentInputData.btns_h & VPAD_BUTTON_STICK_L) == VPAD_BUTTON_STICK_L)
			Buttons2[0] += 2;

		if (((g_currentInputData.btns_d & VPAD_BUTTON_STICK_R) == VPAD_BUTTON_STICK_R) || (g_currentInputData.btns_h & VPAD_BUTTON_STICK_R) == VPAD_BUTTON_STICK_R)
			Buttons2[0] += 1;


		//Button 3
		if (((g_currentInputData.btns_d & VPAD_BUTTON_TV) == VPAD_BUTTON_TV) || (g_currentInputData.btns_h & VPAD_BUTTON_TV) == VPAD_BUTTON_TV)
			Buttons3[0] += 1;

		if (((g_currentInputData.btns_d & VPAD_BUTTON_HOME) == VPAD_BUTTON_HOME) || (g_currentInputData.btns_h & VPAD_BUTTON_HOME) == VPAD_BUTTON_HOME)
			Buttons3[0] += 2;

		if (g_currentInputData.tpdata.touched == 1 || g_currentInputData.tpdata1.touched == 1 || g_currentInputData.tpdata2.touched == 1)
			Buttons3[0] += 4;
	}

	ret = sendwait(bss, clientfd, Buttons1, 1);
	CHECK_ERROR(ret < 0);

	ret = sendwait(bss, clientfd, Buttons2, 1);
	CHECK_ERROR(ret < 0);

	ret = sendwait(bss, clientfd, Buttons3, 1);
	CHECK_ERROR(ret < 0);

	return 0;

error:
	bss->error = ret;
	return -1;
}

//Handles the connection to the Input Viewer App (NintendoSpy)
static int run_inputViewer(struct pygecko_bss_t *bss, int clientfd)
{
	int ret = 0;

	unsigned char Cmd[1];
	Cmd[0] = 0x01;

	while (1)
	{
		GX2WaitForVsync();

		//Send Cmd Bit
		ret = sendwait(bss, clientfd, Cmd, 1);
		CHECK_ERROR(ret < 0);

		ret = SendInputs(bss, clientfd, 0);
		CHECK_ERROR(ret < 0);
	}

error:
	bss->error = ret;
	return ret;
}

static int rungecko_logger(struct pygecko_bss_t *bss, int clientfd) {
	int ret;

	unsigned char Cmd[1];
	Cmd[0] = 0x01;

	//TWW HD Stuff (+6FF000)
	const unsigned char *frameCountPtr = (const unsigned char *)0x10701758;
	unsigned char frameCount[4];

	const unsigned char *LinkXPtr = (const unsigned char *)0x1096EF48;
	unsigned char LinkX[4];

	const unsigned char *LinkYPtr = (const unsigned char *)0x1096EF4C;
	unsigned char LinkY[4];

	const unsigned char *LinkZPtr = (const unsigned char *)0x1096EF50;
	unsigned char LinkZ[4];

	const unsigned char *AnglePtr = (const unsigned char *)0x1096EF12;
	unsigned char Angle[2];

	const unsigned char *LinkPointerPtr = (const unsigned char *)0x10989C74;
	unsigned int LinkPointer;

	unsigned char PotSpeed[4];
	unsigned char ActSpeed[4];

	const unsigned char *CurrentStagePtr = (const unsigned char *)0x109763E4;
	unsigned char CurrentStage[7];

	const unsigned char *CurrentRoomPtr = (const unsigned char *)0x10978CF8; 
	unsigned char CurrentRoom[1];

	const unsigned char *CurrentSpawnPtr = (const unsigned char *)0x109763ED;
	unsigned char CurrentSpawn[1];

	const unsigned char *CurrentLayerPtr = (const unsigned char *)0x109763EF;
	unsigned char CurrentLayer[1];


	//TP HD Stuff : +631E00
	const unsigned char *frameCountPtr_TP = (const unsigned char *)0x1068560C;
	unsigned char frameCount_TP[4];

	const unsigned char *LinkPointerPtr_TP = (const unsigned char *)0x10681E40; //10050040 
	unsigned int LinkPointer_TP;

	unsigned char LinkX_TP[4];
	unsigned char LinkY_TP[4];
	unsigned char LinkZ_TP[4];
	unsigned char Angle_TP[2];

	unsigned char PotSpeed_TP[4];
	unsigned char ActSpeed_TP[4];
	unsigned char LinkStatus_TP[1];


	const unsigned char *EponaPointerPtr_TP = (const unsigned char *)0x1064DDBC;
	unsigned int EponaPointer_TP;

	unsigned char EponaX_TP[4];
	unsigned char EponaY_TP[4];
	unsigned char EponaZ_TP[4];
	unsigned char EponaAngle_TP[2];

	const unsigned char *EponaStagePtr_TP = (const unsigned char *)0x10647B96;
	unsigned char EponaStage_TP[7];

	const unsigned char *EponaRoomPtr_TP = (const unsigned char *)0x10647B9F;
	unsigned char EponaRoom_TP[1];


	const unsigned char *SaveStagePtr_TP = (const unsigned char *)0x10647BA0;
	unsigned char SaveStage_TP[7];

	const unsigned char *SaveRoomPtr_TP = (const unsigned char *)0x10647BA9;
	unsigned char SaveRoom_TP[1];

	const unsigned char *SaveSpawnPtr_TP = (const unsigned char *)0x10647BA8;
	unsigned char SaveSpawn_TP[1];


	const unsigned char *CurrentStagePtr_TP = (const unsigned char *)0x1064CDE8;
	unsigned char CurrentStage_TP[7];

	const unsigned char *CurrentRoomPtr_TP = (const unsigned char *)0x106813D4;
	unsigned char CurrentRoom_TP[1];

	const unsigned char *CurrentSpawnPtr_TP = (const unsigned char *)0x1064CDF1;
	unsigned char CurrentSpawn_TP[1];

	const unsigned char *CurrentStatePtr_TP = (const unsigned char *)0x1062915B;
	unsigned char CurrentState_TP[1];


	while (1) 
	{
	    GX2WaitForVsync();

		if (g_isTPHD == 0) //TWW HD is running
		{
			//Send Cmd Bit
			ret = sendwait(bss, clientfd, Cmd, 1);
			CHECK_ERROR(ret < 0);

			memcpy(frameCount, frameCountPtr, 4);
			ret = sendwait(bss, clientfd, frameCount, 4);
			CHECK_ERROR(ret < 0);

			memcpy(LinkX, LinkXPtr, 4);
			ret = sendwait(bss, clientfd, LinkX, 4);
			CHECK_ERROR(ret < 0);

			memcpy(LinkY, LinkYPtr, 4);
			ret = sendwait(bss, clientfd, LinkY, 4);
			CHECK_ERROR(ret < 0);

			memcpy(LinkZ, LinkZPtr, 4);
			ret = sendwait(bss, clientfd, LinkZ, 4);
			CHECK_ERROR(ret < 0);

			memcpy(Angle, AnglePtr, 2);
			ret = sendwait(bss, clientfd, Angle, 2);
			CHECK_ERROR(ret < 0);

			//Pointer Section
			memcpy(&LinkPointer, LinkPointerPtr, 4);
			if (LinkPointer > 0x40000000)
			{
				unsigned char *potSpeedPtr = (unsigned char *)LinkPointer + 0x6938;
				unsigned char *actSpeedPtr = (unsigned char *)LinkPointer + 0x294;

				memcpy(PotSpeed, potSpeedPtr, 4);
				memcpy(ActSpeed, actSpeedPtr, 4);
			}
			else
			{
				memset(PotSpeed, 0, 4);
				memset(ActSpeed, 0, 4);
			}

			ret = sendwait(bss, clientfd, PotSpeed, 4);
			CHECK_ERROR(ret < 0);

			ret = sendwait(bss, clientfd, ActSpeed, 4);
			CHECK_ERROR(ret < 0);

			//Current Map Info Stuff
			memcpy(CurrentStage, CurrentStagePtr, 7);
			ret = sendwait(bss, clientfd, CurrentStage, 7);
			CHECK_ERROR(ret < 0);

			memcpy(CurrentRoom, CurrentRoomPtr, 1);
			ret = sendwait(bss, clientfd, CurrentRoom, 1);
			CHECK_ERROR(ret < 0);

			memcpy(CurrentSpawn, CurrentSpawnPtr, 1);
			ret = sendwait(bss, clientfd, CurrentSpawn, 1);
			CHECK_ERROR(ret < 0);

			memcpy(CurrentLayer, CurrentLayerPtr, 1);
			ret = sendwait(bss, clientfd, CurrentLayer, 1);
			CHECK_ERROR(ret < 0);

			//Inputs
			ret = SendInputs(bss, clientfd, 1);

			CHECK_ERROR(ret < 0);
		}
		else if (g_isTPHD == 1) //TP HD is running
		{
			//Send Cmd Bit
			ret = sendwait(bss, clientfd, Cmd, 1);
			CHECK_ERROR(ret < 0);

			memcpy(frameCount_TP, frameCountPtr_TP, 4);
			ret = sendwait(bss, clientfd, frameCount_TP, 4);
			CHECK_ERROR(ret < 0);

			//Link Pointer Section
			memcpy(&LinkPointer_TP, LinkPointerPtr_TP, 4);

			if (LinkPointer_TP < 0x10000000)
			{
				memset(LinkX_TP, 0, 4);
				memset(LinkY_TP, 0, 4);
				memset(LinkZ_TP, 0, 4);
				memset(Angle_TP, 0, 2);
				memset(PotSpeed_TP, 0, 4);
				memset(ActSpeed_TP, 0, 4);
				memset(LinkStatus_TP, 0, 1);
			}
			else
			{
				unsigned char *LinkXPtr_TP = (unsigned char *)LinkPointer_TP + 0x0;
				unsigned char *LinkYPtr_TP = (unsigned char *)LinkPointer_TP + 0x4;
				unsigned char *LinkZPtr_TP = (unsigned char *)LinkPointer_TP + 0x8;
				unsigned char *AnglePtr_TP = (unsigned char *)LinkPointer_TP + 0x16;

				unsigned char *potSpeedPtr_TP = (unsigned char *)LinkPointer_TP + 0x2F88;
				unsigned char *actSpeedPtr_TP = (unsigned char *)LinkPointer_TP + 0x5c;
				unsigned char *linkStatusPtr_TP = (unsigned char *)LinkPointer_TP + 0xA6;

				memcpy(LinkX_TP, LinkXPtr_TP, 4);
				memcpy(LinkY_TP, LinkYPtr_TP, 4);
				memcpy(LinkZ_TP, LinkZPtr_TP, 4);
				memcpy(Angle_TP, AnglePtr_TP, 2);

				memcpy(PotSpeed_TP, potSpeedPtr_TP, 4);
				memcpy(ActSpeed_TP, actSpeedPtr_TP, 4);
				memcpy(LinkStatus_TP, linkStatusPtr_TP, 1);
			}

			ret = sendwait(bss, clientfd, LinkX_TP, 4);
			CHECK_ERROR(ret < 0);

			ret = sendwait(bss, clientfd, LinkY_TP, 4);
			CHECK_ERROR(ret < 0);

			ret = sendwait(bss, clientfd, LinkZ_TP, 4);
			CHECK_ERROR(ret < 0);

			ret = sendwait(bss, clientfd, Angle_TP, 2);
			CHECK_ERROR(ret < 0);

			ret = sendwait(bss, clientfd, PotSpeed_TP, 4);
			CHECK_ERROR(ret < 0);

			ret = sendwait(bss, clientfd, ActSpeed_TP, 4);
			CHECK_ERROR(ret < 0);

			ret = sendwait(bss, clientfd, LinkStatus_TP, 1);
			CHECK_ERROR(ret < 0);


			//Epona Pointer Section
			memcpy(&EponaPointer_TP, EponaPointerPtr_TP, 4);

			if (EponaPointer_TP < 0x10000000)
			{
				memset(EponaX_TP, 0, 4);
				memset(EponaY_TP, 0, 4);
				memset(EponaZ_TP, 0, 4);
				memset(EponaAngle_TP, 0, 2);
			}
			else
			{
				unsigned char *EponaXPtr_TP = (unsigned char *)EponaPointer_TP + 0x4D0;
				unsigned char *EponaYPtr_TP = (unsigned char *)EponaPointer_TP + 0x4D4;
				unsigned char *EponaZPtr_TP = (unsigned char *)EponaPointer_TP + 0x4D8;
				unsigned char *EponaAnglePtr_TP = (unsigned char *)EponaPointer_TP + 0x4E6;

				memcpy(EponaX_TP, EponaXPtr_TP, 4);
				memcpy(EponaY_TP, EponaYPtr_TP, 4);
				memcpy(EponaZ_TP, EponaZPtr_TP, 4);
				memcpy(EponaAngle_TP, EponaAnglePtr_TP, 2);
			}

			ret = sendwait(bss, clientfd, EponaX_TP, 4);
			CHECK_ERROR(ret < 0);

			ret = sendwait(bss, clientfd, EponaY_TP, 4);
			CHECK_ERROR(ret < 0);

			ret = sendwait(bss, clientfd, EponaZ_TP, 4);
			CHECK_ERROR(ret < 0);

			ret = sendwait(bss, clientfd, EponaAngle_TP, 2);
			CHECK_ERROR(ret < 0);

			//Epona Stage
			memcpy(EponaStage_TP, EponaStagePtr_TP, 7);
			ret = sendwait(bss, clientfd, EponaStage_TP, 7);
			CHECK_ERROR(ret < 0);

			memcpy(EponaRoom_TP, EponaRoomPtr_TP, 1);
			ret = sendwait(bss, clientfd, EponaRoom_TP, 1);
			CHECK_ERROR(ret < 0);

			//Savefile Stuff
			memcpy(SaveStage_TP, SaveStagePtr_TP, 7);
			ret = sendwait(bss, clientfd, SaveStage_TP, 7);
			CHECK_ERROR(ret < 0);

			memcpy(SaveRoom_TP, SaveRoomPtr_TP, 1);
			ret = sendwait(bss, clientfd, SaveRoom_TP, 1);
			CHECK_ERROR(ret < 0);

			memcpy(SaveSpawn_TP, SaveSpawnPtr_TP, 1);
			ret = sendwait(bss, clientfd, SaveSpawn_TP, 1);
			CHECK_ERROR(ret < 0);

			//Current Map Info Stuff
			memcpy(CurrentStage_TP, CurrentStagePtr_TP, 7);
			ret = sendwait(bss, clientfd, CurrentStage_TP, 7);
			CHECK_ERROR(ret < 0);

			memcpy(CurrentRoom_TP, CurrentRoomPtr_TP, 1);
			ret = sendwait(bss, clientfd, CurrentRoom_TP, 1);
			CHECK_ERROR(ret < 0);

			memcpy(CurrentSpawn_TP, CurrentSpawnPtr_TP, 1);
			ret = sendwait(bss, clientfd, CurrentSpawn_TP, 1);
			CHECK_ERROR(ret < 0);

			memcpy(CurrentState_TP, CurrentStatePtr_TP, 1);
			ret = sendwait(bss, clientfd, CurrentState_TP, 1);
			CHECK_ERROR(ret < 0);

			//Inputs
			ret = SendInputs(bss, clientfd, 1);

			CHECK_ERROR(ret < 0);			
		}
    }

	return 0;
error:
	bss->error = ret;
	return 0;
}

static int rungecko_hacker(struct pygecko_bss_t *bss, int clientfd) {
	int ret;
	unsigned char buffer[0x401];

	int startActionOnNextFrame = 0;
	int actionID = 0;
	int zeldaID = 0;
	u32 storedLastFrame = 0;

	int isTWWHD = 0;
	int compatibilityModeTWW = 1;

	if (OSGetTitleID() != 0 && (OSGetTitleID() == 0x0005000010143500 || OSGetTitleID() == 0x0005000010143600 || OSGetTitleID() == 0x0005000010143400)) //TWW HD is running
		isTWWHD = 1;
	else
		isTWWHD = 0;

	log_printf("Initialized Hacker Connection...");

	while (1) 
	{
		GX2WaitForVsync(); //Always wait a frame to allow thread cancellation

		ret = checkbyte(bss, clientfd);

		if (ret < 0) 
		{
			CHECK_ERROR(errno != EWOULDBLOCK);

			if (startActionOnNextFrame == 1)
			{
				//Grab current framecount
				u32 frameCount = 0;

				if (zeldaID == 0) //TWW HD
				{
					const unsigned char *frameCountPtr = (const unsigned char *)0x10701758;

					memcpy(&frameCount, frameCountPtr, 4);
				}
				else if (zeldaID == 1) //TP HD
				{
					const unsigned char *frameCountPtr_TP = (const unsigned char *)0x1068560C;
					
					memcpy(&frameCount, frameCountPtr_TP, 4);
				}

				if (frameCount != storedLastFrame)
				{
					startActionOnNextFrame = 0;

					if (actionID == 1) //TWW: Start Event to cancel Barrier Knockback
					{
						const unsigned char *StartEventPtr = (const unsigned char *)0x1097653D;
						const unsigned char *EventCancelPtr = (const unsigned char *)0x10976543;

						int *startEvent;
						char *cancelEvent;

						startEvent = ((int *)StartEventPtr);
						*startEvent = ((int)1);
						DCFlushRange(startEvent, 4);

						cancelEvent = ((char *)EventCancelPtr);
						*cancelEvent = 1;
						DCFlushRange(cancelEvent, 1);
					}
					else if (actionID == 2) //TWW: Reset Collisions after Position Restore
					{
						const unsigned char *CollisionPointerPtr = (const unsigned char *)0x1097648C;
						unsigned int CollisionPointer;
						
						//Get Collision Pointer
						memcpy(&CollisionPointer, CollisionPointerPtr, 4);

						if (CollisionPointer > 0x40000000)
						{
							unsigned char *collision1Ptr = (unsigned char *)CollisionPointer + 0x836;
							unsigned char *collision2Ptr = (unsigned char *)CollisionPointer + 0x837;

							char *collision1;
							char *collision2;

							collision1 = ((char *)collision1Ptr);
							*collision1 = 0;
							DCFlushRange(collision1, 1);

							collision2 = ((char *)collision2Ptr);
							*collision2 = 0;
							DCFlushRange(collision2, 1);
						}
					}
					else if (actionID == 3 || actionID == 4) //TP: Load Title Screen/File Menu depending on action
					{
						const unsigned char *TitleScreenPointerPtr = (const unsigned char *)0x10680E7C;
						unsigned int TitleScreenPointer;

						//Get Title Screen Pointer
						memcpy(&TitleScreenPointer, TitleScreenPointerPtr, 4);

						if (TitleScreenPointer > 0x10000000)
						{
							unsigned char *TitleScreenFlagPtr = (unsigned char *)TitleScreenPointer + 0x45;

							char *titleScreenFlagNew;
							titleScreenFlagNew = ((char *)TitleScreenFlagPtr);

							if (actionID == 3)
								*titleScreenFlagNew = 12; //Load Title
							else if (actionID == 4)
								*titleScreenFlagNew = 13; //Load File Menu
	
							DCFlushRange(titleScreenFlagNew, 1);
						}
						else
						{
							//Refresh by grabbing current framecount again
							const unsigned char *frameCountPtr_TP = (const unsigned char *)0x1068560C;
							memcpy(&storedLastFrame, frameCountPtr_TP, 4);

							startActionOnNextFrame = 1;
						}
					}
				}
			}

			continue;
		}

		switch (ret) {
		case 0x01: { /* cmd_poke08 */
			char *ptr;
			ret = recvwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);

			ptr = ((char **)buffer)[0];

			ptr = ((char *)CheckTWWCompatibility((unsigned int*)ptr, isTWWHD, compatibilityModeTWW));

			*ptr = buffer[7];
			DCFlushRange(ptr, 1);
			break;
		}
		case 0x02: { /* cmd_poke16 */
			short *ptr;
			ret = recvwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);

			ptr = ((short **)buffer)[0];

			ptr = ((short *)CheckTWWCompatibility((unsigned int*)ptr, isTWWHD, compatibilityModeTWW));

			*ptr = ((short *)buffer)[3];
			DCFlushRange(ptr, 2);
			break;
		}
		case 0x03: { /* cmd_pokemem */
			int *ptr;
			ret = recvwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);

			ptr = ((int **)buffer)[0];

			ptr = ((int *)CheckTWWCompatibility((unsigned int*)ptr, isTWWHD, compatibilityModeTWW));

			*ptr = ((int *)buffer)[1];
			DCFlushRange(ptr, 4);
			break;
		}
		case 0x04: { /* cmd_readmem */
			const unsigned char *ptr, *end;
			ret = recvwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);
			ptr = ((const unsigned char **)buffer)[0];
			end = ((const unsigned char **)buffer)[1];

			while (ptr != end) {
				int len, i;

				len = end - ptr;
				if (len > 0x400)
					len = 0x400;
				for (i = 0; i < len; i++)
					if (ptr[i] != 0) break;

				if (i == len) { // all zero!
					ret = sendbyte(bss, clientfd, 0xb0);
					CHECK_ERROR(ret < 0);
				}
				else {
					memcpy(buffer + 1, ptr, len);
					buffer[0] = 0xbd;
					ret = sendwait(bss, clientfd, buffer, len + 1);
					CHECK_ERROR(ret < 0);
				}

				ret = checkbyte(bss, clientfd);
				if (ret == 0xcc) /* GCFAIL */
					goto next_cmd;
				ptr += len;
			}
			break;
		}
		case 0x0b: { /* cmd_writekern */
			void *ptr, *value;
			ret = recvwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);

			ptr = ((void **)buffer)[0];
			value = ((void **)buffer)[1];

			kern_write(ptr, (uint32_t)value);
			break;
		}
		case 0x0c: { /* cmd_readkern */
			void *ptr, *value;
			ret = recvwait(bss, clientfd, buffer, 4);
			CHECK_ERROR(ret < 0);

			ptr = ((void **)buffer)[0];

			value = (void*)kern_read(ptr);

			*(void **)buffer = value;
			sendwait(bss, clientfd, buffer, 4);
			break;
		}
		case 0x41: { /* cmd_upload */
			unsigned char *ptr, *end, *dst;
			ret = recvwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);
			ptr = ((unsigned char **)buffer)[0];
			end = ((unsigned char **)buffer)[1];

			compatibilityModeTWW = 0;

			while (ptr != end) {
				int len;

				len = end - ptr;
				if (len > 0x400)
					len = 0x400;
				if ((int)ptr >= 0x10000000 && (int)ptr <= 0x50000000) {
					dst = ptr;
				}
				else {
					dst = buffer;
				}
				ret = recvwait(bss, clientfd, dst, len);
				CHECK_ERROR(ret < 0);
				if (dst == buffer) {
					memcpy(ptr, buffer, len);
				}

				ptr += len;
			}

			sendbyte(bss, clientfd, 0xaa); /* GCACK */
			break;
		}
		case 0x50:
		{ /* cmd_status */
			ret = sendbyte(bss, clientfd, 1); /* running */
			CHECK_ERROR(ret < 0);

			compatibilityModeTWW = 0;
			break;
		}
		case 0x70: { /* cmd_rpc */
			long long(*fun)(int, int, int, int, int, int, int, int);
			int r3, r4, r5, r6, r7, r8, r9, r10;
			long long result;

			ret = recvwait(bss, clientfd, buffer, 4 + 8 * 4);
			CHECK_ERROR(ret < 0);

			fun = ((void **)buffer)[0];
			r3 = ((int *)buffer)[1];
			r4 = ((int *)buffer)[2];
			r5 = ((int *)buffer)[3];
			r6 = ((int *)buffer)[4];
			r7 = ((int *)buffer)[5];
			r8 = ((int *)buffer)[6];
			r9 = ((int *)buffer)[7];
			r10 = ((int *)buffer)[8];

			result = fun(r3, r4, r5, r6, r7, r8, r9, r10);

			((long long *)buffer)[0] = result;
			ret = sendwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);
			break;
		}
		case 0x71: { /* cmd_getsymbol */
			int size = recvbyte(bss, clientfd);
			CHECK_ERROR(size < 0);
			ret = recvwait(bss, clientfd, buffer, size);
			CHECK_ERROR(ret < 0);

			/* Identify the RPL name and symbol name */
			char *rplname = (char*)&((int*)buffer)[2];
			char *symname = (char*)(&buffer[0] + ((int*)buffer)[1]);

			/* Get the symbol and store it in the buffer */
			unsigned int module_handle, function_address;
			OSDynLoad_Acquire(rplname, &module_handle);

			char data = recvbyte(bss, clientfd);
			OSDynLoad_FindExport(module_handle, data, symname, &function_address);

			((int*)buffer)[0] = (int)function_address;
			ret = sendwait(bss, clientfd, buffer, 4);
			CHECK_ERROR(ret < 0);
			break;
		}
		case 0x72: { /* cmd_search32 */
			ret = recvwait(bss, clientfd, buffer, 12);
			CHECK_ERROR(ret < 0);
			int addr = ((int *)buffer)[0];
			int val = ((int  *)buffer)[1];
			int size = ((int *)buffer)[2];
			int i;
			int resaddr = 0;
			for (i = addr; i < (addr + size); i += 4)
			{
				if (*(int*)i == val)
				{
					resaddr = i;
					break;
				}
			}
			((int *)buffer)[0] = resaddr;
			ret = sendwait(bss, clientfd, buffer, 4);
			CHECK_ERROR(ret < 0);
			break;
		}
		case 0x80: { /* cmd_rpc_big */
			long long(*fun)(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
			int r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18;
			long long result;

			ret = recvwait(bss, clientfd, buffer, 4 + 16 * 4);
			CHECK_ERROR(ret < 0);

			fun = ((void **)buffer)[0];
			r3 = ((int *)buffer)[1];
			r4 = ((int *)buffer)[2];
			r5 = ((int *)buffer)[3];
			r6 = ((int *)buffer)[4];
			r7 = ((int *)buffer)[5];
			r8 = ((int *)buffer)[6];
			r9 = ((int *)buffer)[7];
			r10 = ((int *)buffer)[8];
			r11 = ((int *)buffer)[9];
			r12 = ((int *)buffer)[10];
			r13 = ((int *)buffer)[11];
			r14 = ((int *)buffer)[12];
			r15 = ((int *)buffer)[13];
			r16 = ((int *)buffer)[14];
			r17 = ((int *)buffer)[15];
			r18 = ((int *)buffer)[16];

			result = fun(r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18);

			((long long *)buffer)[0] = result;
			ret = sendwait(bss, clientfd, buffer, 8);
			CHECK_ERROR(ret < 0);
			break;
		}
		case 0x99: { /* cmd_version */
			ret = sendbyte(bss, clientfd, 0x82); /* WiiU */
			CHECK_ERROR(ret < 0);

			compatibilityModeTWW = 0;
			break;
		}
		case 0x9A: { /* cmd_os_version */
			((int *)buffer)[0] = (int)OS_FIRMWARE;
			ret = sendwait(bss, clientfd, buffer, 4);
			CHECK_ERROR(ret < 0);

			compatibilityModeTWW = 0;
			break;
		}
		case 0x9B: { /* cmd_tww_cancel_knockback */

			if (startActionOnNextFrame == 0)
			{
				const unsigned char *LinkXPtr = (const unsigned char *)0x1096EF48;
				const unsigned char *LinkYPtr = (const unsigned char *)0x1096EF4C;
				const unsigned char *LinkZPtr = (const unsigned char *)0x1096EF50;

				const unsigned char *AnglePtr = (const unsigned char *)0x1096EF12;

				int *LinkX, *LinkY, *LinkZ;
				short *Angle;

				LinkX = ((int *)LinkXPtr);
				*LinkX = ((int)0x41000000);
				DCFlushRange(LinkX, 4);

				LinkY = ((int *)LinkYPtr);
				*LinkY = ((int)0xC51F8A43);
				DCFlushRange(LinkY, 4);

				LinkZ = ((int *)LinkZPtr);
				*LinkZ = ((int)0xC64063DD);
				DCFlushRange(LinkZ, 4);

				Angle = ((short *)AnglePtr);
				*Angle = ((short)32717);
				DCFlushRange(Angle, 2);

				//Grab current framecount
				const unsigned char *frameCountPtr = (const unsigned char *)0x10701758;

				memcpy(&storedLastFrame, frameCountPtr, 4);

				actionID = 1;

				zeldaID = 0;
				startActionOnNextFrame = 1;
			}

			break;
		}
		case 0x9C: { /* cmd_tww_restore_collisions */

			if (startActionOnNextFrame == 0)
			{
				//Grab current framecount
				const unsigned char *frameCountPtr = (const unsigned char *)0x10701758;

				memcpy(&storedLastFrame, frameCountPtr, 4);

				actionID = 2;

				zeldaID = 0;
				startActionOnNextFrame = 1;
			}

			break;
		}
		case 0x9D: { /* cmd_tp_load_title */

			if (startActionOnNextFrame == 0)
			{
				//Grab current framecount
				const unsigned char *frameCountPtr_TP = (const unsigned char *)0x1068560C;

				memcpy(&storedLastFrame, frameCountPtr_TP, 4);

				actionID = 3;

				zeldaID = 1;
				startActionOnNextFrame = 1;
			}

			break;
		}
		case 0x9E: { /* cmd_tp_load_filemenu */

			if (startActionOnNextFrame == 0)
			{
				//Grab current framecount
				const unsigned char *frameCountPtr_TP = (const unsigned char *)0x1068560C;

				memcpy(&storedLastFrame, frameCountPtr_TP, 4);

				actionID = 4;

				zeldaID = 1;
				startActionOnNextFrame = 1;
			}

			break;
		}
		case 0xcc: { /* GCFAIL */
			break;
		}
		default:
			log_printf("Ret is N/A");
			CHECK_ERROR(ret == 0); //Connection was terminated normally
			break;
		}
	next_cmd:
		continue;
	}
	return -1;
error:
	log_printf("An error occured in the Hacker thread!");
	bss->error = ret;
	return -1;
}

//Input Display Thread
static int start_inputViewer(int argc, void *argv)
{
	int sockfd = -1, clientfd = -1, ret = 0, len;
	struct sockaddr_in addr;
	struct pygecko_bss_t *bss = argv;

	while (1)
	{
		addr.sin_family = AF_INET;
		addr.sin_port = 7335; //Input Display uses Port 7335
		addr.sin_addr.s_addr = 0;

		sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  //Open socket
		CHECK_ERROR(sockfd == -1);

		ret = bind(sockfd, (void *)&addr, 16);
		CHECK_ERROR(ret < 0);

		ret = listen(sockfd, 20);
		CHECK_ERROR(ret < 0);

		while (1)
		{
			len = 16;
			clientfd = accept(sockfd, (void *)&addr, &len);
			CHECK_ERROR(clientfd == -1);

			ret = run_inputViewer(bss, clientfd); //This function returns once the client disconnects or an error occurs
			CHECK_ERROR(ret < 0);

			socketclose(clientfd);
			clientfd = -1;
		}

		socketclose(sockfd);
		sockfd = -1;
	error:
		if (clientfd != -1)
			socketclose(clientfd);
		if (sockfd != -1)
			socketclose(sockfd);
		bss->error = ret;

	}
	return 0;
}

static int start_logger(int argc, void *argv) {
	int sockfd = -1, clientfd = -1, ret = 0, len;
	struct sockaddr_in addr;
	struct pygecko_bss_t *bss = argv;

	while (1) {
		addr.sin_family = AF_INET;
    	addr.sin_port = 7332; //Logger uses 7332
    	addr.sin_addr.s_addr = 0;
    	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  //open a file handle to socket
    	CHECK_ERROR(sockfd == -1);
    	ret = bind(sockfd, (void *)&addr, 16);
    	CHECK_ERROR(ret < 0);
    	ret = listen(sockfd, 20);
    	CHECK_ERROR(ret < 0);
     
    	while(1) {
    		len = 16;
    		clientfd = accept(sockfd, (void *)&addr, &len);
     		CHECK_ERROR(clientfd == -1);
     		ret = rungecko_logger(bss, clientfd);
     		CHECK_ERROR(ret < 0);
     		socketclose(clientfd);
     		clientfd = -1;
     		}
     		
    	socketclose(sockfd);
    	sockfd = -1;
error:
    	if (clientfd != -1)
       			socketclose(clientfd);
    	if (sockfd != -1)
       			socketclose(sockfd);
     	bss->error = ret;
   
	}
	return 0;
}

static int start_hacker(int argc, void *argv) {
	int sockfd = -1, clientfd = -1, ret = 0, len;
	struct sockaddr_in addr;
	struct pygecko_bss_t *bss = argv;

	log_printf("Hacker Thread Started!");

	while (1) {
		addr.sin_family = AF_INET;
		addr.sin_port = 7331; //Hacker uses 7331 to preserve backwards compatibility
		addr.sin_addr.s_addr = 0;
		sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  //open a file handle to socket
		CHECK_ERROR(sockfd == -1);
		ret = bind(sockfd, (void *)&addr, 16);
		CHECK_ERROR(ret < 0);
		ret = listen(sockfd, 20);
		CHECK_ERROR(ret < 0);

		while (1) 
		{
			log_printf("Ready for a connection to the Hacker...");

			len = 16;
			clientfd = accept(sockfd, (void *)&addr, &len);
			CHECK_ERROR(clientfd == -1);

			log_printf("An App connected to the Hacker!");

			ret = rungecko_hacker(bss, clientfd);
			CHECK_ERROR(ret < 0);
			socketclose(clientfd);
			clientfd = -1;
		}

		socketclose(sockfd);
		sockfd = -1;
	error:	
		if (clientfd != -1)
			socketclose(clientfd);
		if (sockfd != -1)
			socketclose(sockfd);
		bss->error = ret;

	}
	return 0;
}

static int CCThread(int argc, void *argv) 
{
	//Need to wait a bit so the game has fully started, otherwise it can be unstable and crash the console when trying to connect to a socket
	//usleep(13000000);

	usleep(7000000);

	InitOSFunctionPointers(); //Cafe OS functions e.g. OSGetTitleID
	InitSocketFunctionPointers(); //Sockets
	InitGX2FunctionPointers(); //Graphics e.g. GX2WaitForVsync
	InitSysFunctionPointers(); //for SYSLaunchMenu
	InitFSFunctionPointers(); //for Saviine
	InitPadScoreFunctionPointers();
	//if (g_isTPHD != -1) InitAXFunctionPointers(); //for audio beeps (only in TP/TWW HD ; can't register audio callback in most games or it breaks game audio!)
	SetupOSExceptions(); //re-direct Exceptions to our function

	GX2WaitForVsync();

	//Init twice is needed so logging works properly
	log_init(HOST_IP);
	log_deinit();
	log_init(HOST_IP);

	PatchGameHooks(); //Patch game functions for hacks

	log_printf("Game has launched, starting Threads...\n");


	//Logger Thread
	struct pygecko_bss_t *bss;
	
	bss = memalign(0x40, sizeof(struct pygecko_bss_t));
	if (bss == 0)
		return 0;
	memset(bss, 0, sizeof(struct pygecko_bss_t));
	
	if (OSCreateThread(&bss->thread, start_logger, 1, bss, (u32)bss->stack + sizeof(bss->stack), sizeof(bss->stack), 2, OS_THREAD_ATTRIB_AFFINITY_CPU0 | OS_THREAD_ATTRIB_DETACHED) == 1)
	{
		//OSSetThreadName(&bss->thread, "DB_DataViewer");
		OSResumeThread(&bss->thread);
	}
	else
	{
		log_printf("Starting Data Viewer thread failed!\n");
		free(bss);
	}

	//Hacker Thread
	struct pygecko_bss_t *bss_2;

	bss_2 = memalign(0x40, sizeof(struct pygecko_bss_t));
	if (bss_2 == 0)
		return 0;
	memset(bss_2, 0, sizeof(struct pygecko_bss_t));

	if (OSCreateThread(&bss_2->thread, start_hacker, 1, bss_2, (u32)bss_2->stack + sizeof(bss_2->stack), sizeof(bss_2->stack), 2, OS_THREAD_ATTRIB_AFFINITY_CPU0 | OS_THREAD_ATTRIB_DETACHED) == 1)
	{
		//OSSetThreadName(&bss_2->thread, "DB_Hacker");
		OSResumeThread(&bss_2->thread);
	}
	else
	{
		log_printf("Starting Hacking thread failed!\n");
		free(bss_2);
	}

	//Saviine Thread
	struct pygecko_bss_t *bss_3;

	bss_3 = memalign(0x40, sizeof(struct pygecko_bss_t));
	if (bss_3 == 0)
		return 0;
	memset(bss_3, 0, sizeof(struct pygecko_bss_t));

	if (OSCreateThread(&bss_3->thread, start_saviine, 1, bss_3, (u32)bss_3->stack + sizeof(bss_3->stack), sizeof(bss_3->stack), 2, OS_THREAD_ATTRIB_AFFINITY_CPU0 | OS_THREAD_ATTRIB_DETACHED) == 1)
	{
		//OSSetThreadName(&bss_3->thread, "DB_Saviine");
		OSResumeThread(&bss_3->thread);
	}
	else
	{
		log_printf("Starting Saviine thread failed!\n");
		free(bss_3);
	}

	//Input Display Thread
	struct pygecko_bss_t *bss_4;

	bss_4 = memalign(0x40, sizeof(struct pygecko_bss_t));
	if (bss_4 == 0)
		return 0;
	memset(bss_4, 0, sizeof(struct pygecko_bss_t));

	if (OSCreateThread(&bss_4->thread, start_inputViewer, 1, bss_4, (u32)bss_4->stack + sizeof(bss_4->stack), sizeof(bss_4->stack), 2, OS_THREAD_ATTRIB_AFFINITY_CPU0 | OS_THREAD_ATTRIB_DETACHED) == 1)
	{
		//OSSetThreadName(&bss_4->thread, "DB_InputDisplay");
		OSResumeThread(&bss_4->thread);
	}
	else
	{
		log_printf("Starting Input Display thread failed!\n");
		free(bss_4);
	}

	//Beta Quest Stuff (Cemu + 631E00 = Real Address)
	const unsigned char *GanonAmiiboPtr = (const unsigned char *)0x1066619F;
	char *GanonAmiibo;

	const unsigned char *MidnaZRemovedPtr = (const unsigned char *)0x10647D99;
	char *MidnaZRemoved;

	const unsigned char *FortuneTellerReturnDialogPtr = (const unsigned char *)0x10648DC0;
	char *FortuneTellerReturnDialog;

	const unsigned char *FortuneTellerEponaLockPtr = (const unsigned char *)0x10648DD3;
	char *FortuneTellerEponaLock;


	u8* LinkPointerPtr = (u8*)0x10681E40;
	u32 LinkPointer = 0;
	float LinkXPos = 0.0f, LinkYPos = 0.0f, LinkZPos = 0.0f;

	const unsigned char *IsVoidFlagPtr = (const unsigned char *)0x1064CDFE;
	u8 *IsVoidFlag;

	const unsigned char *AnimationIDPtr = (const unsigned char *)0x10648DBB;
	u8 *AnimationID;

	const unsigned char *NextEventIDPtr = (const unsigned char *)0x10648DB0;
	u8 *NextEventID;


	const unsigned char *NextStagePtr = (const unsigned char *)0x1064CDF6;
	char *NextStage;

	const unsigned char *NextRoomPtr = (const unsigned char *)0x1064CE00;
	u8 *NextRoom;

	const unsigned char *NextSpawnPtr = (const unsigned char *)0x1064CDFF;
	u8 *NextSpawn;

	const unsigned char *NextSpawnModifierPtr = (const unsigned char *)0x10648DA1;
	u8 *NextSpawnModifier;

	const unsigned char *NextStatePtr = (const unsigned char *)0x1064CE01;
	u8 *NextState;

	const unsigned char *VoidXPtr = (const unsigned char *)0x10648DA4;
	float *VoidX;

	const unsigned char *VoidYPtr = (const unsigned char *)0x10648DA8;
	float *VoidY;

	const unsigned char *VoidZPtr = (const unsigned char *)0x10648DAC;
	float *VoidZ;

	const unsigned char *VoidAnglePtr = (const unsigned char *)0x10648DA2;
	u16 *VoidAngle;


	const unsigned char *RespawnRoomIDPtr = (const unsigned char *)0x10648D9C;
	u8 *RespawnRoomID;

	const unsigned char *TriggerLoadingPtr = (const unsigned char *)0x1064CE04;
	u8 *TriggerLoading;

	const unsigned char *IsLoadingPtr_TP = (const unsigned char *)0x10680E94;
	unsigned char IsLoading_TP[4];

	const unsigned char *CurrentStagePtr = (const unsigned char *)0x1064CDE8;
	const char *CurrentStage;

	const unsigned char *CurrentRoomPtr = (const unsigned char *)0x106813D4;
	char *CurrentRoom;

	IsVoidFlag = ((u8 *)IsVoidFlagPtr);
	AnimationID = ((u8 *)AnimationIDPtr);
	NextEventID = ((u8 *)NextEventIDPtr);

	NextStage = ((char *)NextStagePtr);
	NextRoom = ((u8 *)NextRoomPtr);
	NextSpawn = ((u8 *)NextSpawnPtr);
	NextSpawnModifier = ((u8 *)NextSpawnModifierPtr);
	NextState = ((u8 *)NextStatePtr);

	VoidX = ((float *)VoidXPtr);
	VoidY = ((float *)VoidYPtr);
	VoidZ = ((float *)VoidZPtr);
	VoidAngle = ((u16 *)VoidAnglePtr);
	RespawnRoomID = ((u8 *)RespawnRoomIDPtr);

	TriggerLoading = ((u8 *)TriggerLoadingPtr);

	CurrentStage = ((const char *)CurrentStagePtr);
	CurrentRoom = ((char *)CurrentRoomPtr);


	//Set new Save Timers to speed up saving
	const unsigned char *SaveTimerPtr = (const unsigned char *)0x10669AEE;
	const unsigned char *SaveTimerPtr2 = (const unsigned char *)0x10669AEF;
	const unsigned char *SaveTimerPtr3 = (const unsigned char *)0x10669AF0;
	const unsigned char *SaveTimerPtr4 = (const unsigned char *)0x10669AF2;
	char *SaveTimer;

	SaveTimer = ((char *)SaveTimerPtr);
	*SaveTimer = 0;

	DCFlushRange(SaveTimer, 1);

	SaveTimer = ((char *)SaveTimerPtr2);
	*SaveTimer = 0;

	DCFlushRange(SaveTimer, 1);

	SaveTimer = ((char *)SaveTimerPtr3);
	*SaveTimer = 0;

	DCFlushRange(SaveTimer, 1);

	SaveTimer = ((char *)SaveTimerPtr4);
	*SaveTimer = 0;

	DCFlushRange(SaveTimer, 1);


	//Handles home button usage (after closing the menu set running flag)
	u8 frameCounter = 0;
	u8 forceRoomIDUntilReload = 0;
	u8 forcedRoomID = 0;
	u8 saveLoadInProgress = 0;

	char storedStage[8];
	u8 storedRoom = 0;
	u8 storedState = 0;
	float storedXPos = 0.0f;
	float storedYPos = 0.0f;
	float storedZPos = 0.0f;
	u16 storedAngle = 0;

	memset(&storedStage, 0, 8);

	while (1)
	{
		GX2WaitForVsync();

		if (g_gameRunning == 0)
		{
			frameCounter++;

			if (frameCounter == 30)
			{
				frameCounter = 0;
				g_gameRunning = 1;
				g_fileRedirectActive = 1;
			}
		}
		else
		{
			memcpy(&LinkPointer, LinkPointerPtr, 4);

			//Check and set Beta Quest flags every frame
			GanonAmiibo = ((char *)GanonAmiiboPtr);

			if (IsBitSet(*GanonAmiibo, 0) == 0)
			{
				*GanonAmiibo += 1;
				DCFlushRange(GanonAmiibo, 1);
			}

			MidnaZRemoved = ((char *)MidnaZRemovedPtr);

			if (IsBitSet(*MidnaZRemoved, 6) == 1)
			{
				*MidnaZRemoved -= 64;
				DCFlushRange(MidnaZRemoved, 1);
			}

			//Check for Fortune Teller Stuff
			if (!strcmp(CurrentStage, "R_SP160") && *CurrentRoom == 1)
			{
				FortuneTellerReturnDialog = ((char *)FortuneTellerReturnDialogPtr);

				if (IsBitSet(*FortuneTellerReturnDialog, 2) == 1)
				{
					*FortuneTellerReturnDialog -= 4;
					DCFlushRange(FortuneTellerReturnDialog, 1);
				}

				FortuneTellerEponaLock = ((char *)FortuneTellerEponaLockPtr);

				if (IsBitSet(*FortuneTellerEponaLock, 0) == 1)
				{
					*FortuneTellerEponaLock -= 1;
					DCFlushRange(FortuneTellerEponaLock, 1);
				}
			}

			if (g_checkIntroItemReload == 1 && LinkPointer > 0x10000000) //Player is currently in Links House, check for opened chests and reload in case
			{
				const unsigned char *BombBagOwnedPtr = (const unsigned char *)0x10647BF3;
				char *BombBagOwned;

				BombBagOwned = ((char *)BombBagOwnedPtr);

				const unsigned char *HorseCallOwnedPtr = (const unsigned char *)0x10647BF9;
				char *HorseCallOwned;

				HorseCallOwned = ((char *)HorseCallOwnedPtr);

				if (*BombBagOwned != 0xFF || *HorseCallOwned != 0xFF) //Player opened a chest and got the item, reload the map
				{
					g_checkIntroItemReload = 0;

					u8 *linkXPtr = (u8*)LinkPointer + 0x0;
					u8 *linkYPtr = (u8*)LinkPointer + 0x4;
					u8 *linkZPtr = (u8*)LinkPointer + 0x8;
					u8 *linkAnglePtr = (u8*)LinkPointer + 0x16;

					//Trigger refresh
					*IsVoidFlag = 0xFF;
					DCFlushRange(IsVoidFlag, 1);

					*AnimationID = 0;
					DCFlushRange(AnimationID, 1);

					*NextEventID = 201;
					DCFlushRange(NextEventID, 1);

					memcpy(VoidX, linkXPtr, 4);
					DCFlushRange(VoidX, 4);

					memcpy(VoidY, linkYPtr, 4);
					DCFlushRange(VoidY, 4);

					memcpy(VoidZ, linkZPtr, 4);
					DCFlushRange(VoidZ, 4);

					memcpy(VoidAngle, linkAnglePtr, 2);
					DCFlushRange(VoidAngle, 2);

					strcpy(NextStage, "R_SP01");
					DCFlushRange(NextStage, 8);

					*NextRoom = 4;
					DCFlushRange(NextRoom, 1);

					*NextSpawn = 0xFF;
					DCFlushRange(NextSpawn, 1);

					*NextState = 0xFF;
					DCFlushRange(NextState, 1);

					*RespawnRoomID = 4;
					DCFlushRange(RespawnRoomID, 1);

					*TriggerLoading = 1;
					DCFlushRange(TriggerLoading, 1);

					forceRoomIDUntilReload = 1;
					forcedRoomID = 4;

					if (*BombBagOwned != 0xFF) //Player got the Bomb Bag, replace with empty bag
					{
						const unsigned char *BombBagCountPtr = (const unsigned char *)0x10647C35;
						char *BombBagCount;

						BombBagCount = ((char *)BombBagCountPtr);

						*BombBagCount = 0;
						DCFlushRange(BombBagCount, 1);

						*BombBagOwned = 6;
						DCFlushRange(BombBagOwned, 1);			
					}
				}
			}

			if (g_doGorgeFallback == 1) //Player got Map Warping without having a Faron Woods Warp. Reload to prevent a softlock!
			{
				g_doGorgeFallback = 0;

				//Trigger refresh
				*IsVoidFlag = 0xFF;
				DCFlushRange(IsVoidFlag, 1);

				*AnimationID = 0;
				DCFlushRange(AnimationID, 1);

				*NextEventID = 201;
				DCFlushRange(NextEventID, 1);

				*VoidX = -13160.0f;
				DCFlushRange(VoidX, 4);

				*VoidY = -7030.0f;
				DCFlushRange(VoidY, 4);

				*VoidZ = 61900.0f;
				DCFlushRange(VoidZ, 4);

				*VoidAngle = 26000;
				DCFlushRange(VoidAngle, 2);

				strcpy(NextStage, "F_SP121");
				DCFlushRange(NextStage, 8);

				*NextRoom = 3;
				DCFlushRange(NextRoom, 1);

				*NextSpawn = 0xFF;
				DCFlushRange(NextSpawn, 1);

				*NextState = 0xFF;
				DCFlushRange(NextState, 1);

				*RespawnRoomID = 3;
				DCFlushRange(RespawnRoomID, 1);

				*TriggerLoading = 1;
				DCFlushRange(TriggerLoading, 1);

				forceRoomIDUntilReload = 1;
				forcedRoomID = 3;
			}

			//Check for parallel universe glitch and reload the area in case
			if (LinkPointer > 0x10000000)
			{
				memcpy(IsLoading_TP, IsLoadingPtr_TP, 4);

				if (IsLoading_TP[3] == 0 && *TriggerLoading == 0)
				{
					u8 *linkXPtr = (u8*)LinkPointer + 0x0;
					u8 *linkYPtr = (u8*)LinkPointer + 0x4;
					u8 *linkZPtr = (u8*)LinkPointer + 0x8;

					memcpy(&LinkXPos, linkXPtr, 4);
					memcpy(&LinkYPos, linkYPtr, 4);
					memcpy(&LinkZPos, linkZPtr, 4);

					if (forceRoomIDUntilReload == 1) //To prevent corruption
					{
						forceRoomIDUntilReload = 0;

						*RespawnRoomID = forcedRoomID;
						DCFlushRange(RespawnRoomID, 1);
					}

					if (saveLoadInProgress == 1)
					{
						//Trigger Warp to last destination
						*IsVoidFlag = 0xFF;
						DCFlushRange(IsVoidFlag, 1);

						*AnimationID = 0;
						DCFlushRange(AnimationID, 1);

						*NextEventID = 201;
						DCFlushRange(NextEventID, 1);

						*VoidX = storedXPos;
						DCFlushRange(VoidX, 4);

						*VoidY = storedYPos;
						DCFlushRange(VoidY, 4);

						*VoidZ = storedZPos;
						DCFlushRange(VoidZ, 4);

						*VoidAngle = storedAngle;
						DCFlushRange(VoidAngle, 2);

						memcpy(NextStage, &storedStage, 8);
						DCFlushRange(NextStage, 8);

						*NextRoom = storedRoom;
						DCFlushRange(NextRoom, 1);

						*NextSpawn = 0xFF;
						DCFlushRange(NextSpawn, 1);

						*NextState = storedState;
						DCFlushRange(NextState, 1);

						*RespawnRoomID = storedRoom;
						DCFlushRange(RespawnRoomID, 1);

						*TriggerLoading = 1;
						DCFlushRange(TriggerLoading, 1);

						forceRoomIDUntilReload = 1;
						forcedRoomID = storedRoom;

						saveLoadInProgress = 0;
					}
					else if (g_forceReload == 1)
					{
						//Trigger Reset to allow player to become Wolf again after Ooccoo
						*IsVoidFlag = 0xFF;
						DCFlushRange(IsVoidFlag, 1);

						*AnimationID = 0;
						DCFlushRange(AnimationID, 1);

						*NextEventID = 201;
						DCFlushRange(NextEventID, 1);

						*NextSpawn = 0xFF;
						DCFlushRange(NextSpawn, 1);

						*TriggerLoading = 1;
						DCFlushRange(TriggerLoading, 1);

						forceRoomIDUntilReload = 1;
						forcedRoomID = *RespawnRoomID;

						g_forceReload = 0;
					}
					else if (isnan(LinkXPos) && isnan(LinkYPos) && isnan(LinkZPos))
					{
						//Save Load KB1 CS first

						//Store Data
						memcpy(&storedStage, NextStage, 8);
						storedRoom = *NextRoom;
						storedState = *NextState;
						storedXPos = *VoidX;
						storedYPos = *VoidY;
						storedZPos = *VoidZ;
						storedAngle = *VoidAngle;

						//Trigger Save Warp
						*IsVoidFlag = 0x00;
						DCFlushRange(IsVoidFlag, 1);

						*AnimationID = 0;
						DCFlushRange(AnimationID, 1);

						*NextEventID = 0;
						DCFlushRange(NextEventID, 1);

						strcpy(NextStage, "F_SP102"); 
						DCFlushRange(NextStage, 8);

						*NextRoom = 0;
						DCFlushRange(NextRoom, 1);

						*NextSpawn = 0x00;
						DCFlushRange(NextSpawn, 1);

						*NextState = 0xFF;
						DCFlushRange(NextState, 1);

						*RespawnRoomID = 0;
						DCFlushRange(RespawnRoomID, 1);

						*TriggerLoading = 1;
						DCFlushRange(TriggerLoading, 1);

						saveLoadInProgress = 1;
					}
				}
			}

			//Spawn lead to a door and MS wasn't obtained, force human form till next load
			if (g_forceHuman == 1)
			{
				if (g_isLoading == 0)
				{		
					memcpy(IsLoading_TP, IsLoadingPtr_TP, 4);

					if (IsLoading_TP[3] == 0)
					{
						const unsigned char *MSOwnedPtr = (const unsigned char *)0x10647D45;
						char *MSOwned;

						MSOwned = ((char *)MSOwnedPtr);

						g_forceHuman = 0;

						if (IsBitSet(*MSOwned, 2) == 1) //MS flag was set, unset again at end of loading
						{
							*MSOwned = *MSOwned - 4;
							DCFlushRange(MSOwned, 1);
						}
					}
				}
			}
		}
	}

	return 0;
}

//Gets called when a "game" boots. Creates a helper thread so execution can return to the booting game as quickly as possible
void start_pygecko(void)
{
	unsigned int stack = (unsigned int)memalign(0x40, 0x1000); //Allocates 4096 bytes as own RAM for the new thread and aligns them in 64 byte blocks, returns pointer to the memory

	OSThread *thread = (OSThread *)stack;

	//OSCreateThread Reference: http://bombch.us/CSXL ; Attribute Reference: http://bombch.us/CSXP	
	//Note: Core/CPU0 is reserved for the system and not used by games, CPU1 and 2 are for game tasks

	if (OSCreateThread(thread, CCThread, 1, NULL, stack + sizeof(stack), sizeof(stack), 2, OS_THREAD_ATTRIB_AFFINITY_CPU0 | OS_THREAD_ATTRIB_DETACHED) == 1) //Run on System Core
	{
		OSResumeThread(thread); //Thread sleeps by default, so make sure we resume it
	}
	else
	{
		free(thread); //Clear thread memory if something goes wrong
	}
}