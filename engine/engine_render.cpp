#include "engine_render.hpp"
#include <cassert>
#include <string.h>
#include <cstdlib>

Color GameColourToRaylibColor(const GameColour* gc) {
	Color result{};
	result.r = gc->r;
	result.g = gc->g;
	result.b = gc->b;
	result.a = gc->a;
	return result;
}

void GameColourToRaylibColor(const GameColour* gc, Color* out_color) {
	*out_color = GameColourToRaylibColor(gc);
}

void RenderCamera_Create(RenderCamera* camera, double top_left_x, double top_left_y, double screen_w, double screen_h) {
	AABB_Create(&camera->aabb, top_left_x, top_left_y, screen_w, screen_h);
}

RenderFXFlag RenderFXFlag_FromResource(GraphicResource rsc) {
	return RenderFXFlag(1ull << rsc);
}

bool RenderFXFlags_Get(RenderFXFlags flags, RenderFXFlag flag) {
	return flags & flag;
}

void RenderFXFlags_Set(RenderFXFlags* flags, RenderFXFlag flag, bool to) {
	if (to) {
		*flags |= flag;
	}
	else {
		*flags &= ~flag;
	}
}

void EngineBackground_Create(EngineBackground* bg, const GameColour* colour, double w, double h) {
	bg->using_resource = false;
	bg->flat_colour = EngineBackgroundFlatColour{ *colour, w, h };
}

void EngineBackground_Create(EngineBackground* bg, GraphicResource rsc) {
	bg->using_resource = true;
	bg->resource = rsc;
}

void EngineBackground_DrawFlatColour(EngineRenderData* r, const EngineBackgroundFlatColour* bg, float dt_s) {
	EngineRender_DrawRectangleAbsolute(r, 0, 0, bg->w, bg->h, &bg->flat_colour);
}

void EngineBackground_DrawGraphicResource(EngineRenderData* r, GraphicResource bg, float dt_s) {
	EngineRender_DrawGraphicResourceAbsolute(r, bg, 0, 0, &ENGINE_WHITE_COLOUR, dt_s);
}

void EngineBackground_Draw(EngineRenderData* r, const EngineBackground* bg, float dt_s);

bool GraphicResourceOpt_IsPresent(const GraphicResourceOpt* opt) {
	return opt->present;
}

void GraphicResourceImage_Free(GraphicResourceImage* image) {
	if (image == nullptr)
		return;

	UnloadTexture(image->texture);
}

// Returns false on failure.
bool GraphicResourceImage_Load(GraphicResourceImage* image, const char* file_path) {
	Image disc_image = LoadImage(file_path);
	auto success = IsImageValid(disc_image);
	if (!success)
		return false;

	image->texture = LoadTextureFromImage(disc_image);
	image->w = disc_image.width;
	image->h = disc_image.height;

	UnloadImage(disc_image);
	return true;
}

void GraphicResourceImage_Draw(const GraphicResourceImage* image, int x, int y, Color tint) {
	DrawTexture(image->texture, x, y, tint);
}

std::uint32_t GraphicResourceImage_GetWidth(const GraphicResourceImage* image) {
	return image->w;
}

std::uint32_t GraphicResourceImage_GetHeight(const GraphicResourceImage* image) {
	return image->h;
}

// Returns false on failure.
bool GraphicResourceAnimation_Load(GraphicResourceAnimation* anim, float refresh_period_s, const char* file_path) {
	auto num_frames = 0;
	Image disc_anim = LoadImageAnim(file_path, &num_frames);
	auto success = IsImageValid(disc_anim);
	if (!success || num_frames == 0)
		return false;

	anim->texture = LoadTextureFromImage(disc_anim);
	anim->sprite = disc_anim;
	anim->refresh_period_s = refresh_period_s;					// Raylib uses float for timings, so we comply
	anim->time_until_update_s = refresh_period_s;
	anim->num_frames = num_frames;
	anim->current_frame = 0;
	anim->w = disc_anim.width;
	anim->h = disc_anim.height;
	return true;
}

void GraphicResourceAnimation_Reset(GraphicResourceAnimation* anim) {
	anim->time_until_update_s = anim->refresh_period_s;
	anim->current_frame = 0;
}

void GraphicResourceAnimation_Free(GraphicResourceAnimation* anim) {
	if (anim == nullptr)
		return;

	UnloadTexture(anim->texture);
	UnloadImage(anim->sprite);
}

