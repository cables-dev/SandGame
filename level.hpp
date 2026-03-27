#pragma once
#include "common.hpp"
#include "game.hpp"
#include "audio.hpp"
#include "render.hpp"

bool Level_LoadFromFile(AudioData* audio, RenderData* render, SandGame* game, const char* file_path);