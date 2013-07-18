/****************************************************************************
*   NeoCD Redux 0.1
*   NeoGeo CD Emulator
*   Copyright (C) 2007 softdev
****************************************************************************/

//-- Include files -----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "neocdredux.h"

//-- Private Variables -------------------------------------------------------
//static int cdda_min_track;
//static int cdda_max_track;
static int cdda_track_end;
static int cdda_loop_counter;

//-- Public Variables --------------------------------------------------------
int cdda_first_drive = 0;
int cdda_current_drive = 0;
int cdda_current_track = 0;
int cdda_current_frame = 0;
int cdda_playing = 0;
int cdda_autoloop = 0;
int cdda_volume = 0;
int cdda_disabled = 1;

//-- Function Prototypes -----------------------------------------------------
int cdda_init(void);
int cdda_play(int);
void cdda_stop(void);
void cdda_resume(void);
void cdda_shutdown(void);
void cdda_loop_check(void);
int cdda_get_disk_info(void);

//----------------------------------------------------------------------------
int cdda_init(void)
{
  return 1;
}

//----------------------------------------------------------------------------
int cdda_get_disk_info(void)
{
  return 1;
}

//----------------------------------------------------------------------------
int cdda_play(int track)
{
  if (cdda_disabled)
    return 1;
  if (cdda_playing && cdda_current_track == track)
    return 1;

  cdda_current_track = track;
  cdda_loop_counter = 0;
  cdda_playing = 1;
  cdda_track_end = 2000000;

  return 1;
}

//----------------------------------------------------------------------------
void cdda_pause(void)
{
  if (cdda_disabled)
    return;

  cdda_playing = 0;
}


void cdda_stop(void)
{
  if (cdda_disabled)
    return;

  cdda_playing = 0;
}

//----------------------------------------------------------------------------
void cdda_resume(void)
{
  if (cdda_disabled || cdda_playing)
    return;

  cdda_playing = 1;
}

//----------------------------------------------------------------------------
void cdda_shutdown(void)
{
  if (cdda_disabled)
    return;
}

//----------------------------------------------------------------------------
void cdda_loop_check(void)
{
  if (cdda_disabled)
    return;
  if (cdda_playing == 1)
    {
      cdda_loop_counter++;

      if (cdda_autoloop)
        cdda_play(cdda_current_track);
      else
        cdda_stop();
    }
}