std::uint32_t GraphicResourceAnimation_GetWidth(const GraphicResourceAnimation* anim) {
	return anim->w;
}

std::uint32_t GraphicResourceAnimation_GetHeight(const GraphicResourceAnimation* anim) {
	return anim->h;
}

void GraphicResourceAnimation_NextFrame(GraphicResourceAnimation* anim) {
	auto next_frame_data_offset = anim->sprite.width * anim->sprite.height * sizeof(Color) * anim->current_frame;
	UpdateTexture(anim->texture, ((unsigned char*)anim->sprite.data) + next_frame_data_offset);
}

void GraphicResourceAnimation_Draw(GraphicResourceAnimation* anim, int x, int y, Color tint, float dt_s) {
	// Need update(s)?
	anim->time_until_update_s -= dt_s;
	bool next_frame = anim->time_until_update_s <= 0.0;
	if (next_frame) {
		auto num_updates = (-anim->time_until_update_s / anim->refresh_period_s) + 1;
		anim->current_frame = (anim->current_frame + (int)num_updates) % anim->num_frames;
		anim->time_until_update_s = (anim->refresh_period_s - (dt_s - anim->refresh_period_s * num_updates));
		GraphicResourceAnimation_NextFrame(anim);
	}
	DrawTexture(anim->texture, x, y, tint);
}

void GraphicResourceOpt_CreateFromImage(GraphicResourceOpt* opt, const char* file_path) {
	auto load_result = GraphicResourceImage_Load(&opt->resource.image, file_path);
	assert(load_result && "Render_LoadAndSetImageResource: Could not load provided file.");
	opt->type = GRAPHIC_RSC_TYPE_IMAGE;
	opt->present = true;
}

void GraphicResourceOpt_CreateFromAnimation(GraphicResourceOpt* opt, float refresh_period_s, const char* file_path) {
	auto load_result = GraphicResourceAnimation_Load(&opt->resource.animation, refresh_period_s, file_path);
	assert(load_result && "Render_LoadAndSetImageResource: Could not load provided file.");
	opt->type = GRAPHIC_RSC_TYPE_ANIMATION;
	opt->present = true;
}

void GraphicResourceOpt_Reset(GraphicResourceOpt* opt) {
	if (!opt->present)
		return;

	// Branch in future
	switch (opt->type) {
		case GRAPHIC_RSC_TYPE_IMAGE: { /* noop */; break; }
		case GRAPHIC_RSC_TYPE_ANIMATION: { GraphicResourceAnimation_Reset(&opt->resource.animation); break; }
		default: { assert(false && "GraphicResourceOpt_Reset: Unaccounted graphic resource type encountered."); }
	};
}

// Does no work if opt is empty.
void GraphicResourceOpt_Free(GraphicResourceOpt* opt) {
	if (!opt->present)
		return;

	// Branch in future
	switch (opt->type) {
		case GRAPHIC_RSC_TYPE_IMAGE: { GraphicResourceImage_Free(&opt->resource.image); break; }
		case GRAPHIC_RSC_TYPE_ANIMATION: { GraphicResourceAnimation_Free(&opt->resource.animation); break; }
		default: { assert(false && "GraphicResourceOpt_Free: Unaccounted graphic resource type encountered."); }
	};
}

void GraphicResourceOpt_Draw(GraphicResourceOpt* opt, int x, int y, Color tint, float dt_s) {
	if (!opt->present)
		return;

	switch (opt->type) {
		case GRAPHIC_RSC_TYPE_IMAGE: { GraphicResourceImage_Draw(&opt->resource.image, x, y, tint); break; }
		case GRAPHIC_RSC_TYPE_ANIMATION: { GraphicResourceAnimation_Draw(&opt->resource.animation, x, y, tint, dt_s); break; }
		default: { assert(false && "GraphicResourceOpt_Free: Unaccounted graphic resource type encountered."); }
	};
}

std::uint32_t GraphicResourceOpt_GetWidth(const GraphicResourceOpt* opt) {
	assert(opt->present && "GraphicResourceOpt_GetWidth: Requested graphic resource is not loaded.");

	switch (opt->type) {
		case GRAPHIC_RSC_TYPE_IMAGE: { return GraphicResourceImage_GetWidth(&opt->resource.image); break; }
		case GRAPHIC_RSC_TYPE_ANIMATION: { return GraphicResourceAnimation_GetWidth(&opt->resource.animation); break; }
		default: { assert(false && "GraphicResourceOpt_GetWidth: Unaccounted graphic resource type encountered."); }
	};
}

