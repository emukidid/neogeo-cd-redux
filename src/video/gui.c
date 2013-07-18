/****************************************************************************
* NeoCD Redux
*
* GUI File Selector
****************************************************************************/

/*** SVN 
 * $LastChangedDate: 2007-03-27 13:54:15 +0100 (Tue, 27 Mar 2007) $
 * $LastChangedRevision: 50 $
 ***/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "neocdredux.h"
#include "backdrop.h"
#include "banner.h"

/*** GC 2D Video ***/
extern u32 *xfb[2];
extern u32 whichfb;
extern GXRModeObj *vmode;

/*** libOGC Default Font ***/
extern u8 console_font_8x16[];

static u32 fgcolour = COLOR_WHITE;
static u32 bgcolour = COLOR_BLACK;
static char background[1280 * 480] ATTRIBUTE_ALIGN (32);
static char bannerunc[banner_WIDTH * banner_HEIGHT * 2] ATTRIBUTE_ALIGN (32);
static void unpack (void);

/****************************************************************************
* plotpixel
****************************************************************************/
static void
plotpixel (int x, int y)
{
  u32 pixel;

  pixel = xfb[whichfb][(y * 320) + (x >> 1)];

  if (x & 1)
    xfb[whichfb][(y * 320) + (x >> 1)] =
      (pixel & 0xffff00ff) | (COLOR_WHITE & 0xff00);
  else
    xfb[whichfb][(y * 320) + (x >> 1)] =
      (COLOR_WHITE & 0xffff00ff) | (pixel & 0xff00);
}

/****************************************************************************
* roughcircle
****************************************************************************/
static void
roughcircle (int cx, int cy, int x, int y)
{
  if (x == 0)
    {
      plotpixel (cx, cy + y);		/*** Anti ***/
      plotpixel (cx, cy - y);
      plotpixel (cx + y, cy);
      plotpixel (cx - y, cy);
    }
  else
    {
      if (x == y)
	{
	  plotpixel (cx + x, cy + y);			/*** Anti ***/
	  plotpixel (cx - x, cy + y);
	  plotpixel (cx + x, cy - y);
	  plotpixel (cx - x, cy - y);
	}
      else
	{
	  if (x < y)
	    {
	      plotpixel (cx + x, cy + y);			/*** Anti ***/
	      plotpixel (cx - x, cy + y);
	      plotpixel (cx + x, cy - y);
	      plotpixel (cx - x, cy - y);
	      plotpixel (cx + y, cy + x);
	      plotpixel (cx - y, cy + x);
	      plotpixel (cx + y, cy - x);
	      plotpixel (cx - y, cy - x);
	    }

	}
    }
}

/****************************************************************************
* circle
****************************************************************************/
void
circle (int cx, int cy, int radius)
{
  int x = 0;
  int y = radius;
  int p = (5 - radius * 4) / 4;

  roughcircle (cx, cy, x, y);

  while (x < y)
    {
      x++;
      if (p < 0)
	p += (x << 1) + 1;
      else
	{
	  y--;
	  p += ((x - y) << 1) + 1;
	}
      roughcircle (cx, cy, x, y);
    }
}

/****************************************************************************
* drawchar
****************************************************************************/
static void
drawchar (int x, int y, char c)
{
  int yy, xx;
  u32 colour[2];
  int offset;
  u8 bits;

  offset = (y * 320) + (x >> 1);

  for (yy = 0; yy < 16; yy++)
    {
      bits = console_font_8x16[((c << 4) + yy)];

      for (xx = 0; xx < 4; xx++)
	{
	  if (bits & 0x80)
	    colour[0] = fgcolour;
	  else
	    colour[0] = bgcolour;

	  if (bits & 0x40)
	    colour[1] = fgcolour;
	  else
	    colour[1] = bgcolour;

	  xfb[whichfb][offset + xx] =
	    (colour[0] & 0xffff00ff) | (colour[1] & 0xff00);

	  bits <<= 2;
	}

      offset += 320;
    }
}

/****************************************************************************
* drawcharw
****************************************************************************/
static void
drawcharw (int x, int y, char c)
{
  int yy, xx;
  int offset;
  int bits;

  offset = (y * 320) + (x >> 1);

  for (yy = 0; yy < 16; yy++)
    {
      bits = console_font_8x16[((c << 4) + yy)];

      for (xx = 0; xx < 8; xx++)
	{
	  if (bits & 0x80)
	    xfb[whichfb][offset + xx] = xfb[whichfb][offset + 320 + xx] =
	      fgcolour;
	  else
	    xfb[whichfb][offset + xx] = xfb[whichfb][offset + 320 + xx] =
	      bgcolour;

	  bits <<= 1;
	}

      offset += 640;
    }
}

