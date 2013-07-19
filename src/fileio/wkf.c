/**
 * Wiikey Fusion Driver for Gamecube & Wii
 *
 * Written by emu_kidid
**/

#include <stdio.h>
#include <gccore.h>		/*** Wrapper to include common libogc headers ***/
#include <ogcsys.h>		/*** Needed for console support ***/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogc/exi.h>

#include <ogc/disc_io.h>

#include <fat.h>
#include <unistd.h>
#include <malloc.h>
#include <ogc/dvd.h>
#include <sys/dir.h>
#include <ogc/machine/processor.h>
#include <sdcard/gcsd.h>

#include "wkf.h"

#define WKF_BUF_SIZE 0x8000
u8 wkfBuffer[WKF_BUF_SIZE] ATTRIBUTE_ALIGN (32);    // One DVD Sector

static int wkfInitialized = 0;
static volatile unsigned int* const wkf = (unsigned int*)0xCC006000;
static volatile unsigned int* const pireg = (unsigned int*)0xCC003000;

extern u8 udelay();


void __wkfReset() {
	u32 val;
	val = pireg[9];
	pireg[9] = ((val&~4)|1);
	usleep(12);
	val |= 1 | 4;
	pireg[9] = val;
}

unsigned int __wkfCmdImm(unsigned int cmd, unsigned int p1, unsigned int p2) {
	wkf[2] = cmd;
	wkf[3] = p1;
	wkf[4] = p2;
	wkf[6] = 0;
	wkf[8] = 0;
	wkf[7] = 1;
	int retries = 1000000;
	while(( wkf[7] & 1) && retries) {
		retries --;						// Wait for IMM command to finish up
	}
	return !retries ? -1 : wkf[8];
}

// ok
unsigned int __wkfSpiReadId() {
	return __wkfCmdImm(0xDF000000, 0xE0009F00, 0x00000000);
}

unsigned char __wkfSpiReadUC(unsigned int addr) {
	u32 ret = __wkfCmdImm(0xDF000000, 0x3C000300 | ((addr>>16)&0xFF), (((addr>>8)&0xFF) << 24) | (((addr)&0xFF)<<16));
	return (u8)(ret&0xFF);
}

// Reads DVD sectors (returns 0 on success)
void __wkfReadSectors(void* dst, unsigned int len, u64 offset) {
	wkf[2] = 0xA8000000;
	wkf[3] = (u32)(offset >> 2);
	wkf[4] = len;
	wkf[5] = (u32)dst;
	wkf[6] = len;
	wkf[7] = 3; // DMA | START
	DCInvalidateRange(dst, len);
	while (wkf[7] & 1);
}

int wkfSpiRead(unsigned char *buf, unsigned int addr, int len) {
	int i;
	for (i=0; i<len; i++) {
		buf[i]  = __wkfSpiReadUC(addr+i);
//		if((i%4096)==0)
	}
	return 0;
}


// Not sure what this does.
unsigned int wkfReadSpecial3() {
	return __wkfCmdImm(0xDF000000,	0x00030000, 0x00000000);
}

unsigned int wkfReadSwitches() {
	return ((__wkfCmdImm(0xDF000000,	0x00030000, 0x00000000) >> 17) & 3);
}


// Write to the WKF RAM (0xFFFF is the max offset)
void wkfWriteRam(int offset, int data) {
	__wkfCmdImm(0xDD000000,	offset, data);
}

// Write the WKF base offset to base all reads from
void wkfWriteOffset(int offset) {
	__wkfCmdImm(0xDE000000, offset, 0x5A000000);
}

// Returns SD slot status
unsigned int wkfGetSlotStatus() {
	return __wkfCmdImm(0xDF000000, 0x00010000, 0x00000000);
}

