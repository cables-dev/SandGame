#pragma once
#include "raylib.h"
#include "game.hpp"

constexpr auto MAX_GRAPHIC_RESOURCES = 64;
constexpr auto RENDER_BACKGROUND_COLOR = Color{ 12, 5, 26, 255 };
constexpr auto WEAPON_NAME_FONT_SIZE = 20;
constexpr auto WEAPON_NAME_FONT_COLOR = Color{ 0xbf, 0xbf, 0xbf, 0xff };
constexpr auto FX_TOAST_DISPLAY_DURATION = 4.0;			// 4s
constexpr auto FX_TOAST_FADE_AFTER = 3.3;
constexpr auto FX_TOAST_FONT_SIZE = 30;
constexpr auto FX_TOAST_FONT_COLOR = Color{0xff, 0xff, 0xff, 0xff};
constexpr auto FX_WHITE_FLASH_DURATION_S = 1.0;			// 4s
constexpr auto FX_BLACK_FADE_IN_DURATION_S = 1.5;		// Should be the same as DOOR_LOADING_DELAY_S.
constexpr auto FX_BLACK_FADE_OUT_DURATION_S = 0.5;		

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

enum GraphicResourceType {
	GRAPHIC_RSC_TYPE_IMAGE,
	GRAPHIC_RSC_TYPE_ANIMATION,
	GRAPHIC_RSC_TYPE_MAX
};
struct GraphicResourceImage {	
	Texture2D texture;
	std::uint32_t w;
	std::uint32_t h;
};
struct GraphicResourceAnimation {	
	Texture2D texture;
	Image sprite;
	float refresh_period_s;					// Raylib uses float for timings, so we comply
	float time_until_update_s;
	int num_frames;
	int current_frame;
	std::uint32_t w;
	std::uint32_t h;
};

struct GraphicResourceOpt {
	bool present = false;
	GraphicResourceType type;
	union {
		GraphicResourceImage image;		
		GraphicResourceAnimation animation;
	} resource;
};

struct RenderData {
	RenderCamera camera;
	Texture2D sand_texture;
	Color* sand_pixel_buffer;
	float toast_display_duration = 0.0;
	float white_flash_duration = 0.0;
	float black_fade_in_out_duration = 0.0;
	GraphicResourceOpt resources[MAX_GRAPHIC_RESOURCES]{};
	GraphicResource skybox;
};

// TODO: pass global constants as arguments
void Render_Init(RenderData* data, std::uint32_t window_w, std::uint32_t window_h, std::uint8_t fps, const char* window_name, GraphicResource skybox);
void Render_Shutdown(RenderData* data);
bool Render_ShouldGameClose(const RenderData* data);
void Render_RenderGame(RenderData* data, SandGame* game, float dt_s);
void Render_LoadAndSetImageResource(RenderData* data, GraphicResource rsc, const char* file_path);
void Render_LoadAndSetAnimationResource(RenderData* data, GraphicResource rsc, float refresh_period_s, const char* file_path);
float Render_GetFrameTime();
