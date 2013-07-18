/*
 *	Emulation for the NEC PD4990A.
 *
 *	The PD4990A is a serial I/O Calendar & Clock IC used in the
 *		NEO GEO and probably a couple of other machines.
 */  
/*** SVN 
 * $LastChangedDate: 2007-03-14 00:46:07 +0000 (Wed, 14 Mar 2007) $
 * $LastChangedRevision: 36 $
 ***/
  
#include <gccore.h>
#include "neocdredux.h"
  
/* Set the data in the chip to Monday 09/09/73 00:00:00 	*/ 
/* If you ever read this Leejanne, you know what I mean :-) */ 
struct pd4990a_s pd4990a = 
    0x00, /* minutes BCD */ 
    0x00, /* hours   BCD */ 
    0x09, /* days    BCD */ 
    9, /* month   Hexadecimal form */ 
    0x73, /* year    BCD */ 
    1 /* weekday BCD */  
};



  /* test output */ 
static int outputbit = 0;


pd4990a_addretrace (void) 
{
  
  
  
    
  
  
  
    
  
  
  
    
  
  
  
    
  
  
  
    
  
  
  
    
  
  
  
    
  
  


pd4990a_increment_day (void) 
{
  
  
  
    
    {
      
      
    
  
  
    
  
    
    {
    
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
      
	
	{
	  
	  
	
      
    
      
      
	
	{
	  
	    
	    {
	      
	      
	    
	
      
      else
	
	{
	  
	    
	    {
	      
	      
	    
	
      
    
    case 6:
    case 9:
    case 11:
      
	
	{
	  
	  
	
      
    


pd4990a_increment_month (void) 
{
  
  
    
    {
      
      
      
	
	{
	  
	  
	
      
	
    



{
  



{
  



{
  
    
    {
    
      
	
	{
	
	case 0x01:
	case 0x02:
	case 0x03:
	
	case 0x05:
	case 0x06:
	case 0x07:
	  
	  
	
	case 0x09:
	case 0x0a:
	case 0x0b:
	
	case 0x0d:
	case 0x0e:
	case 0x0f:
	  
	  
	
	case 0x11:
	case 0x12:
	case 0x13:
	
	case 0x15:
	case 0x16:
	case 0x17:
	  
	  
	
	case 0x19:
	case 0x1a:
	case 0x1b:
	
	case 0x1d:
	case 0x1e:
	case 0x1f:
	  
	  
	
	case 0x21:
	case 0x22:
	case 0x23:
	  
	  
	
	case 0x25:
	case 0x26:
	case 0x27:
	  
	  
	
	case 0x29:
	case 0x2a:
	case 0x2b:
	
	case 0x2d:
	case 0x2e:
	case 0x2f:
	  
	  
	
      
    
      
      
    
      
      
    
      
    



{
  