/****************************************************************************
* gprint
****************************************************************************/
void
gprint (int x, int y, char *text, int mode)
{
  int n;
  int i;

  n = strlen (text);
  if (!n)
    return;

  if (mode != TXT_DOUBLE)
    {
      for (i = 0; i < n; i++, x += 8)
	drawchar (x, y, text[i]);
    }
  else
    {
      for (i = 0; i < n; i++, x += 16)
	drawcharw (x, y, text[i]);
    }
}

/****************************************************************************
* DrawScreen
****************************************************************************/
void
DrawScreen (void)
{
  static int inited = 0;

  if (!inited)
    {
      unpack ();
      inited = 1;
    }

  VIDEO_WaitVSync ();

  whichfb ^= 1;
  memcpy (xfb[whichfb], background, 1280 * 480);

}

/****************************************************************************
* ShowScreen
****************************************************************************/
void
ShowScreen (void)
{
  VIDEO_SetNextFramebuffer (xfb[whichfb]);
  VIDEO_Flush ();
  VIDEO_WaitVSync ();
}

/****************************************************************************
* setfgcolour
****************************************************************************/
void
setfgcolour (u32 colour)
{
  fgcolour = colour;
}

/****************************************************************************
* setbgcolour
****************************************************************************/
void
setbgcolour (u32 colour)
{
  bgcolour = colour;
}

/****************************************************************************
* WaitButtonA
****************************************************************************/
void
WaitButtonA (void)
{
  while (!(PAD_ButtonsHeld (0) & PAD_BUTTON_A))
    VIDEO_WaitVSync ();

  while (PAD_ButtonsHeld (0) & PAD_BUTTON_A)
    VIDEO_WaitVSync ();
}

/****************************************************************************
* ActionScreen
****************************************************************************/
void
ActionScreen (char *msg)
{
  int n;
  char pressa[] = "Press A to continue";

  DrawScreen ();

  n = strlen (msg);
  fgcolour = COLOR_WHITE;
  bgcolour = BMPANE;

  gprint ((640 - (n * 16)) >> 1, 248, msg, TXT_DOUBLE);

  gprint (168, 288, pressa, TXT_DOUBLE);

  ShowScreen ();

  WaitButtonA ();
}

/****************************************************************************
* InfoScreen
****************************************************************************/
void
InfoScreen (char *msg)
{
  int n;

  DrawScreen ();

  n = strlen (msg);
  fgcolour = COLOR_WHITE;
  bgcolour = BMPANE;

  gprint ((640 - (n * 16)) >> 1, 264, msg, TXT_DOUBLE);

  ShowScreen ();
}

/****************************************************************************
* unpack
****************************************************************************/
static void
unpack (void)
{
  unsigned long res, inbytes, outbytes;

  inbytes = backdrop_COMPRESSED;
  outbytes = backdrop_RAW;

  res = uncompress (background, &outbytes, backdrop, inbytes);

  inbytes = banner_COMPRESSED;
  outbytes = banner_RAW;

  res = uncompress (bannerunc, &outbytes, banner, inbytes);
}

/****************************************************************************
* loadnewgame
****************************************************************************/
int
loadnewgame (void)
{
  char titles[3][20] = { {"Load new game?\0"}, {"A - Yes\0"}, {"B - No \0"} };
  int i;
  int quit = 0;

  while (PAD_ButtonsHeld (0))
    VIDEO_WaitVSync ();

  DrawScreen ();

  fgcolour = COLOR_WHITE;
  bgcolour = BMPANE;

  for (i = 0; i < 3; i++)
    gprint ((640 - (strlen (titles[i]) * 16)) >> 1, 232 + (i * 32), titles[i],
	    TXT_DOUBLE);

  ShowScreen ();

  while (!quit)
    {
      if (PAD_ButtonsHeld (0) & PAD_BUTTON_A)
	quit = 2;

      if (PAD_ButtonsHeld (0) & PAD_BUTTON_B)
	quit = 1;
    }

  while (PAD_ButtonsHeld (0))
    VIDEO_WaitVSync ();

  if (quit == 1)
    return 0;

  return 1;
}

#define MAXOPTS 8
static char options[8][22] = {  "Load New Game       ", 
				"Return to Game      ",
				"Region           USA",
				"SFX Volume       1.0",
				"MP3 Volume       1.0", 
				"Low Gain         1.0", 
				"Mid Gain         1.0",
				"High Gain        1.0" };
				
static float opts[5] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

