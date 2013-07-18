/****************************************************************************
* Generic File I/O
*
* Currently only supports SD
****************************************************************************/
#ifndef __SDFILEIO__
#define __SDFILEIO__

#define MAXDIRENTRIES 1000

/* Required Functions */
int SDInit( void );
int sd_getdir( char *thisdir );
extern char *direntries[MAXDIRENTRIES];

#endif
