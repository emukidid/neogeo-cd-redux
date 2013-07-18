/****************************************************************************
*   NeoCD Redux 0.1
*   NeoGeo CD Emulator
*   Copyright (C) 2007 softdev
****************************************************************************/

#include "neocdredux.h"
#include "streams.h"
#include "eq.h"

/**
 * Nintendo Gamecube Audio Interface 
 */

u8 soundbuffer[2][3840] ATTRIBUTE_ALIGN(32);
u32 mixbuffer;
u32 audioStarted;

/****************************************************************************
 * InitGCAudio
 *
 * Stock code to set the DSP at 48Khz
 ****************************************************************************/
void InitGCAudio(void)
{
    AUDIO_Init(NULL);
    AUDIO_SetDSPSampleRate(AI_SAMPLERATE_48KHZ);

    memset(soundbuffer, 0, 2 * 3840);
    mixer_init();
    audioStarted = 0;
    mixbuffer = 0;
}

/****************************************************************************
 * NeoCD Audio Update
 *
 * This is called on each VBL to get the next frame of audio.
 *****************************************************************************/
void update_audio(void)
{
    static int len;
    mixer_update_audio();
    len = mixer_getaudio(soundbuffer[mixbuffer], 3840);

    s16 *sb = (s16 *)(soundbuffer[mixbuffer]);
    DCFlushRange((void *)sb, len);
    AUDIO_InitDMA((u32) sb, len);
    mixbuffer ^= 1;

    if (!audioStarted)
    {
	AUDIO_StopDMA();
	AUDIO_StartDMA();
	audioStarted = 1;
    }
}