void wkfRead(void* dst, int len, u64 offset)
{
	u8 *sector_buffer = &wkfBuffer[0];
	while (len)
	{
		__wkfReadSectors(sector_buffer, WKF_BUF_SIZE, (offset-(offset%WKF_BUF_SIZE)));
		u32 off = (u32)((u32)(offset) & (WKF_BUF_SIZE-1));

		int rl = WKF_BUF_SIZE - off;
		if (rl > len)
			rl = len;
		memcpy(dst, sector_buffer + off, rl);	

		offset += rl;
		len -= rl;
		dst += rl;
	}
}

void wkfInit() {

	// New DVD (aka put it back to WKF mode)
	__wkfReset();
	
	// one chunk at 0, offset 0
	wkfWriteRam(0, 0x0000);
	wkfWriteRam(2, 0x0000);
	// last chunk sig
	wkfWriteRam(4, 0xFFFF);
	wkfWriteOffset(0);
	
	// SD card detect
	if ((wkfGetSlotStatus() & 0x000F0000)==0x00070000) {
//		DrawFrameStart();
                if (use_WKF == 1) ActionScreen("No WKF SD Card");
//		WriteFont(25+(0.60*116)+10,215, "No WKF SD Card");
//		DrawFrameFinish();
//		sleep(3);
//		// no SD card
//		wkfInitialized = 0;
	}
	else {
		// If there's an SD, reset DVD (why?)
//                WaitPrompt("WKF SD Card Found");
//		WriteFont(25+(0.60*116)+10,215, "WKF SD Card Found");
		__wkfReset();
		udelay(300000);
					
		// one chunk at 0, offset 0
		wkfWriteRam(0, 0x0000);
		wkfWriteRam(2, 0x0000);
		// last chunk sig
		wkfWriteRam(4, 0xFFFF);
		wkfWriteOffset(0);

		// Read first sector of SD card
		wkfRead(&wkfBuffer[0], 0x200, 0);
		if((wkfBuffer[0x1FF] != 0xAA)) {
			// No FAT!
//			DrawFrameStart();
                        if (use_WKF == 1) ActionScreen("No FAT Formatted WKF SD card found!!");
			//WriteFont(25+(0.60*116)+10,240, "No FAT Formatted SD found in Wiikey Fusion!");
			//DrawFrameFinish();
			//sleep(5);
			wkfInitialized = 0;
		} 
		else {
			wkfInitialized = 1;
                        if (use_WKF == 1) InfoScreen("Searching WKF SD Card");
			//WriteFont(25+(0.60*116)+10,240, "Searching WKF SD Card");
		}
	}
}

void wkfReinit() {
	__wkfReset();
	udelay(300000);

	// one chunk at 0, offset 0
	wkfWriteRam(0, 0x0000);
	wkfWriteRam(2, 0x0000);
	// last chunk sig
	wkfWriteRam(4, 0xFFFF);
	wkfWriteOffset(0);
	
	// Read first sector of SD card
	wkfRead(&wkfBuffer[0], 0x200, 0);
}

// Wrapper to read a number of sectors
// 0 on Success, -1 on Error
int wkfReadSectors(int chn, u32 sector, unsigned int numSectors, unsigned char *dest) 
{
	// This is confusing as we're reading 512b sectors from a device that can only address 2048b sectors
	wkfRead(dest, numSectors * 512, (u64)((u64)sector * 512));
	return 0;
}

// Is an SD Card inserted into the Wiikey Fusion?
bool wkfIsInserted(int chn) {
	if(!wkfInitialized) {
		wkfInit();
		if(!wkfInitialized) {
			return false;
		}
	}
	return true;
}

int wkfShutdown(int chn) {
	return 1;
}

static bool __wkf_startup(void)
{
	return wkfIsInserted(0);
}

static bool __wkf_isInserted(void)
{
	return wkfIsInserted(0);
}

static bool __wkf_readSectors(u32 sector, u32 numSectors, void *buffer)
{
	wkfReadSectors(0, sector, numSectors, buffer);
	return true;
}

static bool __wkf_writeSectors(u32 sector, u32 numSectors, void *buffer)
{
	return false;
}

static bool __wkf_clearStatus(void)
{
	return true;
}

static bool __wkf_shutdown(void)
{
	return true;
}



