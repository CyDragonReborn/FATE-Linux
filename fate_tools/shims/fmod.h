/*
 * fmod.h  --  FSOUND (FMOD 3.75) API subset used by Fate, implemented in
 *             fmod_shim.cpp on top of miniaudio (WAV/OGG/MP3/FLAC).
 *
 * Only the calls the game makes exist here.  CD audio calls are no-ops.
 */
#ifndef FMOD_SHIM_H
#define FMOD_SHIM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FSOUND_SAMPLE FSOUND_SAMPLE;
typedef struct FSOUND_STREAM FSOUND_STREAM;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define FSOUND_FREE       (-1)
#define FSOUND_UNMANAGED  (-2)
#define FSOUND_ALL        (-3)
#define FSOUND_STEREOPAN  128

#define FSOUND_LOOP_OFF       0x00000001
#define FSOUND_LOOP_NORMAL    0x00000002
#define FSOUND_LOOP_BIDI      0x00000004
#define FSOUND_8BITS          0x00000008
#define FSOUND_16BITS         0x00000010
#define FSOUND_MONO           0x00000020
#define FSOUND_STEREO         0x00000040
#define FSOUND_UNSIGNED       0x00000080
#define FSOUND_SIGNED         0x00000100
#define FSOUND_DELTA          0x00000200
#define FSOUND_IT214          0x00000400
#define FSOUND_IT215          0x00000800
#define FSOUND_HW3D           0x00001000
#define FSOUND_2D             0x00002000
#define FSOUND_STREAMABLE     0x00004000
#define FSOUND_LOADMEMORY     0x00008000
#define FSOUND_LOADRAW        0x00010000
#define FSOUND_MPEGACCURATE   0x00020000
#define FSOUND_FORCEMONO      0x00040000
#define FSOUND_HW2D           0x00080000
#define FSOUND_ENABLEFX       0x00100000
#define FSOUND_MPEGHALFRATE   0x00200000
#define FSOUND_XADPCM         0x00400000
#define FSOUND_VAG            0x00800000
#define FSOUND_NONBLOCKING    0x01000000
#define FSOUND_NORMAL         (FSOUND_16BITS | FSOUND_SIGNED | FSOUND_MONO)

#define FSOUND_CD_PLAYCONTINUOUS 0
#define FSOUND_CD_PLAYONCE       1
#define FSOUND_CD_PLAYLOOPED     2
#define FSOUND_CD_PLAYRANDOM     3

signed char     FSOUND_SetBufferSize(int len_ms);
signed char     FSOUND_Init(int mixrate, int maxsoftwarechannels, unsigned int flags);
void            FSOUND_Close(void);

FSOUND_SAMPLE*  FSOUND_Sample_Load(int index, const char *name_or_data, unsigned int mode, int offset, int length);
void            FSOUND_Sample_Free(FSOUND_SAMPLE *sptr);

int             FSOUND_PlaySound(int channel, FSOUND_SAMPLE *sptr);
signed char     FSOUND_StopSound(int channel);
signed char     FSOUND_SetVolume(int channel, int vol);
signed char     FSOUND_SetVolumeAbsolute(int channel, int vol);
signed char     FSOUND_SetPan(int channel, int pan);
void            FSOUND_SetSFXMasterVolume(int volume);

FSOUND_STREAM*  FSOUND_Stream_Open(const char *name_or_data, unsigned int mode, int offset, int length);
signed char     FSOUND_Stream_Close(FSOUND_STREAM *stream);
int             FSOUND_Stream_Play(int channel, FSOUND_STREAM *stream);
int             FSOUND_Stream_GetTime(FSOUND_STREAM *stream);
int             FSOUND_Stream_GetLengthMs(FSOUND_STREAM *stream);

/* CD audio: not supported, always reports "no CD". */
signed char     FSOUND_CD_Play(char drive, int track);
void            FSOUND_CD_SetPlayMode(char drive, signed char mode);
signed char     FSOUND_CD_Stop(char drive);
signed char     FSOUND_CD_SetPaused(char drive, signed char paused);
signed char     FSOUND_CD_GetPaused(char drive);
signed char     FSOUND_CD_SetVolume(char drive, int volume);
int             FSOUND_CD_GetNumTracks(char drive);

#ifdef FSOUND_SHIM_TEST
void            FSOUND_Shim_Pump(unsigned frames);   /* advance a device-less engine (tests only) */
int             FSOUND_Shim_ChannelPlaying(int channel);
#endif

#ifdef __cplusplus
}
#endif
#endif /* FMOD_SHIM_H */
