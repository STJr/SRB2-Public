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
/// \brief Thing frame/state LUT

// Data.
#include "doomdef.h"
#include "doomstat.h"
#include "sounds.h"
#include "p_mobj.h"
#include "m_misc.h"
#include "z_zone.h"
#include "d_player.h"
#ifdef HWRENDER
#include "hardware/hw_light.h"
#endif

static char sprnamesbackup[NUMSPRITES + 1][5];
static state_t statesbackup[NUMSTATES];
static mobjinfo_t mobjinfobackup[NUMMOBJTYPES];

char sprnames[NUMSPRITES + 1][5] =
{
	"PLAY","POSS","SPOS","FISH","BUZZ","RBUZ","JETB","JETG","CCOM","DETN",
	"SKIM","TRET","TURR","SHRP","JJAW","SNLR","VLTR","PNTY","ARCH","CBFS",
	"SPSH","ESHI","GSNP","MNUS","BPLD","JETF","EGGM","EGGN","TNKA","TNKB",
	"SPNK","GOOP","EGGO","PRPL","EGGP","EGGQ","EGGR","BRAK","BGOO","BMSL",
	"EGGT","METL","RING","TRNG","EMMY","TOKE","RFLG","BFLG","NWNG","EMBM",
	"CEMG","EMER","FANS","BUBL","SIGN","STEM","SPIK","SFLM","DSPK","USPK",
	"STPT","BMNE","SRBX","RRBX","BRBX","SHTV","PINV","YLTV","BLTV","BKTV",
	"WHTV","GRTV","EGGB","MIXU","RECY","QUES","GBTV","MTEX","MISL","TORP",
	"MINE","JBUL","TRLS","CBLL","AROW","FWR1","FWR2","FWR3","FWR4","BUS1",
	"BUS2","THZP","ALRM","GARG","SEWE","DRIP","CRL1","CRL2","CRL3","BCRY",
	"CHAN","FLAM","ESTA","SMCH","BMCH","SMCE","BMCE","BTBL","STBL","CACT",
	"FLME","XMS1","XMS2","XMS3","DBAL","THOK","SORB","IVSP","SSPK","BIRD",
	"BUNY","MOUS","CHIC","COWZ","SPRY","SPRR","SPRB","SUDY","SUDR","YSPR",
	"RSPR","YSUD","RSUD","RAIN","SNO1","SPLH","SPLA","SMOK","BUBP","BUBO",
	"BUBN","BUBM","POPP","TFOG","SEED","PRTL","SCOR","DRWN","TTAG","GFLG",
	"RRNG","RNGB","RNGR","RNGA","RNGE","RNGS","RNGG","PIKB","PIKR","PIKA",
	"PIKE","PIKS","PIKG","TAUT","TGRE","TSCR","COIN","CPRK","GOOM","BGOM",
	"FFWR","FBLL","SHLL","PUMA","HAMM","KOOP","BFLM","MAXE","MUS1","MUS2",
	"TOAD","NDRN","SUPE","SUPZ","NDRL","NSPK","NBMP","HOOP","NSCR","NPRA",
	"NPRB","NPRC","CAPS","DISS","SUPT","SPRK","XPLD","WPLD","STG0","STG1",
	"STG2","STG3","STG4","STG5","STG6","STG7","STG8","STG9","ROIA","ROIB",
	"ROIC","ROID","ROIE","ROIF","ROIG","ROIH","ROII","ROIJ","ROIK","ROIL",
	"ROIM","ROIN","ROIO","ROIP","BBAL","GWLG","GWLR","SRBA","SRBB","SRBC",
	"SRBD","SRBE","SRBF","SRBG","PXVI","SRBH","SRBI","SRBJ","SRBK","SRBL",
	"SRBM","SRBN","SRBO",
};

// Doesn't work with g++, needs actionf_p1 (don't modify this comment)
state_t states[NUMSTATES] =
{
	// frame is masked through FF_FRAMEMASK
	// FF_FULLBRIGHT (0x8000) activates the fullbright colormap
	// Keep this comment directly above S_NULL.
	{SPR_DISS, 0, -1, {NULL}, 0, 0, S_NULL}, // S_NULL

	// Player
	{SPR_PLAY,     0, 105, {NULL},   0, 0, S_PLAY_TAP1},        // S_PLAY_STND
	{SPR_PLAY,     1,  16, {NULL},   0, 0, S_PLAY_TAP2},        // S_PLAY_TAP1
	{SPR_PLAY,     2,  16, {NULL},   0, 0, S_PLAY_TAP1},        // S_PLAY_TAP2
	{SPR_PLAY,     3,   4, {NULL},   0, 0, S_PLAY_RUN2},        // S_PLAY_RUN1
	{SPR_PLAY,     4,   4, {NULL},   0, 0, S_PLAY_RUN3},        // S_PLAY_RUN2
	{SPR_PLAY,     5,   4, {NULL},   0, 0, S_PLAY_RUN4},        // S_PLAY_RUN3
	{SPR_PLAY,     6,   4, {NULL},   0, 0, S_PLAY_RUN5},        // S_PLAY_RUN4
	{SPR_PLAY,     7,   4, {NULL},   0, 0, S_PLAY_RUN6},        // S_PLAY_RUN5
	{SPR_PLAY,     8,   4, {NULL},   0, 0, S_PLAY_RUN7},        // S_PLAY_RUN6
	{SPR_PLAY,     9,   4, {NULL},   0, 0, S_PLAY_RUN8},        // S_PLAY_RUN7
	{SPR_PLAY,    10,   4, {NULL},   0, 0, S_PLAY_RUN1},        // S_PLAY_RUN8
	{SPR_PLAY,    11,   1, {NULL},   0, 0, S_PLAY_ATK2},        // S_PLAY_ATK1
	{SPR_PLAY,    12,   1, {NULL},   0, 0, S_PLAY_ATK3},        // S_PLAY_ATK2
	{SPR_PLAY,    13,   1, {NULL},   0, 0, S_PLAY_ATK4},        // S_PLAY_ATK3
	{SPR_PLAY,    14,   1, {NULL},   0, 0, S_PLAY_ATK1},        // S_PLAY_ATK4
	{SPR_PLAY,    15,  -1, {NULL},   0, 0, S_NULL},             // S_PLAY_PLG1
	{SPR_PLAY,    16,   2, {NULL},   0, 0, S_PLAY_SPD2},        // S_PLAY_SPD1
	{SPR_PLAY,    17,   2, {NULL},   0, 0, S_PLAY_SPD3},        // S_PLAY_SPD2
	{SPR_PLAY,    18,   2, {NULL},   0, 0, S_PLAY_SPD4},        // S_PLAY_SPD3
	{SPR_PLAY,    19,   2, {NULL},   0, 0, S_PLAY_SPD1},        // S_PLAY_SPD4
	{SPR_PLAY,    20,   2, {NULL},   0, 0, S_PLAY_ABL2},        // S_PLAY_ABL1
	{SPR_PLAY,    21,   2, {NULL},   0, 0, S_PLAY_ABL1},        // S_PLAY_ABL2
	{SPR_PLAY,    22,   6, {NULL},   0, 0, S_PLAY_SPC2},        // S_PLAY_SPC1
	{SPR_PLAY,    23,   6, {NULL},   0, 0, S_PLAY_SPC3},        // S_PLAY_SPC2
	{SPR_PLAY,    24,   6, {NULL},   0, 0, S_PLAY_SPC4},        // S_PLAY_SPC3
	{SPR_PLAY,    25,   6, {NULL},   0, 0, S_PLAY_SPC1},        // S_PLAY_SPC4
	{SPR_PLAY,    22,  -1, {NULL},   0, 0, S_NULL},             // S_PLAY_CLIMB1
	{SPR_PLAY,    23,   5, {NULL},   0, 0, S_PLAY_CLIMB3},      // S_PLAY_CLIMB2
	{SPR_PLAY,    24,   5, {NULL},   0, 0, S_PLAY_CLIMB4},      // S_PLAY_CLIMB3
	{SPR_PLAY,    25,   5, {NULL},   0, 0, S_PLAY_CLIMB5},      // S_PLAY_CLIMB4
	{SPR_PLAY,    24,   5, {NULL},   0, 0, S_PLAY_CLIMB2},      // S_PLAY_CLIMB5
	{SPR_PLAY,    26,  14, {NULL},   0, 0, S_PLAY_RUN1},        // S_PLAY_GASP
	{SPR_PLAY,    27,  -1, {NULL},   0, 0, S_PLAY_STND},        // S_PLAY_PAIN
	{SPR_PLAY,    28,   8, {A_Fall}, 0, 0, S_PLAY_DIE2},        // S_PLAY_DIE1
	{SPR_PLAY,    28,   7, {NULL},   0, 0, S_PLAY_DIE3},        // S_PLAY_DIE2
	{SPR_PLAY,    28,  -1, {NULL},   0, 0, S_NULL},             // S_PLAY_DIE3
	{SPR_PLAY,    29,  12, {NULL},   0, 0, S_PLAY_TEETER2},     // S_PLAY_TEETER1
	{SPR_PLAY,    30,  12, {NULL},   0, 0, S_PLAY_TEETER1},     // S_PLAY_TEETER2
	{SPR_PLAY,    31,   2, {NULL},   0, 0, S_PLAY_FALL2},       // S_PLAY_FALL1
	{SPR_PLAY,    32,   2, {NULL},   0, 0, S_PLAY_FALL1},       // S_PLAY_FALL2
	{SPR_PLAY,    33,  -1, {NULL},   0, 0, S_NULL},             // S_PLAY_CARRY
	{SPR_PLAY,    20,  -1, {NULL},   0, 0, S_PLAY_SUPERSTAND},  // S_PLAY_SUPERSTAND
	{SPR_PLAY,    20,   7, {NULL},   0, 0, S_PLAY_SUPERWALK2},  // S_PLAY_SUPERWALK1
	{SPR_PLAY,    21,   7, {NULL},   0, 0, S_PLAY_SUPERWALK1},  // S_PLAY_SUPERWALK2
	{SPR_PLAY,    22,   7, {NULL},   0, 0, S_PLAY_SUPERFLY2},   // S_PLAY_SUPERFLY1
	{SPR_PLAY,    23,   7, {NULL},   0, 0, S_PLAY_SUPERFLY1},   // S_PLAY_SUPERFLY2
	{SPR_PLAY,    24,  12, {NULL},   0, 0, S_PLAY_SUPERTEETER}, // S_PLAY_SUPERTEETER
	{SPR_PLAY,    25,  -1, {NULL},   0, 0, S_PLAY_SUPERSTAND},  // S_PLAY_SUPERHIT
	{SPR_PLAY,    38,   4, {NULL},   0, 0, S_PLAY_SUPERTRANS2}, // S_PLAY_SUPERTRANS1
	{SPR_PLAY,    39,   4, {NULL},   0, 0, S_PLAY_SUPERTRANS3}, // S_PLAY_SUPERTRANS2
	{SPR_PLAY, 32808,   4, {NULL},   0, 0, S_PLAY_SUPERTRANS4}, // S_PLAY_SUPERTRANS3
	{SPR_PLAY,    41,   3, {NULL},   0, 0, S_PLAY_SUPERTRANS5}, // S_PLAY_SUPERTRANS4
	{SPR_PLAY,    42,   3, {NULL},   0, 0, S_PLAY_SUPERTRANS6}, // S_PLAY_SUPERTRANS5
	{SPR_PLAY,    43,   3, {NULL},   0, 0, S_PLAY_SUPERTRANS7}, // S_PLAY_SUPERTRANS6
	{SPR_PLAY,    44,   3, {NULL},   0, 0, S_PLAY_SUPERTRANS8}, // S_PLAY_SUPERTRANS7
	{SPR_PLAY,    45,   3, {NULL},   0, 0, S_PLAY_SUPERTRANS9}, // S_PLAY_SUPERTRANS8
	{SPR_PLAY,    46,  16, {NULL},   0, 0, S_PLAY_RUN1},        // S_PLAY_SUPERTRANS9

	// 1-Up Box Sprites (uses player sprite)
	{SPR_PLAY, 34, 2, {A_1upThinker}, 0, 0, S_PLAY_BOX1B}, // S_PLAY_BOX1A
	{SPR_PLAY, 35, 2, {NULL}, 0, 0, S_PLAY_BOX1A},         // S_PLAY_BOX1B
	{SPR_PLAY, 36, 18, {NULL}, 0, 0, S_PLAY_BOX1D},        // S_PLAY_BOX1C
	{SPR_PLAY, 36, 18, {A_ExtraLife}, 0, 0, S_NULL},       // S_PLAY_BOX1D

	// Level end sign (uses player sprite)
	{SPR_PLAY, 37, -1, {NULL}, 0, 0, S_NULL},         // S_PLAY_SIGN

	// Blue Crawla
	{SPR_POSS, 0, 5, {A_Look}, 0, 0, S_POSS_STND2},   // S_POSS_STND
	{SPR_POSS, 0, 5, {A_Look}, 0, 0, S_POSS_STND},    // S_POSS_STND2
	{SPR_POSS, 0, 3, {A_Chase}, 0, 0, S_POSS_RUN2},   // S_POSS_RUN1
	{SPR_POSS, 1, 3, {A_Chase}, 0, 0, S_POSS_RUN3},   // S_POSS_RUN2
	{SPR_POSS, 2, 3, {A_Chase}, 0, 0, S_POSS_RUN4},   // S_POSS_RUN3
	{SPR_POSS, 3, 3, {A_Chase}, 0, 0, S_POSS_RUN5},   // S_POSS_RUN4
	{SPR_POSS, 4, 3, {A_Chase}, 0, 0, S_POSS_RUN6},   // S_POSS_RUN5
	{SPR_POSS, 5, 3, {A_Chase}, 0, 0, S_POSS_RUN1},   // S_POSS_RUN6

	// Red Crawla
	{SPR_SPOS, 0, 5, {A_Look}, 0, 0, S_SPOS_STND2},   // S_SPOS_STND
	{SPR_SPOS, 0, 5, {A_Look}, 0, 0, S_SPOS_STND},    // S_SPOS_STND2
	{SPR_SPOS, 0, 1, {A_Chase}, 0, 0, S_SPOS_RUN2},   // S_SPOS_RUN1
	{SPR_SPOS, 1, 1, {A_Chase}, 0, 0, S_SPOS_RUN3},   // S_SPOS_RUN2
	{SPR_SPOS, 2, 1, {A_Chase}, 0, 0, S_SPOS_RUN4},   // S_SPOS_RUN3
	{SPR_SPOS, 3, 1, {A_Chase}, 0, 0, S_SPOS_RUN5},   // S_SPOS_RUN4
	{SPR_SPOS, 4, 1, {A_Chase}, 0, 0, S_SPOS_RUN6},   // S_SPOS_RUN5
	{SPR_SPOS, 5, 1, {A_Chase}, 0, 0, S_SPOS_RUN1},   // S_SPOS_RUN6

	// Greenflower Fish
	{SPR_FISH, 1, 1, {NULL}, 0, 0, S_FISH2},         // S_FISH1
	{SPR_FISH, 1, 1, {A_FishJump}, 0, 0, S_FISH1},   // S_FISH2
	{SPR_FISH, 0, 1, {NULL}, 0, 0, S_FISH4},         // S_FISH3
	{SPR_FISH, 0, 1, {A_FishJump}, 0, 0, S_FISH3},   // S_FISH4

	// Gold Buzz
	{SPR_BUZZ, 0, 2, {A_Look}, 0, 0, S_BUZZLOOK2},   // S_BUZZLOOK1
	{SPR_BUZZ, 1, 2, {A_Look}, 0, 0, S_BUZZLOOK1},   // S_BUZZLOOK2
	{SPR_BUZZ, 0, 2, {A_BuzzFly}, 0, 0, S_BUZZFLY2}, // S_BUZZFLY1
	{SPR_BUZZ, 1, 2, {A_BuzzFly}, 0, 0, S_BUZZFLY1}, // S_BUZZFLY2

	// Red Buzz
	{SPR_RBUZ, 0, 2, {A_Look}, 0, 0, S_RBUZZLOOK2},   // S_RBUZZLOOK1
	{SPR_RBUZ, 1, 2, {A_Look}, 0, 0, S_RBUZZLOOK1},   // S_RBUZZLOOK2
	{SPR_RBUZ, 0, 2, {A_BuzzFly}, 0, 0, S_RBUZZFLY2}, // S_RBUZZFLY1
	{SPR_RBUZ, 1, 2, {A_BuzzFly}, 0, 0, S_RBUZZFLY1}, // S_RBUZZFLY2

	// Jetty-Syn Bomber
	{SPR_JETB, 0, 4, {A_Look}, 0, 0, S_JETBLOOK2},      // S_JETBLOOK1
	{SPR_JETB, 1, 4, {A_Look}, 0, 0, S_JETBLOOK1},      // S_JETBLOOK2
	{SPR_JETB, 0, 1, {A_JetbThink}, 0, 0, S_JETBZOOM2}, // S_JETBZOOM1
	{SPR_JETB, 1, 1, {A_JetbThink}, 0, 0, S_JETBZOOM1}, // S_JETBZOOM2

	// Jetty-Syn Gunner
	{SPR_JETG, 0, 4, {A_Look}, 0, 0, S_JETGLOOK2},       // S_JETGLOOK1
	{SPR_JETG, 1, 4, {A_Look}, 0, 0, S_JETGLOOK1},       // S_JETGLOOK2
	{SPR_JETG, 0, 1, {A_JetgThink}, 0, 0, S_JETGZOOM2},  // S_JETGZOOM1
	{SPR_JETG, 1, 1, {A_JetgThink}, 0, 0, S_JETGZOOM1},  // S_JETGZOOM2
	{SPR_JETG, 2, 1, {A_JetgShoot}, 0, 0, S_JETGSHOOT2}, // S_JETGSHOOT1
	{SPR_JETG, 3, 1, {NULL}, 0, 0, S_JETGZOOM1},         // S_JETGSHOOT2

	// Crawla Commander
	{SPR_CCOM, 0, 1, {A_CrawlaCommanderThink}, 0, 15*FRACUNIT, S_CCOMMAND2}, // S_CCOMMAND1
	{SPR_CCOM, 1, 1, {A_CrawlaCommanderThink}, 0, 15*FRACUNIT, S_CCOMMAND1}, // S_CCOMMAND2
	{SPR_CCOM, 2, 1, {A_CrawlaCommanderThink}, 0, 15*FRACUNIT, S_CCOMMAND4}, // S_CCOMMAND3
	{SPR_CCOM, 3, 1, {A_CrawlaCommanderThink}, 0, 15*FRACUNIT, S_CCOMMAND3}, // S_CCOMMAND4

	// Deton
	{SPR_DETN, 0, 35, {A_Look}, 0, 0, S_DETON1},       // S_DETON1
	{SPR_DETN, 0,  1, {A_DetonChase}, 0, 0, S_DETON3},  // S_DETON2
	{SPR_DETN, 1,  1, {A_DetonChase}, 0, 0, S_DETON4},  // S_DETON3
	{SPR_DETN, 2,  1, {A_DetonChase}, 0, 0, S_DETON5},  // S_DETON4
	{SPR_DETN, 3,  1, {A_DetonChase}, 0, 0, S_DETON6},  // S_DETON5
	{SPR_DETN, 4,  1, {A_DetonChase}, 0, 0, S_DETON7},  // S_DETON6
	{SPR_DETN, 5,  1, {A_DetonChase}, 0, 0, S_DETON8},  // S_DETON7
	{SPR_DETN, 6,  1, {A_DetonChase}, 0, 0, S_DETON9},  // S_DETON8
	{SPR_DETN, 7,  1, {A_DetonChase}, 0, 0, S_DETON10}, // S_DETON9
	{SPR_DETN, 6,  1, {A_DetonChase}, 0, 0, S_DETON11}, // S_DETON10
	{SPR_DETN, 5,  1, {A_DetonChase}, 0, 0, S_DETON12}, // S_DETON11
	{SPR_DETN, 4,  1, {A_DetonChase}, 0, 0, S_DETON13}, // S_DETON12
	{SPR_DETN, 3,  1, {A_DetonChase}, 0, 0, S_DETON14}, // S_DETON13
	{SPR_DETN, 2,  1, {A_DetonChase}, 0, 0, S_DETON15}, // S_DETON14
	{SPR_DETN, 1,  1, {A_DetonChase}, 0, 0, S_DETON2},  // S_DETON15
	{SPR_DETN, 0, -1, {NULL},         0, 0, S_DETON16}, // S_DETON16

	// Skim Mine Dropper
	{SPR_SKIM, 0,  1, {A_SkimChase}, 0, 0, S_SKIM2},    // S_SKIM1
	{SPR_SKIM, 0,  1, {A_SkimChase}, 0, 0, S_SKIM1},    // S_SKIM2
	{SPR_SKIM, 0, 14,        {NULL}, 0, 0, S_SKIM4},    // S_SKIM3
	{SPR_SKIM, 0, 14,  {A_DropMine}, 0, 0, S_SKIM1},    // S_SKIM4

	// THZ Turret
	{SPR_TRET, 32768, 105, {A_TurretStop}, 0, 0, S_TURRETFIRE}, // S_TURRET
	{SPR_TRET, 32768, 105, {A_TurretFire}, 0, 0, S_TURRET},     // S_TURRETFIRE
	{SPR_TRET, 32769, 7, {A_Pain}, 0, 0, S_TURRETSHOCK2},       // S_TURRETSHOCK1
	{SPR_TRET, 32770, 7, {NULL}, 0, 0, S_TURRETSHOCK3},         // S_TURRETSHOCK2
	{SPR_TRET, 32771, 7, {NULL}, 0, 0, S_TURRETSHOCK4},         // S_TURRETSHOCK3
	{SPR_TRET, 32772, 7, {NULL}, 0, 0, S_TURRETSHOCK5},         // S_TURRETSHOCK4
	{SPR_TRET, 32769, 7, {NULL}, 0, 0, S_TURRETSHOCK6},         // S_TURRETSHOCK5
	{SPR_TRET, 32770, 7, {A_Pain}, 0, 0, S_TURRETSHOCK7},       // S_TURRETSHOCK6
	{SPR_TRET, 32771, 7, {NULL}, 0, 0, S_TURRETSHOCK8},         // S_TURRETSHOCK7
	{SPR_TRET, 32772, 7, {NULL}, 0, 0, S_TURRETSHOCK9},         // S_TURRETSHOCK8
	{SPR_TRET, 32772, 7, {A_LinedefExecute}, 32000, 0, S_XPLD1}, // S_TURRETSHOCK9

	{SPR_TURR, 0, 1, {A_Look}, 1, 0, S_TURRETLOOK},          // S_TURRETLOOK
	{SPR_TURR, 1, 2, {A_FaceTarget}, 0, 0, S_TURRETPOPUP2},  // S_TURRETPOPUP1
	{SPR_TURR, 2, 2, {NULL}, 0, 0, S_TURRETPOPUP3},  // S_TURRETPOPUP2
	{SPR_TURR, 3, 2, {NULL}, 0, 0, S_TURRETPOPUP4},  // S_TURRETPOPUP3
	{SPR_TURR, 4, 2, {NULL}, 0, 0, S_TURRETPOPUP5},  // S_TURRETPOPUP4
	{SPR_TURR, 5, 2, {NULL}, 0, 0, S_TURRETPOPUP6},  // S_TURRETPOPUP5
	{SPR_TURR, 6, 2, {NULL}, 0, 0, S_TURRETPOPUP7},  // S_TURRETPOPUP6
	{SPR_TURR, 7, 2, {NULL}, 0, 0, S_TURRETPOPUP8},  // S_TURRETPOPUP7
	{SPR_TURR, 8, 14,{NULL}, 0, 0, S_TURRETSHOOT},   // S_TURRETPOPUP8
	{SPR_TURR, 8, 14,{A_JetgShoot}, 0, 0, S_TURRETPOPDOWN1}, // S_TURRETSHOOT
	{SPR_TURR, 7, 2, {NULL}, 0, 0, S_TURRETPOPDOWN2},        // S_TURRETPOPDOWN1
	{SPR_TURR, 6, 2, {NULL}, 0, 0, S_TURRETPOPDOWN3},        // S_TURRETPOPDOWN2
	{SPR_TURR, 5, 2, {NULL}, 0, 0, S_TURRETPOPDOWN4},        // S_TURRETPOPDOWN3
	{SPR_TURR, 4, 2, {NULL}, 0, 0, S_TURRETPOPDOWN5},        // S_TURRETPOPDOWN4
	{SPR_TURR, 3, 2, {NULL}, 0, 0, S_TURRETPOPDOWN6},        // S_TURRETPOPDOWN5
	{SPR_TURR, 2, 2, {NULL}, 0, 0, S_TURRETPOPDOWN7},        // S_TURRETPOPDOWN6
	{SPR_TURR, 1, 2, {NULL}, 0, 0, S_TURRETPOPDOWN8},        // S_TURRETPOPDOWN7
	{SPR_TURR, 0, 69,{A_SetTics}, 0, 1, S_TURRETLOOK},       // S_TURRETPOPDOWN8

	// Sharp
	{SPR_SHRP, 0, 1, {A_Look},       0, 0, S_SHARP_ROAM1}, // S_SHARP_ROAM1,
	{SPR_SHRP, 0, 1, {A_SharpChase}, 0, 0, S_SHARP_ROAM2}, // S_SHARP_ROAM2,
	{SPR_SHRP, 1, 2, {A_FaceTarget}, 0, 0, S_SHARP_AIM2}, // S_SHARP_AIM1,
	{SPR_SHRP, 2, 2, {A_FaceTarget}, 0, 0, S_SHARP_AIM3}, // S_SHARP_AIM2,
	{SPR_SHRP, 3, 2, {A_FaceTarget}, 0, 0, S_SHARP_AIM4}, // S_SHARP_AIM3,
	{SPR_SHRP, 4, 7, {A_FaceTarget}, 0, 0, S_SHARP_SPIN1}, // S_SHARP_AIM4,
	{SPR_SHRP, 4, 1, {A_SharpSpin},  0, 0, S_SHARP_SPIN2},// S_SHARP_SPIN1,
	{SPR_SHRP, 4, 1, {A_SharpSpin},  0, 0, S_SHARP_SPIN3}, // S_SHARP_SPIN2,
	{SPR_SHRP, 4, 1, {A_SharpSpin},  0, 0, S_SHARP_SPIN4}, // S_SHARP_SPIN3,
	{SPR_SHRP, 4, 1, {A_SharpSpin},  0, 0, S_SHARP_SPIN5}, // S_SHARP_SPIN4,
	{SPR_SHRP, 4, 1, {A_SharpSpin},  0, 0, S_SHARP_SPIN6}, // S_SHARP_SPIN5,
	{SPR_SHRP, 4, 1, {A_SharpSpin},  0, 0, S_SHARP_SPIN7}, // S_SHARP_SPIN6,
	{SPR_SHRP, 4, 1, {A_SharpSpin},  0, 0, S_SHARP_SPIN8}, // S_SHARP_SPIN7,
	{SPR_SHRP, 4, 1, {A_SharpSpin},  0, 0, S_SHARP_SPIN1}, // S_SHARP_SPIN8,

	// Jet Jaw
	{SPR_JJAW, 0, 1, {A_JetJawRoam},  0, 0, S_JETJAW_ROAM2},   // S_JETJAW_ROAM1
	{SPR_JJAW, 0, 1, {A_JetJawRoam},  0, 0, S_JETJAW_ROAM3},   // S_JETJAW_ROAM2
	{SPR_JJAW, 0, 1, {A_JetJawRoam},  0, 0, S_JETJAW_ROAM4},   // S_JETJAW_ROAM3
	{SPR_JJAW, 0, 1, {A_JetJawRoam},  0, 0, S_JETJAW_ROAM5},   // S_JETJAW_ROAM4
	{SPR_JJAW, 1, 1, {A_JetJawRoam},  0, 0, S_JETJAW_ROAM6},   // S_JETJAW_ROAM5
	{SPR_JJAW, 1, 1, {A_JetJawRoam},  0, 0, S_JETJAW_ROAM7},   // S_JETJAW_ROAM6
	{SPR_JJAW, 1, 1, {A_JetJawRoam},  0, 0, S_JETJAW_ROAM8},   // S_JETJAW_ROAM7
	{SPR_JJAW, 1, 1, {A_JetJawRoam},  0, 0, S_JETJAW_ROAM1},   // S_JETJAW_ROAM8
	{SPR_JJAW, 0, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP2},  // S_JETJAW_CHOMP1
	{SPR_JJAW, 0, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP3},  // S_JETJAW_CHOMP2
	{SPR_JJAW, 0, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP4},  // S_JETJAW_CHOMP3
	{SPR_JJAW, 0, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP5},  // S_JETJAW_CHOMP4
	{SPR_JJAW, 1, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP6},  // S_JETJAW_CHOMP5
	{SPR_JJAW, 1, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP7},  // S_JETJAW_CHOMP6
	{SPR_JJAW, 1, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP8},  // S_JETJAW_CHOMP7
	{SPR_JJAW, 1, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP9},  // S_JETJAW_CHOMP8
	{SPR_JJAW, 2, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP10}, // S_JETJAW_CHOMP9
	{SPR_JJAW, 2, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP11}, // S_JETJAW_CHOMP10
	{SPR_JJAW, 2, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP12}, // S_JETJAW_CHOMP11
	{SPR_JJAW, 2, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP13}, // S_JETJAW_CHOMP12
	{SPR_JJAW, 3, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP14}, // S_JETJAW_CHOMP13
	{SPR_JJAW, 3, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP15}, // S_JETJAW_CHOMP14
	{SPR_JJAW, 3, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP16}, // S_JETJAW_CHOMP15
	{SPR_JJAW, 3, 1, {A_JetJawChomp}, 0, 0, S_JETJAW_CHOMP1},  // S_JETJAW_CHOMP16

	// Snailer
	{SPR_SNLR, 0, 1, {A_SnailerThink}, 0, 0, S_SNAILER1}, // S_SNAILER1

	// Vulture
	{SPR_VLTR, 4, 35,        {A_Look},  1, 0, S_VULTURE_STND},  // S_VULTURE_STND
	{SPR_VLTR, 4, 1,  {A_VultureVtol},  0, 0, S_VULTURE_VTOL2}, // S_VULTURE_VTOL1
	{SPR_VLTR, 5, 1,  {A_VultureVtol},  0, 0, S_VULTURE_VTOL3}, // S_VULTURE_VTOL2
	{SPR_VLTR, 6, 1,  {A_VultureVtol},  0, 0, S_VULTURE_VTOL4}, // S_VULTURE_VTOL3
	{SPR_VLTR, 7, 1,  {A_VultureVtol},  0, 0, S_VULTURE_VTOL1}, // S_VULTURE_VTOL4
	{SPR_VLTR, 0, 1,       {A_Thrust}, 30, 1, S_VULTURE_ZOOM2}, // S_VULTURE_ZOOM1
	{SPR_VLTR, 0, 1, {A_VultureCheck},  0, 0, S_VULTURE_ZOOM3}, // S_VULTURE_ZOOM2
	{SPR_VLTR, 1, 1, {A_VultureCheck},  0, 0, S_VULTURE_ZOOM4}, // S_VULTURE_ZOOM3
	{SPR_VLTR, 2, 1, {A_VultureCheck},  0, 0, S_VULTURE_ZOOM4}, // S_VULTURE_ZOOM4
	{SPR_VLTR, 3, 1, {A_VultureCheck},  0, 0, S_VULTURE_ZOOM2}, // S_VULTURE_ZOOM5

	// Pointy
	{SPR_PNTY, 0,  1, {A_PointyThink}, 0, 0, S_POINTY1}, // S_POINTY1

	// Pointy Ball
	{SPR_PNTY, 1,  1, {A_CheckBuddy}, 0, 0, S_POINTYBALL1}, // S_POINTYBALL1

	// Robo-Hood
	{SPR_ARCH, 0, 14,       {A_Look}, (512<<16),   0, S_ROBOHOOD_LOOK},  // S_ROBOHOOD_LOOK
	{SPR_ARCH, 0,  1,  {A_HoodThink},         0,   0, S_ROBOHOOD_STND},  // S_ROBOHOOD_STND
	{SPR_ARCH, 0, 35,   {A_FireShot},  MT_ARROW, -24, S_ROBOHOOD_JUMP},  // S_ROBOHOOD_SHOOT
	{SPR_ARCH, 1,  1,         {NULL},         0,   0, S_ROBOHOOD_JUMP2}, // S_ROBOHOOD_JUMP
	{SPR_ARCH, 1,  1,  {A_HoodThink},         0,   0, S_ROBOHOOD_JUMP2}, // S_ROBOHOOD_JUMP2
	{SPR_ARCH, 0,  1,  {A_HoodThink},         0,   0, S_ROBOHOOD_FALL},  // S_ROBOHOOD_FALL

	// CastleBot FaceStabber
	{SPR_CBFS, 0,  1,           {A_Chase},  0, 0, S_FACESTABBER_STND2},   // S_FACESTABBER_STND1
	{SPR_CBFS, 1,  1,           {A_Chase},  0, 0, S_FACESTABBER_STND3},   // S_FACESTABBER_STND2
	{SPR_CBFS, 2,  1,           {A_Chase},  0, 0, S_FACESTABBER_STND4},   // S_FACESTABBER_STND3
	{SPR_CBFS, 3,  1,           {A_Chase},  0, 0, S_FACESTABBER_STND5},   // S_FACESTABBER_STND4
	{SPR_CBFS, 4,  1,           {A_Chase},  0, 0, S_FACESTABBER_STND6},   // S_FACESTABBER_STND5
	{SPR_CBFS, 5,  1,           {A_Chase},  0, 0, S_FACESTABBER_STND1},   // S_FACESTABBER_STND6
	{SPR_CBFS, 6, 14, {A_PlayActiveSound},  0, 0, S_FACESTABBER_CHARGE2}, // S_FACESTABBER_CHARGE1
	{SPR_CBFS, 6,  0, {A_PlayAttackSound},  0, 0, S_FACESTABBER_CHARGE3}, // S_FACESTABBER_CHARGE2
	{SPR_CBFS, 6,  0,      {A_FaceTarget},  0, 0, S_FACESTABBER_CHARGE4}, // S_FACESTABBER_CHARGE3
	{SPR_CBFS, 7, 35,          {A_Thrust}, 20, 1, S_FACESTABBER_STND1},   // S_FACESTABBER_CHARGE4

	// Egg Guard
	{SPR_SPSH,  0,  1,       {A_Look}, 0, 0, S_EGGGUARD_STND},  // S_EGGGUARD_STND
	{SPR_SPSH,  1,  3, {A_GuardChase}, 0, 0, S_EGGGUARD_WALK2}, // S_EGGGUARD_WALK1
	{SPR_SPSH,  2,  3, {A_GuardChase}, 0, 0, S_EGGGUARD_WALK3}, // S_EGGGUARD_WALK2
	{SPR_SPSH,  3,  3, {A_GuardChase}, 0, 0, S_EGGGUARD_WALK4}, // S_EGGGUARD_WALK3
	{SPR_SPSH,  4,  3, {A_GuardChase}, 0, 0, S_EGGGUARD_WALK1}, // S_EGGGUARD_WALK4
	{SPR_SPSH,  5,  5,         {NULL}, 0, 0, S_EGGGUARD_MAD2},  // S_EGGGUARD_MAD1
	{SPR_SPSH,  6,  5,         {NULL}, 0, 0, S_EGGGUARD_MAD3},  // S_EGGGUARD_MAD2
	{SPR_SPSH,  7, 15,         {NULL}, 0, 0, S_EGGGUARD_RUN1},  // S_EGGGUARD_MAD3
	{SPR_SPSH,  8,  1, {A_GuardChase}, 0, 0, S_EGGGUARD_RUN2},  // S_EGGGUARD_RUN1
	{SPR_SPSH,  9,  1, {A_GuardChase}, 0, 0, S_EGGGUARD_RUN3},  // S_EGGGUARD_RUN2
	{SPR_SPSH, 10,  1, {A_GuardChase}, 0, 0, S_EGGGUARD_RUN4},  // S_EGGGUARD_RUN3
	{SPR_SPSH, 11,  1, {A_GuardChase}, 0, 0, S_EGGGUARD_RUN1},  // S_EGGGUARD_RUN4

	// Egg Shield for Egg Guard
	{SPR_ESHI, 0, -1, {NULL}, 0, 0, S_DISS}, // S_EGGSHIELD

	// Green Snapper
	{SPR_GSNP, 0, 1,  {A_Look}, 0, 0, S_GSNAPPER_STND}, // S_GSNAPPER_STND
	{SPR_GSNP, 0, 2, {A_Chase}, 0, 0, S_GSNAPPER2},     // S_GSNAPPER1
	{SPR_GSNP, 1, 2, {A_Chase}, 0, 0, S_GSNAPPER3},     // S_GSNAPPER2
	{SPR_GSNP, 2, 2, {A_Chase}, 0, 0, S_GSNAPPER4},     // S_GSNAPPER3
	{SPR_GSNP, 3, 2, {A_Chase}, 0, 0, S_GSNAPPER1},     // S_GSNAPPER4

	// Minus
	{SPR_DISS,  0, 1, {A_Look},         0, 0, S_MINUS_STND},      // S_MINUS_STND
	{SPR_MNUS, 16, 1, {A_MinusDigging}, 0, 0, S_MINUS_DIGGING},   // S_MINUS_DIGGING
	{SPR_MNUS,  0, 1, {A_MinusPopup},   0, 0, S_MINUS_UPWARD1},   // S_MINUS_POPUP
	{SPR_MNUS,  0, 1, {A_MinusCheck},   0, 0, S_MINUS_UPWARD2},   // S_MINUS_UPWARD1
	{SPR_MNUS,  1, 1, {A_MinusCheck},   0, 0, S_MINUS_UPWARD3},   // S_MINUS_UPWARD2
	{SPR_MNUS,  2, 1, {A_MinusCheck},   0, 0, S_MINUS_UPWARD4},   // S_MINUS_UPWARD3
	{SPR_MNUS,  3, 1, {A_MinusCheck},   0, 0, S_MINUS_UPWARD5},   // S_MINUS_UPWARD4
	{SPR_MNUS,  4, 1, {A_MinusCheck},   0, 0, S_MINUS_UPWARD6},   // S_MINUS_UPWARD5
	{SPR_MNUS,  5, 1, {A_MinusCheck},   0, 0, S_MINUS_UPWARD7},   // S_MINUS_UPWARD6
	{SPR_MNUS,  6, 1, {A_MinusCheck},   0, 0, S_MINUS_UPWARD8},   // S_MINUS_UPWARD7
	{SPR_MNUS,  7, 1, {A_MinusCheck},   0, 0, S_MINUS_UPWARD1},   // S_MINUS_UPWARD8
	{SPR_MNUS,  8, 1, {A_MinusCheck},   0, 0, S_MINUS_DOWNWARD2}, // S_MINUS_DOWNWARD1
	{SPR_MNUS,  9, 1, {A_MinusCheck},   0, 0, S_MINUS_DOWNWARD3}, // S_MINUS_DOWNWARD2
	{SPR_MNUS, 10, 1, {A_MinusCheck},   0, 0, S_MINUS_DOWNWARD4}, // S_MINUS_DOWNWARD3
	{SPR_MNUS, 11, 1, {A_MinusCheck},   0, 0, S_MINUS_DOWNWARD5}, // S_MINUS_DOWNWARD4
	{SPR_MNUS, 12, 1, {A_MinusCheck},   0, 0, S_MINUS_DOWNWARD6}, // S_MINUS_DOWNWARD5
	{SPR_MNUS, 13, 1, {A_MinusCheck},   0, 0, S_MINUS_DOWNWARD7}, // S_MINUS_DOWNWARD6
	{SPR_MNUS, 14, 1, {A_MinusCheck},   0, 0, S_MINUS_DOWNWARD8}, // S_MINUS_DOWNWARD7
	{SPR_MNUS, 15, 1, {A_MinusCheck},   0, 0, S_MINUS_DOWNWARD1}, // S_MINUS_DOWNWARD8

	// Boss Explosion
	{SPR_BPLD, 32768, 5, {NULL}, 0, 0, S_BPLD2}, // S_BPLD1
	{SPR_BPLD, 32769, 5, {NULL}, 0, 0, S_BPLD3}, // S_BPLD2
	{SPR_BPLD, 32770, 5, {NULL}, 0, 0, S_BPLD4}, // S_BPLD3
	{SPR_BPLD, 32771, 5, {NULL}, 0, 0, S_BPLD5}, // S_BPLD4
	{SPR_BPLD, 32772, 5, {NULL}, 0, 0, S_BPLD6}, // S_BPLD5
	{SPR_BPLD, 32773, 5, {NULL}, 0, 0, S_BPLD7}, // S_BPLD6
	{SPR_BPLD, 32774, 5, {NULL}, 0, 0, S_DISS},  // S_BPLD7

	{SPR_JETF, 32768, 1, {NULL}, 0, 0, S_JETFUME2}, // S_JETFUME1
	{SPR_DISS, 32768, 1, {NULL}, 0, 0, S_JETFUME1}, // S_JETFUME2

	// Boss 1
	{SPR_EGGM,  0,   1, {NULL},                    0, 0, S_EGGMOBILE_STND},   // S_EGGMOBILE_STND
	{SPR_EGGM,  1,   3, {NULL},                    0, 0, S_EGGMOBILE_LATK2},  // S_EGGMOBILE_LATK1
	{SPR_EGGM,  2,  30, {NULL},                    0, 0, S_EGGMOBILE_LATK3},  // S_EGGMOBILE_LATK2
	{SPR_EGGM,  3,   2, {NULL},                    0, 0, S_EGGMOBILE_LATK4},  // S_EGGMOBILE_LATK3
	{SPR_EGGM,  4,   1, {NULL},                    0, 0, S_EGGMOBILE_LATK5},  // S_EGGMOBILE_LATK4
	{SPR_EGGM,  5,   1, {NULL},                    0, 0, S_EGGMOBILE_LATK6},  // S_EGGMOBILE_LATK5
	{SPR_EGGM,  6,   1, {NULL},                    0, 0, S_EGGMOBILE_LATK7},  // S_EGGMOBILE_LATK6
	{SPR_EGGM,  7,   1, {NULL},                    0, 0, S_EGGMOBILE_LATK8},  // S_EGGMOBILE_LATK7
	{SPR_EGGM,  8,  15, {A_BossFireShot},  MT_ROCKET, 0, S_EGGMOBILE_LATK9},  // S_EGGMOBILE_LATK8
	{SPR_EGGM,  9,  10, {NULL},                    0, 0, S_EGGMOBILE_LATK10}, // S_EGGMOBILE_LATK9
	{SPR_EGGM, 10,   2, {NULL},                    0, 0, S_EGGMOBILE_STND},   // S_EGGMOBILE_LATK10
	{SPR_EGGM, 11,   3, {NULL},                    0, 0, S_EGGMOBILE_RATK2},  // S_EGGMOBILE_RATK1
	{SPR_EGGM, 12,  30, {NULL},                    0, 0, S_EGGMOBILE_RATK3},  // S_EGGMOBILE_RATK2
	{SPR_EGGM, 13,   2, {NULL},                    0, 0, S_EGGMOBILE_RATK4},  // S_EGGMOBILE_RATK3
	{SPR_EGGM, 14,   1, {NULL},                    0, 0, S_EGGMOBILE_RATK5},  // S_EGGMOBILE_RATK4
	{SPR_EGGM, 15,   1, {NULL},                    0, 0, S_EGGMOBILE_RATK6},  // S_EGGMOBILE_RATK5
	{SPR_EGGM, 16,   1, {NULL},                    0, 0, S_EGGMOBILE_RATK7},  // S_EGGMOBILE_RATK6
	{SPR_EGGM, 17,   1, {NULL},                    0, 0, S_EGGMOBILE_RATK8},  // S_EGGMOBILE_RATK7
	{SPR_EGGM, 18,  15, {A_BossFireShot},  MT_ROCKET, 1, S_EGGMOBILE_RATK9},  // S_EGGMOBILE_RATK8
	{SPR_EGGM, 19,  10, {NULL},                    0, 0, S_EGGMOBILE_RATK10}, // S_EGGMOBILE_RATK9
	{SPR_EGGM, 20,   2, {NULL},                    0, 0, S_EGGMOBILE_STND},   // S_EGGMOBILE_RATK10
	{SPR_EGGM,  3,  35, {NULL},                    0, 0, S_EGGMOBILE_PANIC2}, // S_EGGMOBILE_PANIC1
	{SPR_EGGM,  0,  35, {A_SkullAttack},           0, 0, S_EGGMOBILE_STND},   // S_EGGMOBILE_PANIC2
	{SPR_EGGM, 21,  24, {A_Pain},                  0, 0, S_EGGMOBILE_PAIN2},  // S_EGGMOBILE_PAIN
	{SPR_EGGM, 21,  16, {A_SkullAttack},           1, 0, S_EGGMOBILE_STND},   // S_EGGMOBILE_PAIN2
	{SPR_EGGM, 22,  8, {A_Fall},                   0, 0, S_EGGMOBILE_DIE2},   // S_EGGMOBILE_DIE1
	{SPR_EGGM, 22,  8, {A_BossScream},             0, 0, S_EGGMOBILE_DIE3},   // S_EGGMOBILE_DIE2
	{SPR_EGGM, 22,  8, {A_BossScream},             0, 0, S_EGGMOBILE_DIE4},   // S_EGGMOBILE_DIE3
	{SPR_EGGM, 22,  8, {A_BossScream},             0, 0, S_EGGMOBILE_DIE5},   // S_EGGMOBILE_DIE4
	{SPR_EGGM, 22,  8, {A_BossScream},             0, 0, S_EGGMOBILE_DIE6},   // S_EGGMOBILE_DIE5
	{SPR_EGGM, 22,  8, {A_BossScream},             0, 0, S_EGGMOBILE_DIE7},   // S_EGGMOBILE_DIE6
	{SPR_EGGM, 22,  8, {A_BossScream},             0, 0, S_EGGMOBILE_DIE8},   // S_EGGMOBILE_DIE7
	{SPR_EGGM, 22,  8, {A_BossScream},             0, 0, S_EGGMOBILE_DIE9},   // S_EGGMOBILE_DIE8
	{SPR_EGGM, 22,  8, {A_BossScream},             0, 0, S_EGGMOBILE_DIE10},  // S_EGGMOBILE_DIE9
	{SPR_EGGM, 22,  8, {A_BossScream},             0, 0, S_EGGMOBILE_DIE11},  // S_EGGMOBILE_DIE10
	{SPR_EGGM, 22,  8, {A_BossScream},             0, 0, S_EGGMOBILE_DIE12},  // S_EGGMOBILE_DIE11
	{SPR_EGGM, 22,  8, {A_BossScream},             0, 0, S_EGGMOBILE_DIE13},  // S_EGGMOBILE_DIE12
	{SPR_EGGM, 22,  8, {A_BossScream},             0, 0, S_EGGMOBILE_DIE14},  // S_EGGMOBILE_DIE13
	{SPR_EGGM, 22,  -1, {A_BossDeath},             0, 0, S_NULL},             // S_EGGMOBILE_DIE14
	{SPR_EGGM, 23,  5, {NULL},                     0, 0, S_EGGMOBILE_FLEE2},  // S_EGGMOBILE_FLEE1
	{SPR_EGGM, 24,  5, {NULL},                     0, 0, S_EGGMOBILE_FLEE1},  // S_EGGMOBILE_FLEE2

	// Boss 2
	{SPR_EGGN, 0, -1,              {NULL},           0,          0, S_NULL},             // S_EGGMOBILE2_STND
	{SPR_EGGN, 1, 1,               {NULL},           0,          0, S_EGGMOBILE2_POGO2}, // S_EGGMOBILE2_POGO1
	{SPR_EGGN, 0, 1,     {A_Boss2PogoSFX}, 12*FRACUNIT, 5*FRACUNIT, S_EGGMOBILE2_POGO3}, // S_EGGMOBILE2_POGO2
	{SPR_EGGN, 1, 1,               {NULL},           0,          0, S_EGGMOBILE2_POGO4}, // S_EGGMOBILE2_POGO3
	{SPR_EGGN, 2, -1,              {NULL},           0,          0, S_NULL},             // S_EGGMOBILE2_POGO4
	{SPR_EGGN, 3, 23,  {A_Invinciblerize},           0,          0, S_EGGMOBILE2_PAIN2}, // S_EGGMOBILE2_PAIN
	{SPR_EGGN, 3, 1, {A_DeInvinciblerize},           0,          0, S_EGGMOBILE2_STND},  // S_EGGMOBILE2_PAIN2
	{SPR_EGGN, 4, 8,             {A_Fall},           0,          0, S_EGGMOBILE2_DIE2},  // S_EGGMOBILE2_DIE1
	{SPR_EGGN, 4, 8,       {A_BossScream},           0,          0, S_EGGMOBILE2_DIE3},  // S_EGGMOBILE2_DIE2
	{SPR_EGGN, 4, 8,       {A_BossScream},           0,          0, S_EGGMOBILE2_DIE4},  // S_EGGMOBILE2_DIE3
	{SPR_EGGN, 4, 8,       {A_BossScream},           0,          0, S_EGGMOBILE2_DIE5},  // S_EGGMOBILE2_DIE4
	{SPR_EGGN, 4, 8,       {A_BossScream},           0,          0, S_EGGMOBILE2_DIE6},  // S_EGGMOBILE2_DIE5
	{SPR_EGGN, 4, 8,       {A_BossScream},           0,          0, S_EGGMOBILE2_DIE7},  // S_EGGMOBILE2_DIE6
	{SPR_EGGN, 4, 8,       {A_BossScream},           0,          0, S_EGGMOBILE2_DIE8},  // S_EGGMOBILE2_DIE7
	{SPR_EGGN, 4, 8,       {A_BossScream},           0,          0, S_EGGMOBILE2_DIE9},  // S_EGGMOBILE2_DIE8
	{SPR_EGGN, 4, 8,       {A_BossScream},           0,          0, S_EGGMOBILE2_DIE10}, // S_EGGMOBILE2_DIE9
	{SPR_EGGN, 4, 8,       {A_BossScream},           0,          0, S_EGGMOBILE2_DIE11}, // S_EGGMOBILE2_DIE10
	{SPR_EGGN, 4, 8,       {A_BossScream},           0,          0, S_EGGMOBILE2_DIE12}, // S_EGGMOBILE2_DIE11
	{SPR_EGGN, 4, 8,       {A_BossScream},           0,          0, S_EGGMOBILE2_DIE13}, // S_EGGMOBILE2_DIE12
	{SPR_EGGN, 4, 8,       {A_BossScream},           0,          0, S_EGGMOBILE2_DIE14}, // S_EGGMOBILE2_DIE13
	{SPR_EGGN, 4, -1,       {A_BossDeath},           0,          0, S_NULL},             // S_EGGMOBILE2_DIE14
	{SPR_EGGN, 5, 5,               {NULL},           0,          0, S_EGGMOBILE2_FLEE2}, // S_EGGMOBILE2_FLEE1
	{SPR_EGGN, 6, 5,               {NULL},           0,          0, S_EGGMOBILE2_FLEE1}, // S_EGGMOBILE2_FLEE2

	{SPR_TNKA, 0, 35, {NULL}, 0, 0, S_DISS}, // S_BOSSTANK1
	{SPR_TNKB, 0, 35, {NULL}, 0, 0, S_DISS}, // S_BOSSTANK2
	{SPR_SPNK, 0, 35, {NULL}, 0, 0, S_DISS}, // S_BOSSSPIGOT

	// Boss 2 Goop
	{SPR_GOOP, 32768, 2, {NULL}, 0, 0, S_GOOP2}, // S_GOOP1
	{SPR_GOOP, 32769, 2, {NULL}, 0, 0, S_GOOP1}, // S_GOOP2
	{SPR_GOOP, 32770, -1, {NULL}, 0, 0, S_DISS}, // S_GOOP3

	// Boss 3
	{SPR_EGGO,  0,   1, {NULL},                    0, 0, S_EGGMOBILE3_STND},    // S_EGGMOBILE3_STND
	{SPR_EGGO,  1,   2, {NULL},                    0, 0, S_EGGMOBILE3_ATK2},    // S_EGGMOBILE3_ATK1
	{SPR_EGGO,  2,   2, {NULL},                    0, 0, S_EGGMOBILE3_ATK3A},   // S_EGGMOBILE3_ATK2
	{SPR_EGGO,  3,   2, {A_BossFireShot}, MT_TORPEDO, 2, S_EGGMOBILE3_ATK3B},   // S_EGGMOBILE3_ATK3A
	{SPR_EGGO,  3,   2, {A_BossFireShot}, MT_TORPEDO, 4, S_EGGMOBILE3_ATK3C},   // S_EGGMOBILE3_ATK3B
	{SPR_EGGO,  3,   2, {A_BossFireShot}, MT_TORPEDO, 3, S_EGGMOBILE3_ATK3D},   // S_EGGMOBILE3_ATK3C
	{SPR_EGGO,  3,   2, {A_BossFireShot}, MT_TORPEDO, 5, S_EGGMOBILE3_ATK4},    // S_EGGMOBILE3_ATK3D
	{SPR_EGGO,  4,   2, {NULL},                    0, 0, S_EGGMOBILE3_ATK5},    // S_EGGMOBILE3_ATK4
	{SPR_EGGO,  5,   2, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH1},  // S_EGGMOBILE3_ATK5
	{SPR_EGGO,  6,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH2},  // S_EGGMOBILE3_LAUGH1
	{SPR_EGGO,  7,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH3},  // S_EGGMOBILE3_LAUGH2
	{SPR_EGGO,  6,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH4},  // S_EGGMOBILE3_LAUGH3
	{SPR_EGGO,  7,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH5},  // S_EGGMOBILE3_LAUGH4
	{SPR_EGGO,  6,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH6},  // S_EGGMOBILE3_LAUGH5
	{SPR_EGGO,  7,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH7},  // S_EGGMOBILE3_LAUGH6
	{SPR_EGGO,  6,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH8},  // S_EGGMOBILE3_LAUGH7
	{SPR_EGGO,  7,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH9},  // S_EGGMOBILE3_LAUGH8
	{SPR_EGGO,  6,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH10}, // S_EGGMOBILE3_LAUGH9
	{SPR_EGGO,  7,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH11}, // S_EGGMOBILE3_LAUGH10
	{SPR_EGGO,  6,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH12}, // S_EGGMOBILE3_LAUGH11
	{SPR_EGGO,  7,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH13}, // S_EGGMOBILE3_LAUGH12
	{SPR_EGGO,  6,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH14}, // S_EGGMOBILE3_LAUGH13
	{SPR_EGGO,  7,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH15}, // S_EGGMOBILE3_LAUGH14
	{SPR_EGGO,  6,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH16}, // S_EGGMOBILE3_LAUGH15
	{SPR_EGGO,  7,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH17}, // S_EGGMOBILE3_LAUGH16
	{SPR_EGGO,  6,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH18}, // S_EGGMOBILE3_LAUGH17
	{SPR_EGGO,  7,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH19}, // S_EGGMOBILE3_LAUGH18
	{SPR_EGGO,  6,   4, {NULL},                    0, 0, S_EGGMOBILE3_LAUGH20}, // S_EGGMOBILE3_LAUGH19
	{SPR_EGGO,  7,   4, {NULL},                    0, 0, S_EGGMOBILE3_ATK1},    // S_EGGMOBILE3_LAUGH20
	{SPR_EGGO,  8,   1, {A_Boss3TakeDamage},       0, 0, S_EGGMOBILE3_PAIN2},   // S_EGGMOBILE3_PAIN
	{SPR_EGGO,  8,  23, {A_Pain},                  0, 0, S_EGGMOBILE3_STND},    // S_EGGMOBILE3_PAIN2
	{SPR_EGGO,  9,   8, {A_Fall},                  0, 0, S_EGGMOBILE3_DIE2},    // S_EGGMOBILE3_DIE1
	{SPR_EGGO,  9,   8, {A_BossScream},            0, 0, S_EGGMOBILE3_DIE3},    // S_EGGMOBILE3_DIE2
	{SPR_EGGO,  9,   8, {A_BossScream},            0, 0, S_EGGMOBILE3_DIE4},    // S_EGGMOBILE3_DIE3
	{SPR_EGGO,  9,   8, {A_BossScream},            0, 0, S_EGGMOBILE3_DIE5},    // S_EGGMOBILE3_DIE4
	{SPR_EGGO,  9,   8, {A_BossScream},            0, 0, S_EGGMOBILE3_DIE6},    // S_EGGMOBILE3_DIE5
	{SPR_EGGO,  9,   8, {A_BossScream},            0, 0, S_EGGMOBILE3_DIE7},    // S_EGGMOBILE3_DIE6
	{SPR_EGGO,  9,   8, {A_BossScream},            0, 0, S_EGGMOBILE3_DIE8},    // S_EGGMOBILE3_DIE7
	{SPR_EGGO,  9,   8, {A_BossScream},            0, 0, S_EGGMOBILE3_DIE9},    // S_EGGMOBILE3_DIE8
	{SPR_EGGO,  9,   8, {A_BossScream},            0, 0, S_EGGMOBILE3_DIE10},   // S_EGGMOBILE3_DIE9
	{SPR_EGGO,  9,   8, {A_BossScream},            0, 0, S_EGGMOBILE3_DIE11},   // S_EGGMOBILE3_DIE10
	{SPR_EGGO,  9,   8, {A_BossScream},            0, 0, S_EGGMOBILE3_DIE12},   // S_EGGMOBILE3_DIE11
	{SPR_EGGO,  9,   8, {A_BossScream},            0, 0, S_EGGMOBILE3_DIE13},   // S_EGGMOBILE3_DIE12
	{SPR_EGGO,  9,   8, {A_BossScream},            0, 0, S_EGGMOBILE3_DIE14},   // S_EGGMOBILE3_DIE13
	{SPR_EGGO,  9,  -1,  {A_BossDeath},            0, 0, S_NULL},               // S_EGGMOBILE3_DIE14
	{SPR_EGGO, 10,   5, {A_LinedefExecute},       101, 0, S_EGGMOBILE3_FLEE2},   // S_EGGMOBILE3_FLEE1
	{SPR_EGGO, 11,   5, {NULL},                     0, 0, S_EGGMOBILE3_FLEE1},   // S_EGGMOBILE3_FLEE2

	// Boss 3 Propeller
	{SPR_PRPL, 0, 1, {NULL}, 0, 0, S_PROPELLER2}, // S_PROPELLER1
	{SPR_PRPL, 1, 1, {NULL}, 0, 0, S_PROPELLER3}, // S_PROPELLER2
	{SPR_PRPL, 2, 1, {NULL}, 0, 0, S_PROPELLER4}, // S_PROPELLER3
	{SPR_PRPL, 3, 1, {NULL}, 0, 0, S_PROPELLER5}, // S_PROPELLER4
	{SPR_PRPL, 4, 1, {NULL}, 0, 0, S_PROPELLER6}, // S_PROPELLER5
	{SPR_PRPL, 5, 1, {NULL}, 0, 0, S_PROPELLER7}, // S_PROPELLER6
	{SPR_PRPL, 6, 1, {NULL}, 0, 0, S_PROPELLER1}, // S_PROPELLER7

	// Boss 4
	{SPR_EGGP, 0, -1, {NULL}, 0, 0, S_EGGMOBILE4_STND}, // S_EGGMOBILE4_STND

	{SPR_BRAK, 0, 1, {A_SetReactionTime}, 0, 0, S_BLACKEGG_STND2}, // S_BLACKEGG_STND
	{SPR_BRAK, 0, 7, {A_Look}, 1, 0, S_BLACKEGG_STND2}, // S_BLACKEGG_STND2
	{SPR_BRAK, 1, 7, {NULL}, 0, 0, S_BLACKEGG_WALK2}, // S_BLACKEGG_WALK1
	{SPR_BRAK, 2, 7, {NULL}, 0, 0, S_BLACKEGG_WALK3}, // S_BLACKEGG_WALK2
	{SPR_BRAK, 3, 7, {A_PlaySound}, sfx_bestep, 0, S_BLACKEGG_WALK4}, // S_BLACKEGG_WALK3
	{SPR_BRAK, 4, 7, {NULL}, 0, 0, S_BLACKEGG_WALK5}, // S_BLACKEGG_WALK4
	{SPR_BRAK, 5, 7, {NULL}, 0, 0, S_BLACKEGG_WALK6}, // S_BLACKEGG_WALK5
	{SPR_BRAK, 6, 7, {A_PlaySound}, sfx_bestp2, 0, S_BLACKEGG_WALK1}, // S_BLACKEGG_WALK6
	{SPR_BRAK, 7, 3, {NULL}, 0, 0, S_BLACKEGG_SHOOT2}, // S_BLACKEGG_SHOOT1
	{SPR_BRAK, 24, 1, {A_PlaySound}, sfx_befire, 0, S_BLACKEGG_SHOOT1}, // S_BLACKEGG_SHOOT2

	{SPR_BRAK, 8, 1, {A_PlaySound}, sfx_behurt, 0, S_BLACKEGG_PAIN2}, // S_BLACKEGG_PAIN1
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN3}, // S_BLACKEGG_PAIN2
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN4}, // S_BLACKEGG_PAIN3
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN5}, // S_BLACKEGG_PAIN4
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN6}, // S_BLACKEGG_PAIN5
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN7}, // S_BLACKEGG_PAIN6
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN8}, // S_BLACKEGG_PAIN7
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN9}, // S_BLACKEGG_PAIN8
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN10}, // S_BLACKEGG_PAIN9
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN11}, // S_BLACKEGG_PAIN10
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN12}, // S_BLACKEGG_PAIN11
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN13}, // S_BLACKEGG_PAIN12
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN14}, // S_BLACKEGG_PAIN13
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN15}, // S_BLACKEGG_PAIN14
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN16}, // S_BLACKEGG_PAIN15
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN17}, // S_BLACKEGG_PAIN16
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN18}, // S_BLACKEGG_PAIN17
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN19}, // S_BLACKEGG_PAIN18
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN20}, // S_BLACKEGG_PAIN19
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN21}, // S_BLACKEGG_PAIN20
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN22}, // S_BLACKEGG_PAIN21
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN23}, // S_BLACKEGG_PAIN22
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN24}, // S_BLACKEGG_PAIN23
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN25}, // S_BLACKEGG_PAIN24
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN26}, // S_BLACKEGG_PAIN25
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN27}, // S_BLACKEGG_PAIN26
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN28}, // S_BLACKEGG_PAIN27
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN29}, // S_BLACKEGG_PAIN28
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN30}, // S_BLACKEGG_PAIN29
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN31}, // S_BLACKEGG_PAIN30
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN32}, // S_BLACKEGG_PAIN31
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN33}, // S_BLACKEGG_PAIN32
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN34}, // S_BLACKEGG_PAIN33
	{SPR_BRAK, 25, 1, {NULL}, 0, 0, S_BLACKEGG_PAIN35}, // S_BLACKEGG_PAIN34
	{SPR_BRAK, 8, 1, {NULL}, 0, 0, S_BLACKEGG_WALK1}, // S_BLACKEGG_PAIN35

	{SPR_BRAK, 9, 20, {NULL}, 0, 0, S_BLACKEGG_HITFACE2}, // S_BLACKEGG_HITFACE1
	{SPR_BRAK, 10, 2, {NULL}, 0, 0, S_BLACKEGG_HITFACE3}, // S_BLACKEGG_HITFACE2
	{SPR_BRAK, 11, 2, {NULL}, 0, 0, S_BLACKEGG_HITFACE4}, // S_BLACKEGG_HITFACE3
	{SPR_BRAK, 12,14, {NULL}, 0, 0, S_BLACKEGG_PAIN1}, // S_BLACKEGG_HITFACE4

	{SPR_BRAK, 13, 14, {A_Scream}, sfx_bedie2, 0, S_BLACKEGG_DIE2}, // S_BLACKEGG_DIE1
	{SPR_BRAK, 14, 7, {NULL}, 0, 0, S_BLACKEGG_DIE3}, // S_BLACKEGG_DIE2
	{SPR_BRAK, 15, 5, {NULL}, 0, 0, S_BLACKEGG_DIE4}, // S_BLACKEGG_DIE3
	{SPR_BRAK, 16, 3, {A_PlaySound}, sfx_bgxpld, 0, S_BLACKEGG_DIE5}, // S_BLACKEGG_DIE4
	{SPR_BRAK, 17, -1, {NULL}, 0, 0, S_BLACKEGG_DIE5}, // S_BLACKEGG_DIE5

	{SPR_BRAK, 18, 14, {NULL}, 0, 0, S_BLACKEGG_MISSILE2}, // S_BLACKEGG_MISSILE1
	{SPR_BRAK, 19, 5, {NULL}, 0, 0, S_BLACKEGG_MISSILE3}, // S_BLACKEGG_MISSILE2
	{SPR_BRAK, 20, 35, {A_PlaySound}, sfx_beshot, 0, S_BLACKEGG_JUMP1}, // S_BLACKEGG_MISSILE3

	{SPR_BRAK, 21, -1, {NULL}, 0, 0, S_BLACKEGG_STND}, // S_BLACKEGG_GOOP

	{SPR_BRAK, 22, 14, {A_PlaySound}, sfx_belnch, 0, S_BLACKEGG_JUMP2}, // S_BLACKEGG_JUMP1
	{SPR_BRAK, 23, -1, {NULL}, 0, 0, S_BLACKEGG_WALK1}, // S_BLACKEGG_JUMP2

	{SPR_BRAK, 21, 3*TICRATE, {NULL}, 0, 0, S_BLACKEGG_DESTROYPLAT2}, // S_BLACKEGG_DESTROYPLAT1
	{SPR_BRAK, 21, 1, {A_PlaySound}, sfx_s3k_35, 0, S_BLACKEGG_DESTROYPLAT3}, // S_BLACKEGG_DESTROYPLAT2
	{SPR_BRAK, 21, 14, {A_LinedefExecute}, 4200, 0, S_BLACKEGG_STND}, // S_BLACKEGG_DESTROYPLAT3

	{SPR_DISS, 0, 1, {A_CapeChase}, (160 - 16) << 16, 0, S_BLACKEGG_HELPER}, // S_BLACKEGG_HELPER

	{SPR_BGOO, 0, 2, {NULL}, 0, 0, S_BLACKEGG_GOOP2}, // S_BLACKEGG_GOOP1
	{SPR_BGOO, 1, 2, {NULL}, 0, 0, S_BLACKEGG_GOOP1}, // S_BLACKEGG_GOOP2
	{SPR_BGOO, 2, 6*TICRATE, {A_GoopSplat}, 0, 0, S_BLACKEGG_GOOP4}, // S_BLACKEGG_GOOP3
	{SPR_BGOO, 2, 4, {NULL}, 0, 0, S_BLACKEGG_GOOP5}, // S_BLACKEGG_GOOP4
	{SPR_BGOO, 2, 4, {NULL}, 0, 0, S_BLACKEGG_GOOP6}, // S_BLACKEGG_GOOP5
	{SPR_BGOO, 2, 4, {NULL}, 0, 0, S_BLACKEGG_GOOP7}, // S_BLACKEGG_GOOP6
	{SPR_BGOO, 2, 4, {NULL}, 0, 0, S_DISS}, // S_BLACKEGG_GOOP7

	{SPR_BMSL, 0, 1, {NULL}, 0, 0, S_BLACKEGG_MISSILE}, // S_BLACKEGG_MISSILE

	// Ring
	{SPR_RING,  0, 1, {A_AttractChase}, 0, 0, S_RING2},  // S_RING1
	{SPR_RING,  1, 1, {A_AttractChase}, 0, 0, S_RING3},  // S_RING2
	{SPR_RING,  2, 1, {A_AttractChase}, 0, 0, S_RING4},  // S_RING3
	{SPR_RING,  3, 1, {A_AttractChase}, 0, 0, S_RING5},  // S_RING4
	{SPR_RING,  4, 1, {A_AttractChase}, 0, 0, S_RING6},  // S_RING5
	{SPR_RING,  5, 1, {A_AttractChase}, 0, 0, S_RING7},  // S_RING6
	{SPR_RING,  6, 1, {A_AttractChase}, 0, 0, S_RING8},  // S_RING7
	{SPR_RING,  7, 1, {A_AttractChase}, 0, 0, S_RING9},  // S_RING8
	{SPR_RING,  8, 1, {A_AttractChase}, 0, 0, S_RING10}, // S_RING9
	{SPR_RING,  9, 1, {A_AttractChase}, 0, 0, S_RING11}, // S_RING10
	{SPR_RING, 10, 1, {A_AttractChase}, 0, 0, S_RING12}, // S_RING11
	{SPR_RING, 11, 1, {A_AttractChase}, 0, 0, S_RING13}, // S_RING12
	{SPR_RING, 12, 1, {A_AttractChase}, 0, 0, S_RING14}, // S_RING13
	{SPR_RING, 13, 1, {A_AttractChase}, 0, 0, S_RING15}, // S_RING14
	{SPR_RING, 14, 1, {A_AttractChase}, 0, 0, S_RING16}, // S_RING15
	{SPR_RING, 15, 1, {A_AttractChase}, 0, 0, S_RING17}, // S_RING16
	{SPR_RING, 16, 1, {A_AttractChase}, 0, 0, S_RING18}, // S_RING17
	{SPR_RING, 17, 1, {A_AttractChase}, 0, 0, S_RING19}, // S_RING18
	{SPR_RING, 18, 1, {A_AttractChase}, 0, 0, S_RING20}, // S_RING19
	{SPR_RING, 19, 1, {A_AttractChase}, 0, 0, S_RING21}, // S_RING20
	{SPR_RING, 20, 1, {A_AttractChase}, 0, 0, S_RING22}, // S_RING21
	{SPR_RING, 21, 1, {A_AttractChase}, 0, 0, S_RING23}, // S_RING22
	{SPR_RING, 22, 1, {A_AttractChase}, 0, 0, S_RING24}, // S_RING23
	{SPR_RING, 23, 1, {A_AttractChase}, 0, 0, S_RING1},  // S_RING24

	// Blue Sphere Replacement for special stages
	{SPR_BBAL, 0, 0, {A_AttractChase}, 0, 0, S_BLUEBALL},  // S_BLUEBALL
	{SPR_BBAL, 1, -1, {NULL}, 0, 0, S_DISS},  // S_BLUEBALLSPARK

	// Gravity Well sprites for Egg Rock's Special Stage
	{SPR_GWLG, 0, 1, {NULL}, 0, 0, S_GRAVWELLGREEN2}, // S_GRAVWELLGREEN
	{SPR_GWLG, 1, 1, {NULL}, 0, 0, S_GRAVWELLGREEN3}, // S_GRAVWELLGREEN2
	{SPR_GWLG, 2, 1, {NULL}, 0, 0, S_GRAVWELLGREEN},  // S_GRAVWELLGREEN3

	{SPR_GWLR, 0, 1, {NULL}, 0, 0, S_GRAVWELLRED2},   // S_GRAVWELLRED
	{SPR_GWLR, 1, 1, {NULL}, 0, 0, S_GRAVWELLRED3},   // S_GRAVWELLRED2
	{SPR_GWLR, 2, 1, {NULL}, 0, 0, S_GRAVWELLRED},    // S_GRAVWELLRED3

	// Individual Team Rings (now with shield attracting action! =P)
	{SPR_TRNG,  0, 1, {A_AttractChase}, 0, 0, S_TEAMRING2},  // S_TEAMRING1
	{SPR_TRNG,  1, 1, {A_AttractChase}, 0, 0, S_TEAMRING3},  // S_TEAMRING2
	{SPR_TRNG,  2, 1, {A_AttractChase}, 0, 0, S_TEAMRING4},  // S_TEAMRING3
	{SPR_TRNG,  3, 1, {A_AttractChase}, 0, 0, S_TEAMRING5},  // S_TEAMRING4
	{SPR_TRNG,  4, 1, {A_AttractChase}, 0, 0, S_TEAMRING6},  // S_TEAMRING5
	{SPR_TRNG,  5, 1, {A_AttractChase}, 0, 0, S_TEAMRING7},  // S_TEAMRING6
	{SPR_TRNG,  6, 1, {A_AttractChase}, 0, 0, S_TEAMRING8},  // S_TEAMRING7
	{SPR_TRNG,  7, 1, {A_AttractChase}, 0, 0, S_TEAMRING9},  // S_TEAMRING8
	{SPR_TRNG,  8, 1, {A_AttractChase}, 0, 0, S_TEAMRING10}, // S_TEAMRING9
	{SPR_TRNG,  9, 1, {A_AttractChase}, 0, 0, S_TEAMRING11}, // S_TEAMRING10
	{SPR_TRNG, 10, 1, {A_AttractChase}, 0, 0, S_TEAMRING12}, // S_TEAMRING11
	{SPR_TRNG, 11, 1, {A_AttractChase}, 0, 0, S_TEAMRING13}, // S_TEAMRING12
	{SPR_TRNG, 12, 1, {A_AttractChase}, 0, 0, S_TEAMRING14}, // S_TEAMRING13
	{SPR_TRNG, 13, 1, {A_AttractChase}, 0, 0, S_TEAMRING15}, // S_TEAMRING14
	{SPR_TRNG, 14, 1, {A_AttractChase}, 0, 0, S_TEAMRING16}, // S_TEAMRING15
	{SPR_TRNG, 15, 1, {A_AttractChase}, 0, 0, S_TEAMRING17}, // S_TEAMRING16
	{SPR_TRNG, 16, 1, {A_AttractChase}, 0, 0, S_TEAMRING18}, // S_TEAMRING17
	{SPR_TRNG, 17, 1, {A_AttractChase}, 0, 0, S_TEAMRING19}, // S_TEAMRING18
	{SPR_TRNG, 18, 1, {A_AttractChase}, 0, 0, S_TEAMRING20}, // S_TEAMRING19
	{SPR_TRNG, 19, 1, {A_AttractChase}, 0, 0, S_TEAMRING21}, // S_TEAMRING20
	{SPR_TRNG, 20, 1, {A_AttractChase}, 0, 0, S_TEAMRING22}, // S_TEAMRING21
	{SPR_TRNG, 21, 1, {A_AttractChase}, 0, 0, S_TEAMRING23}, // S_TEAMRING22
	{SPR_TRNG, 22, 1, {A_AttractChase}, 0, 0, S_TEAMRING24}, // S_TEAMRING23
	{SPR_TRNG, 23, 1, {A_AttractChase}, 0, 0, S_TEAMRING1},  // S_TEAMRING24

	// Special Stage Token
	{SPR_EMMY, 32768, 2, {NULL}, 0, 0, S_EMMY2}, // S_EMMY1
	{SPR_EMMY, 32769, 2, {NULL}, 0, 0, S_EMMY3}, // S_EMMY2
	{SPR_EMMY, 32770, 2, {NULL}, 0, 0, S_EMMY4}, // S_EMMY3
	{SPR_EMMY, 32771, 2, {NULL}, 0, 0, S_EMMY5}, // S_EMMY4
	{SPR_EMMY, 32772, 2, {NULL}, 0, 0, S_EMMY6}, // S_EMMY5
	{SPR_EMMY, 32773, 2, {NULL}, 0, 0, S_EMMY7}, // S_EMMY6
	{SPR_EMMY, 32774, 2, {NULL}, 0, 0, S_EMMY1}, // S_EMMY7

	// Special Stage Token
	{SPR_TOKE, 32768, -1, {NULL}, 0, 0, S_DISS}, // S_TOKEN

	// CTF Flags
	{SPR_RFLG, 0, -1, {NULL}, 0, 0, S_NULL}, // S_REDFLAG
	{SPR_BFLG, 0, -1, {NULL}, 0, 0, S_NULL}, // S_BLUEFLAG

	// Emblem
	{SPR_EMBM, 0, -1, {NULL}, 0, 0, S_NULL}, // S_EMBLEM1

	// Chaos Emeralds
	{SPR_CEMG, 32768, -1, {NULL}, 0, 0, S_NULL}, // S_CEMG1
	{SPR_CEMG, 32769, -1, {NULL}, 0, 0, S_NULL}, // S_CEMG2
	{SPR_CEMG, 32770, -1, {NULL}, 0, 0, S_NULL}, // S_CEMG3
	{SPR_CEMG, 32771, -1, {NULL}, 0, 0, S_NULL}, // S_CEMG4
	{SPR_CEMG, 32772, -1, {NULL}, 0, 0, S_NULL}, // S_CEMG5
	{SPR_CEMG, 32773, -1, {NULL}, 0, 0, S_NULL}, // S_CEMG6
	{SPR_CEMG, 32774, -1, {NULL}, 0, 0, S_NULL}, // S_CEMG7

	// Emeralds (for hunt)
	{SPR_EMER, 0, -1, {NULL}, 0, 0, S_NULL}, // S_EMER1

	// Fan
	{SPR_FANS, 0, 1, {NULL}, 0, 0, S_FAN2}, // S_FAN
	{SPR_FANS, 1, 1, {NULL}, 0, 0, S_FAN3}, // S_FAN2
	{SPR_FANS, 2, 1, {NULL}, 0, 0, S_FAN4}, // S_FAN3
	{SPR_FANS, 3, 1, {NULL}, 0, 0, S_FAN5}, // S_FAN4
	{SPR_FANS, 4, 1, {NULL}, 0, 0, S_FAN},  // S_FAN5

	// Bubble Source
	{SPR_BUBL, 0, 8, {A_BubbleSpawn}, 0, 0, S_BUBBLES2}, // S_BUBBLES1
	{SPR_BUBL, 1, 8, {A_BubbleCheck}, 0, 0, S_BUBBLES1}, // S_BUBBLES2

	// Level End Sign
	{SPR_SIGN, 0, 1, {NULL}, 0, 0, S_SIGN2},         // S_SIGN1
	{SPR_SIGN, 1, 1, {NULL}, 0, 0, S_SIGN3},         // S_SIGN2
	{SPR_SIGN, 2, 1, {NULL}, 0, 0, S_SIGN4},         // S_SIGN3
	{SPR_SIGN, 5, 1, {NULL}, 0, 0, S_SIGN5},         // S_SIGN4
	{SPR_SIGN, 0, 1, {NULL}, 0, 0, S_SIGN6},         // S_SIGN5
	{SPR_SIGN, 1, 1, {NULL}, 0, 0, S_SIGN7},         // S_SIGN6
	{SPR_SIGN, 2, 1, {NULL}, 0, 0, S_SIGN8},         // S_SIGN7
	{SPR_SIGN, 3, 1, {NULL}, 0, 0, S_SIGN9},         // S_SIGN8
	{SPR_SIGN, 0, 1, {NULL}, 0, 0, S_SIGN10},        // S_SIGN9
	{SPR_SIGN, 1, 1, {NULL}, 0, 0, S_SIGN11},        // S_SIGN10
	{SPR_SIGN, 2, 1, {NULL}, 0, 0, S_SIGN12},        // S_SIGN11
	{SPR_SIGN, 4, 1, {NULL}, 0, 0, S_SIGN13},        // S_SIGN12
	{SPR_SIGN, 0, 1, {NULL}, 0, 0, S_SIGN14},        // S_SIGN13
	{SPR_SIGN, 1, 1, {NULL}, 0, 0, S_SIGN15},        // S_SIGN14
	{SPR_SIGN, 2, 1, {NULL}, 0, 0, S_SIGN16},        // S_SIGN15
	{SPR_SIGN, 3, 1, {NULL}, 0, 0, S_SIGN17},        // S_SIGN16
	{SPR_SIGN, 0, 1, {NULL}, 0, 0, S_SIGN18},        // S_SIGN17
	{SPR_SIGN, 1, 1, {NULL}, 0, 0, S_SIGN19},        // S_SIGN18
	{SPR_SIGN, 2, 1, {NULL}, 0, 0, S_SIGN20},        // S_SIGN19
	{SPR_SIGN, 6, 1, {NULL}, 0, 0, S_SIGN21},        // S_SIGN20
	{SPR_SIGN, 0, 1, {NULL}, 0, 0, S_SIGN22},        // S_SIGN21
	{SPR_SIGN, 1, 1, {NULL}, 0, 0, S_SIGN23},        // S_SIGN22
	{SPR_SIGN, 2, 1, {NULL}, 0, 0, S_SIGN24},        // S_SIGN23
	{SPR_SIGN, 3, 1, {NULL}, 0, 0, S_SIGN25},        // S_SIGN24
	{SPR_SIGN, 0, 1, {NULL}, 0, 0, S_SIGN26},        // S_SIGN25
	{SPR_SIGN, 1, 1, {NULL}, 0, 0, S_SIGN27},        // S_SIGN26
	{SPR_SIGN, 2, 1, {NULL}, 0, 0, S_SIGN28},        // S_SIGN27
	{SPR_SIGN, 5, 1, {NULL}, 0, 0, S_SIGN29},        // S_SIGN28
	{SPR_SIGN, 0, 1, {NULL}, 0, 0, S_SIGN30},        // S_SIGN29
	{SPR_SIGN, 1, 1, {NULL}, 0, 0, S_SIGN31},        // S_SIGN30
	{SPR_SIGN, 2, 1, {NULL}, 0, 0, S_SIGN32},        // S_SIGN31
	{SPR_SIGN, 3, 1, {NULL}, 0, 0, S_SIGN33},        // S_SIGN32
	{SPR_SIGN, 0, 1, {NULL}, 0, 0, S_SIGN34},        // S_SIGN33
	{SPR_SIGN, 1, 1, {NULL}, 0, 0, S_SIGN35},        // S_SIGN34
	{SPR_SIGN, 2, 1, {NULL}, 0, 0, S_SIGN36},        // S_SIGN35
	{SPR_SIGN, 4, 1, {NULL}, 0, 0, S_SIGN37},        // S_SIGN36
	{SPR_SIGN, 0, 1, {NULL}, 0, 0, S_SIGN38},        // S_SIGN37
	{SPR_SIGN, 1, 1, {NULL}, 0, 0, S_SIGN39},        // S_SIGN38
	{SPR_SIGN, 2, 1, {NULL}, 0, 0, S_SIGN40},        // S_SIGN39
	{SPR_SIGN, 3, 1, {NULL}, 0, 0, S_SIGN41},        // S_SIGN40
	{SPR_SIGN, 0, 1, {NULL}, 0, 0, S_SIGN42},        // S_SIGN41
	{SPR_SIGN, 1, 1, {NULL}, 0, 0, S_SIGN43},        // S_SIGN42
	{SPR_SIGN, 2, 1, {NULL}, 0, 0, S_SIGN44},        // S_SIGN43
	{SPR_SIGN, 6, 1, {NULL}, 0, 0, S_SIGN45},        // S_SIGN44
	{SPR_SIGN, 0, 1, {NULL}, 0, 0, S_SIGN46},        // S_SIGN45
	{SPR_SIGN, 1, 1, {NULL}, 0, 0, S_SIGN47},        // S_SIGN46
	{SPR_SIGN, 2, 1, {NULL}, 0, 0, S_SIGN48},        // S_SIGN47
	{SPR_SIGN, 3, 1, {NULL}, 0, 0, S_SIGN49},        // S_SIGN48
	{SPR_SIGN, 0, 1, {NULL}, 0, 0, S_SIGN50},        // S_SIGN49
	{SPR_SIGN, 1, 1, {NULL}, 0, 0, S_SIGN51},        // S_SIGN50
	{SPR_SIGN, 2, 1, {NULL}, 0, 0, S_SIGN53},        // S_SIGN51
	{SPR_SIGN, 3, -1, {NULL}, 0, 0, S_NULL},         // S_SIGN52 Eggman
	{SPR_SIGN, 0, -1, {A_SignPlayer}, 0, 0, S_NULL}, // S_SIGN53

	// Steam Riser
	{SPR_STEM, 0, 2, {A_SetSolidSteam}, 0, 0, S_STEAM2},   // S_STEAM1
	{SPR_STEM, 1, 2, {A_UnsetSolidSteam}, 0, 0, S_STEAM3}, // S_STEAM2
	{SPR_STEM, 2, 2, {NULL}, 0, 0, S_STEAM4},              // S_STEAM3
	{SPR_STEM, 3, 2, {NULL}, 0, 0, S_STEAM5},              // S_STEAM4
	{SPR_STEM, 4, 2, {NULL}, 0, 0, S_STEAM6},              // S_STEAM5
	{SPR_STEM, 5, 2, {NULL}, 0, 0, S_STEAM7},              // S_STEAM6
	{SPR_STEM, 6, 2, {NULL}, 0, 0, S_STEAM8},              // S_STEAM7
	{SPR_STEM, 7, 18, {NULL}, 0, 0, S_STEAM1},             // S_STEAM8

	// Spike Ball
	{SPR_SPIK, 0, 1, {A_RotateSpikeBall}, 0, 0, S_SPIKEBALL2}, // S_SPIKEBALL1
	{SPR_SPIK, 1, 1, {A_RotateSpikeBall}, 0, 0, S_SPIKEBALL3}, // S_SPIKEBALL2
	{SPR_SPIK, 2, 1, {A_RotateSpikeBall}, 0, 0, S_SPIKEBALL4}, // S_SPIKEBALL3
	{SPR_SPIK, 3, 1, {A_RotateSpikeBall}, 0, 0, S_SPIKEBALL5}, // S_SPIKEBALL4
	{SPR_SPIK, 4, 1, {A_RotateSpikeBall}, 0, 0, S_SPIKEBALL6}, // S_SPIKEBALL5
	{SPR_SPIK, 5, 1, {A_RotateSpikeBall}, 0, 0, S_SPIKEBALL7}, // S_SPIKEBALL6
	{SPR_SPIK, 6, 1, {A_RotateSpikeBall}, 0, 0, S_SPIKEBALL8}, // S_SPIKEBALL7
	{SPR_SPIK, 7, 1, {A_RotateSpikeBall}, 0, 0, S_SPIKEBALL1}, // S_SPIKEBALL8

	// Red Shield's Spawn
	{SPR_SFLM, 32768, 2, {NULL}, 0, 0, S_SPINFIRE2}, // S_SPINFIRE1
	{SPR_SFLM, 32769, 2, {NULL}, 0, 0, S_SPINFIRE3}, // S_SPINFIRE2
	{SPR_SFLM, 32770, 2, {NULL}, 0, 0, S_SPINFIRE4}, // S_SPINFIRE3
	{SPR_SFLM, 32771, 2, {NULL}, 0, 0, S_SPINFIRE5}, // S_SPINFIRE4
	{SPR_SFLM, 32772, 2, {NULL}, 0, 0, S_SPINFIRE6}, // S_SPINFIRE5
	{SPR_SFLM, 32773, 2, {NULL}, 0, 0, S_SPINFIRE1}, // S_SPINFIRE6

	// Ceiling Spike
	{SPR_DSPK, 0, -1, {NULL}, 0, 0, S_NULL}, // S_CEILINGSPIKE

	// Floor Spike
	{SPR_USPK, 0,-1, {NULL}, 0, 0, S_NULL}, // S_FLOORSPIKE

	// Starpost
	{SPR_STPT, 0, -1, {NULL}, 0, 0, S_NULL},       // S_STARPOST1
	{SPR_STPT, 0, 2, {NULL}, 0, 0, S_STARPOST3},   // S_STARPOST2
	{SPR_STPT, 1, 2, {NULL}, 0, 0, S_STARPOST2},   // S_STARPOST3
	{SPR_STPT, 2, 1, {NULL}, 0, 0, S_STARPOST5},   // S_STARPOST4
	{SPR_STPT, 3, 1, {NULL}, 0, 0, S_STARPOST6},   // S_STARPOST5
	{SPR_STPT, 4, 1, {NULL}, 0, 0, S_STARPOST7},   // S_STARPOST6
	{SPR_STPT, 5, 1, {NULL}, 0, 0, S_STARPOST8},   // S_STARPOST7
	{SPR_STPT, 6, 1, {NULL}, 0, 0, S_STARPOST9},   // S_STARPOST8
	{SPR_STPT, 7, 1, {NULL}, 0, 0, S_STARPOST10},  // S_STARPOST9
	{SPR_STPT, 8, 1, {NULL}, 0, 0, S_STARPOST11},  // S_STARPOST10
	{SPR_STPT, 9, 1, {NULL}, 0, 0, S_STARPOST12},  // S_STARPOST11
	{SPR_STPT, 10, 1, {NULL}, 0, 0, S_STARPOST13}, // S_STARPOST12
	{SPR_STPT, 11, 1, {NULL}, 0, 0, S_STARPOST14}, // S_STARPOST13
	{SPR_STPT, 12, 1, {NULL}, 0, 0, S_STARPOST15}, // S_STARPOST14
	{SPR_STPT, 13, 1, {NULL}, 0, 0, S_STARPOST16}, // S_STARPOST15
	{SPR_STPT, 14, 1, {NULL}, 0, 0, S_STARPOST17}, // S_STARPOST16
	{SPR_STPT, 15, 1, {NULL}, 0, 0, S_STARPOST18}, // S_STARPOST17
	{SPR_STPT, 16, 1, {NULL}, 0, 0, S_STARPOST19}, // S_STARPOST18
	{SPR_STPT, 0, 1, {NULL}, 0, 0, S_STARPOST20},  // S_STARPOST19
	{SPR_STPT, 2, 1, {NULL}, 0, 0, S_STARPOST21},  // S_STARPOST20
	{SPR_STPT, 3, 1, {NULL}, 0, 0, S_STARPOST22},  // S_STARPOST21
	{SPR_STPT, 4, 1, {NULL}, 0, 0, S_STARPOST23},  // S_STARPOST22
	{SPR_STPT, 5, 1, {NULL}, 0, 0, S_STARPOST24},  // S_STARPOST23
	{SPR_STPT, 6, 1, {NULL}, 0, 0, S_STARPOST25},  // S_STARPOST24
	{SPR_STPT, 7, 1, {NULL}, 0, 0, S_STARPOST26},  // S_STARPOST25
	{SPR_STPT, 8, 1, {NULL}, 0, 0, S_STARPOST27},  // S_STARPOST26
	{SPR_STPT, 9, 1, {NULL}, 0, 0, S_STARPOST28},  // S_STARPOST27
	{SPR_STPT, 10, 1, {NULL}, 0, 0, S_STARPOST29}, // S_STARPOST28
	{SPR_STPT, 11, 1, {NULL}, 0, 0, S_STARPOST30}, // S_STARPOST29
	{SPR_STPT, 12, 1, {NULL}, 0, 0, S_STARPOST31}, // S_STARPOST30
	{SPR_STPT, 13, 1, {NULL}, 0, 0, S_STARPOST32}, // S_STARPOST31
	{SPR_STPT, 14, 1, {NULL}, 0, 0, S_STARPOST33}, // S_STARPOST32
	{SPR_STPT, 15, 1, {NULL}, 0, 0, S_STARPOST34}, // S_STARPOST33
	{SPR_STPT, 16, 1, {NULL}, 0, 0, S_STARPOST2},  // S_STARPOST34

	// Big floating mine
	{SPR_BMNE, 0, 5, {NULL}, 0, 0, S_BIGMINE2},    // S_BIGMINE1
	{SPR_BMNE, 1, 5, {NULL}, 0, 0, S_BIGMINE3},    // S_BIGMINE2
	{SPR_BMNE, 2, 5, {NULL}, 0, 0, S_BIGMINE4},    // S_BIGMINE3
	{SPR_BMNE, 3, 5, {NULL}, 0, 0, S_BIGMINE5},    // S_BIGMINE4
	{SPR_BMNE, 4, 5, {NULL}, 0, 0, S_BIGMINE6},    // S_BIGMINE5
	{SPR_BMNE, 5, 5, {NULL}, 0, 0, S_BIGMINE7},    // S_BIGMINE6
	{SPR_BMNE, 6, 5, {NULL}, 0, 0, S_BIGMINE8},    // S_BIGMINE7
	{SPR_BMNE, 7, 5, {NULL}, 0, 0, S_BIGMINE1},    // S_BIGMINE8

	// Cannon launcher
	{SPR_DISS, 0, 1,    {A_FindTarget},             0,         0, S_CANNONLAUNCHER2}, // S_CANNONLAUNCHER1
	{SPR_DISS, 0, 1,       {A_LobShot}, MT_CANNONBALL, 4*TICRATE, S_CANNONLAUNCHER3}, // S_CANNONLAUNCHER2
	{SPR_DISS, 0, 2, {A_SetRandomTics},     TICRATE/2, 3*TICRATE, S_CANNONLAUNCHER1}, // S_CANNONLAUNCHER1

	// Super Ring Box
	{SPR_SRBX, 0, 2, {NULL}, 0, 0, S_SUPERRINGBOX1},          // S_SUPERRINGBOX
	{SPR_MTEX, 0, 1, {NULL}, 0, 0, S_SUPERRINGBOX},           // S_SUPERRINGBOX1
	{SPR_SRBX, 1, 18, {A_MonitorPop}, 0, 0, S_SUPERRINGBOX3}, // S_SUPERRINGBOX2
	{SPR_SRBX, 1, 18, {A_RingBox}, 0, 0, S_DISS},             // S_SUPERRINGBOX3

	// Red Team Ring Box
	{SPR_RRBX, 0, 2, {NULL}, 0, 0, S_REDRINGBOX1},          // S_REDRINGBOX
	{SPR_RRBX, 1, 1, {NULL}, 0, 0, S_REDRINGBOX},           // S_REDRINGBOX1

	// Blue Team Ring Box
	{SPR_BRBX, 0, 2, {NULL}, 0, 0, S_BLUERINGBOX1},          // S_BLUERINGBOX
	{SPR_BRBX, 1, 1, {NULL}, 0, 0, S_BLUERINGBOX},           // S_BLUERINGBOX1

	// Super Sneakers Box
	{SPR_SHTV, 0, 2, {NULL}, 0, 0, S_SHTV1},            // S_SHTV
	{SPR_MTEX, 0, 1, {NULL}, 0, 0, S_SHTV},             // S_SHTV1
	{SPR_SHTV, 1, 18, {A_MonitorPop}, 0, 0, S_SHTV3},   // S_SHTV2
	{SPR_SHTV, 1, 18, {A_SuperSneakers}, 0, 0, S_DISS}, // S_SHTV3

	// Invincibility Box
	{SPR_PINV, 0, 2, {NULL}, 0, 0, S_PINV2},            // S_PINV
	{SPR_MTEX, 0, 1, {NULL}, 0, 0, S_PINV},             // S_PINV2
	{SPR_PINV, 1, 18, {A_MonitorPop}, 0, 0, S_PINV4},   // S_PINV3
	{SPR_PINV, 1, 18, {A_Invincibility}, 0, 0, S_DISS}, // S_PINV4

	// 1up Box Remains
	{SPR_MTEX, 0, 1, {A_MonitorPop}, 0, 0, S_PLAY_BOX1C}, // S_PRUP1

	// Ring Shield Box
	{SPR_YLTV, 0, 2, {NULL}, 0, 0, S_YLTV1},           // S_YLTV
	{SPR_MTEX, 0, 1, {NULL}, 0, 0, S_YLTV},            // S_YLTV1
	{SPR_YLTV, 1, 18, {A_MonitorPop}, 0, 0, S_YLTV3},  // S_YLTV2
	{SPR_YLTV, 1, 18, {A_RingShield}, 0, 0, S_DISS},   // S_YLTV3

	// Force Shield Box
	{SPR_BLTV, 0, 2, {NULL}, 0, 0, S_BLTV2},          // S_BLTV1
	{SPR_MTEX, 0, 1, {NULL}, 0, 0, S_BLTV1},          // S_BLTV2
	{SPR_BLTV, 1, 18, {A_MonitorPop}, 0, 0, S_BLTV4}, // S_BLTV3
	{SPR_BLTV, 1, 18, {A_ForceShield}, 0, 0, S_DISS}, // S_BLTV4

	// Bomb Shield Box
	{SPR_BKTV, 0, 2, {NULL}, 0, 0, S_BKTV2},          // S_BKTV1
	{SPR_MTEX, 0, 1, {NULL}, 0, 0, S_BKTV1},          // S_BKTV2
	{SPR_BKTV, 1, 18, {A_MonitorPop}, 0, 0, S_BKTV4}, // S_BKTV3
	{SPR_BKTV, 1, 18, {A_BombShield}, 0, 0, S_DISS},  // S_BKTV4

	// Jump Shield Box
	{SPR_WHTV, 0, 2, {NULL}, 0, 0, S_WHTV2},          // S_WHTV1
	{SPR_MTEX, 0, 1, {NULL}, 0, 0, S_WHTV1},          // S_WHTV2
	{SPR_WHTV, 1, 18, {A_MonitorPop}, 0, 0, S_WHTV4}, // S_WHTV3
	{SPR_WHTV, 1, 18, {A_JumpShield}, 0, 0, S_DISS},  // S_WHTV4

	// Water Shield Box
	{SPR_GRTV, 0, 2, {NULL}, 0, 0, S_GRTV1},          // S_GRTV
	{SPR_MTEX, 0, 1, {NULL}, 0, 0, S_GRTV},           // S_GRTV1
	{SPR_GRTV, 1, 18, {A_MonitorPop}, 0, 0, S_GRTV3}, // S_GRTV2
	{SPR_GRTV, 1, 18, {A_WaterShield}, 0, 0, S_DISS}, // S_GRTV3

	// Eggman Box
	{SPR_EGGB, 0, 2, {NULL}, 0, 0, S_EGGTV2},          // S_EGGTV1
	{SPR_MTEX, 0, 1, {NULL}, 0, 0, S_EGGTV1},          // S_EGGTV2
	{SPR_EGGB, 1, 18, {A_MonitorPop}, 0, 0, S_EGGTV4}, // S_EGGTV3
	{SPR_EGGB, 1, 18, {A_EggmanBox}, 0, 0, S_DISS},    // S_EGGTV4

	// Teleport Box
	{SPR_MIXU, 0, 2, {NULL}, 0, 0, S_MIXUPBOX2},          // S_MIXUPBOX1
	{SPR_MTEX, 0, 1, {NULL}, 0, 0, S_MIXUPBOX1},          // S_MIXUPBOX2
	{SPR_MIXU, 1, 18, {A_MonitorPop}, 0, 0, S_MIXUPBOX4}, // S_MIXUPBOX3
	{SPR_MIXU, 1, 18, {A_MixUp}, 0, 0, S_DISS},           // S_MIXUPBOX4

	// Recycler Box
	{SPR_RECY, 0, 2, {NULL}, 0, 0, S_RECYCLETV2},          // S_RECYCLETV1
	{SPR_MTEX, 0, 1, {NULL}, 0, 0, S_RECYCLETV1},          // S_RECYCLETV2
	{SPR_RECY, 1, 18, {A_MonitorPop}, 0, 0, S_RECYCLETV4}, // S_RECYCLETV3
	{SPR_RECY, 1, 18, {A_RecyclePowers}, 0, 0, S_DISS},      // S_RECYCLETV4

	// Question Box
	{SPR_QUES, 0, 2, {NULL}, 0, 0, S_RANDOMBOX2},   // S_RANDOMBOX1
	{SPR_MTEX, 0, 1, {NULL}, 0, 0, S_RANDOMBOX1},   // S_RANDOMBOX2
	{SPR_QUES, 0, 1, {A_MonitorPop}, 0, 0, S_DISS}, // S_RANDOMBOX3

	// Gravity Boots Box
	{SPR_GBTV, 0, 2, {NULL}, 0, 0, S_GBTV2},          // S_GBTV1
	{SPR_MTEX, 0, 1, {NULL}, 0, 0, S_GBTV1},          // S_GBTV2
	{SPR_GBTV, 1, 18, {A_MonitorPop}, 0, 0, S_GBTV4}, // S_GBTV3
	{SPR_GBTV, 1, 18, {A_GravityBox}, 0, 0, S_DISS},  // S_GBTV4

	// Monitor Explosion
	{SPR_MTEX, 1, 2, {NULL}, 0, 0, S_MONITOREXPLOSION2}, // S_MONITOREXPLOSION1
	{SPR_MTEX, 2, 2, {NULL}, 0, 0, S_MONITOREXPLOSION3}, // S_MONITOREXPLOSION2
	{SPR_MTEX, 3, 2, {NULL}, 0, 0, S_MONITOREXPLOSION4}, // S_MONITOREXPLOSION3
	{SPR_MTEX, 4, 2, {NULL}, 0, 0, S_MONITOREXPLOSION5}, // S_MONITOREXPLOSION4
	{SPR_MTEX, 5, -1, {NULL}, 0, 0, S_NULL},             // S_MONITOREXPLOSION5

	{SPR_MISL, 32768, 1, {A_SmokeTrailer}, MT_SMOK, 0, S_ROCKET}, // S_ROCKET

	{SPR_TORP, 32768, 1, {A_SmokeTrailer}, MT_SMOK, 0, S_TORPEDO}, // S_TORPEDO

	// Skim Mine (also dropped by Jetty-Syn bomber)
	{SPR_MINE, 0, -1, {NULL}, 0, 0, S_NULL},           // S_MINE1
	{SPR_MINE, 1, 1, {A_Fall}, 0, 0, S_MINE_BOOM2},    // S_MINE_BOOM1
	{SPR_MINE, 2, 3, {A_Scream}, 0, 0, S_MINE_BOOM3},  // S_MINE_BOOM2
	{SPR_MINE, 3, 3, {A_Explode}, 0, 0, S_MINE_BOOM4}, // S_MINE_BOOM3
	{SPR_MINE, 4, 3, {NULL}, 0, 0, S_DISS},            // S_MINE_BOOM4

	// Jetty-Syn Bullet
	{SPR_JBUL, 32768, 1, {NULL}, 0, 0, S_JETBULLET2}, // S_JETBULLET1
	{SPR_JBUL, 32769, 1, {NULL}, 0, 0, S_JETBULLET1}, // S_JETBULLET2

	{SPR_TRLS, 32768, 1, {NULL}, 0, 0, S_TURRETLASER},          // S_TURRETLASER
	{SPR_TRLS, 32769, 2, {NULL}, 0, 0, S_TURRETLASEREXPLODE2},  // S_TURRETLASEREXPLODE1
	{SPR_TRLS, 32770, 2, {NULL}, 0, 0, S_DISS},                 // S_TURRETLASEREXPLODE2

	{SPR_CBLL, 0, -1, {NULL}, 0, 0, S_DISS}, // S_CANNONBALL1

	{SPR_AROW, 0, 1, {A_ArrowCheck}, 0, 0, S_ARROW},     // S_ARROW
	{SPR_AROW, 1, 1, {A_ArrowCheck}, 0, 0, S_ARROWUP},   // S_ARROWUP
	{SPR_AROW, 2, 1, {A_ArrowCheck}, 0, 0, S_ARROWDOWN}, // S_ARROWDOWN

	// GFZ Flower
	{SPR_FWR1, 0, 14, {NULL}, 0, 0, S_GFZFLOWERA2}, // S_GFZFLOWERA
	{SPR_FWR1, 1, 14, {NULL}, 0, 0, S_GFZFLOWERA},  // S_GFZFLOWERA2

	{SPR_FWR2, 0, 7, {NULL}, 0, 0, S_GFZFLOWERB2}, // S_GFZFLOWERB1
	{SPR_FWR2, 1, 7, {NULL}, 0, 0, S_GFZFLOWERB1}, // S_GFZFLOWERB1

	{SPR_FWR3, 0, -1, {NULL}, 0, 0, S_NULL},       // S_GFZFLOWERC1

	{SPR_BUS1, 0, -1, {NULL}, 0, 0, S_NULL},       // S_BERRYBUSH
	{SPR_BUS2, 0, -1, {NULL}, 0, 0, S_NULL},       // S_BUSH

	{SPR_THZP, 0, 4, {NULL}, 0, 0, S_THZPLANT2}, // S_THZPLANT1
	{SPR_THZP, 1, 4, {NULL}, 0, 0, S_THZPLANT3}, // S_THZPLANT1
	{SPR_THZP, 2, 4, {NULL}, 0, 0, S_THZPLANT4}, // S_THZPLANT1
	{SPR_THZP, 3, 4, {NULL}, 0, 0, S_THZPLANT1}, // S_THZPLANT1

	// THZ Alarm
	{SPR_ALRM, 32768, 35, {A_Scream}, 0, 0, S_ALARM1}, // S_ALARM1

	// Deep Sea Gargoyle
	{SPR_GARG, 0, -1, {NULL}, 0, 0, S_NULL},  // S_GARGOYLE

	// DSZ Seaweed
	{SPR_SEWE, 0, -1, {NULL}, 0, 0, S_SEAWEED2}, // S_SEAWEED1
	{SPR_SEWE, 1, 5, {NULL}, 0, 0, S_SEAWEED3}, // S_SEAWEED2
	{SPR_SEWE, 2, 5, {NULL}, 0, 0, S_SEAWEED4}, // S_SEAWEED3
	{SPR_SEWE, 3, 5, {NULL}, 0, 0, S_SEAWEED5}, // S_SEAWEED4
	{SPR_SEWE, 4, 5, {NULL}, 0, 0, S_SEAWEED6}, // S_SEAWEED5
	{SPR_SEWE, 5, 5, {NULL}, 0, 0, S_SEAWEED1}, // S_SEAWEED6

	// Dripping water
	{SPR_DISS, 0, 3*TICRATE, {NULL},                  0, 0, S_DRIPA2}, // S_DRIPA1
	{SPR_DRIP, 0,         2, {NULL},                  0, 0, S_DRIPA3}, // S_DRIPA2
	{SPR_DRIP, 1,         2, {NULL},                  0, 0, S_DRIPA4}, // S_DRIPA3
	{SPR_DRIP, 2,         2, {A_SpawnObjectRelative}, 0, MT_WATERDROP, S_DRIPA1}, // S_DRIPA4
	{SPR_DRIP, 3,        -1, {NULL},                  0, 0, S_DRIPB1}, // S_DRIPB1
	{SPR_DRIP, 4,         1, {NULL},                  0, 0, S_DRIPC2}, // S_DRIPC1
	{SPR_DRIP, 5,         1, {NULL},                  0, 0,   S_DISS}, // S_DRIPC2

	// Coral 1
	{SPR_CRL1, 0, -1, {NULL}, 0, 0, S_NULL}, // S_CORAL1

	// Coral 2
	{SPR_CRL2, 0, -1, {NULL}, 0, 0, S_NULL}, // S_CORAL2

	// Coral 3
	{SPR_CRL3, 0, -1, {NULL}, 0, 0, S_NULL}, // S_CORAL3

	// Blue Crystal
	{SPR_BCRY, 0, -1, {NULL}, 0, 0, S_NULL}, // S_BLUECRYSTAL1

	// CEZ Chain
	{SPR_CHAN, 0, -1, {NULL}, 0, 0, S_NULL}, // S_CEZCHAIN

	// Flame
	{SPR_FLAM, 32768, 3, {NULL}, 0, 0, S_FLAME2}, // S_FLAME1
	{SPR_FLAM, 32769, 3, {NULL}, 0, 0, S_FLAME3}, // S_FLAME2
	{SPR_FLAM, 32770, 3, {NULL}, 0, 0, S_FLAME4}, // S_FLAME3
	{SPR_FLAM, 32771, 3, {NULL}, 0, 0, S_FLAME1}, // S_FLAME4

	// Eggman statue
	{SPR_ESTA, 0, -1, {NULL}, 0, 0, S_DISS}, // S_EGGSTATUE1

	// Small Mace Chain
	{SPR_SMCH, 0, 1, {A_MaceRotate}, 0, 0, S_SMALLMACECHAIN}, // S_SMALLMACECHAIN

	// Big Mace Chain
	{SPR_BMCH, 0, 1, {A_MaceRotate}, 0, 0, S_BIGMACECHAIN}, // S_BIGMACECHAIN

	// Small Mace
	{SPR_SMCE, 0, 1, {A_MaceRotate}, 0, 0, S_SMALLMACE}, // S_SMALLMACE

	// Big Mace
	{SPR_BMCE, 0, 1, {A_MaceRotate}, 0, 0, S_BIGMACE}, // S_BIGMACE

	// CEZ Flower
	{SPR_FWR4, 0, -1, {NULL}, 0, 0, S_NULL}, // S_CEZFLOWER1

		// Big Tumbleweed
	{SPR_BTBL, 0, -1, {NULL}, 0, 0, S_NULL}, // S_BIGTUMBLEWEED
	{SPR_BTBL, 0, 5, {NULL}, 0, 0, S_BIGTUMBLEWEED_ROLL2}, // S_BIGTUMBLEWEED_ROLL1
	{SPR_BTBL, 1, 5, {NULL}, 0, 0, S_BIGTUMBLEWEED_ROLL3}, // S_BIGTUMBLEWEED_ROLL2
	{SPR_BTBL, 2, 5, {NULL}, 0, 0, S_BIGTUMBLEWEED_ROLL4}, // S_BIGTUMBLEWEED_ROLL3
	{SPR_BTBL, 3, 5, {NULL}, 0, 0, S_BIGTUMBLEWEED_ROLL5}, // S_BIGTUMBLEWEED_ROLL4
	{SPR_BTBL, 4, 5, {NULL}, 0, 0, S_BIGTUMBLEWEED_ROLL6}, // S_BIGTUMBLEWEED_ROLL5
	{SPR_BTBL, 5, 5, {NULL}, 0, 0, S_BIGTUMBLEWEED_ROLL7}, // S_BIGTUMBLEWEED_ROLL6
	{SPR_BTBL, 6, 5, {NULL}, 0, 0, S_BIGTUMBLEWEED_ROLL8}, // S_BIGTUMBLEWEED_ROLL7
	{SPR_BTBL, 7, 5, {NULL}, 0, 0, S_BIGTUMBLEWEED_ROLL1}, // S_BIGTUMBLEWEED_ROLL8

	// Little Tumbleweed
	{SPR_STBL, 0, -1, {NULL}, 0, 0, S_NULL}, // S_LITTLETUMBLEWEED
	{SPR_STBL, 0, 5, {NULL}, 0, 0, S_LITTLETUMBLEWEED_ROLL2}, // S_LITTLETUMBLEWEED_ROLL1
	{SPR_STBL, 1, 5, {NULL}, 0, 0, S_LITTLETUMBLEWEED_ROLL3}, // S_LITTLETUMBLEWEED_ROLL2
	{SPR_STBL, 2, 5, {NULL}, 0, 0, S_LITTLETUMBLEWEED_ROLL4}, // S_LITTLETUMBLEWEED_ROLL3
	{SPR_STBL, 3, 5, {NULL}, 0, 0, S_LITTLETUMBLEWEED_ROLL5}, // S_LITTLETUMBLEWEED_ROLL4
	{SPR_STBL, 4, 5, {NULL}, 0, 0, S_LITTLETUMBLEWEED_ROLL6}, // S_LITTLETUMBLEWEED_ROLL5
	{SPR_STBL, 5, 5, {NULL}, 0, 0, S_LITTLETUMBLEWEED_ROLL7}, // S_LITTLETUMBLEWEED_ROLL6
	{SPR_STBL, 6, 5, {NULL}, 0, 0, S_LITTLETUMBLEWEED_ROLL8}, // S_LITTLETUMBLEWEED_ROLL7
	{SPR_STBL, 7, 5, {NULL}, 0, 0, S_LITTLETUMBLEWEED_ROLL1}, // S_LITTLETUMBLEWEED_ROLL8

	// Cacti Sprites
	{SPR_CACT, 0, -1, {NULL}, 0, 0, S_NULL}, // S_CACTI1
	{SPR_CACT, 1, -1, {NULL}, 0, 0, S_NULL}, // S_CACTI2
	{SPR_CACT, 2, -1, {NULL}, 0, 0, S_NULL}, // S_CACTI3
	{SPR_CACT, 3, -1, {NULL}, 0, 0, S_NULL}, // S_CACTI4

	// Flame jet
	{SPR_DISS, 0, 2*TICRATE, {NULL},             0, 0, S_FLAMEJETSTART}, // S_FLAMEJETSTND
	{SPR_DISS, 0, 3*TICRATE, {A_ToggleFlameJet}, 0, 0,  S_FLAMEJETSTOP}, // S_FLAMEJETSTART
	{SPR_DISS, 0,         1, {A_ToggleFlameJet}, 0, 0,  S_FLAMEJETSTND}, // S_FLAMEJETSTOP
	{SPR_FLME, 0,         4, {NULL}, 0, 0, S_FLAMEJETFLAME2}, // S_FLAMEJETFLAME1
	{SPR_FLME, 1,         5, {NULL}, 0, 0, S_FLAMEJETFLAME3}, // S_FLAMEJETFLAME2
	{SPR_FLME, 2,        11, {NULL}, 0, 0,           S_DISS}, // S_FLAMEJETFLAME3

	// Xmas-specific stuff
	{SPR_XMS1, 0, -1, {NULL}, 0, 0, S_NULL}, // S_XMASPOLE
	{SPR_XMS2, 0, -1, {NULL}, 0, 0, S_NULL}, // S_CANDYCANE
	{SPR_XMS3, 0, -1, {NULL}, 0, 0, S_NULL}, // S_SNOWMAN

	{SPR_DBAL, 32768, 5, {NULL}, 0, 0, S_DBALL2}, // S_DBALL1
	{SPR_DBAL, 32769, 5, {NULL}, 0, 0, S_DBALL3}, // S_DBALL2
	{SPR_DBAL, 32770, 5, {NULL}, 0, 0, S_DBALL4}, // S_DBALL3
	{SPR_DBAL, 32771, 5, {NULL}, 0, 0, S_DBALL5}, // S_DBALL4
	{SPR_DBAL, 32772, 5, {NULL}, 0, 0, S_DBALL6}, // S_DBALL5
	{SPR_DBAL, 32773, 5, {NULL}, 0, 0, S_DBALL1}, // S_DBALL6

	{SPR_ESTA, 1, -1, {NULL}, 0, 0, S_DISS}, // S_EGGSTATUE2

	// Thok
	{SPR_THOK, 0, 8, {NULL}, 0, 0, S_NULL}, // S_THOK1

	// Shield Orb
	{SPR_SORB, 0, 1, {NULL}, 0, 0, S_SORB2}, // S_SORB1
	{SPR_SORB, 1, 1, {NULL}, 0, 0, S_SORB3}, // S_SORB2
	{SPR_SORB, 2, 1, {NULL}, 0, 0, S_SORB4}, // S_SORB3
	{SPR_SORB, 3, 1, {NULL}, 0, 0, S_SORB5}, // S_SORB4
	{SPR_SORB, 4, 1, {NULL}, 0, 0, S_SORB6}, // S_SORB5
	{SPR_SORB, 5, 1, {NULL}, 0, 0, S_SORB7}, // S_SORB6
	{SPR_SORB, 6, 1, {NULL}, 0, 0, S_SORB8}, // S_SORB7
	{SPR_SORB, 7, 1, {NULL}, 0, 0, S_SORB1}, // S_SORB8

	// Invincibility Sparkles
	{SPR_IVSP, 0, 1, {NULL}, 0, 0, S_IVSP2},   // S_IVSP1
	{SPR_IVSP, 1, 1, {NULL}, 0, 0, S_IVSP3},   // S_IVSP2
	{SPR_IVSP, 2, 1, {NULL}, 0, 0, S_IVSP4},   // S_IVSP3
	{SPR_IVSP, 3, 1, {NULL}, 0, 0, S_IVSP5},   // S_IVSP4
	{SPR_IVSP, 4, 1, {NULL}, 0, 0, S_IVSP6},   // S_IVSP5
	{SPR_IVSP, 5, 1, {NULL}, 0, 0, S_IVSP7},   // S_IVSP6
	{SPR_IVSP, 6, 1, {NULL}, 0, 0, S_IVSP8},   // S_IVSP7
	{SPR_IVSP, 7, 1, {NULL}, 0, 0, S_IVSP9},   // S_IVSP8
	{SPR_IVSP, 8, 1, {NULL}, 0, 0, S_IVSP10},  // S_IVSP9
	{SPR_IVSP, 9, 1, {NULL}, 0, 0, S_IVSP11},  // S_IVSP10
	{SPR_IVSP, 10, 1, {NULL}, 0, 0, S_IVSP12}, // S_IVSP11
	{SPR_IVSP, 11, 1, {NULL}, 0, 0, S_IVSP13}, // S_IVSP12
	{SPR_IVSP, 12, 1, {NULL}, 0, 0, S_IVSP14}, // S_IVSP13
	{SPR_IVSP, 13, 1, {NULL}, 0, 0, S_IVSP15}, // S_IVSP14
	{SPR_IVSP, 14, 1, {NULL}, 0, 0, S_IVSP16}, // S_IVSP15
	{SPR_IVSP, 15, 1, {NULL}, 0, 0, S_IVSP17}, // S_IVSP16
	{SPR_IVSP, 16, 1, {NULL}, 0, 0, S_IVSP18}, // S_IVSP17
	{SPR_IVSP, 17, 1, {NULL}, 0, 0, S_IVSP19}, // S_IVSP18
	{SPR_IVSP, 18, 1, {NULL}, 0, 0, S_IVSP20}, // S_IVSP19
	{SPR_IVSP, 19, 1, {NULL}, 0, 0, S_IVSP21}, // S_IVSP20
	{SPR_IVSP, 20, 1, {NULL}, 0, 0, S_IVSP22}, // S_IVSP21
	{SPR_IVSP, 21, 1, {NULL}, 0, 0, S_IVSP23}, // S_IVSP22
	{SPR_IVSP, 22, 1, {NULL}, 0, 0, S_IVSP24}, // S_IVSP23
	{SPR_IVSP, 23, 1, {NULL}, 0, 0, S_IVSP25}, // S_IVSP24
	{SPR_IVSP, 24, 1, {NULL}, 0, 0, S_IVSP26}, // S_IVSP25
	{SPR_IVSP, 25, 1, {NULL}, 0, 0, S_IVSP27}, // S_IVSP26
	{SPR_IVSP, 26, 1, {NULL}, 0, 0, S_IVSP28}, // S_IVSP27
	{SPR_IVSP, 27, 1, {NULL}, 0, 0, S_IVSP29}, // S_IVSP28
	{SPR_IVSP, 28, 1, {NULL}, 0, 0, S_IVSP30}, // S_IVSP29
	{SPR_IVSP, 29, 1, {NULL}, 0, 0, S_IVSP31}, // S_IVSP30
	{SPR_IVSP, 30, 1, {NULL}, 0, 0, S_IVSP32}, // S_IVSP31
	{SPR_IVSP, 31, 1, {NULL}, 0, 0, S_NULL},   // S_IVSP32

	// Super Sonic Spark
	{SPR_SSPK, 0, 2, {NULL}, 0, 0, S_SSPK2}, // S_SSPK1
	{SPR_SSPK, 1, 2, {NULL}, 0, 0, S_SSPK3}, // S_SSPK2
	{SPR_SSPK, 2, 2, {NULL}, 0, 0, S_SSPK4}, // S_SSPK3
	{SPR_SSPK, 1, 2, {NULL}, 0, 0, S_SSPK5}, // S_SSPK4
	{SPR_SSPK, 0, 2, {NULL}, 0, 0, S_DISS},  // S_SSPK5

	// Freed Birdie
	{SPR_BIRD, 0, 4, {NULL}, 0, 0, S_BIRD2},    // S_BIRD1
	{SPR_BIRD, 0, 4, {A_Chase}, 0, 0, S_BIRD3}, // S_BIRD2
	{SPR_BIRD, 1, 4, {A_Chase}, 0, 0, S_BIRD2}, // S_BIRD3

	// Freed Bunny
	{SPR_BUNY, 0, 4, {NULL}, 0, 0, S_BUNNY2},       // S_BUNNY1
	{SPR_BUNY, 0, 64, {NULL}, 0, 0, S_BUNNY3},      // S_BUNNY2
	{SPR_BUNY, 1, 2, {A_BunnyHop}, 6, 3, S_BUNNY4}, // S_BUNNY3
	{SPR_BUNY, 1, 2, {A_Chase}, 0, 0, S_BUNNY5},    // S_BUNNY4
	{SPR_BUNY, 1, 2, {A_Chase}, 0, 0, S_BUNNY6},    // S_BUNNY5
	{SPR_BUNY, 1, 2, {A_Chase}, 0, 0, S_BUNNY7},    // S_BUNNY6
	{SPR_BUNY, 1, 2, {A_Chase}, 0, 0, S_BUNNY8},    // S_BUNNY7
	{SPR_BUNY, 1, 2, {A_Chase}, 0, 0, S_BUNNY9},    // S_BUNNY8
	{SPR_BUNY, 1, 2, {A_Chase}, 0, 0, S_BUNNY10},   // S_BUNNY9
	{SPR_BUNY, 1, 2, {A_Chase}, 0, 0, S_BUNNY2},    // S_BUNNY10

	// Freed Mouse
	{SPR_MOUS, 0, 2, {A_MouseThink}, 0, 0, S_MOUSE2}, // S_MOUSE1
	{SPR_MOUS, 1, 2, {A_MouseThink}, 0, 0, S_MOUSE1}, // S_MOUSE2

	// Freed Chicken
	{SPR_CHIC, 0, 7, {A_Chase},        3, 0, S_CHICKENHOP},  // S_CHICKEN1
	{SPR_CHIC, 0, 1, {A_BunnyHop},     4, 2, S_CHICKENFLY1}, // S_CHICKENHOP
	{SPR_CHIC, 1, 2, {A_ChickenCheck}, 0, 0, S_CHICKENFLY2}, // S_CHICKENFLY1
	{SPR_CHIC, 2, 2, {NULL},           0, 0, S_CHICKENFLY1}, // S_CHICKENFLY2

	// Freed Cow
	{SPR_COWZ, 0, 4, {A_Chase}, 3, 0, S_COW2}, // S_COW1
	{SPR_COWZ, 1, 4, {A_Chase}, 3, 0, S_COW3}, // S_COW2
	{SPR_COWZ, 2, 4, {A_Chase}, 3, 0, S_COW4}, // S_COW3
	{SPR_COWZ, 3, 4, {A_Chase}, 3, 0, S_COW1}, // S_COW4

	// Yellow Spring
	{SPR_SPRY, 0, -1, {NULL}, 0, 0, S_NULL},           // S_YELLOWSPRING
	{SPR_SPRY, 4, 4, {A_Pain}, 0, 0, S_YELLOWSPRING3}, // S_YELLOWSPRING2
	{SPR_SPRY, 3, 1, {NULL}, 0, 0, S_YELLOWSPRING4},   // S_YELLOWSPRING3
	{SPR_SPRY, 2, 1, {NULL}, 0, 0, S_YELLOWSPRING5},   // S_YELLOWSPRING4
	{SPR_SPRY, 1, 1, {NULL}, 0, 0, S_YELLOWSPRING},    // S_YELLOWSPRING5

	// Red Spring
	{SPR_SPRR, 0, -1, {NULL}, 0, 0, S_NULL},        // S_REDSPRING
	{SPR_SPRR, 4, 4, {A_Pain}, 0, 0, S_REDSPRING3}, // S_REDSPRING2
	{SPR_SPRR, 3, 1, {NULL}, 0, 0, S_REDSPRING4},   // S_REDSPRING3
	{SPR_SPRR, 2, 1, {NULL}, 0, 0, S_REDSPRING5},   // S_REDSPRING4
	{SPR_SPRR, 1, 1, {NULL}, 0, 0, S_REDSPRING},    // S_REDSPRING5

	// Blue Spring
	{SPR_SPRB, 0, -1, {NULL}, 0, 0, S_NULL},         // S_BLUESPRING
	{SPR_SPRB, 4, 4, {A_Pain}, 0, 0, S_BLUESPRING3}, // S_BLUESPRING2
	{SPR_SPRB, 3, 1, {NULL}, 0, 0, S_BLUESPRING4},   // S_BLUESPRING3
	{SPR_SPRB, 2, 1, {NULL}, 0, 0, S_BLUESPRING5},   // S_BLUESPRING4
	{SPR_SPRB, 1, 1, {NULL}, 0, 0, S_BLUESPRING},    // S_BLUESPRING5


	{SPR_SUDY, 0, -1, {NULL}, 0, 0, S_NULL},             // S_YELLOWSPRINGUD
	{SPR_SUDY, 4, 4, {A_Pain}, 0, 0, S_YELLOWSPRINGUD3}, // S_YELLOWSPRINGUD2
	{SPR_SUDY, 3, 1, {NULL}, 0, 0, S_YELLOWSPRINGUD4},   // S_YELLOWSPRINGUD3
	{SPR_SUDY, 2, 1, {NULL}, 0, 0, S_YELLOWSPRINGUD5},   // S_YELLOWSPRINGUD4
	{SPR_SUDY, 1, 1, {NULL}, 0, 0, S_YELLOWSPRINGUD},    // S_YELLOWSPRINGUD5


	// Upside-Down Red Spring
	{SPR_SUDR, 0, -1, {NULL}, 0, 0, S_NULL},          // S_REDSPRINGUD
	{SPR_SUDR, 4, 4, {A_Pain}, 0, 0, S_REDSPRINGUD3}, // S_REDSPRINGUD2
	{SPR_SUDR, 3, 1, {NULL}, 0, 0, S_REDSPRINGUD4},   // S_REDSPRINGUD3
	{SPR_SUDR, 2, 1, {NULL}, 0, 0, S_REDSPRINGUD5},   // S_REDSPRINGUD4
	{SPR_SUDR, 1, 1, {NULL}, 0, 0, S_REDSPRINGUD},    // S_REDSPRINGUD5

	// Yellow Diagonal Spring
	{SPR_YSPR, 0, -1, {NULL}, 0, 0, S_NULL},    // S_YDIAG1
	{SPR_YSPR, 1, 1, {A_Pain}, 0, 0, S_YDIAG3}, // S_YDIAG2
	{SPR_YSPR, 2, 1, {NULL}, 0, 0, S_YDIAG4},   // S_YDIAG3
	{SPR_YSPR, 3, 1, {NULL}, 0, 0, S_YDIAG5},   // S_YDIAG4
	{SPR_YSPR, 4, 1, {NULL}, 0, 0, S_YDIAG6},   // S_YDIAG5
	{SPR_YSPR, 3, 1, {NULL}, 0, 0, S_YDIAG7},   // S_YDIAG6
	{SPR_YSPR, 2, 1, {NULL}, 0, 0, S_YDIAG8},   // S_YDIAG7
	{SPR_YSPR, 1, 1, {NULL}, 0, 0, S_YDIAG1},   // S_YDIAG8

	// Red Diagonal Spring
	{SPR_RSPR, 0, -1, {NULL}, 0, 0, S_NULL},    // S_RDIAG1
	{SPR_RSPR, 1, 1, {A_Pain}, 0, 0, S_RDIAG3}, // S_RDIAG2
	{SPR_RSPR, 2, 1, {NULL}, 0, 0, S_RDIAG4},   // S_RDIAG3
	{SPR_RSPR, 3, 1, {NULL}, 0, 0, S_RDIAG5},   // S_RDIAG4
	{SPR_RSPR, 4, 1, {NULL}, 0, 0, S_RDIAG6},   // S_RDIAG5
	{SPR_RSPR, 3, 1, {NULL}, 0, 0, S_RDIAG7},   // S_RDIAG6
	{SPR_RSPR, 2, 1, {NULL}, 0, 0, S_RDIAG8},   // S_RDIAG7
	{SPR_RSPR, 1, 1, {NULL}, 0, 0, S_RDIAG1},   // S_RDIAG8

	// Yellow Upside-Down Diagonal Spring
	{SPR_YSUD, 0, -1, {NULL}, 0, 0, S_NULL},     // S_YDIAGD1
	{SPR_YSUD, 1, 1, {A_Pain}, 0, 0, S_YDIAGD3}, // S_YDIAGD2
	{SPR_YSUD, 2, 1, {NULL}, 0, 0, S_YDIAGD4},   // S_YDIAGD3
	{SPR_YSUD, 3, 1, {NULL}, 0, 0, S_YDIAGD5},   // S_YDIAGD4
	{SPR_YSUD, 4, 1, {NULL}, 0, 0, S_YDIAGD6},   // S_YDIAGD5
	{SPR_YSUD, 3, 1, {NULL}, 0, 0, S_YDIAGD7},   // S_YDIAGD6
	{SPR_YSUD, 2, 1, {NULL}, 0, 0, S_YDIAGD8},   // S_YDIAGD7
	{SPR_YSUD, 1, 1, {NULL}, 0, 0, S_YDIAGD1},   // S_YDIAGD8

	// Red Upside-Down Diagonal Spring
	{SPR_RSUD, 0, -1, {NULL}, 0, 0, S_NULL},     // S_RDIAGD1
	{SPR_RSUD, 1, 1, {A_Pain}, 0, 0, S_RDIAGD3}, // S_RDIAGD2
	{SPR_RSUD, 2, 1, {NULL}, 0, 0, S_RDIAGD4},   // S_RDIAGD3
	{SPR_RSUD, 3, 1, {NULL}, 0, 0, S_RDIAGD5},   // S_RDIAGD4
	{SPR_RSUD, 4, 1, {NULL}, 0, 0, S_RDIAGD6},   // S_RDIAGD5
	{SPR_RSUD, 3, 1, {NULL}, 0, 0, S_RDIAGD7},   // S_RDIAGD6
	{SPR_RSUD, 2, 1, {NULL}, 0, 0, S_RDIAGD8},   // S_RDIAGD7
	{SPR_RSUD, 1, 1, {NULL}, 0, 0, S_RDIAGD1},   // S_RDIAGD8

	// Rain
	{SPR_RAIN, 32768, -1, {NULL}, 0, 0, S_NULL}, // S_RAIN1
	{SPR_RAIN, 32768, 1, {NULL}, 0, 0, S_RAIN1}, // S_RAINRETURN

	// Snowflake
	{SPR_SNO1, 0, -1, {NULL}, 0, 0, S_NULL}, // S_SNOW1
	{SPR_SNO1, 1, -1, {NULL}, 0, 0, S_NULL}, // S_SNOW2
	{SPR_SNO1, 2, -1, {NULL}, 0, 0, S_NULL}, // S_SNOW3

	// Water Splish
	{SPR_SPLH, 0, 2, {NULL}, 0, 0, S_SPLISH2}, // S_SPLISH1
	{SPR_SPLH, 1, 2, {NULL}, 0, 0, S_SPLISH3}, // S_SPLISH2
	{SPR_SPLH, 2, 2, {NULL}, 0, 0, S_SPLISH4}, // S_SPLISH3
	{SPR_SPLH, 3, 2, {NULL}, 0, 0, S_SPLISH5}, // S_SPLISH4
	{SPR_SPLH, 4, 2, {NULL}, 0, 0, S_SPLISH6}, // S_SPLISH5
	{SPR_SPLH, 5, 2, {NULL}, 0, 0, S_SPLISH7}, // S_SPLISH6
	{SPR_SPLH, 6, 2, {NULL}, 0, 0, S_SPLISH8}, // S_SPLISH7
	{SPR_SPLH, 7, 2, {NULL}, 0, 0, S_SPLISH9}, // S_SPLISH8
	{SPR_SPLH, 8, 2, {NULL}, 0, 0, S_DISS},    // S_SPLISH9

	// Water Splash
	{SPR_SPLA, 0, 3, {NULL}, 0, 0, S_SPLASH2},    // S_SPLASH1
	{SPR_SPLA, 1, 3, {NULL}, 0, 0, S_SPLASH3},    // S_SPLASH2
	{SPR_SPLA, 2, 3, {NULL}, 0, 0, S_RAINRETURN}, // S_SPLASH3

	// Smoke
	{SPR_SMOK, 0, 4, {NULL}, 0, 0, S_SMOK2}, // S_SMOK1
	{SPR_SMOK, 1, 5, {NULL}, 0, 0, S_SMOK3}, // S_SMOK2
	{SPR_SMOK, 2, 6, {NULL}, 0, 0, S_SMOK4}, // S_SMOK3
	{SPR_SMOK, 3, 7, {NULL}, 0, 0, S_SMOK5}, // S_SMOK4
	{SPR_SMOK, 4, 8, {NULL}, 0, 0, S_NULL},  // S_SMOK5

	// Bubbles
	{SPR_BUBP, 0, 1, {A_BubbleRise}, 0, 1024, S_SMALLBUBBLE1},  // S_SMALLBUBBLE
	{SPR_BUBP, 0, 1, {A_BubbleRise}, 0, 1024, S_SMALLBUBBLE},   // S_SMALLBUBBLE1
	{SPR_BUBO, 0, 1, {A_BubbleRise}, 0, 1024, S_MEDIUMBUBBLE1}, // S_MEDIUMBUBBLE
	{SPR_BUBO, 0, 1, {A_BubbleRise}, 0, 1024, S_MEDIUMBUBBLE},  // S_MEDIUMBUBBLE1

	// Extra Large Bubble (breathable)
	{SPR_BUBN, 32768, 16, {A_BubbleRise}, 0, 1024, S_EXTRALARGEBUBBLE}, // S_LARGEBUBBLE
	{SPR_BUBM, 32768, 16, {A_BubbleRise}, 0, 1024, S_EXTRALARGEBUBBLE}, // S_EXTRALARGEBUBBLE

	// Extra Large Bubble goes POP!
	{SPR_POPP, 0, 16, {NULL}, 0, 0, S_DISS}, // S_POP1

	{SPR_TFOG, 32768, 2, {NULL}, 0, 0, S_FOG2},  // S_FOG1
	{SPR_TFOG, 32769, 2, {NULL}, 0, 0, S_FOG3},  // S_FOG2
	{SPR_TFOG, 32770, 2, {NULL}, 0, 0, S_FOG4},  // S_FOG3
	{SPR_TFOG, 32771, 2, {NULL}, 0, 0, S_FOG5},  // S_FOG4
	{SPR_TFOG, 32772, 2, {NULL}, 0, 0, S_FOG6},  // S_FOG5
	{SPR_TFOG, 32773, 2, {NULL}, 0, 0, S_FOG7},  // S_FOG6
	{SPR_TFOG, 32774, 2, {NULL}, 0, 0, S_FOG8},  // S_FOG7
	{SPR_TFOG, 32775, 2, {NULL}, 0, 0, S_FOG9},  // S_FOG8
	{SPR_TFOG, 32776, 2, {NULL}, 0, 0, S_FOG10}, // S_FOG9
	{SPR_TFOG, 32777, 2, {NULL}, 0, 0, S_FOG11}, // S_FOG10
	{SPR_TFOG, 32778, 2, {NULL}, 0, 0, S_FOG12}, // S_FOG11
	{SPR_TFOG, 32779, 2, {NULL}, 0, 0, S_FOG13}, // S_FOG12
	{SPR_TFOG, 32780, 2, {NULL}, 0, 0, S_FOG14}, // S_FOG13
	{SPR_TFOG, 32781, 2, {NULL}, 0, 0, S_DISS},  // S_FOG14

	// Flower Seed
	{SPR_SEED, 32768, -1, {NULL}, 0, 0, S_NULL}, // S_SEED

	// Particle sprite
	{SPR_PRTL, 32768, 2*TICRATE, {NULL}, 0, 0, S_DISS}, // S_PARTICLE
	{SPR_DISS,     0,         1, {A_ParticleSpawn}, 0, 0, S_PARTICLEGEN}, // S_PARTICLEGEN

	{SPR_SCOR, 0, 32, {A_ScoreRise}, 0, 0, S_DISS}, // S_SCRA
	{SPR_SCOR, 1, 32, {A_ScoreRise}, 0, 0, S_DISS}, // S_SCRB
	{SPR_SCOR, 2, 32, {A_ScoreRise}, 0, 0, S_DISS}, // S_SCRC
	{SPR_SCOR, 3, 32, {A_ScoreRise}, 0, 0, S_DISS}, // S_SCRD

	// Drowning Timer Numbers
	{SPR_DRWN, 0, 40, {NULL}, 0, 0, S_DISS}, // S_ZERO1
	{SPR_DRWN, 1, 40, {NULL}, 0, 0, S_DISS}, // S_ONE1
	{SPR_DRWN, 2, 40, {NULL}, 0, 0, S_DISS}, // S_TWO1
	{SPR_DRWN, 3, 40, {NULL}, 0, 0, S_DISS}, // S_THREE1
	{SPR_DRWN, 4, 40, {NULL}, 0, 0, S_DISS}, // S_FOUR1
	{SPR_DRWN, 5, 40, {NULL}, 0, 0, S_DISS}, // S_FIVE1

	{SPR_TTAG, 32768, 2, {NULL}, 0, 0, S_DISS}, // S_TTAG1

	// CTF Sign
	{SPR_GFLG, 0, 1, {NULL}, 0, 0, S_GOTFLAG2}, // S_GOTFLAG1
	{SPR_GFLG, 1, 1, {NULL}, 0, 0, S_DISS},     // S_GOTFLAG2
	{SPR_GFLG, 0, 1, {NULL}, 0, 0, S_GOTFLAG4}, // S_GOTFLAG3
	{SPR_GFLG, 2, 1, {NULL}, 0, 0, S_DISS},     // S_GOTFLAG4

	// Red Rings (thrown)
	{SPR_RRNG, 32768, 1, {A_ThrownRing}, 0, 0, S_RRNG2}, // S_RRNG1
	{SPR_RRNG, 32769, 1, {A_ThrownRing}, 0, 0, S_RRNG3}, // S_RRNG2
	{SPR_RRNG, 32770, 1, {A_ThrownRing}, 0, 0, S_RRNG4}, // S_RRNG3
	{SPR_RRNG, 32771, 1, {A_ThrownRing}, 0, 0, S_RRNG5}, // S_RRNG4
	{SPR_RRNG, 32772, 1, {A_ThrownRing}, 0, 0, S_RRNG6}, // S_RRNG5
	{SPR_RRNG, 32773, 1, {A_ThrownRing}, 0, 0, S_RRNG7}, // S_RRNG6
	{SPR_RRNG, 32774, 1, {A_ThrownRing}, 0, 0, S_RRNG1}, // S_RRNG7

	// Bounce Ring
	{SPR_RNGB, 0, 1, {NULL}, 0, 0, S_BOUNCERING2},   // S_BOUNCERING1
	{SPR_RNGB, 1, 1, {NULL}, 0, 0, S_BOUNCERING3},   // S_BOUNCERING2
	{SPR_RNGB, 2, 1, {NULL}, 0, 0, S_BOUNCERING4},   // S_BOUNCERING3
	{SPR_RNGB, 3, 1, {NULL}, 0, 0, S_BOUNCERING5},   // S_BOUNCERING4
	{SPR_RNGB, 4, 1, {NULL}, 0, 0, S_BOUNCERING6},   // S_BOUNCERING5
	{SPR_RNGB, 5, 1, {NULL}, 0, 0, S_BOUNCERING7},   // S_BOUNCERING6
	{SPR_RNGB, 6, 1, {NULL}, 0, 0, S_BOUNCERING8},   // S_BOUNCERING7
	{SPR_RNGB, 7, 1, {NULL}, 0, 0, S_BOUNCERING9},   // S_BOUNCERING8
	{SPR_RNGB, 8, 1, {NULL}, 0, 0, S_BOUNCERING10},  // S_BOUNCERING9
	{SPR_RNGB, 9, 1, {NULL}, 0, 0, S_BOUNCERING11},  // S_BOUNCERING10
	{SPR_RNGB, 10, 1, {NULL}, 0, 0, S_BOUNCERING12}, // S_BOUNCERING11
	{SPR_RNGB, 11, 1, {NULL}, 0, 0, S_BOUNCERING13}, // S_BOUNCERING12
	{SPR_RNGB, 12, 1, {NULL}, 0, 0, S_BOUNCERING14}, // S_BOUNCERING13
	{SPR_RNGB, 13, 1, {NULL}, 0, 0, S_BOUNCERING15}, // S_BOUNCERING14
	{SPR_RNGB, 14, 1, {NULL}, 0, 0, S_BOUNCERING16}, // S_BOUNCERING15
	{SPR_RNGB, 15, 1, {NULL}, 0, 0, S_BOUNCERING17}, // S_BOUNCERING16
	{SPR_RNGB, 16, 1, {NULL}, 0, 0, S_BOUNCERING18}, // S_BOUNCERING17
	{SPR_RNGB, 17, 1, {NULL}, 0, 0, S_BOUNCERING19}, // S_BOUNCERING18
	{SPR_RNGB, 18, 1, {NULL}, 0, 0, S_BOUNCERING20}, // S_BOUNCERING19
	{SPR_RNGB, 19, 1, {NULL}, 0, 0, S_BOUNCERING21}, // S_BOUNCERING20
	{SPR_RNGB, 20, 1, {NULL}, 0, 0, S_BOUNCERING22}, // S_BOUNCERING21
	{SPR_RNGB, 21, 1, {NULL}, 0, 0, S_BOUNCERING23}, // S_BOUNCERING22
	{SPR_RNGB, 22, 1, {NULL}, 0, 0, S_BOUNCERING24}, // S_BOUNCERING23
	{SPR_RNGB, 23, 1, {NULL}, 0, 0, S_BOUNCERING25}, // S_BOUNCERING24
	{SPR_RNGB, 24, 1, {NULL}, 0, 0, S_BOUNCERING26}, // S_BOUNCERING25
	{SPR_RNGB, 25, 1, {NULL}, 0, 0, S_BOUNCERING27}, // S_BOUNCERING26
	{SPR_RNGB, 26, 1, {NULL}, 0, 0, S_BOUNCERING28}, // S_BOUNCERING27
	{SPR_RNGB, 27, 1, {NULL}, 0, 0, S_BOUNCERING29}, // S_BOUNCERING28
	{SPR_RNGB, 28, 1, {NULL}, 0, 0, S_BOUNCERING30}, // S_BOUNCERING29
	{SPR_RNGB, 29, 1, {NULL}, 0, 0, S_BOUNCERING31}, // S_BOUNCERING30
	{SPR_RNGB, 30, 1, {NULL}, 0, 0, S_BOUNCERING32}, // S_BOUNCERING31
	{SPR_RNGB, 31, 1, {NULL}, 0, 0, S_BOUNCERING33}, // S_BOUNCERING32
	{SPR_RNGB, 32, 1, {NULL}, 0, 0, S_BOUNCERING34}, // S_BOUNCERING33
	{SPR_RNGB, 33, 1, {NULL}, 0, 0, S_BOUNCERING35}, // S_BOUNCERING34
	{SPR_RNGB, 34, 1, {NULL}, 0, 0, S_BOUNCERING1},  // S_BOUNCERING35

	// Rail Ring
	{SPR_RNGR, 0, 1, {NULL}, 0, 0, S_RAILRING2},   // S_RAILRING1
	{SPR_RNGR, 1, 1, {NULL}, 0, 0, S_RAILRING3},   // S_RAILRING2
	{SPR_RNGR, 2, 1, {NULL}, 0, 0, S_RAILRING4},   // S_RAILRING3
	{SPR_RNGR, 3, 1, {NULL}, 0, 0, S_RAILRING5},   // S_RAILRING4
	{SPR_RNGR, 4, 1, {NULL}, 0, 0, S_RAILRING6},   // S_RAILRING5
	{SPR_RNGR, 5, 1, {NULL}, 0, 0, S_RAILRING7},   // S_RAILRING6
	{SPR_RNGR, 6, 1, {NULL}, 0, 0, S_RAILRING8},   // S_RAILRING7
	{SPR_RNGR, 7, 1, {NULL}, 0, 0, S_RAILRING9},   // S_RAILRING8
	{SPR_RNGR, 8, 1, {NULL}, 0, 0, S_RAILRING10},  // S_RAILRING9
	{SPR_RNGR, 9, 1, {NULL}, 0, 0, S_RAILRING11},  // S_RAILRING10
	{SPR_RNGR, 10, 1, {NULL}, 0, 0, S_RAILRING12}, // S_RAILRING11
	{SPR_RNGR, 11, 1, {NULL}, 0, 0, S_RAILRING13}, // S_RAILRING12
	{SPR_RNGR, 12, 1, {NULL}, 0, 0, S_RAILRING14}, // S_RAILRING13
	{SPR_RNGR, 13, 1, {NULL}, 0, 0, S_RAILRING15}, // S_RAILRING14
	{SPR_RNGR, 14, 1, {NULL}, 0, 0, S_RAILRING16}, // S_RAILRING15
	{SPR_RNGR, 15, 1, {NULL}, 0, 0, S_RAILRING17}, // S_RAILRING16
	{SPR_RNGR, 16, 1, {NULL}, 0, 0, S_RAILRING18}, // S_RAILRING17
	{SPR_RNGR, 17, 1, {NULL}, 0, 0, S_RAILRING19}, // S_RAILRING18
	{SPR_RNGR, 18, 1, {NULL}, 0, 0, S_RAILRING20}, // S_RAILRING19
	{SPR_RNGR, 19, 1, {NULL}, 0, 0, S_RAILRING21}, // S_RAILRING20
	{SPR_RNGR, 20, 1, {NULL}, 0, 0, S_RAILRING22}, // S_RAILRING21
	{SPR_RNGR, 21, 1, {NULL}, 0, 0, S_RAILRING23}, // S_RAILRING22
	{SPR_RNGR, 22, 1, {NULL}, 0, 0, S_RAILRING24}, // S_RAILRING23
	{SPR_RNGR, 23, 1, {NULL}, 0, 0, S_RAILRING25}, // S_RAILRING24
	{SPR_RNGR, 24, 1, {NULL}, 0, 0, S_RAILRING26}, // S_RAILRING25
	{SPR_RNGR, 25, 1, {NULL}, 0, 0, S_RAILRING27}, // S_RAILRING26
	{SPR_RNGR, 26, 1, {NULL}, 0, 0, S_RAILRING28}, // S_RAILRING27
	{SPR_RNGR, 27, 1, {NULL}, 0, 0, S_RAILRING29}, // S_RAILRING28
	{SPR_RNGR, 28, 1, {NULL}, 0, 0, S_RAILRING30}, // S_RAILRING29
	{SPR_RNGR, 29, 1, {NULL}, 0, 0, S_RAILRING31}, // S_RAILRING30
	{SPR_RNGR, 30, 1, {NULL}, 0, 0, S_RAILRING32}, // S_RAILRING31
	{SPR_RNGR, 31, 1, {NULL}, 0, 0, S_RAILRING33}, // S_RAILRING32
	{SPR_RNGR, 32, 1, {NULL}, 0, 0, S_RAILRING34}, // S_RAILRING33
	{SPR_RNGR, 33, 1, {NULL}, 0, 0, S_RAILRING35}, // S_RAILRING34
	{SPR_RNGR, 34, 1, {NULL}, 0, 0, S_RAILRING1},  // S_RAILRING35

	// Automatic Ring
	{SPR_RNGA, 0, 1, {NULL}, 0, 0, S_AUTOMATICRING2},   // S_AUTOMATICRING1
	{SPR_RNGA, 1, 1, {NULL}, 0, 0, S_AUTOMATICRING3},   // S_AUTOMATICRING2
	{SPR_RNGA, 2, 1, {NULL}, 0, 0, S_AUTOMATICRING4},   // S_AUTOMATICRING3
	{SPR_RNGA, 3, 1, {NULL}, 0, 0, S_AUTOMATICRING5},   // S_AUTOMATICRING4
	{SPR_RNGA, 4, 1, {NULL}, 0, 0, S_AUTOMATICRING6},   // S_AUTOMATICRING5
	{SPR_RNGA, 5, 1, {NULL}, 0, 0, S_AUTOMATICRING7},   // S_AUTOMATICRING6
	{SPR_RNGA, 6, 1, {NULL}, 0, 0, S_AUTOMATICRING8},   // S_AUTOMATICRING7
	{SPR_RNGA, 7, 1, {NULL}, 0, 0, S_AUTOMATICRING9},   // S_AUTOMATICRING8
	{SPR_RNGA, 8, 1, {NULL}, 0, 0, S_AUTOMATICRING10},  // S_AUTOMATICRING9
	{SPR_RNGA, 9, 1, {NULL}, 0, 0, S_AUTOMATICRING11},  // S_AUTOMATICRING10
	{SPR_RNGA, 10, 1, {NULL}, 0, 0, S_AUTOMATICRING12}, // S_AUTOMATICRING11
	{SPR_RNGA, 11, 1, {NULL}, 0, 0, S_AUTOMATICRING13}, // S_AUTOMATICRING12
	{SPR_RNGA, 12, 1, {NULL}, 0, 0, S_AUTOMATICRING14}, // S_AUTOMATICRING13
	{SPR_RNGA, 13, 1, {NULL}, 0, 0, S_AUTOMATICRING15}, // S_AUTOMATICRING14
	{SPR_RNGA, 14, 1, {NULL}, 0, 0, S_AUTOMATICRING16}, // S_AUTOMATICRING15
	{SPR_RNGA, 15, 1, {NULL}, 0, 0, S_AUTOMATICRING17}, // S_AUTOMATICRING16
	{SPR_RNGA, 16, 1, {NULL}, 0, 0, S_AUTOMATICRING18}, // S_AUTOMATICRING17
	{SPR_RNGA, 17, 1, {NULL}, 0, 0, S_AUTOMATICRING19}, // S_AUTOMATICRING18
	{SPR_RNGA, 18, 1, {NULL}, 0, 0, S_AUTOMATICRING20}, // S_AUTOMATICRING19
	{SPR_RNGA, 19, 1, {NULL}, 0, 0, S_AUTOMATICRING21}, // S_AUTOMATICRING20
	{SPR_RNGA, 20, 1, {NULL}, 0, 0, S_AUTOMATICRING22}, // S_AUTOMATICRING21
	{SPR_RNGA, 21, 1, {NULL}, 0, 0, S_AUTOMATICRING23}, // S_AUTOMATICRING22
	{SPR_RNGA, 22, 1, {NULL}, 0, 0, S_AUTOMATICRING24}, // S_AUTOMATICRING23
	{SPR_RNGA, 23, 1, {NULL}, 0, 0, S_AUTOMATICRING25}, // S_AUTOMATICRING24
	{SPR_RNGA, 24, 1, {NULL}, 0, 0, S_AUTOMATICRING26}, // S_AUTOMATICRING25
	{SPR_RNGA, 25, 1, {NULL}, 0, 0, S_AUTOMATICRING27}, // S_AUTOMATICRING26
	{SPR_RNGA, 26, 1, {NULL}, 0, 0, S_AUTOMATICRING28}, // S_AUTOMATICRING27
	{SPR_RNGA, 27, 1, {NULL}, 0, 0, S_AUTOMATICRING29}, // S_AUTOMATICRING28
	{SPR_RNGA, 28, 1, {NULL}, 0, 0, S_AUTOMATICRING30}, // S_AUTOMATICRING29
	{SPR_RNGA, 29, 1, {NULL}, 0, 0, S_AUTOMATICRING31}, // S_AUTOMATICRING30
	{SPR_RNGA, 30, 1, {NULL}, 0, 0, S_AUTOMATICRING32}, // S_AUTOMATICRING31
	{SPR_RNGA, 31, 1, {NULL}, 0, 0, S_AUTOMATICRING33}, // S_AUTOMATICRING32
	{SPR_RNGA, 32, 1, {NULL}, 0, 0, S_AUTOMATICRING34}, // S_AUTOMATICRING33
	{SPR_RNGA, 33, 1, {NULL}, 0, 0, S_AUTOMATICRING35}, // S_AUTOMATICRING34
	{SPR_RNGA, 34, 1, {NULL}, 0, 0, S_AUTOMATICRING1},  // S_AUTOMATICRING35

	// Explosion Ring
	{SPR_RNGE, 0, 1, {NULL}, 0, 0, S_EXPLOSIONRING2},   // S_EXPLOSIONRING1
	{SPR_RNGE, 1, 1, {NULL}, 0, 0, S_EXPLOSIONRING3},   // S_EXPLOSIONRING2
	{SPR_RNGE, 2, 1, {NULL}, 0, 0, S_EXPLOSIONRING4},   // S_EXPLOSIONRING3
	{SPR_RNGE, 3, 1, {NULL}, 0, 0, S_EXPLOSIONRING5},   // S_EXPLOSIONRING4
	{SPR_RNGE, 4, 1, {NULL}, 0, 0, S_EXPLOSIONRING6},   // S_EXPLOSIONRING5
	{SPR_RNGE, 5, 1, {NULL}, 0, 0, S_EXPLOSIONRING7},   // S_EXPLOSIONRING6
	{SPR_RNGE, 6, 1, {NULL}, 0, 0, S_EXPLOSIONRING8},   // S_EXPLOSIONRING7
	{SPR_RNGE, 7, 1, {NULL}, 0, 0, S_EXPLOSIONRING9},   // S_EXPLOSIONRING8
	{SPR_RNGE, 8, 1, {NULL}, 0, 0, S_EXPLOSIONRING10},  // S_EXPLOSIONRING9
	{SPR_RNGE, 9, 1, {NULL}, 0, 0, S_EXPLOSIONRING11},  // S_EXPLOSIONRING10
	{SPR_RNGE, 10, 1, {NULL}, 0, 0, S_EXPLOSIONRING12}, // S_EXPLOSIONRING11
	{SPR_RNGE, 11, 1, {NULL}, 0, 0, S_EXPLOSIONRING13}, // S_EXPLOSIONRING12
	{SPR_RNGE, 12, 1, {NULL}, 0, 0, S_EXPLOSIONRING14}, // S_EXPLOSIONRING13
	{SPR_RNGE, 13, 1, {NULL}, 0, 0, S_EXPLOSIONRING15}, // S_EXPLOSIONRING14
	{SPR_RNGE, 14, 1, {NULL}, 0, 0, S_EXPLOSIONRING16}, // S_EXPLOSIONRING15
	{SPR_RNGE, 15, 1, {NULL}, 0, 0, S_EXPLOSIONRING17}, // S_EXPLOSIONRING16
	{SPR_RNGE, 16, 1, {NULL}, 0, 0, S_EXPLOSIONRING18}, // S_EXPLOSIONRING17
	{SPR_RNGE, 17, 1, {NULL}, 0, 0, S_EXPLOSIONRING19}, // S_EXPLOSIONRING18
	{SPR_RNGE, 18, 1, {NULL}, 0, 0, S_EXPLOSIONRING20}, // S_EXPLOSIONRING19
	{SPR_RNGE, 19, 1, {NULL}, 0, 0, S_EXPLOSIONRING21}, // S_EXPLOSIONRING20
	{SPR_RNGE, 20, 1, {NULL}, 0, 0, S_EXPLOSIONRING22}, // S_EXPLOSIONRING21
	{SPR_RNGE, 21, 1, {NULL}, 0, 0, S_EXPLOSIONRING23}, // S_EXPLOSIONRING22
	{SPR_RNGE, 22, 1, {NULL}, 0, 0, S_EXPLOSIONRING24}, // S_EXPLOSIONRING23
	{SPR_RNGE, 23, 1, {NULL}, 0, 0, S_EXPLOSIONRING25}, // S_EXPLOSIONRING24
	{SPR_RNGE, 24, 1, {NULL}, 0, 0, S_EXPLOSIONRING26}, // S_EXPLOSIONRING25
	{SPR_RNGE, 25, 1, {NULL}, 0, 0, S_EXPLOSIONRING27}, // S_EXPLOSIONRING26
	{SPR_RNGE, 26, 1, {NULL}, 0, 0, S_EXPLOSIONRING28}, // S_EXPLOSIONRING27
	{SPR_RNGE, 27, 1, {NULL}, 0, 0, S_EXPLOSIONRING29}, // S_EXPLOSIONRING28
	{SPR_RNGE, 28, 1, {NULL}, 0, 0, S_EXPLOSIONRING30}, // S_EXPLOSIONRING29
	{SPR_RNGE, 29, 1, {NULL}, 0, 0, S_EXPLOSIONRING31}, // S_EXPLOSIONRING30
	{SPR_RNGE, 30, 1, {NULL}, 0, 0, S_EXPLOSIONRING32}, // S_EXPLOSIONRING31
	{SPR_RNGE, 31, 1, {NULL}, 0, 0, S_EXPLOSIONRING33}, // S_EXPLOSIONRING32
	{SPR_RNGE, 32, 1, {NULL}, 0, 0, S_EXPLOSIONRING34}, // S_EXPLOSIONRING33
	{SPR_RNGE, 33, 1, {NULL}, 0, 0, S_EXPLOSIONRING35}, // S_EXPLOSIONRING34
	{SPR_RNGE, 34, 1, {NULL}, 0, 0, S_EXPLOSIONRING1},  // S_EXPLOSIONRING35

	// Scatter Ring
	{SPR_RNGS, 0, 1, {NULL}, 0, 0, S_SCATTERRING2},   // S_SCATTERRING1
	{SPR_RNGS, 1, 1, {NULL}, 0, 0, S_SCATTERRING3},   // S_SCATTERRING2
	{SPR_RNGS, 2, 1, {NULL}, 0, 0, S_SCATTERRING4},   // S_SCATTERRING3
	{SPR_RNGS, 3, 1, {NULL}, 0, 0, S_SCATTERRING5},   // S_SCATTERRING4
	{SPR_RNGS, 4, 1, {NULL}, 0, 0, S_SCATTERRING6},   // S_SCATTERRING5
	{SPR_RNGS, 5, 1, {NULL}, 0, 0, S_SCATTERRING7},   // S_SCATTERRING6
	{SPR_RNGS, 6, 1, {NULL}, 0, 0, S_SCATTERRING8},   // S_SCATTERRING7
	{SPR_RNGS, 7, 1, {NULL}, 0, 0, S_SCATTERRING9},   // S_SCATTERRING8
	{SPR_RNGS, 8, 1, {NULL}, 0, 0, S_SCATTERRING10},  // S_SCATTERRING9
	{SPR_RNGS, 9, 1, {NULL}, 0, 0, S_SCATTERRING11},  // S_SCATTERRING10
	{SPR_RNGS, 10, 1, {NULL}, 0, 0, S_SCATTERRING12}, // S_SCATTERRING11
	{SPR_RNGS, 11, 1, {NULL}, 0, 0, S_SCATTERRING13}, // S_SCATTERRING12
	{SPR_RNGS, 12, 1, {NULL}, 0, 0, S_SCATTERRING14}, // S_SCATTERRING13
	{SPR_RNGS, 13, 1, {NULL}, 0, 0, S_SCATTERRING15}, // S_SCATTERRING14
	{SPR_RNGS, 14, 1, {NULL}, 0, 0, S_SCATTERRING16}, // S_SCATTERRING15
	{SPR_RNGS, 15, 1, {NULL}, 0, 0, S_SCATTERRING17}, // S_SCATTERRING16
	{SPR_RNGS, 16, 1, {NULL}, 0, 0, S_SCATTERRING18}, // S_SCATTERRING17
	{SPR_RNGS, 17, 1, {NULL}, 0, 0, S_SCATTERRING19}, // S_SCATTERRING18
	{SPR_RNGS, 18, 1, {NULL}, 0, 0, S_SCATTERRING20}, // S_SCATTERRING19
	{SPR_RNGS, 19, 1, {NULL}, 0, 0, S_SCATTERRING21}, // S_SCATTERRING20
	{SPR_RNGS, 20, 1, {NULL}, 0, 0, S_SCATTERRING22}, // S_SCATTERRING21
	{SPR_RNGS, 21, 1, {NULL}, 0, 0, S_SCATTERRING23}, // S_SCATTERRING22
	{SPR_RNGS, 22, 1, {NULL}, 0, 0, S_SCATTERRING24}, // S_SCATTERRING23
	{SPR_RNGS, 23, 1, {NULL}, 0, 0, S_SCATTERRING25}, // S_SCATTERRING24
	{SPR_RNGS, 24, 1, {NULL}, 0, 0, S_SCATTERRING26}, // S_SCATTERRING25
	{SPR_RNGS, 25, 1, {NULL}, 0, 0, S_SCATTERRING27}, // S_SCATTERRING26
	{SPR_RNGS, 26, 1, {NULL}, 0, 0, S_SCATTERRING28}, // S_SCATTERRING27
	{SPR_RNGS, 27, 1, {NULL}, 0, 0, S_SCATTERRING29}, // S_SCATTERRING28
	{SPR_RNGS, 28, 1, {NULL}, 0, 0, S_SCATTERRING30}, // S_SCATTERRING29
	{SPR_RNGS, 29, 1, {NULL}, 0, 0, S_SCATTERRING31}, // S_SCATTERRING30
	{SPR_RNGS, 30, 1, {NULL}, 0, 0, S_SCATTERRING32}, // S_SCATTERRING31
	{SPR_RNGS, 31, 1, {NULL}, 0, 0, S_SCATTERRING33}, // S_SCATTERRING32
	{SPR_RNGS, 32, 1, {NULL}, 0, 0, S_SCATTERRING34}, // S_SCATTERRING33
	{SPR_RNGS, 33, 1, {NULL}, 0, 0, S_SCATTERRING35}, // S_SCATTERRING34
	{SPR_RNGS, 34, 1, {NULL}, 0, 0, S_SCATTERRING1},  // S_SCATTERRING35

	// Grenade Ring
	{SPR_RNGG, 0, 1, {NULL}, 0, 0, S_GRENADERING2},   // S_GRENADERING1
	{SPR_RNGG, 1, 1, {NULL}, 0, 0, S_GRENADERING3},   // S_GRENADERING2
	{SPR_RNGG, 2, 1, {NULL}, 0, 0, S_GRENADERING4},   // S_GRENADERING3
	{SPR_RNGG, 3, 1, {NULL}, 0, 0, S_GRENADERING5},   // S_GRENADERING4
	{SPR_RNGG, 4, 1, {NULL}, 0, 0, S_GRENADERING6},   // S_GRENADERING5
	{SPR_RNGG, 5, 1, {NULL}, 0, 0, S_GRENADERING7},   // S_GRENADERING6
	{SPR_RNGG, 6, 1, {NULL}, 0, 0, S_GRENADERING8},   // S_GRENADERING7
	{SPR_RNGG, 7, 1, {NULL}, 0, 0, S_GRENADERING9},   // S_GRENADERING8
	{SPR_RNGG, 8, 1, {NULL}, 0, 0, S_GRENADERING10},  // S_GRENADERING9
	{SPR_RNGG, 9, 1, {NULL}, 0, 0, S_GRENADERING11},  // S_GRENADERING10
	{SPR_RNGG, 10, 1, {NULL}, 0, 0, S_GRENADERING12}, // S_GRENADERING11
	{SPR_RNGG, 11, 1, {NULL}, 0, 0, S_GRENADERING13}, // S_GRENADERING12
	{SPR_RNGG, 12, 1, {NULL}, 0, 0, S_GRENADERING14}, // S_GRENADERING13
	{SPR_RNGG, 13, 1, {NULL}, 0, 0, S_GRENADERING15}, // S_GRENADERING14
	{SPR_RNGG, 14, 1, {NULL}, 0, 0, S_GRENADERING16}, // S_GRENADERING15
	{SPR_RNGG, 15, 1, {NULL}, 0, 0, S_GRENADERING17}, // S_GRENADERING16
	{SPR_RNGG, 16, 1, {NULL}, 0, 0, S_GRENADERING18}, // S_GRENADERING17
	{SPR_RNGG, 17, 1, {NULL}, 0, 0, S_GRENADERING19}, // S_GRENADERING18
	{SPR_RNGG, 18, 1, {NULL}, 0, 0, S_GRENADERING20}, // S_GRENADERING19
	{SPR_RNGG, 19, 1, {NULL}, 0, 0, S_GRENADERING21}, // S_GRENADERING20
	{SPR_RNGG, 20, 1, {NULL}, 0, 0, S_GRENADERING22}, // S_GRENADERING21
	{SPR_RNGG, 21, 1, {NULL}, 0, 0, S_GRENADERING23}, // S_GRENADERING22
	{SPR_RNGG, 22, 1, {NULL}, 0, 0, S_GRENADERING24}, // S_GRENADERING23
	{SPR_RNGG, 23, 1, {NULL}, 0, 0, S_GRENADERING25}, // S_GRENADERING24
	{SPR_RNGG, 24, 1, {NULL}, 0, 0, S_GRENADERING26}, // S_GRENADERING25
	{SPR_RNGG, 25, 1, {NULL}, 0, 0, S_GRENADERING27}, // S_GRENADERING26
	{SPR_RNGG, 26, 1, {NULL}, 0, 0, S_GRENADERING28}, // S_GRENADERING27
	{SPR_RNGG, 27, 1, {NULL}, 0, 0, S_GRENADERING29}, // S_GRENADERING28
	{SPR_RNGG, 28, 1, {NULL}, 0, 0, S_GRENADERING30}, // S_GRENADERING29
	{SPR_RNGG, 29, 1, {NULL}, 0, 0, S_GRENADERING31}, // S_GRENADERING30
	{SPR_RNGG, 30, 1, {NULL}, 0, 0, S_GRENADERING32}, // S_GRENADERING31
	{SPR_RNGG, 31, 1, {NULL}, 0, 0, S_GRENADERING33}, // S_GRENADERING32
	{SPR_RNGG, 32, 1, {NULL}, 0, 0, S_GRENADERING34}, // S_GRENADERING33
	{SPR_RNGG, 33, 1, {NULL}, 0, 0, S_GRENADERING35}, // S_GRENADERING34
	{SPR_RNGG, 34, 1, {NULL}, 0, 0, S_GRENADERING1},  // S_GRENADERING35

	// Bounce Ring Pickup
	{SPR_PIKB,  0, 1, {NULL}, 0, 0, S_BOUNCEPICKUP2},  // S_BOUNCEPICKUP1
	{SPR_PIKB,  1, 1, {NULL}, 0, 0, S_BOUNCEPICKUP3},  // S_BOUNCEPICKUP2
	{SPR_PIKB,  2, 1, {NULL}, 0, 0, S_BOUNCEPICKUP4},  // S_BOUNCEPICKUP3
	{SPR_PIKB,  3, 1, {NULL}, 0, 0, S_BOUNCEPICKUP5},  // S_BOUNCEPICKUP4
	{SPR_PIKB,  4, 1, {NULL}, 0, 0, S_BOUNCEPICKUP6},  // S_BOUNCEPICKUP5
	{SPR_PIKB,  5, 1, {NULL}, 0, 0, S_BOUNCEPICKUP7},  // S_BOUNCEPICKUP6
	{SPR_PIKB,  6, 1, {NULL}, 0, 0, S_BOUNCEPICKUP8},  // S_BOUNCEPICKUP7
	{SPR_PIKB,  7, 1, {NULL}, 0, 0, S_BOUNCEPICKUP9},  // S_BOUNCEPICKUP8
	{SPR_PIKB,  8, 1, {NULL}, 0, 0, S_BOUNCEPICKUP10}, // S_BOUNCEPICKUP9
	{SPR_PIKB,  9, 1, {NULL}, 0, 0, S_BOUNCEPICKUP11}, // S_BOUNCEPICKUP10
	{SPR_PIKB, 10, 1, {NULL}, 0, 0, S_BOUNCEPICKUP12}, // S_BOUNCEPICKUP11
	{SPR_PIKB, 11, 1, {NULL}, 0, 0, S_BOUNCEPICKUP13}, // S_BOUNCEPICKUP12
	{SPR_PIKB, 12, 1, {NULL}, 0, 0, S_BOUNCEPICKUP14}, // S_BOUNCEPICKUP13
	{SPR_PIKB, 13, 1, {NULL}, 0, 0, S_BOUNCEPICKUP15}, // S_BOUNCEPICKUP14
	{SPR_PIKB, 14, 1, {NULL}, 0, 0, S_BOUNCEPICKUP16}, // S_BOUNCEPICKUP15
	{SPR_PIKB, 15, 1, {NULL}, 0, 0, S_BOUNCEPICKUP1},  // S_BOUNCEPICKUP16

	// Bounce Ring Pickup Fade
	{SPR_PIKB,  0, 1, {NULL}, 0, 0, S_BOUNCEPICKUPFADE2}, // S_BOUNCEPICKUPFADE1
	{SPR_PIKB,  2, 1, {NULL}, 0, 0, S_BOUNCEPICKUPFADE3}, // S_BOUNCEPICKUPFADE2
	{SPR_PIKB,  4, 1, {NULL}, 0, 0, S_BOUNCEPICKUPFADE4}, // S_BOUNCEPICKUPFADE3
	{SPR_PIKB,  6, 1, {NULL}, 0, 0, S_BOUNCEPICKUPFADE5}, // S_BOUNCEPICKUPFADE4
	{SPR_PIKB,  8, 1, {NULL}, 0, 0, S_BOUNCEPICKUPFADE6}, // S_BOUNCEPICKUPFADE5
	{SPR_PIKB, 10, 1, {NULL}, 0, 0, S_BOUNCEPICKUPFADE7}, // S_BOUNCEPICKUPFADE6
	{SPR_PIKB, 12, 1, {NULL}, 0, 0, S_BOUNCEPICKUPFADE8}, // S_BOUNCEPICKUPFADE7
	{SPR_PIKB, 14, 1, {NULL}, 0, 0, S_BOUNCEPICKUPFADE1}, // S_BOUNCEPICKUPFADE8

	// Rail Ring Pickup
	{SPR_PIKR,  0, 1, {NULL}, 0, 0, S_RAILPICKUP2},  // S_RAILPICKUP1
	{SPR_PIKR,  1, 1, {NULL}, 0, 0, S_RAILPICKUP3},  // S_RAILPICKUP2
	{SPR_PIKR,  2, 1, {NULL}, 0, 0, S_RAILPICKUP4},  // S_RAILPICKUP3
	{SPR_PIKR,  3, 1, {NULL}, 0, 0, S_RAILPICKUP5},  // S_RAILPICKUP4
	{SPR_PIKR,  4, 1, {NULL}, 0, 0, S_RAILPICKUP6},  // S_RAILPICKUP5
	{SPR_PIKR,  5, 1, {NULL}, 0, 0, S_RAILPICKUP7},  // S_RAILPICKUP6
	{SPR_PIKR,  6, 1, {NULL}, 0, 0, S_RAILPICKUP8},  // S_RAILPICKUP7
	{SPR_PIKR,  7, 1, {NULL}, 0, 0, S_RAILPICKUP9},  // S_RAILPICKUP8
	{SPR_PIKR,  8, 1, {NULL}, 0, 0, S_RAILPICKUP10}, // S_RAILPICKUP9
	{SPR_PIKR,  9, 1, {NULL}, 0, 0, S_RAILPICKUP11}, // S_RAILPICKUP10
	{SPR_PIKR, 10, 1, {NULL}, 0, 0, S_RAILPICKUP12}, // S_RAILPICKUP11
	{SPR_PIKR, 11, 1, {NULL}, 0, 0, S_RAILPICKUP13}, // S_RAILPICKUP12
	{SPR_PIKR, 12, 1, {NULL}, 0, 0, S_RAILPICKUP14}, // S_RAILPICKUP13
	{SPR_PIKR, 13, 1, {NULL}, 0, 0, S_RAILPICKUP15}, // S_RAILPICKUP14
	{SPR_PIKR, 14, 1, {NULL}, 0, 0, S_RAILPICKUP16}, // S_RAILPICKUP15
	{SPR_PIKR, 15, 1, {NULL}, 0, 0, S_RAILPICKUP1},  // S_RAILPICKUP16

	// Rail Ring Pickup Fade
	{SPR_PIKR,  0, 1, {NULL}, 0, 0, S_RAILPICKUPFADE2}, // S_RAILPICKUPFADE1
	{SPR_PIKR,  2, 1, {NULL}, 0, 0, S_RAILPICKUPFADE3}, // S_RAILPICKUPFADE2
	{SPR_PIKR,  4, 1, {NULL}, 0, 0, S_RAILPICKUPFADE4}, // S_RAILPICKUPFADE3
	{SPR_PIKR,  6, 1, {NULL}, 0, 0, S_RAILPICKUPFADE5}, // S_RAILPICKUPFADE4
	{SPR_PIKR,  8, 1, {NULL}, 0, 0, S_RAILPICKUPFADE6}, // S_RAILPICKUPFADE5
	{SPR_PIKR, 10, 1, {NULL}, 0, 0, S_RAILPICKUPFADE7}, // S_RAILPICKUPFADE6
	{SPR_PIKR, 12, 1, {NULL}, 0, 0, S_RAILPICKUPFADE8}, // S_RAILPICKUPFADE7
	{SPR_PIKR, 14, 1, {NULL}, 0, 0, S_RAILPICKUPFADE1}, // S_RAILPICKUPFADE8

	// Auto Ring Pickup
	{SPR_PIKA,  0, 1, {NULL}, 0, 0, S_AUTOPICKUP2},  // S_AUTOPICKUP1
	{SPR_PIKA,  1, 1, {NULL}, 0, 0, S_AUTOPICKUP3},  // S_AUTOPICKUP2
	{SPR_PIKA,  2, 1, {NULL}, 0, 0, S_AUTOPICKUP4},  // S_AUTOPICKUP3
	{SPR_PIKA,  3, 1, {NULL}, 0, 0, S_AUTOPICKUP5},  // S_AUTOPICKUP4
	{SPR_PIKA,  4, 1, {NULL}, 0, 0, S_AUTOPICKUP6},  // S_AUTOPICKUP5
	{SPR_PIKA,  5, 1, {NULL}, 0, 0, S_AUTOPICKUP7},  // S_AUTOPICKUP6
	{SPR_PIKA,  6, 1, {NULL}, 0, 0, S_AUTOPICKUP8},  // S_AUTOPICKUP7
	{SPR_PIKA,  7, 1, {NULL}, 0, 0, S_AUTOPICKUP9},  // S_AUTOPICKUP8
	{SPR_PIKA,  8, 1, {NULL}, 0, 0, S_AUTOPICKUP10}, // S_AUTOPICKUP9
	{SPR_PIKA,  9, 1, {NULL}, 0, 0, S_AUTOPICKUP11}, // S_AUTOPICKUP10
	{SPR_PIKA, 10, 1, {NULL}, 0, 0, S_AUTOPICKUP12}, // S_AUTOPICKUP11
	{SPR_PIKA, 11, 1, {NULL}, 0, 0, S_AUTOPICKUP13}, // S_AUTOPICKUP12
	{SPR_PIKA, 12, 1, {NULL}, 0, 0, S_AUTOPICKUP14}, // S_AUTOPICKUP13
	{SPR_PIKA, 13, 1, {NULL}, 0, 0, S_AUTOPICKUP15}, // S_AUTOPICKUP14
	{SPR_PIKA, 14, 1, {NULL}, 0, 0, S_AUTOPICKUP16}, // S_AUTOPICKUP15
	{SPR_PIKA, 15, 1, {NULL}, 0, 0, S_AUTOPICKUP1},  // S_AUTOPICKUP16

	{SPR_PIKA,  0, 1, {NULL}, 0, 0, S_AUTOPICKUPFADE2}, // S_AUTOPICKUPFADE1
	{SPR_PIKA,  2, 1, {NULL}, 0, 0, S_AUTOPICKUPFADE3}, // S_AUTOPICKUPFADE2
	{SPR_PIKA,  4, 1, {NULL}, 0, 0, S_AUTOPICKUPFADE4}, // S_AUTOPICKUPFADE3
	{SPR_PIKA,  6, 1, {NULL}, 0, 0, S_AUTOPICKUPFADE5}, // S_AUTOPICKUPFADE4
	{SPR_PIKA,  8, 1, {NULL}, 0, 0, S_AUTOPICKUPFADE6}, // S_AUTOPICKUPFADE5
	{SPR_PIKA, 10, 1, {NULL}, 0, 0, S_AUTOPICKUPFADE7}, // S_AUTOPICKUPFADE6
	{SPR_PIKA, 12, 1, {NULL}, 0, 0, S_AUTOPICKUPFADE8}, // S_AUTOPICKUPFADE7
	{SPR_PIKA, 14, 1, {NULL}, 0, 0, S_AUTOPICKUPFADE1}, // S_AUTOPICKUPFADE8

	// Explode Ring Pickup
	{SPR_PIKE,  0, 1, {NULL}, 0, 0, S_EXPLODEPICKUP2},  // S_EXPLODEPICKUP1
	{SPR_PIKE,  1, 1, {NULL}, 0, 0, S_EXPLODEPICKUP3},  // S_EXPLODEPICKUP2
	{SPR_PIKE,  2, 1, {NULL}, 0, 0, S_EXPLODEPICKUP4},  // S_EXPLODEPICKUP3
	{SPR_PIKE,  3, 1, {NULL}, 0, 0, S_EXPLODEPICKUP5},  // S_EXPLODEPICKUP4
	{SPR_PIKE,  4, 1, {NULL}, 0, 0, S_EXPLODEPICKUP6},  // S_EXPLODEPICKUP5
	{SPR_PIKE,  5, 1, {NULL}, 0, 0, S_EXPLODEPICKUP7},  // S_EXPLODEPICKUP6
	{SPR_PIKE,  6, 1, {NULL}, 0, 0, S_EXPLODEPICKUP8},  // S_EXPLODEPICKUP7
	{SPR_PIKE,  7, 1, {NULL}, 0, 0, S_EXPLODEPICKUP9},  // S_EXPLODEPICKUP8
	{SPR_PIKE,  8, 1, {NULL}, 0, 0, S_EXPLODEPICKUP10}, // S_EXPLODEPICKUP9
	{SPR_PIKE,  9, 1, {NULL}, 0, 0, S_EXPLODEPICKUP11}, // S_EXPLODEPICKUP10
	{SPR_PIKE, 10, 1, {NULL}, 0, 0, S_EXPLODEPICKUP12}, // S_EXPLODEPICKUP11
	{SPR_PIKE, 11, 1, {NULL}, 0, 0, S_EXPLODEPICKUP13}, // S_EXPLODEPICKUP12
	{SPR_PIKE, 12, 1, {NULL}, 0, 0, S_EXPLODEPICKUP14}, // S_EXPLODEPICKUP13
	{SPR_PIKE, 13, 1, {NULL}, 0, 0, S_EXPLODEPICKUP15}, // S_EXPLODEPICKUP14
	{SPR_PIKE, 14, 1, {NULL}, 0, 0, S_EXPLODEPICKUP16}, // S_EXPLODEPICKUP15
	{SPR_PIKE, 15, 1, {NULL}, 0, 0, S_EXPLODEPICKUP1},  // S_EXPLODEPICKUP16

	{SPR_PIKE,  0, 1, {NULL}, 0, 0, S_EXPLODEPICKUPFADE2}, // S_EXPLODEPICKUPFADE1
	{SPR_PIKE,  2, 1, {NULL}, 0, 0, S_EXPLODEPICKUPFADE3}, // S_EXPLODEPICKUPFADE2
	{SPR_PIKE,  4, 1, {NULL}, 0, 0, S_EXPLODEPICKUPFADE4}, // S_EXPLODEPICKUPFADE3
	{SPR_PIKE,  6, 1, {NULL}, 0, 0, S_EXPLODEPICKUPFADE5}, // S_EXPLODEPICKUPFADE4
	{SPR_PIKE,  8, 1, {NULL}, 0, 0, S_EXPLODEPICKUPFADE6}, // S_EXPLODEPICKUPFADE5
	{SPR_PIKE, 10, 1, {NULL}, 0, 0, S_EXPLODEPICKUPFADE7}, // S_EXPLODEPICKUPFADE6
	{SPR_PIKE, 12, 1, {NULL}, 0, 0, S_EXPLODEPICKUPFADE8}, // S_EXPLODEPICKUPFADE7
	{SPR_PIKE, 14, 1, {NULL}, 0, 0, S_EXPLODEPICKUPFADE1}, // S_EXPLODEPICKUPFADE8

	// Scatter Ring Pickup
	{SPR_PIKS,  0, 1, {NULL}, 0, 0, S_SCATTERPICKUP2},  // S_SCATTERPICKUP1
	{SPR_PIKS,  1, 1, {NULL}, 0, 0, S_SCATTERPICKUP3},  // S_SCATTERPICKUP2
	{SPR_PIKS,  2, 1, {NULL}, 0, 0, S_SCATTERPICKUP4},  // S_SCATTERPICKUP3
	{SPR_PIKS,  3, 1, {NULL}, 0, 0, S_SCATTERPICKUP5},  // S_SCATTERPICKUP4
	{SPR_PIKS,  4, 1, {NULL}, 0, 0, S_SCATTERPICKUP6},  // S_SCATTERPICKUP5
	{SPR_PIKS,  5, 1, {NULL}, 0, 0, S_SCATTERPICKUP7},  // S_SCATTERPICKUP6
	{SPR_PIKS,  6, 1, {NULL}, 0, 0, S_SCATTERPICKUP8},  // S_SCATTERPICKUP7
	{SPR_PIKS,  7, 1, {NULL}, 0, 0, S_SCATTERPICKUP9},  // S_SCATTERPICKUP8
	{SPR_PIKS,  8, 1, {NULL}, 0, 0, S_SCATTERPICKUP10}, // S_SCATTERPICKUP9
	{SPR_PIKS,  9, 1, {NULL}, 0, 0, S_SCATTERPICKUP11}, // S_SCATTERPICKUP10
	{SPR_PIKS, 10, 1, {NULL}, 0, 0, S_SCATTERPICKUP12}, // S_SCATTERPICKUP11
	{SPR_PIKS, 11, 1, {NULL}, 0, 0, S_SCATTERPICKUP13}, // S_SCATTERPICKUP12
	{SPR_PIKS, 12, 1, {NULL}, 0, 0, S_SCATTERPICKUP14}, // S_SCATTERPICKUP13
	{SPR_PIKS, 13, 1, {NULL}, 0, 0, S_SCATTERPICKUP15}, // S_SCATTERPICKUP14
	{SPR_PIKS, 14, 1, {NULL}, 0, 0, S_SCATTERPICKUP16}, // S_SCATTERPICKUP15
	{SPR_PIKS, 15, 1, {NULL}, 0, 0, S_SCATTERPICKUP1},  // S_SCATTERPICKUP16

	{SPR_PIKS,  0, 1, {NULL}, 0, 0, S_SCATTERPICKUPFADE2}, // S_SCATTERPICKUPFADE1
	{SPR_PIKS,  2, 1, {NULL}, 0, 0, S_SCATTERPICKUPFADE3}, // S_SCATTERPICKUPFADE2
	{SPR_PIKS,  4, 1, {NULL}, 0, 0, S_SCATTERPICKUPFADE4}, // S_SCATTERPICKUPFADE3
	{SPR_PIKS,  6, 1, {NULL}, 0, 0, S_SCATTERPICKUPFADE5}, // S_SCATTERPICKUPFADE4
	{SPR_PIKS,  8, 1, {NULL}, 0, 0, S_SCATTERPICKUPFADE6}, // S_SCATTERPICKUPFADE5
	{SPR_PIKS, 10, 1, {NULL}, 0, 0, S_SCATTERPICKUPFADE7}, // S_SCATTERPICKUPFADE6
	{SPR_PIKS, 12, 1, {NULL}, 0, 0, S_SCATTERPICKUPFADE8}, // S_SCATTERPICKUPFADE7
	{SPR_PIKS, 14, 1, {NULL}, 0, 0, S_SCATTERPICKUPFADE1}, // S_SCATTERPICKUPFADE8

	// Grenade Ring Pickup
	{SPR_PIKG,  0, 1, {NULL}, 0, 0, S_GRENADEPICKUP2},  // S_GRENADEPICKUP1
	{SPR_PIKG,  1, 1, {NULL}, 0, 0, S_GRENADEPICKUP3},  // S_GRENADEPICKUP2
	{SPR_PIKG,  2, 1, {NULL}, 0, 0, S_GRENADEPICKUP4},  // S_GRENADEPICKUP3
	{SPR_PIKG,  3, 1, {NULL}, 0, 0, S_GRENADEPICKUP5},  // S_GRENADEPICKUP4
	{SPR_PIKG,  4, 1, {NULL}, 0, 0, S_GRENADEPICKUP6},  // S_GRENADEPICKUP5
	{SPR_PIKG,  5, 1, {NULL}, 0, 0, S_GRENADEPICKUP7},  // S_GRENADEPICKUP6
	{SPR_PIKG,  6, 1, {NULL}, 0, 0, S_GRENADEPICKUP8},  // S_GRENADEPICKUP7
	{SPR_PIKG,  7, 1, {NULL}, 0, 0, S_GRENADEPICKUP9},  // S_GRENADEPICKUP8
	{SPR_PIKG,  8, 1, {NULL}, 0, 0, S_GRENADEPICKUP10}, // S_GRENADEPICKUP9
	{SPR_PIKG,  9, 1, {NULL}, 0, 0, S_GRENADEPICKUP11}, // S_GRENADEPICKUP10
	{SPR_PIKG, 10, 1, {NULL}, 0, 0, S_GRENADEPICKUP12}, // S_GRENADEPICKUP11
	{SPR_PIKG, 11, 1, {NULL}, 0, 0, S_GRENADEPICKUP13}, // S_GRENADEPICKUP12
	{SPR_PIKG, 12, 1, {NULL}, 0, 0, S_GRENADEPICKUP14}, // S_GRENADEPICKUP13
	{SPR_PIKG, 13, 1, {NULL}, 0, 0, S_GRENADEPICKUP15}, // S_GRENADEPICKUP14
	{SPR_PIKG, 14, 1, {NULL}, 0, 0, S_GRENADEPICKUP16}, // S_GRENADEPICKUP15
	{SPR_PIKG, 15, 1, {NULL}, 0, 0, S_GRENADEPICKUP1},  // S_GRENADEPICKUP16

	{SPR_PIKG,  0, 1, {NULL}, 0, 0, S_GRENADEPICKUPFADE2}, // S_GRENADEPICKUPFADE1
	{SPR_PIKG,  2, 1, {NULL}, 0, 0, S_GRENADEPICKUPFADE3}, // S_GRENADEPICKUPFADE2
	{SPR_PIKG,  4, 1, {NULL}, 0, 0, S_GRENADEPICKUPFADE4}, // S_GRENADEPICKUPFADE3
	{SPR_PIKG,  6, 1, {NULL}, 0, 0, S_GRENADEPICKUPFADE5}, // S_GRENADEPICKUPFADE4
	{SPR_PIKG,  8, 1, {NULL}, 0, 0, S_GRENADEPICKUPFADE6}, // S_GRENADEPICKUPFADE5
	{SPR_PIKG, 10, 1, {NULL}, 0, 0, S_GRENADEPICKUPFADE7}, // S_GRENADEPICKUPFADE6
	{SPR_PIKG, 12, 1, {NULL}, 0, 0, S_GRENADEPICKUPFADE8}, // S_GRENADEPICKUPFADE7
	{SPR_PIKG, 14, 1, {NULL}, 0, 0, S_GRENADEPICKUPFADE1}, // S_GRENADEPICKUPFADE8

	// Thrown Weapon Rings
	{SPR_RNGB, 32768, 1, {A_ThrownRing}, 0, 0, S_THROWNBOUNCE2}, // S_THROWNBOUNCE1
	{SPR_RNGB, 32773, 1, {A_ThrownRing}, 0, 0, S_THROWNBOUNCE3}, // S_THROWNBOUNCE2
	{SPR_RNGB, 32778, 1, {A_ThrownRing}, 0, 0, S_THROWNBOUNCE4}, // S_THROWNBOUNCE3
	{SPR_RNGB, 32783, 1, {A_ThrownRing}, 0, 0, S_THROWNBOUNCE5}, // S_THROWNBOUNCE4
	{SPR_RNGB, 32788, 1, {A_ThrownRing}, 0, 0, S_THROWNBOUNCE6}, // S_THROWNBOUNCE5
	{SPR_RNGB, 32793, 1, {A_ThrownRing}, 0, 0, S_THROWNBOUNCE7}, // S_THROWNBOUNCE6
	{SPR_RNGB, 32798, 1, {A_ThrownRing}, 0, 0, S_THROWNBOUNCE1}, // S_THROWNBOUNCE7

	{SPR_TAUT, 32768, 1, {A_ThrownRing}, 0, 0, S_THROWNAUTOMATIC2}, // S_THROWNAUTOMATIC1
	{SPR_TAUT, 32769, 1, {A_ThrownRing}, 0, 0, S_THROWNAUTOMATIC3}, // S_THROWNAUTOMATIC2
	{SPR_TAUT, 32770, 1, {A_ThrownRing}, 0, 0, S_THROWNAUTOMATIC4}, // S_THROWNAUTOMATIC3
	{SPR_TAUT, 32771, 1, {A_ThrownRing}, 0, 0, S_THROWNAUTOMATIC5}, // S_THROWNAUTOMATIC4
	{SPR_TAUT, 32772, 1, {A_ThrownRing}, 0, 0, S_THROWNAUTOMATIC6}, // S_THROWNAUTOMATIC5
	{SPR_TAUT, 32773, 1, {A_ThrownRing}, 0, 0, S_THROWNAUTOMATIC7}, // S_THROWNAUTOMATIC6
	{SPR_TAUT, 32774, 1, {A_ThrownRing}, 0, 0, S_THROWNAUTOMATIC1}, // S_THROWNAUTOMATIC7

	{SPR_RNGE, 32768, 1, {A_ThrownRing}, 0, 0, S_THROWNEXPLOSION2}, // S_THROWNEXPLOSION1
	{SPR_RNGE, 32773, 1, {A_ThrownRing}, 0, 0, S_THROWNEXPLOSION3}, // S_THROWNEXPLOSION2
	{SPR_RNGE, 32778, 1, {A_ThrownRing}, 0, 0, S_THROWNEXPLOSION4}, // S_THROWNEXPLOSION3
	{SPR_RNGE, 32783, 1, {A_ThrownRing}, 0, 0, S_THROWNEXPLOSION5}, // S_THROWNEXPLOSION4
	{SPR_RNGE, 32788, 1, {A_ThrownRing}, 0, 0, S_THROWNEXPLOSION6}, // S_THROWNEXPLOSION5
	{SPR_RNGE, 32793, 1, {A_ThrownRing}, 0, 0, S_THROWNEXPLOSION7}, // S_THROWNEXPLOSION6
	{SPR_RNGE, 32798, 1, {A_ThrownRing}, 0, 0, S_THROWNEXPLOSION1}, // S_THROWNEXPLOSION7

	{SPR_TGRE, 32768, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE2},  // S_THROWNGRENADE1
	{SPR_TGRE, 32769, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE3},  // S_THROWNGRENADE2
	{SPR_TGRE, 32770, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE4},  // S_THROWNGRENADE3
	{SPR_TGRE, 32771, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE5},  // S_THROWNGRENADE4
	{SPR_TGRE, 32772, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE6},  // S_THROWNGRENADE5
	{SPR_TGRE, 32773, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE7},  // S_THROWNGRENADE6
	{SPR_TGRE, 32774, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE8},  // S_THROWNGRENADE7
	{SPR_TGRE, 32775, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE9},  // S_THROWNGRENADE8
	{SPR_TGRE, 32776, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE10}, // S_THROWNGRENADE9
	{SPR_TGRE, 32777, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE11}, // S_THROWNGRENADE10
	{SPR_TGRE, 32778, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE12}, // S_THROWNGRENADE11
	{SPR_TGRE, 32779, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE13}, // S_THROWNGRENADE12
	{SPR_TGRE, 32780, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE14}, // S_THROWNGRENADE13
	{SPR_TGRE, 32781, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE15}, // S_THROWNGRENADE14
	{SPR_TGRE, 32782, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE16}, // S_THROWNGRENADE15
	{SPR_TGRE, 32783, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE17}, // S_THROWNGRENADE16
	{SPR_TGRE, 32784, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE18}, // S_THROWNGRENADE17
	{SPR_TGRE, 32785, 1, {A_GrenadeRing}, 0, 0, S_THROWNGRENADE1},  // S_THROWNGRENADE18

	{SPR_TSCR, 0, 1, {A_ThrownRing}, 0, 0, S_THROWNSCATTER}, // S_THROWNSCATTER

	{SPR_DISS, 0, 1, {A_RingExplode}, MT_REDRING, 0, S_XPLD1}, // S_RINGEXPLODE

	// Coin
	{SPR_COIN, 32768, 5, {A_AttractChase}, 0, 0, S_COIN2}, // S_COIN1
	{SPR_COIN, 32769, 5, {A_AttractChase}, 0, 0, S_COIN3}, // S_COIN2
	{SPR_COIN, 32770, 5, {A_AttractChase}, 0, 0, S_COIN1}, // S_COIN3

	// Coin Sparkle
	{SPR_CPRK, 32768, 5, {NULL}, 0, 0, S_COINSPARKLE2}, // S_COINSPARKLE1
	{SPR_CPRK, 32769, 5, {NULL}, 0, 0, S_COINSPARKLE3}, // S_COINSPARKLE2
	{SPR_CPRK, 32770, 5, {NULL}, 0, 0, S_COINSPARKLE4}, // S_COINSPARKLE3
	{SPR_CPRK, 32771, 5, {NULL}, 0, 0, S_DISS},         // S_COINSPARKLE4

	// Goomba
	{SPR_GOOM, 0, 6, {A_Look}, 0, 0, S_GOOMBA1B}, // S_GOOMBA1
	{SPR_GOOM, 1, 6, {A_Look}, 0, 0, S_GOOMBA1},  // S_GOOMBA1B
	{SPR_GOOM, 0, 3, {A_Chase}, 0, 0, S_GOOMBA3}, // S_GOOMBA2
	{SPR_GOOM, 0, 3, {A_Chase}, 0, 0, S_GOOMBA4}, // S_GOOMBA3
	{SPR_GOOM, 1, 3, {A_Chase}, 0, 0, S_GOOMBA5}, // S_GOOMBA4
	{SPR_GOOM, 1, 3, {A_Chase}, 0, 0, S_GOOMBA6}, // S_GOOMBA5
	{SPR_GOOM, 0, 3, {A_Chase}, 0, 0, S_GOOMBA7}, // S_GOOMBA6
	{SPR_GOOM, 0, 3, {A_Chase}, 0, 0, S_GOOMBA8}, // S_GOOMBA7
	{SPR_GOOM, 1, 3, {A_Chase}, 0, 0, S_GOOMBA9}, // S_GOOMBA8
	{SPR_GOOM, 1, 3, {A_Chase}, 0, 0, S_GOOMBA2}, // S_GOOMBA9
	{SPR_GOOM, 2, 16, {A_Scream}, 0, 0, S_DISS},  // S_GOOMBA_DEAD

	// Blue Goomba
	{SPR_BGOM, 0, 6, {A_Look}, 0, 0, S_BLUEGOOMBA1B}, // BLUEGOOMBA1
	{SPR_BGOM, 1, 6, {A_Look}, 0, 0, S_BLUEGOOMBA1},  // BLUEGOOMBA1B
	{SPR_BGOM, 0, 3, {A_Chase}, 0, 0, S_BLUEGOOMBA3}, // S_BLUEGOOMBA2
	{SPR_BGOM, 0, 3, {A_Chase}, 0, 0, S_BLUEGOOMBA4}, // S_BLUEGOOMBA3
	{SPR_BGOM, 1, 3, {A_Chase}, 0, 0, S_BLUEGOOMBA5}, // S_BLUEGOOMBA4
	{SPR_BGOM, 1, 3, {A_Chase}, 0, 0, S_BLUEGOOMBA6}, // S_BLUEGOOMBA5
	{SPR_BGOM, 0, 3, {A_Chase}, 0, 0, S_BLUEGOOMBA7}, // S_BLUEGOOMBA6
	{SPR_BGOM, 0, 3, {A_Chase}, 0, 0, S_BLUEGOOMBA8}, // S_BLUEGOOMBA7
	{SPR_BGOM, 1, 3, {A_Chase}, 0, 0, S_BLUEGOOMBA9}, // S_BLUEGOOMBA8
	{SPR_BGOM, 1, 3, {A_Chase}, 0, 0, S_BLUEGOOMBA2}, // S_BLUEGOOMBA9
	{SPR_BGOM, 2, 16, {A_Scream}, 0, 0, S_DISS},      // S_BLUEGOOMBA_DEAD

	// Fire Flower
	{SPR_FFWR, 0, 3, {NULL}, 0, 0, S_FIREFLOWER2}, // S_FIREFLOWER1
	{SPR_FFWR, 1, 3, {NULL}, 0, 0, S_FIREFLOWER3}, // S_FIREFLOWER2
	{SPR_FFWR, 2, 3, {NULL}, 0, 0, S_FIREFLOWER4}, // S_FIREFLOWER3
	{SPR_FFWR, 3, 3, {NULL}, 0, 0, S_FIREFLOWER1}, // S_FIREFLOWER4

	// Thrown Mario Fireball
	{SPR_FBLL, 32768, 3, {NULL}, 0, 0, S_FIREBALL2},    // S_FIREBALL1
	{SPR_FBLL, 32769, 3, {NULL}, 0, 0, S_FIREBALL3},    // S_FIREBALL2
	{SPR_FBLL, 32770, 3, {NULL}, 0, 0, S_FIREBALL4},    // S_FIREBALL3
	{SPR_FBLL, 32771, 3, {NULL}, 0, 0, S_FIREBALL1},    // S_FIREBALL4
	{SPR_FBLL, 32772, 3, {NULL}, 0, 0, S_FIREBALLEXP2}, // S_FIREBALLEXP1
	{SPR_FBLL, 32773, 3, {NULL}, 0, 0, S_FIREBALLEXP3}, // S_FIREBALLEXP2
	{SPR_FBLL, 32774, 3, {NULL}, 0, 0, S_NULL},         // S_FIREBALLEXP3

	// Turtle Shell
	{SPR_SHLL, 0, -1, {NULL}, 0, 0, S_NULL},  // S_SHELL
	{SPR_SHLL, 0, 2, {NULL}, 0, 0, S_SHELL2}, // S_SHELL1
	{SPR_SHLL, 1, 2, {NULL}, 0, 0, S_SHELL3}, // S_SHELL2
	{SPR_SHLL, 2, 2, {NULL}, 0, 0, S_SHELL4}, // S_SHELL3
	{SPR_SHLL, 3, 2, {NULL}, 0, 0, S_SHELL1}, // S_SHELL4

	// Puma (Mario fireball)
	{SPR_PUMA, 32768, 3, {A_FishJump}, 0, 0, S_PUMA2}, // S_PUMA1
	{SPR_PUMA, 32769, 3, {A_FishJump}, 0, 0, S_PUMA3}, // S_PUMA2
	{SPR_PUMA, 32770, 3, {A_FishJump}, 0, 0, S_PUMA1}, // S_PUMA3
	{SPR_PUMA, 32771, 3, {A_FishJump}, 0, 0, S_PUMA5}, // S_PUMA4
	{SPR_PUMA, 32772, 3, {A_FishJump}, 0, 0, S_PUMA6}, // S_PUMA5
	{SPR_PUMA, 32773, 3, {A_FishJump}, 0, 0, S_PUMA4}, // S_PUMA6

	// Hammer
	{SPR_HAMM, 0, 3, {NULL}, 0, 0, S_HAMMER2}, // S_HAMMER1
	{SPR_HAMM, 1, 3, {NULL}, 0, 0, S_HAMMER3}, // S_HAMMER2
	{SPR_HAMM, 2, 3, {NULL}, 0, 0, S_HAMMER4}, // S_HAMMER3
	{SPR_HAMM, 3, 3, {NULL}, 0, 0, S_HAMMER1}, // S_HAMMER4

	// Koopa
	{SPR_KOOP, 0, -1, {NULL}, 0, 0, S_NULL},   // S_KOOPA1
	{SPR_KOOP, 1, 24, {NULL}, 0, 0, S_KOOPA1}, // S_KOOPA2

	{SPR_BFLM, 0, 3,{NULL}, 0, 0, S_KOOPAFLAME2}, // S_KOOPAFLAME1
	{SPR_BFLM, 1, 3,{NULL}, 0, 0, S_KOOPAFLAME3}, // S_KOOPAFLAME2
	{SPR_BFLM, 2, 3,{NULL}, 0, 0, S_KOOPAFLAME1}, // S_KOOPAFLAME3

	// Axe
	{SPR_MAXE, 0, 3, {NULL}, 0, 0, S_AXE2}, // S_AXE1
	{SPR_MAXE, 1, 3, {NULL}, 0, 0, S_AXE3}, // S_AXE2
	{SPR_MAXE, 2, 3, {NULL}, 0, 0, S_AXE1}, // S_AXE3

	{SPR_MUS1, 0, -1, {NULL}, 0, 0, S_NULL}, // S_MARIOBUSH1
	{SPR_MUS2, 0, -1, {NULL}, 0, 0, S_NULL}, // S_MARIOBUSH2
	{SPR_TOAD, 0, -1, {NULL}, 0, 0, S_NULL}, // S_TOAD

	// Nights Drone
	{SPR_NDRN, 0, -1, {NULL}, 0, 0, S_NIGHTSDRONE2}, // S_NIGHTSDRONE1
	{SPR_NDRN, 0, -1, {NULL}, 0, 0, S_NIGHTSDRONE1}, // S_NIGHTSDRONE2

	// Nights Player, Flying
	{SPR_SUPE, 0, 1, {NULL}, 0, 0, S_NIGHTSFLY1B},  // S_NIGHTSFLY1A
	{SPR_SUPE, 1, 1, {NULL}, 0, 0, S_NIGHTSFLY1A},  // S_NIGHTSFLY1B
	{SPR_SUPE, 2, 1, {NULL}, 0, 0, S_NIGHTSFLY2B},  // S_NIGHTSFLY2A
	{SPR_SUPE, 3, 1, {NULL}, 0, 0, S_NIGHTSFLY2A},  // S_NIGHTSFLY2B
	{SPR_SUPE, 4, 1, {NULL}, 0, 0, S_NIGHTSFLY3B},  // S_NIGHTSFLY3A
	{SPR_SUPE, 5, 1, {NULL}, 0, 0, S_NIGHTSFLY3A},  // S_NIGHTSFLY3B
	{SPR_SUPE, 6, 1, {NULL}, 0, 0, S_NIGHTSFLY4B},  // S_NIGHTSFLY4A
	{SPR_SUPE, 7, 1, {NULL}, 0, 0, S_NIGHTSFLY4A},  // S_NIGHTSFLY4B
	{SPR_SUPE, 8, 1, {NULL}, 0, 0, S_NIGHTSFLY5B},  // S_NIGHTSFLY5A
	{SPR_SUPE, 9, 1, {NULL}, 0, 0, S_NIGHTSFLY5A},  // S_NIGHTSFLY5B
	{SPR_SUPE, 10, 1, {NULL}, 0, 0, S_NIGHTSFLY6B}, // S_NIGHTSFLY6A
	{SPR_SUPE, 11, 1, {NULL}, 0, 0, S_NIGHTSFLY6A}, // S_NIGHTSFLY6B
	{SPR_SUPE, 12, 1, {NULL}, 0, 0, S_NIGHTSFLY7B}, // S_NIGHTSFLY7A
	{SPR_SUPE, 13, 1, {NULL}, 0, 0, S_NIGHTSFLY7A}, // S_NIGHTSFLY7B
	{SPR_SUPE, 14, 1, {NULL}, 0, 0, S_NIGHTSFLY8B}, // S_NIGHTSFLY8A
	{SPR_SUPE, 15, 1, {NULL}, 0, 0, S_NIGHTSFLY8A}, // S_NIGHTSFLY8B
	{SPR_SUPE, 16, 1, {NULL}, 0, 0, S_NIGHTSFLY9B}, // S_NIGHTSFLY9A
	{SPR_SUPE, 17, 1, {NULL}, 0, 0, S_NIGHTSFLY9A}, // S_NIGHTSFLY9B

	// Nights Player, Falling
	{SPR_SUPZ, 0, 1, {NULL}, 0, 0, S_NIGHTSHURT2},   // S_NIGHTSHURT1
	{SPR_SUPZ, 1, 1, {NULL}, 0, 0, S_NIGHTSHURT3},   // S_NIGHTSHURT2
	{SPR_SUPZ, 2, 1, {NULL}, 0, 0, S_NIGHTSHURT4},   // S_NIGHTSHURT3
	{SPR_SUPZ, 3, 1, {NULL}, 0, 0, S_NIGHTSHURT5},   // S_NIGHTSHURT4
	{SPR_SUPZ, 4, 1, {NULL}, 0, 0, S_NIGHTSHURT6},   // S_NIGHTSHURT5
	{SPR_SUPZ, 5, 1, {NULL}, 0, 0, S_NIGHTSHURT7},   // S_NIGHTSHURT6
	{SPR_SUPZ, 6, 1, {NULL}, 0, 0, S_NIGHTSHURT8},   // S_NIGHTSHURT7
	{SPR_SUPZ, 7, 1, {NULL}, 0, 0, S_NIGHTSHURT9},   // S_NIGHTSHURT8
	{SPR_SUPZ, 8, 1, {NULL}, 0, 0, S_NIGHTSHURT10},  // S_NIGHTSHURT9
	{SPR_SUPZ, 9, 1, {NULL}, 0, 0, S_NIGHTSHURT11},  // S_NIGHTSHURT10
	{SPR_SUPZ, 10, 1, {NULL}, 0, 0, S_NIGHTSHURT12}, // S_NIGHTSHURT11
	{SPR_SUPZ, 11, 1, {NULL}, 0, 0, S_NIGHTSHURT13}, // S_NIGHTSHURT12
	{SPR_SUPZ, 12, 1, {NULL}, 0, 0, S_NIGHTSHURT14}, // S_NIGHTSHURT13
	{SPR_SUPZ, 13, 1, {NULL}, 0, 0, S_NIGHTSHURT15}, // S_NIGHTSHURT14
	{SPR_SUPZ, 14, 1, {NULL}, 0, 0, S_NIGHTSHURT16}, // S_NIGHTSHURT15
	{SPR_SUPZ, 15, 1, {NULL}, 0, 0, S_NIGHTSHURT17}, // S_NIGHTSHURT16
	{SPR_SUPZ, 0, 1, {NULL}, 0, 0, S_NIGHTSHURT18},  // S_NIGHTSHURT17
	{SPR_SUPZ, 1, 1, {NULL}, 0, 0, S_NIGHTSHURT19},  // S_NIGHTSHURT18
	{SPR_SUPZ, 2, 1, {NULL}, 0, 0, S_NIGHTSHURT20},  // S_NIGHTSHURT19
	{SPR_SUPZ, 3, 1, {NULL}, 0, 0, S_NIGHTSHURT21},  // S_NIGHTSHURT20
	{SPR_SUPZ, 4, 1, {NULL}, 0, 0, S_NIGHTSHURT22},  // S_NIGHTSHURT21
	{SPR_SUPZ, 5, 1, {NULL}, 0, 0, S_NIGHTSHURT23},  // S_NIGHTSHURT22
	{SPR_SUPZ, 6, 1, {NULL}, 0, 0, S_NIGHTSHURT24},  // S_NIGHTSHURT23
	{SPR_SUPZ, 7, 1, {NULL}, 0, 0, S_NIGHTSHURT25},  // S_NIGHTSHURT24
	{SPR_SUPZ, 8, 1, {NULL}, 0, 0, S_NIGHTSHURT26},  // S_NIGHTSHURT25
	{SPR_SUPZ, 9, 1, {NULL}, 0, 0, S_NIGHTSHURT27},  // S_NIGHTSHURT26
	{SPR_SUPZ, 10, 1, {NULL}, 0, 0, S_NIGHTSHURT28}, // S_NIGHTSHURT27
	{SPR_SUPZ, 11, 1, {NULL}, 0, 0, S_NIGHTSHURT29}, // S_NIGHTSHURT28
	{SPR_SUPZ, 12, 1, {NULL}, 0, 0, S_NIGHTSHURT30}, // S_NIGHTSHURT29
	{SPR_SUPZ, 13, 1, {NULL}, 0, 0, S_NIGHTSHURT31}, // S_NIGHTSHURT30
	{SPR_SUPZ, 14, 1, {NULL}, 0, 0, S_NIGHTSHURT32}, // S_NIGHTSHURT31
	{SPR_SUPZ, 15, 1, {NULL}, 0, 0, S_NIGHTSFLY1A},  // S_NIGHTSHURT32

	// Nights Player, Drilling
	{SPR_NDRL, 0, 2, {NULL}, 0, 0, S_NIGHTSDRILL1B},  // S_NIGHTSDRILL1A
	{SPR_NDRL, 1, 2, {NULL}, 0, 0, S_NIGHTSDRILL1C},  // S_NIGHTSDRILL1B
	{SPR_NDRL, 2, 2, {NULL}, 0, 0, S_NIGHTSDRILL1D},  // S_NIGHTSDRILL1C
	{SPR_NDRL, 3, 2, {NULL}, 0, 0, S_NIGHTSDRILL1A},  // S_NIGHTSDRILL1D
	{SPR_NDRL, 4, 2, {NULL}, 0, 0, S_NIGHTSDRILL2B},  // S_NIGHTSDRILL2A
	{SPR_NDRL, 5, 2, {NULL}, 0, 0, S_NIGHTSDRILL2C},  // S_NIGHTSDRILL2B
	{SPR_NDRL, 6, 2, {NULL}, 0, 0, S_NIGHTSDRILL2D},  // S_NIGHTSDRILL2C
	{SPR_NDRL, 7, 2, {NULL}, 0, 0, S_NIGHTSDRILL2A},  // S_NIGHTSDRILL2D
	{SPR_NDRL, 8, 2, {NULL}, 0, 0, S_NIGHTSDRILL3B},  // S_NIGHTSDRILL3A
	{SPR_NDRL, 9, 2, {NULL}, 0, 0, S_NIGHTSDRILL3C},  // S_NIGHTSDRILL3B
	{SPR_NDRL, 10, 2, {NULL}, 0, 0, S_NIGHTSDRILL3D}, // S_NIGHTSDRILL3C
	{SPR_NDRL, 11, 2, {NULL}, 0, 0, S_NIGHTSDRILL3A}, // S_NIGHTSDRILL3D
	{SPR_NDRL, 12, 2, {NULL}, 0, 0, S_NIGHTSDRILL4B}, // S_NIGHTSDRILL4A
	{SPR_NDRL, 13, 2, {NULL}, 0, 0, S_NIGHTSDRILL4C}, // S_NIGHTSDRILL4B
	{SPR_NDRL, 14, 2, {NULL}, 0, 0, S_NIGHTSDRILL4D}, // S_NIGHTSDRILL4C
	{SPR_NDRL, 15, 2, {NULL}, 0, 0, S_NIGHTSDRILL4A}, // S_NIGHTSDRILL4D
	{SPR_NDRL, 16, 2, {NULL}, 0, 0, S_NIGHTSDRILL5B}, // S_NIGHTSDRILL5A
	{SPR_NDRL, 17, 2, {NULL}, 0, 0, S_NIGHTSDRILL5C}, // S_NIGHTSDRILL5B
	{SPR_NDRL, 18, 2, {NULL}, 0, 0, S_NIGHTSDRILL5D}, // S_NIGHTSDRILL5C
	{SPR_NDRL, 19, 2, {NULL}, 0, 0, S_NIGHTSDRILL5A}, // S_NIGHTSDRILL5D
	{SPR_NDRL, 20, 2, {NULL}, 0, 0, S_NIGHTSDRILL6B}, // S_NIGHTSDRILL6A
	{SPR_NDRL, 21, 2, {NULL}, 0, 0, S_NIGHTSDRILL6C}, // S_NIGHTSDRILL6B
	{SPR_NDRL, 22, 2, {NULL}, 0, 0, S_NIGHTSDRILL6D}, // S_NIGHTSDRILL6C
	{SPR_NDRL, 23, 2, {NULL}, 0, 0, S_NIGHTSDRILL6A}, // S_NIGHTSDRILL6D
	{SPR_NDRL, 24, 2, {NULL}, 0, 0, S_NIGHTSDRILL7B}, // S_NIGHTSDRILL7A
	{SPR_NDRL, 25, 2, {NULL}, 0, 0, S_NIGHTSDRILL7C}, // S_NIGHTSDRILL7B
	{SPR_NDRL, 26, 2, {NULL}, 0, 0, S_NIGHTSDRILL7D}, // S_NIGHTSDRILL7C
	{SPR_NDRL, 27, 2, {NULL}, 0, 0, S_NIGHTSDRILL7A}, // S_NIGHTSDRILL7D
	{SPR_NDRL, 28, 2, {NULL}, 0, 0, S_NIGHTSDRILL8B}, // S_NIGHTSDRILL8A
	{SPR_NDRL, 29, 2, {NULL}, 0, 0, S_NIGHTSDRILL8C}, // S_NIGHTSDRILL8B
	{SPR_NDRL, 30, 2, {NULL}, 0, 0, S_NIGHTSDRILL8D}, // S_NIGHTSDRILL8C
	{SPR_NDRL, 31, 2, {NULL}, 0, 0, S_NIGHTSDRILL8A}, // S_NIGHTSDRILL8D
	{SPR_NDRL, 32, 2, {NULL}, 0, 0, S_NIGHTSDRILL9B}, // S_NIGHTSDRILL9A
	{SPR_NDRL, 33, 2, {NULL}, 0, 0, S_NIGHTSDRILL9C}, // S_NIGHTSDRILL9B
	{SPR_NDRL, 34, 2, {NULL}, 0, 0, S_NIGHTSDRILL9D}, // S_NIGHTSDRILL9C
	{SPR_NDRL, 35, 2, {NULL}, 0, 0, S_NIGHTSDRILL9A}, // S_NIGHTSDRILL9D

	// Nights Sparkle
	{SPR_NSPK, 32768, 140, {NULL}, 0, 0, S_NIGHTSPARKLE2}, // S_NIGHTSPARKLE1
	{SPR_NSPK, 32769, 7, {NULL}, 0, 0, S_NIGHTSPARKLE3},   // S_NIGHTSPARKLE2
	{SPR_NSPK, 32770, 7, {NULL}, 0, 0, S_NIGHTSPARKLE4},   // S_NIGHTSPARKLE3
	{SPR_NSPK, 32771, 7, {NULL}, 0, 0, S_NULL},            // S_NIGHTSPARKLE4

	// NiGHTS bumper
	{SPR_NBMP, 0, -1, {NULL}, 0, 0, S_NULL},  // S_NIGHTSBUMPER1
	{SPR_NBMP, 1, -1, {NULL}, 0, 0, S_NULL},  // S_NIGHTSBUMPER2
	{SPR_NBMP, 2, -1, {NULL}, 0, 0, S_NULL},  // S_NIGHTSBUMPER3
	{SPR_NBMP, 3, -1, {NULL}, 0, 0, S_NULL},  // S_NIGHTSBUMPER4
	{SPR_NBMP, 4, -1, {NULL}, 0, 0, S_NULL},  // S_NIGHTSBUMPER5
	{SPR_NBMP, 5, -1, {NULL}, 0, 0, S_NULL},  // S_NIGHTSBUMPER6
	{SPR_NBMP, 6, -1, {NULL}, 0, 0, S_NULL},  // S_NIGHTSBUMPER7
	{SPR_NBMP, 7, -1, {NULL}, 0, 0, S_NULL},  // S_NIGHTSBUMPER8
	{SPR_NBMP, 8, -1, {NULL}, 0, 0, S_NULL},  // S_NIGHTSBUMPER9
	{SPR_NBMP, 9, -1, {NULL}, 0, 0, S_NULL},  // S_NIGHTSBUMPER10
	{SPR_NBMP, 10, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSBUMPER11
	{SPR_NBMP, 11, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSBUMPER12

	{SPR_HOOP, 0, -1, {NULL}, 0, 0, S_NULL}, // S_HOOP

	{SPR_NSCR, 32768, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE10
	{SPR_NSCR, 32769, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE20
	{SPR_NSCR, 32770, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE30
	{SPR_NSCR, 32771, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE40
	{SPR_NSCR, 32772, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE50
	{SPR_NSCR, 32773, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE60
	{SPR_NSCR, 32774, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE70
	{SPR_NSCR, 32775, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE80
	{SPR_NSCR, 32776, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE90
	{SPR_NSCR, 32777, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE100
	{SPR_NSCR, 32778, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE10_2
	{SPR_NSCR, 32779, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE20_2
	{SPR_NSCR, 32780, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE30_2
	{SPR_NSCR, 32781, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE40_2
	{SPR_NSCR, 32782, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE50_2
	{SPR_NSCR, 32783, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE60_2
	{SPR_NSCR, 32784, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE70_2
	{SPR_NSCR, 32785, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE80_2
	{SPR_NSCR, 32786, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE90_2
	{SPR_NSCR, 32787, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSCORE100_2

	{SPR_NWNG, 0, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSWING

	// NiGHTS Paraloop Powerups
	{SPR_DISS, 0, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSPOWERUP1
	{SPR_NPRA, 0, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSPOWERUP2
	{SPR_DISS, 0, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSPOWERUP3
	{SPR_NPRB, 0, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSPOWERUP4
	{SPR_DISS, 0, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSPOWERUP5
	{SPR_NPRC, 0, -1, {NULL}, 0, 0, S_NULL}, // S_NIGHTSPOWERUP6

	{SPR_CAPS, 0, -1, {NULL}, 0, 0, S_NULL}, // S_EGGCAPSULE

	{SPR_DISS, 0, 35, {NULL}, 0, 0, S_CRUMBLE2},  // S_CRUMBLE1
	{SPR_DISS, 0, 105, {A_Scream}, 0, 0, S_DISS}, // S_CRUMBLE2

	{SPR_DISS, 0, -1, {NULL}, 0, 0, S_INVISIBLE}, // S_INVISIBLE

	// Dissipating Item
	{SPR_DISS, 0, 1, {NULL}, 0, 0, S_NULL}, // S_DISS

	{SPR_SUPT, 0,     4, {NULL}, 0, 0,  S_SUPERTRANS2}, // S_SUPERTRANS1
	{SPR_SUPT, 1,     4, {NULL}, 0, 0,  S_SUPERTRANS3}, // S_SUPERTRANS2
	{SPR_SUPT, 32770, 4, {NULL}, 0, 0,  S_SUPERTRANS4}, // S_SUPERTRANS3
	{SPR_SUPT, 3,     3, {NULL}, 0, 0,  S_SUPERTRANS5}, // S_SUPERTRANS4
	{SPR_SUPT, 4,     3, {NULL}, 0, 0,  S_SUPERTRANS6}, // S_SUPERTRANS5
	{SPR_SUPT, 5,     3, {NULL}, 0, 0,  S_SUPERTRANS7}, // S_SUPERTRANS6
	{SPR_SUPT, 6,     3, {NULL}, 0, 0,  S_SUPERTRANS8}, // S_SUPERTRANS7
	{SPR_SUPT, 7,     3, {NULL}, 0, 0,  S_SUPERTRANS9}, // S_SUPERTRANS8
	{SPR_SUPT, 8,    16, {NULL}, 0, 0, S_NIGHTSDRONE1}, // S_SUPERTRANS9

	// Spark
	{SPR_SPRK, 0, 1, {NULL}, 0, 0, S_SPRK2},  // S_SPRK1
	{SPR_SPRK, 1, 1, {NULL}, 0, 0, S_SPRK3},  // S_SPRK2
	{SPR_SPRK, 2, 1, {NULL}, 0, 0, S_SPRK4},  // S_SPRK3
	{SPR_SPRK, 3, 1, {NULL}, 0, 0, S_SPRK5},  // S_SPRK4
	{SPR_SPRK, 0, 1, {NULL}, 0, 0, S_SPRK6},  // S_SPRK5
	{SPR_SPRK, 1, 1, {NULL}, 0, 0, S_SPRK7},  // S_SPRK6
	{SPR_SPRK, 2, 1, {NULL}, 0, 0, S_SPRK8},  // S_SPRK7
	{SPR_SPRK, 3, 1, {NULL}, 0, 0, S_SPRK9},  // S_SPRK8
	{SPR_SPRK, 0, 1, {NULL}, 0, 0, S_SPRK10}, // S_SPRK9
	{SPR_SPRK, 1, 1, {NULL}, 0, 0, S_SPRK11}, // S_SPRK10
	{SPR_SPRK, 2, 1, {NULL}, 0, 0, S_SPRK12}, // S_SPRK11
	{SPR_SPRK, 3, 1, {NULL}, 0, 0, S_SPRK13}, // S_SPRK12
	{SPR_SPRK, 0, 1, {NULL}, 0, 0, S_SPRK14}, // S_SPRK13
	{SPR_SPRK, 1, 1, {NULL}, 0, 0, S_SPRK15}, // S_SPRK14
	{SPR_SPRK, 2, 1, {NULL}, 0, 0, S_SPRK16}, // S_SPRK15
	{SPR_SPRK, 3, 1, {NULL}, 0, 0, S_NULL},   // S_SPRK16

	// Robot Explosion
	{SPR_XPLD, 0, 1, {A_Scream}, 0, 0, S_XPLD2}, // S_XPLD1
	{SPR_XPLD, 1, 5, {NULL}, 0, 0, S_XPLD3},     // S_XPLD2
	{SPR_XPLD, 2, 5, {NULL}, 0, 0, S_XPLD4},     // S_XPLD3
	{SPR_XPLD, 3, 5, {NULL}, 0, 0, S_DISS},      // S_XPLD4

	// Underwater Explosion
	{SPR_WPLD, 0, 3, {A_Scream}, 0, 0, S_WPLD2}, // S_WPLD1
	{SPR_WPLD, 1, 3, {NULL},     0, 0, S_WPLD3}, // S_WPLD2
	{SPR_WPLD, 2, 3, {NULL},     0, 0, S_WPLD4}, // S_WPLD3
	{SPR_WPLD, 3, 3, {NULL},     0, 0, S_WPLD5}, // S_WPLD4
	{SPR_WPLD, 4, 3, {NULL},     0, 0, S_WPLD6}, // S_WPLD5
	{SPR_WPLD, 5, 3, {NULL},     0, 0, S_DISS},  // S_WPLD6

	// Stalagmites
	{SPR_STG0, 0, -1, {NULL}, 0, 0, S_DISS}, // S_STG0
	{SPR_STG1, 0, -1, {NULL}, 0, 0, S_DISS}, // S_STG1
	{SPR_STG2, 0, -1, {NULL}, 0, 0, S_DISS}, // S_STG2
	{SPR_STG3, 0, -1, {NULL}, 0, 0, S_DISS}, // S_STG3
	{SPR_STG4, 0, -1, {NULL}, 0, 0, S_DISS}, // S_STG4
	{SPR_STG5, 0, -1, {NULL}, 0, 0, S_DISS}, // S_STG5
	{SPR_STG6, 0, -1, {NULL}, 0, 0, S_DISS}, // S_STG6
	{SPR_STG7, 0, -1, {NULL}, 0, 0, S_DISS}, // S_STG7
	{SPR_STG8, 0, -1, {NULL}, 0, 0, S_DISS}, // S_STG8
	{SPR_STG9, 0, -1, {NULL}, 0, 0, S_DISS}, // S_STG9

	{SPR_DISS, 0, 1, {A_RockSpawn}, 0, 0, S_ROCKSPAWN}, // S_ROCKSPAWN

	{SPR_ROIA, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLEA2}, // S_ROCKCRUMBLEA1
	{SPR_ROIA, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLEA3}, // S_ROCKCRUMBLEA2
	{SPR_ROIA, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLEA4}, // S_ROCKCRUMBLEA3
	{SPR_ROIA, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLEA5}, // S_ROCKCRUMBLEA4
	{SPR_ROIA, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLEA1}, // S_ROCKCRUMBLEA5

	{SPR_ROIB, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLEB2}, // S_ROCKCRUMBLEB1
	{SPR_ROIB, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLEB3}, // S_ROCKCRUMBLEB2
	{SPR_ROIB, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLEB4}, // S_ROCKCRUMBLEB3
	{SPR_ROIB, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLEB5}, // S_ROCKCRUMBLEB4
	{SPR_ROIB, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLEB1}, // S_ROCKCRUMBLEB5

	{SPR_ROIC, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLEC2}, // S_ROCKCRUMBLEC1
	{SPR_ROIC, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLEC3}, // S_ROCKCRUMBLEC2
	{SPR_ROIC, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLEC4}, // S_ROCKCRUMBLEC3
	{SPR_ROIC, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLEC5}, // S_ROCKCRUMBLEC4
	{SPR_ROIC, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLEC1}, // S_ROCKCRUMBLEC5

	{SPR_ROID, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLED2}, // S_ROCKCRUMBLED1
	{SPR_ROID, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLED3}, // S_ROCKCRUMBLED2
	{SPR_ROID, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLED4}, // S_ROCKCRUMBLED3
	{SPR_ROID, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLED5}, // S_ROCKCRUMBLED4
	{SPR_ROID, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLED1}, // S_ROCKCRUMBLED5

	{SPR_ROIE, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLEE2}, // S_ROCKCRUMBLEE1
	{SPR_ROIE, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLEE3}, // S_ROCKCRUMBLEE2
	{SPR_ROIE, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLEE4}, // S_ROCKCRUMBLEE3
	{SPR_ROIE, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLEE5}, // S_ROCKCRUMBLEE4
	{SPR_ROIE, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLEE1}, // S_ROCKCRUMBLEE5

	{SPR_ROIF, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLEF2}, // S_ROCKCRUMBLEF1
	{SPR_ROIF, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLEF3}, // S_ROCKCRUMBLEF2
	{SPR_ROIF, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLEF4}, // S_ROCKCRUMBLEF3
	{SPR_ROIF, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLEF5}, // S_ROCKCRUMBLEF4
	{SPR_ROIF, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLEF1}, // S_ROCKCRUMBLEF5

	{SPR_ROIG, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLEG2}, // S_ROCKCRUMBLEG1
	{SPR_ROIG, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLEG3}, // S_ROCKCRUMBLEG2
	{SPR_ROIG, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLEG4}, // S_ROCKCRUMBLEG3
	{SPR_ROIG, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLEG5}, // S_ROCKCRUMBLEG4
	{SPR_ROIG, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLEG1}, // S_ROCKCRUMBLEG5

	{SPR_ROIH, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLEH2}, // S_ROCKCRUMBLEH1
	{SPR_ROIH, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLEH3}, // S_ROCKCRUMBLEH2
	{SPR_ROIH, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLEH4}, // S_ROCKCRUMBLEH3
	{SPR_ROIH, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLEH5}, // S_ROCKCRUMBLEH4
	{SPR_ROIH, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLEH1}, // S_ROCKCRUMBLEH5

	{SPR_ROII, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLEI2}, // S_ROCKCRUMBLEI1
	{SPR_ROII, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLEI3}, // S_ROCKCRUMBLEI2
	{SPR_ROII, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLEI4}, // S_ROCKCRUMBLEI3
	{SPR_ROII, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLEI5}, // S_ROCKCRUMBLEI4
	{SPR_ROII, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLEI1}, // S_ROCKCRUMBLEI5

	{SPR_ROIJ, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLEJ2}, // S_ROCKCRUMBLEJ1
	{SPR_ROIJ, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLEJ3}, // S_ROCKCRUMBLEJ2
	{SPR_ROIJ, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLEJ4}, // S_ROCKCRUMBLEJ3
	{SPR_ROIJ, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLEJ5}, // S_ROCKCRUMBLEJ4
	{SPR_ROIJ, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLEJ1}, // S_ROCKCRUMBLEJ5

	{SPR_ROIK, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLEK2}, // S_ROCKCRUMBLEK1
	{SPR_ROIK, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLEK3}, // S_ROCKCRUMBLEK2
	{SPR_ROIK, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLEK4}, // S_ROCKCRUMBLEK3
	{SPR_ROIK, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLEK5}, // S_ROCKCRUMBLEK4
	{SPR_ROIK, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLEK1}, // S_ROCKCRUMBLEK5

	{SPR_ROIL, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLEL2}, // S_ROCKCRUMBLEL1
	{SPR_ROIL, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLEL3}, // S_ROCKCRUMBLEL2
	{SPR_ROIL, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLEL4}, // S_ROCKCRUMBLEL3
	{SPR_ROIL, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLEL5}, // S_ROCKCRUMBLEL4
	{SPR_ROIL, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLEL1}, // S_ROCKCRUMBLEL5

	{SPR_ROIM, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLEM2}, // S_ROCKCRUMBLEM1
	{SPR_ROIM, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLEM3}, // S_ROCKCRUMBLEM2
	{SPR_ROIM, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLEM4}, // S_ROCKCRUMBLEM3
	{SPR_ROIM, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLEM5}, // S_ROCKCRUMBLEM4
	{SPR_ROIM, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLEM1}, // S_ROCKCRUMBLEM5

	{SPR_ROIN, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLEN2}, // S_ROCKCRUMBLEN1
	{SPR_ROIN, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLEN3}, // S_ROCKCRUMBLEN2
	{SPR_ROIN, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLEN4}, // S_ROCKCRUMBLEN3
	{SPR_ROIN, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLEN5}, // S_ROCKCRUMBLEN4
	{SPR_ROIN, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLEN1}, // S_ROCKCRUMBLEN5

	{SPR_ROIO, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLEO2}, // S_ROCKCRUMBLEO1
	{SPR_ROIO, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLEO3}, // S_ROCKCRUMBLEO2
	{SPR_ROIO, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLEO4}, // S_ROCKCRUMBLEO3
	{SPR_ROIO, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLEO5}, // S_ROCKCRUMBLEO4
	{SPR_ROIO, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLEO1}, // S_ROCKCRUMBLEO5

	{SPR_ROIP, 0, 2, {NULL}, 0, 0, S_ROCKCRUMBLEP2}, // S_ROCKCRUMBLEP1
	{SPR_ROIP, 1, 2, {NULL}, 0, 0, S_ROCKCRUMBLEP3}, // S_ROCKCRUMBLEP2
	{SPR_ROIP, 2, 2, {NULL}, 0, 0, S_ROCKCRUMBLEP4}, // S_ROCKCRUMBLEP3
	{SPR_ROIP, 3, 2, {NULL}, 0, 0, S_ROCKCRUMBLEP5}, // S_ROCKCRUMBLEP4
	{SPR_ROIP, 4, 2, {NULL}, 0, 0, S_ROCKCRUMBLEP1}, // S_ROCKCRUMBLEP5

	{SPR_SRBA, 0, 5, {A_Look}, 0, 0, S_SRB1_CRAWLA1}, // S_SRB1_CRAWLA1
	{SPR_SRBA, 0, 3, {A_Chase}, 0, 0, S_SRB1_CRAWLA3}, // S_SRB1_CRAWLA2
	{SPR_SRBA, 1, 3, {A_Chase}, 0, 0, S_SRB1_CRAWLA4}, // S_SRB1_CRAWLA3
	{SPR_SRBA, 2, 3, {A_Chase}, 0, 0, S_SRB1_CRAWLA2}, // S_SRB1_CRAWLA4

	{SPR_SRBB, 0, 2, {A_Look}, 0, 0, S_SRB1_BAT2}, // S_SRB1_BAT1
	{SPR_SRBB, 1, 2, {A_Look}, 0, 0, S_SRB1_BAT1}, // S_SRB1_BAT2
	{SPR_SRBB, 0, 2, {A_BuzzFly}, 0, 0, S_SRB1_BAT4}, // S_SRB1_BAT3
	{SPR_SRBB, 1, 2, {A_BuzzFly}, 0, 0, S_SRB1_BAT3}, // S_SRB1_BAT4

	{SPR_SRBC, 0, 2, {A_Look}, 0, 0, S_SRB1_ROBOFISH1}, // S_SRB1_ROBOFISH1
	{SPR_SRBC, 1, 2, {A_BuzzFly}, 0, 0, S_SRB1_ROBOFISH3}, // S_SRB1_ROBOFISH2
	{SPR_SRBC, 0, 2, {A_BuzzFly}, 0, 0, S_SRB1_ROBOFISH2}, // S_SRB1_ROBOFISH3

	{SPR_SRBD, 0, 2, {A_Look}, 0, 0, S_SRB1_VOLCANOGUY1}, // S_SRB1_VOLCANOGUY1
	{SPR_SRBD, 0, 2, {A_BuzzFly}, 0, 0, S_SRB1_VOLCANOGUY2}, // S_SRB1_VOLCANOGUY2

	{SPR_SRBE, 0, 2, {A_BunnyHop}, 7, 2, S_SRB1_HOPPY2}, // S_SRB1_HOPPY1
	{SPR_SRBE, 1, 2, {A_BunnyHop}, 7, -2, S_SRB1_HOPPY1}, // S_SRB1_HOPPY2

	{SPR_SRBE, 0, 2, {A_BunnyHop}, 4, 2, S_SRB1_HOPPYWATER2}, // S_SRB1_HOPPYWATER1
	{SPR_SRBE, 1, 2, {A_BunnyHop}, 4, -2, S_SRB1_HOPPYWATER1}, // S_SRB1_HOPPYWATER2

	{SPR_SRBF, 0, 2, {A_BunnyHop}, 7, 0, S_SRB1_HOPPYSKYLAB1}, // S_SRB1_HOPPYSKYLAB1

	{SPR_PXVI, 0, -1, {NULL}, 0, 0, S_PXVI}, // S_PXVI

	{SPR_SRBG, 0, 16, {A_MoveAbsolute}, 0, 5, S_SRB1_MMZFLYING2}, // S_SRB1_MMZFLYING1
	{SPR_SRBG, 0, 16, {A_MoveAbsolute}, 180, 5, S_SRB1_MMZFLYING3}, // S_SRB1_MMZFLYING2
	{SPR_SRBG, 0, 1, {A_MoveAbsolute}, 0, 5, S_SRB1_MMZFLYING4}, // S_SRB1_MMZFLYING3
	{SPR_SRBG, 0, 7, {A_MoveAbsolute}, 0, 5, S_SRB1_MMZFLYING5}, // S_SRB1_MMZFLYING4
	{SPR_SRBG, 0, 8, {A_MoveAbsolute}, 180, 5, S_SRB1_MMZFLYING1}, // S_SRB1_MMZFLYING5

	{SPR_SRBH, 0, 16, {A_MoveAbsolute}, 180, 5, S_SRB1_UFO2}, // S_SRB1_UFO1
	{SPR_SRBH, 0, 16, {A_MoveAbsolute}, 0, 5, S_SRB1_UFO1}, // S_SRB1_UFO2

	{SPR_SRBI, 0, 4, {A_MoveAbsolute}, 0, 7, S_SRB1_GRAYBOT2}, // S_SRB1_GRAYBOT1
	{SPR_SRBI, 1, 4, {A_MoveAbsolute}, 0, 7, S_SRB1_GRAYBOT3}, // S_SRB1_GRAYBOT2
	{SPR_SRBI, 0, 4, {A_MoveAbsolute}, 0, 7, S_SRB1_GRAYBOT4}, // S_SRB1_GRAYBOT3
	{SPR_SRBI, 1, 4, {A_MoveAbsolute}, 180, 7, S_SRB1_GRAYBOT5}, // S_SRB1_GRAYBOT4
	{SPR_SRBI, 0, 4, {A_MoveAbsolute}, 180, 7, S_SRB1_GRAYBOT6}, // S_SRB1_GRAYBOT5
	{SPR_SRBI, 1, 4, {A_MoveAbsolute}, 180, 7, S_SRB1_GRAYBOT1}, // S_SRB1_GRAYBOT6

	{SPR_SRBJ, 0, 8, {A_MoveAbsolute}, 0, 5, S_SRB1_ROBOTOPOLIS2}, // S_SRB1_ROBOTOPOLIS1
	{SPR_SRBJ, 1, 8, {A_MoveAbsolute}, 0, -5, S_SRB1_ROBOTOPOLIS1}, // S_SRB1_ROBOTOPOLIS2

	{SPR_SRBK, 0, 8, {A_MoveAbsolute}, 0, 5, S_SRB1_RBZBUZZ2}, // S_SRB1_RBZBUZZ1
	{SPR_SRBK, 1, 8, {A_MoveAbsolute}, 0, -5, S_SRB1_RBZBUZZ1}, // S_SRB1_RBZBUZZ2

	{SPR_SRBL, 0, 35, {A_ZThrust}, 4, FRACUNIT+1, S_SRB1_RBZSPIKES2}, // S_SRB1_RBZSPIKES1
	{SPR_SRBL, 0, 35, {A_ZThrust}, -4, FRACUNIT+1, S_SRB1_RBZSPIKES1}, // S_SRB1_RBZSPIKES2

	{SPR_SRBM, 0, 4, {NULL}, 0, 0, S_SRB1_METALSONIC2}, // S_SRB1_METALSONIC1
	{SPR_SRBM, 1, 4, {NULL}, 0, 0, S_SRB1_METALSONIC3}, // S_SRB1_METALSONIC2
	{SPR_SRBM, 2, 4, {NULL}, 0, 0, S_SRB1_METALSONIC1}, // S_SRB1_METALSONIC3

	{SPR_SRBN, 0, 2, {A_MoveAbsolute}, 0, 7, S_SRB1_GOLDBOT2}, // S_SRB1_GOLDBOT1
	{SPR_SRBN, 1, 2, {A_MoveAbsolute}, 0, 7, S_SRB1_GOLDBOT3}, // S_SRB1_GOLDBOT2
	{SPR_SRBN, 0, 2, {A_MoveAbsolute}, 0, 7, S_SRB1_GOLDBOT4}, // S_SRB1_GOLDBOT3
	{SPR_SRBN, 1, 2, {A_MoveAbsolute}, 180, 7, S_SRB1_GOLDBOT5}, // S_SRB1_GOLDBOT4
	{SPR_SRBN, 0, 2, {A_MoveAbsolute}, 180, 7, S_SRB1_GOLDBOT6}, // S_SRB1_GOLDBOT5
	{SPR_SRBN, 1, 2, {A_MoveAbsolute}, 180, 7, S_SRB1_GOLDBOT1}, // S_SRB1_GOLDBOT6

	{SPR_SRBO, 0, 2, {A_Look}, 0, 0, S_SRB1_GENREX1}, // S_SRB1_GENREX1
	{SPR_SRBO, 0, 2, {A_BuzzFly}, 0, 0, S_SRB1_GENREX2}, // S_SRB1_GENREX2
#ifdef SEENAMES
	{SPR_DISS, 0, 1, {NULL}, 0, 0, S_DISS}, // S_NAMECHECK
#endif
};

mobjinfo_t mobjinfo[NUMMOBJTYPES] =
{
	{           // MT_PLAYER
		-1,             // doomednum
		S_PLAY_STND,    // spawnstate
		1,              // spawnhealth
		S_PLAY_RUN1,    // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_thok,       // attacksound
		S_PLAY_PAIN,    // painstate
		MT_THOK,        // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_PLAY_ATK1,    // missilestate
		S_PLAY_DIE1,    // deathstate
		S_PLAY_DIE1,    // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		16*FRACUNIT,    // radius
		56*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		MT_THOK,        // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE, // flags
		MT_THOK         // raisestate
	},

	{           // MT_BLUECRAWLA
		100,            // doomednum
		S_POSS_STND,    // spawnstate
		1,              // spawnhealth
		S_POSS_RUN1,    // seestate
		0,              // seesound
		32,             // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		200,            // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		3,              // speed
		24*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE, // flags
		S_NULL          // raisestate
	},

	{           // MT_REDCRAWLA
		101,            // doomednum
		S_SPOS_STND,    // spawnstate
		1,              // spawnhealth
		S_SPOS_RUN1,    // seestate
		0,              // seesound
		32,             // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		170,            // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		3,              // speed
		24*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE, // flags
		S_NULL          // raisestate
	},

	{           // MT_GFZFISH
		102,            // doomednum
		S_FISH2,        // spawnstate
		1,              // spawnhealth
		S_FISH1,        // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_FISH3,        // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_FISH4,        // xdeathstate
		sfx_pop,        // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		28*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE, // flags
		S_NULL          // raisestate
	},

	{           // MT_GOLDBUZZ
		103,            // doomednum
		S_BUZZLOOK1,    // spawnstate
		1,              // spawnhealth
		S_BUZZFLY1,     // seestate
		0,              // seesound
		2,              // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		3072,           // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		4*FRACUNIT,     // speed
		20*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_SLIDEME|MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_REDBUZZ
		104,            // doomednum
		S_RBUZZLOOK1,   // spawnstate
		1,              // spawnhealth
		S_RBUZZFLY1,    // seestate
		0,              // seesound
		2,              // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		3072,           // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		8*FRACUNIT,     // speed
		20*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_SLIDEME|MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_JETTBOMBER
		105,            // doomednum
		S_JETBLOOK1,    // spawnstate
		1,              // spawnhealth
		S_JETBZOOM1,    // seestate
		0,              // seesound
		32,             // reactiontime
		sfx_s3k_32,     // attacksound
		S_NULL,         // painstate
		200,            // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		1*FRACUNIT,     // speed
		20*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_SLIDEME|MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE|MF_NOGRAVITY, // flags
		MT_MINE         // raisestate
	},

	{           // MT_JETTGUNNER
		106,            // doomednum
		S_JETGLOOK1,    // spawnstate
		1,              // spawnhealth
		S_JETGZOOM1,    // seestate
		0,              // seesound
		5,              // reactiontime
		sfx_s3k_28,     // attacksound
		S_NULL,         // painstate
		3072,           // painchance
		0,              // painsound
		0,              // meleestate
		S_JETGSHOOT1,   // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		1*FRACUNIT,     // speed
		20*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_SLIDEME|MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE|MF_NOGRAVITY, // flags
		MT_JETTBULLET   // raisestate
	},

	{           // MT_CRAWLACOMMANDER
		107,            // doomednum
		S_CCOMMAND1,    // spawnstate
		2,              // spawnhealth
		S_CCOMMAND3,    // seestate
		0,              // seesound
		2*TICRATE,      // reactiontime
		0,              // attacksound
		S_CCOMMAND1,    // painstate
		200,            // painchance
		sfx_dmpain,     // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		3,              // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_SLIDEME|MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE, // flags
		S_NULL          // raisestate
	},

	{           // MT_DETON
		108,            // doomednum
		S_DETON1,       // spawnstate
		1,              // spawnhealth
		S_DETON2,       // seestate
		0,              // seesound
		1,              // reactiontime
		sfx_deton,      // attacksound
		S_NULL,         // painstate
		3072,           // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_DETON16,      // xdeathstate
		sfx_pop,        // deathsound
		1*FRACUNIT,     // speed
		20*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		0,              // activesound
		MF_ENEMY|MF_SHOOTABLE|MF_NOGRAVITY|MF_MISSILE, // flags
		ANG15           // raisestate: largest angle to turn in one tic (here, 15 degrees)
	},

	{           // MT_SKIM
		109,            // doomednum
		S_SKIM1,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_s3k_32,     // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_SKIM3,        // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		8,              // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_ENEMY|MF_SPECIAL|MF_NOGRAVITY|MF_SHOOTABLE, // flags
		MT_MINE         // raisestate
	},

	{           // MT_TURRET
		110,            // doomednum
		S_TURRET,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_trfire,     // attacksound
		0,              // painstate
		0,              // painchance
		sfx_fizzle,     // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_TURRETSHOCK1, // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		MT_TURRETLASER, // mass
		1,              // damage
		sfx_trpowr,     // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_POPUPTURRET
		111,            // doomednum
		S_TURRETLOOK,   // spawnstate
		1,              // spawnhealth
		S_TURRETPOPUP1, // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_trfire,     // attacksound
		S_NULL,         // painstate
		1024,           // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		0,              // speed
		12*FRACUNIT,    // radius
		64*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE, // flags
		MT_JETTBULLET   // raisestate
	},

	{           // MT_SHARP
		112,            // doomednum
		S_SHARP_ROAM1,  // spawnstate
		1,              // spawnhealth
		S_SHARP_ROAM2,  // seestate
		0,              // seesound
		3*TICRATE,      // reactiontime
		sfx_s3k_93,     // attacksound
		S_NULL,         // painstate
		5*TICRATE,      // painchance
		0,              // painsound
		0,              // meleestate
		S_SHARP_AIM1,   // missilestate
		S_XPLD1,        // deathstate
		S_SHARP_SPIN1,  // xdeathstate
		sfx_pop,        // deathsound
		2,              // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE|MF_BOUNCE, // flags
		S_SHARP_SPIN8   // raisestate
	},

	{           // MT_JETJAW
		113,            // doomednum
		S_JETJAW_ROAM1, // spawnstate
		1,              // spawnhealth
		S_JETJAW_CHOMP1,// seestate
		0,              // seesound
		4*TICRATE,      // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		0,              // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		8,              // speed
		12*FRACUNIT,    // radius
		20*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_SLIDEME|MF_FLOAT|MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_SNAILER
		114,            // doomednum
		S_SNAILER1,     // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		0,              // seesound
		2,              // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		0,              // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		FRACUNIT,       // speed
		24*FRACUNIT,    // radius
		48*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_SLIDEME|MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_VULTURE
		115,            // doomednum
		S_VULTURE_STND, // spawnstate
		1,              // spawnhealth
		S_VULTURE_VTOL1,// seestate
		0,              // seesound
		2,              // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		0,              // painchance
		0,              // painsound
		0,              // meleestate
		S_VULTURE_ZOOM1,// missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		3,              // speed
		12*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		TICRATE,        // mass
		0,              // damage
		sfx_jet,        // activesound
		MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_POINTY
		116,            // doomednum
		S_POINTY1,      // spawnstate
		1,              // spawnhealth
		S_POINTY1,      // seestate
		0,              // seesound
		6,              // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		4,              // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		5*FRACUNIT,     // speed
		4*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		MT_POINTYBALL,  // mass
		128,            // damage
		0,              // activesound
		MF_SLIDEME|MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_POINTYBALL
		-1,             // doomednum
		S_POINTYBALL1,  // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		4*FRACUNIT,     // speed
		4*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_ROBOHOOD
		117,              // doomednum
		S_ROBOHOOD_LOOK,  // spawnstate
		1,                // spawnhealth
		S_ROBOHOOD_STND,  // seestate
		0,                // seesound
		35,               // reactiontime
		0,                // attacksound
		S_ROBOHOOD_JUMP,  // painstate
		0,                // painchance
		0,                // painsound
		0,                // meleestate
		S_ROBOHOOD_SHOOT, // missilestate
		S_XPLD1,          // deathstate
		S_ROBOHOOD_JUMP2, // xdeathstate
		sfx_pop,          // deathsound
		0,                // speed
		24*FRACUNIT,      // radius
		32*FRACUNIT,      // height
		0,                // display offset
		100,              // mass
		0,                // damage
		0,                // activesound
		MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE, // flags
		S_ROBOHOOD_FALL   // raisestate
	},

	{           // MT_FACESTABBER
		118,            // doomednum
		S_FACESTABBER_STND1, // spawnstate
		1,              // spawnhealth
		S_FACESTABBER_STND1, // seestate
		0,              // seesound
		35,             // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		0,              // painchance
		0,              // painsound
		S_FACESTABBER_CHARGE1, // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		3,              // speed
		32*FRACUNIT,    // radius
		64*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE, // flags
		S_NULL          // raisestate
	},

	{           // MT_EGGGUARD
		119,             // doomednum
		S_EGGGUARD_STND, // spawnstate
		1,               // spawnhealth
		S_EGGGUARD_WALK1,// seestate
		0,               // seesound
		35,              // reactiontime
		0,               // attacksound
		S_EGGGUARD_MAD1, // painstate
		0,               // painchance
		0,               // painsound
		S_EGGGUARD_RUN1, // meleestate
		S_NULL,          // missilestate
		S_XPLD1,         // deathstate
		S_NULL,          // xdeathstate
		sfx_pop,         // deathsound
		6,               // speed
		16*FRACUNIT,     // radius
		48*FRACUNIT,     // height
		0,               // display offset
		100,             // mass
		0,               // damage
		0,               // activesound
		0,               // flags
		S_NULL           // raisestate
	},

	{           // MT_EGGSHIELD
		-1,              // doomednum
		S_EGGSHIELD,     // spawnstate
		1,               // spawnhealth
		S_EGGSHIELD,     // seestate
		0,               // seesound
		35,              // reactiontime
		0,               // attacksound
		S_NULL,          // painstate
		0,               // painchance
		0,               // painsound
		S_NULL,          // meleestate
		S_NULL,          // missilestate
		S_XPLD1,         // deathstate
		S_NULL,          // xdeathstate
		sfx_pop,         // deathsound
		3,               // speed
		24*FRACUNIT,     // radius
		128*FRACUNIT,    // height
		0,               // display offset
		100,             // mass
		0,               // damage
		0,               // activesound
		MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL           // raisestate
	},

	{           // MT_GSNAPPER
		120,            // doomednum
		S_GSNAPPER_STND,// spawnstate
		1,              // spawnhealth
		S_GSNAPPER1,    // seestate
		0,              // seesound
		32,             // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		200,            // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		3,              // speed
		24*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE, // flags
		S_NULL          // raisestate
	},

	{           // MT_MINUS
		121,            // doomednum
		S_MINUS_STND,   // spawnstate
		1,              // spawnhealth
		S_MINUS_DIGGING,// seestate
		0,              // seesound
		32,             // reactiontime
		sfx_s3k_78,     // attacksound
		S_NULL,         // painstate
		200,            // painchance
		0,              // painsound
		S_MINUS_DOWNWARD1,// meleestate
		S_MINUS_POPUP,  // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		12,             // speed
		24*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_mindig,     // activesound
		MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE, // flags
		S_MINUS_UPWARD1  // raisestate
	},

	{           // MT_BOSSEXPLODE
		-1,             // doomednum
		S_BPLD1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_BOSSFLYPOINT
		290,            // doomednum
		S_NULL,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		2*FRACUNIT,     // radius
		4*FRACUNIT,     // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_EGGTRAP
		291,            // doomednum
		S_NULL,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR, // flags
		S_NULL          // raisestate
	},

	{           // MT_BOSS3WAYPOINT
		292,            // doomednum
		S_NULL,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		2*FRACUNIT,     // radius
		4*FRACUNIT,     // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},


	{           // MT_EGGMOBILE
		200,               // doomednum
		S_EGGMOBILE_STND,  // spawnstate
		8,                 // spawnhealth
		S_EGGMOBILE_STND,  // seestate
		0,                 // seesound
		8,                 // reactiontime
		0,                 // attacksound
		S_EGGMOBILE_PAIN,  // painstate
		MT_THOK,           // painchance
		sfx_dmpain,        // painsound
		S_EGGMOBILE_LATK1, // meleestate
		S_EGGMOBILE_RATK1, // missilestate
		S_EGGMOBILE_DIE1,  // deathstate
		S_EGGMOBILE_FLEE1, // xdeathstate
		sfx_cybdth,        // deathsound
		4,                 // speed
		24*FRACUNIT,       // radius
		52*FRACUNIT,       // height
		0,                 // display offset
		0,                 // mass
		2,                 // damage
		sfx_telept,        // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_FLOAT|MF_NOGRAVITY|MF_BOSS, // flags
		S_EGGMOBILE_PANIC1 // raisestate
	},

	{           // MT_JETFUME1
		-1,             // doomednum
		S_JETFUME1,     // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		4*FRACUNIT,     // radius
		4*FRACUNIT,     // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOGRAVITY|MF_NOCLIP|MF_NOCLIPHEIGHT, // flags
		S_NULL          // raisestate
	},

	{           // MT_EGGMOBILE2
		201,               // doomednum
		S_EGGMOBILE2_STND, // spawnstate
		8,                 // spawnhealth
		S_EGGMOBILE2_STND, // seestate
		0,                 // seesound
		8,                 // reactiontime
		sfx_gspray,        // attacksound
		S_EGGMOBILE2_PAIN, // painstate
		MT_GOOP,           // painchance
		sfx_dmpain,        // painsound
		S_EGGMOBILE2_STND, // meleestate
		S_EGGMOBILE2_STND, // missilestate
		S_EGGMOBILE2_DIE1, // deathstate
		S_EGGMOBILE2_FLEE1,// xdeathstate
		sfx_cybdth,        // deathsound
		2*FRACUNIT,        // speed
		24*FRACUNIT,       // radius
		48*FRACUNIT,       // height
		0,                 // display offset
		0,                 // mass
		2,                 // damage
		sfx_pogo,          // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_NOGRAVITY|MF_BOSS, // flags
		S_EGGMOBILE2_POGO1 // raisestate
	},

	{           // MT_BOSSTANK1
		-1,             // doomednum
		S_BOSSTANK1,    // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		64*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_BOSSTANK2
		-1,             // doomednum
		S_BOSSTANK2,    // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		64*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_BOSSSPIGOT
		-1,             // doomednum
		S_BOSSSPIGOT,   // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_GOOP
		-1,             // doomednum
		S_GOOP1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_ghit,       // painsound
		S_GOOP3,        // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		4*FRACUNIT,     // radius
		4*FRACUNIT,     // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL,     // flags
		S_NULL          // raisestate
	},

	{           // MT_EGGMOBILE3
		202,                // doomednum
		S_EGGMOBILE3_STND,  // spawnstate
		8,                  // spawnhealth
		S_EGGMOBILE3_STND,  // seestate
		0,                  // seesound
		0,                  // reactiontime
		0,                  // attacksound
		S_EGGMOBILE3_PAIN,  // painstate
		MT_PROPELLER,       // painchance
		sfx_dmpain,         // painsound
		S_EGGMOBILE3_STND,  // meleestate
		S_EGGMOBILE3_ATK1,  // missilestate
		S_EGGMOBILE3_DIE1,  // deathstate
		S_EGGMOBILE3_FLEE1, // xdeathstate
		sfx_cybdth,         // deathsound
		8*FRACUNIT,         // speed
		32*FRACUNIT,        // radius
		80*FRACUNIT,        // height
		0,                  // display offset
		0,                  // mass
		2,                  // damage
		sfx_telept,         // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_NOGRAVITY|MF_BOSS, // flags
		S_EGGMOBILE3_LAUGH20 // raisestate
	},

	{           // MT_PROPELLER
		-1,             // doomednum
		S_PROPELLER1,   // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		4*FRACUNIT,     // radius
		4*FRACUNIT,     // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOGRAVITY|MF_NOCLIP|MF_NOCLIPHEIGHT, // flags
		S_NULL          // raisestate
	},

	{           // MT_EGGMOBILE4
		203,               // doomednum
		S_EGGMOBILE4_STND, // spawnstate
		8,                 // spawnhealth
		S_EGGMOBILE4_STND, // seestate
		0,                 // seesound
		8,                 // reactiontime
		0,                 // attacksound
		S_EGGMOBILE4_STND, // painstate
		MT_THOK,           // painchance
		sfx_dmpain,        // painsound
		S_EGGMOBILE4_STND, // meleestate
		S_EGGMOBILE4_STND, // missilestate
		S_EGGMOBILE4_STND, // deathstate
		S_EGGMOBILE4_STND, // xdeathstate
		sfx_cybdth,        // deathsound
		4,                 // speed
		24*FRACUNIT,       // radius
		52*FRACUNIT,       // height
		0,                 // display offset
		0,                 // mass
		2,                 // damage
		sfx_telept,        // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_NOGRAVITY|MF_BOSS, // flags
		S_EGGMOBILE4_STND // raisestate
	},

	{           // MT_BLACKEGGMAN
		206,               // doomednum
		S_BLACKEGG_STND,   // spawnstate
		8,                 // spawnhealth
		S_BLACKEGG_WALK1,  // seestate
		0,                 // seesound
		8*TICRATE,         // reactiontime
		0,                 // attacksound
		S_BLACKEGG_PAIN1,  // painstate
		0,                 // painchance
		sfx_None,          // painsound
		S_BLACKEGG_HITFACE1, // meleestate
		S_BLACKEGG_MISSILE1, // missilestate
		S_BLACKEGG_DIE1,   // deathstate
		S_BLACKEGG_GOOP,   // xdeathstate
		sfx_None,          // deathsound
		1,                 // speed
		48*FRACUNIT,       // radius
		160*FRACUNIT,      // height
		0,                 // display offset
		0,                 // mass
		1,                 // damage
		sfx_None,          // activesound
		MF_SPECIAL|MF_BOSS,// flags
		S_BLACKEGG_JUMP1   // raisestate
	},

	{           // MT_BLACKEGGMAN_HELPER
		-1,                // doomednum
		S_BLACKEGG_HELPER, // spawnstate
		8,                 // spawnhealth
		S_DISS,            // seestate
		0,                 // seesound
		0,                 // reactiontime
		0,                 // attacksound
		S_DISS,            // painstate
		0,                 // painchance
		sfx_None,          // painsound
		S_DISS,            // meleestate
		S_DISS,            // missilestate
		S_DISS,            // deathstate
		S_DISS,            // xdeathstate
		sfx_None,          // deathsound
		1,                 // speed
		48*FRACUNIT,       // radius
		32*FRACUNIT,       // height
		0,                 // display offset
		0,                 // mass
		1,                 // damage
		sfx_None,          // activesound
		MF_SOLID|MF_PUSHABLE|MF_NOGRAVITY,          // flags
		S_DISS             // raisestate
	},

	{           // MT_BLACKEGGMAN_GOOPFIRE
		-1,             // doomednum
		S_BLACKEGG_GOOP1, // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_BLACKEGG_GOOP3, // deathstate
		S_NULL,         // xdeathstate
		sfx_ghit,       // deathsound
		30*FRACUNIT,    // speed
		11*FRACUNIT,    // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_BLACKEGGMAN_MISSILE
		-1,             // doomednum
		S_BLACKEGG_MISSILE, // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_BPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_bexpld,     // deathsound
		10*FRACUNIT,    // speed
		24*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_RING
		300,            // doomednum
		S_RING1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		MT_FLINGRING,   // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_FLINGRING
		-1,             // doomednum
		S_RING1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		MT_FLINGRING,   // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		MT_RING,        // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL, // flags
		S_NULL          // raisestate
	},

	{           // MT_BLUEBALL
		-1,             // doomednum
		S_BLUEBALL,    // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		MT_FLINGBALL,   // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_BLUEBALLSPARK
		-1,             // doomednum
		S_BLUEBALLSPARK,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		2,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		2*FRACUNIT,     // radius
		4*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_FLINGBALL
		-1,             // doomednum
		S_BLUEBALL,     // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		MT_FLINGBALL,   // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		MT_BLUEBALL,    // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL, // flags
		S_NULL          // raisestate
	},

	{           // MT_REDTEAMRING
		308,            // doomednum
		S_TEAMRING1,    // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		MT_FLINGRING,   // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_BLUETEAMRING
		309,            // doomednum
		S_TEAMRING1,    // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		MT_FLINGRING,   // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_EMMY
		312,            // doomednum
		S_EMMY1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_TOKEN
		-1,             // doomednum
		S_TOKEN,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY|MF_SPECIAL, // flags
		S_NULL          // raisestate
	},

	{           // MT_REDFLAG
		310,            // doomednum
		S_REDFLAG,      // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		24*FRACUNIT,    // radius
		64*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL,     // flags
		S_NULL          // raisestate
	},

	{           // MT_BLUEFLAG
		311,            // doomednum
		S_BLUEFLAG,     // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		24*FRACUNIT,    // radius
		64*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL,     // flags
		S_NULL          // raisestate
	},

	{           // MT_EMBLEM
		-1,             // doomednum
		S_EMBLEM1,      // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_EMERALD1
		313,            // doomednum
		S_CEMG1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_cgot,       // deathsound
		EMERALD1,       // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY|MF_SPECIAL, // flags
		S_NULL          // raisestate
	},
	{           // MT_EMERALD2
		314,            // doomednum
		S_CEMG2,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_cgot,       // deathsound
		EMERALD2,       // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY|MF_SPECIAL, // flags
		S_NULL          // raisestate
	},
	{           // MT_EMERALD3
		315,            // doomednum
		S_CEMG3,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_cgot,       // deathsound
		EMERALD3,       // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY|MF_SPECIAL, // flags
		S_NULL          // raisestate
	},
	{           // MT_EMERALD4
		316,            // doomednum
		S_CEMG4,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_cgot,       // deathsound
		EMERALD4,       // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY|MF_SPECIAL, // flags
		S_NULL          // raisestate
	},
	{           // MT_EMERALD5
		317,            // doomednum
		S_CEMG5,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_cgot,       // deathsound
		EMERALD5,       // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY|MF_SPECIAL, // flags
		S_NULL          // raisestate
	},
	{           // MT_EMERALD6
		318,            // doomednum
		S_CEMG6,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_cgot,       // deathsound
		EMERALD6,       // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY|MF_SPECIAL, // flags
		S_NULL          // raisestate
	},
	{           // MT_EMERALD7
		319,            // doomednum
		S_CEMG7,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_cgot,       // deathsound
		EMERALD7,       // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY|MF_SPECIAL, // flags
		S_NULL          // raisestate
	},

	{           // MT_EMERHUNT
		320,            // doomednum
		S_EMER1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_cgot,       // deathsound
		8,              // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_EMERALDSPAWN
		323,            // doomednum
		S_INVISIBLE,    // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8,              // radius
		8,              // height
		0,              // display offset
		10,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOGRAVITY|MF_NOSECTOR,  // flags
		S_NULL          // raisestate
	},

	{           // MT_FLINGEMERALD
		-1,             // doomednum
		S_CEMG1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_cgot,       // deathsound
		60*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		48*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL, // flags
		S_NULL          // raisestate
	},

	{           // MT_FAN
		540,            // doomednum
		S_FAN,          // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		5*FRACUNIT,     // speed
		16*FRACUNIT,    // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SOLID,       // flags
		S_NULL          // raisestate
	},

	{           // MT_BUBBLES
		500,            // doomednum
		S_BUBBLES1,     // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_SIGN
		501,            // doomednum
		S_SIGN52,       // spawnstate
		1000,           // spawnhealth
		S_PLAY_SIGN,    // seestate
		sfx_lvpass,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		8*FRACUNIT,     // radius
		32*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_STEAM
		541,            // doomednum
		S_STEAM1,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_steam2,     // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_steam1,     // deathsound
		20*FRACUNIT,    // speed
		32*FRACUNIT,    // radius
		1*FRACUNIT,     // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SOLID,       // flags
		S_NULL          // raisestate
	},

	{           // MT_SPIKEBALL
		-1,            // doomednum
		S_SPIKEBALL1,   // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		10*FRACUNIT,    // speed
		4*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_SPECIALSPIKEBALL
		521,            // doomednum
		S_SPIKEBALL1,   // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		10*FRACUNIT,    // speed
		12*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_SPINFIRE
		-1,             // doomednum
		S_SPINFIRE1,    // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		14*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY|MF_FIRE, // flags
		S_NULL          // raisestate
	},

	{           // MT_CEILINGSPIKE
		522,             // doomednum
		S_CEILINGSPIKE, // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		8*FRACUNIT,     // radius
		42*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SPAWNCEILING, // flags
		S_NULL          // raisestate
	},

	{           // MT_FLOORSPIKE
		523,            // doomednum
		S_FLOORSPIKE,   // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		8*FRACUNIT,     // radius
		42*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOGRAVITY|MF_SCENERY|MF_NOCLIPHEIGHT,  // flags
		S_NULL          // raisestate
	},

	{           // MT_STARPOST
		502,            // doomednum
		S_STARPOST1,    // spawnstate
		1,              // spawnhealth
		S_STARPOST2,    // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_STARPOST4,    // painstate
		0,              // painchance
		sfx_strpst,     // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		64*FRACUNIT,    // radius
		80*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SPECIALFLAGS, // flags
		S_NULL          // raisestate
	},

	{           // MT_BIGMINE
		524,            // doomednum
		S_BIGMINE1,     // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		TICRATE,        // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		2*FRACUNIT,     // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		MT_UWEXPLODE,   // mass
		64,             // damage
		sfx_gbeep,      // activesound
		MF_SPECIAL|MF_NOGRAVITY,     // flags
		S_NULL          // raisestate
	},

	{           // MT_BIGAIRMINE
		527,            // doomednum
		S_BIGMINE1,     // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		TICRATE,        // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_bexpld,     // deathsound
		2*FRACUNIT,     // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		MT_BOSSEXPLODE, // mass
		64,             // damage
		sfx_gbeep,      // activesound
		MF_SPECIAL|MF_NOGRAVITY,     // flags
		S_NULL          // raisestate
	},

	{           // MT_CANNONLAUNCHER
		525,            // doomednum
		S_CANNONLAUNCHER1, // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		2*TICRATE,      // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		TICRATE,        // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		2*FRACUNIT,     // radius
		4*FRACUNIT,     // height
		0,              // display offset
		0,              // mass
		1,              // damage
		sfx_pop,        // activesound
		MF_NOGRAVITY|MF_NOSECTOR|MF_NOBLOCKMAP, // flags
		S_NULL          // raisestate
	},

	{           // MT_SUPERRINGBOX
		400,            // doomednum
		S_SUPERRINGBOX, // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_SUPERRINGBOX, // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_SUPERRINGBOX2,// deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		MT_RINGICO,     // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR, // flags
		S_NULL          // raisestate
	},

		{           // MT_REDRINGBOX
		414,            // doomednum
		S_REDRINGBOX,   // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_REDRINGBOX,   // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_SUPERRINGBOX2,// deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		MT_RINGICO,     // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR, // flags
		S_NULL          // raisestate
	},

	{           // MT_BLUERINGBOX
		415,            // doomednum
		S_BLUERINGBOX,  // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_BLUERINGBOX,  // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_SUPERRINGBOX2,// deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		MT_RINGICO,     // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR, // flags
		S_NULL          // raisestate
	},

	{           // MT_SNEAKERTV
		407,            // doomednum
		S_SHTV,         // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_SHTV,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_SHTV2,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		MT_SHOESICO,    // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR, // flags
		S_NULL          // raisestate
	},

	{           // MT_INV
		408,            // doomednum
		S_PINV,         // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_PINV,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_PINV3,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		MT_INVCICO,     // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR, // flags
		S_NULL          // raisestate
	},

	// 1-up box
	{           // MT_PRUP
		409,            // doomednum
		S_PLAY_BOX1A,   // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_PLAY_BOX1A,   // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_PRUP1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		MT_1UPICO,      // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR|MF_RUNSPAWNFUNC, // flags
		S_NULL          // raisestate
	},

	{           // MT_YELLOWTV
		402,            // doomednum
		S_YLTV,         // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_YLTV,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_YLTV2,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		MT_YSHIELDICO,  // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR, // flags
		S_NULL          // raisestate
	},

	{           // MT_BLUETV
		403,            // doomednum
		S_BLTV1,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_BLTV1,        // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_BLTV3,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		MT_BSHIELDICO,  // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR, // flags
		S_NULL          // raisestate
	},

	// bomb shield box
	{           // MT_BLACKTV
		404,            // doomednum
		S_BKTV1,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_BKTV1,        // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_BKTV3,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		MT_KSHIELDICO,  // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR, // flags
		S_NULL          // raisestate
	},

	// jump shield box
	{           // MT_WHITETV
		405,            // doomednum
		S_WHTV1,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_WHTV1,        // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_WHTV3,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		MT_WSHIELDICO,  // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR, // flags
		S_NULL          // raisestate
	},

	{           // MT_GREENTV
		406,            // doomednum
		S_GRTV,         // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_GRTV,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_GRTV2,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		MT_GSHIELDICO,  // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR, // flags
		S_NULL          // raisestate
	},

	{           // MT_EGGMANBOX
		410,            // doomednum
		S_EGGTV1,       // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_EGGTV1,       // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_EGGTV3,       // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		MT_EGGMANICO,   // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR, // flags
		S_NULL          // raisestate
	},

	{           // MT_MIXUPBOX
		411,            // doomednum
		S_MIXUPBOX1,    // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_MIXUPBOX1,    // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_MIXUPBOX3,    // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		MT_MIXUPICO,    // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR, // flags
		S_NULL          // raisestate
	},

	{           // MT_RECYCLETV
		416,            // doomednum
		S_RECYCLETV1,   // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_RECYCLETV1,   // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_RECYCLETV3,   // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		MT_RECYCLEICO,  // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR, // flags
		S_NULL          // raisestate
	},

	{           // MT_RECYCLEICO
		-1,             // doomednum
		S_RECYCLETV3,   // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		2*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		14*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		62*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY|MF_NOGRAVITY|MF_BOXICON, // flags
		S_NULL          // raisestate
	},

	{           // MT_QUESTIONBOX
		412,            // doomednum
		S_RANDOMBOX1,   // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_RANDOMBOX1,   // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_RANDOMBOX3,   // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR, // flags
		S_NULL          // raisestate
	},

	{           // MT_GRAVITYBOX
		413,            // doomednum
		S_GBTV1,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_GBTV1,        // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_GBTV3,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		MT_MONITOREXPLOSION, // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		MT_GRAVITYICO,  // damage
		sfx_cgot,       // activesound
		MF_SOLID|MF_SHOOTABLE|MF_MONITOR, // flags
		S_NULL          // raisestate
	},

	{           // MT_MONITOREXPLOSION
		-1,             // doomednum
		S_MONITOREXPLOSION1, // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_MONITOREXPLOSION1, // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP,      // flags
		S_NULL          // raisestate
	},

	{           // MT_RINGICO
		-1,              // doomednum
		S_SUPERRINGBOX2, // spawnstate
		1,               // spawnhealth
		S_NULL,          // seestate
		sfx_itemup,      // seesound
		10,              // reactiontime
		sfx_None,        // attacksound
		0,               // painstate
		0,               // painchance
		sfx_None,        // painsound
		S_NULL,          // meleestate
		S_NULL,          // missilestate
		S_DISS,          // deathstate
		S_NULL,          // xdeathstate
		sfx_None,        // deathsound
		2*FRACUNIT,      // speed
		8*FRACUNIT,      // radius
		14*FRACUNIT,     // height
		0,               // display offset
		100,             // mass
		62*FRACUNIT,     // damage
		sfx_None,        // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY|MF_NOGRAVITY|MF_BOXICON, // flags
		S_NULL           // raisestate
	},

	{           // MT_SHOESICO
		-1,             // doomednum
		S_SHTV2,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		2*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		14*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		62*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY|MF_NOGRAVITY|MF_BOXICON, // flags
		S_NULL          // raisestate
	},

	{           // MT_INVCICO
		-1,             // doomednum
		S_PINV3,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		2*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		14*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		62*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY|MF_NOGRAVITY|MF_BOXICON, // flags
		S_NULL          // raisestate
	},

	{           // MT_1UPICO
		-1,             // doomednum
		S_PLAY_BOX1C,   // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		2*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		14*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		62*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY|MF_NOGRAVITY|MF_BOXICON, // flags
		S_NULL          // raisestate
	},

	{           // MT_YSHIELDICO
		-1,             // doomednum
		S_YLTV2,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_shield,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		2*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		14*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		62*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY|MF_NOGRAVITY|MF_BOXICON, // flags
		S_NULL          // raisestate
	},

	{           // MT_BSHIELDICO
		-1,             // doomednum
		S_BLTV3,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_shield,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		2*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		14*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		62*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY|MF_NOGRAVITY|MF_BOXICON, // flags
		S_NULL          // raisestate
	},

	{           // MT_KSHIELDICO
		-1,             // doomednum
		S_BKTV3,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_shield,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		2*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		14*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		62*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY|MF_NOGRAVITY|MF_BOXICON, // flags
		S_NULL          // raisestate
	},

	{           // MT_WSHIELDICO
		-1,             // doomednum
		S_WHTV3,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_shield,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		2*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		14*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		62*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY|MF_NOGRAVITY|MF_BOXICON, // flags
		S_NULL          // raisestate
	},

	{           // MT_GSHIELDICO
		-1,             // doomednum
		S_GRTV2,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_shield,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		2*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		14*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		62*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY|MF_NOGRAVITY|MF_BOXICON, // flags
		S_NULL          // raisestate
	},

	{           // MT_EGGMANICO
		-1,             // doomednum
		S_EGGTV3,       // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		2*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		14*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		62*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY|MF_NOGRAVITY|MF_BOXICON, // flags
		S_NULL          // raisestate
	},

	{           // MT_MIXUPICO
		-1,             // doomednum
		S_MIXUPBOX3,    // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		2*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		14*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		62*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY|MF_NOGRAVITY|MF_BOXICON, // flags
		S_NULL          // raisestate
	},

	{           // MT_GRAVITYICO
		-1,             // doomednum
		S_GBTV3,       // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		2*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		14*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		62*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY|MF_NOGRAVITY|MF_BOXICON, // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKET
		-1,             // doomednum
		S_ROCKET,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_rlaunc,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		20*FRACUNIT,    // speed
		11*FRACUNIT,    // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		20,             // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_TORPEDO
		-1,             // doomednum
		S_TORPEDO,      // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_rlaunc,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		20*FRACUNIT,    // speed
		11*FRACUNIT,    // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		20,             // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_MINE
		-1,             // doomednum
		S_MINE1,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_MINE_BOOM1,   // deathstate
		S_MINE_BOOM1,   // xdeathstate
		sfx_cybdth,     // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		10*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		64,             // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_MISSILE, // flags
		S_NULL          // raisestate
	},

	{           // MT_JETTBULLET
		-1,             // doomednum
		S_JETBULLET1,   // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		0,              // seesound
		32,             // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		200,            // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		20*FRACUNIT,    // speed
		4*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		1,              // damage
		0,              // activesound
		MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_TURRETLASER
		-1,             // doomednum
		S_TURRETLASER,  // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_TURRETLASEREXPLODE1, // deathstate
		S_NULL,         // xdeathstate
		sfx_turhit,     // deathsound
		50*FRACUNIT,    // speed
		12*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_CANNONBALL
		-1,             // doomednum
		S_CANNONBALL1,  // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_cannon,     // seesound
		32,             // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		200,            // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_cybdth,     // deathsound
		16*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		0,              // activesound
		MF_NOBLOCKMAP|MF_MISSILE, // flags
		S_NULL          // raisestate
	},

	{           // MT_CANNONBALLDECOR
		526,            // doomednum
		S_CANNONBALL1,  // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		0,              // seesound
		32,             // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		200,            // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		16*FRACUNIT,    // speed
		20*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		0,              // activesound
		MF_SOLID|MF_PUSHABLE|MF_SLIDEME, // flags
		S_NULL          // raisestate
	},

	{           // MT_ARROW
		-1,             // doomednum
		S_ARROW,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		0,              // seesound
		32,             // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		200,            // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_ARROWDOWN,    // xdeathstate
		sfx_None,       // deathsound
		16*FRACUNIT,    // speed
		4*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		1,              // damage
		0,              // activesound
		MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY, // flags
		S_ARROWUP       // raisestate
	},

	{           // MT_GFZFLOWER1
		800,            // doomednum
		S_GFZFLOWERA,   // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_GFZFLOWER2
		801,            // doomednum
		S_GFZFLOWERB1,  // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		96*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_GFZFLOWER3
		802,            // doomednum
		S_GFZFLOWERC1,  // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_BERRYBUSH
		804,            // doomednum
		S_BERRYBUSH,    // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_BUSH
		805,            // doomednum
		S_BUSH,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_THZPLANT
		900,            // doomednum
		S_THZPLANT1,    // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		8*FRACUNIT,     // radius
		32*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_ALARM
		901,            // doomednum
		S_ALARM1,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_alarm,      // deathsound
		1,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_GARGOYLE
		1000,           // doomednum
		S_GARGOYLE,     // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		21*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_statu2,     // activesound
		MF_SLIDEME|MF_SOLID|MF_PUSHABLE, // flags
		S_NULL          // raisestate
	},

	{           // MT_SEAWEED
		1001,           // doomednum
		S_SEAWEED1,     // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		24*FRACUNIT,    // radius
		56*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_WATERDRIP
		1002,           // doomednum
		S_DRIPA1,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		1*FRACUNIT,     // radius
		15*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SPAWNCEILING|MF_NOGRAVITY,  // flags
		S_NULL          // raisestate
	},

	{           // MT_WATERDROP
		-1,             // doomednum
		S_DRIPB1,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DRIPC1,       // deathstate
		S_NULL,         // xdeathstate
		sfx_wdrip1,     // deathsound
		0,              // speed
		2*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		8,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SCENERY,     // flags
		S_NULL          // raisestate
	},

	{           // MT_CORAL1
		1003,           // doomednum
		S_CORAL1,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY,     // flags
		S_NULL          // raisestate
	},

	{           // MT_CORAL2
		1004,           // doomednum
		S_CORAL2,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY,     // flags
		S_NULL          // raisestate
	},

	{           // MT_CORAL3
		1005,           // doomednum
		S_CORAL3,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY,     // flags
		S_NULL          // raisestate
	},

	{           // MT_BLUECRYSTAL
		1006,           // doomednum
		S_BLUECRYSTAL1, // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY,     // flags
		S_NULL          // raisestate
	},

	{           // MT_CHAIN
		1100,           // doomednum
		S_CEZCHAIN,     // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		4*FRACUNIT,     // radius
		128*FRACUNIT,   // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SPAWNCEILING|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_FLAME
		1101,           // doomednum
		S_FLAME1,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY|MF_SPECIAL|MF_FIRE, // flags
		S_NULL          // raisestate
	},

	{           // MT_EGGSTATUE
		1102,           // doomednum
		S_EGGSTATUE1,   // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		32*FRACUNIT,    // radius
		240*FRACUNIT,   // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_SOLID|MF_PUSHABLE|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_MACEPOINT
		1104,           // doomednum
		S_NULL,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		10*FRACUNIT,    // speed
		128*FRACUNIT,   // radius
		1*FRACUNIT,     // height
		0,              // display offset
		10000,          // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOCLIP|MF_NOGRAVITY|MF_NOCLIPHEIGHT, // flags
		S_NULL          // raisestate
	},

	{           // MT_SWINGMACEPOINT
		1105,           // doomednum
		S_NULL,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		10*FRACUNIT,    // speed
		128*FRACUNIT,   // radius
		1*FRACUNIT,     // height
		0,              // display offset
		10000,          // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOCLIP|MF_NOGRAVITY|MF_NOCLIPHEIGHT, // flags
		S_NULL          // raisestate
	},

	{           // MT_HANGMACEPOINT
		1106,           // doomednum
		S_NULL,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		10*FRACUNIT,    // speed
		128*FRACUNIT,   // radius
		1*FRACUNIT,     // height
		0,              // display offset
		10000,          // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOCLIP|MF_NOGRAVITY|MF_NOCLIPHEIGHT, // flags
		S_NULL          // raisestate
	},

	{           // MT_SPINMACEPOINT
		1107,           // doomednum
		S_NULL,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		10*FRACUNIT,    // speed
		128*FRACUNIT,   // radius
		1*FRACUNIT,     // height
		0,              // display offset
		200,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOCLIP|MF_NOGRAVITY|MF_NOCLIPHEIGHT, // flags
		S_NULL          // raisestate
	},

	{            // MT_SMALLMACECHAIN
		-1,               // doomednum
		S_SMALLMACECHAIN, // spawnstate
		1000,             // spawnhealth
		S_NULL,           // seestate
		sfx_None,         // seesound
		8,                // reactiontime
		sfx_None,         // attacksound
		S_NULL,           // painstate
		0,                // painchance
		sfx_None,         // painsound
		S_NULL,           // meleestate
		S_NULL,           // missilestate
		S_NULL,           // deathstate
		S_NULL,           // xdeathstate
		sfx_None,         // deathsound
		24*FRACUNIT,      // speed
		24*FRACUNIT,      // radius
		48*FRACUNIT,      // height
		0,                // display offset
		100,              // mass
		1,                // damage
		sfx_None,         // activesound
		MF_NOGRAVITY|MF_SPECIAL|MF_NOCLIPHEIGHT, // flags
		S_NULL            // raisestate
	},

	{            // MT_BIGMACECHAIN
		-1,             // doomednum
		S_BIGMACECHAIN,	// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		48*FRACUNIT,    // speed
		48*FRACUNIT,    // radius
		96*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY|MF_SPECIAL|MF_NOCLIPHEIGHT, // flags
		S_NULL          // raisestate
	},

	{            // MT_SMALLMACE
		-1,             // doomednum
		S_SMALLMACE,    // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		24*FRACUNIT,    // speed
		17*FRACUNIT,    // radius
		34*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_mswing,     // activesound
		MF_SPECIAL|MF_NOGRAVITY|MF_NOCLIPHEIGHT, // flags
		S_NULL          // raisestate
	},

	{            // MT_BIGMACE
		-1,             // doomednum
		S_BIGMACE,      // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		48*FRACUNIT,    // speed
		34*FRACUNIT,    // radius
		68*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_mswing,     // activesound
		MF_SPECIAL|MF_NOGRAVITY|MF_NOCLIPHEIGHT, // flags
		S_NULL          // raisestate
	},

	{           // MT_CEZFLOWER
		1103,           // doomednum
		S_CEZFLOWER1,   // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_BIGTUMBLEWEED
		1200,           // doomednum
		S_BIGTUMBLEWEED,// spawnstate
		1000,           // spawnhealth
		S_BIGTUMBLEWEED_ROLL1, // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		24*FRACUNIT,    // radius
		48*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_s3k_50,     // activesound
		MF_SPECIAL|MF_BOUNCE,      // flags
		S_NULL          // raisestate
	},

	{           // MT_LITTLETUMBLEWEED
		1201,           // doomednum
		S_LITTLETUMBLEWEED,// spawnstate
		1000,           // spawnhealth
		S_LITTLETUMBLEWEED_ROLL1, // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		12*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_s3k_50,     // activesound
		MF_SPECIAL|MF_BOUNCE,      // flags
		S_NULL          // raisestate
	},

	{           // MT_CACTI1
		1203,           // doomednum
		S_CACTI1,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_CACTI2
		1204,           // doomednum
		S_CACTI2,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		64*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_CACTI3
		1205,           // doomednum
		S_CACTI3,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_CACTI4
		1206,           // doomednum
		S_CACTI4,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		80*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_FLAMEJET
		1300,           // doomednum
		S_FLAMEJETSTND, // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY|MF_NOGRAVITY|MF_NOBLOCKMAP|MF_NOSECTOR, // flags
		S_NULL          // raisestate
	},

	{           // MT_VERTICALFLAMEJET
		1301,           // doomednum
		S_FLAMEJETSTND, // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY|MF_NOGRAVITY|MF_NOBLOCKMAP|MF_NOSECTOR, // flags
		S_NULL          // raisestate
	},

	{           // MT_FLAMEJETFLAME
		-1,             // doomednum
		S_FLAMEJETFLAME1, // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		5*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY|MF_MISSILE|MF_FIRE, // flags
		S_NULL          // raisestate
	},

	{           // MT_XMASPOLE
		1850,           // doomednum
		S_XMASPOLE,     // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_CANDYCANE
		1851,           // doomednum
		S_CANDYCANE,    // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		8*FRACUNIT,     // radius
		32*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_SNOWMAN
		1852,           // doomednum
		S_SNOWMAN,      // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		25*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SOLID|MF_PUSHABLE, // flags
		S_NULL          // raisestate
	},

	{           // MT_DBALL
		1875,           // doomednum
		S_DBALL1,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		16*FRACUNIT,    // radius
		54*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SPAWNCEILING|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_EGGSTATUE2
		1876,           // doomednum
		S_EGGSTATUE2,   // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		20*FRACUNIT,    // radius
		96*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_SOLID|MF_PUSHABLE|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_THOK
		-1,             // doomednum
		S_THOK1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		32*FRACUNIT,    // radius
		64*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_GREENORB
		-1,             // doomednum
		S_SORB1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		12,             // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		pw_watershield, // speed
		64*FRACUNIT,    // radius
		64*FRACUNIT,    // height
		1,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_YELLOWORB
		-1,             // doomednum
		S_SORB1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		14,             // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		pw_ringshield,  // speed
		64*FRACUNIT,    // radius
		64*FRACUNIT,    // height
		1,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_BLUEORB
		-1,             // doomednum
		S_SORB1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		7,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		pw_forceshield, // speed
		64*FRACUNIT,    // radius
		64*FRACUNIT,    // height
		1,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_BLACKORB
		-1,             // doomednum
		S_SORB1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		6,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		pw_bombshield,  // speed
		64*FRACUNIT,    // radius
		64*FRACUNIT,    // height
		1,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_WHITEORB
		-1,             // doomednum
		S_SORB1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		13,             // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		pw_jumpshield,  // speed
		64*FRACUNIT,    // radius
		64*FRACUNIT,    // height
		1,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_IVSP
		-1,             // doomednum
		S_IVSP1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		64*FRACUNIT,    // radius
		64*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_SUPERSPARK
		-1,             // doomednum
		S_SSPK1,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_BIRD
		-1,             // doomednum
		S_BIRD1,        // spawnstate
		1000,           // spawnhealth
		S_BIRD1,        // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIPTHING|MF_FLOAT|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	// freed bunny
	{           // MT_BUNNY
		-1,             // doomednum
		S_BUNNY1,       // spawnstate
		1000,           // spawnhealth
		S_BUNNY1,       // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIPTHING|MF_FLOAT, // flags
		S_NULL          // raisestate
	},

	{           // MT_MOUSE
		-1,             // doomednum
		S_MOUSE1,       // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		0,              // seesound
		1,              // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		200,            // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		20*FRACUNIT,    // speed
		4*FRACUNIT,     // radius
		4*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_NOCLIPTHING, // flags
		S_NULL          // raisestate
	},

	{           // MT_CHICKEN
		-1,             // doomednum
		S_CHICKEN1,     // spawnstate
		1000,           // spawnhealth
		S_CHICKEN1,     // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIPTHING|MF_FLOAT, // flags
		S_NULL          // raisestate
	},

	{           // MT_COW
		-1,             // doomednum
		S_COW1,       // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		0,              // seesound
		1,              // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		200,            // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		4,              // speed
		4*FRACUNIT,     // radius
		4*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_NOCLIPTHING, // flags
		S_NULL          // raisestate
	},

	{           // MT_YELLOWSPRING
		550,            // doomednum
		S_YELLOWSPRING, // spawnstate
		1000,           // spawnhealth
		S_YELLOWSPRING2,// seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_spring,     // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		20*FRACUNIT,    // speed
		20*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SPRING, // flags
		S_NULL          // raisestate
	},

	{           // MT_REDSPRING
		551,            // doomednum
		S_REDSPRING,    // spawnstate
		1000,           // spawnhealth
		S_REDSPRING2,   // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_spring,     // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		32*FRACUNIT,    // speed
		20*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SPRING, // flags
		S_NULL          // raisestate
	},

	{           // MT_BLUESPRING
		552,            // doomednum
		S_BLUESPRING,   // spawnstate
		1000,           // spawnhealth
		S_BLUESPRING2,  // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_spring,     // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		11*FRACUNIT,    // speed
		20*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SPRING, // flags
		S_NULL          // raisestate
	},

	{           // MT_YELLOWSPRINGDOWN
		553,            // doomednum
		S_YELLOWSPRINGUD, // spawnstate
		1000,           // spawnhealth
		S_YELLOWSPRINGUD2, // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_spring,     // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		-20*FRACUNIT,   // speed
		20*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SOLID|MF_NOGRAVITY|MF_SPRING|MF_SPAWNCEILING, // flags
		S_NULL          // raisestate
	},

	{           // MT_REDSPRINGDOWN
		554,            // doomednum
		S_REDSPRINGUD,  // spawnstate
		1000,           // spawnhealth
		S_REDSPRINGUD2, // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_spring,     // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		-32*FRACUNIT,   // speed
		20*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SOLID|MF_NOGRAVITY|MF_SPRING|MF_SPAWNCEILING, // flags
		S_NULL          // raisestate
	},

	{           // MT_YELLOWDIAG
		555,            // doomednum
		S_YDIAG1,       // spawnstate
		1,              // spawnhealth
		S_YDIAG2,       // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_spring,     // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		20*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		20*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SPRING, // flags
		S_NULL          // raisestate
	},

	{           // MT_REDDIAG
		556,            // doomednum
		S_RDIAG1,       // spawnstate
		1,              // spawnhealth
		S_RDIAG2,       // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_spring,     // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		32*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		32*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_SOLID|MF_SPRING, // flags
		S_NULL          // raisestate
	},

	{           // MT_YELLOWDIAGDOWN
		557,            // doomednum
		S_YDIAGD1,      // spawnstate
		1,              // spawnhealth
		S_YDIAGD2,      // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_spring,     // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		-20*FRACUNIT,   // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		20*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_SOLID|MF_NOGRAVITY|MF_SPRING|MF_SPAWNCEILING, // flags
		S_NULL          // raisestate
	},

	{           // MT_REDDIAGDOWN
		558,            // doomednum
		S_RDIAGD1,      // spawnstate
		1,              // spawnhealth
		S_RDIAGD2,      // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_spring,     // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		-32*FRACUNIT,   // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		32*FRACUNIT,    // damage
		sfx_None,       // activesound
		MF_SOLID|MF_NOGRAVITY|MF_SPRING|MF_SPAWNCEILING, // flags
		S_NULL          // raisestate
	},

	{           // MT_RAIN
		-1,             // doomednum
		S_RAIN1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		-24*FRACUNIT,   // speed
		1*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SNOWFLAKE
		-1,             // doomednum
		S_SNOW1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		-2*FRACUNIT,    // speed
		4*FRACUNIT,     // radius
		4*FRACUNIT,     // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SPLISH
		-1,             // doomednum
		S_SPLISH1,      // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		6*FRACUNIT,     // radius
		1*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_SMOK
		-1,             // doomednum
		S_SMOK1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		20*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOGRAVITY|MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_SMALLBUBBLE
		-1,             // doomednum
		S_SMALLBUBBLE,  // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		4*FRACUNIT,     // radius
		4*FRACUNIT,     // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_MEDIUMBUBBLE
		-1,             // doomednum
		S_MEDIUMBUBBLE, // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		8*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_EXTRALARGEBUBBLE
		-1,             // doomednum
		S_LARGEBUBBLE,  // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		8*FRACUNIT,     // radius
		12*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_POP
		-1,             // doomednum
		S_POP1,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_TFOG
		-1,             // doomednum
		S_FOG1,         // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOGRAVITY|MF_NOCLIP, // flags
		S_NULL          // raisestate
	},

	{           // MT_SEED
		-1,             // doomednum
		S_SEED,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		-2*FRACUNIT,    // speed
		4*FRACUNIT,     // radius
		4*FRACUNIT,     // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_PARTICLE
		-1,             // doomednum
		S_PARTICLE,     // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		4*FRACUNIT,     // speed
		4*FRACUNIT,     // radius
		4*FRACUNIT,     // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOGRAVITY|MF_NOCLIPHEIGHT|MF_NOCLIP, // flags
		S_NULL          // raisestate
	},

	{           // MT_PARTICLEGEN
		757,            // doomednum
		S_PARTICLEGEN,  // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		1*FRACUNIT,     // radius
		1*FRACUNIT,     // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOCLIP|MF_NOGRAVITY|MF_NOCLIPHEIGHT, // flags
		S_NULL          // raisestate
	},

	{           // MT_SCORE
		-1,             // doomednum
		S_SCRA,         // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		3*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_DROWNNUMBERS
		-1,             // doomednum
		S_ZERO1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		8*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_GOTEMERALD
		-1,             // doomednum
		S_CEMG1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOGRAVITY|MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_TAG
		-1,             // doomednum
		S_TTAG1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_GOTFLAG
		-1,             // doomednum
		S_GOTFLAG1,     // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		64*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_GOTFLAG2
		-1,             // doomednum
		S_GOTFLAG3,     // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		64*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	// ambient water 1a (large)
	{           // MT_AWATERA
		700,            // doomednum
		S_NULL,         // spawnstate
		35,             // spawnhealth
		S_NULL,         // seestate
		sfx_amwtr1,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY|MF_AMBIENT, // flags
		S_NULL          // raisestate
	},

	// ambient water 1b (large)
	{           // MT_AWATERB
		701,            // doomednum
		S_NULL,         // spawnstate
		35,             // spawnhealth
		S_NULL,         // seestate
		sfx_amwtr2,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY|MF_AMBIENT, // flags
		S_NULL          // raisestate
	},

	// ambient water 2a (medium)
	{           // MT_AWATERC
		702,            // doomednum
		S_NULL,         // spawnstate
		35,             // spawnhealth
		S_NULL,         // seestate
		sfx_amwtr3,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY|MF_AMBIENT, // flags
		S_NULL          // raisestate
	},

	// ambient water 2b (medium)
	{           // MT_AWATERD
		703,            // doomednum
		S_NULL,         // spawnstate
		35,             // spawnhealth
		S_NULL,         // seestate
		sfx_amwtr4,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY|MF_AMBIENT, // flags
		S_NULL          // raisestate
	},

	// ambient water 3a (small)
	{           // MT_AWATERE
		704,            // doomednum
		S_NULL,         // spawnstate
		35,             // spawnhealth
		S_NULL,         // seestate
		sfx_amwtr5,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY|MF_AMBIENT, // flags
		S_NULL          // raisestate
	},

	// ambient water 3b (small)
	{           // MT_AWATERF
		705,            // doomednum
		S_NULL,         // spawnstate
		35,             // spawnhealth
		S_NULL,         // seestate
		sfx_amwtr6,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY|MF_AMBIENT, // flags
		S_NULL          // raisestate
	},

	// ambient water 4a (extra large)
	{           // MT_AWATERG
		706,            // doomednum
		S_NULL,         // spawnstate
		35,             // spawnhealth
		S_NULL,         // seestate
		sfx_amwtr7,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY|MF_AMBIENT, // flags
		S_NULL          // raisestate
	},

	// ambient water 4b (extra large)
	{           // MT_AWATERH
		707,            // doomednum
		S_NULL,         // spawnstate
		35,             // spawnhealth
		S_NULL,         // seestate
		sfx_amwtr8,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY|MF_AMBIENT, // flags
		S_NULL          // raisestate
	},

	{           // MT_RANDOMAMBIENT
		708,            // doomednum
		S_NULL,         // spawnstate
		512,            // spawnhealth: repeat speed
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOSECTOR|MF_NOBLOCKMAP|MF_NOGRAVITY|MF_AMBIENT, // flags
		S_NULL          // raisestate
	},

	{           // MT_RANDOMAMBIENT2
		709,            // doomednum
		S_NULL,         // spawnstate
		220,            // spawnhealth: repeat speed
		S_NULL,         // seestate
		sfx_ambin2,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOSECTOR|MF_NOBLOCKMAP|MF_NOGRAVITY|MF_AMBIENT, // flags
		S_NULL          // raisestate
	},

	{           // MT_REDRING
		-1,             // doomednum
		S_RRNG1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_thok,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_SPRK1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_itemup,     // deathsound
		60*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_BOUNCERING
		301,            // doomednum
		S_BOUNCERING1,  // spawnstate
		10,             // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		24*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_RAILRING
		302,            // doomednum
		S_RAILRING1,    // spawnstate
		5,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		24*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_AUTOMATICRING
		304,            // doomednum
		S_AUTOMATICRING1, // spawnstate
		30,             // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		24*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_EXPLOSIONRING
		305,            // doomednum
		S_EXPLOSIONRING1, // spawnstate
		5,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_DISS,         // meleestate
		S_DISS,         // missilestate
		S_DISS,         // deathstate
		S_DISS,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		24*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_SCATTERRING
		306,            // doomednum
		S_SCATTERRING1, // spawnstate
		10,             // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		24*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_GRENADERING
		307,            // doomednum
		S_GRENADERING1, // spawnstate
		5,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_DISS,         // meleestate
		S_DISS,         // missilestate
		S_DISS,         // deathstate
		S_DISS,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		24*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_BOUNCEPICKUP
		330,            // doomednum
		S_BOUNCEPICKUP1,// spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		10,             // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		1,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		24*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		2*TICRATE,      // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_BOUNCEPICKUPFADE1 // raisestate
	},

	{           // MT_RAILPICKUP
		331,            // doomednum
		S_RAILPICKUP1,  // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		5,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		2,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		24*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		2*TICRATE,      // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_RAILPICKUPFADE1 // raisestate
	},

	{           // MT_AUTOPICKUP
		332,            // doomednum
		S_AUTOPICKUP1,  // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		30,             // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		4,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		24*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		2*TICRATE,      // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_AUTOPICKUPFADE1 // raisestate
	},

	{           // MT_EXPLODEPICKUP
		333,            // doomednum
		S_EXPLODEPICKUP1,// spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		5,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		8,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		24*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		2*TICRATE,      // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_EXPLODEPICKUPFADE1 // raisestate
	},

	{           // MT_SCATTERPICKUP
		334,            // doomednum
		S_SCATTERPICKUP1,// spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		10,             // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		8,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		24*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		2*TICRATE,      // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_SCATTERPICKUPFADE1 // raisestate
	},

	{           // MT_GRENADEPICKUP
		335,            // doomednum
		S_GRENADEPICKUP1,// spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		5,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		8,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		24*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		2*TICRATE,      // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_GRENADEPICKUPFADE1 // raisestate
	},

	{           // MT_THROWNBOUNCE
		-1,             // doomednum
		S_THROWNBOUNCE1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
#ifdef WEAPON_SFX
		sfx_bnce1,      // seesound
#else
		sfx_thok,       // seesound
#endif
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_SPRK1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_itemup,     // deathsound
		60*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_bnce1,      // activesound
		MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY|MF_BOUNCE, // flags
		S_NULL          // raisestate
	},

	{           // MT_THROWNAUTOMATIC
		-1,             // doomednum
		S_THROWNAUTOMATIC1, // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
#ifdef WEAPON_SFX
		sfx_s3k_129,    // seesound
#else
		sfx_thok,       // seesound
#endif
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_SPRK1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_itemup,     // deathsound
		60*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_THROWNSCATTER
		-1,             // doomednum
		S_THROWNSCATTER,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
#ifdef WEAPON_SFX
		sfx_s3k_129,    // seesound
#else
		sfx_thok,       // seesound
#endif
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_SPRK1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_itemup,     // deathsound
		60*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_THROWNEXPLOSION
		-1,             // doomednum
		S_THROWNEXPLOSION1, // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
#ifdef WEAPON_SFX
		sfx_s3k_45,     // seesound
#else
		sfx_thok,       // seesound
#endif
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		192*FRACUNIT,   // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_RINGEXPLODE,  // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		60*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_THROWNGRENADE
		-1,             // doomednum
		S_THROWNGRENADE1, // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
#ifdef WEAPON_SFX
		sfx_s3k_32,     // seesound 30?
#else
		sfx_thok,       // seesound
#endif
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		128*FRACUNIT,   // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_RINGEXPLODE,  // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		60*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		7*TICRATE,      // mass
		1,              // damage
		sfx_gbeep,      // activesound
		MF_NOBLOCKMAP|MF_SLIDEME|MF_FLOAT|MF_NOCLIPTHING|MF_MISSILE|MF_SHOOTABLE, // flags
		S_NULL          // raisestate
	},

	{           // MT_COIN
		1800,           // doomednum
		S_COIN1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		MT_FLINGCOIN,   // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_FLINGCOIN
		-1,             // doomednum
		S_COIN1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		MT_FLINGCOIN,   // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		MT_COIN,        // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL, // flags
		S_NULL          // raisestate
	},

	{           // MT_COINSPARKLE
		-1,             // doomednum
		S_COINSPARKLE1, // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		32*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_GOOMBA
		1801,           // doomednum
		S_GOOMBA1,      // spawnstate
		1,              // spawnhealth
		S_GOOMBA2,      // seestate
		0,              // seesound
		32,             // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		200,            // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_GOOMBA_DEAD,  // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		6,              // speed
		24*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE, // flags
		S_NULL          // raisestate
	},

	{           // MT_BLUEGOOMBA
		1802,              // doomednum
		S_BLUEGOOMBA1,     // spawnstate
		1,                 // spawnhealth
		S_BLUEGOOMBA2,     // seestate
		0,                 // seesound
		32,                // reactiontime
		0,                 // attacksound
		S_NULL,            // painstate
		170,               // painchance
		0,                 // painsound
		0,                 // meleestate
		S_NULL,            // missilestate
		S_BLUEGOOMBA_DEAD, // deathstate
		S_NULL,            // xdeathstate
		sfx_pop,           // deathsound
		6,                 // speed
		24*FRACUNIT,       // radius
		32*FRACUNIT,       // height
		0,                 // display offset
		100,               // mass
		0,                 // damage
		0,                 // activesound
		MF_ENEMY|MF_SPECIAL|MF_SHOOTABLE, // flags
		S_NULL             // raisestate
	},

	{           // MT_FIREFLOWER
		1803,           // doomednum
		S_FIREFLOWER1,  // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL,     // flags
		S_NULL          // raisestate
	},

	{           // MT_FIREBALL
		-1,             // doomednum
		S_FIREBALL1,    // spawnstate
		1000,           // spawnhealth
		S_FIREBALLEXP1, // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_FIREBALLEXP1, // meleestate
		S_FIREBALLEXP1, // missilestate
		S_FIREBALLEXP1, // deathstate
		S_FIREBALLEXP1, // xdeathstate
		sfx_None,       // deathsound
		10*FRACUNIT,    // speed
		4*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_FIRE, // flags
		S_NULL          // raisestate
	},

	{           // MT_SHELL
		1804,           // doomednum
		S_SHELL,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		20*FRACUNIT,    // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_tink,       // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_BOUNCE, // flags
		S_NULL          // raisestate
	},

	{           // MT_PUMA
		1805,           // doomednum
		S_PUMA1,        // spawnstate
		1000,           // spawnhealth
		S_PUMA1,        // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_PUMA4,        // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_PUMA6,        // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_FIRE,     // flags
		S_NULL          // raisestate
	},
	{           // MT_HAMMER
		-1,             // doomednum
		S_HAMMER1,      // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		4*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL,     // flags
		S_NULL          // raisestate
	},
	{           // MT_KOOPA
		1806,           // doomednum
		S_KOOPA1,       // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_DISS,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		48*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL,     // flags
		S_NULL          // raisestate
	},

	{           // MT_KOOPAFLAME
		-1,             // doomednum
		S_KOOPAFLAME1,  // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		5*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY|MF_MISSILE|MF_FIRE, // flags
		S_NULL          // raisestate
	},

	{           // MT_AXE
		1807,           // doomednum
		S_AXE1,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL,     // flags
		S_NULL          // raisestate
	},

	{           // MT_MARIOBUSH1
		1808,           // doomednum
		S_MARIOBUSH1,   // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_MARIOBUSH2
		1809,           // doomednum
		S_MARIOBUSH2,   // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_TOAD
		1810,           // doomednum
		S_TOAD,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_AXIS
		1700,           // doomednum
		S_NULL,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		10*FRACUNIT,    // speed
		256*FRACUNIT,   // radius
		1*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOCLIP, // flags
		S_NULL          // raisestate
	},

	{           // MT_AXISTRANSFER
		1701,           // doomednum
		S_NULL,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		10,             // speed
		16*FRACUNIT,    // radius
		1,              // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOCLIP,    // flags
		S_NULL          // raisestate
	},

	{           // MT_AXISTRANSFERLINE
		1702,           // doomednum
		S_NULL,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		10,             // speed
		32*FRACUNIT,    // radius
		1,              // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOCLIP,    // flags
		S_NULL          // raisestate
	},

	{           // MT_NIGHTSDRONE
		1703,           // doomednum
		S_NIGHTSDRONE1, // spawnstate
		120,            // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		56*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL,     // flags
		S_NULL          // raisestate
	},

	{           // MT_NIGHTSCHAR
		-1,             // doomednum
		S_NIGHTSFLY1A,  // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NIGHTSFLY1A,  // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NIGHTSFLY1A,  // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		56*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_NOGRAVITY|MF_NOBLOCKMAP, // flags
		S_NULL          // raisestate
	},

	{           // MT_NIGHTSPARKLE
		-1,             // doomednum
		S_NIGHTSPARKLE1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		0,              // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		2*FRACUNIT,     // radius
		4*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_NIGHTSBUMPER
		1704,           // doomednum
		S_NIGHTSBUMPER1,// spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_nbmper,     // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		21000,          // speed
		32*FRACUNIT,    // radius
		64*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SCENERY|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_HOOP
		-1,             // doomednum
		S_HOOP,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOGRAVITY|MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_HOOPCOLLIDE
		-1,             // doomednum
		S_NULL,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOSECTOR|MF_NOCLIP|MF_NOGRAVITY|MF_SPECIAL|MF_NOTHINK, // flags
		S_NULL          // raisestate
	},

	{           // MT_HOOPCENTER
		-1,             // doomednum
		S_NULL,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		2*FRACUNIT,     // radius
		4*FRACUNIT,     // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_NIGHTSCORE
		-1,             // doomednum
		S_NIGHTSCORE10, // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NIGHTSCORE10_2, // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		8*FRACUNIT,     // radius
		8*FRACUNIT,     // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_NIGHTSWING
		1706,           // doomednum
		S_NIGHTSWING,   // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		12*FRACUNIT,    // radius
		24*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SLIDEME|MF_SPECIAL|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_NIGHTSSUPERLOOP
		1707,           // doomednum
		S_NIGHTSPOWERUP1, // spawnstate
		1000,           // spawnhealth
		S_NIGHTSPOWERUP2, // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY,   // flags
		S_NULL          // raisestate
	},

	{           // MT_NIGHTSDRILLREFILL
		1708,           // doomednum
		S_NIGHTSPOWERUP3, // spawnstate
		1000,           // spawnhealth
		S_NIGHTSPOWERUP4, // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY,   // flags
		S_NULL          // raisestate
	},

	{           // MT_NIGHTSHELPER
		1709,           // doomednum
		S_NIGHTSPOWERUP5, // spawnstate
		1000,           // spawnhealth
		S_NIGHTSPOWERUP6, // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY,   // flags
		S_NULL          // raisestate
	},

	{           // MT_EGGCAPSULE
		1710,           // doomednum
		S_EGGCAPSULE,   // spawnstate
		20,             // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		72*FRACUNIT,    // radius
		144*FRACUNIT,   // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,       // activesound
		MF_NOGRAVITY|MF_FLOAT|MF_SPECIAL, // flags
		S_NULL          // raisestate
	},

	{           // MT_CHAOSSPAWNER
		750,            // doomednum
		S_NULL,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_TELEPORTMAN
		751,            // doomednum
		S_INVISIBLE,    // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8,              // radius
		8,              // height
		0,              // display offset
		10,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{			// MT_ALTVIEWMAN
		752,            // doomednum
		S_INVISIBLE,    // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8,              // radius
		8,              // height
		0,              // display offset
		10,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_CRUMBLEOBJ
		-1,             // doomednum
		S_CRUMBLE1,     // spawnstate
		1000,           // spawnhealth
		S_CRUMBLE1,     // seestate
		0,              // seesound
		1,              // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		200,            // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_crumbl,     // deathsound
		3,              // speed
		1*FRACUNIT,     // radius
		1*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	// dissipating object
	{           // MT_DISS
		-1,             // doomednum
		S_DISS,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		1*FRACUNIT,     // radius
		1*FRACUNIT,     // height
		0,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	// Waypoint for zoom tubes
	{           // MT_TUBEWAYPOINT
		753,            // doomednum
		S_NULL,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		1*FRACUNIT,     // radius
		2*FRACUNIT,     // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOCLIP|MF_NOGRAVITY|MF_NOCLIPHEIGHT,    // flags
		S_NULL          // raisestate
	},

	// for use with wind and current effects
	{           // MT_PUSH
		754,            // doomednum
		S_INVISIBLE,    // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8,              // radius
		8,              // height
		0,              // display offset
		10,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOGRAVITY,  // flags
		S_NULL          // raisestate
	},

	// for use with wind and current effects
	{           // MT_PULL
		755,            // doomednum
		S_INVISIBLE,    // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8,              // radius
		8,              // height
		0,              // display offset
		10,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOGRAVITY, // flags
		S_NULL          // raisestate
	},

	{           // MT_GHOST
		-1,             // doomednum
		S_THOK1,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		3,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		1*FRACUNIT,     // radius
		1*FRACUNIT,     // height
		0,              // display offset
		1000,           // mass
		8,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOGRAVITY|MF_NOCLIP|MF_NOCLIPHEIGHT, // flags
		S_NULL          // raisestate
	},

	{           // MT_POLYANCHOR
		760,            // doomednum
		S_NULL,         // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		3,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		1*FRACUNIT,     // radius
		1*FRACUNIT,     // height
		0,              // display offset
		1000,           // mass
		8,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY|MF_NOCLIP, // flags
		S_NULL          // raisestate
	},

	{           // MT_POLYSPAWN
		761,            // doomednum
		S_NULL,         // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		3,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		1*FRACUNIT,     // radius
		1*FRACUNIT,     // height
		0,              // display offset
		1000,           // mass
		8,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY|MF_NOCLIP, // flags
		S_NULL          // raisestate
	},

	{           // MT_POLYSPAWNCRUSH
		762,            // doomednum
		S_NULL,         // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		3,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		1*FRACUNIT,     // radius
		1*FRACUNIT,     // height
		0,              // display offset
		1000,           // mass
		8,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY|MF_NOCLIP, // flags
		S_NULL          // raisestate
	},

	{           // MT_SPARK
		-1,             // doomednum
		S_SPRK1,        // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		8,              // speed
		32*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		1,              // display offset
		16,             // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_EXPLODE
		-1,             // doomednum
		S_XPLD1,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		0,              // seesound
		32,             // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		200,            // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1*FRACUNIT,     // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_UWEXPLODE
		-1,             // doomednum
		S_WPLD1,        // spawnstate
		1,              // spawnhealth
		S_NULL,         // seestate
		0,              // seesound
		32,             // reactiontime
		0,              // attacksound
		S_NULL,         // painstate
		200,            // painchance
		0,              // painsound
		0,              // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		1*FRACUNIT,     // speed
		16*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		0,              // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_STALAGMITE0
		1900,           // doomednum
		S_STG0,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_STALAGMITE1
		1901,           // doomednum
		S_STG1,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_STALAGMITE2
		1902,           // doomednum
		S_STG2,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_STALAGMITE3
		1903,           // doomednum
		S_STG3,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_STALAGMITE4
		1904,           // doomednum
		S_STG4,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_STALAGMITE5
		1905,           // doomednum
		S_STG5,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_STALAGMITE6
		1906,           // doomednum
		S_STG6,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_STALAGMITE7
		1907,           // doomednum
		S_STG7,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_STALAGMITE8
		1908,           // doomednum
		S_STG8,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_STALAGMITE9
		1909,           // doomednum
		S_STG9,         // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOCLIP|MF_SCENERY, // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKSPAWNER
		1202,           // doomednum
		S_ROCKSPAWN,    // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY,  // flags
		S_NULL          // raisestate
	},

	{           // MT_FALLINGROCK
		-1,             // doomednum
		S_ROCKCRUMBLEA1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		4,              // mass
		0,              // damage
		sfx_rocks1,     // activesound
		MF_SPECIAL|MF_BOUNCE,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE1
		-1,             // doomednum
		S_ROCKCRUMBLEA1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE2
		-1,             // doomednum
		S_ROCKCRUMBLEB1, // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE3
		-1,             // doomednum
		S_ROCKCRUMBLEC1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE4
		-1,             // doomednum
		S_ROCKCRUMBLED1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE5
		-1,             // doomednum
		S_ROCKCRUMBLEE1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE6
		-1,             // doomednum
		S_ROCKCRUMBLEF1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE7
		-1,             // doomednum
		S_ROCKCRUMBLEG1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE8
		-1,             // doomednum
		S_ROCKCRUMBLEH1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE9
		-1,             // doomednum
		S_ROCKCRUMBLEI1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE10
		-1,             // doomednum
		S_ROCKCRUMBLEJ1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE11
		-1,             // doomednum
		S_ROCKCRUMBLEK1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE12
		-1,             // doomednum
		S_ROCKCRUMBLEL1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE13
		-1,             // doomednum
		S_ROCKCRUMBLEM1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE14
		-1,             // doomednum
		S_ROCKCRUMBLEN1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE15
		-1,             // doomednum
		S_ROCKCRUMBLEO1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_ROCKCRUMBLE16
		-1,             // doomednum
		S_ROCKCRUMBLEP1,// spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_ambint,     // seesound
		0,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		255,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		0,              // speed
		8*FRACUNIT,     // radius
		16*FRACUNIT,    // height
		0,              // display offset
		1000,           // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_CRAWLA
		4000,           // doomednum
		S_SRB1_CRAWLA1, // spawnstate
		1,              // spawnhealth
		S_SRB1_CRAWLA2, // seestate
		0,              // seesound
		32,             // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		200,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		3,              // speed
		20*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_ENEMY,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_BAT
		4001,           // doomednum
		S_SRB1_BAT1,    // spawnstate
		1,              // spawnhealth
		S_SRB1_BAT3,    // seestate
		0,              // seesound
		2,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		3072,           // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		4*FRACUNIT,     // speed
		17*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_ENEMY|MF_NOGRAVITY|MF_SLIDEME,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_ROBOFISH
		4002,           // doomednum
		S_SRB1_ROBOFISH1,    // spawnstate
		1,              // spawnhealth
		S_SRB1_ROBOFISH2,    // seestate
		0,              // seesound
		2,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		3072,           // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		4*FRACUNIT,     // speed
		22*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_ENEMY|MF_NOGRAVITY|MF_SLIDEME,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_VOLCANOGUY
		4003,           // doomednum
		S_SRB1_VOLCANOGUY1,    // spawnstate
		1,              // spawnhealth
		S_SRB1_VOLCANOGUY2,    // seestate
		0,              // seesound
		2,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		3072,           // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		4*FRACUNIT,     // speed
		20*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_ENEMY|MF_NOGRAVITY|MF_SLIDEME,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_HOPPY
		4004,           // doomednum
		S_SRB1_HOPPY1,  // spawnstate
		1,              // spawnhealth
		S_SRB1_HOPPY1,  // seestate
		0,              // seesound
		2,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		3072,           // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		FRACUNIT,       // speed
		20*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_ENEMY,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_HOPPYWATER
		4005,           // doomednum
		S_SRB1_HOPPYWATER1,    // spawnstate
		1,              // spawnhealth
		S_SRB1_HOPPYWATER1,    // seestate
		0,              // seesound
		2,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		3072,           // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		FRACUNIT,       // speed
		20*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_ENEMY,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_HOPPYSKYLAB
		4006,           // doomednum
		S_SRB1_HOPPYSKYLAB1,    // spawnstate
		1,              // spawnhealth
		S_SRB1_HOPPYSKYLAB1,    // seestate
		0,              // seesound
		2,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		3072,           // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		FRACUNIT,       // speed
		10*FRACUNIT,    // radius
		34*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_ENEMY,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_MMZFLYING
		4007,           // doomednum
		S_SRB1_MMZFLYING1,    // spawnstate
		1,              // spawnhealth
		S_SRB1_MMZFLYING1,    // seestate
		0,              // seesound
		32,             // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		200,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		FRACUNIT,       // speed
		24*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_ENEMY|MF_NOGRAVITY|MF_FLOAT,  // flags
		S_NULL          // raisestate
	},

	{           // MT_PXVI
		-1,             // doomednum
		S_PXVI,         // spawnstate
		1000,           // spawnhealth
		S_PXVI,         // seestate
		0,              // seesound
		2,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		3072,           // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		FRACUNIT,       // speed
		20*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_NOCLIP|MF_NOGRAVITY,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_UFO
		4008,           // doomednum
		S_SRB1_UFO1,    // spawnstate
		1,              // spawnhealth
		S_SRB1_UFO1,    // seestate
		0,              // seesound
		32,             // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		200,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		3,              // speed
		24*FRACUNIT,    // radius
		32*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_ENEMY|MF_NOGRAVITY|MF_FLOAT|MF_SPAWNCEILING,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_GRAYBOT
		4009,           // doomednum
		S_SRB1_GRAYBOT1,// spawnstate
		1,              // spawnhealth
		S_SRB1_GRAYBOT1,// seestate
		0,              // seesound
		32,             // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		200,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		3,              // speed
		21*FRACUNIT,    // radius
		69*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_ENEMY|MF_NOGRAVITY|MF_FLOAT,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_ROBOTOPOLIS
		4010,           // doomednum
		S_SRB1_ROBOTOPOLIS1,    // spawnstate
		1,              // spawnhealth
		S_SRB1_ROBOTOPOLIS1,    // seestate
		0,              // seesound
		32,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		200,           // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		3,              // speed
		36*FRACUNIT,    // radius
		62*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_ENEMY|MF_NOGRAVITY|MF_FLOAT,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_RBZBUZZ
		4011,           // doomednum
		S_SRB1_RBZBUZZ1,// spawnstate
		1,              // spawnhealth
		S_SRB1_RBZBUZZ1,// seestate
		0,              // seesound
		32,             // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		200,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		3,              // speed
		44*FRACUNIT,    // radius
		45*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_ENEMY|MF_NOGRAVITY|MF_FLOAT,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_RBZSPIKES
		4012,           // doomednum
		S_SRB1_RBZSPIKES1,    // spawnstate
		1,              // spawnhealth
		S_SRB1_RBZSPIKES1,    // seestate
		0,              // seesound
		32,             // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		200,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		3,              // speed
		10*FRACUNIT,    // radius
		53*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SOLID|MF_ENEMY|MF_NOGRAVITY|MF_FLOAT,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_METALSONIC
		4013,           // doomednum
		S_SRB1_METALSONIC1,     // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_NULL,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		21*FRACUNIT,    // speed
		16*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		1,              // damage
		sfx_None,     // activesound
		MF_SLIDEME|MF_SOLID|MF_PUSHABLE, // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_GOLDBOT
		4014,           // doomednum
		S_SRB1_GOLDBOT1,// spawnstate
		1,              // spawnhealth
		S_SRB1_GOLDBOT1,// seestate
		0,              // seesound
		32,             // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		200,            // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		3,              // speed
		21*FRACUNIT,    // radius
		69*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_ENEMY|MF_NOGRAVITY|MF_FLOAT,  // flags
		S_NULL          // raisestate
	},

	{           // MT_SRB1_GENREX
		4015,           // doomednum
		S_SRB1_GENREX1, // spawnstate
		1,              // spawnhealth
		S_SRB1_GENREX2, // seestate
		0,              // seesound
		2,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		3072,           // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_XPLD1,        // deathstate
		S_NULL,         // xdeathstate
		sfx_pop,        // deathsound
		4*FRACUNIT,     // speed
		17*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_SPECIAL|MF_SHOOTABLE|MF_ENEMY|MF_NOGRAVITY|MF_SLIDEME,  // flags
		S_NULL          // raisestate
	},

#ifdef SEENAMES
	{           // MT_NAMECHECK
		-1,             // doomednum
		S_NAMECHECK,    // spawnstate
		1000,           // spawnhealth
		S_NULL,         // seestate
		sfx_None,       // seesound
		8,              // reactiontime
		sfx_None,       // attacksound
		S_NULL,         // painstate
		0,              // painchance
		sfx_None,       // painsound
		S_NULL,         // meleestate
		S_NULL,         // missilestate
		S_DISS,         // deathstate
		S_NULL,         // xdeathstate
		sfx_None,       // deathsound
		60*FRACUNIT,    // speed
		30*FRACUNIT,    // radius
		40*FRACUNIT,    // height
		0,              // display offset
		100,            // mass
		0,              // damage
		sfx_None,       // activesound
		MF_NOBLOCKMAP|MF_MISSILE|MF_NOGRAVITY|MF_NOSECTOR, // flags
		S_NULL          // raisestate
	},
#endif
};


/** Patches the mobjinfo table and state table.
  * Free slots are emptied out and set to initial values.
  * If NEWTICRATERATIO is not 1, times are recomputed.
  */
void P_PatchInfoTables(void)
{
	INT32 i;
	char *tempname;

#if NUMSPRITEFREESLOTS > 1000
"Update P_PatchInfoTables, you big dumb head"
#endif

	// empty out free slots
	for (i = SPR_FIRSTFREESLOT; i <= SPR_LASTFREESLOT; i++)
	{
		tempname = sprnames[i];
		tempname[0] = 'F';
		tempname[1] = (char)('0' + (char)((i-SPR_FIRSTFREESLOT+1)/100));
		tempname[2] = (char)('0' + (char)(((i-SPR_FIRSTFREESLOT+1)/10)%10));
		tempname[3] = (char)('0' + (char)((i-SPR_FIRSTFREESLOT+1)%10));
		tempname[4] = '\0';
	}
	sprnames[i][0] = '\0'; // i == NUMSPRITES
	memset(&states[S_FIRSTFREESLOT], 0, sizeof (state_t) * NUMSTATEFREESLOTS);
	memset(&mobjinfo[MT_FIRSTFREESLOT], 0, sizeof (mobjinfo_t) * NUMMOBJFREESLOTS);
	for (i = MT_FIRSTFREESLOT; i <= MT_LASTFREESLOT; i++)
		mobjinfo[i].doomednum = -1;

#if NEWTICRATERATIO != 1
	for (i = 0; i < MT_FIRSTFREESLOT; i++)
	{
		// Check fields against size to ensure a better guess
		// that this field is used for momentum.
		if (mobjinfo[i].spawnhealth >= FRACUNIT)
			mobjinfo[i].spawnhealth *= NEWTICRATERATIO;

		if (mobjinfo[i].seestate >= FRACUNIT)
			mobjinfo[i].seestate *= NEWTICRATERATIO;

		if (mobjinfo[i].reactiontime >= FRACUNIT)
			mobjinfo[i].reactiontime *= NEWTICRATERATIO;

		if (mobjinfo[i].painstate >= FRACUNIT)
			mobjinfo[i].painstate *= NEWTICRATERATIO;

		if (mobjinfo[i].painchance >= FRACUNIT)
			mobjinfo[i].painchance *= NEWTICRATERATIO;

		if (mobjinfo[i].meleestate >= FRACUNIT)
			mobjinfo[i].meleestate *= NEWTICRATERATIO;

		if (mobjinfo[i].missilestate >= FRACUNIT)
			mobjinfo[i].missilestate *= NEWTICRATERATIO;

		if (mobjinfo[i].deathstate >= FRACUNIT)
			mobjinfo[i].deathstate *= NEWTICRATERATIO;

		if (mobjinfo[i].xdeathstate >= FRACUNIT)
			mobjinfo[i].xdeathstate *= NEWTICRATERATIO;

		// We really don't need to check this one...
		mobjinfo[i].speed *= NEWTICRATERATIO;

		if (mobjinfo[i].mass >= FRACUNIT)
			mobjinfo[i].mass *= NEWTICRATERATIO;

		if (mobjinfo[i].damage >= FRACUNIT)
			mobjinfo[i].damage *= NEWTICRATERATIO;

		if (mobjinfo[i].raisestate >= FRACUNIT)
			mobjinfo[i].raisestate *= NEWTICRATERATIO;
	}

	for (i = 0; i < NUMSTATES; i++)
		states[i].tics *= NEWTICRATERATIO;
#endif
}

void P_BackupTables(void)
{
	M_Memcpy(sprnamesbackup, sprnames, sizeof(sprnames));
	M_Memcpy(statesbackup, states, sizeof(states));
	M_Memcpy(mobjinfobackup, mobjinfo, sizeof(mobjinfo));
}

void P_ResetData(INT32 flags)
{
	if (flags & 1)
		M_Memcpy(sprnames, sprnamesbackup, sizeof(sprnamesbackup));

	if (flags & 2)
		M_Memcpy(states, statesbackup, sizeof(statesbackup));

	if (flags & 4)
		M_Memcpy(mobjinfo, mobjinfobackup, sizeof(mobjinfobackup));
}