const DISC_INTERFACE __io_wkf = {
	DEVICE_TYPE_GC_WKF,
	FEATURE_MEDIUM_CANREAD | FEATURE_GAMECUBE_DVD,
	(FN_MEDIUM_STARTUP)&__wkf_startup,
	(FN_MEDIUM_ISINSERTED)&__wkf_isInserted,
	(FN_MEDIUM_READSECTORS)&__wkf_readSectors,
	(FN_MEDIUM_WRITESECTORS)&__wkf_writeSectors,
	(FN_MEDIUM_CLEARSTATUS)&__wkf_clearStatus,
	(FN_MEDIUM_SHUTDOWN)&__wkf_shutdown
} ;

const DISC_INTERFACE* WKF_slot = &__io_wkf;


/////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <ogc/mutex.h>

#include "fileio.h"
#include <dirent.h>


/* Generic File I/O */
#define MAXFILES	32
static FILE *wkf_files[MAXFILES];
static GENHANDLER wkfhandler;
static u32 wkf_mutex = 0;

#define MAXDIRENTRIES 0x4000
static char *direntries[MAXDIRENTRIES];

char msg[128];

/****************************************************************************
* WKF FindFree
****************************************************************************/
int WKF_FindFree( void )
{
  int i;

  for( i = 0; i < MAXFILES; i++ )
    {
      if ( wkf_files[i] == NULL )
        return i;
    }

  return -1;
}

/****************************************************************************
* WKF fopen
****************************************************************************/
static u32 WKF_fopen (const char *filename, const char *mode)
{
  /* No writing allowed */
  if ( strstr(mode,"w") )
    {
      return 0;
    }

  /* Open for reading */
  int handle = WKF_FindFree();
  if ( handle == -1 )
    {
      sprintf(msg,"OUT OF HANDLES!");
      ActionScreen(msg);
      return 0;
    }

  while ( LWP_MutexLock( wkf_mutex ) );


  wkf_files[handle] =  fopen(filename, mode);
  if ( wkf_files[handle] == NULL )
    {
      LWP_MutexUnlock( wkf_mutex );
      return 0;
    }

  LWP_MutexUnlock( wkf_mutex );
  return handle | 0x8000;
}

/****************************************************************************
* WKF fclose
****************************************************************************/
static int WKF_fclose (u32 fp)
{
  while ( LWP_MutexLock( wkf_mutex ) );

  if( wkf_files[fp & 0x7FFF] != NULL )
    {
      fclose(wkf_files[fp & 0x7FFF]);
      wkf_files[fp & 0x7FFF] = NULL;
      LWP_MutexUnlock( wkf_mutex );
      return 1;
    }

  LWP_MutexUnlock( wkf_mutex );
  return 0;
}

/****************************************************************************
* WKF fread
****************************************************************************/
static u32 WKF_fread (char *buf, int block, int len, u32 fp)
{
  int handle = ( fp & 0x7FFF );

  if( wkf_files[handle] == NULL )
    return 0;

  while ( LWP_MutexLock ( wkf_mutex ) );

  int bytesdone = fread(buf, block, len, wkf_files[handle]);

  LWP_MutexUnlock( wkf_mutex );

  return bytesdone;

}

/****************************************************************************
* WKF Seek
****************************************************************************/
static int WKF_fseek (u32 fp, int where, int whence)
{
  int handle = ( fp & 0x7FFF );

  if ( wkf_files[handle] == NULL )
    {
      sprintf(msg,"SEEK : Invalid Handle %d", handle);
      ActionScreen(msg);
      return -1;
    }

  return fseek(wkf_files[handle], where, whence );

}

/****************************************************************************
 * WKF ftell
 ****************************************************************************/
static int WKF_ftell (u32 fp)
{
  int handle = ( fp & 0x7FFF );

  if ( wkf_files[handle] == NULL )
    {
      sprintf(msg,"FTELL : Invalid Handle %d", handle);
      ActionScreen(msg);
      return -1;
    }

  return ftell(wkf_files[handle]);
}
/****************************************************************************
 * WKF fclose all
 ****************************************************************************/
