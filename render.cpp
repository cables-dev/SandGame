#include "render.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <utility>

void WorldToScreen(double x, double y, double* out_x, double* out_y) {
	*out_x = x;
	*out_y = WINDOW_HEIGHT - y;
}

Color GameColourToRaylibColor(const GameColour* gc) {
	Color result{};
	result.r = gc->r;
	result.g = gc->g;
	result.b = gc->b;
	result.a = gc->a;
	return result;
}

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

void Render_FreeGraphicResourceIfPresent(RenderData* data, GraphicResource rsc) {
	assert(rsc >= 0 && rsc < GRAPHIC_RSC_MAX && "Render_FreeGraphicResourceIfPresent: Received invalid graphic resource.");
	auto* pigeonhole = &data->resources[rsc];
	if (GraphicResourceOpt_IsPresent(pigeonhole)) {
		GraphicResourceOpt_Free(pigeonhole);
	}
}

void Render_LoadAndSetImageResource(RenderData* data, GraphicResource rsc, const char* file_path) {
	assert(rsc >= 0 && rsc < GRAPHIC_RSC_MAX && "Render_LoadAndSetImageResource: Received invalid graphic resource.");

	Render_FreeGraphicResourceIfPresent(data, rsc);
	GraphicResourceOpt_CreateFromImage(&data->resources[rsc], file_path);
}

void Render_LoadAndSetAnimationResource(RenderData* data, GraphicResource rsc, float refresh_period_s, const char* file_path) {
	assert(rsc >= 0 && rsc < GRAPHIC_RSC_MAX && "Render_LoadAndSetAnimationResource: Received invalid graphic resource.");

	Render_FreeGraphicResourceIfPresent(data, rsc);
	GraphicResourceOpt_CreateFromAnimation(&data->resources[rsc], refresh_period_s, file_path);
}

float Render_GetFrameTime() {
	return GetFrameTime();
}

bool Render_IsGraphicResourceLoaded(const RenderData* data, GraphicResource rsc) {
	if (rsc < 0 || rsc >= GRAPHIC_RSC_MAX)
		return false;
	auto* resource = &data->resources[rsc];
	return GraphicResourceOpt_IsPresent(resource);
}

void Render_ResetGraphicResource(RenderData* data, GraphicResource rsc) {
	if (rsc < 0 || rsc >= GRAPHIC_RSC_MAX)
		return;

	auto* resource = &data->resources[rsc];
	return GraphicResourceOpt_Reset(resource);
}

void Render_DrawGraphicResource(
	RenderData* data, 
	GraphicResource rsc, 
	int x, int y, 
	Color colour, 
	float dt_s
) {
	assert(Render_IsGraphicResourceLoaded(data, rsc));

	auto* resource = &data->resources[rsc];
	GraphicResourceOpt_Draw(resource, x, y, colour, dt_s);
}

std::uint32_t Render_GetGraphicResourceWidth(const RenderData* data, GraphicResource rsc) {
	assert(Render_IsGraphicResourceLoaded(data, rsc));
	auto* resource = &data->resources[rsc];
	return GraphicResourceOpt_GetWidth(resource);
}

std::uint32_t Render_GetGraphicResourceHeight(const RenderData* data, GraphicResource rsc) {
	assert(Render_IsGraphicResourceLoaded(data, rsc));
	auto* resource = &data->resources[rsc];
	return GraphicResourceOpt_GetHeight(resource);
}

