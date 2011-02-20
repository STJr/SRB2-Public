#include <nds.h>

#include "../doomdef.h"
#include "../i_system.h"

UINT8 graphics_started = 0;

UINT8 keyboard_started = 0;

static volatile tic_t ticcount;


UINT32 I_GetFreeMem(UINT32 *total)
{
	*total = 0;
	return 0;
}

tic_t I_GetTime(void)
{
	return ticcount;
}

void I_Sleep(void){}

void I_GetEvent(void){}

void I_OsPolling(void){}

ticcmd_t *I_BaseTiccmd(void)
{
	static ticcmd_t emptyticcmd;
	return &emptyticcmd;
}

ticcmd_t *I_BaseTiccmd2(void)
{
	static ticcmd_t emptyticcmd2;
	return &emptyticcmd2;
}

void I_Quit(void)
{
	exit(0);
}

void I_Error(const char *error, ...)
{
	va_list argptr;

        va_start(argptr, error);
        viprintf(error, argptr);
        va_end(argptr);

	for(;;);
}

void I_Tactile(FFType Type, const JoyFF_t *Effect)
{
	(void)Type;
	(void)Effect;
}

void I_Tactile2(FFType Type, const JoyFF_t *Effect)
{
	(void)Type;
	(void)Effect;
}

void I_JoyScale(void){}

void I_JoyScale2(void){}

void I_InitJoystick(void){}

void I_InitJoystick2(void){}

INT32 I_NumJoys(void)
{
	return 0;
}

const char *I_GetJoyName(INT32 joyindex)
{
	(void)joyindex;
	return NULL;
}

void I_SetupMumble(void)
{
}

void I_UpdateMumble(const MumblePos_t *MPos)
{
	(void)MPos;
}

void I_OutputMsg(const char *error, ...)
{
	va_list argptr;

	va_start(argptr, error);
	viprintf(error, argptr);
	va_end(argptr);
}

void I_StartupMouse(void){}

void I_StartupMouse2(void){}

void I_StartupKeyboard(void){}

INT32 I_GetKey(void)
{
	return 0;
}

static void NDS_VBlankHandler(void)
{
	ticcount++;
}

void I_StartupTimer(void)
{
	irqSet(IRQ_VBLANK, NDS_VBlankHandler);
}

void I_AddExitFunc(void (*func)())
{
	(void)func;
}

void I_RemoveExitFunc(void (*func)())
{
	(void)func;
}

// Adapted in part from the devkitPro examples.
INT32 I_StartupSystem(void)
{
	//set the mode for 2 text layers and two extended background layers
	videoSetMode(MODE_5_2D);

	//set the first two banks as background memory and the third as sub background memory
	//D is not used..if you need a bigger background then you will need to map
	//more vram banks consecutivly (VRAM A-D are all 0x20000 bytes in size)
	vramSetPrimaryBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_BG_0x06020000,
		VRAM_C_SUB_BG , VRAM_D_LCD);

	consoleDemoInit();

	// start FAT filesystem code, required for reading SD card
	if(!fatInitDefault())
		I_Error("Couldn't init FAT.");

	return 0;
}

void I_ShutdownSystem(void){}

void I_GetDiskFreeSpace(INT64* freespace)
{
	*freespace = 0;
}

char *I_GetUserName(void)
{
	return NULL;
}

INT32 I_mkdir(const char *dirname, INT32 unixright)
{
	(void)dirname;
	(void)unixright;
	return -1;
}

const CPUInfoFlags *I_CPUInfo(void)
{
	return NULL;
}

const char *I_LocateWad(void)
{
	return NULL;
}

void I_GetJoystickEvents(void){}

void I_GetJoystick2Events(void){}

void I_GetMouseEvents(void){}

char *I_GetEnv(const char *name)
{
	(void)name;
	return NULL;
}

INT32 I_PutEnv(char *variable)
{
	(void)variable;
	return -1;
}

void I_RegisterSysCommands(void) {}

#include "../sdl/dosstr.c"