/****************************************************************************
* draw_options
****************************************************************************/
static void draw_options(  int currsel )
{
	int i;
	int j = 158;
	char inverse[34];
		
	DrawScreen ();	
	
	for( i = 0; i < MAXOPTS; i++ )
	{
		if ( i == currsel )
		{
			setfgcolour (BMPANE);
			setbgcolour (INVTEXT);
			memset(inverse, 32, 34);
			inverse[32] = 0;
			memcpy(inverse + 6, options[i], strlen(options[i]));
			gprint( 64, j, inverse, TXT_DOUBLE);
			
		}
		else
		{
			setfgcolour (COLOR_WHITE);
			setbgcolour (BMPANE);
			gprint( ( 640 - ( strlen(options[i]) << 4 )) >> 1  ,
				j, options[i], TXT_DOUBLE);
		}
		j += 32;
	}
	
	ShowScreen();
}

/****************************************************************************
* load_options
****************************************************************************/				
int load_options( void )
{
	int currsel = 0;
	int quit = 0;
	int redraw = 1;
	u16 joy;
	int ret = 0;
	char buf[22];
	signed char x,y;
	
	while ( !quit )
	{
		if ( redraw )
		{
			draw_options( currsel );
			redraw = 0;
		}
		
		joy = PAD_ButtonsDown(0);
		x = PAD_StickX(0);
		y = PAD_StickY(0);
		
		if ( x > 70 )
			joy |= PAD_BUTTON_RIGHT;
		else
			if ( x < -70 )
				joy |= PAD_BUTTON_LEFT;
		
		if ( y > 70 )
			joy |= PAD_BUTTON_UP;
		else
			if ( y < -70 )
				joy |= PAD_BUTTON_DOWN;
		
		if ( joy & PAD_BUTTON_DOWN )
		{
			currsel++;
			if ( currsel >= MAXOPTS )
				currsel = 0;
			
			redraw = 1;
		}
		
		if ( joy & PAD_BUTTON_UP )
		{
			currsel--;
			if ( currsel < 0 )
				currsel = 0;
			
			redraw = 1;
		}
		
		if ( joy & PAD_BUTTON_A )
		{
			switch ( currsel )
			{
				case 0:	/*** Load new game ***/
					ret = quit = 1;
					break;
				
				case 1: /*** Return to game ***/
					ret = 0;
					quit = 1;
					break;
			
				case 2: /*** Region ***/
					neogeo_region++;
					if ( neogeo_region > 2 )
						neogeo_region = 0;
					switch ( neogeo_region )
					{
						case 0: strcpy(options[2], "Region         JAPAN");
							break;
						
						case 1: strcpy(options[2], "Region           USA");
							break;
						
						default:
							strcpy(options[2], "Region        EUROPE");
							break;
					}
					break;
					
				default:
					opts[currsel-3] += 0.1f;
					if ( opts[currsel-3] >= 2.0f )
						opts[currsel-3] = 2.0f;
					
					strcpy(buf, options[currsel]);
					buf[17]=0;
					sprintf(options[currsel],"%s%1.1f", buf, opts[currsel-3]);
					break;
			}
			
			redraw = 1;
		}
		
		if ( joy & PAD_BUTTON_B )
		{
			if ( currsel > 2 )
			{
				opts[currsel-3] -= 0.1f;
				if ( opts[currsel-3] < 0.0f )
					opts[currsel-3] = 0.0f;
				
				strcpy(buf, options[currsel]);
				buf[17]=0;
				sprintf(options[currsel],"%s%1.1f", buf, opts[currsel-3]);
				
				redraw = 1;
			}
		}
	}
	
	/*** Update before we go ***/
	mixer_set( opts[0], opts[1], opts[2], opts[3], opts[4]);
	
	/*** Clear joy buffer ***/
	while ( PAD_ButtonsHeld(0) )
		VIDEO_WaitVSync();
	
	return ret;
}


/****************************************************************************
* bannerscreen
****************************************************************************/
void
bannerscreen (void)
{
  int y, x, j;
  int offset;
  int *bb = (int *) bannerunc;

  whichfb ^= 1;
  offset = (200 * 320) + 40;
  VIDEO_ClearFrameBuffer (vmode, xfb[whichfb], COLOR_BLACK);

  for (y = 0, j = 0; y < banner_HEIGHT; y++)
    {
      for (x = 0; x < (banner_WIDTH >> 1); x++)
	xfb[whichfb][offset + x] = bb[j++];

      offset += 320;
    }

  VIDEO_SetNextFramebuffer (xfb[whichfb]);
  VIDEO_Flush ();
  VIDEO_WaitVSync ();
    
}
