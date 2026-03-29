#pragma once
#include "raylib.h"
#include "game.hpp"
#include <cstdint>

constexpr auto MAX_SOUNDS = 64;

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

struct AudioData {
	SoundOpt sounds[MAX_SOUNDS]{};
	SoundFXFlags prev_sfx = NULL_SFX_FLAGS;
};

void Audio_Init(AudioData* audio);
void Audio_Shutdown(AudioData* audio);
void Audio_LoadAndSetSoundResource(AudioData* audio, AudioResource rsc, const char* file_path);
void Audio_LoadAndSetStreamResource(AudioData* audio, AudioResource rsc, const char* file_path);
bool Audio_IsSoundResourceLoaded(AudioData* audio, AudioResource rsc);
void Audio_Play(AudioData* audio, SoundFXFlags* flags);