void RenderRectangleObstacle(RenderData* data, const EntityRectangleObstacle* rect, float dt_s) {
	double top_left_x_w = 0;
	double top_left_y_w = 0;
	double bottom_right_x_w = 0;
	double bottom_right_y_w = 0;
	AABB_GetCornerCoords(&rect->aabb, AABB_TOP_LEFT, &top_left_x_w, &top_left_y_w);
	AABB_GetCornerCoords(&rect->aabb, AABB_BOTTOM_RIGHT, &bottom_right_x_w, &bottom_right_y_w);

	double top_left_x = 0;
	double top_left_y = 0;
	double bottom_right_x = 0;
	double bottom_right_y = 0;
	WorldToScreen(top_left_x_w, top_left_y_w, &top_left_x, &top_left_y);
	WorldToScreen(bottom_right_x_w, bottom_right_y_w, &bottom_right_x, &bottom_right_y);

	auto w = bottom_right_x - top_left_x;
	auto h = bottom_right_y - top_left_y;
	auto color = GameColourToRaylibColor(&rect->colour);

	if (!rect->has_graphic)
		DrawRectangle(top_left_x - data->camera.start_x, top_left_y - data->camera.start_y, w, h, color);
	else
		Render_DrawGraphicResource(data, rect->graphic, top_left_x - data->camera.start_x, top_left_y - data->camera.start_y, color, dt_s);
}

void RenderBarrel(RenderData* data, const EntityBarrel* barrel, float dt_s) {
	double bottom_left_x_w = 0;
	double bottom_left_y_w = 0;
	AABB_GetCornerCoords(&barrel->aabb, AABB_BOTTOM_LEFT, &bottom_left_x_w, &bottom_left_y_w);

	double bottom_left_x = 0;
	double bottom_left_y = 0;
	WorldToScreen(bottom_left_x_w , bottom_left_y_w , &bottom_left_x, &bottom_left_y);

	auto sprite_rsc = (barrel->defused) ? GRAPHIC_RSC_BARREL_DEFUSED : barrel->active_sprite;
	auto needs_reset = barrel->sprite_changed;
	if (needs_reset)
		Render_ResetGraphicResource(data, sprite_rsc);

	auto sprite_h = Render_GetGraphicResourceHeight(data, sprite_rsc);
	Render_DrawGraphicResource(data, sprite_rsc, bottom_left_x - data->camera.start_x, bottom_left_y - sprite_h - data->camera.start_y, WHITE, dt_s);
}

void RenderLevelDoor(RenderData* data, const EntityLevelDoor* door, float dt_s) {
	double top_left_x_w = 0;
	double top_left_y_w = 0;
	double bottom_right_x_w = 0;
	double bottom_right_y_w = 0;
	AABB_GetCornerCoords(&door->aabb, AABB_TOP_LEFT, &top_left_x_w, &top_left_y_w);
	AABB_GetCornerCoords(&door->aabb, AABB_BOTTOM_RIGHT, &bottom_right_x_w, &bottom_right_y_w);

	double top_left_x = 0;
	double top_left_y = 0;
	double bottom_right_x = 0;
	double bottom_right_y = 0;
	WorldToScreen(top_left_x_w, top_left_y_w, &top_left_x, &top_left_y);
	WorldToScreen(bottom_right_x_w, bottom_right_y_w, &bottom_right_x, &bottom_right_y);

	auto w = bottom_right_x - top_left_x;
	auto h = bottom_right_y - top_left_y;
	auto color = Color{ 0xff, 0xbb, 0xff, 0xff };
	DrawRectangle(top_left_x - data->camera.start_x, top_left_y - data->camera.start_y, w, h, color);
}

GraphicResource Render_RenderLadybirdGetGraphicResource(RenderData* data, const EntityLadybird* ladybird) {
	auto direction = ladybird->direction;
	switch(ladybird->state) {
		case LADYBIRD_IDLE: { return (direction == ENTITY_FACING_LEFT) ? GRAPHIC_RSC_LADYBIRD_IDLE_LEFT : GRAPHIC_RSC_LADYBIRD_IDLE_RIGHT; }
		case LADYBIRD_MOVING: { return (direction == ENTITY_FACING_LEFT) ? GRAPHIC_RSC_LADYBIRD_WALK_LEFT : GRAPHIC_RSC_LADYBIRD_WALK_RIGHT; }
		case LADYBIRD_SHOCKED: { return GRAPHIC_RSC_LADYBIRD_SHOCKED; }
		case LADYBIRD_FLIGHT: { return GRAPHIC_RSC_LADYBIRD_FLIGHT; }
	}
}

