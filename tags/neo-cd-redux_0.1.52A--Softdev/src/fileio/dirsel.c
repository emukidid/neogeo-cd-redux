/****************************************************************************
* NeoGeo Directory Selector
*
* As there is no 'ROM' to speak of, use the directory as the starting point
****************************************************************************/

/*** SVN
 * $LastChangedDate: 2007-03-19 08:08:55 +0000 (Mon, 19 Mar 2007) $
 * $LastChangedRevision: 41 $
 ***/
#include "sdfileio.h"
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "neocdredux.h"

#define PAGE_SIZE 8

char basedir[1024];
static char scratchdir[1024];
char dirbuffer[0x10000] ATTRIBUTE_ALIGN (32);

/****************************************************************************
* DirGetDirectory
****************************************************************************/
static int
DirGetDirectory (char *dir)
{
  int *p = (int *) dirbuffer;

  GetSubDirectories (dir, dirbuffer, 0x10000);
  return p[0];
}

/****************************************************************************
* DrawDirSelector
****************************************************************************/
static void
DrawDirSelector (int maxfile, int menupos, int currsel)
{
  int i;
  int j = 158;
  int *p = (int *) dirbuffer;
  char *m;
  char display[40];
  char inverse[40];

  DrawScreen ();

  for (i = menupos; i < (menupos + PAGE_SIZE) && (i < maxfile); i++)
    {
      m = (char *) p[i + 1];
      memset (display, 0, 40);
      memcpy (display, m, 32);

      if (i == currsel)
        {
          setfgcolour (BMPANE);
          setbgcolour (INVTEXT);
          memset (inverse, 32, 40);
          inverse[32] = 0;
          memcpy (inverse, display, strlen (display));
          gprint (64, j, inverse, TXT_DOUBLE);
        }
      else
        {
          setfgcolour (COLOR_WHITE);
          setbgcolour (BMPANE);
          gprint (64, j, display, TXT_DOUBLE);
        }

      j += 32;
    }

  ShowScreen ();
}

/****************************************************************************
* DirSelector
*
* A == Enter directory
* B == Parent directory
* X == Set directory
****************************************************************************/
static void
DirSelector (void)
{
  int *p = (int *) dirbuffer;
  char *m;
  int maxfile, i;
  int currsel = 0;
  int menupos = 0;
  int redraw = 1;
  unsigned short joy;
  int quit = 0;

  maxfile = p[0];

  while (!quit)
    {
      if (redraw)
        {
          DrawDirSelector (maxfile, menupos, currsel);
          redraw = 0;
        }

      joy = PAD_ButtonsDown (0);

      if (joy & PAD_BUTTON_DOWN)
        {
          currsel++;
          if (currsel == maxfile)
            currsel = menupos = 0;
          if ((currsel - menupos) >= PAGE_SIZE)
            menupos += PAGE_SIZE;

          redraw = 1;
        }

      if (joy & PAD_BUTTON_UP)
        {
          currsel--;
          if (currsel < 0)
            {
              currsel = maxfile - 1;
              menupos = currsel - PAGE_SIZE + 1;
            }

          if (currsel < menupos)
            menupos -= PAGE_SIZE;

          if (menupos < 0)
            menupos = 0;

          redraw = 1;
        }

      if (joy & PAD_BUTTON_A)
        {
          while (PAD_ButtonsHeld (0) & PAD_BUTTON_A)
            VIDEO_WaitVSync ();

          strcpy (scratchdir, basedir);

          if (scratchdir[strlen (scratchdir) - 1] != '/')
            strcat (scratchdir, "/");

          m = (char *) p[currsel + 1];

          strcat (scratchdir, m);

#ifdef USEDVD
          if (DirGetDirectory (scratchdir))
#else
          if ( gen_getdir(scratchdir) )
#endif
            {
              maxfile = p[0];
              currsel = menupos = 0;
              strcpy (basedir, scratchdir);
            }
          else
#ifdef USEDVD
            DirGetDirectory (basedir);
#else
            gen_getdir( basedir );
#endif

          redraw = 1;
        }

      if (joy & PAD_BUTTON_B)
        {
          while (PAD_ButtonsHeld (0) & PAD_BUTTON_B)
            VIDEO_WaitVSync ();

          if (strcmp (basedir, "/"))
            {
              strcpy (scratchdir, basedir);
              for (i = strlen (scratchdir) - 1; i >= 0; i--)
                {
                  if (scratchdir[i] == '/')
                    {
                      if (i == 0)
                        strcpy (scratchdir, "/");
                      else
                        scratchdir[i] = 0;

#ifdef USEDVD
                      if (DirGetDirectory (scratchdir))
#else
                      if (gen_getdir(scratchdir) )
#endif
                        {
                          maxfile = p[0];
                          currsel = menupos = 0;
                          strcpy (basedir, scratchdir);
                        }
                      break;
                    }
                }
            }
          redraw = 1;
        }

      if (joy & PAD_BUTTON_X)
        {
          /*** Set basedir to mount point ***/
          if (basedir[strlen (basedir) - 1] != '/')
            strcat (basedir, "/");

          m = (char *) p[currsel + 1];
          strcat (basedir, m);
          quit = 1;
        }
    }

  /*** Remove any pending buttons ***/
  joy = PAD_ButtonsHeld (0);
  while (joy)
    {
      VIDEO_WaitVSync ();
      joy = PAD_ButtonsHeld (0);
    }
}

/****************************************************************************
* DirMount
****************************************************************************/
int
DirMount (void)
{
  static int mounted = 0;

  memset (basedir, 0, 1024);
  memset (scratchdir, 0, 1024);

  /*** Mount the DVD ***/
  if (!mounted)
    {
      if (!mount_image ())
        return 0;

      mounted = 1;
    }

  /*** Always start at the root ***/
  strcpy (basedir, "/");

  /*** Build sub-directory tree ***/
  DirGetDirectory (basedir);

  /*** Do dir selection ***/
  DirSelector ();
  bannerscreen();

  return 1;
}

/****************************************************************************
* SD_Mount
****************************************************************************/
int SD_Mount( void )
{
  memset (basedir, 0, 1024);
  memset (scratchdir, 0, 1024);
  memset (dirbuffer, 0, 0x10000);

  strcpy(basedir,"/");

  if ( !gen_getdir(basedir) )
    return 0;

  DirSelector();
  bannerscreen();

  return 1;
}

