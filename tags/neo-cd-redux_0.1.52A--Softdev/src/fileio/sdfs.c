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

/****************************************************************************
* SD FileIO
*
* Uses old 8.3 filenames only
****************************************************************************/
#include "sdfileio.h"
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ogc/mutex.h>
#include "neocdredux.h"

/* Generic File I/O */
#define MAXFILES	32
static FIL *sdfsfiles[MAXFILES];
static GENHANDLER sdhandler;
static u32 sdmutex = 0;

char msg[128];

/****************************************************************************
* SDFindFree
****************************************************************************/
int SDFindFree( void )
{
  int i;

  for( i = 0; i < MAXFILES; i++ )
    {
      if ( sdfsfiles[i] == NULL )
        return i;
    }

  return -1;
}

/****************************************************************************
* SDfopen
****************************************************************************/
static u32
SDfopen (const char *filename, const char *mode)
{
  char tname[1024];
  int i,handle;
  int res;

  /* No writing allowed */
  if ( strstr(mode,"w") )
    {
      return 0;
    }

  for( i = 0; i < strlen(filename); i++ )
    tname[i] = toupper(filename[i]);

  tname[i] = 0;

  /* Open for reading */
  handle = SDFindFree();
  if ( handle == -1 )
    {
      sprintf(msg,"OUT OF HANDLES!");
      ActionScreen(msg);
      return 0;
    }

  while ( LWP_MutexLock( sdmutex ) );

  sprintf(msg,"Opening : %s", tname);
  //ActionScreen(msg);

  sdfsfiles[handle] = (FIL *)malloc(sizeof(FIL));
  res = f_open(sdfsfiles[handle], tname, FA_READ);
  if ( res != FR_OK )
    {
      free(sdfsfiles[handle]);
      sdfsfiles[handle] = NULL;
      LWP_MutexUnlock( sdmutex );
      sprintf(msg, "No OPEN : %s", tname);
      //ActionScreen(msg);
      return 0;
    }

  LWP_MutexUnlock( sdmutex );
  return handle | 0x8000;
}

/****************************************************************************
* SDfclose
****************************************************************************/
static int
SDfclose (u32 fp)
{
  while ( LWP_MutexLock( sdmutex ) );

  if( sdfsfiles[fp & 0x7FFF] != NULL )
    {
      free( sdfsfiles[fp & 0x7FFF] );
      sdfsfiles[fp & 0x7FFF] = NULL;
      LWP_MutexUnlock( sdmutex );
      return 1;
    }

  LWP_MutexUnlock( sdmutex );
  return 0;
}

/****************************************************************************
* SDfread
****************************************************************************/
static u32
SDfread (char *buf, int block, int len, u32 fp)
{
  int handle = ( fp & 0x7FFF );
  int res;
  int actuallen;
  int i, blocks;
  u32 bytesdone;
  u16 rb;

  if( sdfsfiles[handle] == NULL )
    return 0;

  actuallen = block * len;

  if ( !actuallen )
    return 0;

  while ( LWP_MutexLock ( sdmutex ) );

  sprintf(msg, "Requested %d", actuallen);
  //ActionScreen(msg);

  blocks = actuallen >> 15;
  bytesdone = 0;
  for( i = 0; i < blocks; i++ )
    {
      res = f_read(sdfsfiles[handle], buf + bytesdone, 32768, &rb);
      if ( res || ( rb != 32768 ) )
        {
          if ( res == FR_OK )
            bytesdone += rb;

          sprintf(msg,"Actual ReadB %d", bytesdone);
          //ActionScreen(msg);
          LWP_MutexUnlock( sdmutex );
          return bytesdone;
        }
      bytesdone += 32768;
    }

  blocks = actuallen & 0x7FFF;
  if ( blocks )
    {
      res = f_read(sdfsfiles[handle], buf + bytesdone, blocks, &rb);
      if ( res || ( rb != blocks ) )
        {
          if( res == FR_OK )
            bytesdone += rb;

          sprintf(msg,"Actual ReadS %d", bytesdone);
          //ActionScreen(msg);

          LWP_MutexUnlock( sdmutex );
          return bytesdone;
        }
      bytesdone += blocks;
    }

  LWP_MutexUnlock( sdmutex );

  return bytesdone;

}

/****************************************************************************
* SDSeek
****************************************************************************/
static int
SDfseek (u32 fp, int where, int whence)
{
  int handle = ( fp & 0x7FFF );

  if ( sdfsfiles[handle] == NULL )
    {
      sprintf(msg,"SEEK : Invalid Handle %d", handle);
      ActionScreen(msg);
      return -1;
    }

  switch( whence )
    {
    case SEEK_SET:
      if ( where < 0 )
        return 0;

      sdfsfiles[handle]->fptr = where;
      return 1;

    case SEEK_CUR:
      if ( ( where + sdfsfiles[handle]->fptr ) > sdfsfiles[handle]->fsize )
        return 0;

      sdfsfiles[handle]->fptr += where;
      return 1;

    case SEEK_END:
      if ( where > 0 )
        return 0;

      sdfsfiles[handle]->fptr = sdfsfiles[handle]->fsize;
      return 1;
    }

  return 0;

}

/****************************************************************************
* SDftell
*
* Return current position
****************************************************************************/
static int
SDftell (u32 fp)
{
  int handle = ( fp & 0x7FFF );

  if ( sdfsfiles[handle] != NULL )
    return sdfsfiles[handle]->fptr;

  sprintf(msg,"Invalid handle FTELL : %d",handle);
  ActionScreen(msg);

  return 0;
}

static void
SDfcloseall (void)
{
  int i;

  while ( LWP_MutexLock( sdmutex ) );

  for( i = 0; i < MAXFILES; i++ )
    {
      if ( sdfsfiles[i] != NULL )
        f_close(sdfsfiles[i]);
    }

  LWP_MutexUnlock( sdmutex );
}

void
SD_SetHandler (void)
{
  SDInit();

  /* Clear */
  memset(&sdhandler, 0, sizeof(GENHANDLER));
  memset(&sdfsfiles, 0, MAXFILES * 4);

  sdhandler.gen_fopen = SDfopen;
  sdhandler.gen_fclose = SDfclose;
  sdhandler.gen_fread = SDfread;
  sdhandler.gen_fseek = SDfseek;
  sdhandler.gen_ftell = SDftell;
  sdhandler.gen_fcloseall = SDfcloseall;
  GEN_SetHandler (&sdhandler);

  /* Set mutex */
  LWP_MutexInit(&sdmutex, FALSE);

}


