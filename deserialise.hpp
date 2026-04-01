#pragma once
#include "common.hpp"
#include "game.hpp"
#include "engine/engine_audio.hpp"
#include "render.hpp"
#include "serial.hpp"

bool Level_DeserialiseFile(
	EngineAudioData* audio, 
	RenderData* render, 
	SandGame* game, 
	DeserialiseMetadata* out_meta,
	const char* file_path,
	DeserialiseError* out_error
);
