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
/// \brief Command line arguments

#include <string.h>

#include "doomdef.h"
#include "command.h"
#include "m_argv.h"

/**	\brief number of arg
*/
INT32 myargc;

/**	\brief string table
*/
char **myargv;

/**	\brief founded the parm
*/
static INT32 found;


/**	\brief	The M_CheckParm function

	\param	check Checks for the given parameter in the program's command line arguments.

	\return	number (1 to argc-1) or 0 if not present


*/
INT32 M_CheckParm(const char *check)
{
	INT32 i;

	for (i = 1; i < myargc; i++)
	{
		if (!strcasecmp(check, myargv[i]))
		{
			found = i;
			return i;
		}
	}
	found = 0;
	return 0;
}

/**	\brief the M_IsNextParm

  \return  true if there is an available next parameter
*/

boolean M_IsNextParm(void)
{
	if (found > 0 && found + 1 < myargc && myargv[found+1][0] != '-' && myargv[found+1][0] != '+')
		return true;
	return false;
}

/**	\brief the M_GetNextParm function

  \return the next parameter after a M_CheckParm, NULL if not found
	use M_IsNextParm to verify there is a parameter
*/
const char *M_GetNextParm(void)
{
	if (M_IsNextParm())
	{
		found++;
		return myargv[found];
	}
	return NULL;
}

/**	\brief the  M_PushSpecialParameters function
	push all parameters beginning with '+'
*/
void M_PushSpecialParameters(void)
{
	INT32 i;
	char s[256];
	boolean onetime = false;

	for (i = 1; i < myargc; i++)
	{
		if (myargv[i][0] == '+')
		{
			strcpy(s, &myargv[i][1]);
			i++;

			// get the parameters of the command too
			for (; i < myargc && myargv[i][0] != '+' && myargv[i][0] != '-'; i++)
			{
				strcat(s, " ");
				if (!onetime)
				{
					strcat(s, "\"");
					onetime = true;
				}
				strcat(s, myargv[i]);
			}
			if (onetime)
			{
				strcat(s, "\"");
				onetime = false;
			}
			strcat(s, "\n");

			// push it
			COM_BufAddText(s);
			i--;
		}
	}
}

/// \brief max args

#if defined (_arch_dreamcast) || defined (_XBOX)
#define MAXARGVS 1
#else
#define MAXARGVS 256
#endif

/**	\brief the M_FindResponseFile function
	Find a response file
*/
void M_FindResponseFile(void)
{
	INT32 i;

	for (i = 1; i < myargc; i++)
		if (myargv[i][0] == '@')
		{
			FILE *handle;
			INT32 k, pindex, indexinfile;
			long size;
			boolean inquote = false;
			UINT8 *infile;
			char *file;
			char *moreargs[20];
			char *firstargv;

			// read the response file into memory
			handle = fopen(&myargv[i][1], "rb");
			if (!handle)
				I_Error("Response file %s not found", &myargv[i][1]);

			CONS_Printf("Found response file %s\n", &myargv[i][1]);
			fseek(handle, 0, SEEK_END);
			size = ftell(handle);
			fseek(handle, 0, SEEK_SET);
			file = malloc(size);
			if (!file)
				I_Error("No more free memory for the respone file");
			if (fread(file, size, 1, handle) != 1)
				I_Error("Couldn't read respone file because %s", strerror(ferror(handle)));
			fclose(handle);

			// keep all the command line arguments following @responsefile
			for (pindex = 0, k = i+1; k < myargc; k++)
				moreargs[pindex++] = myargv[k];

			firstargv = myargv[0];
			myargv = malloc(sizeof (char *) * MAXARGVS);
			if (!myargv)
			{
				free(file);
				I_Error("Not enough memory to read response file");
			}
			memset(myargv, 0, sizeof (char *) * MAXARGVS);
			myargv[0] = firstargv;

			infile = (UINT8 *)file;
			indexinfile = k = 0;
			indexinfile++; // skip past argv[0]
			do
			{
				inquote = infile[k] == '"';
				if (inquote) // strip enclosing double-quote
					k++;
				myargv[indexinfile++] = (char *)&infile[k];
				while (k < size && ((inquote && infile[k] != '"')
					|| (!inquote && infile[k] > ' ')))
				{
					k++;
				}
				infile[k] = 0;
				while (k < size && (infile[k] <= ' '))
					k++;
			} while (k < size);

			free(file);

			for (k = 0; k < pindex; k++)
				myargv[indexinfile++] = moreargs[k];
			myargc = indexinfile;

			// display arguments
			CONS_Printf("%d command-line args:\n", myargc);
			for (k = 1; k < myargc; k++)
				CONS_Printf("%s\n", myargv[k]);

			break;
		}
}