std::uint32_t GraphicResourceOpt_GetHeight(const GraphicResourceOpt* opt) {
	assert(opt->present && "GraphicResourceOpt_GetHeight: Requested graphic resource is not loaded.");

	switch (opt->type) {
		case GRAPHIC_RSC_TYPE_IMAGE: { return GraphicResourceImage_GetHeight(&opt->resource.image); break; }
		case GRAPHIC_RSC_TYPE_ANIMATION: { return GraphicResourceAnimation_GetHeight(&opt->resource.animation); break; }
		default: { assert(false && "GraphicResourceOpt_GetHeight: Unaccounted graphic resource type encountered."); }
	};
}

bool GraphicResource_IsValidValue(GraphicResource rsc) {
	return rsc >= 0 && rsc <= MAX_GRAPHIC_RESOURCES;
}

void EngineRender_FreeGraphicResourceIfPresent(EngineRenderData* data, GraphicResource rsc) {
	assert(GraphicResource_IsValidValue(rsc) && "Render_FreeGraphicResourceIfPresent: Received invalid graphic resource.");
	auto* pigeonhole = &data->resources[rsc];
	if (GraphicResourceOpt_IsPresent(pigeonhole)) {
		GraphicResourceOpt_Free(pigeonhole);
	}
}

void EngineRender_LoadAndSetImageResource(EngineRenderData* data, GraphicResource rsc, const char* file_path) {
	assert(GraphicResource_IsValidValue(rsc) && "Render_LoadAndSetImageResource: Received invalid graphic resource.");

	EngineRender_FreeGraphicResourceIfPresent(data, rsc);
	GraphicResourceOpt_CreateFromImage(&data->resources[rsc], file_path);
}

void EngineRender_LoadAndSetAnimationResource(EngineRenderData* data, GraphicResource rsc, float refresh_period_s, const char* file_path) {
	assert(GraphicResource_IsValidValue(rsc) && "Render_LoadAndSetAnimationResource: Received invalid graphic resource.");

	EngineRender_FreeGraphicResourceIfPresent(data, rsc);
	GraphicResourceOpt_CreateFromAnimation(&data->resources[rsc], refresh_period_s, file_path);
}

bool Render_IsGraphicResourceLoaded(const EngineRenderData* data, GraphicResource rsc) {
	if (!GraphicResource_IsValidValue(rsc))
		return false;
	auto* resource = &data->resources[rsc];
	return GraphicResourceOpt_IsPresent(resource);
}

void EngineRender_ResetGraphicResource(EngineRenderData* data, GraphicResource rsc) {
	if (!GraphicResource_IsValidValue(rsc))
		return;

	auto* resource = &data->resources[rsc];
	return GraphicResourceOpt_Reset(resource);
}

void EngineRender_GetCameraOrigin(const EngineRenderData* data, double* out_top_left_x, double* out_top_left_y) {
	*out_top_left_x = data->camera.aabb.top_left_x;
	*out_top_left_y = data->camera.aabb.top_left_y;
}

void EngineRender_DrawGraphicResourceAbsolute(
	EngineRenderData* data, 
	GraphicResource rsc, 
	int x, int y, 
	const GameColour* colour, 
	float dt_s
) {
	assert(Render_IsGraphicResourceLoaded(data, rsc));

	auto color = GameColourToRaylibColor(colour);
	auto* resource = &data->resources[rsc];
	GraphicResourceOpt_Draw(resource, x, y, color, dt_s);
}

void EngineRender_DrawGraphicResource(
	EngineRenderData* data,					// can't be const* since animation graphics need to be updated on render
	GraphicResource rsc,
	int x, int y,
	const GameColour* colour,
	float dt_s
) {
	double cam_origin_x;
	double cam_origin_y;
	EngineRender_GetCameraOrigin(data, &cam_origin_x, &cam_origin_y);
	EngineRender_DrawGraphicResourceAbsolute(data, rsc, x - cam_origin_x, y - cam_origin_y, colour, dt_s);
}