void RenderLadybird(RenderData* data, const EntityLadybird* ladybird, float dt_s) {
	auto rsc = Render_RenderLadybirdGetGraphicResource(data, ladybird);
	if (ladybird->new_state)
		Render_ResetGraphicResource(data, rsc);

	auto bottom_left_x_w = 0.0;
	auto bottom_left_y_w = 0.0;
	Ladybird_GetFeet(ladybird, &bottom_left_x_w, &bottom_left_y_w);

	double bottom_left_x = 0;
	double bottom_left_y = 0;
	WorldToScreen(bottom_left_x_w, bottom_left_y_w, &bottom_left_x, &bottom_left_y);
	auto sprite_w = Render_GetGraphicResourceHeight(data, rsc);
	auto sprite_h = Render_GetGraphicResourceHeight(data, rsc);

	Render_DrawGraphicResource(data, rsc, bottom_left_x - data->camera.start_x, bottom_left_y - sprite_h - data->camera.start_y, WHITE, dt_s);
}

void RenderEntity(RenderData* renderer, const Entity* entity, float dt_s) {
	switch (entity->type) {
		case ENTITY_RECTANGLE: { RenderRectangleObstacle(renderer, &entity->entity.rect, dt_s); break; }
		case ENTITY_HINT_BOX: { /* pass */ break; }
		case ENTITY_BARREL: { RenderBarrel(renderer, &entity->entity.barrel, dt_s); break; }
		case ENTITY_LEVEL_DOOR: { RenderLevelDoor(renderer, &entity->entity.door, dt_s); break; }
		case ENTITY_LADYBIRD: { RenderLadybird(renderer, &entity->entity.ladybird, dt_s); break; }
		default: { assert("RenderEntity: Unaccounted entity type encountered." && false); }
	}
}

void Render_RenderEntities(RenderData* renderer, const SandGame* game, float dt_s) {
	static RenderData* rr;
	static float dt;
	rr = renderer;
	dt = dt_s;
	auto cb = [](Entity* ent) {
		RenderEntity(rr, ent, dt);
	};
	SandGame_ForEachEntity(game, cb);
}

void InitSandTexture(RenderData* renderer) {
	auto img = GenImageColor(PIT_SECTOR_WIDTH, PIT_SECTOR_HEIGHT, BLANK);
	renderer->sand_pixel_buffer = (Color*)malloc(PIT_SECTOR_WIDTH * PIT_SECTOR_HEIGHT * sizeof(Color));
	memset(renderer->sand_pixel_buffer, 0, PIT_SECTOR_WIDTH * PIT_SECTOR_HEIGHT * sizeof(Color));
	renderer->sand_texture = LoadTextureFromImage(img);
	UnloadImage(img);
}

void ResetRenderCamera(RenderCamera* camera) {
	camera->start_x = 0;
	camera->start_y = 0;
	camera->w = WINDOW_WIDTH;
	camera->h = WINDOW_HEIGHT;
}

void Render_InitWindow(RenderData* data, std::uint32_t window_w, std::uint32_t window_h, const char* window_name) {
	InitWindow(window_w, window_h, window_name);
	ToggleBorderlessWindowed();
}

void Render_SetSkybox(RenderData* data, GraphicResource skybox) {
	data->skybox = skybox;
}

void Render_Init(RenderData* data, std::uint32_t window_w, std::uint32_t window_h, std::uint8_t fps, const char* window_name, GraphicResource skybox) {
	Render_InitWindow(data, window_w, window_h, window_name);
	SetTargetFPS(fps);
	ResetRenderCamera(&data->camera);
	InitSandTexture(data);
	Render_SetSkybox(data, skybox);
}

void Render_FreeSandData(RenderData* data) {
	UnloadTexture(data->sand_texture);
	free(data->sand_pixel_buffer);
}

void Render_FreeGraphicResources(RenderData* data) {
	for (int i = 0; i < MAX_GRAPHIC_RESOURCES; i++) {
		GraphicResourceOpt_Free(&data->resources[i]);
	}
}

