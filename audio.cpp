#include "audio.hpp"
#include <cassert>

void Audio_Init(AudioData* audio) {
	InitAudioDevice();
}

void SoundOpt_Create(SoundOpt* opt, Sound sound) {
	opt->present = true;
	opt->sound = sound;
}

bool SoundOpt_IsPresent(const SoundOpt* opt) {
	return opt->present;
}

// Will do no work if opt is empty.
void SoundOpt_Free(SoundOpt* opt) {
	if (opt == nullptr)
		return;
	if (SoundOpt_IsPresent(opt))
		UnloadSound(opt->sound);
	opt->present = false;
}

void Audio_FreeSounds(AudioData* audio) {
	for (int i = 0; i < MAX_SOUNDS; i++) {
		SoundOpt_Free(&audio->sounds[i]);
	}
}

void Audio_Shutdown(AudioData* audio) {
	Audio_FreeSounds(audio);
	CloseAudioDevice();
}

void Audio_LoadAndSetResource(AudioData* audio, AudioResource rsc, const char* file_path) {
	auto sound = LoadSound(file_path);
	assert(IsSoundValid(sound) && "Audio_LoadSound: Could not load sound at provided path.");
	
	auto* pigeonhole = &audio->sounds[rsc];
	SoundOpt_Free(pigeonhole);
	SoundOpt_Create(pigeonhole, sound);
}

bool Audio_IsSoundResourceLoaded(AudioData* audio, AudioResource rsc) {
	if (rsc > MAX_SOUNDS)
		return false;
	auto* pigeonhole = &audio->sounds[rsc];
	return SoundOpt_IsPresent(pigeonhole);
}

void Audio_PlaySoundResource(AudioData* audio, AudioResource rsc) {
	assert(Audio_IsSoundResourceLoaded(audio, rsc));
	
	auto sound = audio->sounds[rsc].sound;
	PlaySound(sound);
}

void Audio_PlayFor(AudioData* audio, const SandGame* game) {
	auto sfx = game->sfx_flags;
	for (std::uint32_t rsc_i = 0; rsc_i < AUDIO_RSC_MAX; rsc_i++) {
		auto rsc = (AudioResource)rsc_i;

		SoundFX test_flag = (SoundFX)(1 << rsc);
		if (SoundFXFlags_Get(sfx, (SoundFX)test_flag)) {
			Audio_PlaySoundResource(audio, rsc);
		}
	}
}