std::uint32_t EngineRender_GetGraphicResourceWidth(const EngineRenderData* data, GraphicResource rsc) {
	assert(Render_IsGraphicResourceLoaded(data, rsc));
	auto* resource = &data->resources[rsc];
	return GraphicResourceOpt_GetWidth(resource);
}

std::uint32_t EngineRender_GetGraphicResourceHeight(const EngineRenderData* data, GraphicResource rsc) {
	assert(Render_IsGraphicResourceLoaded(data, rsc));
	auto* resource = &data->resources[rsc];
	return GraphicResourceOpt_GetHeight(resource);
}

void EngineRender_InitWindow(const EngineRenderData* data, std::uint32_t window_w, std::uint32_t window_h, const char* window_name, bool borderless_windowed) {
	InitWindow(window_w, window_h, window_name);
	if (borderless_windowed)
		ToggleBorderlessWindowed();
}

void EngineRender_SetFramerate(const EngineRenderData* data, std::uint8_t fps) {
	SetTargetFPS(fps);
}

void EngineRender_SetupCamera(
	RenderCamera* camera, 
	std::uint32_t start_x, 
	std::uint32_t start_y, 
	std::uint32_t window_w, 
	std::uint32_t window_h
) {
	RenderCamera_Create(camera, start_x, start_y, window_w, window_h);
}

void EngineRender_SetFramerate(EngineRenderData* data, std::uint32_t fps) {
	SetTargetFPS(fps);
}

void EngineRender_InitBase(
	EngineRenderData* data, 
	std::uint32_t window_w, 
	std::uint32_t window_h, 
	std::uint32_t fps, 
	const char* window_name, 
	bool borderless_windowed, 
	std::uint32_t camera_x, 
	std::uint32_t camera_y
) {
	EngineRender_InitWindow(data, window_w, window_h, window_name, borderless_windowed);
	EngineRender_SetFramerate(data, fps);
	EngineRender_SetupCamera(&data->camera, camera_x, camera_y, window_w, window_h);
}

void EngineRender_FreeGraphicResources(EngineRenderData* data) {
	for (int i = 0; i < MAX_GRAPHIC_RESOURCES; i++) {
		GraphicResourceOpt_Free(&data->resources[i]);
	}
}

void EngineRender_CloseWindow(EngineRenderData* data) {
	CloseWindow();
}

void EngineRender_InitBackground(EngineRenderData* data, GraphicResource rsc) {
	EngineBackground_Create(&data->background, rsc);
}

void EngineRender_InitBackground(EngineRenderData* data, const GameColour* colour) {
	auto w = EngineRender_GetCameraWidth(data); 
	auto h = EngineRender_GetCameraHeight(data);
	EngineBackground_Create(&data->background, colour, w, h);
}

void EngineRender_Init(EngineRenderData* data, std::uint32_t window_w, std::uint32_t window_h, std::uint32_t fps, const char* window_name, bool borderless_windowed, GraphicResource background, std::uint32_t camera_x, std::uint32_t camera_y) {
	EngineRender_InitBackground(data, background);
	EngineRender_InitBase(data, window_w, window_h, fps, window_name, borderless_windowed, camera_x, camera_y);
}

void EngineRender_Init(EngineRenderData* data, std::uint32_t window_w, std::uint32_t window_h, std::uint32_t fps, const char* window_name, bool borderless_windowed, const GameColour* background, std::uint32_t camera_x, std::uint32_t camera_y) {
	EngineRender_InitBackground(data, background);
	EngineRender_InitBase(data, window_w, window_h, fps, window_name, borderless_windowed, camera_x, camera_y);
}

void EngineRender_Shutdown(EngineRenderData* data) {
	EngineRender_FreeGraphicResources(data);
}

bool EngineRender_ShouldGameClose(const EngineRenderData* data) {
	return WindowShouldClose();
}

void EngineRender_MoveCameraTo(EngineRenderData* data, double top_left_x, double top_left_y) {
	data->camera.aabb.top_left_x = top_left_x;
	data->camera.aabb.top_left_y = top_left_y;
}

void EngineRender_MoveCameraBy(EngineRenderData* data, double dx, double dy) {
	AABB_MoveBy(&data->camera.aabb, dx, dy);
}

double EngineRender_GetCameraWidth(const EngineRenderData* data) {
	return AABB_GetWidth(&data->camera.aabb);
}