void Render_Shutdown(RenderData* renderer) {
	Render_FreeSandData(renderer);
	Render_FreeGraphicResources(renderer);
	CloseWindow();
}

void BeginRendering() {
	BeginDrawing();
}

void EndRendering() {
	EndDrawing();
}

void Render_RenderSand(const RenderData* r, const SandPit* pit) {
	// Hacky....
	static auto start_x = 0;
	static auto start_y = 0;
	static auto* sand_pixel_buffer = r->sand_pixel_buffer;		// ASSUMES BUFFER DOES NOT MOVE IN MEMORY...
	auto grain_size = pit->grain_size;
	auto sector_x = 0u;
	auto sector_y = 0u;
	start_x = r->camera.start_x / grain_size;
	start_y = r->camera.start_y / grain_size;

	SandPit_WorldCoordsToSectorCoords(pit, start_x, start_y, &sector_x, &sector_y);
	// Clear texture 
	// TODO: extract function
    memset(r->sand_pixel_buffer, 0, PIT_SECTOR_WIDTH * PIT_SECTOR_HEIGHT * sizeof(Color));
    auto cb = [](TWO_BIT_ID id, uint32_t x, uint32_t y) {
		sand_pixel_buffer[(PIT_SECTOR_HEIGHT - y - 1) * PIT_SECTOR_WIDTH + x] = PARTICLE_COLOURS[id];
    };
    SandPit_ForEachGrain(pit, sector_x, sector_y, cb);

    UpdateTexture(r->sand_texture, sand_pixel_buffer);
    DrawTextureEx(r->sand_texture, {0, 0}, 0.0, (float)grain_size, WHITE);
}

void Render_RenderPlayer(RenderData* r, const Player* player, float dt_s) {
	auto direction = Player_GetDirection(player);
	auto graphic = (direction == ENTITY_FACING_LEFT) ? GRAPHIC_RSC_PLAYER_IDLE_LEFT : GRAPHIC_RSC_PLAYER_IDLE_RIGHT;
	if (Player_IsMoving(player))
		graphic = (direction == ENTITY_FACING_LEFT) ? GRAPHIC_RSC_PLAYER_WALK_LEFT : GRAPHIC_RSC_PLAYER_WALK_RIGHT;

	auto start_x = r->camera.start_x;
	auto start_y = r->camera.start_y;
	double player_x_w = 0;
	double player_y_w = 0;
	AABB_GetCornerCoords(&player->bbox, AABB_BOTTOM, &player_x_w, &player_y_w);
	double player_w = Render_GetGraphicResourceWidth(r, graphic);
	double player_h = Render_GetGraphicResourceHeight(r, graphic);

	double player_x = 0;
	double player_y = 0;
	WorldToScreen(player_x_w, player_y_w, &player_x, &player_y);
	Render_DrawGraphicResource(r, graphic, player_x - player_w / 2.0 - start_x, player_y - player_h - start_y, WHITE, dt_s);

	//constexpr auto PLAYER_TRAIL_SCALER = 15.0;
	//if (Player_IsGoingFast(player)) {
	//	auto x_vel = player->x_speed;
	//	auto y_vel = player->y_speed;
	//	auto vel_mag = sqrt(x_vel * x_vel + y_vel * y_vel);
	//	auto trail_displacement_x = -(x_vel / vel_mag) * scale * PLAYER_TRAIL_SCALER;
	//	auto trail_displacement_y = (y_vel / vel_mag) * scale * PLAYER_TRAIL_SCALER;
	//
	//	// Draw Trail
	//	player_color.g = 255;
	//	player_color.r = 5;
	//	player_color.a *= 0.7;
	//	Rectangle player_rect_trail_1 = { 
	//		player_x - start_x + trail_displacement_x, 
	//		player_y - start_y + trail_displacement_y, 
	//		player_w, player_h 
	//	};
	//	DrawRectangleRec(player_rect_trail_1, player_color);

	//	player_color.a *= 0.9;
	//	Rectangle player_rect_trail_2 = { 
	//		player_x - start_x + trail_displacement_x + trail_displacement_x, 
	//		player_y - start_y + trail_displacement_y + trail_displacement_y, 
	//		player_w, player_h 
	//	};
	// 	DrawRectangleRec(player_rect_trail_2, player_color);
	//}
}

