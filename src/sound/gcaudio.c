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

u8 soundbuffer[2][8192] ATTRIBUTE_ALIGN(32);
u32 mixbuffer;
u32 audioStarted;
static int whichab = 0;
static int IsPlaying = 0;
/****************************************************************************
 * AudioSwitchBuffers
 *
 * Genesis Plus only provides sound data on completion of each frame.
 * To try to make the audio less choppy, this function is called from both the
 * DMA completion and update_audio.
 *
 * Testing for data in the buffer ensures that there are no clashes.
 ****************************************************************************/
static void AudioSwitchBuffers(void)
{
    static int len[2] = { 8192, 8192 };
   
    whichab ^= 1;
    len[whichab] = mixer_getaudio(soundbuffer[whichab], 8192);
    
    IsPlaying = 1;
    AUDIO_InitDMA((u32) soundbuffer[whichab], len[whichab]);
    DCFlushRange(soundbuffer[whichab], len[whichab]);
    AUDIO_StartDMA();


//    len[whichab] = mixer_getaudio(soundbuffer[whichab], 3200);


//    whichab ^= 1;
 //   len[whichab] = mixer_getaudio(soundbuffer[whichab], 3200);
 //       mixer_update_audio();
}


/****************************************************************************
 * InitGCAudio
 *
 * Stock code to set the DSP at 48Khz
 ****************************************************************************/
void InitGCAudio(void)
{
    AUDIO_Init(NULL);   
    AUDIO_SetDSPSampleRate(AI_SAMPLERATE_48KHZ);
    AUDIO_RegisterDMACallback(AudioSwitchBuffers);
    memset(soundbuffer, 0, 8192);

    mixer_init();
}

/****************************************************************************
 * NeoCD Audio Update
 *
 * This is called on each VBL to get the next frame of audio.
 *****************************************************************************/
void update_audio(void)
{

    mixer_update_audio();
 
    if (IsPlaying == 0) {
       AUDIO_StopDMA();
//       AUDIO_StartDMA();
       AudioSwitchBuffers();
    }

}

//void update_audio(void)
//{
//    static int len;
//    mixer_update_audio();
//    len = mixer_getaudio(soundbuffer[mixbuffer], 3840);

//    s16 *sb = (s16 *)(soundbuffer[mixbuffer]);
//    DCFlushRange((void *)sb, len);
//    AUDIO_InitDMA((u32) sb, len);
//    mixbuffer ^= 1;

//    if (!audioStarted)
//    {
//	AUDIO_StopDMA();
//	AUDIO_StartDMA();
//	audioStarted = 1;
//    }
//}
