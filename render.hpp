#pragma once
#include "raylib.h"
#include "game.hpp"

constexpr Color PARTICLE_COLOURS[]{
	Color{243, 227, 124, 255},
	Color{232, 225, 74, 255},
	Color{234, 202, 74, 255},
	Color{253, 221, 88, 255}
};
struct RenderCamera {
	std::uint32_t start_x{};
	std::uint32_t start_y{};
	std::uint32_t w{};
	std::uint32_t h{};
};
struct RenderData {
	RenderCamera camera;
	Texture2D sand_texture;
	Color* sand_pixel_buffer;
};

// TODO: pass global constants as arguments
void Render_Init(RenderData* data, std::uint32_t window_w, std::uint32_t window_h, std::uint8_t fps, const char* window_name);
void Render_Shutdown(RenderData* data);
bool Render_ShouldGameClose(const RenderData* data);
void Render_RenderGame(RenderData* data, const SandGame* game);