void Render_MoveCamera(RenderCamera* camera, const SandGame* game) {
	double player_x = 0;
	double player_y = 0;
	AABB_GetCornerCoords(&game->player.bbox, AABB_BOTTOM, &player_x, &player_y);
	double player_w = AABB_GetWidth(&game->player.bbox);
	double player_h = AABB_GetHeight(&game->player.bbox);

	auto start_x = ((int)player_x / camera->w) * camera->w;
	auto start_y = ((int)player_y / camera->h) * camera->h;
	camera->start_x = start_x;
	camera->start_y = start_y;
}

// Effects are drawn in UI stage.
void Render_ProcessFx(RenderData* r, RenderFXFlags* fx_flags) {
	if (RenderFXFlags_Get(*fx_flags, FX_REFRESH_TOAST)) {
		r->toast_display_duration = FX_TOAST_DISPLAY_DURATION;
	} 
	if (RenderFXFlags_Get(*fx_flags, FX_WHITE_FLASH)) {
		r->white_flash_duration = FX_WHITE_FLASH_DURATION_S;
	}
	if (RenderFXFlags_Get(*fx_flags, FX_BLACK_FADE_IN_OUT)) {
		r->black_fade_in_out_duration = FX_BLACK_FADE_IN_DURATION_S + FX_BLACK_FADE_OUT_DURATION_S;
	}
	*fx_flags = NULL_FX_FLAGS;
}

void Render_RenderToast(RenderData* r, const SandGame* game, float dt_s) {
	if (r->toast_display_duration - dt_s > 0) {
		auto* toast = game->toast;
		auto text_w = MeasureText(toast, FX_TOAST_FONT_SIZE);	
		auto text_color = FX_TOAST_FONT_COLOR;
		auto fade_denominator = FX_TOAST_DISPLAY_DURATION - FX_TOAST_FADE_AFTER;
		r->toast_display_duration -= dt_s;
		if (r->toast_display_duration <= fade_denominator) {
			text_color.a = (std::uint8_t)((r->toast_display_duration * 255.0) / fade_denominator);
		}

		auto center_x = r->camera.w / 2;
		auto center_y = r->camera.h / 2;
		DrawText(game->toast, center_x - text_w / 2.0, center_y / 2, FX_TOAST_FONT_SIZE, text_color);
	}
	else {
		r->toast_display_duration = 0.0;
	}
}

void Render_RenderPlayerWeaponType(RenderData* r, const SandGame* game, float dt_s) {
	constexpr const char* weapon_names[FIRE_TYPE_MAX]{
		"Stream",
		"Place",													
		"Vacuum"
	};

	auto player_weapon = game->player.fire_mode;
	assert((player_weapon >= 0 && player_weapon < FIRE_TYPE_MAX) && "Render_RenderPlayerWeaponType: Unknown fire type.");
	auto* weapon_name = weapon_names[player_weapon];

	auto h = r->camera.h;
	auto text_h = WEAPON_NAME_FONT_SIZE;
	DrawText(weapon_name, 10, h - WEAPON_NAME_FONT_SIZE - 10, text_h, WEAPON_NAME_FONT_COLOR);
}

void Render_RenderPlayerWeaponAmmo(RenderData* r, const SandGame* game, float dt_s) {
	auto ammo = game->player.ammo;
	auto capacity = PLAYER_SAND_CAPACITY;
	auto completion = (float)ammo / (float)capacity;
	auto bar_h = 100;
	auto hot_height = completion * bar_h;
	auto not_height = (1.0 - completion) * bar_h;

	auto h = r->camera.h;
	DrawRectangle(10, h - 40 - hot_height, 4, hot_height, WEAPON_NAME_FONT_COLOR);
	DrawRectangle(10, h - 40 - bar_h, 4, not_height, Color{0xaa, 0x10, 0x44, 0xff});
}

