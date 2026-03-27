#pragma once
#include "raylib.h"
#include "game.hpp"
#include <cstdint>

constexpr auto MAX_SOUNDS = 32;

struct SoundOpt {
	bool present = false;			
	Sound sound;
};
struct AudioData {
	SoundOpt sounds[MAX_SOUNDS]{};
};

void Audio_Init(AudioData* audio);
void Audio_Shutdown(AudioData* audio);
void Audio_LoadAndSetResource(AudioData* audio, AudioResource rsc, const char* file_path);
bool Audio_IsSoundResourceLoaded(AudioData* audio, AudioResource rsc);
void Audio_PlayFor(AudioData* audio, const SandGame* game);