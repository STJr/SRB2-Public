// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//-----------------------------------------------------------------------------
/// \file
/// \brief console drawing, input

#ifdef __GNUC__
#include <unistd.h>
#ifdef _XBOX
#include <openxdk/debug.h>
#endif
#endif

#include "doomdef.h"
#include "console.h"
#include "g_game.h"
#include "g_input.h"
#include "hu_stuff.h"
#include "keys.h"
#include "r_defs.h"
#include "sounds.h"
#include "st_stuff.h"
#include "s_sound.h"
#include "v_video.h"
#include "i_video.h"
#include "z_zone.h"
#include "i_system.h"
#include "d_main.h"
#include "m_menu.h"

#ifdef _WINDOWS
#include "win32/win_main.h"
#endif

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

#define MAXHUDLINES 20

static boolean con_started = false; // console has been initialised
       boolean con_startup = false; // true at game startup, screen need refreshing
static boolean con_forcepic = true; // at startup toggle console translucency when first off
       boolean con_recalc;          // set true when screen size has changed

static tic_t con_tick; // console ticker for anim or blinking prompt cursor
                        // con_scrollup should use time (currenttime - lasttime)..

static boolean consoletoggle; // true when console key pushed, ticker will handle
static boolean consoleready;  // console prompt is ready

       INT32 con_destlines; // vid lines used by console at final position
static INT32 con_curlines;  // vid lines currently used by console

       INT32 con_clipviewtop; // clip value for planes & sprites, so that the
                            // part of the view covered by the console is not
                            // drawn when not needed, this must be -1 when
                            // console is off

static INT32 con_hudlines;        // number of console heads up message lines
static INT32 con_hudtime[MAXHUDLINES];      // remaining time of display for hud msg lines

       INT32 con_clearlines;      // top screen lines to refresh when view reduced
       boolean con_hudupdate;   // when messages scroll, we need a backgrnd refresh

// console text output
static char *con_line;          // console text output current line
static size_t con_cx;           // cursor position in current line
static size_t con_cy;           // cursor line number in con_buffer, is always
                                // increasing, and wrapped around in the text
                                // buffer using modulo.

static size_t con_totallines;      // lines of console text into the console buffer
static size_t con_width;           // columns of chars, depend on vid mode width

static size_t con_scrollup;        // how many rows of text to scroll up (pgup/pgdn)

// hold 32 last lines of input for history
#define CON_MAXPROMPTCHARS 256
#define CON_PROMPTCHAR '>'

static char inputlines[32][CON_MAXPROMPTCHARS]; // hold last 32 prompt lines

static INT32 inputline;    // current input line number
static INT32 inputhist;    // line number of history input line to restore
static size_t input_cx;  // position in current input line

static patch_t *con_backpic; // console background picture, loaded static

// protos.
static void CON_InputInit(void);
static void CON_RecalcSize(void);

static void CONS_hudlines_Change(void);
static void CONS_speed_Change(void);
static void CON_DrawBackpic(patch_t *pic, INT32 startx, INT32 destwidth);
//static void CON_DrawBackpic2(pic_t *pic, INT32 startx, INT32 destwidth);

//======================================================================
//                   CONSOLE VARS AND COMMANDS
//======================================================================
#ifdef macintosh
#define CON_BUFFERSIZE 4096 // my compiler can't handle local vars >32k
#else
#define CON_BUFFERSIZE 16384
#endif

static char con_buffer[CON_BUFFERSIZE];

// how many seconds the hud messages lasts on the screen
static consvar_t cons_msgtimeout = {"con_hudtime", "5", CV_SAVE, CV_Unsigned, NULL, 0, NULL, NULL, 0, 0, NULL};

// number of lines displayed on the HUD
static consvar_t cons_hudlines = {"con_hudlines", "5", CV_CALL|CV_SAVE, CV_Unsigned, CONS_hudlines_Change, 0, NULL, NULL, 0, 0, NULL};

// (con_speed needs a limit, apparently)
static CV_PossibleValue_t speed_cons_t[] = {{1, "MIN"}, {255, "MAX"}, {0, NULL}};

// number of lines console move per frame
static consvar_t cons_speed = {"con_speed", "8", CV_CALL|CV_SAVE, speed_cons_t, CONS_speed_Change, 0, NULL, NULL, 0, 0, NULL};

// percentage of screen height to use for console
static consvar_t cons_height = {"con_height", "50", CV_SAVE, CV_Unsigned, NULL, 0, NULL, NULL, 0, 0, NULL};

