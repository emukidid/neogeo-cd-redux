/****************************************************************************
* Generic File I/O
*
* Currently only supports SD
****************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

extern char dirbuffer[0x10000];
#define MAXDIRENTRIES 0x4000
static char *direntries[MAXDIRENTRIES];

/**
 * SDInit
 */
void SDInit( void )
{
  memset(&direntries, 0, MAXDIRENTRIES * 4);
}

/**
 * SortListing
 */
void SortListing( int max )
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

/**
 * Get directory listing
 */
int sd_getdir( char *thisdir )
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

  if ( count > 1 )
    SortListing(count);

  memcpy(dirbuffer, &count, 4);
  p = (unsigned int *)(dirbuffer + 4);
  for ( i = 0; i < count; i++ )
    {
      memcpy(&p[i], &direntries[i], 4);
    }

  return count;
}
