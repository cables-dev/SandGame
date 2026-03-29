#include "audio.hpp"
#include <cassert>

void AudioResourceSound_Load(AudioResourceSound* sound_rsc, const char* file_path) {
	sound_rsc->sound = LoadSound(file_path);
	assert(IsSoundValid(sound_rsc->sound));
}
void AudioResourceSound_Free(AudioResourceSound* sound_rsc) {
	UnloadSound(sound_rsc->sound);
}
void AudioResourceSound_ToggleOn(AudioResourceSound* sound_rsc) { 
	PlaySound(sound_rsc->sound);
}
void AudioResourceSound_ToggleOff(AudioResourceSound* sound_rsc) { /* noop */ }
bool AudioResourceSound_Update(const AudioResourceSound* sound_rsc) { return false; }

void AudioResourceStream_Load(AudioResourceStream* sound_rsc, const char* file_path) {
	sound_rsc->stream = LoadMusicStream(file_path);
	assert(IsMusicValid(sound_rsc->stream));
}
void AudioResourceStream_Free(AudioResourceStream* sound_rsc) {
	UnloadMusicStream(sound_rsc->stream);
}
void AudioResourceStream_ToggleOn(AudioResourceStream* sound_rsc) {
	PlayMusicStream(sound_rsc->stream);
	sound_rsc->playing = true;
}
void AudioResourceStream_ToggleOff(AudioResourceStream* sound_rsc) {
	StopMusicStream(sound_rsc->stream);
	sound_rsc->playing = false;
}
bool AudioResourceStream_Update(const AudioResourceStream* sound_rsc){	
	if (sound_rsc->playing) UpdateMusicStream(sound_rsc->stream);
	return sound_rsc->playing;
}

void SoundOpt_CreateFromSound(SoundOpt* opt, const char* file_path) {
	opt->present = true;
	opt->type = AUDIO_RSC_TYPE_SOUND;
	AudioResourceSound_Load(&opt->sound.sound, file_path);
}
void SoundOpt_CreateFromStream(SoundOpt* opt, const char* file_path) {
	opt->present = true;
	opt->type = AUDIO_RSC_TYPE_STREAM;
	AudioResourceStream_Load(&opt->sound.stream, file_path);
}
bool SoundOpt_IsPresent(const SoundOpt* opt) {
	return opt->present;
}
void SoundOpt_Free(SoundOpt* opt) {
	if (opt == nullptr || !SoundOpt_IsPresent(opt))
		return;

	switch (opt->type) {
		case AUDIO_RSC_TYPE_SOUND: { AudioResourceSound_Free(&opt->sound.sound); break; }
		case AUDIO_RSC_TYPE_STREAM: { AudioResourceStream_Free(&opt->sound.stream); break; }
		default: { assert(false && "SoundOpt_Free: Unaccounted audio resource type."); }
	}
	opt->present = false;
}

void SoundOpt_ToggleOn(SoundOpt* opt) {
	if (opt == nullptr || !SoundOpt_IsPresent(opt))
		return;

	switch (opt->type) {
		case AUDIO_RSC_TYPE_SOUND: { AudioResourceSound_ToggleOn(&opt->sound.sound); break; }
		case AUDIO_RSC_TYPE_STREAM: { AudioResourceStream_ToggleOn(&opt->sound.stream); break; }
		default: { assert(false && "SoundOpt_Free: Unaccounted audio resource type."); }
	}
}

void SoundOpt_ToggleOff(SoundOpt* opt) {
	if (opt == nullptr || !SoundOpt_IsPresent(opt))
		return;

	switch (opt->type) {
		case AUDIO_RSC_TYPE_SOUND: { AudioResourceSound_ToggleOff(&opt->sound.sound); break; }
		case AUDIO_RSC_TYPE_STREAM: { AudioResourceStream_ToggleOff(&opt->sound.stream); break; }
		default: { assert(false && "SoundOpt_Free: Unaccounted audio resource type."); }
	}
}

