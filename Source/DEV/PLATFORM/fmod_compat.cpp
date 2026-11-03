#include "fmod.h"

#include <algorithm>
#include <unordered_map>

struct FSOUND_SAMPLE
{
	FMOD::Sound* sound = nullptr;
	unsigned int mode = 0;
};

struct FSOUND_STREAM
{
	FMOD::Sound* sound = nullptr;
	FMOD::Channel* channel = nullptr;
	int channelId = FSOUND_FREE;
};

namespace
{
FMOD::System* g_system = nullptr;
FMOD::ChannelGroup* g_sfxGroup = nullptr;
FMOD::ChannelGroup* g_musicGroup = nullptr;
std::unordered_map<int, FMOD::Channel*> g_channels;
int g_nextChannelId = 0;

int RegisterChannel(FMOD::Channel* channel)
{
	if (channel == nullptr)
	{
		return FSOUND_FREE;
	}
	const int id = g_nextChannelId++;
	g_channels[id] = channel;
	return id;
}

FMOD::Channel* FindChannel(int id)
{
	const auto found = g_channels.find(id);
	return found == g_channels.end() ? nullptr : found->second;
}

FMOD_MODE ConvertMode(unsigned int mode, bool stream)
{
	FMOD_MODE result = stream ? FMOD_CREATESTREAM : FMOD_CREATESAMPLE;
	result |= FMOD_2D;
	if (mode & FSOUND_LOOP_NORMAL)
	{
		result |= FMOD_LOOP_NORMAL;
	}
	else
	{
		result |= FMOD_LOOP_OFF;
	}
	return result;
}

void SetChannelVolume(int id, int volume)
{
	if (FMOD::Channel* channel = FindChannel(id))
	{
		channel->setVolume(static_cast<float>(std::clamp(volume, 0, 255)) / 255.0f);
	}
}
}

bool FSOUND_Init(int mixRate, int maxChannels, unsigned int)
{
	if (g_system != nullptr)
	{
		return true;
	}
	if (FMOD::System_Create(&g_system) != FMOD_OK)
	{
		g_system = nullptr;
		return false;
	}
	if (g_system->setSoftwareFormat(mixRate, FMOD_SPEAKERMODE_STEREO, 0) != FMOD_OK ||
		g_system->setSoftwareChannels(maxChannels) != FMOD_OK ||
		g_system->init(maxChannels, FMOD_INIT_NORMAL, nullptr) != FMOD_OK ||
		g_system->createChannelGroup("FATE SFX", &g_sfxGroup) != FMOD_OK ||
		g_system->createChannelGroup("FATE Music", &g_musicGroup) != FMOD_OK)
	{
		FSOUND_Close();
		return false;
	}
	return true;
}

void FSOUND_Close()
{
	g_channels.clear();
	g_sfxGroup = nullptr;
	g_musicGroup = nullptr;
	if (g_system != nullptr)
	{
		g_system->release();
		g_system = nullptr;
	}
}

void FSOUND_SetBufferSize(int)
{
}

void FSOUND_SetSFXMasterVolume(int volume)
{
	if (g_sfxGroup != nullptr)
	{
		g_sfxGroup->setVolume(static_cast<float>(std::clamp(volume, 0, 255)) / 255.0f);
	}
}

FSOUND_SAMPLE* FSOUND_Sample_Load(int, const char* name, unsigned int mode, int, int)
{
	if (g_system == nullptr || name == nullptr)
	{
		return nullptr;
	}
	FSOUND_SAMPLE* sample = new FSOUND_SAMPLE();
	sample->mode = mode;
	if (g_system->createSound(name, ConvertMode(mode, false), nullptr, &sample->sound) != FMOD_OK)
	{
		delete sample;
		return nullptr;
	}
	return sample;
}

void FSOUND_Sample_Free(FSOUND_SAMPLE* sample)
{
	if (sample == nullptr)
	{
		return;
	}
	if (g_system != nullptr && sample->sound != nullptr)
	{
		sample->sound->release();
	}
	delete sample;
}

int FSOUND_PlaySound(int, FSOUND_SAMPLE* sample)
{
	if (g_system == nullptr || g_sfxGroup == nullptr || sample == nullptr || sample->sound == nullptr)
	{
		return FSOUND_FREE;
	}
	FMOD::Channel* channel = nullptr;
	if (g_system->playSound(sample->sound, g_sfxGroup, false, &channel) != FMOD_OK)
	{
		return FSOUND_FREE;
	}
	return RegisterChannel(channel);
}

FSOUND_STREAM* FSOUND_Stream_Open(const char* name, unsigned int mode, int, int)
{
	if (g_system == nullptr || name == nullptr)
	{
		return nullptr;
	}
	FSOUND_STREAM* stream = new FSOUND_STREAM();
	if (g_system->createStream(name, ConvertMode(mode, true), nullptr, &stream->sound) != FMOD_OK)
	{
		delete stream;
		return nullptr;
	}
	return stream;
}

int FSOUND_Stream_Play(int, FSOUND_STREAM* stream)
{
	if (g_system == nullptr || g_musicGroup == nullptr || stream == nullptr || stream->sound == nullptr)
	{
		return FSOUND_FREE;
	}
	if (g_system->playSound(stream->sound, g_musicGroup, false, &stream->channel) != FMOD_OK)
	{
		return FSOUND_FREE;
	}
	stream->channelId = RegisterChannel(stream->channel);
	return stream->channelId;
}

int FSOUND_Stream_GetTime(FSOUND_STREAM* stream)
{
	unsigned int position = 0;
	if (g_system == nullptr || stream == nullptr || stream->channel == nullptr ||
		stream->channel->getPosition(&position, FMOD_TIMEUNIT_MS) != FMOD_OK)
	{
		return 0;
	}
	return static_cast<int>(position);
}

int FSOUND_Stream_GetLengthMs(FSOUND_STREAM* stream)
{
	unsigned int length = 0;
	if (g_system == nullptr || stream == nullptr || stream->sound == nullptr ||
		stream->sound->getLength(&length, FMOD_TIMEUNIT_MS) != FMOD_OK)
	{
		return 0;
	}
	return static_cast<int>(length);
}

void FSOUND_Stream_Close(FSOUND_STREAM* stream)
{
	if (stream == nullptr)
	{
		return;
	}
	if (g_system != nullptr)
	{
		FSOUND_StopSound(stream->channelId);
		if (stream->sound != nullptr)
		{
			stream->sound->release();
		}
	}
	delete stream;
}

void FSOUND_SetVolume(int channel, int volume)
{
	SetChannelVolume(channel, volume);
}

void FSOUND_SetVolumeAbsolute(int channel, int volume)
{
	SetChannelVolume(channel, volume);
}

void FSOUND_SetPan(int id, int pan)
{
	if (FMOD::Channel* channel = FindChannel(id))
	{
		const int boundedPan = std::clamp(pan, 0, 255);
		channel->setPan(static_cast<float>(boundedPan - 128) / 128.0f);
	}
}

void FSOUND_StopSound(int id)
{
	const auto found = g_channels.find(id);
	if (found != g_channels.end())
	{
		found->second->stop();
		g_channels.erase(found);
	}
}

int FSOUND_CD_GetNumTracks(int)
{
	return 0;
}

bool FSOUND_CD_GetPaused(int)
{
	return false;
}

void FSOUND_CD_Play(int, int)
{
}

void FSOUND_CD_SetPaused(int, bool)
{
}

void FSOUND_CD_SetPlayMode(int, int)
{
}

void FSOUND_CD_SetVolume(int, int)
{
}

void FSOUND_CD_Stop(int)
{
}