void Render_RenderWhiteFlash(RenderData* r, float dt_s) {
	if (r->white_flash_duration - dt_s <= 0.0) {
		r->white_flash_duration = -1.0;
		return;
	}

	r->white_flash_duration -= dt_s;
	auto completion = r->white_flash_duration / FX_WHITE_FLASH_DURATION_S;
	auto alpha = (uint8_t)(254.0 * completion);											// 254 to prevent potential integer overflow
	Color display{ 0xff, 0xff, 0xff, alpha };
	DrawRectangle(0, 0, r->camera.w, r->camera.h, display);
}

void Render_RenderBlackFadeInOut(RenderData* r, float dt_s) {
	if (r->black_fade_in_out_duration - dt_s <= 0.0) {
		r->black_fade_in_out_duration = -1.0;
		return;
	}

	r->black_fade_in_out_duration -= dt_s;
	auto alpha = 0xff;
	// fade out (after)
	if (r->black_fade_in_out_duration <= FX_BLACK_FADE_OUT_DURATION_S) {
		auto completion = r->black_fade_in_out_duration / FX_BLACK_FADE_OUT_DURATION_S;
		alpha = (uint8_t)(254.0 * completion);		// 254 to prevent potential integer overflow
	}
	// fade in (before)
	else {
		auto completion = (r->black_fade_in_out_duration - FX_BLACK_FADE_OUT_DURATION_S) / FX_BLACK_FADE_IN_DURATION_S;
		alpha = (uint8_t)(254.0 * (1.0 - completion));		// 254 to prevent potential integer overflow
	}
	Color display{ 0x00, 0x00, 0x00, alpha };
	DrawRectangle(0, 0, r->camera.w, r->camera.h, display);
}

const char* Render_GetElapsedTimeString(RenderData* r, const SandGame* game) {
	auto elapsed_seconds = SandGame_GetElapsedSeconds(game);
    auto minutes = (int)(SandGame_GetElapsedSeconds(game) / 60);
    auto remaining_seconds = fmod(elapsed_seconds, 60.0);
    auto seconds = (int)remaining_seconds;
    auto milliseconds = (int)((remaining_seconds - seconds) * 1000);
    snprintf(r->time_readout_buffer, sizeof(r->time_readout_buffer) - 1, "%02d:%02d.%03d", minutes, seconds, milliseconds);
	return r->time_readout_buffer;
}

// Chuddy says hi and foenem
void Render_RenderTimer(RenderData* r, const SandGame* game) {
	auto* readout = Render_GetElapsedTimeString(r, game);
	auto text_w = TIME_READOUT_X_OFFSET;
	DrawText(readout, r->camera.w - text_w, 10, TIME_READOUT_FONT_SIZE, WHITE);
}

void Render_RenderUI(RenderData* r, const SandGame* game, float dt_s) {
	Render_RenderPlayerWeaponType(r, game, dt_s);
	Render_RenderPlayerWeaponAmmo(r, game, dt_s);
	Render_RenderToast(r, game, dt_s);
	Render_RenderWhiteFlash(r, dt_s);
	Render_RenderTimer(r, game);
	Render_RenderBlackFadeInOut(r, dt_s);
	DrawFPS(20, 20);
}

void Render_DrawSkybox(RenderData* r, float dt_s) {
	assert(Render_IsGraphicResourceLoaded(r, r->skybox));
	Render_DrawGraphicResource(r, r->skybox, 0, 0, WHITE, dt_s);
}

void Render_RenderGame(RenderData* r, SandGame* game, float dt_s) {
	BeginRendering();

	Render_MoveCamera(&r->camera, game);
	Render_DrawSkybox(r, dt_s);
	Render_ProcessFx(r, &game->fx_flags);
	Render_RenderEntities(r, game, dt_s);
	Render_RenderPlayer(r, &game->player, dt_s);
	Render_RenderSand(r, &game->pit);
	Render_RenderUI(r, game, dt_s);

	EndRendering();
}

bool Render_ShouldGameClose(const RenderData* data) {
	return WindowShouldClose();
}