double EngineRender_GetCameraHeight(const EngineRenderData* data) {
	return AABB_GetHeight(&data->camera.aabb);
}

bool EngineRender_IsPointOnScreen(const EngineRenderData* data, double x, double y) {
	return AABB_ContainsPoint(&data->camera.aabb, x, y);
}

bool EngineRender_IsOnScreen(const EngineRenderData* data, const AABB* aabb) {
	return AABB_Intersects(&data->camera.aabb, aabb);
}

void EngineRender_CopyAndSetToastText(EngineRenderData* data, char* str) {
	free(data->toast);
	auto str_len = strlen(str);
	auto* buffer = (char*)(malloc(str_len + 1));
	buffer[str_len] = '\0';
	memcpy_s(buffer, str_len + 1, str, str_len);

	data->toast = buffer;
}

// Since callers might pass dynamically allocated strings with
// no lifetime guarantee, we must make a local copy of toast text.
void EngineRender_SetToast(EngineRenderData* data, const char* text) {
	data->toast_display_duration = FX_TOAST_DISPLAY_DURATION;
	EngineRender_CopyAndSetToastText(data, (char*)text);
}

bool EngineRender_DoWhiteFlash(const EngineRenderData* data) {
	return data->white_flash_duration > 0.0;	
}

void EngineRender_ClearWhiteFlash(EngineRenderData* data) {
	data->white_flash_duration = -1.0;
}

void EngineRender_RenderWhiteFlash(const EngineRenderData* data) {
	auto display_w = EngineRender_GetCameraWidth(data);
	auto display_h = EngineRender_GetCameraHeight(data);
	auto completion = data->white_flash_duration / FX_WHITE_FLASH_DURATION_S;
	auto alpha = (uint8_t)(254.0 * completion);						// 254 to prevent potential integer overflow
	GameColour display{};
	display.r = 0xff;
	display.g = 0xff;
	display.b = 0xff;
	display.a = alpha;
	EngineRender_DrawRectangleAbsolute(data, 0, 0, display_w, display_h, &display);				// ? Is 0,0 platform-dependent?
}

void EngineRender_UpdateWhiteFlash(EngineRenderData* data, float dt_s) {
	auto was_active_last_frame = EngineRender_DoWhiteFlash(data);
	data->white_flash_duration -= dt_s;
	if (EngineRender_DoWhiteFlash(data)) {
		EngineRender_RenderWhiteFlash(data);
	}
	else if (was_active_last_frame) {				// Falling edge
		EngineRender_ClearWhiteFlash(data);
	}
}

void EngineRender_StartWhiteFlash(EngineRenderData* data) {
	data->white_flash_duration = FX_WHITE_FLASH_DURATION_S;
}

void EngineRender_DrawBackground(EngineRenderData* data, float dt_s) {
	EngineBackground_Draw(data, &data->background, dt_s);
}

void EngineRender_Begin(EngineRenderData* data) {
	BeginDrawing();
	EngineRender_DrawBackground(data, 0.0);
}

void EngineRender_Begin(EngineRenderData* data, EngineTime dt_s) {
	BeginDrawing();
	EngineRender_DrawBackground(data, EngineTime_GetTotalSeconds(&dt_s));
}

void EngineRender_StartBlackFadeInOut(EngineRenderData* data) {
	data->black_fade_in_out_duration = FX_BLACK_FADE_IN_DURATION_S + FX_BLACK_FADE_OUT_DURATION_S;
}

bool EngineRender_DoBlackFadeInOut(const EngineRenderData* data) {
	return data->black_fade_in_out_duration > 0.0;
}

void EngineRender_ClearBlackFadeInOut(EngineRenderData* data) {
	data->black_fade_in_out_duration = -1.0;
}

void EngineRender_RenderBlackFadeInOut(const EngineRenderData* data) {
	auto display = GameColour{ 0x000000FF };
	auto display_w = EngineRender_GetCameraWidth(data);
	auto display_h = EngineRender_GetCameraHeight(data);

	// fade out (after)
	if (data->black_fade_in_out_duration <= FX_BLACK_FADE_OUT_DURATION_S) {
		auto completion = data->black_fade_in_out_duration / FX_BLACK_FADE_OUT_DURATION_S;
		display.a = (uint8_t)(254.0 * completion);		// 254 to prevent potential integer overflow
	}
	// fade in (before)
	else {
		auto completion = (data->black_fade_in_out_duration - FX_BLACK_FADE_OUT_DURATION_S) / FX_BLACK_FADE_IN_DURATION_S;
		display.a = (uint8_t)(254.0 * (1.0 - completion));		// 254 to prevent potential integer overflow
	}

	EngineRender_DrawRectangleAbsolute(data, 0, 0, display_w, display_h, &display);
}

