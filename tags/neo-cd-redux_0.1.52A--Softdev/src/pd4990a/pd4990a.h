/*
 *	Header file for the PD4990A Serial I/O calendar & clock.
 */

/*** SVN 
 * $LastChangedDate: 2007-03-14 00:46:07 +0000 (Wed, 14 Mar 2007) $
 * $LastChangedRevision: 36 $
 ***/

{
  
   
   
   
   
   
   
 


#define READ8_HANDLER(name) 	unsigned char  name(unsigned int offset)
#define WRITE8_HANDLER(name) 	void   name(unsigned int offset, unsigned char data)
#define WRITE16_HANDLER(name)	void   name(unsigned int offset, unsigned short data, unsigned short mem_mask)







