#include "render.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <utility>

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
void WorldToScreen(double x, double y, double* out_x, double* out_y) {
	*out_x = x;
	*out_y = WINDOW_HEIGHT - y;
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
	if (!rect->has_graphic)
		EngineRender_DrawRectangle(&data->engine, top_left_x, top_left_y, w, h, &rect->colour);
	else
		EngineRender_DrawGraphicResource(&data->engine, rect->graphic, top_left_x, top_left_y, &rect->colour, dt_s);
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
		EngineRender_ResetGraphicResource(&data->engine, sprite_rsc);

	auto sprite_h = EngineRender_GetGraphicResourceHeight(&data->engine, sprite_rsc);
	EngineRender_DrawGraphicResource(&data->engine, sprite_rsc, bottom_left_x, bottom_left_y - sprite_h, &WHITE_COLOUR, dt_s);
}

void RenderLevelDoor(RenderData* data, const EntityLevelDoor* door, float dt_s) {
	double top_left_x_w = 0;/*-------------------------------------------------------------------------*/
	double top_left_y_w = 0;/*-------------------------------------------------------------------------*/
	double bottom_right_x_w = 0;/*---------------------------------------------------------------------*/
	double bottom_right_y_w = 0;/*---------------------------------------------------------------------*/
	AABB_GetCornerCoords(&door->aabb, AABB_TOP_LEFT, &top_left_x_w, &top_left_y_w);/*------------------*/
	AABB_GetCornerCoords(&door->aabb, AABB_BOTTOM_RIGHT, &bottom_right_x_w, &bottom_right_y_w);/*------*/
	/*-------------------------------------------------------------------------------------------------*/
	double top_left_x = 0;/*---------------------------------------------------------------------------*/
	double top_left_y = 0;/*---------------------------------------------------------------------------*/
	double bottom_right_x = 0;/*-------------------------------------thislooks-cool--------------------*/
	double bottom_right_y = 0;/*-----------------------------------------------------------------------*/
	WorldToScreen(top_left_x_w, top_left_y_w, &top_left_x, &top_left_y);/*-----------------------------*/
	WorldToScreen(bottom_right_x_w, bottom_right_y_w, &bottom_right_x, &bottom_right_y);/*-------------*/
	/*-------------------------------------------------------------------------------------------------*/
	auto w = bottom_right_x - top_left_x;/*------------------------------------------------------------*/
	auto h = bottom_right_y - top_left_y;/*------------------------------------------------------------*/
	auto colour = GameColour{ 0xffffffff };/*----------------------------------------------------------*/
	EngineRender_DrawRectangle(&data->engine, top_left_x, top_left_y, w, h, &colour);/*-- -------------*/
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
		EngineRender_ResetGraphicResource(&data->engine, rsc);

	auto bottom_left_x_w = 0.0;
	auto bottom_left_y_w = 0.0;
	Ladybird_GetFeet(ladybird, &bottom_left_x_w, &bottom_left_y_w);

	double bottom_left_x = 0;
	double bottom_left_y = 0;
	WorldToScreen(bottom_left_x_w, bottom_left_y_w, &bottom_left_x, &bottom_left_y);
	auto sprite_w = EngineRender_GetGraphicResourceHeight(&data->engine, rsc);
	auto sprite_h = EngineRender_GetGraphicResourceHeight(&data->engine, rsc);

	EngineRender_DrawGraphicResource(&data->engine, rsc, bottom_left_x, bottom_left_y - sprite_h, &WHITE_COLOUR, dt_s);
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

void Render_InitWindow(RenderData* data, std::uint32_t window_w, std::uint32_t window_h, const char* window_name) {
	InitWindow(window_w, window_h, window_name);
	ToggleBorderlessWindowed();
}

void Render_Init(RenderData* data, std::uint32_t window_w, std::uint32_t window_h, std::uint8_t fps, const char* window_name, GraphicResource skybox) {
	EngineRender_Init(&data->engine, window_w, window_h, fps, window_name, true, skybox);
	InitSandTexture(data);

	data->screen_w = window_w;
	data->screen_h = window_h;
}

void Render_FreeSandData(RenderData* data) {
	UnloadTexture(data->sand_texture);
	free(data->sand_pixel_buffer);
}

void Render_Shutdown(RenderData* renderer) {
	Render_FreeSandData(renderer);
	EngineRender_Shutdown(&renderer->engine);
}

void BeginRendering() {
	BeginDrawing();
}

void EndRendering() {
	EndDrawing();
}

void Render_RenderSand(const RenderData* r, const SandPit* pit) {
	// Hacky....
	static auto start_x = 0.0;
	static auto start_y = 0.0;
	auto sector_x = 0u;
	auto sector_y = 0u;
	static auto* sand_pixel_buffer = r->sand_pixel_buffer;		// ASSUMES BUFFER DOES NOT MOVE IN MEMORY...
	EngineRender_GetCameraOrigin(&r->engine, &start_x, &start_y);

	auto grain_size = pit->grain_size;
	start_x /= grain_size;
	start_y /= grain_size;
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

	double player_x_w = 0;
	double player_y_w = 0;
	double player_w = EngineRender_GetGraphicResourceWidth(&r->engine, graphic);
	double player_h = EngineRender_GetGraphicResourceHeight(&r->engine, graphic);
	AABB_GetCornerCoords(&player->bbox, AABB_BOTTOM, &player_x_w, &player_y_w);

	double player_x = 0;
	double player_y = 0;
	WorldToScreen(player_x_w, player_y_w, &player_x, &player_y);
	EngineRender_DrawGraphicResource(&r->engine, graphic, player_x - player_w / 2.0, player_y - player_h, &WHITE_COLOUR, dt_s);

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

void Render_MoveCamera(RenderData* r, const SandGame* game) {
	double player_x = 0.0;
	double player_y = 0.0;
	double player_w = AABB_GetWidth(&game->player.bbox);
	double player_h = AABB_GetHeight(&game->player.bbox);
	auto screen_w = r->screen_w;
	auto screen_h = r->screen_h;
	AABB_GetCornerCoords(&game->player.bbox, AABB_BOTTOM, &player_x, &player_y);

	auto start_x = ((int)player_x / (int)screen_w) * screen_w;
	auto start_y = ((int)player_y / (int)screen_h) * screen_h;
	EngineRender_MoveCameraTo(&r->engine, start_x, start_y);
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

	auto h = r->screen_h;
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

	auto h = r->screen_h;
	DrawRectangle(10, h - 40 - hot_height, 4, hot_height, WEAPON_NAME_FONT_COLOR);
	DrawRectangle(10, h - 40 - bar_h, 4, not_height, Color{0xaa, 0x10, 0x44, 0xff});
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
	EngineRender_DrawTextAbsolute(&r->engine, readout, r->screen_w - text_w, 10, TIME_READOUT_FONT_SIZE, &TIMER_READOUT_FONT_COLOUR);
}

void Render_SetToast(RenderData* r, const SandGame* game, float dt_s) {
	EngineRender_SetToast(&r->engine, game->toast);
}

void Render_RenderUI(RenderData* r, const SandGame* game, float dt_s) {
	Render_RenderPlayerWeaponType(r, game, dt_s);
	Render_RenderPlayerWeaponAmmo(r, game, dt_s);
	Render_SetToast(r, game, dt_s);
	Render_RenderTimer(r, game);
}

void Render_RenderGame(RenderData* r, SandGame* game, EngineTime dt) {
	EngineRender_Begin(&r->engine);

	auto dt_s = dt.seconds;
	Render_MoveCamera(r, game);
	Render_RenderEntities(r, game, dt_s);
	Render_RenderPlayer(r, &game->player, dt_s);
	Render_RenderSand(r, &game->pit);
	Render_RenderUI(r, game, dt_s);
	EngineRender_Draw(&r->engine, &game->fx_flags, dt);

	EngineRender_End();
}

bool Render_ShouldGameClose(const RenderData* data) {
	return EngineRender_ShouldGameClose(&data->engine);
}

