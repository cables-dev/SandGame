#pragma once
#include "raylib.h"
#include "game.hpp"

constexpr auto RENDER_BACKGROUND_COLOR = Color{ 12, 5, 26, 255 };
constexpr auto WEAPON_NAME_FONT_SIZE = 20;
constexpr auto WEAPON_NAME_FONT_COLOR = Color{ 0xbf, 0xbf, 0xbf, 0xff };
constexpr auto WHITE_COLOUR = GameColour{ 0xffffffff };
constexpr auto TIMER_READOUT_FONT_COLOUR = WHITE_COLOUR;
constexpr auto TIME_READOUT_FONT_SIZE = 30;
constexpr auto TIME_READOUT_X_OFFSET = 148;

constexpr Color PARTICLE_COLOURS[]{
	Color{243, 227, 124, 255},
	Color{232, 225, 74, 255},
	Color{234, 202, 74, 255},
	Color{253, 221, 88, 255}
};

//constexpr Color PARTICLE_COLOURS[]{
//	Color{245, 100, 134, 255},
//	Color{158, 21, 21, 255},
//	Color{184, 17, 17, 255},
//	Color{250, 69, 69, 255}
//};


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
bool Render_ShouldGameClose(const RenderData* data);
void Render_RenderGame(RenderData* data, SandGame* game, EngineTime dt);
