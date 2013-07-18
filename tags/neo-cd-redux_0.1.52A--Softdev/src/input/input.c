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
 * $LastChangedDate: 2007-03-14 00:46:07 +0000 (Wed, 14 Mar 2007) $
 * $LastChangedRevision: 36 $
 ***/

#include "neocdredux.h"

#define P1UP    0x00000001
#define P1DOWN  0x00000002
#define P1LEFT  0x00000004
#define P1RIGHT 0x00000008
#define P1A     0x00000010
#define P1B     0x00000020
#define P1C     0x00000040
#define P1D     0x00000080

#define P2UP    0x00000100
#define P2DOWN  0x00000200
#define P2LEFT  0x00000400
#define P2RIGHT 0x00000800
#define P2A     0x00001000
#define P2B     0x00002000
#define P2C     0x00004000
#define P2D     0x00008000

#define P1START 0x00010000
#define P1SEL   0x00020000
#define P2START 0x00040000
#define P2SEL   0x00080000

#define SPECIAL 0x01000000
static u32 keys = 0;
int padcal = 80;
int accept_input = 0;

unsigned short gcpadmap[] =
  { PAD_BUTTON_UP, PAD_BUTTON_DOWN, PAD_BUTTON_LEFT, PAD_BUTTON_RIGHT,
  PAD_BUTTON_A, PAD_BUTTON_B, PAD_BUTTON_X, PAD_BUTTON_Y
};
unsigned int neopadmap[] = { P1UP, P1DOWN, P1LEFT, P1RIGHT, P1A, P1B, P1C, P1D
};


/****************************************************************************
 * DecodeJoy
 ****************************************************************************/
unsigned int
DecodeJoy (unsigned short p)
{
  unsigned int J = 0;
  int i;
  for (i = 0; i < 8; i++)

    {
      if (p & gcpadmap[i])
	J |= neopadmap[i];
    }
  return J;
}


/****************************************************************************
 * GetAnalog
 ****************************************************************************/
unsigned int
GetAnalog (int Joy)
{
  signed char x, y;
  unsigned int i = 0;
  x = PAD_StickX (Joy);
  y = PAD_StickY (Joy);
  float t;

  if ((x * x + y * y) > (padcal * padcal))
    {
      if (x > 0 && y == 0)
	i |= P1RIGHT;
      if (x < 0 && y == 0)
	i |= P1LEFT;
      if (x == 0 && y > 0)
	i |= P1UP;
      if (x == 0 && y < 0)
	i |= P1DOWN;

      if (x != 0 && y != 0)
	{

      /*** Recalc left / right ***/
	  t = (float) y / x;
	  if (t >= -2.41421356237 && t < 2.41421356237)
	    {
	      if (x >= 0)
		i |= P1RIGHT;
	      else
		i |= P1LEFT;
	    }

	/*** Recalc up / down ***/
	  t = (float) x / y;
	  if (t >= -2.41421356237 && t < 2.41421356237)
	    {
	      if (y >= 0)
		i |= P1UP;
	      else
		i |= P1DOWN;
	    }
	}
    }

  return i;

}


/****************************************************************************
 * StartSel
 ****************************************************************************/
unsigned int
startsel (unsigned short p)
{
  int J = 0;
  if (p & PAD_BUTTON_START)
    J |= 1;
  if (p & PAD_TRIGGER_Z)
    J |= 2;
  return J;
}


/****************************************************************************
 * update_input
 ****************************************************************************/
void
update_input (void)
{
  unsigned short p;
  unsigned int t;

  if (!accept_input)
    return;

	/*** Do player one ***/
  p = PAD_ButtonsHeld (0);
  if (p & PAD_TRIGGER_L)
    {
      if (mcard_written)
	{
	  if (neogeo_set_memorycard ())
	    mcard_written = 0;
	}
    }

  if (p & PAD_TRIGGER_R)
    neogeo_new_game ();

  keys = DecodeJoy (p);
  keys |= GetAnalog (0);
  t = startsel (p);
  keys |= (t << 16);

	/*** Do player two ***/
  p = PAD_ButtonsHeld (1);
  keys |= (DecodeJoy (p) << 8);
  keys |= (GetAnalog (1) << 8);
  t = startsel (p);
  keys |= (t << 18);
}

/*--------------------------------------------------------------------------*/
unsigned char
read_player1 (void)
{
  return ~keys & 0xff;
}


/*--------------------------------------------------------------------------*/
unsigned char
read_player2 (void)
{
  return ~(keys >> 8) & 0xff;
}


/*--------------------------------------------------------------------------*/
unsigned char
read_pl12_startsel (void)
{
  return ~(keys >> 16) & 0x0f;
}
