#pragma once
#include "engine_common.hpp"
#include "raylib.h"

constexpr auto ENGINE_WHITE_COLOUR = GameColour{ 0xFFFFFFFF };
constexpr auto FX_TOAST_DISPLAY_DURATION = 4.0;
constexpr auto FX_TOAST_FADE_AFTER = 3.3;
constexpr auto FX_TOAST_FONT_SIZE = 30;
constexpr auto FX_TOAST_FONT_COLOUR = ENGINE_WHITE_COLOUR;
constexpr auto FX_WHITE_FLASH_DURATION_S = 1.0;	
constexpr auto FX_BLACK_FADE_IN_DURATION_S = 1.5;		// Should be the same as DOOR_LOADING_DELAY_S.
constexpr auto FX_BLACK_FADE_OUT_DURATION_S = 0.5;		

struct RenderCamera {
	AABB aabb{};
};
void RenderCamera_Create(RenderCamera* camera, double top_left_x, double top_left_y, double screen_w, double screen_h);

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

using RenderFXFlags = std::uint32_t;
enum RenderFXFlag {
	FX_REFRESH_TOAST = 1 << 0,
	FX_WHITE_FLASH = 1 << 1,
	FX_BLACK_FADE_IN_OUT = 1 << 2,
	FX_MAX = 1 << 3 // Remeber to update me!
};
using GraphicResource = int;
constexpr auto MAX_GRAPHIC_RESOURCES = 64;
constexpr RenderFXFlags NULL_FX_FLAGS = 0;

RenderFXFlag RenderFXFlag_FromResource(GraphicResource rsc);
bool RenderFXFlags_Get(RenderFXFlags flags, RenderFXFlag flag);
void RenderFXFlags_Set(RenderFXFlags* flags, RenderFXFlag flag, bool to=true);

struct EngineRenderData;
struct EngineBackgroundFlatColour {
	GameColour flat_colour{};
	double w{};
	double h{};
};
struct EngineBackground {
	bool using_resource = false;
	union {
		EngineBackgroundFlatColour flat_colour{};
		GraphicResource resource;
	};
};
void EngineBackground_Create(EngineBackground* bg, const GameColour* colour, double screen_w, double screen_h);
void EngineBackground_Create(EngineBackground* bg, GraphicResource rsc);
void EngineBackground_Draw(EngineRenderData* data, const EngineBackground* bg, float dt_s);

struct EngineRenderData {
	RenderCamera camera{};
	EngineBackground background{};
	char* toast = nullptr;						// Since callers might pass dynamically allocated strings with
	float toast_display_duration = 0.0;			// no lifetime guarantee, we will make a local copy.
	float white_flash_duration = 0.0;
	float black_fade_in_out_duration = 0.0;
	GraphicResourceOpt resources[MAX_GRAPHIC_RESOURCES]{};
};

// TODO: ON WINDOW RESIZE (change camera pos, resize background)
void EngineRender_Init(
	EngineRenderData* data,
	std::uint32_t window_w,
	std::uint32_t window_h,
	std::uint32_t fps,
	const char* window_name,
	bool borderless_windowed,
	GraphicResource background,
	std::uint32_t camera_x = 0,
	std::uint32_t camera_y = 0
);
void EngineRender_Init(
	EngineRenderData* data,
	std::uint32_t window_w,
	std::uint32_t window_h,
	std::uint32_t fps,
	const char* window_name,
	bool borderless_windowed,
	const GameColour* background,
	std::uint32_t camera_x = 0,
	std::uint32_t camera_y = 0
);
void EngineRender_Shutdown(EngineRenderData* data);
bool EngineRender_ShouldGameClose(const EngineRenderData* data);
void EngineRender_LoadAndSetImageResource(EngineRenderData* data, GraphicResource rsc, const char* file_path);
void EngineRender_LoadAndSetAnimationResource(EngineRenderData* data, GraphicResource rsc, float refresh_period_s, const char* file_path);
void EngineRender_ResetGraphicResource(EngineRenderData* data, GraphicResource rsc);
std::uint32_t EngineRender_GetGraphicResourceWidth(const EngineRenderData* data, GraphicResource rsc);
std::uint32_t EngineRender_GetGraphicResourceHeight(const EngineRenderData* data, GraphicResource rsc);

void EngineRender_MoveCameraTo(EngineRenderData* data, double top_left_x, double top_left_y);
void EngineRender_MoveCameraBy(EngineRenderData* data, double dx, double dy);
double EngineRender_GetCameraWidth(const EngineRenderData* data);
double EngineRender_GetCameraHeight(const EngineRenderData* data);
void EngineRender_GetCameraOrigin(const EngineRenderData* data, double* out_x, double* out_y);
bool EngineRender_IsPointOnScreen(const EngineRenderData* data, double x, double y);
bool EngineRender_IsOnScreen(const EngineRenderData* data, const AABB* aabb);
void EngineRender_SetToast(EngineRenderData* data, const char* text);

void EngineRender_Begin(EngineRenderData* data);
void EngineRender_Begin(EngineRenderData* data, EngineTime dt);		// If you are using an animated background you must call this function in order for it to update.
void EngineRender_Draw(EngineRenderData* data, RenderFXFlags* fx_flags, EngineTime dt);
void EngineRender_End();

// ioctl
void EngineRender_SetFramerate(EngineRenderData* data, std::uint32_t fps);

// Drawing fns all internally defer to raylib
// Currently adding functions when needed...
void EngineRender_DrawGraphicResourceAbsolute(
	EngineRenderData* data,
	GraphicResource rsc,
	int x, int y,
	const GameColour* colour,
	float dt_s
);
void EngineRender_DrawGraphicResource(
	EngineRenderData* data,
	GraphicResource rsc,
	int x, int y,
	const GameColour* colour,
	float dt_s
);
void EngineRender_DrawTextAbsolute(
	const EngineRenderData* data, 
	const char* text,
	float x,
	float y,
	float font_size,
	const GameColour* colour
);
double EngineRender_MeasureTextWidth(
	const EngineRenderData* data,
	const char* text,
	float font_size
);
void EngineRender_DrawText(						// Camera-corrected
	const EngineRenderData* data, 
	const char* text,
	float x,
	float y,
	float font_size,
	const GameColour* colour
);
void EngineRender_DrawRectangleAbsolute(
	const EngineRenderData* data, 
	float x,
	float y,
	float w,
	float h,
	const GameColour* colour
);
void EngineRender_DrawRectangle(				// Camera-corrected
	const EngineRenderData* data, 
	float x,
	float y,
	float w,
	float h,
	const GameColour* colour
);
void EngineRender_DrawRectangleOutlineAbsolute(
	const EngineRenderData* data, 
	float x,
	float y,
	float w,
	float h,
	float thickness,
	const GameColour* colour
);
void EngineRender_DrawRectangleOutline(				// Camera-corrected
	const EngineRenderData* data, 
	float x,
	float y,
	float w,
	float h,
	float thickness,
	const GameColour* colour
);
void EngineRender_DrawAABBAbsolute(
	const EngineRenderData* data,
	const AABB* aabb,
	const GameColour* colour
);
void EngineRender_DrawAABB(				// Camera-corrected
	const EngineRenderData* data,
	const AABB* aabb,
	const GameColour* colour
);
void EngineRender_DrawAABBOutlineAbsolute(
	const EngineRenderData* data,
	const AABB* aabb,
	float thickness,
	const GameColour* colour
);
void EngineRender_DrawAABBOutline(				// Camera-corrected
	const EngineRenderData* data,
	const AABB* aabb,
	float thickness,
	const GameColour* colour
);
