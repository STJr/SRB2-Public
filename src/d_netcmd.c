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
/// \brief host/client network commands
///	commands are executed through the command buffer
///	like console commands
///	other miscellaneous commands (at the end)

#include "doomdef.h"

#include "console.h"
#include "command.h"
#include "i_system.h"
#include "dstrings.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "g_input.h"
#include "m_menu.h"
#include "r_local.h"
#include "r_things.h"
#include "p_local.h"
#include "p_setup.h"
#include "s_sound.h"
#include "m_misc.h"
#include "am_map.h"
#include "byteptr.h"
#include "d_netfil.h"
#include "p_spec.h"
#include "m_cheat.h"
#include "d_clisrv.h"
#include "v_video.h"
#include "d_main.h"
#include "m_random.h"
#include "f_finale.h"
#include "mserv.h"
#include "md5.h"
#include "z_zone.h"

#ifdef JOHNNYFUNCODE
#define CV_JOHNNY CV_NETVAR
#else
#define CV_JOHNNY 0
#endif

//#define DELFILE

// ------
// protos
// ------

static void Got_NameAndColor(UINT8 **cp, INT32 playernum);
static void Got_WeaponPref(UINT8 **cp, INT32 playernum);
static void Got_Mapcmd(UINT8 **cp, INT32 playernum);
static void Got_ExitLevelcmd(UINT8 **cp, INT32 playernum);
static void Got_RequestAddfilecmd(UINT8 **cp, INT32 playernum);
#ifdef DELFILE
static void Got_Delfilecmd(UINT8 **cp, INT32 playernum);
#endif
static void Got_Addfilecmd(UINT8 **cp, INT32 playernum);
static void Got_Pause(UINT8 **cp, INT32 playernum);
static void Got_RandomSeed(UINT8 **cp, INT32 playernum);
static void Got_PizzaOrder(UINT8 **cp, INT32 playernum);
static void Got_RunSOCcmd(UINT8 **cp, INT32 playernum);
static void Got_Teamchange(UINT8 **cp, INT32 playernum);
static void Got_Clearscores(UINT8 **cp, INT32 playernum);

static void PointLimit_OnChange(void);
static void TimeLimit_OnChange(void);
static void NumLaps_OnChange(void);
static void Mute_OnChange(void);

static void AutoBalance_OnChange(void);
static void TeamScramble_OnChange(void);

static void Cheats_OnChange(void);

static void Tagtype_OnChange(void);
static void Hidetime_OnChange(void);

static void NetTimeout_OnChange(void);

static void Ringslinger_OnChange(void);
static void Setrings_OnChange(void);
static void Setlives_OnChange(void);
static void Setcontinues_OnChange(void);
static void Gravity_OnChange(void);
static void ForceSkin_OnChange(void);

static void Name_OnChange(void);
static void Name2_OnChange(void);
static void Skin_OnChange(void);
static void Skin2_OnChange(void);
static void Color_OnChange(void);
static void Color2_OnChange(void);
static void DummyConsvar_OnChange(void);
static void SoundTest_OnChange(void);

#ifdef FISHCAKE
static void Fishcake_OnChange(void);
#endif

static void Command_Playdemo_f(void);
static void Command_Timedemo_f(void);
static void Command_Stopdemo_f(void);
static void Command_StartMovie_f(void);
static void Command_StopMovie_f(void);
static void Command_Map_f(void);
static void Command_Teleport_f(void);
static void Command_RTeleport_f(void);
static void Command_ResetCamera_f(void);

static void Command_OrderPizza_f(void);

static void Command_Addfile(void);
static void Command_ListWADS_f(void);
#ifdef DELFILE
static void Command_Delfile(void);
#endif
static void Command_RunSOC(void);
static void Command_Pause(void);

static void Command_Version_f(void);
#ifdef UPDATE_ALERT
static void Command_ModDetails_f(void);
#endif
static void Command_ShowGametype_f(void);
static void Command_JumpToAxis_f(void);
FUNCNORETURN static ATTRNORETURN void Command_Quit_f(void);
static void Command_Playintro_f(void);
static void Command_Writethings_f(void);

static void Command_Displayplayer_f(void);
static void Command_Tunes_f(void);
static void Command_Skynum_f(void);

static void Command_ExitLevel_f(void);
static void Command_Showmap_f(void);

static void Command_Teamchange_f(void);
static void Command_Teamchange2_f(void);
static void Command_ServerTeamChange_f(void);

static void Command_Clearscores_f(void);

// Remote Administration
static void Command_Changepassword_f(void);
static void Command_Login_f(void);
static void Got_Login(UINT8 **cp, INT32 playernum);
static void Got_Verification(UINT8 **cp, INT32 playernum);
static void Command_Verify_f(void);
static void Command_MotD_f(void);
static void Got_MotD_f(UINT8 **cp, INT32 playernum);

static void Command_ShowScores_f(void);
static void Command_ShowTime_f(void);

static void Command_Isgamemodified_f(void);
#ifdef _DEBUG
static void Command_Togglemodified_f(void);
#endif

// =========================================================================
//                           CLIENT VARIABLES
// =========================================================================

static void SendWeaponPref(void);

static CV_PossibleValue_t usemouse_cons_t[] = {{0, "Off"}, {1, "On"}, {2, "Force"}, {0, NULL}};
#if defined (__unix__) || defined(__APPLE__) || defined (UNIXCOMMON)
static CV_PossibleValue_t mouse2port_cons_t[] = {{0, "/dev/gpmdata"}, {1, "/dev/ttyS0"},
	{2, "/dev/ttyS1"}, {3, "/dev/ttyS2"}, {4, "/dev/ttyS3"}, {0, NULL}};
#else
static CV_PossibleValue_t mouse2port_cons_t[] = {{1, "COM1"}, {2, "COM2"}, {3, "COM3"}, {4, "COM4"},
	{0, NULL}};
#endif

#ifdef LJOYSTICK
static CV_PossibleValue_t joyport_cons_t[] = {{1, "/dev/js0"}, {2, "/dev/js1"}, {3, "/dev/js2"},
	{4, "/dev/js3"}, {0, NULL}};
#else
// accept whatever value - it is in fact the joystick device number
#define usejoystick_cons_t NULL
#endif

static CV_PossibleValue_t matchtype_cons_t[] = {{0, "Normal"}, {1, "Team"}, {0, NULL}};
static CV_PossibleValue_t tagtype_cons_t[] = {{0, "Normal"}, {1, "Hide and Seek"}, {0, NULL}};

static CV_PossibleValue_t autobalance_cons_t[] = {{0, "MIN"}, {4, "MAX"}, {0, NULL}};
static CV_PossibleValue_t teamscramble_cons_t[] = {{0, "Off"}, {1, "Random"}, {2, "Points"}, {0, NULL}};

static CV_PossibleValue_t ringlimit_cons_t[] = {{0, "MIN"}, {9999, "MAX"}, {0, NULL}};
static CV_PossibleValue_t liveslimit_cons_t[] = {{0, "MIN"}, {99, "MAX"}, {0, NULL}};
static CV_PossibleValue_t sleeping_cons_t[] = {{-1, "MIN"}, {1000/TICRATE, "MAX"}, {0, NULL}};
static CV_PossibleValue_t racetype_cons_t[] = {{0, "Normal"}, {1, "Classic"}, {0, NULL}};
static CV_PossibleValue_t raceitemboxes_cons_t[] = {{0, "Normal"}, {1, "Random"}, {2, "Teleports"},
	{3, "None"}, {0, NULL}};

static CV_PossibleValue_t matchboxes_cons_t[] = {{0, "Normal"}, {1, "Random"}, {2, "Non-Random"},
	{3, "None"}, {0, NULL}};

static CV_PossibleValue_t chances_cons_t[] = {{0, "Off"}, {1, "Low"}, {2, "Medium"}, {3, "High"},
	{0, NULL}};
static CV_PossibleValue_t snapto_cons_t[] = {{0, "Off"}, {1, "Floor"}, {2, "Ceiling"}, {3, "Halfway"},
	{0, NULL}};
static CV_PossibleValue_t match_scoring_cons_t[] = {{0, "Normal"}, {1, "Classic"}, {0, NULL}};
static CV_PossibleValue_t pause_cons_t[] = {{0, "Server"}, {1, "All"}, {0, NULL}};

static CV_PossibleValue_t timetic_cons_t[] = {{0, "Off"}, {1, "On"}, {2, "Full"}, {0, NULL}};

#ifdef FISHCAKE
static consvar_t cv_fishcake = {"fishcake", "Off", CV_CALL|CV_NOSHOWHELP, CV_OnOff, Fishcake_OnChange, 0, NULL, NULL, 0, 0, NULL};
#endif
static consvar_t cv_dummyconsvar = {"dummyconsvar", "Off", CV_CALL|CV_NOSHOWHELP, CV_OnOff,
	DummyConsvar_OnChange, 0, NULL, NULL, 0, 0, NULL};