void EngineRender_UpdateBlackFadeInOut(EngineRenderData* data, float dt_s) {
	auto was_active_last_frame = EngineRender_DoBlackFadeInOut(data);
	data->black_fade_in_out_duration -= dt_s;
	if (EngineRender_DoBlackFadeInOut(data)) {
		EngineRender_RenderBlackFadeInOut(data);
	}
	else if (was_active_last_frame) {				// Falling edge
		EngineRender_ClearBlackFadeInOut(data);
	}
}

void EngineRender_ProcessFX(EngineRenderData* data, RenderFXFlags* fx_flags) {
	if (RenderFXFlags_Get(*fx_flags, FX_WHITE_FLASH)) {
		EngineRender_StartWhiteFlash(data);
	}
	if (RenderFXFlags_Get(*fx_flags, FX_BLACK_FADE_IN_OUT)) {
		EngineRender_StartBlackFadeInOut(data);
	}
}

bool EngineRender_DoDrawToast(const EngineRenderData* r) {
	return r->toast_display_duration > 0.0;
}

void EngineRender_ClearToast(EngineRenderData* r) {
	r->toast = nullptr;
	r->toast_display_duration = -1.0;
}

void EngineRender_RenderToast(EngineRenderData* r) {
	auto* toast = r->toast;
	if (!toast)
		return;

	auto text_color = FX_TOAST_FONT_COLOUR;
	auto fade_denominator = FX_TOAST_DISPLAY_DURATION - FX_TOAST_FADE_AFTER;
	if (r->toast_display_duration <= fade_denominator) {
		text_color.a = (std::uint8_t)((r->toast_display_duration * 255.0) / fade_denominator);
	}

	auto text_w = MeasureText(toast, FX_TOAST_FONT_SIZE);	
	auto draw_x = EngineRender_GetCameraWidth(r) / 2.0;
	auto draw_y = EngineRender_GetCameraHeight(r) / 2.0;
	EngineRender_DrawTextAbsolute(r, toast, draw_x - (text_w / 2.0), draw_y / 2, FX_TOAST_FONT_SIZE, &text_color);
}

void EngineRender_UpdateToast(EngineRenderData* r, float dt_s) {
	auto was_active_last_frame = EngineRender_DoDrawToast(r);
	r->toast_display_duration -= dt_s;
	if (EngineRender_DoDrawToast(r)) {
		EngineRender_RenderToast(r);
	}
	else if (was_active_last_frame) {				// Falling edge
		EngineRender_ClearToast(r);
	}
}


void EngineRender_DrawFX(EngineRenderData* r, float dt_s) {
	EngineRender_UpdateToast(r, dt_s);
	EngineRender_UpdateWhiteFlash(r, dt_s);
	EngineRender_UpdateBlackFadeInOut(r, dt_s);
	DrawFPS(20, 20);
}

void EngineRender_UpdateFXFlags(RenderFXFlags* fx_flags) {
	*fx_flags = NULL_FX_FLAGS;
}

void EngineRender_Draw(EngineRenderData* data, RenderFXFlags* fx_flags, EngineTime dt) {
	auto dt_s = EngineTime_GetTotalSeconds(&dt);
	EngineRender_ProcessFX(data, fx_flags);
	EngineRender_DrawFX(data, dt_s);
	EngineRender_UpdateFXFlags(fx_flags);
}

void EngineRender_End() {
	EndDrawing();
}

void EngineRender_DrawTextAbsolute(const EngineRenderData* data, const char* text, float x, float y, float font_size, const GameColour* colour) {
	Color raylib_color;
	GameColourToRaylibColor(colour, &raylib_color);
	DrawText(text, x, y, font_size, raylib_color);
}

double EngineRender_MeasureTextWidth(const EngineRenderData* data, const char* text, float font_size) {
	return MeasureText(text, font_size);
}

