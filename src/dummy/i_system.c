#include "../doomdef.h"
#include "../i_system.h"

byte graphics_started = 0;

byte keyboard_started = 0;

ULONG I_GetFreeMem(ULONG *total)
{
	*total = 0;
	return 0;
}

tic_t I_GetTime(void)
{
	return 0;
}

void I_Sleep(void){}

void I_GetEvent(void){}

void I_OsPolling(void){}

ticcmd_t *I_BaseTiccmd(void)
{
	return NULL;
}

ticcmd_t *I_BaseTiccmd2(void)
{
	return NULL;
}

void I_Quit(void)
{
	exit(0);
}

void I_Error(const char *error, ...)
{
	(void)error;
	exit(-1);
}

byte *I_AllocLow(int length)
{
	(void)length;
	return NULL;
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

int I_NumJoys(void)
{
	return 0;
}

const char *I_GetJoyName(int joyindex)
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
	(void)error;
}

void I_StartupMouse(void){}

void I_StartupMouse2(void){}

void I_StartupKeyboard(void){}

int I_GetKey(void)
{
	return 0;
}

void I_StartupTimer(void){}

void I_AddExitFunc(void (*func)())
{
	(void)func;
}

void I_RemoveExitFunc(void (*func)())
{
	(void)func;
}

int I_StartupSystem(void)
{
	return -1;
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

int I_mkdir(const char *dirname, int unixright)
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

int I_PutEnv(char *variable)
{
	(void)variable;
	return -1;
}

void I_RegisterSysCommands(void) {}

#include "../sdl/dosstr.c"

