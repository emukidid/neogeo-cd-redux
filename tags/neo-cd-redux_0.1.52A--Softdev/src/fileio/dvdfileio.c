/****************************************************************************
*   NeoCD Redux 0.1
*   NeoGeo CD Emulator
*   Copyright (C) 2007 softdev
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License along
*   with this program; if not, write to the Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
****************************************************************************/

/*** SVN
 * $LastChangedDate: 2007-04-08 05:35:23 +0100 (Sun, 08 Apr 2007) $
 * $LastChangedRevision: 51 $
 ***/

/****************************************************************************
* DVD File I/O
****************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "neocdredux.h"

/*** Generic file system ***/
#define MAXFILES 32
#define BUFFSIZE 0x8000
GENFILEINFO fileinfo[MAXFILES];
static GENHANDLER dvdhandler;
static u8 dvdfixup[BUFFSIZE + 0x20] ATTRIBUTE_ALIGN (32);

/****************************************************************************
* DVDFindFree
****************************************************************************/
static int
DVDFindFree (void)
{
  int i = 0;

  while (i < MAXFILES)
    {
      if (fileinfo[i].handle == -1)
        return i;
      i++;
    }

  return -1;
}

/****************************************************************************
* DVD_fopen
*
* Open a file on the DVD. This will only apply to the current DVD directory.
****************************************************************************/
static u32
DVDfopen (const char *filename, const char *mode)
{
  DIRENTRY *fdir;
  int handle;

  /*** No write mode available ***/
  if (strstr (mode, "w"))
    return 0;

  handle = DVDFindFree ();
  if (handle == -1)
    return 0;

  fdir = FindFile ((char *)filename);

  if (fdir == NULL)
    {
      return 0;
    }

  /*** Make a copy in the file info structure ***/
  fileinfo[handle].handle = handle;
  fileinfo[handle].mode = GEN_MODE_READ;
  fileinfo[handle].length = fdir->lengthbe;
  fileinfo[handle].offset_on_media64 = (long long int)fdir->offsetbe * 2048;
  fileinfo[handle].currpos = 0;

  return handle | 0x8000;
}

/****************************************************************************
* DVD_fclose
*
* Close a previously opened file
****************************************************************************/
static int
DVDfclose (u32 fp)
{
  if (fileinfo[fp & 0x7fff].handle != -1)
    {
      fileinfo[fp & 0x7fff].handle = -1;
      return 1;
    }
  return 0;
}

/****************************************************************************
* DVDfread
****************************************************************************/
static u32
DVDfread (char *buf, int block, int len, u32 fp)
{
  int handle = fp & 0x7fff;
  int bytesrequested = 0;
  int bytesavailable = 0;
  int bytestoread = 0;
  int bytesdone = 0;
  int blocks = 0;
  int i, remain;

  /*** Is this handle valid? ***/
  if (fileinfo[handle].handle == -1)
    return 0;

  bytesrequested = (block * len);
  bytesavailable = fileinfo[handle].length - fileinfo[handle].currpos;

  if (bytesrequested <= bytesavailable)
    bytestoread = bytesrequested;
  else
    bytestoread = bytesavailable;

  if (bytestoread == 0)
    return 0;

  memset (dvdfixup, 0, BUFFSIZE);

  /*** How many blocks ***/
  blocks = bytestoread / BUFFSIZE;

  if (blocks)
    {
      for (i = 0; i < blocks; i++)
        {
          gcsim64 (dvdfixup,
                   fileinfo[handle].offset_on_media64 + (long long int)fileinfo[handle].currpos,
                   BUFFSIZE);
          memcpy (buf + bytesdone, dvdfixup, BUFFSIZE);
          fileinfo[handle].currpos += BUFFSIZE;
          bytesdone += BUFFSIZE;
        }
    }

  remain = (bytestoread & (BUFFSIZE - 1));
  /*** Get remaining bytes ***/
  if (remain)
    {
      gcsim64 (dvdfixup,
               fileinfo[handle].offset_on_media64 + (long long int)fileinfo[handle].currpos,
               (remain & ~0x1f) + 32);
      memcpy (buf + bytesdone, dvdfixup, remain);
      fileinfo[handle].currpos += remain;
      bytesdone += remain;
    }

  return bytesdone;
}

/****************************************************************************
* DVD_fseek
*
* Seek to a position in a file
****************************************************************************/
static int
DVDfseek (u32 fp, int where, int whence)
{
  int handle;

  handle = fp & 0x7fff;

  if (fileinfo[handle].handle == -1)
    return -1;

  switch (whence)
    {
    case SEEK_END:
      if (where > 0)
        return 0;				/*** Fail ***/

      fileinfo[handle].currpos = fileinfo[handle].length + where;
      return 1;

    case SEEK_CUR:
      if ((where + fileinfo[handle].currpos) > fileinfo[handle].length)
        return 0;

      fileinfo[handle].currpos += where;
      return 1;

    case SEEK_SET:
      if (where < 0)
        return 0;

      fileinfo[handle].currpos = where;
      return 1;
    }

  return 0;
}

/****************************************************************************
* DVD_ftell
*
* Return current position
****************************************************************************/
static int
DVDftell (u32 fp)
{
  int handle = fp & 0x7fff;

  if (fileinfo[handle].handle != -1)
    {
      return fileinfo[handle].currpos;
    }

  return 0;
}

/****************************************************************************
* DVDfcloseall
****************************************************************************/
static void
DVDfcloseall (void)
{
  memset (&fileinfo, 0xff, sizeof (GENFILEINFO) * MAXFILES);
}

/****************************************************************************
* DVD_SetHandler
****************************************************************************/
void
DVD_SetHandler (void)
{
  /*** Clear ***/
  memset (&dvdhandler, 0, sizeof (GENHANDLER));
  memset (&fileinfo, 0xff, sizeof (GENFILEINFO) * MAXFILES);

  /*** Set generic handlers ***/
  dvdhandler.gen_fopen = DVDfopen;
  dvdhandler.gen_fclose = DVDfclose;
  dvdhandler.gen_fread = DVDfread;
  dvdhandler.gen_fseek = DVDfseek;
  dvdhandler.gen_ftell = DVDftell;
  dvdhandler.gen_fcloseall = DVDfcloseall;
  GEN_SetHandler (&dvdhandler);

}