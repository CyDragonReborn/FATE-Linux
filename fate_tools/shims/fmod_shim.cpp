/*
 * fmod_shim.cpp -- FSOUND API subset on top of miniaudio.
 *
 * Semantics follow FMOD 3.75: channel handles, 0..255 volume/pan, SFX master volume
 * that scales every channel except those set with SetVolumeAbsolute.
 */
#include "fmod.h"

#define STB_VORBIS_HEADER_ONLY
#include "third_party/stb_vorbis.c"      /* declarations only, so miniaudio enables Vorbis */
#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#include "third_party/miniaudio.h"

#include <string.h>
#include <stdlib.h>

namespace {

const int kChannels = 64;

struct Sample {
    void        *pcm;          /* s16 interleaved, owned (ma_free) */
    ma_uint64    frames;
    ma_uint32    channels;
    bool         loop;
};

struct Stream {
    ma_decoder   decoder;
    ma_sound     sound;
    bool         loop;
    int          channel;      /* handle while playing, else -1 */
    int          lengthMs;
};

struct Channel {
    bool         used;
    unsigned     gen;
    ma_sound     sound;        /* sample playback (unused for streams) */
    ma_audio_buffer_ref ref;
    Sample      *sample;
    Stream      *stream;
    int          vol;          /* 0..255 */
    int          pan;          /* 0..255, 128 centre */
    bool         absolute;
};

ma_engine   g_engine;
bool        g_engineUp = false;
int         g_master = 255;
int         g_bufferMs = 100;
unsigned    g_gen = 0;
Channel     g_ch[kChannels];

inline ma_sound* SoundOf(Channel &c) { return c.stream ? &c.stream->sound : &c.sound; }

float PanF(int pan)
{
    if (pan < 0 || pan == 128) return 0.0f;
    float p = (pan - 128) / 127.0f;
    return p < -1.0f ? -1.0f : p > 1.0f ? 1.0f : p;
}

void ApplyVolume(Channel &c)
{
    float v = c.vol / 255.0f;
    if (!c.absolute) v *= g_master / 255.0f;
    ma_sound_set_volume(SoundOf(c), v);
}

void Release(Channel &c)
{
    if (!c.used) return;
    if (c.stream) {
        ma_sound_stop(&c.stream->sound);
        c.stream->channel = -1;
    } else {
        ma_sound_uninit(&c.sound);
    }
    c.used = false; c.sample = NULL; c.stream = NULL;
}

void Reap()
{
    for (int i = 0; i < kChannels; i++) {
        Channel &c = g_ch[i];
        if (c.used && ma_sound_at_end(SoundOf(c))) Release(c);
    }
}

int Handle(int idx) { return (int)(((g_ch[idx].gen & 0x7FFFF) << 12) | (unsigned)idx); }

Channel* Lookup(int handle)
{
    if (handle < 0) return NULL;
    int idx = handle & 0xFFF;
    if (idx >= kChannels) return NULL;
    Channel &c = g_ch[idx];
    if (!c.used || ((unsigned)(handle >> 12) != (c.gen & 0x7FFFF))) return NULL;
    return &c;
}

int Alloc(int requested)
{
    Reap();
    int idx = -1;
    if (requested >= 0 && (requested & 0xFFF) < kChannels) { idx = requested & 0xFFF; Release(g_ch[idx]); }
    else for (int i = 0; i < kChannels; i++) if (!g_ch[i].used) { idx = i; break; }
    if (idx < 0) return -1;
    Channel &c = g_ch[idx];
    c.used = true; c.gen = ++g_gen; c.sample = NULL; c.stream = NULL; c.vol = 255; c.pan = 128; c.absolute = false;
    return idx;
}

bool ExtIsOgg(const char *n)
{
    size_t l = strlen(n);
    return l > 4 && (n[l-4] == '.') && (n[l-3] | 0x20) == 'o' && (n[l-2] | 0x20) == 'g' && (n[l-1] | 0x20) == 'g';
}

} /* namespace */