//Console variables used solely in the menu system.
//todo: add a way to use non-console variables in the menu
//      or make these consvars legitimate like color or skin.
static CV_PossibleValue_t dummyteam_cons_t[] = {{0, "Spectator"}, {1, "Red"}, {2, "Blue"}, {0, NULL}};
consvar_t cv_dummyteam = {"dummyteam", "Spectator", CV_HIDEN, dummyteam_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
static CV_PossibleValue_t dummyscramble_cons_t[] = {{0, "Random"}, {1, "Points"}, {0, NULL}};
consvar_t cv_dummyscramble = {"dummyscramble", "Random", CV_HIDEN, dummyscramble_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_allowteamchange = {"allowteamchange", "Yes", CV_NETVAR, CV_YesNo, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_racetype = {"racetype", "Normal", CV_NETVAR, racetype_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_raceitemboxes = {"race_itemboxes", "Random", CV_NETVAR, raceitemboxes_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

#ifdef SEENAMES
static CV_PossibleValue_t seenames_cons_t[] = {{0, "Off"}, {1, "Colorless"}, {2, "Team"}, {3, "Ally/Foe"}, {0, NULL}};
consvar_t cv_seenames = {"seenames", "Ally/Foe", CV_SAVE, seenames_cons_t, 0, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_allowseenames = {"allowseenames", "Yes", CV_NETVAR, CV_YesNo, NULL, 0, NULL, NULL, 0, 0, NULL};
#endif

// these two are just meant to be saved to the config
consvar_t cv_playername = {"name", "Sonic", CV_SAVE|CV_CALL|CV_NOINIT, NULL, Name_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_playercolor = {"color", "Blue", CV_CALL|CV_NOINIT, Color_cons_t, Color_OnChange, 0, NULL, NULL, 0, 0, NULL};
// player's skin, saved for commodity, when using a favorite skins wad..
consvar_t cv_skin = {"skin", DEFAULTSKIN, CV_CALL|CV_NOINIT, NULL, Skin_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_autoaim = {"autoaim", "On", CV_CALL|CV_NOINIT, CV_OnOff, SendWeaponPref, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_autoaim2 = {"autoaim2", "On", CV_CALL|CV_NOINIT, CV_OnOff, SendWeaponPref, 0, NULL, NULL, 0, 0, NULL};
// secondary player for splitscreen mode
consvar_t cv_playername2 = {"name2", "Tails", CV_SAVE|CV_CALL|CV_NOINIT, NULL, Name2_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_playercolor2 = {"color2", "Orange", CV_CALL|CV_NOINIT, Color_cons_t, Color2_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_skin2 = {"skin2", "Tails", CV_CALL|CV_NOINIT, NULL, Skin2_OnChange, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_skipmapcheck = {"skipmapcheck", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

INT32 cv_debug;

consvar_t cv_usemouse = {"use_mouse", "On", CV_SAVE|CV_CALL,usemouse_cons_t, I_StartupMouse, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_usemouse2 = {"use_mouse2", "Off", CV_SAVE|CV_CALL,usemouse_cons_t, I_StartupMouse2, 0, NULL, NULL, 0, 0, NULL};

#if defined (DC) || defined (_XBOX) //joystick 1 and 2
consvar_t cv_usejoystick = {"use_joystick", "1", CV_SAVE|CV_CALL, usejoystick_cons_t,
	I_InitJoystick, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_usejoystick2 = {"use_joystick2", "2", CV_SAVE|CV_CALL, usejoystick_cons_t,
	I_InitJoystick2, 0, NULL, NULL, 0, 0, NULL};
#elif defined (PSP) || defined(GP2X) //only one joystick
consvar_t cv_usejoystick = {"use_joystick", "1", CV_SAVE|CV_CALL, usejoystick_cons_t,
	I_InitJoystick, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_usejoystick2 = {"use_joystick2", "0", CV_SAVE|CV_CALL, usejoystick_cons_t,
	I_InitJoystick2, 0, NULL, NULL, 0, 0, NULL};
#else //all esle, no joystick
consvar_t cv_usejoystick = {"use_joystick", "0", CV_SAVE|CV_CALL, usejoystick_cons_t,
	I_InitJoystick, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_usejoystick2 = {"use_joystick2", "0", CV_SAVE|CV_CALL, usejoystick_cons_t,
	I_InitJoystick2, 0, NULL, NULL, 0, 0, NULL};
#endif
#if (defined (LJOYSTICK) || defined (SDL))
#ifdef LJOYSTICK
consvar_t cv_joyport = {"joyport", "/dev/js0", CV_SAVE, joyport_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_joyport2 = {"joyport2", "/dev/js0", CV_SAVE, joyport_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL}; //Alam: for later
#endif
consvar_t cv_joyscale = {"joyscale", "1", CV_SAVE|CV_CALL, NULL, I_JoyScale, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_joyscale2 = {"joyscale2", "1", CV_SAVE|CV_CALL, NULL, I_JoyScale2, 0, NULL, NULL, 0, 0, NULL};
#else
consvar_t cv_joyscale = {"joyscale", "1", CV_SAVE|CV_HIDEN, NULL, NULL, 0, NULL, NULL, 0, 0, NULL}; //Alam: Dummy for save
consvar_t cv_joyscale2 = {"joyscale2", "1", CV_SAVE|CV_HIDEN, NULL, NULL, 0, NULL, NULL, 0, 0, NULL}; //Alam: Dummy for save
#endif
#if defined (__unix__) || defined(__APPLE__) || defined (UNIXCOMMON)
consvar_t cv_mouse2port = {"mouse2port", "/dev/gpmdata", CV_SAVE, mouse2port_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_mouse2opt = {"mouse2opt", "0", CV_SAVE, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
#else
consvar_t cv_mouse2port = {"mouse2port", "COM2", CV_SAVE, mouse2port_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
#endif
consvar_t cv_matchboxes = {"matchboxes", "Normal", CV_NETVAR, matchboxes_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_specialrings = {"specialrings", "On", CV_NETVAR, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_powerstones = {"powerstones", "On", CV_NETVAR, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

#ifdef CHAOSISNOTDEADYET
consvar_t cv_chaos_bluecrawla = {"chaos_bluecrawla", "High", CV_NETVAR, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_chaos_redcrawla = {"chaos_redcrawla", "High", CV_NETVAR, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_chaos_crawlacommander = {"chaos_crawlacommander", "Low", CV_NETVAR, chances_cons_t,
	NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_chaos_jettysynbomber = {"chaos_jettysynbomber", "Medium", CV_NETVAR, chances_cons_t,
	NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_chaos_jettysyngunner = {"chaos_jettysyngunner", "Low", CV_NETVAR, chances_cons_t,
	NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_chaos_eggmobile1 = {"chaos_eggmobile1", "Low", CV_NETVAR, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_chaos_eggmobile2 = {"chaos_eggmobile2", "Low", CV_NETVAR, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_chaos_skim = {"chaos_skim", "Medium", CV_NETVAR, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_chaos_spawnrate = {"chaos_spawnrate", "30",CV_NETVAR, CV_Unsigned, NULL, 0, NULL, NULL, 0, 0, NULL};
#endif

consvar_t cv_recycler = {"recycler", "Medium", CV_NETVAR|CV_CHEAT, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_teleporters = {"teleporters", "Medium", CV_NETVAR|CV_CHEAT, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_superring = {"superring", "Medium", CV_NETVAR|CV_CHEAT, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_supersneakers = {"supersneakers", "Medium", CV_NETVAR|CV_CHEAT, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_invincibility = {"invincibility", "Medium", CV_NETVAR|CV_CHEAT, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_jumpshield = {"jumpshield", "Medium", CV_NETVAR|CV_CHEAT, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_watershield = {"watershield", "Medium", CV_NETVAR|CV_CHEAT, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_ringshield = {"ringshield", "Medium", CV_NETVAR|CV_CHEAT, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_forceshield = {"forceshield", "Medium", CV_NETVAR|CV_CHEAT, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_bombshield = {"bombshield", "Medium", CV_NETVAR|CV_CHEAT, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_1up = {"1up", "Medium", CV_NETVAR|CV_CHEAT, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_eggmanbox = {"eggmantv", "Medium", CV_NETVAR|CV_CHEAT, chances_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

// Question boxes aren't spawned by randomly respawning monitors, so there is no need
// for chances. Rather, a simple on/off is used.
consvar_t cv_questionbox = {"randomtv", "On", CV_NETVAR, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_ringslinger = {"ringslinger", "No", CV_NETVAR|CV_NOSHOWHELP|CV_CALL|CV_CHEAT, CV_YesNo,
	Ringslinger_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_setrings = {"setrings", "0", CV_NETVAR|CV_NOSHOWHELP|CV_CALL|CV_CHEAT, ringlimit_cons_t,
	Setrings_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_setlives = {"setlives", "0", CV_NETVAR|CV_NOSHOWHELP|CV_CALL|CV_CHEAT, liveslimit_cons_t,
	Setlives_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_setcontinues = {"setcontinues", "0",CV_NETVAR|CV_NOSHOWHELP|CV_CALL|CV_CHEAT,
	liveslimit_cons_t, Setcontinues_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_gravity = {"gravity", "0.5", CV_NETVAR|CV_FLOAT|CV_CALL, NULL, Gravity_OnChange, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_soundtest = {"soundtest", "0", CV_CALL, NULL, SoundTest_OnChange, 0, NULL, NULL, 0, 0, NULL};

static CV_PossibleValue_t minitimelimit_cons_t[] = {{15, "MIN"}, {9999, "MAX"}, {0, NULL}};
consvar_t cv_countdowntime = {"countdowntime", "60", CV_NETVAR, minitimelimit_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_tagtype = {"tagtype", "Normal", CV_NETVAR|CV_CALL, tagtype_cons_t, Tagtype_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_touchtag = {"touchtag", "Off", CV_NETVAR, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_hidetime = {"hidetime", "30", CV_NETVAR|CV_CALL, minitimelimit_cons_t, Hidetime_OnChange, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_autobalance = {"autobalance", "0", CV_NETVAR|CV_CALL, autobalance_cons_t, AutoBalance_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_teamscramble = {"teamscramble", "Off", CV_NETVAR|CV_CALL|CV_NOINIT, teamscramble_cons_t, TeamScramble_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_scrambleonchange = {"scrambleonchange", "Off", CV_NETVAR, teamscramble_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_matchtype = {"matchtype", "Normal", CV_NETVAR|CV_CALL, matchtype_cons_t, MatchType_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_friendlyfire = {"friendlyfire", "Off", CV_NETVAR, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_timetic = {"timetic", "Off", 0, timetic_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL}; // use tics in display
consvar_t cv_objectplace = {"objectplace", "Off", CV_CALL|CV_JOHNNY, CV_OnOff,
	ObjectPlace_OnChange, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_cheats = {"cheats", "Off", CV_NETVAR|CV_CALL, CV_OnOff, Cheats_OnChange, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_snapto = {"snapto", "Off", CV_JOHNNY, snapto_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_speed = {"speed", "16", CV_JOHNNY, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_objflags = {"objflags", "0", CV_JOHNNY, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_mapthingnum = {"mapthingnum", "0", CV_JOHNNY, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grid = {"grid", "0", CV_JOHNNY, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};

// Scoring type options
consvar_t cv_match_scoring = {"match_scoring", "Normal", CV_NETVAR, match_scoring_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_overtime = {"overtime", "Yes", CV_NETVAR, CV_YesNo, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_realnames = {"realnames", "Off", CV_NOSHOWHELP, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_resetmusic = {"resetmusic", "No", CV_SAVE, CV_YesNo, NULL, 0, NULL, NULL, 0, 0, NULL};

static CV_PossibleValue_t pointlimit_cons_t[] = {{0, "MIN"}, {999999990, "MAX"}, {0, NULL}};
consvar_t cv_pointlimit = {"pointlimit", "0", CV_NETVAR|CV_CALL|CV_NOINIT, pointlimit_cons_t,
	PointLimit_OnChange, 0, NULL, NULL, 0, 0, NULL};
static CV_PossibleValue_t timelimit_cons_t[] = {{0, "MIN"}, {30, "MAX"}, {0, NULL}};
consvar_t cv_timelimit = {"timelimit", "0", CV_NETVAR|CV_CALL|CV_NOINIT, timelimit_cons_t,
	TimeLimit_OnChange, 0, NULL, NULL, 0, 0, NULL};
static CV_PossibleValue_t numlump_cons_t[] = {{0, "MIN"}, {50, "MAX"}, {0, NULL}};
consvar_t cv_numlaps = {"numlaps", "4", CV_NETVAR|CV_CALL|CV_NOINIT, numlump_cons_t,
	NumLaps_OnChange, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_forceskin = {"forceskin", "-1", CV_NETVAR|CV_CALL, NULL, ForceSkin_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_downloading = {"downloading", "On", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_allowexitlevel = {"allowexitlevel", "No", CV_NETVAR, CV_YesNo, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_killingdead = {"killingdead", "Off", CV_NETVAR, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_netstat = {"netstat", "Off", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL}; // show bandwidth statistics
static CV_PossibleValue_t nettimeout_cons_t[] = {{TICRATE/7, "MIN"}, {60*TICRATE, "MAX"}, {0, NULL}};
consvar_t cv_nettimeout = {"nettimeout", "525", CV_CALL|CV_SAVE, nettimeout_cons_t, NetTimeout_OnChange, 0, NULL, NULL, 0, 0, NULL};
#ifdef NEWPING
consvar_t cv_maxping = {"maxping", "0", CV_SAVE, CV_Unsigned, NULL, 0, NULL, NULL, 0, 0, NULL};
#endif
// Intermission time Tails 04-19-2002
static CV_PossibleValue_t inttime_cons_t[] = {{0, "MIN"}, {3600, "MAX"}, {0, NULL}};
consvar_t cv_inttime = {"inttime", "15", CV_NETVAR, inttime_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

static CV_PossibleValue_t advancemap_cons_t[] = {{0, "Off"}, {1, "Next"}, {2, "Random"}, {0, NULL}};
consvar_t cv_advancemap = {"advancemap", "Next", CV_NETVAR, advancemap_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
static CV_PossibleValue_t playersforexit_cons_t[] = {{0, "One"}, {1, "All"}, {0, NULL}};
consvar_t cv_playersforexit = {"playersforexit", "One", CV_NETVAR, playersforexit_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_runscripts = {"runscripts", "Yes", 0, CV_YesNo, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_pause = {"pausepermission", "Server", CV_NETVAR, pause_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_mute = {"mute", "Off", CV_NETVAR|CV_CALL, CV_OnOff, Mute_OnChange, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_sleep = {"cpusleep", "-1", CV_SAVE, sleeping_cons_t, NULL, -1, NULL, NULL, 0, 0, NULL};

static boolean triggerforcedskin = false;
INT16 gametype = GT_COOP;
boolean splitscreen = false;
boolean circuitmap = false;
INT32 adminplayer = -1;

// =========================================================================
//                           SERVER STARTUP
// =========================================================================

/** Registers server commands and variables.
  * Anything required by a dedicated server should probably go here.
  *
  * \sa D_RegisterClientCommands
  */
void D_RegisterServerCommands(void)
{
	RegisterNetXCmd(XD_NAMEANDCOLOR, Got_NameAndColor);
	RegisterNetXCmd(XD_WEAPONPREF, Got_WeaponPref);
	RegisterNetXCmd(XD_MAP, Got_Mapcmd);
	RegisterNetXCmd(XD_EXITLEVEL, Got_ExitLevelcmd);
	RegisterNetXCmd(XD_ADDFILE, Got_Addfilecmd);
	RegisterNetXCmd(XD_REQADDFILE, Got_RequestAddfilecmd);
#ifdef DELFILE
	RegisterNetXCmd(XD_DELFILE, Got_Delfilecmd);
#endif
	RegisterNetXCmd(XD_PAUSE, Got_Pause);
	RegisterNetXCmd(XD_RUNSOC, Got_RunSOCcmd);

	// Remote Administration
	COM_AddCommand("password", Command_Changepassword_f);
	RegisterNetXCmd(XD_LOGIN, Got_Login);
	COM_AddCommand("login", Command_Login_f); // useful in dedicated to kick off remote admin
	COM_AddCommand("verify", Command_Verify_f);
	RegisterNetXCmd(XD_VERIFIED, Got_Verification);

	COM_AddCommand("motd", Command_MotD_f);
	RegisterNetXCmd(XD_SETMOTD, Got_MotD_f); // For remote admin

	RegisterNetXCmd(XD_TEAMCHANGE, Got_Teamchange);
	COM_AddCommand("serverchangeteam", Command_ServerTeamChange_f);

	RegisterNetXCmd(XD_CLEARSCORES, Got_Clearscores);
	COM_AddCommand("clearscores", Command_Clearscores_f);
	COM_AddCommand("map", Command_Map_f);

	COM_AddCommand("exitgame", Command_ExitGame_f);
	COM_AddCommand("exitlevel", Command_ExitLevel_f);
	COM_AddCommand("showmap", Command_Showmap_f);

	COM_AddCommand("addfile", Command_Addfile);
	COM_AddCommand("listwad", Command_ListWADS_f);

#ifdef DELFILE
	COM_AddCommand("delfile", Command_Delfile);
#endif
	COM_AddCommand("runsoc", Command_RunSOC);
	COM_AddCommand("pause", Command_Pause);

	COM_AddCommand("gametype", Command_ShowGametype_f);
	COM_AddCommand("jumptoaxis", Command_JumpToAxis_f);
	COM_AddCommand("version", Command_Version_f);
#ifdef UPDATE_ALERT
	COM_AddCommand("mod_details", Command_ModDetails_f);
#endif
	COM_AddCommand("quit", Command_Quit_f);

	COM_AddCommand("saveconfig", Command_SaveConfig_f);
	COM_AddCommand("loadconfig", Command_LoadConfig_f);
	COM_AddCommand("changeconfig", Command_ChangeConfig_f);
	COM_AddCommand("isgamemodified", Command_Isgamemodified_f); // test
	COM_AddCommand("showscores", Command_ShowScores_f);
	COM_AddCommand("showtime", Command_ShowTime_f);
#ifdef _DEBUG
	COM_AddCommand("togglemodified", Command_Togglemodified_f);
#endif

	// for master server connection
	AddMServCommands();

	// p_mobj.c
	CV_RegisterVar(&cv_itemrespawntime);
	CV_RegisterVar(&cv_itemrespawn);
	CV_RegisterVar(&cv_flagtime);
	CV_RegisterVar(&cv_suddendeath);

	// misc
	CV_RegisterVar(&cv_matchtype);
	CV_RegisterVar(&cv_friendlyfire);
	CV_RegisterVar(&cv_pointlimit);
	CV_RegisterVar(&cv_numlaps);
	CV_RegisterVar(&cv_timetic);

	CV_RegisterVar(&cv_cheats);

	CV_RegisterVar(&cv_autobalance);
	CV_RegisterVar(&cv_teamscramble);
	CV_RegisterVar(&cv_scrambleonchange);

	CV_RegisterVar(&cv_tagtype);
	CV_RegisterVar(&cv_touchtag);
	CV_RegisterVar(&cv_hidetime);

	CV_RegisterVar(&cv_inttime);
	CV_RegisterVar(&cv_advancemap);
	CV_RegisterVar(&cv_playersforexit);
	CV_RegisterVar(&cv_timelimit);
	CV_RegisterVar(&cv_playdemospeed);
	CV_RegisterVar(&cv_forceskin);
	CV_RegisterVar(&cv_downloading);

	CV_RegisterVar(&cv_specialrings);
	CV_RegisterVar(&cv_powerstones);
	CV_RegisterVar(&cv_racetype);
	CV_RegisterVar(&cv_raceitemboxes);
	CV_RegisterVar(&cv_matchboxes);

#ifdef CHAOSISNOTDEADYET
	CV_RegisterVar(&cv_chaos_bluecrawla);
	CV_RegisterVar(&cv_chaos_redcrawla);
	CV_RegisterVar(&cv_chaos_crawlacommander);
	CV_RegisterVar(&cv_chaos_jettysynbomber);
	CV_RegisterVar(&cv_chaos_jettysyngunner);
	CV_RegisterVar(&cv_chaos_eggmobile1);
	CV_RegisterVar(&cv_chaos_eggmobile2);
	CV_RegisterVar(&cv_chaos_skim);
	CV_RegisterVar(&cv_chaos_spawnrate);
#endif

	CV_RegisterVar(&cv_recycler);
	CV_RegisterVar(&cv_teleporters);
	CV_RegisterVar(&cv_superring);
	CV_RegisterVar(&cv_supersneakers);
	CV_RegisterVar(&cv_invincibility);
	CV_RegisterVar(&cv_jumpshield);
	CV_RegisterVar(&cv_watershield);
	CV_RegisterVar(&cv_ringshield);
	CV_RegisterVar(&cv_forceshield);
	CV_RegisterVar(&cv_bombshield);
	CV_RegisterVar(&cv_1up);
	CV_RegisterVar(&cv_eggmanbox);
	CV_RegisterVar(&cv_questionbox);

	CV_RegisterVar(&cv_ringslinger);
	CV_RegisterVar(&cv_setrings);
	CV_RegisterVar(&cv_setlives);
	CV_RegisterVar(&cv_setcontinues);
	CV_RegisterVar(&cv_soundtest);

	CV_RegisterVar(&cv_countdowntime);
	CV_RegisterVar(&cv_runscripts);
	CV_RegisterVar(&cv_match_scoring);
	CV_RegisterVar(&cv_overtime);
	CV_RegisterVar(&cv_pause);
	CV_RegisterVar(&cv_mute);

	RegisterNetXCmd(XD_RANDOMSEED, Got_RandomSeed);

	RegisterNetXCmd(XD_ORDERPIZZA, Got_PizzaOrder);

	CV_RegisterVar(&cv_allowexitlevel);
	CV_RegisterVar(&cv_allowautoaim);
	CV_RegisterVar(&cv_allowteamchange);
	CV_RegisterVar(&cv_killingdead);

	// d_clisrv
	CV_RegisterVar(&cv_maxplayers);
	CV_RegisterVar(&cv_maxsend);

	COM_AddCommand("ping", Command_Ping_f);
	CV_RegisterVar(&cv_nettimeout);

	// In-game thing placing stuff
	CV_RegisterVar(&cv_objectplace);
	CV_RegisterVar(&cv_snapto);
	CV_RegisterVar(&cv_speed);
	CV_RegisterVar(&cv_objflags);
	CV_RegisterVar(&cv_mapthingnum);
	CV_RegisterVar(&cv_grid);
	CV_RegisterVar(&cv_skipmapcheck);

	CV_RegisterVar(&cv_sleep);
#ifdef NEWPING
	CV_RegisterVar(&cv_maxping);
#endif

#ifdef SEENAMES
	 CV_RegisterVar(&cv_allowseenames);
#endif

	CV_RegisterVar(&cv_dummyconsvar);
}

// =========================================================================
//                           CLIENT STARTUP
// =========================================================================

/** Registers client commands and variables.
  * Nothing needed for a dedicated server should be registered here.
  *
  * \sa D_RegisterServerCommands
  */
void D_RegisterClientCommands(void)
{
	const char *username;
	INT32 i;

	for (i = 0; i < MAXSKINCOLORS; i++)
	{
		Color_cons_t[i].value = i;
		Color_cons_t[i].strvalue = Color_Names[i];
	}
	Color_cons_t[MAXSKINCOLORS].value = 0;
	Color_cons_t[MAXSKINCOLORS].strvalue = NULL;

	if (dedicated)
		return;

	COM_AddCommand("numthinkers", Command_Numthinkers_f);
	COM_AddCommand("countmobjs", Command_CountMobjs_f);

	COM_AddCommand("changeteam", Command_Teamchange_f);
	COM_AddCommand("changeteam2", Command_Teamchange2_f);

	COM_AddCommand("playdemo", Command_Playdemo_f);
	COM_AddCommand("timedemo", Command_Timedemo_f);
	COM_AddCommand("stopdemo", Command_Stopdemo_f);
	COM_AddCommand("startmovie", Command_StartMovie_f);
	COM_AddCommand("stopmovie", Command_StopMovie_f);
	COM_AddCommand("teleport", Command_Teleport_f);
	COM_AddCommand("rteleport", Command_RTeleport_f);
	COM_AddCommand("playintro", Command_Playintro_f);
	COM_AddCommand("writethings", Command_Writethings_f);

	COM_AddCommand("orderpizza", Command_OrderPizza_f);

	COM_AddCommand("resetcamera", Command_ResetCamera_f);

	COM_AddCommand("setcontrol", Command_Setcontrol_f);
	COM_AddCommand("setcontrol2", Command_Setcontrol2_f);

	COM_AddCommand("screenshot", M_ScreenShot);
	CV_RegisterVar(&cv_screenshot_option);
	CV_RegisterVar(&cv_screenshot_folder);
	CV_RegisterVar(&cv_zlib_level);
	CV_RegisterVar(&cv_zlib_memory);
	CV_RegisterVar(&cv_zlib_strategy);
	CV_RegisterVar(&cv_zlib_window_bits);
	CV_RegisterVar(&cv_zlib_levela);
	CV_RegisterVar(&cv_zlib_memorya);
	CV_RegisterVar(&cv_zlib_strategya);
	CV_RegisterVar(&cv_zlib_window_bitsa);
	CV_RegisterVar(&cv_apng_disable);
	CV_RegisterVar(&cv_splats);

	// register these so it is saved to config
	if ((username = I_GetUserName()))
		cv_playername.defaultvalue =  username;
	CV_RegisterVar(&cv_playername);
	CV_RegisterVar(&cv_playercolor);
#ifdef SEENAMES
	CV_RegisterVar(&cv_seenames);
#endif
	CV_RegisterVar(&cv_realnames);
	CV_RegisterVar(&cv_netstat);

#ifdef FISHCAKE
	CV_RegisterVar(&cv_fishcake);
#endif

	COM_AddCommand("displayplayer", Command_Displayplayer_f);
	COM_AddCommand("tunes", Command_Tunes_f);
	CV_RegisterVar(&cv_resetmusic);
	COM_AddCommand("skynum", Command_Skynum_f);

	// r_things.c (skin NAME)
	CV_RegisterVar(&cv_skin);
	// secondary player (splitscreen)
	CV_RegisterVar(&cv_skin2);
	CV_RegisterVar(&cv_playername2);
	CV_RegisterVar(&cv_playercolor2);

	// FIXME: not to be here.. but needs be done for config loading
	CV_RegisterVar(&cv_usegamma);

	// m_menu.c
	CV_RegisterVar(&cv_crosshair);
	CV_RegisterVar(&cv_invertmouse);
	CV_RegisterVar(&cv_alwaysfreelook);
	CV_RegisterVar(&cv_mousemove);

	// see m_menu.c
	CV_RegisterVar(&cv_crosshair2);
	CV_RegisterVar(&cv_autoaim);
	CV_RegisterVar(&cv_autoaim2);

	// g_input.c
	CV_RegisterVar(&cv_usemouse2);
	CV_RegisterVar(&cv_invertmouse2);
	CV_RegisterVar(&cv_alwaysfreelook2);
	CV_RegisterVar(&cv_mousemove2);
	CV_RegisterVar(&cv_mousesens2);
	CV_RegisterVar(&cv_mlooksens2);
	CV_RegisterVar(&cv_sideaxis);
	CV_RegisterVar(&cv_turnaxis);
	CV_RegisterVar(&cv_moveaxis);
	CV_RegisterVar(&cv_lookaxis);
	CV_RegisterVar(&cv_fireaxis);
	CV_RegisterVar(&cv_firenaxis);
	CV_RegisterVar(&cv_sideaxis2);
	CV_RegisterVar(&cv_turnaxis2);
	CV_RegisterVar(&cv_moveaxis2);
	CV_RegisterVar(&cv_lookaxis2);
	CV_RegisterVar(&cv_fireaxis2);
	CV_RegisterVar(&cv_firenaxis2);

	// WARNING: the order is important when initialising mouse2
	// we need the mouse2port
	CV_RegisterVar(&cv_mouse2port);
#if defined (__unix__) || defined(__APPLE__) || defined (UNIXCOMMON)
	CV_RegisterVar(&cv_mouse2opt);
#endif
	CV_RegisterVar(&cv_mousesens);
	CV_RegisterVar(&cv_mlooksens);
	CV_RegisterVar(&cv_controlperkey);

	CV_RegisterVar(&cv_usemouse);
	CV_RegisterVar(&cv_usejoystick);
	CV_RegisterVar(&cv_usejoystick2);
#ifdef LJOYSTICK
	CV_RegisterVar(&cv_joyport);
	CV_RegisterVar(&cv_joyport2);
#endif
	CV_RegisterVar(&cv_joyscale);
	CV_RegisterVar(&cv_joyscale2);

	// Analog Control
	CV_RegisterVar(&cv_analog);
	CV_RegisterVar(&cv_analog2);
	CV_RegisterVar(&cv_useranalog);
	CV_RegisterVar(&cv_useranalog2);

	// s_sound.c
	CV_RegisterVar(&cv_soundvolume);
	CV_RegisterVar(&cv_digmusicvolume);
	CV_RegisterVar(&cv_midimusicvolume);
	CV_RegisterVar(&cv_numChannels);

	// i_cdmus.c
	CV_RegisterVar(&cd_volume);
	CV_RegisterVar(&cdUpdate);

	// screen.c
	CV_RegisterVar(&cv_fullscreen);
	CV_RegisterVar(&cv_renderview);
	CV_RegisterVar(&cv_scr_depth);
	CV_RegisterVar(&cv_scr_width);
	CV_RegisterVar(&cv_scr_height);

	// p_fab.c
	CV_RegisterVar(&cv_translucency);

	// add cheat commands
	COM_AddCommand("noclip", Command_CheatNoClip_f);
	COM_AddCommand("god", Command_CheatGod_f);
	COM_AddCommand("getallemeralds", Command_Getallemeralds_f);
	COM_AddCommand("resetemeralds", Command_Resetemeralds_f);
#ifdef _DEBUG
	COM_AddCommand("unlockall", Command_Unlockall_f);
#endif
	COM_AddCommand("devmode", Command_Devmode_f);
	COM_AddCommand("savecheckpoint", Command_Savecheckpoint_f);
	COM_AddCommand("scale", Command_Scale_f);
	COM_AddCommand("gravflip", Command_Gravflip_f);
	COM_AddCommand("hurtme", Command_Hurtme_f);
	COM_AddCommand("charability", Command_Charability_f);
	COM_AddCommand("charspeed", Command_Charspeed_f);
#ifdef _DEBUG
	COM_AddCommand("causecfail", Command_CauseCfail_f);
#endif

	// hacks for menu system.
	CV_RegisterVar(&cv_dummyteam);
	CV_RegisterVar(&cv_dummyscramble);
}

#if 0 //keeping this for refrence purposes. -Jazz
static void Command_DummyCommand_f(void)
{
	dummypacket_union NetPacket;
	UINT16 usvalue;
	NetPacket.value.l = NetPacket.value.b = 0;

	//   0        1         2
	// dummy <playernum> <value>

	if (COM_Argc() < 2)
	{
		CONS_Printf("dummy <playernum> <value>\n");
		return;
	}

	NetPacket.packet.infosent = atoi(COM_Argv(2));

	if (!playeringame[atoi(COM_Argv(1))])
	{
		CONS_Printf("Can't send a dummy command to a dummy player that doesn't exist, dummy! =P\n");
		return;
	}
	else
		NetPacket.packet.playernum = atoi(COM_Argv(1));

	if (server)
		NetPacket.packet.verification = true;

	usvalue = SHORT(NetPacket.value.l|NetPacket.value.b);
	SendNetXCmd(XD_DUMMY, &usvalue, sizeof(usvalue));
}

static void Got_DummyCommand(UINT8 **cp, INT32 playernum)
{
	dummypacket_union NetPacket;

	NetPacket.value.l = NetPacket.value.b = READINT16(*cp);

#ifdef PARANOIA
	if (playernum < 0 || playernum > MAXPLAYERS)
	{
		CONS_Printf("There is no player %d! from this dummy command", playernum);
	}
	else
#endif
	if (NetPacket.packet.verification)
	{
		CONS_Printf("Legitimate dummy packet sent.\n");

		if (NetPacket.packet.playernum == (unsigned)playernum)
			CONS_Printf("Hi dummy. Here's your info: %d\n", NetPacket.packet.infosent);

	}
	else
		CONS_Printf("Bad dummy packet sent.\n");
}
#endif


/** Checks if a name (as received from another player) is okay.
  * A name is okay if it is no fewer than 1 and no more than ::MAXPLAYERNAME
  * chars long (not including NUL), it does not begin or end with a space,
  * it does not contain non-printing characters (according to isprint(), which
  * allows space), it does not start with a digit, and no other player is
  * currently using it.
  * \param name      Name to check.
  * \param playernum Player who wants the name, so we can check if they already
  *                  have it, and let them keep it if so.
  * \sa CleanupPlayerName, SetPlayerName, Got_NameAndColor
  * \author Graue <graue@oceanbase.org>
  */
static boolean IsNameGood(char *name, INT32 playernum)
{
	INT32 ix;

	if (strlen(name) == 0 || strlen(name) > MAXPLAYERNAME)
		return false; // Empty or too long.
	if (name[0] == ' ' || name[strlen(name)-1] == ' ')
		return false; // Starts or ends with a space.
	if (isdigit(name[0]))
		return false; // Starts with a digit.
	if (name[0] == '@' || name[0] == '~')
		return false; // Starts with an admin symbol.

	// Check if it contains a non-printing character.
	// Note: ANSI C isprint() considers space a printing character.
	// Also don't allow semicolons, since they are used as
	// console command separators.
	for (ix = 0; name[ix] != '\0'; ix++)
		if (!isprint(name[ix]) || name[ix] == ';')
			return false;

	// Check if a player is currently using the name, case-insensitively.
	for (ix = 0; ix < MAXPLAYERS; ix++)
	{
		if (ix != playernum && playeringame[ix]
			&& strcasecmp(name, player_names[ix]) == 0)
		{
			// We shouldn't kick people out just because
			// they joined the game with the same name
			// as someone else -- modify the name instead.
			size_t len = strlen(name);

			// Recursion!
			// Slowly strip characters off the end of the
			// name until we no longer have a duplicate.
			if (len > 1)
			{
				name[len-1] = '\0';
				if (!IsNameGood (name, playernum))
					return false;
			}
			else if (len == 1) // Agh!
			{
				// Last ditch effort...
				sprintf(name, "%d", M_Random() & 7);
				if (!IsNameGood (name, playernum))
					return false;
			}
			else
				return false;
		}
	}

	return true;
}

/** Cleans up a local player's name before sending a name change.
  * Spaces at the beginning or end of the name are removed. Then if the new
  * name is identical to another player's name, ignoring case, the name change
  * is canceled, and the name in cv_playername.value or cv_playername2.value
  * is restored to what it was before.
  *
  * We assume that if playernum is ::consoleplayer or ::secondarydisplayplayer
  * the console variable ::cv_playername or ::cv_playername2 respectively is
  * already set to newname. However, the player name table is assumed to
  * contain the old name.
  *
  * \param playernum Player number who has requested a name change.
  *                  Should be ::consoleplayer or ::secondarydisplayplayer.
  * \param newname   New name for that player; should already be in
  *                  ::cv_playername or ::cv_playername2 if player is the
  *                  console or secondary display player, respectively.
  * \sa cv_playername, cv_playername2, SendNameAndColor, SendNameAndColor2,
  *     SetPlayerName
  * \author Graue <graue@oceanbase.org>
  */
static void CleanupPlayerName(INT32 playernum, const char *newname)
{
	char *buf;
	char *p;
	char *tmpname = NULL;
	INT32 i;
	boolean namefailed = true;

	buf = Z_StrDup(newname);

	do
	{
		p = buf;

		while (*p == ' ')
			p++; // remove leading spaces

		if (strlen(p) == 0)
			break; // empty names not allowed

		if (isdigit(*p))
			break; // names starting with digits not allowed

		if (*p == '@' || *p == '~')
			break; // names that start with @ or ~ (admin symbols) not allowed

		tmpname = p;

		// Remove trailing spaces.
		p = &tmpname[strlen(tmpname)-1]; // last character
		while (*p == ' ' && p >= tmpname)
		{
			*p = '\0';
			p--;
		}

		if (strlen(tmpname) == 0)
			break; // another case of an empty name

		// Truncate name if it's too long (max MAXPLAYERNAME chars
		// excluding NUL).
		if (strlen(tmpname) > MAXPLAYERNAME)
			tmpname[MAXPLAYERNAME] = '\0';

		// Remove trailing spaces again.
		p = &tmpname[strlen(tmpname)-1]; // last character
		while (*p == ' ' && p >= tmpname)
		{
			*p = '\0';
			p--;
		}

		// no stealing another player's name
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (i != playernum && playeringame[i]
				&& strcasecmp(tmpname, player_names[i]) == 0)
			{
				break;
			}
		}

		if (i < MAXPLAYERS)
			break;

		// name is okay then
		namefailed = false;
	} while (0);

	if (namefailed)
		tmpname = player_names[playernum];

	// set consvars whether namefailed or not, because even if it succeeded,
	// spaces may have been removed
	if (playernum == consoleplayer)
		CV_StealthSet(&cv_playername, tmpname);
	else if (playernum == secondarydisplayplayer
		|| (!netgame && playernum == 1))
	{
		CV_StealthSet(&cv_playername2, tmpname);
	}
	else I_Assert(((void)"CleanupPlayerName used on non-local player", 0));

	Z_Free(buf);
}

/** Sets a player's name, if it is good.
  * If the name is not good (indicating a modified or buggy client), it is not
  * set, and if we are the server in a netgame, the player responsible is
  * kicked with a consistency failure.
  *
  * This function prints a message indicating the name change, unless the game
  * is currently showing the intro, e.g. when processing autoexec.cfg.
  *
  * \param playernum Player number who has requested a name change.
  * \param newname   New name for that player. Should be good, but won't
  *                  necessarily be if the client is maliciously modified or
  *                  buggy.
  * \sa CleanupPlayerName, IsNameGood
  * \author Graue <graue@oceanbase.org>
  */
static void SetPlayerName(INT32 playernum, char *newname)
{
	if (IsNameGood(newname, playernum))
	{
		if (strcasecmp(newname, player_names[playernum]) != 0)
		{
			if (gamestate != GS_INTRO && gamestate != GS_INTRO2)
				CONS_Printf(text[RENAMED_TO],
					player_names[playernum], newname);
			strcpy(player_names[playernum], newname);
		}
	}
	else
	{
		CONS_Printf(text[ILLEGALNAMECMD], playernum+1);
		if (server && netgame)
		{
			XBOXSTATIC UINT8 buf[2];

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
		}
	}
}

static INT32 snacpending = 0, snac2pending = 0, chmappending = 0;

// name, color, or skin has changed
//
static void SendNameAndColor(void)
{
	XBOXSTATIC char buf[MAXPLAYERNAME+1+SKINNAMESIZE+1];
	char *p;
	UINT8 extrainfo = 0; // color and (if applicable) CTF team

	if (netgame && !addedtogame)
		return;

	p = buf;

	// normal player colors in single player
	if (!multiplayer && !netgame && (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION || gamestate == GS_WAITINGPLAYERS))
		if (cv_playercolor.value != players[consoleplayer].prefcolor)
			CV_StealthSetValue(&cv_playercolor, players[consoleplayer].prefcolor);

	// normal player colors
	if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
	{
		if (players[consoleplayer].ctfteam == 1 && cv_playercolor.value != 6)
			CV_StealthSetValue(&cv_playercolor, 6); // Red
		else if (players[consoleplayer].ctfteam == 2 && cv_playercolor.value != 7)
			CV_StealthSetValue(&cv_playercolor, 7); // Blue
	}

	// disallow the use of yellow in Match/Team Match/CTF
	if (gametype == GT_MATCH || gametype == GT_CTF)
	{
		if (cv_playercolor.value == 15) //yellow
			CV_StealthSetValue(&cv_playercolor, players[consoleplayer].skincolor);
	}

	// never allow the color "none"
	if (!cv_playercolor.value)
	{
		if (players[consoleplayer].skincolor)
			CV_StealthSetValue(&cv_playercolor, players[consoleplayer].skincolor);
		else if (players[consoleplayer].prefcolor)
			CV_StealthSetValue(&cv_playercolor, players[consoleplayer].prefcolor);
		else
			CV_StealthSet(&cv_playercolor, cv_playercolor.defaultvalue);
	}

	extrainfo = (UINT8)(extrainfo + (UINT8)cv_playercolor.value);

	// If you're not in a netgame, merely update the skin, color, and name.
	if (!netgame)
	{
		INT32 foundskin;

		players[consoleplayer].skincolor = (cv_playercolor.value&31) % MAXSKINCOLORS;

		if (players[consoleplayer].mo)
		{
			players[consoleplayer].mo->flags |= MF_TRANSLATION;
			players[consoleplayer].mo->color = (UINT8)players[consoleplayer].skincolor;
		}

		if (cv_mute.value && !(server || adminplayer == consoleplayer)) //server doesn't want name changes.
		{
			CV_StealthSet(&cv_playername, player_names[consoleplayer]);
			SetPlayerName(consoleplayer, player_names[consoleplayer]);
		}
		else
		{
			CleanupPlayerName(consoleplayer, cv_playername.zstring);
			SetPlayerName(consoleplayer, cv_playername.zstring);
		}

		if (cv_forceskin.value >= 0 && (netgame || multiplayer)) // Server wants everyone to use the same player
		{
			const INT32 forcedskin = cv_forceskin.value;

			if (triggerforcedskin)
			{
				INT32 i;

				for (i = 0; i < MAXPLAYERS; i++)
				{
					if (playeringame[i])
					{
						SetPlayerSkinByNum(i, forcedskin);

						// If it's me (or my brother), set appropriate skin value in cv_skin/cv_skin2
						if (i == consoleplayer)
							CV_StealthSet(&cv_skin, skins[forcedskin].name);
						else if (i == secondarydisplayplayer)
							CV_StealthSet(&cv_skin2, skins[forcedskin].name);
					}
				}
				triggerforcedskin = false;
			}
			else
			{
				SetPlayerSkinByNum(consoleplayer, forcedskin);
				CV_StealthSet(&cv_skin, skins[forcedskin].name);
			}
		}
		else if ((foundskin = R_SkinAvailable(cv_skin.string)) != -1)
		{
			boolean notsame;

			cv_skin.value = foundskin;

			notsame = (cv_skin.value != players[consoleplayer].skin);

			SetPlayerSkin(consoleplayer, cv_skin.string);
			CV_StealthSet(&cv_skin, skins[cv_skin.value].name);

			if (notsame)
			{
				CV_StealthSetValue(&cv_playercolor, players[consoleplayer].prefcolor);

				players[consoleplayer].skincolor = (cv_playercolor.value&31) % MAXSKINCOLORS;

				if (players[consoleplayer].mo)
				{
					players[consoleplayer].mo->flags |= MF_TRANSLATION;
					players[consoleplayer].mo->color = (UINT8)players[consoleplayer].skincolor;
				}
			}
		}

		return;
	}

	snacpending++;

	WRITEUINT8(p, extrainfo);

	if (cv_mute.value && !(server || adminplayer == consoleplayer))
	{
		WRITESTRING(p, player_names[consoleplayer]);
		CV_StealthSet(&cv_playername, player_names[consoleplayer]);
		SetPlayerName(consoleplayer, player_names[consoleplayer]);
	}
	else
	{
		// As before, CleanupPlayerName truncates the string for us if need be,
		// so no need to check here.
		CleanupPlayerName(consoleplayer, cv_playername.zstring);
		WRITESTRING(p, cv_playername.string);
	}

	// Don't change skin if the server doesn't want you to.
	if (!server && (cv_forceskin.value != -1) && !(adminplayer == consoleplayer && serverplayer == -1))
	{
		SendNetXCmd(XD_NAMEANDCOLOR, buf, p - buf);
		return;
	}

	// check if player has the skin loaded (cv_skin may have
	// the name of a skin that was available in the previous game)
	cv_skin.value = R_SkinAvailable(cv_skin.string);
	if (!cv_skin.value)
	{
		WRITESTRINGN(p, DEFAULTSKIN, SKINNAMESIZE)
		CV_StealthSet(&cv_skin, DEFAULTSKIN);
		SetPlayerSkin(consoleplayer, DEFAULTSKIN);
	}
	else
		WRITESTRINGN(p, cv_skin.string, SKINNAMESIZE);

	SendNetXCmd(XD_NAMEANDCOLOR, buf, p - buf);
}

// splitscreen
static void SendNameAndColor2(void)
{
	XBOXSTATIC char buf[MAXPLAYERNAME+1+SKINNAMESIZE+1];
	char *p;
	INT32 secondplaya;
	UINT8 extrainfo = 0;

	if (!splitscreen)
		return; // can happen if skin2/color2/name2 changed

	if (netgame)
		secondplaya = secondarydisplayplayer;
	else
		secondplaya = 1;

	p = buf;

	// normal player colors
	if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
	{
		if (players[secondplaya].ctfteam == 1 && cv_playercolor2.value != 6)
			CV_StealthSetValue(&cv_playercolor2, 6);
		else if (players[secondplaya].ctfteam == 2 && cv_playercolor2.value != 7)
			CV_StealthSetValue(&cv_playercolor2, 7);
	}

	// disallow the use of yellow in Match/Team Match/CTF
	if (gametype == GT_MATCH || gametype == GT_CTF)
	{
		if (cv_playercolor2.value == 15) //yellow
			CV_StealthSetValue(&cv_playercolor2, players[secondplaya].skincolor);
	}

	// never allow the color "none"
	if (!cv_playercolor2.value)
	{
		if (players[secondplaya].skincolor)
			CV_StealthSetValue(&cv_playercolor2, players[secondplaya].skincolor);
		else if (players[secondplaya].prefcolor)
			CV_StealthSetValue(&cv_playercolor2, players[secondplaya].prefcolor);
		else
			CV_StealthSet(&cv_playercolor2, cv_playercolor2.defaultvalue);
	}

	extrainfo = (UINT8)cv_playercolor2.value; // do this after, because the above might've changed it

	// If you're not in a netgame, merely update the skin, color, and name.
	if (!netgame || (server && secondplaya == consoleplayer))
	{
		INT32 foundskin;
		// don't use secondarydisplayplayer: the second player must be 1
		players[1].skincolor = cv_playercolor2.value;
		if (players[1].mo)
		{
			players[1].mo->flags |= MF_TRANSLATION;
			players[1].mo->color = (UINT8)players[1].skincolor;
		}

		if (cv_mute.value) //server doesn't want name changes.
		{
			CV_StealthSet(&cv_playername2, player_names[secondarydisplayplayer]);
			SetPlayerName(secondarydisplayplayer, player_names[secondarydisplayplayer]);
		}
		else
		{
			CleanupPlayerName(1, cv_playername2.zstring);
			SetPlayerName(1, cv_playername2.zstring);
		}

		if (cv_forceskin.value >= 0 && (netgame || multiplayer)) // Server wants everyone to use the same player
		{
			const INT32 forcedskin = cv_forceskin.value;

			SetPlayerSkinByNum(consoleplayer, forcedskin);
			CV_StealthSet(&cv_skin, skins[forcedskin].name);
		}
		else if ((foundskin = R_SkinAvailable(cv_skin2.string)) != -1)
		{
			boolean notsame;

			cv_skin2.value = foundskin;

			notsame = (cv_skin2.value != players[1].skin);

			SetPlayerSkin(1, cv_skin2.string);

			if (notsame)
			{
				CV_StealthSetValue(&cv_playercolor2, players[1].prefcolor);

				players[1].skincolor = (cv_playercolor2.value&31) % MAXSKINCOLORS;

				if (players[1].mo)
				{
					players[1].mo->flags |= MF_TRANSLATION;
					players[1].mo->color = (UINT8)players[1].skincolor;
				}
			}
		}
		return;
	}
	else if (!addedtogame || secondplaya == consoleplayer)
		return;

	snac2pending++;

	WRITEUINT8(p, extrainfo);

	if (cv_mute.value)
	{
		WRITESTRING(p, player_names[secondarydisplayplayer]);
		CV_StealthSet(&cv_playername2, player_names[secondarydisplayplayer]);
		SetPlayerName(secondarydisplayplayer, player_names[secondarydisplayplayer]);
	}
	else
	{
		// As before, CleanupPlayerName truncates the string for us if need be,
		// so no need to check here.
		CleanupPlayerName(secondarydisplayplayer, cv_playername2.zstring);
		WRITESTRING(p, cv_playername2.string);
	}

	// Don't change skin if the server doesn't want you to.
	// Note: Splitscreen player is never serverplayer. No exceptions!
	if (cv_forceskin.value != -1 && (netgame || multiplayer))
	{
		SendNetXCmd2(XD_NAMEANDCOLOR, buf, p - buf);
		return;
	}

	// check if player has the skin loaded (cv_skin may have
	// the name of a skin that was available in the previous game)
	cv_skin2.value = R_SkinAvailable(cv_skin2.string);
	if (!cv_skin2.value)
	{
		WRITESTRINGN(p, DEFAULTSKIN, SKINNAMESIZE);
		CV_StealthSet(&cv_skin2, DEFAULTSKIN);
		SetPlayerSkin(secondarydisplayplayer, DEFAULTSKIN);
	}
	else
		WRITESTRINGN(p, cv_skin2.string, SKINNAMESIZE);

	SendNetXCmd2(XD_NAMEANDCOLOR, buf, p - buf);
}

static void Got_NameAndColor(UINT8 **cp, INT32 playernum)
{
	player_t *p = &players[playernum];
	INT32 i;
	char *str;
	UINT8 extrainfo;

#ifdef PARANOIA
	if (playernum < 0 || playernum > MAXPLAYERS)
		I_Error("There is no player %d!", playernum);
#endif

	if (netgame && !server && !addedtogame)
	{
		// A bogus change from a player on the server.
		// There's a one-tic delay on XD_ messages. Here's how
		// this happens: on tic 1, server sends an XD_NAMEANDCOLOR
		// message, starting the server; on tic 2, we join and
		// receive the XD_NAMEANDCOLOR and the server sends our
		// XD_ADDPLAYER message, so we haven't gotten it yet.
		// So we get the name and color message first, and think
		// it means us, since it refers to player 0, which we think
		// we are, having not received an XD_ADDPLAYER message
		// telling us otherwise.
		// FIXME: What a mess.

		// Skip the message, ignoring it. The server will send
		// another later.
		extrainfo = READUINT8(*cp);
		SKIPSTRING(*cp); // name
		SKIPSTRING(*cp); // skin
		return;
	}

	if (playernum == consoleplayer)
		snacpending--;
	else if (playernum == secondarydisplayplayer)
		snac2pending--;

#ifdef PARANOIA
	if (snacpending < 0 || snac2pending < 0)
		I_Error("snacpending negative!");
#endif

	extrainfo = READUINT8(*cp);

	if (playernum == consoleplayer && ((extrainfo&31) % MAXSKINCOLORS) != cv_playercolor.value
		&& !snacpending && !chmappending)
	{
		I_Error("consoleplayer color received as %d, cv_playercolor.value is %d",
			(extrainfo&31) % MAXSKINCOLORS, cv_playercolor.value);
	}
	if (splitscreen && playernum == secondarydisplayplayer
		&& ((extrainfo&31) % MAXSKINCOLORS) != cv_playercolor2.value && !snac2pending
		&& !chmappending)
	{
		I_Error("secondarydisplayplayer color received as %d, cv_playercolor2.value is %d",
			(extrainfo&31) % MAXSKINCOLORS, cv_playercolor2.value);
	}

	str = (char *)*cp; // name
	SKIPSTRING(*cp);
	if (strcasecmp(player_names[playernum], str) != 0)
		SetPlayerName(playernum, str);

	// moving players cannot change colors
	if (P_PlayerMoving(playernum) && p->skincolor != (extrainfo&31))
	{
		if (playernum == consoleplayer)
			CV_StealthSetValue(&cv_playercolor, p->skincolor);
		else if (splitscreen && playernum == secondarydisplayplayer)
			CV_StealthSetValue(&cv_playercolor2, p->skincolor);
	}
	else
	{
		p->skincolor = (extrainfo&31) % MAXSKINCOLORS;

		// a copy of color
		if (p->mo)
		{
			p->mo->flags |= MF_TRANSLATION;
			p->mo->color = (UINT8)p->skincolor;
		}
	}

	// normal player colors
	if (server && (gametype == GT_MATCH || gametype == GT_TAG || gametype == GT_CTF) &&
		(p != &players[consoleplayer] && p != &players[secondarydisplayplayer]))
	{
		boolean kick = false;

		// team colors
		if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
		{
			if (p->ctfteam == 1 && p->skincolor != 6)
				kick = true;
			else if (p->ctfteam == 2 && p->skincolor != 7)
				kick = true;
		}

		// disallow the use of yellow in Match/Team Match/CTF
		if (gametype == GT_MATCH || gametype == GT_CTF)
		{
			if (p->skincolor == 15) //yellow
				kick = true;
		}

		// don't allow color "none"
		if (!p->skincolor)
			kick = true;

		if (kick)
		{
			XBOXSTATIC UINT8 buf[2];
			CONS_Printf(text[ILLEGALCOLORCMD], player_names[playernum], p->ctfteam, p->skincolor);

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
			return;
		}
	}

	str = (char *)*cp; // moving players cannot change skins
	SKIPSTRING(*cp);
	if ((P_PlayerMoving(playernum) && strcasecmp(skins[players[playernum].skin].name, str) != 0))
	{
		if (playernum == consoleplayer)
			CV_StealthSet(&cv_skin, skins[players[consoleplayer].skin].name);
		else if (splitscreen && playernum == secondarydisplayplayer)
			CV_StealthSet(&cv_skin2, skins[players[secondarydisplayplayer].skin].name);
		return;
	}

	// skin
	if (cv_forceskin.value >= 0 && (netgame || multiplayer)) // Server wants everyone to use the same player
	{
		const INT32 forcedskin = cv_forceskin.value;

		if (triggerforcedskin)
		{
			for (i = 0; i < MAXPLAYERS; i++)
			{
				if (playeringame[i])
				{
					SetPlayerSkinByNum(i, forcedskin);

					// If it's me (or my brother), set appropriate skin value in cv_skin/cv_skin2
					if (i == consoleplayer)
						CV_StealthSet(&cv_skin, skins[forcedskin].name);
					else if (i == secondarydisplayplayer)
						CV_StealthSet(&cv_skin2, skins[forcedskin].name);
				}
			}
			triggerforcedskin = false;
		}
		else
		{
			SetPlayerSkinByNum(playernum, forcedskin);

			if (playernum == consoleplayer)
				CV_StealthSet(&cv_skin, skins[forcedskin].name);
			else if (playernum == secondarydisplayplayer)
				CV_StealthSet(&cv_skin2, skins[forcedskin].name);
		}
	}
	else
	{
		SetPlayerSkin(playernum, str);
	}
}

static void SendWeaponPref(void)
{
	XBOXSTATIC SINT8 buf[1];

	buf[0] = (SINT8)cv_autoaim.value;
	SendNetXCmd(XD_WEAPONPREF, buf, 1);

	if (splitscreen)
	{
		buf[0] = (SINT8)cv_autoaim2.value;
		SendNetXCmd2(XD_WEAPONPREF, buf, 1);
	}
}

static void Got_WeaponPref(UINT8 **cp,INT32 playernum)
{
	if (READSINT8(*cp))
		players[playernum].pflags |= PF_AUTOAIM;
	else
		players[playernum].pflags &= ~PF_AUTOAIM;
}

void D_SendPlayerConfig(void)
{
	SendNameAndColor();
	if (splitscreen)
		SendNameAndColor2();
	SendWeaponPref();
}

static void Command_OrderPizza_f(void)
{
	if (COM_Argc() < 6 || COM_Argc() > 7)
	{
		CONS_Printf("%s", text[ORDERPIZZAHELP]);
		return;
	}

	SendNetXCmd(XD_ORDERPIZZA, NULL, 0);
}

static void Got_PizzaOrder(UINT8 **cp, INT32 playernum)
{
	cp = NULL;
	CONS_Printf(text[ORDEREDPIZZA], player_names[playernum]);
}

// Only works for displayplayer, sorry!
static void Command_ResetCamera_f(void)
{
	P_ResetCamera(&players[displayplayer], &camera);
}

static void Command_RTeleport_f(void)
{
	fixed_t intx, inty, intz;
	size_t i;
	player_t *p = &players[consoleplayer];
	subsector_t *ss;

	if (!(cv_debug || devparm))
	{
		CONS_Printf("%s", text[NEED_DEVMODE]);
		return;
	}

	if (COM_Argc() < 3 || COM_Argc() > 7)
	{
		CONS_Printf("%s", text[RTELEPORTHELP]);
		return;
	}

	if (netgame)
	{
		CONS_Printf("%s", text[NETGAME_TELEPORT]);
		return;
	}

	if (!p->mo)
	{
		CONS_Printf("%s", text[PDEAD_ETC]);
		return;
	}

	i = COM_CheckParm("-x");
	if (i)
		intx = atoi(COM_Argv(i + 1));
	else
		intx = 0;

	i = COM_CheckParm("-y");
	if (i)
		inty = atoi(COM_Argv(i + 1));
	else
		inty = 0;

	ss = R_PointInSubsector(p->mo->x + intx*FRACUNIT, p->mo->y + inty*FRACUNIT);
	if (!ss || ss->sector->ceilingheight - ss->sector->floorheight < p->mo->height)
	{
		CONS_Printf("%s", text[INVALIDLOCATION]);
		return;
	}
	i = COM_CheckParm("-z");
	if (i)
	{
		intz = atoi(COM_Argv(i + 1));
		intz <<= FRACBITS;
		intz += p->mo->z;
		if (intz < ss->sector->floorheight)
			intz = ss->sector->floorheight;
		if (intz > ss->sector->ceilingheight - p->mo->height)
			intz = ss->sector->ceilingheight - p->mo->height;
	}
	else
		intz = 0;

	CONS_Printf(text[TELEPORTINGBY], intx, inty, FixedInt((intz-p->mo->z)));

	P_MapStart();
	if (!P_TeleportMove(p->mo, p->mo->x+intx*FRACUNIT, p->mo->y+inty*FRACUNIT, intz))
		CONS_Printf("%s",text[UNABLE_TELEPORT]);
	else
		S_StartSound(p->mo, sfx_mixup);
	P_MapEnd();
}

static void Command_Teleport_f(void)
{
	fixed_t intx, inty, intz;
	size_t i;
	player_t *p = &players[consoleplayer];
	subsector_t *ss;

	if (!(cv_debug || devparm))
		return;

	if (COM_Argc() < 3 || COM_Argc() > 7)
	{
		CONS_Printf("%s", text[TELEPORT_HELP]);
		return;
	}

	if (netgame)
	{
		CONS_Printf("%s",text[NETGAME_TELEPORT]);
		return;
	}

	if (!p->mo)
	{
		CONS_Printf("%s",text[PDEAD_ETC]);
		return;
	}

	i = COM_CheckParm("-x");
	if (i)
		intx = atoi(COM_Argv(i + 1));
	else
	{
		CONS_Printf("%s", text[XNOTSPECIFIED]);
		return;
	}

	i = COM_CheckParm("-y");
	if (i)
		inty = atoi(COM_Argv(i + 1));
	else
	{
		CONS_Printf("%s", text[YNOTSPECIFIED]);
		return;
	}

	ss = R_PointInSubsector(intx*FRACUNIT, inty*FRACUNIT);
	if (!ss || ss->sector->ceilingheight - ss->sector->floorheight < p->mo->height)
	{
		CONS_Printf("%s",text[INVALID_LOCATION]);
		return;
	}
	i = COM_CheckParm("-z");
	if (i)
	{
		intz = atoi(COM_Argv(i + 1));
		intz <<= FRACBITS;
		if (intz < ss->sector->floorheight)
			intz = ss->sector->floorheight;
		if (intz > ss->sector->ceilingheight - p->mo->height)
			intz = ss->sector->ceilingheight - p->mo->height;
	}
	else
		intz = ss->sector->floorheight;

	CONS_Printf(text[TELEPORTINGTO], intx, inty, FixedInt(intz));

	P_MapStart();
	if (!P_TeleportMove(p->mo, intx*FRACUNIT, inty*FRACUNIT, intz))
		CONS_Printf("%s",text[UNABLE_TELEPORT]);
	else
		S_StartSound(p->mo, sfx_mixup);
	P_MapEnd();
}

// ========================================================================

// play a demo, add .lmp for external demos
// eg: playdemo demo1 plays the internal game demo
//
// UINT8 *demofile; // demo file buffer
static void Command_Playdemo_f(void)
{
	char name[256];

	if (COM_Argc() != 2)
	{
		CONS_Printf("%s", text[PLAYDEMO_HELP]);
		return;
	}

	// disconnect from server here?
	if (demoplayback)
		G_StopDemo();
	if (netgame)
	{
		CONS_Printf("%s",text[NETGAME_DEMO]);
		return;
	}

	// open the demo file
	strcpy(name, COM_Argv(1));
	// dont add .lmp so internal game demos can be played

	CONS_Printf(text[PLAYBACK_DEMO], name);

	G_DoPlayDemo(name);
}

static void Command_Timedemo_f(void)
{
	char name[256];

	if (COM_Argc() != 2)
	{
		CONS_Printf("%s", text[TIMEDEMO_HELP]);
		return;
	}

	// disconnect from server here?
	if (demoplayback)
		G_StopDemo();
	if (netgame)
	{
		CONS_Printf("%s",text[NETGAME_DEMO]);
		return;
	}

	// open the demo file
	strcpy (name, COM_Argv(1));
	// dont add .lmp so internal game demos can be played

	CONS_Printf(text[TIMING_DEMO], name);

	G_TimeDemo(name);
}

// stop current demo
static void Command_Stopdemo_f(void)
{
	G_CheckDemoStatus();
	CONS_Printf("%s", text[STOPPED_DEMO]);
}

static void Command_StartMovie_f(void)
{
	G_MovieMode(true);
}

static void Command_StopMovie_f(void)
{
	G_MovieMode(false);
}

INT32 mapchangepending = 0;

/** Runs a map change.
  * The supplied data are assumed to be good. If provided by a user, they will
  * have already been checked in Command_Map_f().
  *
  * Do \b NOT call this function directly from a menu! M_Responder() is called
  * from within the event processing loop, and this function calls
  * SV_SpawnServer(), which calls CL_ConnectToServer(), which gives you "Press
  * ESC to abort", which calls I_GetKey(), which adds an event. In other words,
  * 63 old events will get reexecuted, with ridiculous results. Just don't do
  * it (without setting delay to 1, which is the current solution).
  *
  * \param mapnum          Map number to change to.
  * \param gametype        Gametype to switch to.
  * \param pultmode        Is this 'Ultimate Mode'?
  * \param resetplayers    1 to reset player scores and lives and such, 0 not to.
  * \param delay           Determines how the function will be executed: 0 to do
  *                        it all right now (must not be done from a menu), 1 to
  *                        do step one and prepare step two, 2 to do step two.
  * \param skipprecutscene To skip the precutscence or not?
  * \sa D_GameTypeChanged, Command_Map_f
  * \author Graue <graue@oceanbase.org>
  */
void D_MapChange(INT32 mapnum, INT32 newgametype, boolean pultmode, INT32 resetplayers, INT32 delay, boolean skipprecutscene, boolean FLS)
{
	static char buf[MAX_WADPATH+1+5];
#define MAPNAME &buf[5]

	// The supplied data are assumed to be good.
	I_Assert(delay >= 0 && delay <= 2);

	if (devparm)
		CONS_Printf(text[MAPCHANGE_DEBUG], mapnum, newgametype, pultmode, resetplayers, delay, skipprecutscene);
	if (delay != 2)
	{
		const char *mapname = G_BuildMapName(mapnum);

		I_Assert(W_CheckNumForName(mapname) != LUMPERROR);

		strncpy(MAPNAME, mapname, MAX_WADPATH);

		buf[0] = (char)pultmode;

		// bit 0 doesn't currently do anything
		buf[1] = 0;

		if (!resetplayers)
			buf[1] |= 2;

		// new gametype value
		buf[2] = (char)newgametype;
	}

	if (delay == 1)
		mapchangepending = 1;
	else
	{
		mapchangepending = 0;
		// spawn the server if needed
		// reset players if there is a new one
		if (!(adminplayer == consoleplayer) && SV_SpawnServer())
			buf[1] &= ~2;

		chmappending++;
		if (server && netgame)
		{
			UINT8 seed = (UINT8)(totalplaytime % 256);
			SendNetXCmd(XD_RANDOMSEED, &seed, 1);
		}

		buf[3] = (char)skipprecutscene;

		if (netgame || multiplayer)
			FLS = false;

		buf[4] = (char)FLS;

		SendNetXCmd(XD_MAP, buf, 5+strlen(MAPNAME)+1);
	}
#undef MAPNAME
}

// Warp to map code.
// Called either from map <mapname> console command, or idclev cheat.
//
static void Command_Map_f(void)
{
	const char *mapname;
	size_t i;
	INT32 j, newmapnum, newgametype, newresetplayers;

	// max length of command: map map03 -gametype coop -noresetplayers -force
	//                         1    2       3       4         5           6
	// = 8 arg max
	if (COM_Argc() < 2 || COM_Argc() > 8)
	{
		CONS_Printf("%s", text[MAPCHANGE_HELP]);
		return;
	}

	if (!server && !(adminplayer == consoleplayer))
	{
		CONS_Printf("%s", text[SERVER_CHANGELEVEL]);
		return;
	}

	// internal wad lump always: map command doesn't support external files as in doom legacy
	if (W_CheckNumForName(COM_Argv(1)) == LUMPERROR)
	{
		CONS_Printf(text[LEVEL_NOTFOUND], COM_Argv(1));
		return;
	}

	if (!(netgame || multiplayer) && (!modifiedgame || savemoddata))
	{
		CONS_Printf("%s", text[NOLVLCHANGE]);
		return;
	}

	newresetplayers = !COM_CheckParm("-noresetplayers");

	if (!newresetplayers && !cv_debug)
	{
		CONS_Printf("%s",text[NEED_DEVMODE]);
		newresetplayers = true;
	}

	mapname = COM_Argv(1);
	if (strlen(mapname) != 5
	|| (newmapnum = M_MapNumber(mapname[3], mapname[4])) == 0)
	{
		CONS_Printf(text[INVALID_LEVELNAME], mapname);
		return;
	}

	// Ultimate Mode only in SP via menu
	if (netgame || multiplayer)
		ultimatemode = false;

	// new gametype value
	// use current one by default
	if (gametype == GT_RACE && cv_racetype.value)
		newgametype = GTF_CLASSICRACE;
	else if (gametype == GT_MATCH && cv_matchtype.value)
		newgametype = GTF_TEAMMATCH;
	else if (gametype == GT_TAG && cv_tagtype.value)
		newgametype = GTF_HIDEANDSEEK;
	else
		newgametype = gametype;


	i = COM_CheckParm("-gametype");
	if (i)
	{
		if (!multiplayer)
		{
			CONS_Printf("%s", text[NOGTCHANGE]);
			return;
		}

		for (j = 0; gametype_cons_t[j].strvalue; j++)
			if (!strcasecmp(gametype_cons_t[j].strvalue, COM_Argv(i+1)))
			{
				// Don't do any variable setting here. Wait until you get your
				// map packet first to avoid sending the same info twice! -Jazz
				if (gametype_cons_t[j].value == GT_MATCH)
					newgametype = GT_MATCH;
				else if (gametype_cons_t[j].value == GT_RACE)
					newgametype = GT_RACE;
				else if (gametype_cons_t[j].value == GT_TAG)
					newgametype = GT_TAG;

				if (gametype_cons_t[j].value == GTF_TEAMMATCH)
					newgametype = GTF_TEAMMATCH;
				else if (gametype_cons_t[j].value == GTF_CLASSICRACE)
					newgametype = GTF_CLASSICRACE;
				else if (gametype_cons_t[j].value == GTF_HIDEANDSEEK)
					newgametype = GTF_HIDEANDSEEK;
				else
					newgametype = gametype_cons_t[j].value;

				break;
			}

		if (!gametype_cons_t[j].strvalue) // reached end of the list with no match
		{
			// assume they gave us a gametype number, which is okay too
			for (j = 0; gametype_cons_t[j].strvalue != NULL; j++)
			{
				if (atoi(COM_Argv(i+1)) == gametype_cons_t[j].value)
				{
					newgametype = gametype_cons_t[j].value;
					break;
				}
			}
		}
	}

	// don't use a gametype the map doesn't support
	if (cv_debug || COM_CheckParm("-force") || cv_skipmapcheck.value)
		;
	else if (multiplayer)
	{
		INT16 tol = mapheaderinfo[newmapnum-1].typeoflevel, tolflag = 0;

		switch (newgametype)
		{
			case GT_MATCH: case GTF_TEAMMATCH: tolflag = TOL_MATCH; break;
#ifdef CHAOSISNOTDEADYET
			case GT_CHAOS: tolflag = TOL_CHAOS; break;
#endif
			case GT_COOP: tolflag = TOL_COOP; break;
			case GT_RACE: case GTF_CLASSICRACE: tolflag = TOL_RACE; break;
			case GT_CTF: tolflag = TOL_CTF; break;
			case GT_TAG: case GTF_HIDEANDSEEK: tolflag = TOL_TAG; break;
		}

		if (!(tol & tolflag))
		{
			char gametypestring[32];

			for (i = 0; gametype_cons_t[i].strvalue != NULL; i++)
			{
				if (gametype_cons_t[i].value == newgametype)
				{
					strcpy(gametypestring, gametype_cons_t[i].strvalue);
					break;
				}
			}

			CONS_Printf(text[GTNOTSUPPORTED], gametypestring);
			return;
		}
	}
	else if (!(mapheaderinfo[newmapnum-1].typeoflevel & TOL_SP))
	{
		CONS_Printf("%s", text[SPNOTSUPPORTED]);
		return;
	}

	fromlevelselect = false;
	D_MapChange(newmapnum, newgametype, false, newresetplayers, 0, false, false);
}

/** Receives a map command and changes the map.
  *
  * \param cp        Data buffer.
  * \param playernum Player number responsible for the message. Should be
  *                  ::serverplayer or ::adminplayer.
  * \sa D_MapChange
  */
static void Got_Mapcmd(UINT8 **cp, INT32 playernum)
{
	char mapname[MAX_WADPATH+1];
	INT32 resetplayer = 1, lastgametype;
	UINT8 skipprecutscene, FLS;

	if (playernum != serverplayer && playernum != adminplayer)
	{
		CONS_Printf(text[ILLEGALMAPCMD], player_names[playernum]);
		if (server)
		{
			XBOXSTATIC UINT8 buf[2];

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		return;
	}

	if (chmappending)
		chmappending--;

	ultimatemode = READUINT8(*cp);

	if (netgame || multiplayer)
		ultimatemode = false;

	resetplayer = ((READUINT8(*cp) & 2) == 0);

	lastgametype = gametype;
	gametype = READUINT8(*cp);

	// Base Gametypes
	if (gametype == GT_MATCH)
	{
		if (server)
			CV_SetValue(&cv_matchtype, 0);
	}
	else if (gametype == GT_RACE)
	{
		if (server)
			CV_SetValue(&cv_racetype, 0);
	}
	else if (gametype == GT_TAG)
	{
		if (server)
			CV_SetValue(&cv_tagtype, 0);
	}
	// Special Cases
	else if (gametype == GTF_TEAMMATCH)
	{
		gametype = GT_MATCH;

		if (server)
			CV_SetValue(&cv_matchtype, 1);
	}
	else if (gametype == GTF_CLASSICRACE)
	{
		gametype = GT_RACE;

		if (server)
			CV_SetValue(&cv_racetype, 1);
	}
	else if (gametype == GTF_HIDEANDSEEK)
	{
		gametype = GT_TAG;

		if (server)
			CV_SetValue(&cv_tagtype, 1);
	}

	if (gametype != lastgametype)
		D_GameTypeChanged(lastgametype); // emulate consvar_t behavior for gametype

	skipprecutscene = READUINT8(*cp);

	FLS = READUINT8(*cp);

	READSTRINGN(*cp, mapname, MAX_WADPATH);

	if (!skipprecutscene)
	{
		DEBFILE(va("Warping to %s [resetplayer=%d lastgametype=%d gametype=%d cpnd=%d]\n",
			mapname, resetplayer, lastgametype, gametype, chmappending));
		CONS_Printf(text[STSTR_CLEV], devparm ? mapname:"level");
	}
	if (demoplayback && !timingdemo)
		precache = false;

	if (resetplayer)
	{
		if (!FLS || (netgame || multiplayer))
			emeralds = 0;
	}

	// why here? because, this is only called the first time a level is loaded.
	// also, this needs to be done before the level is loaded, duh :p
	mapmusic = mapheaderinfo[gamemap-1].musicslot;

	G_InitNew(ultimatemode, mapname, resetplayer, skipprecutscene);
	if (demoplayback && !timingdemo)
		precache = true;
	CON_ToggleOff();
	if (timingdemo)
		G_DoneLevelLoad();

	if (timeattacking)
	{
		SetPlayerSkinByNum(0, cv_chooseskin.value-1);
		players[0].skincolor = (atoi(skins[cv_chooseskin.value-1].prefcolor)) % MAXSKINCOLORS;
		CV_StealthSetValue(&cv_playercolor, players[0].skincolor);

		// a copy of color
		if (players[0].mo)
		{
			players[0].mo->flags |= MF_TRANSLATION;
			players[0].mo->color = (UINT8)players[0].skincolor;
		}
	}
}

static void Command_Pause(void)
{
	XBOXSTATIC UINT8 buf[2];
	UINT8 *cp = buf;

	if (COM_Argc() > 1)
		WRITEUINT8(cp, (char)(atoi(COM_Argv(1)) != 0));
	else
		WRITEUINT8(cp, (char)(!paused));

	if (dedicated)
		WRITEUINT8(cp, 1);
	else
		WRITEUINT8(cp, 0);

	if (cv_pause.value || server || (adminplayer == consoleplayer))
	{
		if (!(gamestate == GS_LEVEL || gamestate == GS_INTERMISSION))
		{
			CONS_Printf("%s",text[PAUSEINFO]);
			return;
		}
		SendNetXCmd(XD_PAUSE, &buf, 2);
	}
	else
		CONS_Printf("%s",text[SERVERPAUSE]);
}

static void Got_Pause(UINT8 **cp, INT32 playernum)
{
	UINT8 dedicatedpause = false;
	const char *playername;

	if (netgame && !cv_pause.value && playernum != serverplayer && playernum != adminplayer)
	{
		CONS_Printf(text[ILLEGALPAUSECMD], player_names[playernum]);
		if (server)
		{
			XBOXSTATIC UINT8 buf[2];

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		return;
	}

	paused = READUINT8(*cp);
	dedicatedpause = READUINT8(*cp);

	if (!demoplayback)
	{
		if (netgame)
		{
			if (dedicatedpause)
				playername = "SERVER";
			else
				playername = player_names[playernum];

			if (paused)
				CONS_Printf(text[GAME_PAUSED],playername);
			else
				CONS_Printf(text[GAME_UNPAUSED],playername);
		}

		if (paused)
		{
			if (!menuactive || netgame)
				S_PauseSound();
		}
		else
			S_ResumeSound();
	}
}

/** Deals with an ::XD_RANDOMSEED message in a netgame.
  * These messages set the position of the random number LUT and are crucial to
  * correct synchronization.
  *
  * Such a message should only ever come from the ::serverplayer. If it comes
  * from any other player, it is ignored.
  *
  * \param cp        Data buffer.
  * \param playernum Player responsible for the message. Must be ::serverplayer.
  * \author Graue <graue@oceanbase.org>
  */
static void Got_RandomSeed(UINT8 **cp, INT32 playernum)
{
	UINT8 seed;

	seed = READUINT8(*cp);
	if (playernum != serverplayer) // it's not from the server, wtf?
		return;

	P_SetRandIndex(seed);
}

/** Clears all players' scores in a netgame.
  * Only the server or a remote admin can use this command, for obvious reasons.
  *
  * \sa XD_CLEARSCORES, Got_Clearscores
  * \author SSNTails <http://www.ssntails.org>
  */
static void Command_Clearscores_f(void)
{
	if (!(server || (adminplayer == consoleplayer)))
		return;

	SendNetXCmd(XD_CLEARSCORES, NULL, 1);
}

/** Handles an ::XD_CLEARSCORES message, which resets all players' scores in a
  * netgame to zero.
  *
  * \param cp        Data buffer.
  * \param playernum Player responsible for the message. Must be ::serverplayer
  *                  or ::adminplayer.
  * \sa XD_CLEARSCORES, Command_Clearscores_f
  * \author SSNTails <http://www.ssntails.org>
  */
static void Got_Clearscores(UINT8 **cp, INT32 playernum)
{
	INT32 i;

	(void)cp;
	if (playernum != serverplayer && playernum != adminplayer)
	{
		CONS_Printf(text[ILLEGALCLRSCRCMD], player_names[playernum]);
		if (server)
		{
			XBOXSTATIC UINT8 buf[2];

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		return;
	}

	for (i = 0; i < MAXPLAYERS; i++)
		players[i].score = 0;

	CONS_Printf("%s", text[SCORESRESET]);
}

// Team changing functions
static void Command_Teamchange_f(void)
{
	changeteam_union NetPacket;
	boolean error = false;
	UINT16 usvalue;
	NetPacket.value.l = NetPacket.value.b = 0;

	//      0         1
	// changeteam  <color>

	if (COM_Argc() <= 1)
	{
		if ((gametype == GT_MATCH && !cv_matchtype.value) || gametype == GT_TAG)
			CONS_Printf("%s", text[CHANGETEAM_HELP1]);
		else if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
			CONS_Printf("%s", text[CHANGETEAM_HELP2]);
		else
			CONS_Printf("%s", text[NOMTF]);
		return;
	}

	if ((gametype == GT_MATCH && !cv_matchtype.value) || gametype == GT_TAG)
	{
		if (!strcasecmp(COM_Argv(1), "spectator") || !strcasecmp(COM_Argv(1), "0"))
			NetPacket.packet.newteam = 0;
		else if (!strcasecmp(COM_Argv(1), "playing") || !strcasecmp(COM_Argv(1), "1"))
			NetPacket.packet.newteam = 3;
		else
			error = true;
	}
	else if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
	{
		if (!strcasecmp(COM_Argv(1), "red") || !strcasecmp(COM_Argv(1), "1"))
			NetPacket.packet.newteam = 1;
		else if (!strcasecmp(COM_Argv(1), "blue") || !strcasecmp(COM_Argv(1), "2"))
			NetPacket.packet.newteam = 2;
		else if (!strcasecmp(COM_Argv(1), "spectator") || !strcasecmp(COM_Argv(1), "0"))
			NetPacket.packet.newteam = 0;
		else
			error = true;
	}
	else
	{
		CONS_Printf("%s", text[NOMTF]);
		return;
	}

	if (error)
	{
		if ((gametype == GT_MATCH && !cv_matchtype.value) || gametype == GT_TAG)
			CONS_Printf("%s", text[CHANGETEAM_HELP1]);
		else if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
			CONS_Printf("%s", text[CHANGETEAM_HELP2]);
		return;
	}

	if ((gametype == GT_MATCH && !cv_matchtype.value) || gametype == GT_TAG)
	{
		if ((players[consoleplayer].spectator && !NetPacket.packet.newteam) ||
			(!players[consoleplayer].spectator && NetPacket.packet.newteam == 3))
			error = true;
	}
	else if ((gametype == GT_MATCH && cv_matchtype.value) || gametype == GT_CTF)
	{
		if (NetPacket.packet.newteam == (unsigned)players[consoleplayer].ctfteam ||
			(players[consoleplayer].spectator && !NetPacket.packet.newteam))
			error = true;
	}
	else
		I_Error("Invalid gametype after initial checks!");

	if (error)
	{
		CONS_Printf("%s",text[ALREADYONTEAM]);
		return;
	}

	if (!cv_allowteamchange.value && !NetPacket.packet.newteam) // allow swapping to spectator even in locked teams.
	{
		CONS_Printf("%s",text[NOTEAMCHANGE]);
		return;
	}

	//additional check for hide and seek. Don't allow change of status after hidetime ends.
	if (gametype == GT_TAG && cv_tagtype.value && leveltime >= (hidetime * TICRATE))
	{
		if (NetPacket.packet.newteam)
		{
			CONS_Printf("%s", text[NO_TAGCHANGE]);
			return;
		}
	}

	usvalue = SHORT(NetPacket.value.l|NetPacket.value.b);
	SendNetXCmd(XD_TEAMCHANGE, &usvalue, sizeof(usvalue));
}

static void Command_Teamchange2_f(void)
{
	changeteam_union NetPacket;
	boolean error = false;
	UINT16 usvalue;
	NetPacket.value.l = NetPacket.value.b = 0;

	//      0         1
	// changeteam2 <color>

	if (COM_Argc() <= 1)
	{
		if ((gametype == GT_MATCH && !cv_matchtype.value) || gametype == GT_TAG)
			CONS_Printf("%s", text[CHANGETEAM2_HELP1]);
		else if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
			CONS_Printf("%s", text[CHANGETEAM2_HELP2]);
		else
			CONS_Printf("%s", text[NOMTF]);
		return;
	}

	if ((gametype == GT_MATCH && !cv_matchtype.value) || gametype == GT_TAG)
	{
		if (!strcasecmp(COM_Argv(1), "spectator") || !strcasecmp(COM_Argv(1), "0"))
			NetPacket.packet.newteam = 0;
		else if (!strcasecmp(COM_Argv(1), "playing") || !strcasecmp(COM_Argv(1), "1"))
			NetPacket.packet.newteam = 3;
		else
			error = true;
	}
	else if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
	{
		if (!strcasecmp(COM_Argv(1), "red") || !strcasecmp(COM_Argv(1), "1"))
			NetPacket.packet.newteam = 1;
		else if (!strcasecmp(COM_Argv(1), "blue") || !strcasecmp(COM_Argv(1), "2"))
			NetPacket.packet.newteam = 2;
		else if (!strcasecmp(COM_Argv(1), "spectator") || !strcasecmp(COM_Argv(1), "0"))
			NetPacket.packet.newteam = 0;
		else
			error = true;
	}
	else
	{
		CONS_Printf("%s", text[NOMTF]);
		return;
	}

	if (error)
	{
		if ((gametype == GT_MATCH && !cv_matchtype.value) || gametype == GT_TAG)
			CONS_Printf("%s", text[CHANGETEAM2_HELP1]);
		else if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
			CONS_Printf("%s", text[CHANGETEAM2_HELP2]);
		return;
	}

	if ((gametype == GT_MATCH && !cv_matchtype.value) || gametype == GT_TAG)
	{
		if ((players[secondarydisplayplayer].spectator && !NetPacket.packet.newteam) ||
			(!players[secondarydisplayplayer].spectator && NetPacket.packet.newteam == 3))
			error = true;
	}
	else if ((gametype == GT_MATCH && cv_matchtype.value) || gametype == GT_CTF)
	{
		if (NetPacket.packet.newteam == (unsigned)players[secondarydisplayplayer].ctfteam ||
			(players[secondarydisplayplayer].spectator && !NetPacket.packet.newteam))
			error = true;
	}
	else
		I_Error("Invalid gametype after initial checks!");

	if (error)
	{
		CONS_Printf("%s",text[ALREADYONTEAM]);
		return;
	}

	if (!cv_allowteamchange.value && !NetPacket.packet.newteam) // allow swapping to spectator even in locked teams.
	{
		CONS_Printf("%s",text[NOTEAMCHANGE]);
		return;
	}

	//additional check for hide and seek. Don't allow change of status after hidetime ends.
	if (gametype == GT_TAG && cv_tagtype.value && leveltime >= (hidetime * TICRATE))
	{
		if (NetPacket.packet.newteam)
		{
			CONS_Printf("%s", text[NO_TAGCHANGE]);
			return;
		}
	}

	usvalue = SHORT(NetPacket.value.l|NetPacket.value.b);
	SendNetXCmd2(XD_TEAMCHANGE, &usvalue, sizeof(usvalue));
}

static void Command_ServerTeamChange_f(void)
{
	changeteam_union NetPacket;
	boolean error = false;
	UINT16 usvalue;
	NetPacket.value.l = NetPacket.value.b = 0;

	if (!(server || (adminplayer == consoleplayer)))
	{
		CONS_Printf("%s", text[SERVER_CHANGETEAM]);
		return;
	}

	//        0              1         2
	// serverchangeteam <playernum>  <team>

	if (COM_Argc() < 3)
	{
		if (gametype == GT_MATCH && !cv_matchtype.value)
			CONS_Printf("%s", text[SERVERCHANGETEAM_HELP1]);
		else if (gametype == GT_TAG)
			CONS_Printf("%s", text[SERVERCHANGETEAM_HELP2]);
		else if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
			CONS_Printf("%s", text[SERVERCHANGETEAM_HELP3]);
		else
			CONS_Printf("%s", text[NOMTF]);
		return;
	}

	if (gametype == GT_MATCH && !cv_matchtype.value)
	{
		if (!strcasecmp(COM_Argv(2), "spectator") || !strcasecmp(COM_Argv(2), "0"))
			NetPacket.packet.newteam = 0;
		else if (!strcasecmp(COM_Argv(2), "playing") || !strcasecmp(COM_Argv(2), "1"))
			NetPacket.packet.newteam = 3;
		else
			error = true;
	}
	else if (gametype == GT_TAG)
	{
		if (!strcasecmp(COM_Argv(2), "it") || !strcasecmp(COM_Argv(2), "1"))
			NetPacket.packet.newteam = 1;
		else if (!strcasecmp(COM_Argv(2), "notit") || !strcasecmp(COM_Argv(2), "2"))
			NetPacket.packet.newteam = 2;
		else if (!strcasecmp(COM_Argv(2), "playing") || !strcasecmp(COM_Argv(2), "3"))
			NetPacket.packet.newteam = 3;
		else if (!strcasecmp(COM_Argv(2), "spectator") || !strcasecmp(COM_Argv(2), "0"))
			NetPacket.packet.newteam = 0;
		else
			error = true;
	}
	else if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
	{
		if (!strcasecmp(COM_Argv(2), "red") || !strcasecmp(COM_Argv(2), "1"))
			NetPacket.packet.newteam = 1;
		else if (!strcasecmp(COM_Argv(2), "blue") || !strcasecmp(COM_Argv(2), "2"))
			NetPacket.packet.newteam = 2;
		else if (!strcasecmp(COM_Argv(2), "spectator") || !strcasecmp(COM_Argv(2), "0"))
			NetPacket.packet.newteam = 0;
		else
			error = true;
	}
	else
	{
		CONS_Printf("%s", text[NOMTF]);
		return;
	}

	if (error)
	{
		if (gametype == GT_MATCH && !cv_matchtype.value)
			CONS_Printf("%s", text[SERVERCHANGETEAM_HELP1]);
		else if (gametype == GT_TAG)
			CONS_Printf("%s", text[SERVERCHANGETEAM_HELP2]);
		else if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
			CONS_Printf("%s", text[SERVERCHANGETEAM_HELP3]);
		return;
	}

	NetPacket.packet.playernum = atoi(COM_Argv(1));

	if (gametype == GT_MATCH && !cv_matchtype.value)
	{
		if ((players[NetPacket.packet.playernum].spectator && !NetPacket.packet.newteam) ||
			(!players[NetPacket.packet.playernum].spectator && NetPacket.packet.newteam == 3))
			error = true;
	}
	else if (gametype == GT_TAG)
	{
		if (((players[NetPacket.packet.playernum].pflags & PF_TAGIT) && NetPacket.packet.newteam == 1) ||
			(!(players[NetPacket.packet.playernum].pflags & PF_TAGIT) && NetPacket.packet.newteam == 2) ||
			(players[NetPacket.packet.playernum].spectator && !NetPacket.packet.newteam) ||
			(!players[NetPacket.packet.playernum].spectator && NetPacket.packet.newteam == 3))
			error = true;
	}
	else if ((gametype == GT_MATCH && cv_matchtype.value) || gametype == GT_CTF)
	{
		if (NetPacket.packet.newteam == (unsigned)players[NetPacket.packet.playernum].ctfteam ||
			(players[NetPacket.packet.playernum].spectator && !NetPacket.packet.newteam))
			error = true;
	}
	else
		I_Error("Invalid gametype after initial checks!");

	if (error)
	{
		CONS_Printf("%s", text[PLAYER_ONTEAM]);
		return;
	}

	//additional check for hide and seek. Don't allow change of status after hidetime ends.
	if (gametype == GT_TAG && cv_tagtype.value && leveltime >= (hidetime * TICRATE))
	{
		if (NetPacket.packet.newteam)
		{
			CONS_Printf("%s", text[NO_TAGCHANGE]);
			return;
		}
	}

	NetPacket.packet.verification = true; // This signals that it's a server change

	usvalue = SHORT(NetPacket.value.l|NetPacket.value.b);
	SendNetXCmd(XD_TEAMCHANGE, &usvalue, sizeof(usvalue));
}

//todo: This and the other teamchange functions are getting too long and messy. Needs cleaning.
static void Got_Teamchange(UINT8 **cp, INT32 playernum)
{
	changeteam_union NetPacket;
	boolean error = false;
	NetPacket.value.l = NetPacket.value.b = READINT16(*cp);

	if (!(gametype == GT_MATCH || gametype == GT_TAG || gametype == GT_CTF)) //Make sure you're in the right gametype.
	{
		// this should never happen unless the client is hacked/buggy
		CONS_Printf(text[ILLEGALTEAMCHANGECMD], player_names[playernum]);
		if (server)
		{
			XBOXSTATIC UINT8 buf[2];

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
		}
	}

	if (NetPacket.packet.verification) // Special marker that the server sent the request
	{
		if (playernum != serverplayer && (playernum != adminplayer))
		{
			CONS_Printf(text[ILLEGALTEAMCHANGECMD], player_names[playernum]);
			if (server)
			{
				XBOXSTATIC UINT8 buf[2];

				buf[0] = (UINT8)playernum;
				buf[1] = KICK_MSG_CON_FAIL;
				SendNetXCmd(XD_KICK, &buf, 2);
			}
			return;
		}
		playernum = NetPacket.packet.playernum;
	}

	// Prevent multiple changes in one go.
	if (gametype == GT_MATCH && !cv_matchtype.value)
	{
		if ((players[playernum].spectator && !NetPacket.packet.newteam) ||
			(!players[playernum].spectator && NetPacket.packet.newteam == 3))
			return;
	}
	else if (gametype == GT_TAG)
	{
		if (((players[playernum].pflags & PF_TAGIT) && NetPacket.packet.newteam == 1) ||
			(!(players[playernum].pflags & PF_TAGIT) && NetPacket.packet.newteam == 2) ||
			(players[playernum].spectator && NetPacket.packet.newteam == 0) ||
			(!players[playernum].spectator && NetPacket.packet.newteam == 3))
			return;
	}
	else if ((gametype == GT_MATCH && cv_matchtype.value) || gametype == GT_CTF)
	{
		if ((NetPacket.packet.newteam && (NetPacket.packet.newteam == (unsigned)players[playernum].ctfteam)) ||
			(players[playernum].spectator && !NetPacket.packet.newteam))
			return;
	}
	else
	{
		if (playernum != serverplayer && (playernum != adminplayer))
		{
			CONS_Printf(text[ILLEGALTEAMCHANGECMD], player_names[playernum]);
			if (server)
			{
				XBOXSTATIC UINT8 buf[2];

				buf[0] = (UINT8)playernum;
				buf[1] = KICK_MSG_CON_FAIL;
				SendNetXCmd(XD_KICK, &buf, 2);
			}
		}
		return;
	}

	//Make sure that the right team number is sent. Keep in mind that normal clients cannot change to certain teams in certain gametypes.
	switch (gametype)
	{
	case GT_MATCH: case GT_CTF:
		if (!cv_allowteamchange.value)
		{
			if (!NetPacket.packet.verification && NetPacket.packet.newteam)
				error = true; //Only admin can change status, unless changing to spectator.
		}
		break; //Otherwise, you don't need special permissions.
	case GT_TAG:
		switch (NetPacket.packet.newteam)
		{
		case 0:
			break;
		case 1: case 2:
			if (!NetPacket.packet.verification || leveltime >= (hidetime * TICRATE)) //no status changes after hidetime
				error = true; //Only admin can change player's IT status' in tag.
			break;
		case 3: //Join game via console.
			//no status changes after hidetime in hide and seek.
			if (!cv_allowteamchange.value || (cv_tagtype.value && (leveltime >= (hidetime * TICRATE))))
				error = true;
			break;
		}

		break;
	default:
		I_Error("Invalid gametype after initial checks!");
	}

	if (server && ((NetPacket.packet.newteam < 0 || NetPacket.packet.newteam > 3) || error))
	{
		XBOXSTATIC UINT8 buf[2];

		buf[0] = (UINT8)playernum;
		buf[1] = KICK_MSG_CON_FAIL;
		CONS_Printf(text[SENTILLEGALTEAMCHANGE], player_names[playernum], NetPacket.packet.newteam);
		SendNetXCmd(XD_KICK, &buf, 2);
	}

	//Safety first!
	//Mega hack. P_DamageMobj needs cleaning, badly.
	players[playernum].spectator = true;
	if (players[playernum].mo)
		P_DamageMobj(players[playernum].mo, NULL, NULL, 42000);

	//Now that we've done our error checking and killed the player
	//if necessary, put the player on the correct team/status.
	if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
	{
		if (!NetPacket.packet.newteam)
		{
			players[playernum].ctfteam = 0;
			players[playernum].spectator = true;
		}
		else
		{
			players[playernum].ctfteam = NetPacket.packet.newteam;
			players[playernum].spectator = false;
		}
	}
	else if (gametype == GT_MATCH && !cv_matchtype.value)
	{
		if (!NetPacket.packet.newteam)
			players[playernum].spectator = true;
		else
			players[playernum].spectator = false;
	}
	else if (gametype == GT_TAG)
	{
		if (!NetPacket.packet.newteam)
		{
			players[playernum].spectator = true;
			players[playernum].pflags &= ~PF_TAGIT;
			players[playernum].pflags &= ~PF_TAGGED;
		}
		else if (NetPacket.packet.newteam != 3) // .newteam == 1 or 2.
		{
			players[playernum].spectator = false;
			players[playernum].pflags &= ~PF_TAGGED;//Just in case.

			if (NetPacket.packet.newteam == 1) //Make the player IT.
				players[playernum].pflags |= PF_TAGIT;
			else
				players[playernum].pflags &= ~PF_TAGIT;
		}
		else // Just join the game.
		{
			players[playernum].spectator = false;

			//If joining after hidetime in normal tag, default to being IT.
			if (!cv_tagtype.value && (leveltime > (hidetime * TICRATE)))
			{
				NetPacket.packet.newteam = 1; //minor hack, causes the "is it" message to be printed later.
				players[playernum].pflags |= PF_TAGIT; //make the player IT.
			}
		}
	}

	if (NetPacket.packet.autobalance)
		CONS_Printf(text[AUTOBALANCE_SWITCH], player_names[playernum]);
	else if (NetPacket.packet.scrambled)
		CONS_Printf(text[SCRAMBLE_SWITCH], player_names[playernum]);
	else if (NetPacket.packet.newteam == 1)
	{
		if (gametype == GT_TAG)
			CONS_Printf(text[NOW_IT], player_names[playernum]);
		else
			CONS_Printf(text[REDTEAM_SWITCH], player_names[playernum]);
	}
	else if (NetPacket.packet.newteam == 2)
	{
		if (gametype == GT_TAG)
			CONS_Printf(text[NO_LONGER_IT], player_names[playernum]);
		else
			CONS_Printf(text[BLUETEAM_SWITCH], player_names[playernum]);
	}
	else if (NetPacket.packet.newteam == 3)
		CONS_Printf(text[INGAME_SWITCH], player_names[playernum]);
	else
		CONS_Printf(text[SPECTATOR_SWITCH], player_names[playernum]);

	//reset view if you are changed, or viewing someone who was changed.
	if (playernum == consoleplayer || displayplayer == playernum)
		displayplayer = consoleplayer;

	if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
	{
		if (NetPacket.packet.newteam)
		{
			if (playernum == consoleplayer) //CTF and Team Match colors.
				CV_SetValue(&cv_playercolor, NetPacket.packet.newteam + 5);
			else if (playernum == secondarydisplayplayer)
				CV_SetValue(&cv_playercolor2, NetPacket.packet.newteam + 5);
		}
	}

	// Clear player score and rings if a spectator.
	if (players[playernum].spectator)
	{
		players[playernum].score = 0;
		players[playernum].health = 1;
		if (players[playernum].mo)
			players[playernum].mo->health = 1;
	}

	// In tag, check to see if you still have a game.
	if (gametype == GT_TAG)
		P_CheckSurvivors();
}

// Remote Administration
static void Command_Changepassword_f(void)
{
	if (!server) // cannot change remotely
	{
		CONS_Printf("%s", text[SERVER_CHANGEPASSWORD]);
		return;
	}

	if (COM_Argc() != 2)
	{
		CONS_Printf("%s", text[PASSWORD_HELP]);
		return;
	}

	strncpy(adminpassword, COM_Argv(1), 8);

	// Pad the password
	if (strlen(COM_Argv(1)) < 8)
	{
		size_t i;
		for (i = strlen(COM_Argv(1)); i < 8; i++)
			adminpassword[i] = 'a';
	}
}

static void Command_Login_f(void)
{
	XBOXSTATIC char password[9];

	// If the server uses login, it will effectively just remove admin privileges
	// from whoever has them. This is good.

	if (COM_Argc() != 2)
	{
		CONS_Printf("%s", text[LOGIN_HELP]);
		return;
	}

	strncpy(password, COM_Argv(1), 8);

	// Pad the password
	if (strlen(COM_Argv(1)) < 8)
	{
		size_t i;
		for (i = strlen(COM_Argv(1)); i < 8; i++)
			password[i] = 'a';
	}

	password[8] = '\0';

	CONS_Printf(text[SENDING_LOGIN], password);

	SendNetXCmd(XD_LOGIN, password, 9);
}

static void Got_Login(UINT8 **cp, INT32 playernum)
{
	char compareword[9];

	READMEM(*cp, compareword, 9);

	if (!server)
		return;

	compareword[8] = '\0';

	if (!strcmp(compareword, adminpassword))
	{
		CONS_Printf(text[PASSED_AUTH], player_names[playernum], compareword);
		COM_BufInsertText(va("verify %d\n", playernum)); // do this immediately
	}
	else
		CONS_Printf(text[PASSWORD_FAILED], playernum, compareword);
}

static void Command_Verify_f(void)
{
	XBOXSTATIC char buf[8]; // Should be plenty
	char *temp;
	INT32 playernum;

	if (!server)
	{
		CONS_Printf("%s", text[SERVER_VERIFY]);
		return;
	}

	if (COM_Argc() != 2)
	{
		CONS_Printf("%s", text[VERIFY_HELP]);
		return;
	}

	strlcpy(buf, COM_Argv(1), sizeof (buf));

	playernum = atoi(buf);

	temp = buf;

	WRITEUINT8(temp, playernum);

	if (playeringame[playernum])
		SendNetXCmd(XD_VERIFIED, buf, 1);
}

static void Got_Verification(UINT8 **cp, INT32 playernum)
{
	INT16 num = READUINT8(*cp);

	if (playernum != serverplayer) // it's not from the server (hacker or bug)
	{
		CONS_Printf(text[ILLEGALVERIFYCMD], player_names[playernum], player_names[serverplayer]);
		if (server)
		{
			XBOXSTATIC UINT8 buf[2];

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		return;
	}

	adminplayer = num;

	if (num != consoleplayer)
		return;

	CONS_Printf("%s", text[PASSWORD_CORRECT]);
}

static void Command_MotD_f(void)
{
	size_t i, j;
	XBOXSTATIC char mymotd[sizeof(motd)];

	if ((j = COM_Argc()) < 2)
	{
		CONS_Printf("%s", text[MOTD_HELP]);
		return;
	}

	if (!(server || (adminplayer == consoleplayer)))
	{
		CONS_Printf("%s", text[SERVERONLY]);
		return;
	}

	strlcpy(mymotd, COM_Argv(1), sizeof mymotd);
	for (i = 2; i < j; i++)
	{
		strlcat(mymotd, " ", sizeof mymotd);
		strlcat(mymotd, COM_Argv(i), sizeof mymotd);
	}

	// Disallow non-printing characters and semicolons.
	for (i = 0; mymotd[i] != '\0'; i++)
		if (!isprint(mymotd[i]) || mymotd[i] == ';')
			return;

	if ((netgame || multiplayer) && !server)
		SendNetXCmd(XD_SETMOTD, mymotd, sizeof(mymotd));
	else
	{
		strcpy(motd, mymotd);
		CONS_Printf("%s", text[MOTD_SET]);
	}
}

static void Got_MotD_f(UINT8 **cp, INT32 playernum)
{
	XBOXSTATIC char mymotd[sizeof(motd)];
	INT32 i;
	boolean kick = false;

	READSTRINGN(*cp, mymotd, sizeof(mymotd));

	// Disallow non-printing characters and semicolons.
	for (i = 0; mymotd[i] != '\0'; i++)
		if (!isprint(mymotd[i]) || mymotd[i] == ';')
			kick = true;

	if ((playernum != serverplayer && playernum != adminplayer) || kick)
	{
		CONS_Printf(text[ILLEGALMOTDCMD], player_names[playernum]);
		if (server)
		{
			XBOXSTATIC UINT8 buf[2];

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		return;
	}

	strcpy(motd, mymotd);

	CONS_Printf("%s", text[MOTD_SET]);
}

static void Command_RunSOC(void)
{
	const char *fn;
	XBOXSTATIC char buf[255];
	size_t length = 0;

	if (COM_Argc() != 2)
	{
		CONS_Printf("%s", text[RUNSOC_HELP]);
		return;
	}
	else
		fn = COM_Argv(1);

	if (netgame && !(server || consoleplayer == adminplayer))
	{
		CONS_Printf("%s", text[SERVERONLY]);
		return;
	}

	if (!(netgame || multiplayer))
	{
		if (!P_RunSOC(fn))
			CONS_Printf("Could not find SOC.\n");
		else if (!modifiedgame)
		{
			modifiedgame = true;
			CONS_Printf("%s", text[GAMEMODIFIED]);
		}
		return;
	}

	nameonly(strcpy(buf, fn));
	length = strlen(buf)+1;

	SendNetXCmd(XD_RUNSOC, buf, length);
}

static void Got_RunSOCcmd(UINT8 **cp, INT32 playernum)
{
	char filename[256];
	filestatus_t ncs = FS_NOTFOUND;

	if (playernum != serverplayer && playernum != adminplayer)
	{
		CONS_Printf(text[ILLEGALRUNSOCCMD], player_names[playernum]);
		if (server)
		{
			XBOXSTATIC UINT8 buf[2];

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		return;
	}

	READSTRINGN(*cp, filename, 255);

	// Maybe add md5 support?
	if (strstr(filename, ".soc") != NULL)
	{
		ncs = findfile(filename,NULL,true);

		if (ncs != FS_FOUND)
		{
			Command_ExitGame_f();
			if (ncs == FS_NOTFOUND)
			{
				CONS_Printf(text[CLIENT_NEEDFILE], filename);
				M_StartMessage(va("The server added a file\n(%s)\nthat you do not have.\n\nPress ESC\n",filename), NULL, MM_NOTHING);
			}
			else
			{
				CONS_Printf(text[SOC_NOTFOUND], filename);
				M_StartMessage(va("Unknown error trying to load a file\nthat the server added\n(%s).\n\nPress ESC\n",filename), NULL, MM_NOTHING);
			}
			return;
		}
	}

	P_RunSOC(filename);
	modifiedgame = true;
}

/** Adds a pwad at runtime.
  * Searches for sounds, maps, music, new images.
  */
static void Command_Addfile(void)
{
	const char *fn;
	XBOXSTATIC char buf[255];
	size_t length;
	INT32 i;

	if (COM_Argc() != 2)
	{
		CONS_Printf("%s", text[ADDFILE_HELP]);
		return;
	}
	else
		fn = COM_Argv(1);

	// Disallow non-printing characters and semicolons.
	for (i = 0; fn[i] != '\0'; i++)
		if (!isprint(fn[i]) || fn[i] == ';')
			return;

	if (!W_VerifyNMUSlumps(fn))
	{
		// ... But only so long as they contain nothing more then music and sprites.
		if (netgame && !(server || adminplayer == consoleplayer))
		{
			CONS_Printf("%s", text[SERVERONLY]);
			return;
		}
		if (!(netgame || multiplayer) && !modifiedgame) // Only announce modifiedgame ONCE.
			CONS_Printf("%s", text[GAMEMODIFIED]);
		modifiedgame = true;
	}

	// Add file on your client directly if it is trivial, or you aren't in a netgame.
	if (!(netgame || multiplayer) || W_VerifyNMUSlumps(fn))
	{
		P_AddWadFile(fn, NULL);
		return;
	}

	nameonly(strcpy(buf, fn));
	length = strlen(buf)+1;

	{
		UINT8 md5sum[16+1] = {'\0'};
#ifndef NOMD5
		FILE *fhandle;

		fhandle = fopen(fn, "rb");

		if (fhandle)
		{
			tic_t t = I_GetTime();
#ifdef _arch_dreamcast
			CONS_Printf("Making MD5 for %s\n",fn);
#endif
			md5_stream(fhandle, md5sum);
#ifndef _arch_dreamcast
			if (devparm)
#endif
				CONS_Printf("MD5 calc for %s took %f second\n",
					fn, (float)(I_GetTime() - t)/TICRATE);
			fclose(fhandle);
		}
		else
		{
			CONS_Printf("%s", text[FILE_NOT_FOUND]);
			return;
		}
#endif
		M_Memcpy(&buf[length+1], md5sum, sizeof (md5sum));
		length += sizeof (md5sum);
	}

	if (adminplayer == consoleplayer) // Request to add file
		SendNetXCmd(XD_REQADDFILE, buf, length);
	else
		SendNetXCmd(XD_ADDFILE, buf, length);
}

#ifdef DELFILE
/** removes the last added pwad at runtime.
  * Searches for sounds, maps, music and images to remove
  */
static void Command_Delfile(void)
{
	if (gamestate == GS_LEVEL)
	{
		CONS_Printf("%s", text[NEED_NO_LEVEL]);
		return;
	}

	if (netgame && !(server || adminplayer))
	{
		CONS_Printf("%s", text[SERVERONLY]);
		return;
	}

	if (!(netgame || multiplayer) && mainwads != numwadfiles)
	{
		P_DelWadFile();
		if (mainwads == numwadfiles && modifiedgame)
		{
			modifiedgame = false;
		}
		return;
	}

	SendNetXCmd(XD_DELFILE, NULL, 0);
}
#endif

static void Got_RequestAddfilecmd(UINT8 **cp, INT32 playernum)
{
	char filename[256];
	filestatus_t ncs = FS_NOTFOUND;
	UINT8 md5sum[16+1];
	boolean kick = false;
	INT32 i;

	READSTRINGN(*cp, filename, 255);
	(void)READUINT8(*cp);
	READMEM(*cp, md5sum, 17);

	// Only the server processes this message.
	if (!server)
		return;

	// Disallow non-printing characters and semicolons.
	for (i = 0; filename[i] != '\0'; i++)
		if (!isprint(filename[i]) || filename[i] == ';')
			kick = true;

	if ((playernum != serverplayer && playernum != adminplayer) || kick)
	{
		XBOXSTATIC UINT8 buf[2];

		CONS_Printf(text[ILLEGALADDFILECMD], player_names[playernum]);

		buf[0] = (UINT8)playernum;
		buf[1] = KICK_MSG_CON_FAIL;
		SendNetXCmd(XD_KICK, &buf, 2);
		return;
	}

	ncs = findfile(filename,md5sum,true);

	if (ncs != FS_FOUND)
	{
		char message[256];

		if (ncs == FS_NOTFOUND)
			sprintf(message, "The server doesn't have %s\n", filename);
		else if (ncs == FS_MD5SUMBAD)
			sprintf(message, "Checksum mismatch on %s\n", filename);
		else
			sprintf(message, "Unknown error finding wad file (%s)\n", filename);

		CONS_Printf("%s",message);

		if (adminplayer)
			COM_BufAddText(va("sayto %d %s", adminplayer, message));

		return;
	}

	COM_BufAddText(va("addfile %s\n", filename));
}

#ifdef DELFILE
static void Got_Delfilecmd(UINT8 **cp, INT32 playernum)
{
	if (playernum != serverplayer && playernum != adminplayer)
	{
		CONS_Printf(text[ILLEGALDELFILECMD], player_names[playernum]);
		if (server)
		{
			XBOXSTATIC UINT8 buf[2];

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		return;
	}
	(void)cp;

	P_DelWadFile();
	if (mainwads == numwadfiles && modifiedgame)
	{
		modifiedgame = false;
	}
}
#endif

static void Got_Addfilecmd(UINT8 **cp, INT32 playernum)
{
	char filename[256];
	filestatus_t ncs = FS_NOTFOUND;
	UINT8 md5sum[16+1];

	READSTRINGN(*cp, filename, 255);
	(void)READUINT8(*cp);
	READMEM(*cp, md5sum, 17);

	if (playernum != serverplayer)
	{
		CONS_Printf(text[ILLEGALADDFILECMD], player_names[playernum]);
		if (server)
		{
			XBOXSTATIC UINT8 buf[2];

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		return;
	}

	ncs = findfile(filename,md5sum,true);

	if (ncs != FS_FOUND)
	{
		Command_ExitGame_f();
		if (ncs == FS_NOTFOUND)
		{
			CONS_Printf(text[CLIENT_NEEDFILE], filename);
			M_StartMessage(va("The server added a file \n(%s)\nthat you do not have.\n\nPress ESC\n",filename), NULL, MM_NOTHING);
		}
		else if (ncs == FS_MD5SUMBAD)
		{
			CONS_Printf(text[CHECKSUM_MISMATCH], filename);
			M_StartMessage(va("Checksum mismatch while loading \n%s.\nThe server seems to have a\ndifferent version of this file.\n\nPress ESC\n",filename), NULL, MM_NOTHING);
		}
		else
		{
			CONS_Printf(text[WAD_NOTFOUND], filename);
			M_StartMessage(va("Unknown error trying to load a file\nthat the server added \n(%s).\n\nPress ESC\n",filename), NULL, MM_NOTHING);
		}
		return;
	}

	P_AddWadFile(filename, NULL);
	modifiedgame = true;
}

static void Command_ListWADS_f(void)
{
	INT32 i = numwadfiles;
	char *tempname;
	CONS_Printf(text[NUMWADSLOADED],numwadfiles);
	for (i--; i; i--)
	{
		nameonly(tempname = va("%s", wadfiles[i]->filename));
		if (i > mainwads)
			CONS_Printf(text[LISTWAD1], i, tempname);
		else
			CONS_Printf(text[LISTWAD2], i, tempname);
	}
	CONS_Printf(text[LISTIWAD], wadfiles[0]->filename);
}

// =========================================================================
//                            MISC. COMMANDS
// =========================================================================

/** Prints program version.
  */
static void Command_Version_f(void)
{
	CONS_Printf(text[VERSIONCMD], VERSIONSTRING, compdate, comptime, comprevision);
}

#ifdef UPDATE_ALERT
static void Command_ModDetails_f(void)
{
	CONS_Printf(text[MODDETAILSCMD], MODID, MODVERSION, CODEBASE);
}
#endif
// Returns current gametype being used.
//
static void Command_ShowGametype_f(void)
{
	CONS_Printf(text[GAMETYPECMD], gametype);
}

// Moves the NiGHTS player to another axis within the current mare
// Only for development purposes.
//
static void Command_JumpToAxis_f(void)
{
	if (!cv_debug)
		CONS_Printf("%s",text[NEED_DEVMODE]);

	if (COM_Argc() != 2)
	{
		CONS_Printf("%s",text[JUMPTOAXIS_HELP]);
		return;
	}

	P_TransferToAxis(&players[consoleplayer], atoi(COM_Argv(1)));
}

/** Plays the intro.
  */
static void Command_Playintro_f(void)
{
	if (netgame)
		return;

	F_StartIntro();
}

/** Writes the mapthings in the current level to a file.
  *
  * \sa cv_objectplace
  */
static void Command_Writethings_f(void)
{
	P_WriteThings(W_GetNumForName(G_BuildMapName(gamemap)) + ML_THINGS);
}

/** Quits the game immediately.
  */
FUNCNORETURN static ATTRNORETURN void Command_Quit_f(void)
{
	I_Quit();
}

void ObjectPlace_OnChange(void)
{
	if (gamestate != GS_LEVEL && cv_objectplace.value)
	{
		CONS_Printf("%s",text[MUSTBEINLEVEL]);
		CV_StealthSetValue(&cv_objectplace, false);
		return;
	}
#ifndef JOHNNYFUNCODE
	if ((netgame || multiplayer) && cv_objectplace.value) // You spoon!
	{
		CV_StealthSetValue(&cv_objectplace, 0);
		CONS_Printf("%s",text[CANTUSEMULTIPLAYER]);
		return;
	}
#else
	if (cv_objectplace.value)
	{
		INT32 i;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i])
				continue;

			if (!players[i].mo)
				continue;

			if (players[i].playerstate != PST_LIVE)
				continue;

			if (players[i].pflags & PF_NIGHTSMODE)
				continue;

			P_SetTarget(&players[i].mo->target, NULL);
			players[i].mo->flags2 |= MF2_DONTDRAW;
			players[i].mo->flags |= MF_NOCLIP;
			players[i].mo->flags |= MF_NOGRAVITY;
			P_UnsetThingPosition(players[i].mo);
			players[i].mo->flags |= MF_NOBLOCKMAP;
			P_SetThingPosition(players[i].mo);
			if (!players[i].currentthing)
				players[i].currentthing = 1;
			if (!modifiedgame || savemoddata)
			{
				modifiedgame = true;
				savemoddata = false;
				if (!(netgame || multiplayer))
					CONS_Printf("%s",GAMEMODIFIED);
			}
		}
	}
	else if (players[0].mo)
	{
		INT32 i;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i])
				continue;

			if (!players[i].mo)
				continue;

			if (players[i].playerstate != PST_LIVE)
				continue;

			if (!(players[i].pflags & PF_NIGHTSMODE))
			{
				if (players[i].mo->target)
				{
					P_RemoveMobj(players[i].mo->target);
					P_SetTarget(&players[i].mo->target, NULL);
				}

				players[i].mo->flags2 &= ~MF2_DONTDRAW;
				players[i].mo->flags &= ~MF_NOGRAVITY;
			}

			players[i].mo->flags &= ~MF_NOCLIP;
			P_UnsetThingPosition(players[i].mo);
			players[i].mo->flags &= ~MF_NOBLOCKMAP;
			P_SetThingPosition(players[i].mo);
		}
	}
	return;
#endif

	if (cv_objectplace.value)
	{
		if (!modifiedgame || savemoddata)
		{
			modifiedgame = true;
			savemoddata = false;
			if (!(netgame || multiplayer))
				CONS_Printf("%s",text[GAMEMODIFIED]);
		}

		HU_SetCEchoFlags(1048576);
		HU_SetCEchoDuration(10);
		HU_DoCEcho(text[OBJPLACEINFO]);
		HU_SetCEchoDuration(5);
		HU_SetCEchoFlags(0);

		if ((players[0].pflags & PF_NIGHTSMODE))
			return;

		players[0].mo->flags2 |= MF2_DONTDRAW;
		players[0].mo->flags |= MF_NOCLIP;
		players[0].mo->flags |= MF_NOGRAVITY;
		P_UnsetThingPosition(players[0].mo);
		players[0].mo->flags |= MF_NOBLOCKMAP;
		P_SetThingPosition(players[0].mo);
		if (!players[0].currentthing)
			players[0].currentthing = 1;
		players[0].mo->momx = players[0].mo->momy = players[0].mo->momz = 0;
	}
	else if (players[0].mo)
	{
		if (!(players[0].pflags & PF_NIGHTSMODE))
		{
			if (players[0].mo->target)
				P_SetMobjState(players[0].mo->target, S_DISS);

			players[0].mo->flags2 &= ~MF2_DONTDRAW;
			players[0].mo->flags &= ~MF_NOGRAVITY;
		}

		players[0].mo->flags &= ~MF_NOCLIP;
		P_UnsetThingPosition(players[0].mo);
		players[0].mo->flags &= ~MF_NOBLOCKMAP;
		P_SetThingPosition(players[0].mo);
	}
}

/** Deals with a pointlimit change by printing the change to the console.
  * If the gametype is single player, cooperative, or race, the pointlimit is
  * silently disabled again.
  *
  * Timelimit and pointlimit can be used at the same time.
  *
  * We don't check immediately for the pointlimit having been reached,
  * because you would get "caught" when turning it up in the menu.
  * \sa cv_pointlimit, TimeLimit_OnChange
  * \author Graue <graue@oceanbase.org>
  */
static void PointLimit_OnChange(void)
{
	// Don't allow pointlimit in Single Player/Co-Op/Race!
	if (server && (gametype == GT_COOP || gametype == GT_RACE))
	{
		if (cv_pointlimit.value)
			CV_StealthSetValue(&cv_pointlimit, 0);
		return;
	}

	if (cv_pointlimit.value)
	{
		CONS_Printf(text[POINTLIMIT_MESSAGE],
			cv_matchtype.value == 1 || gametype == GT_CTF
				? text[A_TEAM] : text[SOMEONE],
			cv_pointlimit.value,
			cv_pointlimit.value > 1 ? "s" : "");
	}
	else if (netgame || multiplayer)
		CONS_Printf("%s", text[POINTLIMIT_DISABLED]);
}

static void NumLaps_OnChange(void)
{
	if (gametype != GT_RACE)
		return; // Just don't be verbose

	CONS_Printf(text[NUMLAPS_MESSAGE], cv_numlaps.value);
}

static void NetTimeout_OnChange(void)
{
	connectiontimeout = (tic_t)cv_nettimeout.value;
}

UINT32 timelimitintics = 0;

/** Deals with a timelimit change by printing the change to the console.
  * If the gametype is single player, cooperative, or race, the timelimit is
  * silently disabled again.
  *
  * Timelimit and pointlimit can be used at the same time.
  *
  * \sa cv_timelimit, PointLimit_OnChange
  */
static void TimeLimit_OnChange(void)
{
	// Don't allow timelimit in Single Player/Co-Op/Race!
	if (server && cv_timelimit.value != 0
		&& (gametype == GT_COOP || gametype == GT_RACE))
	{
		CV_SetValue(&cv_timelimit, 0);
		return;
	}

	if (cv_timelimit.value != 0)
	{
		CONS_Printf(text[TIMELIMIT_MESSAGE],cv_timelimit.value,cv_timelimit.value == 1 ? "" : "s"); // Graue 11-17-2003
		timelimitintics = cv_timelimit.value * 60 * TICRATE;

		//add hidetime for tag too!
		if (gametype == GT_TAG)
			timelimitintics += hidetime * TICRATE;

		// Note the deliberate absence of any code preventing
		//   pointlimit and timelimit from being set simultaneously.
		// Some people might like to use them together. It works.
	}
	else if (netgame || multiplayer)
		CONS_Printf("%s", text[TIMELIMIT_DISABLED]);
}

/** Adjusts certain settings to match a changed gametype.
  *
  * \param lastgametype The gametype we were playing before now.
  * \sa D_MapChange
  * \author Graue <graue@oceanbase.org>
  * \todo Get rid of the hardcoded stuff, ugly stuff, etc.
  */
void D_GameTypeChanged(INT32 lastgametype)
{
	if (netgame)
	{
		INT32 j;
		const char *oldgt = NULL, *newgt = NULL;
		for (j = 0; gametype_cons_t[j].strvalue; j++)
		{
			if (gametype_cons_t[j].value == lastgametype)
				oldgt = gametype_cons_t[j].strvalue;
			if (gametype_cons_t[j].value == gametype)
				newgt = gametype_cons_t[j].strvalue;
		}
		if (oldgt && newgt)
			CONS_Printf(text[GAMETYPE_CHANGED], oldgt, newgt);
	}
	// Only do the following as the server, not as remote admin.
	// There will always be a server, and this only needs to be done once.
	if (server && (multiplayer || netgame))
	{
		if (gametype == GT_MATCH || gametype == GT_TAG || gametype == GT_CTF
#ifdef CHAOSISNOTDEADYET
			|| gametype == GT_CHAOS
#endif
			)
		{
			CV_SetValue(&cv_itemrespawn, 1);
		}
		else
			CV_SetValue(&cv_itemrespawn, 0);

		switch (gametype)
		{
#ifdef CHAOSISNOTDEADYET
			case GT_CHAOS:
				if (!cv_timelimit.changed && !cv_pointlimit.changed) // user hasn't changed limits
				{
					// default settings for chaos: timelimit 2 mins, no pointlimit
					CV_SetValue(&cv_pointlimit, 0);
					CV_SetValue(&cv_timelimit, 2);
				}
				if (!cv_itemrespawntime.changed)
					CV_SetValue(&cv_itemrespawntime, 90); // respawn sparingly in chaos
				break;
#endif
			case GT_MATCH:
				if (!cv_timelimit.changed && !cv_pointlimit.changed) // user hasn't changed limits
				{
					// default settings for match: timelimit 5 mins, no pointlimit
					CV_SetValue(&cv_pointlimit, 0);
					CV_SetValue(&cv_timelimit, 5);
				}
				if (!cv_itemrespawntime.changed)
					CV_Set(&cv_itemrespawntime, cv_itemrespawntime.defaultvalue); // respawn normally
				break;
			case GT_TAG:
				if (!cv_timelimit.changed && !cv_pointlimit.changed) // user hasn't changed limits
				{
					// default settings for tag: 3 mins, no pointlimit
					// Note that tag mode also uses an alternate timing mechanism in tandem with timelimit.
					CV_SetValue(&cv_timelimit, 3);
					CV_SetValue(&cv_pointlimit, 0);
				}
				if (!cv_itemrespawntime.changed)
					CV_Set(&cv_itemrespawntime, cv_itemrespawntime.defaultvalue); // respawn normally
				break;
			case GT_CTF:
				if (!cv_timelimit.changed && !cv_pointlimit.changed) // user hasn't changed limits
				{
					// default settings for CTF: no timelimit, pointlimit 5
					CV_SetValue(&cv_timelimit, 0);
					CV_SetValue(&cv_pointlimit, 5);
				}
				if (!cv_itemrespawntime.changed)
					CV_Set(&cv_itemrespawntime, cv_itemrespawntime.defaultvalue); // respawn normally
				break;
		}
	}
	else if (!multiplayer && !netgame)
	{
		gametype = GT_COOP;
		CV_Set(&cv_itemrespawntime, cv_itemrespawntime.defaultvalue);
		CV_SetValue(&cv_itemrespawn, 0);
	}

	if (server && gametype != GT_MATCH)
		CV_SetValue(&cv_matchtype, 0);

	if (server && gametype != GT_TAG)
		CV_SetValue(&cv_tagtype, 0);

	// reset timelimit and pointlimit in race/coop, prevent stupid cheats
	if (server && (gametype == GT_RACE || gametype == GT_COOP))
	{
		if (cv_timelimit.value)
			CV_SetValue(&cv_timelimit, 0);
		if (cv_pointlimit.value)
			CV_SetValue(&cv_pointlimit, 0);
	}

	if ((cv_pointlimit.changed || cv_timelimit.changed) && cv_pointlimit.value)
	{
		if ((
#ifdef CHAOSISNOTDEADYET
			lastgametype == GT_CHAOS ||
#endif
			lastgametype == GT_MATCH ||
			lastgametype == GT_TAG) &&
			gametype == GT_CTF)
			CV_SetValue(&cv_pointlimit, cv_pointlimit.value / 500);
		else if (lastgametype == GT_CTF &&
			(
#ifdef CHAOSISNOTDEADYET
			gametype == GT_CHAOS ||
#endif
			gametype == GT_MATCH ||
			gametype == GT_TAG))
			CV_SetValue(&cv_pointlimit, cv_pointlimit.value * 500);
	}

	// When swapping to a gametype that supports spectators,
	// make everyone a spectator initially.
	if (gametype == GT_CTF || gametype == GT_MATCH || gametype == GT_TAG)
	{
		INT32 i;
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i])
			{
				players[i].ctfteam = 0;
				players[i].spectator = true;
			}
	}

	// don't retain teams in other modes or between changes from ctf to team match.
	// also, stop any and all forms of team scrambling that might otherwise take place.
	if (lastgametype == GT_CTF || lastgametype == GT_MATCH)
	{
		INT32 i;
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i])
				players[i].ctfteam = 0;

		if (server || (adminplayer == consoleplayer))
		{
			CV_StealthSetValue(&cv_teamscramble, 0);
			teamscramble = 0;
		}
	}

	// make sure no players retain the color yellow if swapping to match or CTF.
	// todo: This block is very unwieldy. Make a way for the server to force changing of color. -Jazz
	if (gametype == GT_CTF || gametype == GT_MATCH)
	{
		INT32 i;
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && players[i].skincolor == 15)
			{
				players[i].skincolor = players[i].prefcolor;
				if (splitscreen && (i == secondarydisplayplayer))
					SendNameAndColor2();
				else
				{
					if (P_IsLocalPlayer(&players[i]))
						SendNameAndColor();
				}
			}
		}

		// if an unscrupulous player deletes the above code block,
		// the server will catch them and kick them from the netgame.
		if (server)
		{
			for (i = 0; i < MAXPLAYERS; i++)
			{
				if (players[i].skincolor == 15)
				{
					XBOXSTATIC UINT8 buf[2];
					CONS_Printf(text[ILLEGALCOLORCMD], player_names[i], players[i].ctfteam, players[i].skincolor);

					buf[0] = (UINT8)i;
					buf[1] = KICK_MSG_CON_FAIL;
					SendNetXCmd(XD_KICK, &buf, 2);
				}
			}
		}
	}
}

static void Ringslinger_OnChange(void)
{
	// If you've got a grade less than 3, you can't use this.
	if ((grade&7) < 3 && !netgame && cv_ringslinger.value && !cv_debug)
	{
		CONS_Printf("%s", text[NOTEARNED]);
		CV_StealthSetValue(&cv_ringslinger, 0);
		return;
	}

	if (cv_ringslinger.value) // Only if it's been turned on
	{
		if (!modifiedgame || savemoddata)
		{
			modifiedgame = true;
			savemoddata = false;
			if (!(netgame || multiplayer))
				CONS_Printf("%s", text[GAMEMODIFIED]);
		}
	}
}

static void Setrings_OnChange(void)
{
	if ((grade&7) < 5 && !netgame && cv_setrings.value && !cv_debug)
	{
		CONS_Printf("%s", text[NOTEARNED]);
		CV_StealthSetValue(&cv_setrings, 0);
		return;
	}

	if (cv_setrings.value)
	{
		INT32 i;
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i] && players[i].mo)
			{
				players[i].mo->health = cv_setrings.value + 1;
				players[i].health = players[i].mo->health;
				players[i].losscount = 0;
			}

		if (!modifiedgame || savemoddata)
		{
			modifiedgame = true;
			savemoddata = false;
			if (!(netgame || multiplayer))
				CONS_Printf("%s", text[GAMEMODIFIED]);
		}

		if (adminplayer == consoleplayer || server)
			CV_StealthSetValue(&cv_setrings, 0);
	}
}

static void Setlives_OnChange(void)
{
	if ((grade&7) < 4 && !netgame && cv_setlives.value && !cv_debug)
	{
		CONS_Printf("%s", text[NOTEARNED]);
		CV_StealthSetValue(&cv_setlives, 0);
		return;
	}

	if (cv_setlives.value)
	{
		INT32 i;
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i])
			{
				players[i].lives = 0;
				P_GivePlayerLives(&players[i], cv_setlives.value);
			}

		if (!modifiedgame || savemoddata)
		{
			modifiedgame = true;
			savemoddata = false;
			if (!(netgame || multiplayer))
				CONS_Printf("%s", text[GAMEMODIFIED]);
		}

		if (adminplayer == consoleplayer || server)
			CV_StealthSetValue(&cv_setlives, 0);
	}
}

static void Setcontinues_OnChange(void)
{
	if ((grade&7) < 4 && !netgame && cv_setcontinues.value && !cv_debug)
	{
		CONS_Printf("%s", text[NOTEARNED]);
		CV_StealthSetValue(&cv_setcontinues, 0);
		return;
	}

	if (cv_setcontinues.value)
	{
		INT32 i;
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i])
				players[i].continues = cv_setcontinues.value;

		if (!modifiedgame || savemoddata)
		{
			modifiedgame = true;
			savemoddata = false;
			if (!(netgame || multiplayer))
				CONS_Printf("%s", text[GAMEMODIFIED]);
		}

		if (adminplayer == consoleplayer || server)
			CV_StealthSetValue(&cv_setcontinues, 0);
	}
}

static void Gravity_OnChange(void)
{
	if ((grade&7) < 2 && !netgame
		&& strcmp(cv_gravity.string, cv_gravity.defaultvalue))
	{
		CONS_Printf("%s", text[NOTEARNED]);
		CV_StealthSet(&cv_gravity, cv_gravity.defaultvalue);
		return;
	}

	if(netgame)
	{
		CV_StealthSet(&cv_gravity, cv_gravity.defaultvalue);
		return;
	}

	gravity = cv_gravity.value;
}

static void SoundTest_OnChange(void)
{
	if (cv_soundtest.value < 0)
	{
		CV_SetValue(&cv_soundtest, NUMSFX-1);
		return;
	}

	if (cv_soundtest.value >= NUMSFX)
	{
		CV_SetValue(&cv_soundtest, 0);
		return;
	}

	S_StopSounds();
	S_StartSound(NULL, cv_soundtest.value);
}

static void AutoBalance_OnChange(void)
{
	autobalance = (INT16)cv_autobalance.value;
}

static void TeamScramble_OnChange(void)
{
	INT16 i = 0, j = 0, playercount = 0;
	boolean repick = true;
	INT32 blue = 0, red = 0;
	INT32 maxcomposition = 0;
	INT16 newteam = 0;
	INT32 retries = 0;
	boolean success = false;

	// Don't trigger outside level or intermission!
	if (!(gamestate == GS_LEVEL || gamestate == GS_INTERMISSION))
		return;

	if (!cv_teamscramble.value)
		teamscramble = 0;

	if (((gametype != GT_MATCH && !cv_matchtype.value) && gametype != GT_CTF) &&
		(server || (consoleplayer == adminplayer)))
	{
		CONS_Printf("%s", text[NOTMCTF]);
		CV_StealthSetValue(&cv_teamscramble, 0);
		return;
	}

	// If a team scramble is already in progress, do not allow another one to be started!
	if (teamscramble)
		return;

retryscramble:

	// Clear related global variables. These will get used again in p_tick.c/y_inter.c as the teams are scrambled.
	memset(&scrambleplayers, 0, sizeof(scrambleplayers));
	memset(&scrambleteams, 0, sizeof(scrambleplayers));
	scrambletotal = scramblecount = 0;
	blue = red = maxcomposition = newteam = playercount = 0;
	repick = true;

	// Put each player's node in the array.
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i] && !players[i].spectator)
		{
			scrambleplayers[playercount] = i;
			playercount++;
		}
	}

	if (playercount < 2)
	{
		CV_StealthSetValue(&cv_teamscramble, 0);
		return; // Don't scramble one or zero players.
	}

	// Randomly place players on teams.
	if (cv_teamscramble.value == 1)
	{
		maxcomposition = playercount / 2;

		// Now randomly assign players to teams.
		// If the teams get out of hand, assign the rest to the other team.
		for (i = 0; i < playercount; i++)
		{
			if (repick)
				newteam = (INT16)((M_Random() % 2) + 1);

			// One team has the most players they can get, assign the rest to the other team.
			if (red == maxcomposition || blue == maxcomposition)
			{
				if (red == maxcomposition)
					newteam = 2;
				else if (blue == maxcomposition)
					newteam = 1;

				repick = false;
			}

			scrambleteams[i] = newteam;

			if (newteam == 1)
				red++;
			else
				blue++;
		}
	}
	else if (cv_teamscramble.value == 2) // Same as before, except split teams based on current score.
	{
		// Now, sort the array based on points scored.
		for (i = 1; i < playercount; i++)
		{
			for (j = i; j < playercount; j++)
			{
				INT16 tempplayer = 0;

				if ((players[scrambleplayers[i-1]].score > players[scrambleplayers[j]].score))
				{
					tempplayer = scrambleplayers[i-1];
					scrambleplayers[i-1] = scrambleplayers[j];
					scrambleplayers[j] = tempplayer;
				}
			}
		}

		// Now assign players to teams based on score. Scramble in pairs.
		// If there is an odd number, one team will end up with the unlucky slob who has no points. =(
		for (i = 0; i < playercount; i++)
		{
			if (repick)
			{
				newteam = (INT16)((M_Random() % 2) + 1);
				repick = false;
			}
			else
			{
				// We will only randomly pick the team for the first guy.
				// Otherwise, just alternate back and forth, distributing players.
				if (newteam == 1)
					newteam = 2;
				else
					newteam = 1;
			}

			scrambleteams[i] = newteam;
		}
	}

	// Check to see if our random selection actually
	// changed anybody. If not, we run through and try again.
	for (i = 0; i < playercount; i++)
	{
		if (players[scrambleplayers[i]].ctfteam != scrambleteams[i])
			success = true;
	}

	if (!success && retries < 5)
	{
		retries++;
		goto retryscramble; //try again
	}

	// Display a witty message, but only during scrambles specifically triggered by an admin.
	if (cv_teamscramble.value)
	{
		scrambletotal = playercount;
		teamscramble = (INT16)cv_teamscramble.value;

		if (!(gamestate == GS_INTERMISSION && cv_scrambleonchange.value))
			CONS_Printf("%s", text[TEAMS_SCRAMBLED]);
	}
}

static void Cheats_OnChange(void)
{
	if (cheats && (netgame || multiplayer) && !cv_cheats.value)
	{
		CONS_Printf("%s", text[CANNOT_CHANGE_CHEATS]);
		CV_StealthSetValue(&cv_cheats, 1);
		return;
	}

	if (gamestate == GS_LEVEL && !(netgame || multiplayer) && cv_cheats.value)
	{
		CONS_Printf("%s", text[CANTUSESINGLEPLAYER]);
		CV_StealthSetValue(&cv_cheats, 0);
		return;
	}

	// Automatically disable cheats when playing single player.
	if (gamestate == GS_WAITINGPLAYERS && !(netgame || multiplayer))
		CV_StealthSetValue(&cv_cheats, 0);

	// Display console and hud message.
	if (cv_cheats.value && !cheats)
	{
		HU_DoCEcho(va("%s", text[CHEATS_ACTIVATED]));
		I_OutputMsg("%s", text[CHEATS_ACTIVATED]);
	}

	// When deactivated, restore all variables governed by cheats to their starting values.
	if (!cv_cheats.value && cheats)
		CV_ResetCheatNetVars();

	cheats = cv_cheats.value;
}

static void Tagtype_OnChange(void)
{
	INT32 i, j;

	// Do not execute the below code unless absolutely necessary.
	if (gametype != GT_TAG || gamestate != GS_LEVEL || cv_tagtype.value == tagtype)
		return;

	// Changing from normal tag to hide and seek.
	// Pick the highest scoring IT player to remain it,
	// the rest become frozen as though they were tagged.
	if (cv_tagtype.value)
	{
		INT32 tempplayer;
		INT32 playerarray[MAXPLAYERS];
		INT32 playercount = 0;

		//Store the nodes of all participating players in an array.
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && players[i].pflags & PF_TAGIT)
			{
				playerarray[playercount] = i;
				playercount++;
			}
		}

		//Sort 'em.
		for (i = 1; i < playercount; i++)
		{
			for (j = i; j < playercount; j++)
			{
				if (players[playerarray[i-1]].score < players[playerarray[j]].score)
				{
					tempplayer = playerarray[i-1];
					playerarray[i-1] = playerarray[j];
					playerarray[j] = tempplayer;
				}
			}
		}

		//Top IT player remains it, the rest become frozen.
		for (i = 1; i < playercount; i++) //start at 1 since 0 is the top score.
		{
			players[playerarray[i]].pflags &= ~PF_TAGIT;
			players[playerarray[i]].pflags |= PF_TAGGED;
			players[playerarray[i]].pflags |= PF_STASIS;
		}
	}
	else
	{
		//When going from hide and seek to normal tag,
		//Make the tagged players IT and let them move.
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && players[i].pflags & PF_TAGGED)
			{
				players[i].pflags |= PF_TAGIT;
				players[i].pflags &= ~PF_TAGGED;
				players[i].pflags &= ~PF_STASIS;
			}
		}
	}

	//Better safe than sorry.
	P_CheckSurvivors();

	tagtype = cv_tagtype.value;
}

static void Hidetime_OnChange(void)
{
	if ((gamestate == GS_LEVEL && gametype == GT_TAG) && ((cv_timelimit.value * 60) <= cv_hidetime.value))
	{
		CONS_Printf("%s", text[HIDETIME_ERROR]);
		CV_StealthSetValue(&cv_hidetime, hidetime);
		return;
	}
	hidetime = cv_hidetime.value;

	//uh oh, gotta change timelimitintics now too
	if (gametype == GT_TAG)
		timelimitintics = (cv_timelimit.value * 60 * TICRATE) + (hidetime * TICRATE);
}

static void Command_Showmap_f(void)
{
	if (gamestate == GS_LEVEL)
	{
		if (mapheaderinfo[gamemap-1].actnum)
			CONS_Printf(text[SHOWMAP1], G_BuildMapName(gamemap), gamemap, mapheaderinfo[gamemap-1].lvlttl, mapheaderinfo[gamemap-1].actnum);
		else
			CONS_Printf(text[SHOWMAP2], G_BuildMapName(gamemap), gamemap, mapheaderinfo[gamemap-1].lvlttl);
	}
	else
		CONS_Printf("%s",text[MUSTBEINLEVEL]);
}

static void Command_ExitLevel_f(void)
{
	if (!(netgame || (multiplayer && gametype != GT_COOP)) && !cv_debug)
	{
		CONS_Printf("%s",text[CANTUSESINGLEPLAYER]);
		return;
	}
	if (!(server || (adminplayer == consoleplayer)))
	{
		CONS_Printf("%s",text[SERVERONLY]);
		return;
	}
	if (gamestate != GS_LEVEL || demoplayback)
		CONS_Printf("%s",text[MUSTBEINLEVEL]);

	SendNetXCmd(XD_EXITLEVEL, NULL, 0);
}

static void Got_ExitLevelcmd(UINT8 **cp, INT32 playernum)
{
	cp = NULL;

	// Ignore duplicate XD_EXITLEVEL commands.
	if (gameaction == ga_completed)
		return;

	if (playernum != serverplayer && playernum != adminplayer)
	{
		CONS_Printf(text[ILLEGALEXITLVLCMD], player_names[playernum]);
		if (server)
		{
			XBOXSTATIC UINT8 buf[2];

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		return;
	}

	G_ExitLevel();
}

/** Prints the number of the displayplayer.
  *
  * \todo Possibly remove this; it was useful for debugging at one point.
  */
static void Command_Displayplayer_f(void)
{
	CONS_Printf(text[DISPLAYPLAYERCMD], displayplayer);
}

static void Command_Skynum_f(void)
{
	if (!cv_debug)
	{
		CONS_Printf("%s",text[NEED_DEVMODE]);
		CONS_Printf("%s",text[CHANGESKY_HELP]);
		return;
	}

	if (COM_Argc() != 2)
	{
		CONS_Printf("%s",text[SKYNUM_HELP]);
		return;
	}

	if (netgame || multiplayer)
	{
		CONS_Printf("%s",text[CANTUSEMULTIPLAYER]);
		return;
	}

	CONS_Printf(text[SKYNUM_PREVIEW], COM_Argv(1));

	P_SetupLevelSky(atoi(COM_Argv(1)));
}

static void Command_Tunes_f(void)
{
	INT32 tune;
	const size_t argc = COM_Argc();

	if (argc < 2) //tunes slot ...
	{
		CONS_Printf("%s", text[TUNES_HELP]);
		CONS_Printf(text[TUNES_CURRENT], (mapmusic &= ~2048));
		return;
	}

	tune = atoi(COM_Argv(1));

	if (tune < mus_None || tune >= NUMMUSIC)
	{
		CONS_Printf(text[TUNES_VALIDSLOTS], NUMMUSIC - 1);
		return;
	}

	if (!strcasecmp(COM_Argv(1), "default"))
		tune = mapheaderinfo[gamemap-1].musicslot;

	mapmusic = (INT16)(tune | 2048);

	if (tune == mus_None)
		S_StopMusic();
	else
		S_ChangeMusic(tune, true);

	if (argc > 2)
	{
		float speed = (float)atof(COM_Argv(2));
		if (speed > 0.0f)
			S_SpeedMusic(speed);
	}
}

/** Quits a game and returns to the title screen.
  *
  */
void Command_ExitGame_f(void)
{
	INT32 i;

	D_QuitNetGame();
	CL_Reset();
	CV_ClearChangedFlags();

	for (i = 0; i < MAXPLAYERS; i++)
		CL_ClearPlayer(i);

	splitscreen = false;
	SplitScreen_OnChange();
	cv_debug = 0;
	emeralds = 0;

	if (!timeattacking)
		D_StartTitle();
}

#ifdef FISHCAKE
// Fishcake is back, but only if you've cleared Very Hard mode
static void Fishcake_OnChange(void)
{
	if (grade & 128)
	{
		cv_debug = cv_fishcake.value;
		// consvar_t's get changed to default when registered
		// so don't make modifiedgame always on!
		if (cv_debug)
		{
			if (!modifiedgame || savemoddata)
			{
				modifiedgame = true;
				savemoddata = false;
				if (!(netgame || multiplayer))
					CONS_Printf("%s",GAMEMODIFIED);
			}
		}
	}

	else if (cv_debug != cv_fishcake.value)
		CV_SetValue(&cv_fishcake, cv_debug);
}
#endif

/** Reports to the console whether or not the game has been modified.
  *
  * \todo Make it obvious, so a console command won't be necessary.
  * \sa modifiedgame
  * \author Graue <graue@oceanbase.org>
  */
static void Command_Isgamemodified_f(void)
{
	if (savemoddata)
		CONS_Printf("%s", text[GAMEMODIFIEDHELP1]);
	else if (modifiedgame)
		CONS_Printf("%s", text[GAMEMODIFIEDHELP2]);
	else
		CONS_Printf("%s", text[GAMEMODIFIEDHELP3]);
}

#ifdef _DEBUG
static void Command_Togglemodified_f(void)
{
	modifiedgame = !modifiedgame;
}
#endif

/** Makes a change to ::cv_forceskin take effect immediately.
  *
  * \todo Move the enforcement code out of SendNameAndColor() so this hack
  *       isn't needed.
  * \sa Command_SetForcedSkin_f, cv_forceskin, forcedskin
  * \author Graue <graue@oceanbase.org>
  */
static void ForceSkin_OnChange(void)
{
	if ((server || adminplayer) && (cv_forceskin.value < -1 || cv_forceskin.value >= numskins))
	{
		if (cv_forceskin.value == -2)
			CV_StealthSetValue(&cv_forceskin, numskins-1);
		else
		{
			// hack because I can't restrict this and still allow added skins to be used with forceskin.
			if (!menuactive)
				CONS_Printf(text[FORCESKIN_HELP], numskins - 1);
			CV_SetValue(&cv_forceskin, -1);
			return;
		}
	}

	if (cv_forceskin.value >= 0 && (netgame || multiplayer)) // NOT in SP, silly!
	{
		triggerforcedskin = true;
		SendNameAndColor(); // have it take effect immediately
	}
}

//Allows the player's name to be changed if cv_mute is off.
static void Name_OnChange(void)
{
	if (cv_mute.value && !(server || adminplayer == consoleplayer))
	{
		CONS_Printf("%s", text[NO_NAME_CHANGE]);
		CV_StealthSet(&cv_playername, player_names[consoleplayer]);
	}
	else
		SendNameAndColor();

}

static void Name2_OnChange(void)
{
	if (cv_mute.value) //Secondary player can't be admin.
	{
		CONS_Printf("%s", text[NO_NAME_CHANGE]);
		CV_StealthSet(&cv_playername2, player_names[secondarydisplayplayer]);
	}
	else
		SendNameAndColor2();
}

/** Sends a skin change for the console player, unless that player is moving.
  * \sa cv_skin, Skin2_OnChange, Color_OnChange
  * \author Graue <graue@oceanbase.org>
  */
static void Skin_OnChange(void)
{
	if (!P_PlayerMoving(consoleplayer))
		SendNameAndColor();
	else
		CV_StealthSet(&cv_skin, skins[players[consoleplayer].skin].name);
}

/** Sends a skin change for the secondary splitscreen player, unless that
  * player is moving.
  * \sa cv_skin2, Skin_OnChange, Color2_OnChange
  * \author Graue <graue@oceanbase.org>
  */
static void Skin2_OnChange(void)
{
	if (!P_PlayerMoving(secondarydisplayplayer))
		SendNameAndColor2();
	else
		CV_StealthSet(&cv_skin2, skins[players[secondarydisplayplayer].skin].name);
}

/** Sends a color change for the console player, unless that player is moving.
  * \sa cv_playercolor, Color2_OnChange, Skin_OnChange
  * \author Graue <graue@oceanbase.org>
  */
static void Color_OnChange(void)
{
	if (!P_PlayerMoving(consoleplayer))
	{
		// Color change menu scrolling fix
		// Determine what direction you are scrolling
		// and skip the proper colors.
		if (menuactive)
		{
			if (cv_playercolor.value == 0) // no color
			{
				if (players[consoleplayer].skincolor == 1)
					CV_StealthSetValue(&cv_playercolor, 15);
				else
					CV_StealthSetValue(&cv_playercolor, 1);
			}

			if (gametype == GT_MATCH || gametype == GT_CTF) // yellow
			{
				if (cv_playercolor.value == 15)
				{
					if (players[consoleplayer].skincolor == 1)
						CV_StealthSetValue(&cv_playercolor, 14);
					else
						CV_StealthSetValue(&cv_playercolor, 1);
				}
			}
		}

		SendNameAndColor();
	}
	else
		CV_StealthSetValue(&cv_playercolor,
			players[consoleplayer].skincolor);
}

/** Sends a color change for the secondary splitscreen player, unless that
  * player is moving.
  * \sa cv_playercolor2, Color_OnChange, Skin2_OnChange
  * \author Graue <graue@oceanbase.org>
  */
static void Color2_OnChange(void)
{
	if (!P_PlayerMoving(secondarydisplayplayer))
	{
		// Color change menu scrolling fix
		// Determine what direction you are scrolling
		// and skip the proper colors.
		if (menuactive)
		{
			if (cv_playercolor2.value == 0) // no color
			{
				if (players[secondarydisplayplayer].skincolor == 1)
					CV_StealthSetValue(&cv_playercolor2, 15);
				else
					CV_StealthSetValue(&cv_playercolor2, 1);
			}

			if (gametype == GT_MATCH || gametype == GT_CTF) // yellow
			{
				if (cv_playercolor2.value == 15)
				{
					if (players[secondarydisplayplayer].skincolor == 1)
						CV_StealthSetValue(&cv_playercolor2, 14);
					else
						CV_StealthSetValue(&cv_playercolor2, 1);
				}
			}
		}

		SendNameAndColor2();
	}
	else
		CV_StealthSetValue(&cv_playercolor2,
			players[secondarydisplayplayer].skincolor);
}

/** Displays the result of the chat being muted or unmuted.
  * The server or remote admin should already know and be able to talk
  * regardless, so this is only displayed to clients.
  *
  * \sa cv_mute
  * \author Graue <graue@oceanbase.org>
  */
static void Mute_OnChange(void)
{
	if (server || (adminplayer == consoleplayer))
		return;

	if (cv_mute.value)
		CONS_Printf("%s", text[CHAT_MUTED]);
	else
		CONS_Printf("%s", text[CHAT_NOT_MUTED]);
}

/** Hack to clear all changed flags after game start.
  * A lot of code (written by dummies, obviously) uses COM_BufAddText() to run
  * commands and change consvars, especially on game start. This is problematic
  * because CV_ClearChangedFlags() needs to get called on game start \b after
  * all those commands are run.
  *
  * Here's how it's done: the last thing in COM_BufAddText() is "dummyconsvar
  * 1", so we end up here, where dummyconsvar is reset to 0 and all the changed
  * flags are set to 0.
  *
  * \todo Fix the aforementioned code and make this hack unnecessary.
  * \sa cv_dummyconsvar
  * \author Graue <graue@oceanbase.org>
  */
static void DummyConsvar_OnChange(void)
{
	if (cv_dummyconsvar.value == 1)
	{
		CV_SetValue(&cv_dummyconsvar, 0);
		CV_ClearChangedFlags();
	}
}

static void Command_ShowScores_f(void)
{
	UINT8 i;

	if (!(netgame || multiplayer))
	{
		CONS_Printf("%s", text[NETGAMEONLY]);
		return;
	}

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i])
			CONS_Printf(text[SHOWSCORESCMD], player_names[i], players[i].score);
	}
	CONS_Printf(text[SHOWSCORES_POINTLIMIT], cv_pointlimit.value);

}

static void Command_ShowTime_f(void)
{
	if (!(netgame || multiplayer))
	{
		CONS_Printf("%s", text[NETGAMEONLY]);
		return;
	}

	CONS_Printf(text[SHOWTIMECMD], (double)leveltime/TICRATE, (double)timelimitintics/TICRATE);
}