static CV_PossibleValue_t backpic_cons_t[] = {{0, "translucent"}, {1, "picture"}, {0, NULL}};
// whether to use console background picture, or translucent mode
static consvar_t cons_backpic = {"con_backpic", "translucent", CV_SAVE, backpic_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

static CV_PossibleValue_t backcolor_cons_t[] = {{0, "White"}, {1, "Orange"},
												{2, "Blue"}, {3, "Green"}, {4, "Gray"},
												{5, "Red"}, {0, NULL}};
consvar_t cons_backcolor = {"con_backcolor", "3", CV_SAVE, backcolor_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

static void CON_Print(char *msg);

//
//
static void CONS_hudlines_Change(void)
{
	INT32 i;

	// Clear the currently displayed lines
	for (i = 0; i < con_hudlines; i++)
		con_hudtime[i] = 0;

	if (cons_hudlines.value < 1)
		cons_hudlines.value = 1;
	else if (cons_hudlines.value > MAXHUDLINES)
		cons_hudlines.value = MAXHUDLINES;

	con_hudlines = cons_hudlines.value;

	CONS_Printf("Number of console HUD lines is now %d\n", con_hudlines);
}

// Check CONS_speed value (must be positive)
//
static void CONS_speed_Change(void)
{
	if (cons_speed.value < 1)
		CV_SetValue(&cons_speed, 1);
}

// Clear console text buffer
//
static void CONS_Clear_f(void)
{
	memset(con_buffer, 0, CON_BUFFERSIZE);

	con_cx = 0;
	con_cy = con_totallines-1;
	con_line = &con_buffer[con_cy*con_width];
	con_scrollup = 0;
}

// Choose english keymap
//
static void CONS_English_f(void)
{
	shiftxform = english_shiftxform;
	CONS_Printf("English keymap.\n");
}

static char *bindtable[NUMINPUTS];

static void CONS_Bind_f(void)
{
	size_t na;
	INT32 key;

	na = COM_Argc();

	if (na != 2 && na != 3)
	{
		CONS_Printf("bind <keyname> [<command>]\n");
		CONS_Printf("\2bind table :\n");
		na = 0;
		for (key = 0; key < NUMINPUTS; key++)
			if (bindtable[key])
			{
				CONS_Printf("%s : \"%s\"\n", G_KeynumToString(key), bindtable[key]);
				na = 1;
			}
		if (!na)
			CONS_Printf("Empty\n");
		return;
	}

	key = G_KeyStringtoNum(COM_Argv(1));
	if (!key)
	{
		CONS_Printf("Invalid key name\n");
		return;
	}

	Z_Free(bindtable[key]);
	bindtable[key] = NULL;

	if (na == 3)
		bindtable[key] = Z_StrDup(COM_Argv(2));
}

//======================================================================
//                          CONSOLE SETUP
//======================================================================

// Prepare a colormap for GREEN ONLY translucency over background
//
UINT8 *yellowmap;
UINT8 *purplemap;
UINT8 *lgreenmap;
UINT8 *bluemap;
UINT8 *graymap;
UINT8 *redmap;
UINT8 *orangemap;

// Console BG colors
UINT8 *cwhitemap;
UINT8 *corangemap;
UINT8 *cbluemap;
UINT8 *cgreenmap;
UINT8 *cgraymap;
UINT8 *credmap;

void CON_ReSetupBackColormap(UINT16 num)
{
	UINT16 i, j;
	UINT8 k;
	UINT8 *pal = W_CacheLumpName(R_GetPalname(num), PU_CACHE);

	// setup the green translucent background colormaps
	for (i = 0, k = 0; i < 768; i += 3, k++)
	{
		j = pal[i] + pal[i+1] + pal[i+2];
		cwhitemap[k] = (UINT8)(15 - (j>>6));
		corangemap[k] = (UINT8)(95 - (j>>6));
		cbluemap[k] = (UINT8)(239 - (j>>6));
		cgreenmap[k] = (UINT8)(175 - (j>>6));
		cgraymap[k] = (UINT8)(31 - (j>>6));
		credmap[k] = (UINT8)(143 - (j>>6));
	}
}

static void CON_SetupBackColormap(void)
{
	INT32 i, j, k;
	UINT8 *pal;

	cwhitemap   = (UINT8 *)Z_Malloc(256, PU_STATIC, NULL);
	corangemap  = (UINT8 *)Z_Malloc(256, PU_STATIC, NULL);
	cbluemap    = (UINT8 *)Z_Malloc(256, PU_STATIC, NULL);
	cgreenmap   = (UINT8 *)Z_Malloc(256, PU_STATIC, NULL);
	cgraymap    = (UINT8 *)Z_Malloc(256, PU_STATIC, NULL);
	credmap     = (UINT8 *)Z_Malloc(256, PU_STATIC, NULL);

	yellowmap = (UINT8 *)Z_Malloc(256, PU_STATIC, NULL);
	graymap   = (UINT8 *)Z_Malloc(256, PU_STATIC, NULL);
	purplemap = (UINT8 *)Z_Malloc(256, PU_STATIC, NULL);
	lgreenmap = (UINT8 *)Z_Malloc(256, PU_STATIC, NULL);
	bluemap   = (UINT8 *)Z_Malloc(256, PU_STATIC, NULL);
	redmap    = (UINT8 *)Z_Malloc(256, PU_STATIC, NULL);
	orangemap = (UINT8 *)Z_Malloc(256, PU_STATIC, NULL);

	pal = W_CacheLumpName("PLAYPAL", PU_CACHE);

	// setup the green translucent background colormaps
	for (i = 0, k = 0; i < 768; i += 3, k++)
	{
		j = pal[i] + pal[i+1] + pal[i+2];
		cwhitemap[k] = (UINT8)(15 - (j>>6));
		corangemap[k] = (UINT8)(95 - (j>>6));
		cbluemap[k] = (UINT8)(239 - (j>>6));
		cgreenmap[k] = (UINT8)(175 - (j>>6));
		cgraymap[k] = (UINT8)(31 - (j>>6));
		credmap[k] = (UINT8)(143 - (j>>6));
	}

	// setup the other colormaps, for console text

	// these don't need to be aligned, unless you convert the
	// V_DrawMappedPatch() into optimised asm.

	for (i = 0; i < 256; i++)
	{
		yellowmap[i] = (UINT8)i; // remap each color to itself...
		graymap[i] = (UINT8)i;
		purplemap[i] = (UINT8)i;
		lgreenmap[i] = (UINT8)i;
		bluemap[i] = (UINT8)i;
		redmap[i] = (UINT8)i;
		orangemap[i] = (UINT8)i;
	}

	yellowmap[3] = (UINT8)103;
	yellowmap[9] = (UINT8)115;
	purplemap[3] = (UINT8)195;
	purplemap[9] = (UINT8)198;
	lgreenmap[3] = (UINT8)162;
	lgreenmap[9] = (UINT8)170;
	bluemap[3]   = (UINT8)228;
	bluemap[9]   = (UINT8)238;
	graymap[3]   = (UINT8)10;
	graymap[9]   = (UINT8)15;
	redmap[3]    = (UINT8)124;
	redmap[9]    = (UINT8)127;
	orangemap[3] = (UINT8)85;
	orangemap[9] = (UINT8)90;
}

// Setup the console text buffer
//
void CON_Init(void)
{
	INT32 i;

	for (i = 0; i < NUMINPUTS; i++)
		bindtable[i] = NULL;

	// clear all lines
	memset(con_buffer, 0, CON_BUFFERSIZE);

	// make sure it is ready for the loading screen
	con_width = 0;
	CON_RecalcSize();

	CON_SetupBackColormap();

	//note: CON_Ticker should always execute at least once before D_Display()
	con_clipviewtop = -1; // -1 does not clip

	con_hudlines = atoi(cons_hudlines.defaultvalue);

	// setup console input filtering
	CON_InputInit();

	// load console background pic
	con_backpic = (patch_t *)W_CacheLumpName("CONSBACK",PU_STATIC);

	// register our commands
	//
	COM_AddCommand("cls", CONS_Clear_f);
	COM_AddCommand("english", CONS_English_f);
	// set console full screen for game startup MAKE SURE VID_Init() done !!!
	con_destlines = vid.height;
	con_curlines = vid.height;


	if (!dedicated)
	{
		con_started = true;
		con_startup = true; // need explicit screen refresh until we are in Doom loop
		consoletoggle = false;
		CV_RegisterVar(&cons_msgtimeout);
		CV_RegisterVar(&cons_hudlines);
		CV_RegisterVar(&cons_speed);
		CV_RegisterVar(&cons_height);
		CV_RegisterVar(&cons_backpic);
		CV_RegisterVar(&cons_backcolor);
		COM_AddCommand("bind", CONS_Bind_f);
	}
	else
	{
		con_started = true;
		con_startup = false; // need explicit screen refresh until we are in Doom loop
		consoletoggle = true;
	}
}
// Console input initialization
//
static void CON_InputInit(void)
{
	INT32 i;

	// prepare the first prompt line
	memset(inputlines, 0, sizeof (inputlines));
	for (i = 0; i < 32; i++)
		inputlines[i][0] = CON_PROMPTCHAR;
	inputline = 0;
	input_cx = 1;
}

//======================================================================
//                        CONSOLE EXECUTION
//======================================================================

// Called at screen size change to set the rows and line size of the
// console text buffer.
//
static void CON_RecalcSize(void)
{
	size_t conw, oldcon_width, oldnumlines, i, oldcon_cy;
	XBOXSTATIC char tmp_buffer[CON_BUFFERSIZE];
	XBOXSTATIC char string[CON_BUFFERSIZE]; // BP: it is a line but who know

	con_recalc = false;

	conw = (vid.width>>3) - 2;

	if (con_curlines == 200) // first init
	{
		con_curlines = vid.height;
		con_destlines = vid.height;
	}

	// check for change of video width
	if (conw == con_width)
		return; // didn't change

	oldcon_width = con_width;
	oldnumlines = con_totallines;
	oldcon_cy = con_cy;
	M_Memcpy(tmp_buffer, con_buffer, CON_BUFFERSIZE);

	if (conw < 1)
		con_width = (BASEVIDWIDTH>>3) - 2;
	else
		con_width = conw;

	con_width += 11; // Graue 06-19-2004 up to 11 control chars per line

	con_totallines = CON_BUFFERSIZE / con_width;
	memset(con_buffer, ' ', CON_BUFFERSIZE);

	con_cx = 0;
	con_cy = con_totallines-1;
	con_line = &con_buffer[con_cy*con_width];
	con_scrollup = 0;

	// re-arrange console text buffer to keep text
	if (oldcon_width) // not the first time
	{
		for (i = oldcon_cy + 1; i < oldcon_cy + oldnumlines; i++)
		{
			if (tmp_buffer[(i%oldnumlines)*oldcon_width])
			{
				M_Memcpy(string, &tmp_buffer[(i%oldnumlines)*oldcon_width], oldcon_width);
				conw = oldcon_width - 1;
				while (string[conw] == ' ' && conw)
					conw--;
				string[conw+1] = '\n';
				string[conw+2] = '\0';
				CON_Print(string);
			}
		}
	}
}

// Handles Console moves in/out of screen (per frame)
//
static void CON_MoveConsole(void)
{
	// up/down move to dest
	if (con_curlines < con_destlines)
	{
		con_curlines += (INT32)(cons_speed.value*vid.fdupy)/NEWTICRATERATIO;
		if (con_curlines > con_destlines)
			con_curlines = con_destlines;
	}
	else if (con_curlines > con_destlines)
	{
		con_curlines -= (INT32)(cons_speed.value*vid.fdupy)/NEWTICRATERATIO;
		if (con_curlines < con_destlines)
			con_curlines = con_destlines;
	}
}

// Clear time of console heads up messages
//
void CON_ClearHUD(void)
{
	INT32 i;

	for (i = 0; i < con_hudlines; i++)
		con_hudtime[i] = 0;
}

// Force console to move out immediately
// note: con_ticker will set consoleready false
void CON_ToggleOff(void)
{
	if (!con_destlines)
		return;

	con_destlines = 0;
	con_curlines = 0;
	CON_ClearHUD();
	con_forcepic = 0;
	con_clipviewtop = -1; // remove console clipping of view
}

// Console ticker: handles console move in/out, cursor blinking
//
void CON_Ticker(void)
{
	INT32 i;

	// cursor blinking
	con_tick++;
	con_tick &= 7;

	// console key was pushed
	if (consoletoggle)
	{
		consoletoggle = false;

		// toggle off console
		if (con_destlines > 0)
		{
			con_destlines = 0;
			CON_ClearHUD();
		}
		else
		{
			// toggle console in
			con_destlines = (cons_height.value*vid.height)/100;
			if (con_destlines < 20)
				con_destlines = 20;
			else if (con_destlines > vid.height)
				con_destlines = vid.height;

			con_destlines &= ~0x3; // multiple of text row height
		}
	}

	// console movement
	if (con_destlines != con_curlines)
		CON_MoveConsole();

	// clip the view, so that the part under the console is not drawn
	con_clipviewtop = -1;
	if (cons_backpic.value) // clip only when using an opaque background
	{
		if (con_curlines > 0)
			con_clipviewtop = con_curlines - viewwindowy - 1 - 10;
		// NOTE: BIG HACK::SUBTRACT 10, SO THAT WATER DON'T COPY LINES OF THE CONSOLE
		//       WINDOW!!! (draw some more lines behind the bottom of the console)
		if (con_clipviewtop < 0)
			con_clipviewtop = -1; // maybe not necessary, provided it's < 0
	}

	// check if console ready for prompt
	if (con_destlines >= 20)
		consoleready = true;
	else
		consoleready = false;

	// make overlay messages disappear after a while
	for (i = 0; i < con_hudlines; i++)
	{
		con_hudtime[i]--;
		if (con_hudtime[i] < 0)
			con_hudtime[i] = 0;
	}
}

// Handles console key input
//
boolean CON_Responder(event_t *ev)
{
	static boolean shiftdown;

	// sequential completions a la 4dos
	static char completion[80];
	static INT32 comskips, varskips;

	const char *cmd = "";
	INT32 key;

	if (chat_on)
		return false;

	// special keys state
	if ((ev->data1 == KEY_LSHIFT || ev->data1 == KEY_RSHIFT) && ev->type == ev_keyup)
	{
		shiftdown = false;
		return false;
	}

	// let go keyup events, don't eat them
	if (ev->type != ev_keydown && ev->type != ev_console)
		return false;

	key = ev->data1;

	// check for console toggle key
	if (ev->type != ev_console)
	{
		if (key == gamecontrol[gc_console][0] || key == gamecontrol[gc_console][1])
		{
			consoletoggle = true;

			if (timeattacking)
			{
				G_CheckDemoStatus();
				timeattacking = true;
				M_StartControlPanel();
				consoletoggle = false;
			}

			return true;
		}

		// check other keys only if console prompt is active
		if (!consoleready && key < NUMINPUTS) // metzgermeister: boundary check!!
		{
			if (bindtable[key] && !timeattacking)
			{
				COM_BufAddText(bindtable[key]);
				COM_BufAddText("\n");
				return true;
			}
			return false;
		}

		// escape key toggle off console
		if (key == KEY_ESCAPE)
		{
			consoletoggle = true;

			if (timeattacking)
				G_CheckDemoStatus();

			return true;
		}

	}

	// eat shift only if console active
	if (key == KEY_LSHIFT || key == KEY_RSHIFT)
	{
		shiftdown = true;
		return true;
	}

	// command completion forward (tab) and backward (shift-tab)
	if (key == KEY_TAB)
	{
		// sequential command completion forward and backward

		// remember typing for several completions (a-la-4dos)
		if (inputlines[inputline][input_cx-1] != ' ')
		{
			if (strlen(inputlines[inputline]+1) < 80)
				strcpy(completion, inputlines[inputline]+1);
			else
				completion[0] = 0;

			comskips = varskips = 0;
		}
		else
		{
			if (shiftdown)
			{
				if (comskips < 0)
				{
					if (--varskips < 0)
						comskips = -comskips - 2;
				}
				else if (comskips > 0)
					comskips--;
			}
			else
			{
				if (comskips < 0)
					varskips++;
				else
					comskips++;
			}
		}

		if (comskips >= 0)
		{
			cmd = COM_CompleteCommand(completion, comskips);
			if (!cmd)
				// dirty: make sure if comskips is zero, to have a neg value
				comskips = -comskips - 1;
		}
		if (comskips < 0)
			cmd = CV_CompleteVar(completion, varskips);

		if (cmd)
		{
			memset(inputlines[inputline]+1, 0, CON_MAXPROMPTCHARS-1);
			strcpy(inputlines[inputline]+1, cmd);
			input_cx = strlen(cmd) + 1;
			inputlines[inputline][input_cx] = ' ';
			input_cx++;
			inputlines[inputline][input_cx] = 0;
		}
		else
		{
			if (comskips > 0)
				comskips--;
			else if (varskips > 0)
				varskips--;
		}

		return true;
	}

	// move up (backward) in console textbuffer
	if (key == KEY_PGUP)
	{
		if (con_scrollup < (con_totallines-((con_curlines-16)>>3)))
			con_scrollup++;
		return true;
	}
	else if (key == KEY_PGDN)
	{
		if (con_scrollup > 0)
			con_scrollup--;
		return true;
	}

	if (key == KEY_HOME) // oldest text in buffer
	{
		con_scrollup = (con_totallines-((con_curlines-16)>>3));
		return true;
	}
	else if (key == KEY_END) // most recent text in buffer
	{
		con_scrollup = 0;
		return true;
	}

	// command enter
	if (key == KEY_ENTER)
	{
		if (input_cx < 2)
			return true;

		// push the command
		COM_BufAddText(inputlines[inputline]+1);
		COM_BufAddText("\n");

		CONS_Printf("%s\n", inputlines[inputline]);

		inputline = (inputline+1) & 31;
		inputhist = inputline;

		memset(inputlines[inputline], 0, CON_MAXPROMPTCHARS);
		inputlines[inputline][0] = CON_PROMPTCHAR;
		input_cx = 1;

		return true;
	}

	// backspace command prompt
	if (key == KEY_BACKSPACE)
	{
		if (input_cx > 1)
		{
			input_cx--;
			inputlines[inputline][input_cx] = 0;
		}
		return true;
	}

	// move back in input history
	if (key == KEY_UPARROW)
	{
		// copy one of the previous inputlines to the current
		do
		{
			inputhist = (inputhist - 1) & 31; // cycle back
		} while (inputhist != inputline && !inputlines[inputhist][1]);

		// stop at the last history input line, which is the
		// current line + 1 because we cycle through the 32 input lines
		if (inputhist == inputline)
			inputhist = (inputline + 1) & 31;

		M_Memcpy(inputlines[inputline], inputlines[inputhist], CON_MAXPROMPTCHARS);
		input_cx = strlen(inputlines[inputline]);

		return true;
	}

	// move forward in input history
	if (key == KEY_DOWNARROW)
	{
		if (inputhist == inputline)
			return true;
		do
		{
			inputhist = (inputhist + 1) & 31;
		} while (inputhist != inputline && !inputlines[inputhist][1]);

		memset(inputlines[inputline], 0, CON_MAXPROMPTCHARS);

		// back to currentline
		if (inputhist == inputline)
		{
			inputlines[inputline][0] = CON_PROMPTCHAR;
			input_cx = 1;
		}
		else
		{
			strcpy(inputlines[inputline], inputlines[inputhist]);
			input_cx = strlen(inputlines[inputline]);
		}
		return true;
	}

	// allow people to use keypad in console (good for typing IP addresses) - Calum
	if (key >= KEY_KEYPAD7 && key <= KEY_KPADDEL)
	{
		XBOXSTATIC char keypad_translation[] = {'7','8','9','-',
		                                        '4','5','6','+',
		                                        '1','2','3',
		                                        '0','.'};

		key = keypad_translation[key - KEY_KEYPAD7];
	}
	else if (key == KEY_KPADSLASH)
		key = '/';

	if (shiftdown)
		key = shiftxform[key];

	// enter a char into the command prompt
	if (key < 32 || key > 127)
		return false;

	// add key to cmd line here
	if (input_cx < CON_MAXPROMPTCHARS)
	{
		if (key >= 'A' && key <= 'Z' && !shiftdown) //this is only really necessary for dedicated servers
			key = key + 'a' - 'A';

		inputlines[inputline][input_cx] = (char)key;
		inputlines[inputline][input_cx + 1] = 0;
		input_cx++;
	}

	return true;
}

// Insert a new line in the console text buffer
//
static void CON_Linefeed(void)
{
	// set time for heads up messages
	con_hudtime[con_cy%con_hudlines] = cons_msgtimeout.value*TICRATE;

	con_cy++;
	con_cx = 0;

	con_line = &con_buffer[(con_cy%con_totallines)*con_width];
	memset(con_line, ' ', con_width);

	// make sure the view borders are refreshed if hud messages scroll
	con_hudupdate = true; // see HU_Erase()
}

// Outputs text into the console text buffer
static void CON_Print(char *msg)
{
	size_t l;
	INT32 controlchars = 0; // for color changing

	if (*msg == '\3') // chat text, makes ding sound
		S_StartSound(NULL, sfx_radio);
	else if (*msg == '\4') // chat action, dings and is in yellow
	{
		*msg = '\x82'; // yellow
		S_StartSound(NULL, sfx_radio);
	}

	if (!(*msg & 0x80))
	{
		con_line[con_cx++] = '\x80';
		controlchars = 1;
	}

	while (*msg)
	{
		// skip non-printable characters and white spaces
		while (*msg && *msg <= ' ')
		{
			if (*msg & 0x80)
			{
				con_line[con_cx++] = *(msg++);
				controlchars++;
				continue;
			}
			else if (*msg == '\r') // carriage return
			{
				con_cy--;
				CON_Linefeed();
				controlchars = 0;
			}
			else if (*msg == '\n') // linefeed
			{
				CON_Linefeed();
				controlchars = 0;
			}
			else if (*msg == ' ') // space
			{
				con_line[con_cx++] = ' ';
				if (con_cx - controlchars >= con_width-11)
				{
					CON_Linefeed();
					controlchars = 0;
				}
			}
			else if (*msg == '\t')
			{
				// adds tab spaces for nice layout in console

				do
				{
					con_line[con_cx++] = ' ';
				} while ((con_cx - controlchars) % 4 != 0);

				if (con_cx - controlchars >= con_width-11)
				{
					CON_Linefeed();
					controlchars = 0;
				}
			}
			msg++;
		}

		if (*msg == '\0')
			return;

		// printable character
		for (l = 0; l < (con_width-11) && msg[l] > ' '; l++)
			;

		// word wrap
		if ((con_cx - controlchars) + l > con_width-11)
		{
			CON_Linefeed();
			controlchars = 0;
		}

		// a word at a time
		for (; l > 0; l--)
			con_line[con_cx++] = *(msg++);
	}
}

void CON_LogMessage(const char *msg)
{
	XBOXSTATIC char txt[128], *t;
#if defined (LOGMESSAGES) && defined (_WINDOWS)
	const boolean ls = (logstream != INVALID_HANDLE_VALUE);
#endif
	const char *p = msg, *e = txt+sizeof (txt)-2;

	for (t = txt; *p != '\0'; p++)
	{
		if (*p == '\n' || *p >= ' ') // don't log or console print CON_Print's control characters
		{
#if defined (LOGMESSAGES) && defined (_WINDOWS)
			if (*p == '\n' && (p == msg || *(p-1) != '\r') && ls)
				*t++ = '\r';
#endif
			*t++ = *p;
		}

		if (t >= e)
		{
			*t = '\0'; //end of string
			I_OutputMsg("%s", txt); //print string
			t = txt; //reset t pointer
			memset(txt,'\0', sizeof (txt)); //reset txt
		}
	}
	*t = '\0'; //end of string
	I_OutputMsg("%s", txt);
}

// Console print! Wahooo! Lots o fun!
//

void CONS_Printf(const char *fmt, ...)
{
	va_list argptr;
	XBOXSTATIC char txt[8192];

	va_start(argptr, fmt);
	vsprintf(txt, fmt, argptr);
	va_end(argptr);

	// echo console prints to log file
#ifndef _arch_dreamcast
	DEBFILE(txt);
#endif

	if (!con_started)
	{
#if defined (_XBOX) && defined (__GNUC__)
		if (!keyboard_started) debugPrint(txt);
#endif
#ifdef PC_DOS
		CON_LogMessage(txt);
		return;
#endif
	}
	else
		// write message in con text buffer
		CON_Print(txt);

#ifndef PC_DOS
	CON_LogMessage(txt);
#endif

	// make sure new text is visible
	con_scrollup = 0;

	// if not in display loop, force screen update
	if (con_startup)
	{
#if (defined (_WINDOWS)) || (defined (__OS2__) && !defined (SDL))
		// show startup screen and message using only 'software' graphics
		// (rendermode may be hardware accelerated, but the video mode is not set yet)
		CON_DrawBackpic(con_backpic, 0, vid.width); // put console background
		I_LoadingScreen(txt);
#else
		// here we display the console background and console text
		// (no hardware accelerated support for these versions)
		CON_Drawer();
		I_FinishUpdate(); // page flip or blit buffer
#endif
	}
}

// Print an error message, and wait for ENTER key to continue.
// To make sure the user has seen the message
//
void CONS_Error(const char *msg)
{
#ifdef RPC_NO_WINDOWS_H
	if (!graphics_started)
	{
		MessageBoxA(vid.WndParent, msg, "SRB2 Warning", MB_OK);
		return;
	}
#endif
	CONS_Printf("\2%s", msg); // write error msg in different colour
	CONS_Printf("Press ENTER to continue\n");

	// dirty quick hack, but for the good cause
	while (I_GetKey() != KEY_ENTER)
		I_OsPolling();
}

//======================================================================
//                          CONSOLE DRAW
//======================================================================

// draw console prompt line
//
static void CON_DrawInput(void)
{
	char *p;
	size_t x;
	INT32 y;

	// input line scrolls left if it gets too long
	p = inputlines[inputline];
	if (input_cx >= con_width-11)
		p += input_cx - (con_width-11) + 1;

	y = con_curlines - 12;

	for (x = 0; x < con_width-11; x++)
		V_DrawCharacter((INT32)(x+1)<<3, y, p[x]|V_NOSCALEPATCH|V_NOSCALESTART, !cv_allcaps.value);

	// draw the blinking cursor
	//
	x = (input_cx >= con_width-11) ? (con_width-11) - 1 : input_cx;
	if (con_tick < 4)
		V_DrawCharacter((INT32)(x+1)<<3, y, '_'|V_NOSCALEPATCH|V_NOSCALESTART, !cv_allcaps.value);
}

// draw the last lines of console text to the top of the screen
static void CON_DrawHudlines(void)
{
	UINT8 *p;
	size_t i, x;
	INT32 y;
	INT32 charflags = 0;

	if (con_hudlines <= 0)
		return;

	if (chat_on)
		y = 8; // leave place for chat input in the first row of text
	else
		y = 0;

	for (i = con_cy - con_hudlines+1; i <= con_cy; i++)
	{
		if ((signed)i < 0)
			continue;
		if (con_hudtime[i%con_hudlines] == 0)
			continue;

		p = (UINT8 *)&con_buffer[(i%con_totallines)*con_width];

		for (x = 0; x < con_width; x++, p++)
		{
			while (*p & 0x80) // Graue 06-19-2004
			{
				charflags = (*p & 0x7f) << 8;
				p++;
			}
			V_DrawCharacter((INT32)(x)<<3, y, (INT32)(*p) | charflags | V_NOSCALEPATCH|V_NOSCALESTART, !cv_allcaps.value);
		}

		V_DrawCharacter((INT32)(x)<<3, y, (p[x]&0xff)|V_NOSCALEPATCH|V_NOSCALESTART, !cv_allcaps.value);
		y += 8;
	}

	// top screen lines that might need clearing when view is reduced
	con_clearlines = y; // this is handled by HU_Erase();
}

// Scale a pic_t at 'startx' pos, to 'destwidth' columns.
//   startx, destwidth is resolution dependent
// Used to draw console borders, console background.
// The pic must be sized BASEVIDHEIGHT height.
static void CON_DrawBackpic(patch_t *pic, INT32 startx, INT32 destwidth)
{
	startx = destwidth = 0;
	V_DrawScaledPatch(0, 0, 0, pic);
}

#if 0
static inline void CON_DrawBackpic2(pic_t *pic, INT32 startx, INT32 destwidth)
{
	INT32 x, y;
	INT32 v;
	UINT8 *src, *dest;
	const UINT8 *deststop;
	INT32 frac, fracstep;

	dest = screens[0]+startx;
	deststop = screens[0] + vid.width * vid.height * vid.bpp;

	for (y = 0; y < con_curlines; y++, dest += vid.width)
	{
		// scale the picture to the resolution
		v = SHORT(pic->height) - ((con_curlines - y) * (BASEVIDHEIGHT-1) / vid.height) - 1;

		src = pic->data + v*SHORT(pic->width);

		// in case of the console backpic, simplify
		if (SHORT(pic->width) == destwidth)
			M_Memcpy(dest, src, destwidth);
		else
		{
			// scale pic to screen width
			frac = 0;
			fracstep = (SHORT(pic->width)<<16)/destwidth;
			for (x = 0; x < destwidth; x += 4)
			{
				if (dest+x > deststop) break;
				dest[x] = src[frac>>FRACBITS];
				frac += fracstep;
				if (dest+x+1 > deststop) break;
				dest[x+1] = src[frac>>FRACBITS];
				frac += fracstep;
				if (dest+x+2 > deststop) break;
				dest[x+2] = src[frac>>FRACBITS];
				frac += fracstep;
				if (dest+x+3 > deststop) break;
				dest[x+3] = src[frac>>FRACBITS];
				frac += fracstep;
			}
		}
	}
}
#endif

// draw the console background, text, and prompt if enough place
//
static void CON_DrawConsole(void)
{
	UINT8 *p;
	size_t i, x;
	INT32 y;
	INT32 w = 0, x2 = 0;
	INT32 charflags = 0;

	if (con_curlines <= 0)
		return;

	//FIXME: refresh borders only when console bg is translucent
	con_clearlines = con_curlines; // clear console draw from view borders
	con_hudupdate = true; // always refresh while console is on

	// draw console background
	if (cons_backpic.value || con_forcepic)
	{
		if (rendermode != render_soft)
			V_DrawScaledPatch(0, 0, 0, con_backpic);
		else if (rendermode != render_none)
			CON_DrawBackpic(con_backpic, 0, vid.width); // picture as background
	}
	else
	{
		x2 = vid.width;
		// Hurdler: what's the correct value of w and x2 in hardware mode???
		if (rendermode != render_none) V_DrawFadeConsBack(w, 0, x2, con_curlines, cons_backcolor.value); // translucent background
	}

	// draw console text lines from top to bottom
	if (con_curlines < 20) // 8+8+4
		return;

	i = con_cy - con_scrollup;

	// skip the last empty line due to the cursor being at the start of a new line
	if (!con_scrollup && !con_cx)
		i--;

	i -= (con_curlines - 20) / 8;

	if (rendermode == render_none) return;

	for (y = (con_curlines-20) % 8; y <= con_curlines-20; y += 8, i++)
	{
		p = (UINT8 *)&con_buffer[((i > 0 ? i : 0)%con_totallines)*con_width];

		for (x = 0; x < con_width; x++, p++)
		{
			while (*p & 0x80)
			{
				charflags = (*p & 0x7f) << 8;
				p++;
			}
			V_DrawCharacter((INT32)(x+1)<<3, y, (INT32)(*p)|charflags|V_NOSCALEPATCH|V_NOSCALESTART, !cv_allcaps.value);
		}
	}

	// draw prompt if enough place (not while game startup)
	if ((con_curlines == con_destlines) && (con_curlines >= 20) && !con_startup)
		CON_DrawInput();
}

// Console refresh drawer, call each frame
//
void CON_Drawer(void)
{
	if (!con_started || !graphics_started)
		return;

	if (con_recalc)
		CON_RecalcSize();

	if (con_curlines > 0)
		CON_DrawConsole();
	else if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION || gamestate == GS_CUTSCENE || gamestate == GS_CREDITS)
		CON_DrawHudlines();
}