extern "C" {

signed char FSOUND_SetBufferSize(int len_ms) { g_bufferMs = len_ms; return TRUE; }

signed char FSOUND_Init(int mixrate, int, unsigned int)
{
    if (g_engineUp) return TRUE;
    ma_engine_config cfg = ma_engine_config_init();
    if (mixrate > 0) cfg.sampleRate = (ma_uint32)mixrate;
#ifdef FSOUND_SHIM_TEST
    cfg.noDevice = MA_TRUE; cfg.channels = 2; if (!cfg.sampleRate) cfg.sampleRate = 44100;
#endif
    /* g_bufferMs (game asks for 100 ms) is deliberately ignored: miniaudio's default period is lower latency. */
    if (ma_engine_init(&cfg, &g_engine) != MA_SUCCESS) return FALSE;
    memset(g_ch, 0, sizeof(g_ch));
    g_engineUp = true;
    return TRUE;
}

void FSOUND_Close(void)
{
    if (!g_engineUp) return;
    for (int i = 0; i < kChannels; i++) Release(g_ch[i]);
    ma_engine_uninit(&g_engine);
    g_engineUp = false;
}

FSOUND_SAMPLE* FSOUND_Sample_Load(int, const char *name_or_data, unsigned int mode, int, int length)
{
    if (!g_engineUp || !name_or_data) return NULL;
    ma_decoder_config dc = ma_decoder_config_init(ma_format_s16, 0, ma_engine_get_sample_rate(&g_engine));
    void *pcm = NULL; ma_uint64 frames = 0;
    ma_result r = (mode & FSOUND_LOADMEMORY)
        ? ma_decode_memory(name_or_data, (size_t)length, &dc, &frames, &pcm)
        : ma_decode_file(name_or_data, &dc, &frames, &pcm);
    if (r != MA_SUCCESS || !pcm) return NULL;
    Sample *s = (Sample*)malloc(sizeof(Sample));
    s->pcm = pcm; s->frames = frames; s->channels = dc.channels ? dc.channels : 1;
    s->loop = (mode & (FSOUND_LOOP_NORMAL | FSOUND_LOOP_BIDI)) != 0;
    return (FSOUND_SAMPLE*)s;
}

void FSOUND_Sample_Free(FSOUND_SAMPLE *sp)
{
    Sample *s = (Sample*)sp; if (!s) return;
    for (int i = 0; i < kChannels; i++) if (g_ch[i].used && g_ch[i].sample == s) Release(g_ch[i]);
    ma_free(s->pcm, NULL); free(s);
}

int FSOUND_PlaySound(int channel, FSOUND_SAMPLE *sp)
{
    Sample *s = (Sample*)sp; if (!g_engineUp || !s) return -1;
    int idx = Alloc(channel); if (idx < 0) return -1;
    Channel &c = g_ch[idx];
    ma_audio_buffer_ref_init(ma_format_s16, s->channels, s->pcm, s->frames, &c.ref);
    if (ma_sound_init_from_data_source(&g_engine, &c.ref, MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, &c.sound) != MA_SUCCESS) {
        c.used = false; return -1;
    }
    c.sample = s;
    ma_sound_set_looping(&c.sound, s->loop ? MA_TRUE : MA_FALSE);
    ApplyVolume(c); ma_sound_set_pan(&c.sound, PanF(c.pan));
    ma_sound_start(&c.sound);
    return Handle(idx);
}

signed char FSOUND_StopSound(int channel)
{
    if (channel == FSOUND_ALL) { for (int i = 0; i < kChannels; i++) Release(g_ch[i]); return TRUE; }
    Channel *c = Lookup(channel); if (!c) return FALSE;
    Release(*c); return TRUE;
}

signed char FSOUND_SetVolume(int channel, int vol)
{
    if (vol < 0) vol = 0; if (vol > 255) vol = 255;
    if (channel == FSOUND_ALL) { for (int i = 0; i < kChannels; i++) if (g_ch[i].used) { g_ch[i].vol = vol; g_ch[i].absolute = false; ApplyVolume(g_ch[i]); } return TRUE; }
    Channel *c = Lookup(channel); if (!c) return FALSE;
    c->vol = vol; c->absolute = false; ApplyVolume(*c); return TRUE;
}

signed char FSOUND_SetVolumeAbsolute(int channel, int vol)
{
    if (vol < 0) vol = 0; if (vol > 255) vol = 255;
    if (channel == FSOUND_ALL) { for (int i = 0; i < kChannels; i++) if (g_ch[i].used) { g_ch[i].vol = vol; g_ch[i].absolute = true; ApplyVolume(g_ch[i]); } return TRUE; }
    Channel *c = Lookup(channel); if (!c) return FALSE;
    c->vol = vol; c->absolute = true; ApplyVolume(*c); return TRUE;
}

signed char FSOUND_SetPan(int channel, int pan)
{
    Channel *c = Lookup(channel); if (!c) return FALSE;
    c->pan = pan; ma_sound_set_pan(SoundOf(*c), PanF(pan)); return TRUE;
}

void FSOUND_SetSFXMasterVolume(int volume)
{
    if (volume < 0) volume = 0; if (volume > 255) volume = 255;
    g_master = volume;
    for (int i = 0; i < kChannels; i++) if (g_ch[i].used) ApplyVolume(g_ch[i]);
}

FSOUND_STREAM* FSOUND_Stream_Open(const char *name_or_data, unsigned int mode, int, int length)
{
    if (!g_engineUp || !name_or_data) return NULL;
    Stream *st = (Stream*)calloc(1, sizeof(Stream));
    ma_decoder_config dc = ma_decoder_config_init(ma_format_f32, 0, 0);
    bool mem = (mode & FSOUND_LOADMEMORY) != 0;
    ma_result r = mem ? ma_decoder_init_memory(name_or_data, (size_t)length, &dc, &st->decoder)
                      : ma_decoder_init_file(name_or_data, &dc, &st->decoder);
    if (r != MA_SUCCESS) { free(st); return NULL; }
    if (ma_sound_init_from_data_source(&g_engine, &st->decoder, MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, &st->sound) != MA_SUCCESS) {
        ma_decoder_uninit(&st->decoder); free(st); return NULL;
    }
    st->loop = (mode & (FSOUND_LOOP_NORMAL | FSOUND_LOOP_BIDI)) != 0;
    st->channel = -1;
    ma_sound_set_looping(&st->sound, st->loop ? MA_TRUE : MA_FALSE);

    /* Length: miniaudio reports 0 for Vorbis, so ask stb_vorbis directly. */
    float secs = 0; ma_sound_get_length_in_seconds(&st->sound, &secs);
    st->lengthMs = (int)(secs * 1000.0f + 0.5f);
    if (st->lengthMs <= 0) {
        int err = 0; stb_vorbis *v = mem ? stb_vorbis_open_memory((const unsigned char*)name_or_data, length, &err, NULL)
                                         : (ExtIsOgg(name_or_data) ? stb_vorbis_open_filename(name_or_data, &err, NULL) : NULL);
        if (v) { st->lengthMs = (int)(stb_vorbis_stream_length_in_seconds(v) * 1000.0f + 0.5f); stb_vorbis_close(v); }
    }
    return (FSOUND_STREAM*)st;
}

signed char FSOUND_Stream_Close(FSOUND_STREAM *sp)
{
    Stream *st = (Stream*)sp; if (!st) return FALSE;
    for (int i = 0; i < kChannels; i++) if (g_ch[i].used && g_ch[i].stream == st) Release(g_ch[i]);
    ma_sound_uninit(&st->sound); ma_decoder_uninit(&st->decoder); free(st);
    return TRUE;
}

int FSOUND_Stream_Play(int channel, FSOUND_STREAM *sp)
{
    Stream *st = (Stream*)sp; if (!g_engineUp || !st) return -1;
    if (st->channel >= 0) { Channel *old = Lookup(st->channel); if (old) Release(*old); }
    int idx = Alloc(channel); if (idx < 0) return -1;
    Channel &c = g_ch[idx];
    c.stream = st; st->channel = Handle(idx);
    ma_sound_seek_to_pcm_frame(&st->sound, 0);
    ApplyVolume(c); ma_sound_set_pan(&st->sound, PanF(c.pan));
    ma_sound_start(&st->sound);
    return st->channel;
}

int FSOUND_Stream_GetTime(FSOUND_STREAM *sp)
{
    Stream *st = (Stream*)sp; if (!st) return 0;
    float s = 0; ma_sound_get_cursor_in_seconds(&st->sound, &s);
    return (int)(s * 1000.0f);
}
int FSOUND_Stream_GetLengthMs(FSOUND_STREAM *sp) { Stream *st = (Stream*)sp; return st ? st->lengthMs : 0; }

/* CD audio: no drive, no tracks. */
signed char FSOUND_CD_Play(char, int) { return FALSE; }
void        FSOUND_CD_SetPlayMode(char, signed char) {}
signed char FSOUND_CD_Stop(char) { return FALSE; }
signed char FSOUND_CD_SetPaused(char, signed char) { return FALSE; }
signed char FSOUND_CD_GetPaused(char) { return FALSE; }
signed char FSOUND_CD_SetVolume(char, int) { return FALSE; }
int         FSOUND_CD_GetNumTracks(char) { return 0; }

#ifdef FSOUND_SHIM_TEST
void FSOUND_Shim_Pump(unsigned frames)
{
    static float buf[4096 * 2];
    while (frames) { unsigned n = frames > 4096 ? 4096 : frames; ma_uint64 got = 0; ma_engine_read_pcm_frames(&g_engine, buf, n, &got); frames -= n; }
}
int FSOUND_Shim_ChannelPlaying(int channel) { Channel *c = Lookup(channel); return c && ma_sound_is_playing(SoundOf(*c)); }
#endif

} /* extern "C" */
