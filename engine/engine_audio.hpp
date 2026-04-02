#pragma once
#include "raylib.h"
#include <cstdint>

using SoundFXFlags = std::uint32_t;
using SoundFXFlag = std::uint32_t;
using AudioResource = int;
constexpr auto MAX_AUDIO_RESOURCES = 64;
constexpr SoundFXFlags NULL_SFX_FLAGS = 0;

struct AudioResourceStream {
	Music stream;
	bool playing = false;
};

struct AudioResourceSound {
	Sound sound;
};

enum AudioResourceType {
	AUDIO_RSC_TYPE_SOUND,
	AUDIO_RSC_TYPE_STREAM,
	AUDIO_RSC_TYPE_MAX
};

struct SoundOpt {
	bool present = false;			
	AudioResourceType type;
	union {
		AudioResourceStream stream;
		AudioResourceSound sound;
	} sound;
};

struct EngineAudioData {
	SoundOpt sounds[MAX_AUDIO_RESOURCES]{};
	SoundFXFlags prev_sfx = NULL_SFX_FLAGS;
};

SoundFXFlag SoundFXFlag_FromResource(AudioResource rsc);
bool SoundFXFlags_Get(SoundFXFlags flags, SoundFXFlag flag);
void SoundFXFlags_Set(SoundFXFlags* flags, SoundFXFlag flag, bool to=true);
const auto MAX_SFX_FLAG = SoundFXFlag_FromResource((MAX_AUDIO_RESOURCES - 1));

void EngineAudio_Init(EngineAudioData* audio);
void EngineAudio_Shutdown(EngineAudioData* audio);
void EngineAudio_LoadAndSetSoundResource(EngineAudioData* audio, AudioResource rsc, const char* file_path);
void EngineAudio_LoadAndSetStreamResource(EngineAudioData* audio, AudioResource rsc, const char* file_path);
bool EngineAudio_IsSoundResourceLoaded(EngineAudioData* audio, AudioResource rsc);
void EngineAudio_Play(EngineAudioData* audio, SoundFXFlags* flags);