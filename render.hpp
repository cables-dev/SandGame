#pragma once
#include "raylib.h"
#include "game.hpp"

struct RenderData {
	EngineRenderData engine{};
	Texture2D sand_texture{};
	Color* sand_pixel_buffer{};
	char time_readout_buffer[20]{};
	double screen_w{};
	double screen_h{};
};

// TODO: pass global constants as arguments
void Render_Init(RenderData* data, std::uint32_t window_w, std::uint32_t window_h, std::uint8_t fps, const char* window_name, GraphicResource skybox);
void Render_Shutdown(RenderData* data);
void Render_RenderGame(RenderData* data, SandGame* game, EngineTime dt);
bool Render_ShouldGameClose(const RenderData* data);
