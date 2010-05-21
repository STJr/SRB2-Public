// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
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
/// \brief Globally defined strings.

#include "doomdef.h"
#include "dstrings.h"

/*
Character reference for future "Spanish Mode"

á, é, í, ó, ú, ñ, ¿, ¡
*/

//
// Any additions to this list need to include the #defined name the string goes by in the code at large.
// Lets make it less of a pain in the rump to find out what uses what string.
//
const char *text[NUMTEXT] =
{
	"Development mode ON.\n",                                                      //D_DEVSTR
	"CD-ROM Version: default.cfg from c:\\srb2data\n",                             //D_CDROM
	"Press a key.",                                                                //PRESSKEY
	"Press y or n.",                                                               //PRESSYN
	"Only the server can do a load net game!\n\npress a key.",                     //LOADNET
	"You can't quickload during a netgame!\n\npress a key.",                       //QLOADNET
	"You haven't picked a quicksave slot yet!\n\npress a key.",                    //QSAVESPOT
	"You can't save if you aren't playing!\n\npress a key.",                       //SAVEDEAD
	"Quicksave over your game named\n\n'%s'?\n\npress y or n.",                    //QSPROMPT
	"Do you want to quickload the game named\n\n'%s'?\n\npress y or n.",           //QLPROMPT
	"You are in a network game.\n""End it?\n(Y/N).\n",                             //NEWGAME
	"Are you sure? this skill level\nisn't even remotely fair.\n\npress y or n.",  //ULTIMATE
	"Messages are OFF",                                                            //MSGOFF
	"Messages are ON",                                                             //MSGON
	"You can't end a netgame!\n\nPress a key.",                                    //NETEND
	"Are you sure you want to end the game?\n\npress Y or N.",                     //ENDGAME
	"%s\n\n(Press 'Y' to quit)",                                                   //DOSY

	"No more place in the buffer for netcmd %d\nlocaltextcmd is %d\nnparam is %u\n",//NOROOMNETBUF
	"Got unknown net command [%d]=%d (max %d)\n",                                  //UNKNOWNNETCMD
	"Sending join request...\n",                                                   //JOINREQUEST
	"No more free memory for savegame\n",                                          //NOSAVEGAMEMEM
	"Loading savegame length %d\n",                                                //LOADSAVEGAME
	"Map is now \"%s",                                                             //MAPISNOW
	"ZONE",                                                                        //ZONE
	"Can't load the level!\n",                                                     //CANNOTLOADLEVEL
	"WARNING: Can't delete %s",                                                    //CANNOTDELETE
	"Press ESC to abort\n",                                                        //ESCABORT
	"Searching the server...\n",                                                   //SEARCHSERV
	"Contacting the server...\n",                                                  //CONTACTSERV
	"Connecting to: %s\n",                                                         //CONNECTINGTO
	"Gametype: %s\n",                                                              //NETGAMETYPE
	"Version: %d.%.2d.%u\n",                                                       //NETVERSION
	"Found, ",                                                                     //FOUND
	"Checking files...\n",                                                         //CHECKINGFILES
	"Network game synchronization aborted.\n",                                     //NETSYNCABORT
	"Ban List:\n",                                                                 //BANLIST
	"%u: %s\n",                                                                    //BANLIST2
	"%u: %s/%s\n",                                                                 //BANLIST3
	"Why: %s\n",                                                                   //BANREASON
	" None\n",                                                                     //BANNONE
	"Could not save ban list into ban.txt\n",                                      //CANNOTSAVEBANLIST
	"Could not open ban.txt for ban list\n",                                       //CANNOTLOADBANLIST
	"Connect <serveraddress>: connect to a server\n"\
	"Connect ANY: connect to the first lan server found\n"\
	"Connect SELF: connect to your own server.\n",                                 //CONNECTHELP
	"You cannot connect while in a game\nEnd this game first\n",                   //NOCONNECTINGAME
	"There is no server identification with this network driver\n",                //NOSERVERIDENTIFY
	"There is no network driver\n",                                                //NONETWORKDRIVER
	"\2num:%2d  node:%2d  %s\n",                                                   //PLAYERNUM
	"num:%2d  node:%2d  %s\n",                                                     //SERVERPLAYERNUM
	"There is no player named \"%s\"\n",                                           //NOPLAYERNAMED
	"%.2d: %*s",                                                                   //NODESCMDTXT
	" - %.2d",                                                                     //NODESCMDTXT2
	" - %s",                                                                       //NODESCMDTXT3
	" (verified admin)",                                                           //NODESCMDADMIN
	" (spectator)",                                                                //NODESCMDSPEC
	"Ban <playername> or <playernum> <reason>: ban and kick a player\n",           //BANHELP
	"Too many bans! Geez, that's a lot of people you're excluding...\n",           //TOOMANYBANS
	"You are not the server.\n",                                                   //YOUARENOTTHESERVER
	"Kick <playername> or <playernum> <reason>: kick a player\n",                  //KICKHELP
	"Server is being shut down remotely. Goodbye!\n",                              //SERVERREMOTESHUTDOWN
	"Illegal kick command received from %s for player %d\n",                       //ILLEGALKICKCMD
	"\2%s ",                                                                       //KICKEDPLAYERNAME
	"has been kicked (Go away)\n",                                                 //KICKEDGOAWAY
	"has been kicked (Consistency failure)\n",                                     //KICKEDCONSFAIL
#ifdef NEWPING
	"has been kicked (Broke ping limit)\n",                                        //KICKEDPINGTOOHIGH
#endif
	"left the game (Connection timeout)\n",                                        //LEFTGAME_TIMEOUT
	"left the game\n",                                                             //LEFTGAME
	"has been banned (Don't come back)\n",                                         //KICKEDBANNED
	"has been kicked (%s)\n",                                                      //CUSTOMKICKMSG
	"has been banned (%s)\n",                                                      //CUSTOMBANMSG
	"Illegal add player command received from %s\n",                               //ILLEGALADDPLRCMD
	"Player %d has joined the game (node %d)\n",                                   //PLAYERINGAME
	"Player Address is %s\n",                                                      //PLAYERADDRESS
	"Starting Server....\n",                                                       //STARTINGSERVER
	"Join accepted, waiting for next level change...\n",                           //JOINACCEPTED
	"Consistency failure for player %d, restoring...\n",                           //CONSFAILRESTORE
	"Player kicked is #%d\n",                                                      //CONSIS_DUMP1
	"Player %d is at %d, %d, %d\n",                                                //CONSIS_DUMP2
	"Player %d has a null mo\n",                                                   //CONSIS_DUMP3

	"I_StartupKeyboard...\n",                  //I_STARTUPKEYBOARD
	"I_StartupMouse...\n",                     //I_STARTUPMOUSE
	"I_StartupTimer...\n",                     //I_STARTUPTIMER
	"I_StartupGraphics...\n",                  //I_STARTUPGRAPHICS
	"setvbuf didnt work\n",                    //SETVBUF_FAIL
	"Playing demo %s.\n",                      //PLAYINGDEMO

	"Set extratics to %d\n",                   //SET_EXTRATICS
	"Network bandwidth set to %d\n",           //SET_BANDWIDTH
	"debug output to: %s\n",                   //DEBUG_OUTPUT
	"\2cannot debug output to file %s!\n",     //NODEBUG_OUTPUT
#ifdef NEWPING
	"%.2d : %s\n %d ms\n", //CMD_PING
#else
	"%.2d : %s\n %d tics, %d ms.\n", //CMD_PING
#endif

	"%s renamed to %s\n",                 //RENAMED_TO
	"Player %d sent a bad name change\n", //ILLEGALNAMECMD
	"Illegal color change received from %s (team: %ld, color: %ld)\n",               //ILLEGALCOLORCMD
	"orderpizza -size <value> -address <value> -toppings <value>: order a pizza!\n", //ORDERPIZZA_HELP
	"%s has ordered a delicious pizza.\n",                                           //ORDEREDPIZZA
	"rteleport -x <value> -y <value> -z <value>: relative teleport to a location\n", //RTELEPORT_HELP
	"Not a valid location\n",         //INVALIDLOCATION
	"Teleporting by %d, %d, %d...\n", //TELEPORTINGBY
	"X value not specified\n",        //XNOTSPECIFIED
	"Y value not specified\n",        //YNOTSPECIFIED
	"Teleporting to %d, %d, %d...\n", //TELEPORTINGTO
	"Playdemo <demoname>: playback a demo\n", //PLAYDEMO_HELP
	"Playing back demo '%s'.\n",              //PLAYBACK_DEMO
	"Timedemo <demoname>: time a demo\n",     //TIMEDEMO_HELP
	"Timing demo '%s'.\n", //TIMING_DEMO
	"Stopped demo.\n",     //STOPPED_DEMO
	"Map change: mapnum=%d gametype=%d ultmode=%d resetplayers=%d delay=%d skipprecutscene=%d\n", //MAPCHANGE_DEBUG
	"map <mapname> [-gametype <type> [-force]: warp to map\n",    //MAPCHANGE_HELP
	"Only the server can change the level\n",           //SERVER_CHANGELEVEL
	"\2Internal game level '%s' not found\n",           //LEVEL_NOTFOUND
	"Sorry, level change disabled in single player.\n", //NOLVLCHANGE
	"Invalid level name %s\n",                          //INVALID_LEVELNAME
	"You can't switch gametypes in single player!\n",   //NOGTCHANGE
	"That level doesn't support %s mode!\n(Use -force to override)\n", //GTNOTSUPPORTED
	"That level doesn't support Single Player mode!\n",                //SPNOTSUPPORTED
	"Illegal map change received from %s\n",    //ILLEGALMAPCMD
	"Illegal pause command received from %s\n", //ILLEGALPAUSECMD
	"Game paused by %s\n",                      //GAME_PAUSED
	"Game unpaused by %s\n",                    //GAME_UNPAUSED
	"Illegal clear scores command received from %s\n",                      //ILLEGALCLRSCRCMD
	"Scores have been reset by the server.\n",                              //SCORESRESET
	"changeteam <team>: switch to a new team (spectator or playing)\n",    //CHANGETEAM_HELP1
	"changeteam <team>: switch to a new team (red, blue or spectator)\n",  //CHANGETEAM_HELP2
	"This command cannot be used outside of Match, Tag or CTF.\n",          //NOMTF
	"This command cannot be used outside of Team Match or CTF.\n",          //NOTMCTF
	"changeteam2 <team>: switch to a new team (spectator or playing)\n",   //CHANGETEAM2_HELP1
	"changeteam2 <team>: switch to a new team (red, blue or spectator)\n", //CHANGETEAM2_HELP2
	"You're not the server. You can't change players' teams.\n",            //SERVER_CHANGETEAM
	"serverchangeteam <playernum> <team>: switch player to a new team (spectator or playing)\n",             //SERVERCHANGETEAM_HELP1
	"serverchangeteam <playernum> <team>: switch player to a new team (it, notit, playing, or spectator)\n", //SERVERCHANGETEAM_HELP2
	"serverchangeteam <playernum> <team>: switch player to a new team (red, blue or spectator)\n",           //SERVERCHANGETEAM_HELP3
	"~Eggman~ Teams are going to be scrambled! MUAHAHAHAHA!\n", //TEAMS_SCRAMBLED
	"That player is already on that team!\n",  //PLAYER_ONTEAM
	"No tag status changes after hidetime!\n", //NO_TAGCHANGE
	"Illegal team change received from player %s\n",   //ILLEGALTEAMCHANGECMD
	"Player %s sent illegal team change to team %d\n", //SENTILLEGALTEAMCHANGE
	"%s is now IT!\n",                //NOW_IT
	"%s is no longer IT!\n",          //NO_LONGER_IT
	"%s switched to the red team\n",  //REDTEAM_SWITCH
	"%s switched to the blue team\n", //BLUETEAM_SWITCH
	"%s became a spectator.\n",       //SPECTATOR_SWITCH
	"%s entered the game.\n",         //INGAME_SWITCH
	"%s was autobalanced to even the teams.\n",        //AUTOBALANCE_SWITCH
	"%s was scrambled to a different team.\n",         //SCRAMBLE_SWITCH
	"You're not the server. You can't change this.\n", //SERVER_CHANGEPASSWORD
	"password <password>: change password\n",          //PASSWORD_HELP
	"login <password>: Administrator login\n",         //LOGIN_HELP
	"Sending Login...%s\n(Notice only given if password is correct.)\n", //SENDING_LOGIN
	"%s passed authentication. (%s)\n",                                  //PASSED_AUTH
	"Password from %d failed (%s)\n",                                    //PASSWORD_FAILED
	"You're not the server... you can't give out admin privileges!\n",   //SERVER_VERIFY
	"verify <node>: give admin privileges to a node\n",                  //VERIFY_HELP
	"Illegal verification received from %s (serverplayer is %s)\n",      //ILLEGALVERIFYCMD
	"Password correct. You are now a server administrator.\n",           //PASSWORD_CORRECT
	"motd <message>: Set a message that clients see upon join.\n",       //MOTD_HELP
	"Illegal motd change received from %s\n",                            //ILLEGALMOTDCMD
	"Message of the day set.\n",                                         //MOTD_SET
	"runsoc <socfile.soc> or <lumpname>: run a soc\n",                   //RUNSOC_HELP
	"Illegal runsoc command received from %s\n",                         //ILLEGALRUNSOCCMD
	"Unknown error finding soc file (%s) the server added.\n",           //SOC_NOTFOUND
	"Illegal consistency fix packet received from %s\n",                 //ILLEGALCONSCMD
	"addfile <wadfile.wad>: load wad file\n",                            //ADDFILE_HELP
	"You must NOT be in a level to use this.\n",                         //NEED_NO_LEVEL
	"Illegal addfile command received from %s\n",                        //ILLEGALADDFILECMD
	"Illegal delfile command received from %s\n",                        //ILLEGALDELFILECMD
	"Checksum mismatch while loading %s.\nMake sure you have the copy of\nthis file that the server has.\n", //CHECKSUM_MISMATCH
	"Unknown error finding wad file (%s) the server added.\n",           //WAD_NOTFOUND
	"There are %d wads loaded:\n", //NUMWADSLOADED
	"   %.2d: %s\n",               //LISTWAD1
	"*  %.2d: %s\n",               //LISTWAD2
	"  IWAD: %s\n",                //LISTIWAD
	"SRB2%s (%s %s %s)\n",         //VERSIONCMD
#ifdef UPDATE_ALERT
	"Mod ID: %d\nMod Version: %d\nCode Base:%d\n",
#endif
	"Current gametype is %d\n",    //GAMETYPECMD
	"jumptoaxis <axisnum>: Jump to axis within current mare.\n", //JUMPTOAXIS_HELP
	"Levels will end after %s scores %d point%s.\n",             //POINTLIMIT_MESSAGE
	"a team",  //A_TEAM
	"someone", //SOMEONE
	"Point limit disabled\n",               //POINTLIMIT_DISABLED
	"Number of laps set to %d\n",           //NUMLAPS_MESSAGE
	"Levels will end after %d minute%s.\n", //TIMELIMIT_MESSAGE
	"Time limit disabled\n",                //TIMELIMIT_DISABLED
	"Gametype was changed from %s to %s\n", //GAMETYPE_CHANGED
	"HIDETIME cannot be greater or equal to the time limit!\n", //HIDETIME_ERROR
	"%s (%d): %s %d\n", //SHOWMAP1
	"%s (%d): %s\n",    //SHOWMAP2
	"Illegal exitlevel command received from %s\n", //ILLEGALEXITLVLCMD
	"Displayplayer is %d\n",                        //DISPLAYPLAYERCMD
	"If you want to change the sky interactively on a map, use the linedef executor feature instead.\n", //CHANGESKY_HELP
	"skynum <sky#>: change the sky\n", //SKYNUM_HELP
	"Previewing sky %s...\n",          //SKYNUM_PREVIEW
	"Tunes <slot#/default> <speed>: play a slot or the default stage tune at the specified speed(100%%)\n", //TUNES_HELP
	"The current tune is: %d\n",                           //TUNES_CURRENT
	"Valid slots are 1 to %d, or 0 to stop music\n",       //TUNES_VALIDSLOTS
	"modifiedgame is true, but you can save emblem and time data in this mod.\n", //GAMEMODIFIEDHELP1
	"modifiedgame is true, secrets will not be unlocked\n",                       //GAMEMODIFIEDHELP2
	"modifiedgame is false, you can unlock secrets\n",                            //GAMEMODIFIEDHELP3
	"Valid skin numbers are 0 to %d (-1 disables)\n",                             //FORCESKIN_HELP
	"You may not change your name when chat is muted.\n",                         //NO_NAME_CHANGE
	"Chat has been muted.\n",                         //CHAT_MUTED
	"Chat is no longer muted.\n",                     //CHAT_NOT_MUTED
	"%s's score is %lu\n",                            //SHOWSCORESCMD
	"The pointlimit is %d\n",                         //SHOWSCORES_POINTLIMIT
	"The current time is %f.\nThe timelimit is %f\n", //SHOWTIMECMD

	"Cheats cannot be disabled after they are activated. You must restart the server to disable cheats.\n", //CANNOT_CHANGE_CHEATS
	"Cheats must be enabled to use this.\n", //CHEATS_REQUIRED
	"Changing this variable\nrequires cheats to be enabled.\nDo you wish to enable cheats? (Y/N)\n", //CHEATS_ACTIVATE
	"Cheats have been activated.\n", //CHEATS_ACTIVATED

	"\rDownloading %s...(done)\n", //DOWNLOADING_DONE

	"Thing %d doesn't exist", //THING_NOTEXIST
	"Modification By\n", //MOD_BY
	"Character %d out of range", //CHAR_OUTOFRANGE
	"Level number %d out of range", //LEVEL_OUTOFRANGE
	"Cutscene number %d out of range", //CUTSCENE_OUTOFRANGE
	"Unlockable number %d out of range", //UNLOCKABLE_OUTOFRANGE
	"Frame %d doesn't exist", //FRAME_NOTEXIST
	"Sound %d doesn't exist", //SOUND_NOTEXIST
	"HUD item number %d out of range", //HUDITEM_OUTOFRANGE
	"Emblem number %d out of range", //EMBLEM_OUTOFRANGE
	"Warning: patch from a different SRB2 version (%d), ", //WRONG_VERSION_WARNING
	"only version 2.0 is supported\n", //SUPPORTED_VERSION
	"Unknown word: %s", //UNKNOWN_WORD
	"missing argument for '%s'", //MISSING_ARGUMENT
	"No word in this line: %s", //MISSING_WORD
	"%d warning%s in the SOC lump\n", //WARNING_IN_SOC_LUMP
	"Unloading WAD SOC edits\n", //UNLOADING_SOC_EDITS

	"Gamma correction OFF\n",     //GAMMALVL0
	"Gamma correction level 1\n", //GAMMALVL1
	"Gamma correction level 2\n", //GAMMALVL2
	"Gamma correction level 3\n", //GAMMALVL3
	"Gamma correction level 4\n", //GAMMALVL4
	"empty slot\n",               //EMPTYSTRING

	"Game saved.\n",      //GGSAVED
	"[Message unsent]\n", //HUSTR_MSGU

	"Speeding off to %s...\n", //STSTR_CLEV
	"The round has ended.\n",   //ROUND_END

	"Player is dead, etc.\n",                                  //PDEAD_ETC
	"teleport -x <value> -y <value> -z <value>: teleport to a location\n", //TELEPORT_HELP
	"Unable to teleport to that spot!\n",                      //UNABLE_TELEPORT
	"You can't teleport while in a netgame!\n",                //NETGAME_TELEPORT
	"Not a valid location\n",                                  //INVALID_LOCATION
	"\nYou can't play a demo while in a netgame.\n",           //NETGAME_DEMO
	"This command is only for capture the flag games.\n",      //CTFCMD_ONLY
	"You're already on that team!\n",                          //ALREADYONTEAM
	"The server does not allow team change.\n",                //NOTEAMCHANGE
	"You are not the server. You cannot do this.\n",           //SERVERONLY
	"WARNING: Game must be restarted to record statistics.\n", //GAMEMODIFIED
	"File doesn't exist.\n",                                   //FILE_NOT_FOUND
	"The server tried to add %s,\nbut you don't have this file.\nYou need to find it in order\nto play on this server.", //CLIENT_NEEDFILE
	"DEVMODE must be enabled.\n",                              //NEED_DEVMODE
	"You haven't earned this yet.\n",                          //NOTEARNED
	"You must be in a level to use this.\n",                   //MUSTBEINLEVEL
	"You can't use this in single player.\n",                  //CANTUSESINGLEPLAYER
	"You can't use this in a netgame.\n",                      //CANTUSEMULTIPLAYER
	"This only works in a netgame.\n",                         //NETGAMEONLY
	"This only works in single player.\n",                     //SINGLEPLAYERONLY
	"Only the server can pause the game.\n",                   //SERVERPAUSE
	"You can't pause here.\n",                                 //PAUSEINFO
	"Couldn't read file %s\n",                                 //CANTREADFILE
	"Objectplace Controls:\\"\
	"\\"\
	"Camera L: Cycle backwards      \\"\
	"Camera R: Cycle forwards       \\"\
	"    Jump: Float up             \\"\
	"    Spin: Float down           \\"\
	"   Throw: Place object         \\"\
	"   Taunt: Remove object (buggy)\\",                       //OBJPLACEINFO

	"ERROR: Powerup has no target!\n", //POWERUPNOTARGET

	"%s returned the red flag to base.\n",          //REDFLAG_RETURNED
	"%s picked up the red flag!\n",                 //REDFLAG_PICKUP
	"%s returned the blue flag to base.\n",         //BLUEFLAG_RETURNED
	"%s picked up the blue flag!\n",                //BLUEFLAG_PICKUP
	"%s tossed the %s flag.\n",                     //PLAYERTOSSFLAG
	"%s dropped the %s flag.\n",                    //PLAYERDROPFLAG
	"%s%s%s crushed %s%s%s with a heavy object!\n", //PDEAD_MATCHCRUSHED
	"%s%s%s suicided.\n",                           //PDEAD_SUICIDE
	"%s%s%s died.\n",                               //PDEAD_DIED
	"hit",                                          //P_HITVERB
	"killed",                                       //P_KILLEDVERB
	"reflected ",                                   //P_REFLECT
	"%s%s%s got nuked by %s%s%s!\n",                //PHURT_GOTNUKED
	"%s%s%s was burnt by %s%s%s's fire trail.\n",   //PHURT_GOTBURNED
	"%s%s%s was fried by %s%s%s's fire trail.\n",   //PHURT_FIRETRAIL
	"%s%s%s was %s by %s%s%s's %sbounce ring.\n",     //PHURT_B
	"%s%s%s was %s by %s%s%s's %sautomatic ring.\n",  //PHURT_A
	"%s%s%s was %s by %s%s%s's %sexplosion ring.\n",  //PHURT_E
	"%s%s%s was %s by %s%s%s's %srail ring.\n",       //PHURT_R
	"%s%s%s was %s by %s%s%s's %sscatter ring.\n",    //PHURT_S
	"%s%s%s was %s by %s%s%s's %sgrenade ring.\n",    //PHURT_G
	"%s%s%s was %s by %s%s%s's %sring.\n",            //PHURT_RING
	"%s%s%s was %s by %s%s%s.\n",                   //PHURT_MATCHDEFAULT
	"%s%s%s drowned.\n",                            //PDEAD_DROWNED
	"%s%s%s was crushed.\n",                        //PDEAD_CRUSHED
	"%s%s%s was %s by a blue crawla!\n",            //PHURT_BCRAWLA
	"%s%s%s was %s by a red crawla!\n",             //PHURT_RCRAWLA
	"%s%s%s was %s by a jetty-syn gunner!\n",       //PHURT_JETG
	"%s%s%s was %s by a jetty-syn bomber!\n",       //PHURT_JETB
	"%s%s%s was %s by a crawla commander!\n",       //PHURT_CCRAWLA
	"%s%s%s was %s by the Egg Mobile!\n",           //PHURT_BOSS1
	"%s%s%s was %s by the Egg Slimer!\n",           //PHURT_BOSS2
	"%s%s%s fell into a bottomless pit.\n",         //PDEAD_PIT
	"%s%s%s fell in some nasty goop!\n",            //PDEAD_GOOP
	"%s%s%s burned to death!\n",                    //PDEAD_FIRE
	"%s%s%s was electrocuted!\n",                   //PDEAD_ELEC
	"%s%s%s was impaled by spikes!\n",              //PDEAD_SPIK
	"%s%s%s asphyxiated in space!\n",               //PDEAD_SPAC
	"%s%s%s was %s by %s%s%s.\n",                   //PHURT_HIT
	"%s caused a world of pain.\n",                 //WORLD_OF_PAIN
	"%s got a game over.\n",                        //PLAYERGAMEOVER
	"%s has %d lives remaining.\n",                 //PLAYERLIVESREMAINING
	"%s is it!\n",                                  //PLAYERISIT

	"Warning: State Cycle Detected\n", //CYCLE_DETECT

	"%s has finished the race.\n", //FINISHEDFINALLAP
	"%s started lap %d\n",         //STARTEDLAP

	"%s has completed the level.\n",                                                 //FINISHEDLEVEL
	"Sorry, you're too high to place this object (max: 4095 above bottom floor).\n", //TOOHIGH_4095
	"Sorry, you're too high to place this object (max: 2047 above bottom floor).\n", //TOOHIGH_2047
	"%s ran out of time.\n",                                                         //OUT_OF_TIME

	" #", //INTRO01TEXT

	"Two months had passed since Dr. Eggman\n"\
	"tried to take over the world using his\n"\
	"Ring Satellite.\n#", //INTRO02TEXT

	"As it was about to drain the rings\n"\
	"away from the planet, Sonic burst into\n"\
	"the Satellite and for what he thought\n"\
	"would be the last time, defeated\n"\
	"Dr. Eggman.\n#", //INTRO03TEXT

	"\nWhat Sonic, Tails, and Knuckles had\n"\
	"not anticipated was that Eggman would\n"
	"return, bringing an all new threat.\n#", //INTRO04TEXT

	"About once every year, a strange asteroid\n"\
	"hovers around the planet. it suddenly\n"\
	"appears from nowhere, circles around, and\n"\
	"- just as mysteriously as it arrives, it\n"\
	"vanishes after about two months.\n"\
	"No one knows why it appears, or how.\n#", //INTRO05TEXT

	"\"Curses!\" Eggman yelled. \"That hedgehog\n"\
	"and his ridiculous friends will pay\n"\
	"dearly for this!\" Just then his scanner\n"\
	"blipped as the Black Rock made its\n"\
	"appearance from nowhere. Eggman looked at\n"\
	"the screen, and just shrugged it off.\n#", //INTRO06TEXT

	"It was only later\n"\
	"that he had an\n"\
	"idea. \"The Black\n"\
	"Rock usually has a\n"\
	"lot of energy\n"\
	"within it... if I\n"\
	"can somehow\n"\
	"harness this, I\n"\
	"can turn it into\n"\
	"the ultimate\n"\
	"battle station,\n"\
	"and every last\n"\
	"person will be\n"\
	"begging for mercy,\n"\
	"including Sonic!\"\n#", //INTRO07TEXT

	"\n\nBefore beginning his scheme,\n"\
	"Eggman decided to give Sonic\n"\
	"a reunion party...\n#", //INTRO08TEXT

	"\"We're ready to fire in 15 seconds!\"\n"\
	"The robot said, his voice crackling a\n"
	"little down the com-link. \"Good!\"\n"\
	"Eggman sat back in his Egg-Mobile and\n"\
	"began to count down as he saw the\n"\
	"GreenFlower city on the main monitor.\n#", //INTRO09TEXT

	"\"10...9...8...\"\n"\
	"Meanwhile, Sonic was tearing across the\n"\
	"zones, and everything became nothing but\n"\
	"a blur as he ran around loops, skimmed\n"\
	"over water, and catapulted himself off\n"\
	"rocks with his phenomenal speed.\n#", //INTRO10TEXT

	"\"5...4...3...\"\n"\
	"Sonic knew he was getting closer to the\n"\
	"City, and pushed himself harder. Finally,\n"\
	"the city appeared in the horizon.\n"\
	"\"2...1...Zero.\"\n#", //INTRO11TEXT

	"GreenFlower City was gone.\n"\
	"Sonic arrived just in time to see what\n"\
	"little of the 'ruins' were left. Everyone\n"\
	"and everything in the city had been\n"\
	"obliterated.\n#", //INTRO12TEXT

	"\"You're not quite as dead as we thought,\n"\
	"huh? Are you going to tell us your plan as\n"\
	"usual or will I 'have to work it out' or\n"\
	"something?\"                         \n"\
	"\"We'll see... let's give you a quick warm\n"\
	"up, Sonic! JETTYSYNS! Open fire!\"\n#", //INTRO13TEXT

	"Eggman took this\n"\
	"as his cue and\n"\
	"blasted off,\n"\
	"leaving Sonic\n"\
	"and Tails behind.\n"\
	"Tails looked at\n"\
	"the ruins of the\n"\
	"Greenflower City\n"\
	"with a grim face\n"\
	"and sighed.           \n"\
	"\"Now what do we\n"\
	"do?\", he asked.\n#", //INTRO14TEXT

	"\"Easy! We go\n"\
	"find Eggman\n"\
	"and stop his\n"\
	"latest\n"\
	"insane plan.\n"\
	"Just like\n"\
	"we've always\n"\
	"done, right?                 \n\n"\
	"...                    \n\n"\
	"\"Tails,what\n"\
	"*ARE* you\n"\
	"doing?\"\n#", //INTRO15TEXT

	"\"I'm just finding what mission obje...\n"\
	"a-ha! Here it is! This will only give\n"\
	"the robot's primary objective. It says,\n"\
	"* LOCATE AND RETRIEVE CHAOS EMERALD.\n"\
	"ESTIMATED LOCATION: GREENFLOWER ZONE *\"\n"\
	"\"All right, then let's go!\"\n#", //INTRO16TEXT

/*
"What are we waiting for? The first emerald is ours!" Sonic was about to
run, when he saw a shadow pass over him, he recognized the silhouette
instantly.
	"Knuckles!" Sonic said. The echidna stopped his glide and landed
facing Sonic. "What are you doing here?"
	He replied, "This crisis affects the Floating Island,
if that explosion I saw is anything to go by."
	"If you're willing to help then... let's go!"
	*/

	"Eggman's tied explosives\nto your girlfriend, and\nwill activate them if\nyou press the 'Y' key!\nPress 'N' to save her!", //QUITMSG
	"What would Tails say if\nhe saw you quitting the game?",             //QUITMSG1
	"Hey!\nWhere do ya think you're goin'?",                              //QUITMSG2
	"Forget your studies!\nPlay some more!",                              //QUITMSG3
	"You're trying to say you\nlike Sonic 2K6 better than\nthis, right?", //QUITMSG4
	"Don't leave yet -- there's a\nsuper emerald around that corner!",    //QUITMSG5
	"You'd rather work than play?",                                       //QUITMSG6
	"Go ahead and leave. See if I care...\n*sniffle*",                    //QUITMSG7

	"If you leave now,\nEggman will take over the world!", //QUIT2MSG
	"Don't quit!\nThere are animals\nto save!",            //QUIT2MSG1
	"Aw c'mon, just bop\na few more robots!",              //QUIT2MSG2
	"Did you get all those Chaos Emeralds?",               //QUIT2MSG3
	"If you leave, I'll use\nmy spin attack on you!",      //QUIT2MSG4
	"Don't go!\nYou might find the hidden\nlevels!",       //QUIT2MSG5
	"Hit the 'N' key, Sonic!\nThe 'N' key!",               //QUIT2MSG6

	"Are you really going to\ngive up?\nWe certainly would never give you up.",     //QUIT3MSG
	"Come on, just ONE more netgame!",                                              //QUIT3MSG1
	"Press 'N' to unlock\nthe Ultimate Cheat!",                                     //QUIT3MSG2
	"Why don't you go back and try\njumping on that house to\nsee what happens?",   //QUIT3MSG3
	"Every time you press 'Y', an\nSRB2 Developer cries...",                        //QUIT3MSG4
	"You'll be back to play soon, though...\n......right?",                         //QUIT3MSG5
	"Aww, is Egg Rock Zone too\ndifficult for you?",                                //QUIT3MSG6
	"===========================================================================\n"
	"                       Sonic Robo Blast II!\n"
	"                       by Sonic Team Junior\n"
	"                      http://www.srb2.org\n"
	"      This is a modified version. Go to our site for the original.\n"
	"===========================================================================\n", //MODIFIED

	"===========================================================================\n"
	"                   We hope you enjoy this game as\n"
	"                     much as we did making it!\n"
	"                            ...wait. =P\n"
	"===========================================================================\n", //COMERCIAL

	"M_LoadDefaults: Load system defaults.\n",         //M_LOAD
	"Z_Init: Init zone memory allocation daemon. \n",  //Z_INIT
	"W_Init: Init WADfiles.\n",                        //W_INIT
	"M_Init: Init miscellaneous info.\n",              //M_INIT
	"R_Init: Init SRB2 refresh daemon - ",             //R_INIT
	"\nP_Init: Init Playloop state.\n",                //P_INIT
	"I_Init: Setting up machine state.\n",             //I_INIT
	"D_CheckNetGame: Checking network game status.\n", //D_CHECKNET
	"S_Init: Setting up sound.\n",                     //S_SETSOUND
	"HU_Init: Setting up heads up display.\n",         //HU_INIT
	"ST_Init: Init status bar.\n",                     //ST_INIT

	"srb2.srb",  //SRB2SRB
	"srb2.wad",  //SRB2WAD
	"sonic.plr", //SONICPLR
	"tails.plr", //TAILSPLR
	"knux.plr",  //KNUXPLR
	"music.dta", //MUSICWAD

	"c:\\srb2data\\"SAVEGAMENAME"%u.ssg", //CDROM_SAVEI
	SAVEGAMENAME"%u.ssg",                 //NORM_SAVEI

	//BP: here is special dehacked handling, include centering and version
	"Sonic Robo Blast 2:" VERSIONSTRING, //SPECIALDEHACKED
};

char savegamename[256];