// Returns whether the resource is currently playing or not.
bool SoundOpt_Update(SoundOpt* opt) {
	if (opt == nullptr || !SoundOpt_IsPresent(opt))
		return false;

	switch (opt->type) {
		case AUDIO_RSC_TYPE_SOUND: { return AudioResourceSound_Update(&opt->sound.sound); break; }
		case AUDIO_RSC_TYPE_STREAM: { return AudioResourceStream_Update(&opt->sound.stream); break; }
		default: { assert(false && "SoundOpt_Free: Unaccounted audio resource type."); }
	}
}

void Audio_ToggleOn(AudioData* data, AudioResource rsc) {
	auto* opt = &data->sounds[rsc];
	if (SoundOpt_IsPresent(opt))
		SoundOpt_ToggleOn(opt);
}

void Audio_ToggleOff(AudioData* data, AudioResource rsc) {
	auto* opt = &data->sounds[rsc];
	if (SoundOpt_IsPresent(opt))
		SoundOpt_ToggleOff(opt);
}

bool Audio_Update(AudioData* data, AudioResource rsc) {
	auto* opt = &data->sounds[rsc];
	if (SoundOpt_IsPresent(opt))
		return SoundOpt_Update(opt);
}

void Audio_FreeSounds(AudioData* audio) {
	for (int i = 0; i < MAX_SOUNDS; i++) {
		SoundOpt_Free(&audio->sounds[i]);
	}
}

void Audio_Init(AudioData* audio) {
	InitAudioDevice();
}

void Audio_Shutdown(AudioData* audio) {
	Audio_FreeSounds(audio);
	CloseAudioDevice();
}

void Audio_LoadAndSetSoundResource(AudioData* audio, AudioResource rsc, const char* file_path) {
	auto* pigeonhole = &audio->sounds[rsc];
	if (SoundOpt_IsPresent(pigeonhole))
		SoundOpt_Free(pigeonhole);
	SoundOpt_CreateFromSound(pigeonhole, file_path);
}

void Audio_LoadAndSetStreamResource(AudioData* audio, AudioResource rsc, const char* file_path) {
	auto* pigeonhole = &audio->sounds[rsc];
	if (SoundOpt_IsPresent(pigeonhole))
		SoundOpt_Free(pigeonhole);
	SoundOpt_CreateFromStream(pigeonhole, file_path);
}

bool Audio_IsSoundResourceLoaded(AudioData* audio, AudioResource rsc) {
	if (rsc > MAX_SOUNDS)
		return false;
	auto* pigeonhole = &audio->sounds[rsc];
	return SoundOpt_IsPresent(pigeonhole);
}

void Audio_PlaySoundResource(AudioData* audio, AudioResource rsc) {
	assert(Audio_IsSoundResourceLoaded(audio, rsc));
	
	auto* sound = &audio->sounds[rsc];
}

void Audio_UpdateSFXHistory(AudioData* audio, SoundFXFlags flags) {
	audio->prev_sfx = flags;
}

void Audio_Play(AudioData* audio, SoundFXFlags* sfx) {
	SoundFXFlags edges = *sfx ^ audio->prev_sfx;

	for (auto rsc_i = 0; rsc_i < AUDIO_RSC_MAX; rsc_i++) {
		auto rsc = (AudioResource)rsc_i;
		auto test_flag = (SoundFX)(1ull << rsc);

		if (SoundFXFlags_Get(edges, test_flag)) {			// Toggles
			if (SoundFXFlags_Get(*sfx, test_flag)) 
				Audio_ToggleOn(audio, rsc);
			else 
				Audio_ToggleOff(audio, rsc);
		}

		if (SoundFXFlags_Get(*sfx, (SoundFX)test_flag)) {
			auto state = Audio_Update(audio, rsc);
			SoundFXFlags_Set(sfx, test_flag, state);
		}
	}

	Audio_UpdateSFXHistory(audio, *sfx);
}

