#pragma once
#include "common.hpp"
#include "game.hpp"
#include "engine/engine_audio.hpp"
#include "render.hpp"

bool Level_LoadFromFile(EngineAudioData* audio, RenderData* render, SandGame* game, const char* file_path);