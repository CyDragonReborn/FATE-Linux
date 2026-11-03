#ifndef FATE_FMOD_COMPAT_H
#define FATE_FMOD_COMPAT_H

#include <fmod.hpp>

struct FSOUND_SAMPLE;
struct FSOUND_STREAM;

enum
{
	FSOUND_FREE = -1,
	FSOUND_LOOP_OFF = 0,
	FSOUND_LOOP_NORMAL = 1 << 0,
	FSOUND_2D = 1 << 1,
	FSOUND_NORMAL = 1 << 2,
	FSOUND_CD_PLAYCONTINUOUS = 0,
	FSOUND_CD_PLAYRANDOM = 1
};

bool FSOUND_Init(int mixRate, int maxChannels, unsigned int flags);
void FSOUND_Close();
void FSOUND_SetBufferSize(int milliseconds);
void FSOUND_SetSFXMasterVolume(int volume);

FSOUND_SAMPLE* FSOUND_Sample_Load(int index, const char* name, unsigned int mode, int offset, int length);
void FSOUND_Sample_Free(FSOUND_SAMPLE* sample);
int FSOUND_PlaySound(int channel, FSOUND_SAMPLE* sample);

FSOUND_STREAM* FSOUND_Stream_Open(const char* name, unsigned int mode, int offset, int length);
int FSOUND_Stream_Play(int channel, FSOUND_STREAM* stream);
int FSOUND_Stream_GetTime(FSOUND_STREAM* stream);
int FSOUND_Stream_GetLengthMs(FSOUND_STREAM* stream);
void FSOUND_Stream_Close(FSOUND_STREAM* stream);

void FSOUND_SetVolume(int channel, int volume);
void FSOUND_SetVolumeAbsolute(int channel, int volume);
void FSOUND_SetPan(int channel, int pan);
void FSOUND_StopSound(int channel);

int FSOUND_CD_GetNumTracks(int drive);
bool FSOUND_CD_GetPaused(int drive);
void FSOUND_CD_Play(int drive, int track);
void FSOUND_CD_SetPaused(int drive, bool paused);
void FSOUND_CD_SetPlayMode(int drive, int mode);
void FSOUND_CD_SetVolume(int drive, int volume);
void FSOUND_CD_Stop(int drive);

#endif