void EngineRender_DrawText(const EngineRenderData* data, const char* text, float x, float y, float font_size, const GameColour* colour) {
	double origin_x;
	double origin_y;
	EngineRender_GetCameraOrigin(data, &origin_x, &origin_y);
	EngineRender_DrawTextAbsolute(data, text, x - origin_x, y - origin_y, font_size, colour);
}

void EngineRender_DrawRectangleAbsolute(const EngineRenderData* data, float x, float y, float w, float h, const GameColour* colour) {
	Color raylib_color;
	GameColourToRaylibColor(colour, &raylib_color);
	DrawRectangle(x, y, w, h, raylib_color);
}

void EngineRender_DrawRectangle(const EngineRenderData* data, float x, float y, float w, float h, const GameColour* colour) {
	double origin_x;
	double origin_y;
	EngineRender_GetCameraOrigin(data, &origin_x, &origin_y);
	EngineRender_DrawRectangleAbsolute(data, x - origin_x, y - origin_y, w, h, colour);
}

void EngineRender_DrawRectangleOutlineAbsolute(const EngineRenderData* data, float x, float y, float w, float h, float thickness, const GameColour* colour) {
	Rectangle rect{x, y, w, h};
	Color raylib_color;
	GameColourToRaylibColor(colour, &raylib_color);
	DrawRectangleLinesEx(rect, thickness, raylib_color);
}

void EngineRender_DrawRectangleOutline(const EngineRenderData* data, float x, float y, float w, float h, float thickness, const GameColour* colour) {
	double origin_x;
	double origin_y;
	EngineRender_GetCameraOrigin(data, &origin_x, &origin_y);
	EngineRender_DrawRectangleOutlineAbsolute(data, x - origin_x, y - origin_y, w, h, thickness, colour);
}

void EngineRender_DrawAABBAbsolute(const EngineRenderData* data, const AABB* aabb, const GameColour* colour) {
	auto x = 0.0;
	auto y = 0.0;
	auto w = AABB_GetWidth(aabb);
	auto h = AABB_GetHeight(aabb);
	AABB_GetCornerCoords(aabb, AABB_TOP_LEFT, &x, &y);
	EngineRender_DrawRectangleAbsolute(data, x, y, w, h, colour);
}

void EngineRender_DrawAABB(const EngineRenderData* data, const AABB* aabb, const GameColour* colour) {
	auto origin_x = 0.0;
	auto origin_y = 0.0;
	auto x = 0.0;
	auto y = 0.0;
	auto w = AABB_GetWidth(aabb);
	auto h = AABB_GetHeight(aabb);
	AABB_GetCornerCoords(aabb, AABB_TOP_LEFT, &x, &y);
	EngineRender_GetCameraOrigin(data, &origin_x, &origin_y);
	EngineRender_DrawRectangleAbsolute(data, x - origin_x, y - origin_y, w, h, colour);
}

void EngineRender_DrawAABBOutlineAbsolute(const EngineRenderData* data, const AABB* aabb, float thickness, const GameColour* colour) {
	double w = AABB_GetWidth(aabb);
	double h = AABB_GetHeight(aabb);
	Rectangle rect{ aabb->top_left_x, aabb->top_left_y, w, h };
	Color raylib_color;
	GameColourToRaylibColor(colour, &raylib_color);
	DrawRectangleLinesEx(rect, thickness, raylib_color);
}

void EngineRender_DrawAABBOutline(const EngineRenderData* data, const AABB* aabb, float thickness, const GameColour* colour){
	double origin_x;
	double origin_y;
	Color raylib_color;
	double w = AABB_GetWidth(aabb);
	double h = AABB_GetHeight(aabb);
	Rectangle rect{ aabb->top_left_x, aabb->top_left_y, w, h };
	GameColourToRaylibColor(colour, &raylib_color);
	DrawRectangleLinesEx(rect, thickness, raylib_color);
	EngineRender_GetCameraOrigin(data, &origin_x, &origin_y);
	rect.x -= origin_x;
	rect.y -= origin_y;
	DrawRectangleLinesEx(rect, thickness, raylib_color);
}

void EngineBackground_Draw(EngineRenderData* r, const EngineBackground* bg, float dt_s) {
	if (!bg->using_resource) {
		EngineBackground_DrawFlatColour(r, &bg->flat_colour, dt_s);
	}
	else {
		EngineBackground_DrawGraphicResource(r, bg->resource, dt_s);
	}
}
