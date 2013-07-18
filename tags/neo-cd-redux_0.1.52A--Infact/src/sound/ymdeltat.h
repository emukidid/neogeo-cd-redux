#ifndef __YMDELTAT_H_
#define __YMDELTAT_H_

#include "neocdredux.h"

#define YM_DELTAT_SHIFT    (16)

/* adpcm type A and type B struct */
typedef struct deltat_adpcm_state
{
  u8 *memory;
  int memory_size;
  double freqbase;
  s32 *output_pointer;	/* pointer of output pointers */
  int output_range;

  u8 reg[16];
  u8 portstate, portcontrol;
  int portshift;

  u8 flag;			/* port state        */
  u8 flagMask;		/* arrived flag mask */
  u8 now_data;
  u32 now_addr;
  u32 now_step;
  u32 step;
  u32 start;
  u32 end;
  u32 delta;
  s32 volume;
  s32 *pan;			/* &output_pointer[pan] */
    s32 /*adpcmm, */ adpcmx, adpcmd;
  s32 adpcml;		/* hiro-shi!! */

  /* leveling and re-sampling state for DELTA-T */
  s32 volume_w_step;		/* volume with step rate */
  s32 next_leveling;		/* leveling value        */
  s32 sample_step;		/* step of re-sampling   */

  u8 arrivedFlag;		/* flag of arrived end address */
} YM_DELTAT;

/* static state */
extern u8 *ym_deltat_memory;	/* memory pointer */

/* before YM_DELTAT_ADPCM_CALC(YM_DELTAT *DELTAT); */
#define YM_DELTAT_DECODE_PRESET(DELTAT) {ym_deltat_memory = DELTAT->memory;}

void YM_DELTAT_ADPCM_Write (YM_DELTAT * DELTAT, int r, int v);
void YM_DELTAT_ADPCM_Reset (YM_DELTAT * DELTAT, int pan);


/* ---------- inline block ---------- */

/* DELTA-T particle adjuster */
#define YM_DELTAT_DELTA_MAX (24576)
#define YM_DELTAT_DELTA_MIN (127)
#define YM_DELTAT_DELTA_DEF (127)

#define YM_DELTAT_DECODE_RANGE 32768
#define YM_DELTAT_DECODE_MIN (-(YM_DELTAT_DECODE_RANGE))
#define YM_DELTAT_DECODE_MAX ((YM_DELTAT_DECODE_RANGE)-1)

extern const s32 ym_deltat_decode_tableB1[];
extern const s32 ym_deltat_decode_tableB2[];

#define YM_DELTAT_Limit(val,max,min)	\
{					\
	if ( val > max ) val = max;	\
	else if ( val < min ) val = min;\
}

/**** ADPCM B (Delta-T control type) ****/
inline void YM_DELTAT_ADPCM_CALC (YM_DELTAT * DELTAT);

/* INLINE void YM_DELTAT_ADPCM_CALC(YM_DELTAT *DELTAT); */
//#define YM_INLINE_BLOCK
//#include "ymdeltat.c" /* include inline function section */
//#undef  YM_INLINE_BLOCK

void YM_DELTAT_postload (YM_DELTAT * DELTAT, u8 * regs);
void YM_DELTAT_savestate (YM_DELTAT * DELTAT);

#endif