static void WKF_fcloseall (void)
{
  int i;

  while ( LWP_MutexLock( wkf_mutex ) );

  for( i = 0; i < MAXFILES; i++ )
    {
      if ( wkf_files[i] != NULL )
        fclose(wkf_files[i]);
    }

  LWP_MutexUnlock( wkf_mutex );
}
/****************************************************************************
 * SORT
 ****************************************************************************/
void WKF_SortListing( int max )
{
  int top,seek;
  char *t;

  for( top = 0; top < max - 1; top++ )
    {
      for( seek = top + 1; seek < max; seek++ )
        {
          if ( stricmp(direntries[top], direntries[seek]) > 0 )
            {
              t = direntries[top];
              direntries[top] = direntries[seek];
              direntries[seek] = t;
            }
        }
    }
}

/****************************************************************************
 * WKF getdir
 ****************************************************************************/
static int WKF_getdir( char *thisdir )
{
  DIR *dirs = NULL;
  static int count = 0;
  int i;
  unsigned int *p;
  struct dirent *entry;

  for( i = 0; i < count; i++ )
    free(direntries[i]);

  count = 0;

  dirs = opendir(thisdir);
  if ( dirs != NULL )
    {
      entry = readdir(dirs);
      while ( entry != NULL )
        {
          /* Only get subdirectories */
         if (entry->d_type == DT_DIR)
		   if (strcmp(entry->d_name,".") &&
             strcmp(entry->d_name,".."))
		   {
			 direntries[count++] = strdup(entry->d_name);
		   }

		  if ( count == MAXDIRENTRIES ) break;
          entry = readdir(dirs);
        }
    }
  
  if ( count > 1 ) WKF_SortListing(count);
  
  memcpy(dirbuffer, &count, 4);
  p = (unsigned int *)(dirbuffer + 4);
  for ( i = 0; i < count; i++ )
    {
      memcpy(&p[i], &direntries[i], 4);
    }

  return count;
}


/****************************************************************************
* WKF mount
****************************************************************************/
static void WKF_mount( void )
{
  memset (basedir, 0, 1024);
  memset (scratchdir, 0, 1024);
  memset (dirbuffer, 0, 0x10000);
  
  int WKF_ret = 0;
  WKF_ret =  WKF_slot->startup() && fatMountSimple("WKF", WKF_slot);

  if (WKF_ret <= 0) {
     ActionScreen("WKF not initialized");
     return;
  }
  
  
  strcpy(basedir,"WKF:/neocd/roms/");         //  search this DIR first

  DIR *dir = opendir(basedir);
  if (!dir) { 
     strcpy(basedir,"WKF:/");        // else default to root DIR
     DIR *dir = opendir(basedir);
     if (!dir) {
        InfoScreen((char *) "Cannot open WKF directory!");
         while (1);
     }
     else closedir(dir);
  }
  else closedir(dir);

  if ( !WKF_getdir(basedir) )
    return;

  DirSelector();
  bannerscreen();
}



/****************************************************************************
* Device Handler
****************************************************************************/
void WKF_SetHandler (void)
{
  /* Clear */
  memset(&wkfhandler, 0, sizeof(GENHANDLER));
  memset(&wkf_files, 0, MAXFILES * 4);
  memset(&direntries, 0, MAXDIRENTRIES * 4);

  wkfhandler.gen_fopen     = WKF_fopen;
  wkfhandler.gen_fclose    = WKF_fclose;
  wkfhandler.gen_fread     = WKF_fread;
  wkfhandler.gen_fseek     = WKF_fseek;
  wkfhandler.gen_ftell     = WKF_ftell;
  wkfhandler.gen_fcloseall = WKF_fcloseall;
  wkfhandler.gen_getdir    = WKF_getdir;
  wkfhandler.gen_mount     = WKF_mount;
 
  GEN_SetHandler (&wkfhandler);

  /* Set mutex */
  LWP_MutexInit(&wkf_mutex, FALSE);

}

