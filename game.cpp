#include "game.hpp"
#include "raylib.h"
#include <cstdlib>
#include <vcruntime_string.h>			// memset
#include <corecrt_memcpy_s.h>		
#include <cassert>
#include <Math.h>
#include <utility>
#include <cstdlib>

void PlaceSandCircle(SandPit* p, std::int32_t x, std::int32_t y, std::int32_t r) {
	for (auto i_x = -r; i_x < r; i_x++) {
		for (auto i_y = -r; i_y < r; i_y++) {
			if (i_x * i_x + i_y * i_y < r * r) {
				SandPit_PlaceGrain(p, x + i_x, y + i_y, rand() % 4);
			}
		}
	}
}

void VacuumSand(SandPit* p, std::int32_t x, std::int32_t y, std::int32_t r, SandPitQueryResult* out_region_query) {
	for (auto i_x = -r; i_x < r; i_x++) {
		for (auto i_y = -r; i_y < r; i_y++) {
			if (i_x * i_x + i_y * i_y < r * r) {
				SandPit_AddImpulse(p, x + i_x, y + i_y, (i_x > 0) ? -1 : 1, (i_y > 0) ? -1 : 1);
			}
		}
	}

	AABB region{};
	AABB_Create(&region, x - (r/2.0), y + (r/2.0), r, r);
	*out_region_query = SandPit_QueryRegion(p, &region);
	SandPit_ClearRegion(p, &region, false);
}

void BlowSand(SandPit* p, std::int32_t x, std::int32_t y, std::int32_t r, std::int32_t v_x, std::int32_t v_y) {
	for (auto i_x = -r; i_x < r; i_x++) {
		for (auto i_y = -r; i_y < r; i_y++) {
			if (i_x * i_x + i_y * i_y < r * r) {
				SandPit_AddImpulse(p, x + i_x, y + i_y, v_x, v_y);
			}
		}
	}
}

void SetPlayerXVelocity(Player* player, double to) {
	player->x_speed = Bounds(to, PLAYER_MAX_SPEED, -PLAYER_MAX_SPEED);
}

void IncrementPlayerXVelocity(Player* player, double by) {
	SetPlayerXVelocity(player, player->x_speed + by);
}

void SetPlayerYVelocity(Player* player, double to) {
	player->y_speed = Bounds(to, PLAYER_MAX_SPEED, -PLAYER_MAX_SPEED);
}

void IncrementPlayerYVelocity(Player* player, double by) {
	SetPlayerYVelocity(player, player->y_speed + by);
}

void ApplyPlayerFriction(Player* player, float dt) {
	// Slow to a halt
	auto friction_delta_speed = (player->x_speed > 0) ? -PLAYER_HORIZONTAL_SLOW_SPEED * dt : PLAYER_HORIZONTAL_SLOW_SPEED * dt;
	if (-PLAYER_HORIZONTAL_SLOW_SPEED_THRESH <= player->x_speed && player->x_speed <= PLAYER_HORIZONTAL_SLOW_SPEED_THRESH)
		SetPlayerXVelocity(player, 0.0);
	else
		IncrementPlayerXVelocity(player, friction_delta_speed);
}

PlayerFireType NextFireType(PlayerFireType type) {
	return (PlayerFireType)((type + 1) % FIRE_TYPE_MAX);
}

void DoPlayerMovement(Player* player, SandGame* game, bool left, bool right, bool jump, float dt) {
	auto* world = &game->pit;

	// Buttons
	if (left) {
		auto delta_speed = (player->x_speed > 0) ? PLAYER_HORIZONTAL_SLOW_SPEED : PLAYER_HORIZONTAL_SPEED;
		IncrementPlayerXVelocity(player, -delta_speed * dt);
	}
	else if (right) {
		auto delta_speed = (player->x_speed < 0) ? PLAYER_HORIZONTAL_SLOW_SPEED : PLAYER_HORIZONTAL_SPEED;
		IncrementPlayerXVelocity(player, delta_speed * dt);
	}
	else {
		ApplyPlayerFriction(player, dt);
	}

	auto player_bbox_sand_coords = player->bbox;
	AABB_ScaleByReciprocal(&player_bbox_sand_coords, world->grain_size);
	auto player_w_sand = AABB_GetWidth(&player_bbox_sand_coords);
	auto player_h_sand = AABB_GetHeight(&player_bbox_sand_coords);

	double player_x_sand = 0;
	double player_y_sand = 0;
	AABB_GetCornerCoords(&player_bbox_sand_coords, AABB_BOTTOM, &player_x_sand, &player_y_sand);
	double prev_player_x_sand = player_x_sand;
	double prev_player_y_sand = player_y_sand;

	constexpr auto headroom = 0;	// !Will be invalid if we change player height below a certain value...
	auto projected_player_x_sand = player_x_sand + (player->x_speed / world->grain_size) * dt;
	auto sand_query = SandPit_QueryRegion(world, (projected_player_x_sand - player_w_sand / 2), (player_y_sand + player_h_sand - headroom), player_w_sand, player_h_sand - 2 * headroom);

	if (sand_query.any_grain || sand_query.any_solid) {		// Did we intersect with a piece of sand?
		// Can we autowalk over it?
		auto highest_collision = std::max(sand_query.highest_grain_y, sand_query.highest_solid_y);
		auto overhead_sand_difference = highest_collision - player_y_sand;
		if (overhead_sand_difference < PLAYER_AUTO_JUMP_HEIGHT) {
			player_y_sand = highest_collision;
			player_x_sand = projected_player_x_sand;
		}
		else {
			// Collide with wall
			SetPlayerXVelocity(player, 0.0);
		}
	}
	else {
		player_x_sand = projected_player_x_sand;
	}

	auto sand_query_bottom = SandPit_QueryRegion(world, (player_x_sand - player_w_sand / 2), player_y_sand, player_w_sand, 1);
	auto is_falling = player_y_sand > 0 && (sand_query_bottom.grain_count + sand_query_bottom.solid_count == 0);
	if (is_falling)
		IncrementPlayerYVelocity(player, -G * dt);
	else if (jump) {
		SetPlayerYVelocity(player, PLAYER_JUMP_SPEED);
		IncrementPlayerXVelocity(player, (player->x_speed > 0) ? PLAYER_JUMP_SPEED_BOOST_X * dt : -PLAYER_JUMP_SPEED_BOOST_X * dt);

		// If our framerate is too high we will not move far enough to leave the ground,
		// so we give the player a little extra nudge.
		player_y_sand += PLAYER_JUMP_EPSILON * world->grain_size;
		SandGame_SetSFXFlag(game, SFX_JUMP);
	}

	player_y_sand += (player->y_speed / world->grain_size) * dt;

	// Snap y coordinate if we are intersecting with sand 
	auto sand_query_top = SandPit_QueryRegion(world, (player_x_sand - player_w_sand / 2), (player_y_sand + player_h_sand + 2), player_w_sand, player_h_sand );

	// Hitting our head
	if (sand_query_top.any_solid) {
		SetPlayerYVelocity(player, 0.0);
		player_y_sand = std::min(player_y_sand - 2, (sand_query_top.lowest_solid_y - player_h_sand));
	} else if (!jump && (sand_query_bottom.any_grain || sand_query_bottom.any_solid)) {	
		SetPlayerYVelocity(player, 0.0);
		player_y_sand = std::max(sand_query_bottom.highest_grain_y, sand_query_bottom.highest_solid_y) + 1;
	}

	player_y_sand = std::max(player_y_sand, (double)(LOWEST_Y_COORDINATE / world->grain_size));				// Prevent from falling under the map 
	if (player_x_sand - player_w_sand / 2.0 <= 0) {
		player_x_sand = player_w_sand / 2.0;
		SetPlayerXVelocity(player, 0.0);
	}
	if (player_x_sand + player_w_sand / 2.0 >= world->w) {
		player_x_sand = world->w - player_w_sand / 2.0;
		SetPlayerXVelocity(player, 0.0);
	}

	auto dx = player_x_sand - prev_player_x_sand;
	auto dy = player_y_sand - prev_player_y_sand;
	AABB_MoveBy(&player->bbox, dx * world->grain_size, dy * world->grain_size);
}

bool PlayerFireType_DoesConsumeSand(PlayerFireType t) {
	return t == FIRE_TYPE_STREAM || t == FIRE_TYPE_BURST;
}

void DoPlayerFireSand(Player* player, SandPit* world, bool fire_held, bool fire_press, bool switch_fire_mode, int mouse_x, int mouse_y) {
	if (switch_fire_mode) {
		player->fire_mode = NextFireType(player->fire_mode);
	}
	if (!fire_held && !fire_press)
		return;

	mouse_x /= world->grain_size;
	mouse_y /= world->grain_size;
	
	if (PlayerFireType_DoesConsumeSand(player->fire_mode) && player->ammo <= 0) {
		return;
	}

	auto player_bbox_sand_coords = player->bbox;
	AABB_ScaleByReciprocal(&player_bbox_sand_coords, world->grain_size);
	auto player_w_sand = AABB_GetWidth(&player_bbox_sand_coords);
	auto player_h_sand = AABB_GetHeight(&player_bbox_sand_coords);

	double player_x_sand = 0;
	double player_y_sand = 0;
	AABB_GetCornerCoords(&player_bbox_sand_coords, AABB_BOTTOM, &player_x_sand, &player_y_sand);

	bool lhs = mouse_x < (player_x_sand);
	auto sand_x = (lhs) ? (player_x_sand - player_w_sand) : (player_x_sand + player_w_sand);
	auto sand_y = (player_y_sand + 4);
	auto sight_vec_x = (double)mouse_x - (double)player_x_sand;
	auto sight_vec_y = (double)mouse_y - (double)player_y_sand;

	if (sight_vec_y > 0 && sight_vec_x <= 0 && sight_vec_x > -80) {
		sight_vec_x = -80;
	}
	else if (sight_vec_y > 0 && sight_vec_x >= 0 && sight_vec_x < 80) {
		sight_vec_x = 80;
	}
	auto sight_vec_mag = sqrt(sight_vec_x * sight_vec_x + sight_vec_y * sight_vec_y);
	auto unit_x = sight_vec_x / sight_vec_mag;
	auto unit_y = sight_vec_y / sight_vec_mag;
	if (player->fire_mode == FIRE_TYPE_STREAM && fire_held) {
		PlaceSandCircle(world, sand_x, sand_y, 1);
		BlowSand(
			world,
			sand_x,
			sand_y,
			2,
			unit_x * 8,
			unit_y * 8
		);
		player->ammo = std::max(0, player->ammo - 1);
	}
	else if (player->fire_mode == FIRE_TYPE_BURST && fire_press) {
		auto x_offset = (lhs) ? -7 : 7;
		PlaceSandCircle(world, (sand_x + x_offset), sand_y, 6);
		BlowSand(
			world,
			(sand_x + x_offset),
			sand_y,
			8,
			unit_x * 12,
			unit_y * 12 
		);
		player->ammo = std::max(0, player->ammo - 144);
	}
	else if (player->fire_mode == FIRE_TYPE_VACUUM && fire_held) {
		//if (player->ammo >= PLAYER_SAND_CAPACITY)					// Uncomment to prevent unlimited vacuum
		//	return;
		SandPitQueryResult sweep{};
		AABB_GetCornerCoords(&player_bbox_sand_coords, AABB_MIDDLE, &player_x_sand, &player_y_sand);
		VacuumSand(
			world,
			player_x_sand,
			player_y_sand,
			20,
			&sweep
		);
		player->ammo = std::min(PLAYER_SAND_CAPACITY, player->ammo + (int)sweep.grain_count);
	}
}

void Player_Create(Player* player, double player_x, double player_y, double player_w, double player_h) {
	AABB_Create(&player->bbox, player_x, player_y, player_w, player_h);
	player->x_speed = 0.0;
	player->y_speed = 0.0;
	player->do_jump = false;
	player->fire_mode = FIRE_TYPE_STREAM;
	player->ammo = PLAYER_SAND_CAPACITY;
}

void Player_UpdatePlayer(
	Player* player, 
	SandGame* game, 
	bool left, 
	bool right, 
	bool jump, 
	bool fire_held, 
	bool fire_press, 
	bool switch_fire_mode,
	int mouse_x,
	int mouse_y,
	float dt
) {
	DoPlayerMovement(player, game, left, right, jump, dt);
	DoPlayerFireSand(player, &game->pit, fire_held, fire_press, switch_fire_mode, mouse_x, mouse_y);
}

void SandGame_InsertEntityInList(SandGame* game, Entity* ent) {
	if (!ent)
		return;

	if (game->entities_head == nullptr) {
		// New head
		ent->_next = ent;
		ent->_prev = ent;
		game->entities_head = ent;
	}
	else {
		auto* head = game->entities_head;
		ent->_next = head->_next;
		ent->_next->_prev = ent;
		ent->_prev = head;
		head->_next = ent;
	}
}

void SandGame_RemoveEntityFromList(SandGame* game, Entity* ent) {
	assert(ent != game->entities_head && "SandGame_RemoveEntityFromList: Tried to remove the list head!");

	auto* next = ent->_next;
	auto* prev = ent->_prev;
	next->_prev = prev;
	prev->_next = next;
}

Entity* SandGame_FreeStoreAllocEntity(SandGame* game) {
	Entity* result;
	if (game->entities_avail == nullptr) {
		// Allocate from top
		result = game->entities_top;
        game->entities_top += 1;

        // Bounds check
		assert(game->entities_top < &game->entities[game->max_entities] && "SandGame: Entity list overflow!");
    }
    else {
        result = game->entities_avail;
        game->entities_avail = result->_next;
	}

	return result;
}

void SandGame_FreeStoreFreeEntity(SandGame* game, Entity* ent) {
	SandGame_RemoveEntityFromList(game, ent);
	ent->_next = game->entities_avail;
	game->entities_avail = ent;
}

void SandGame_CreateEntityList(SandGame* game, std::uint32_t max_entities) {
	auto entities_size = sizeof(Entity) * max_entities; 
	game->entities = (Entity*)malloc(entities_size);
	memset(game->entities, 0, entities_size);
	game->max_entities = max_entities + 1;					// + 1 for dummy list head entity

	game->entities_top = game->entities;
	game->entities_avail = nullptr;

	// Setup our list head
	auto* head = SandGame_FreeStoreAllocEntity(game);
	SandGame_InsertEntityInList(game, head);
}

void SandGame_DestroyEntityList(SandGame* game) {
	free(game->entities);
	game->entities = nullptr;
	game->entities_top = nullptr;
	game->entities_avail = nullptr;
	game->entities_head = nullptr;
}

void SandGame_Create(SandGame* game, double player_x, double player_y, double player_w, double player_h, std::uint32_t pit_w, std::uint32_t pit_h, std::uint32_t pit_num_screens_horizontal, std::uint32_t pit_num_screens_vertical, std::uint32_t pit_stubbornness, std::uint16_t pit_grain_size, std::uint32_t max_entities, SandGamePersistentState* persistent) {
	Player_Create(&game->player, player_x, player_y, player_w, player_h);
	SandPit_Create(&game->pit, pit_w, pit_h, pit_num_screens_horizontal, pit_num_screens_vertical, pit_stubbornness, pit_grain_size);
	SandGame_CreateEntityList(game, max_entities);

	if (persistent == nullptr)
		game->persistent = (SandGamePersistentState*)malloc(sizeof(SandGamePersistentState));
	else
		game->persistent = persistent;
}

void SandGame_Destroy(SandGame* game, SandGamePersistentState** out_persistent_state) {
	*out_persistent_state = game->persistent;
	SandGame_Destroy(game);
}

void SandGame_Destroy(SandGame* game) {
	free(game->persistent);					// game->persistent could be nullptr, but free will still succeed.
	SandPit_Destroy(&game->pit);
	SandGame_DestroyEntityList(game);
}

void UpdateSandPit(
	SandPit* pit, 
	bool place_button, 
	bool clear_pit, 
	int mouse_x, 
	int mouse_y, 
	std::uint32_t sand_sector_x,
	std::uint32_t sand_sector_y
) {
	auto mouse_x_raw = mouse_x;
	auto mouse_y_raw = mouse_y;
	auto mouse_x_corrected = mouse_x_raw / pit->grain_size;
	auto mouse_y_corrected = mouse_y_raw / pit->grain_size;

	if (place_button) {
		PlaceSandCircle(pit, mouse_x_corrected, PIT_SECTOR_HEIGHT - (mouse_y_corrected), 6);
	}
	if (clear_pit) {
		SandPit_Clear(pit, sand_sector_x, sand_sector_y);
	}

	SandPit_SimulateStep(pit, sand_sector_x, sand_sector_y);
}

void SandGame_NewUpdate(SandGame* game) {
	game->fx_flags = NULL_FX_FLAGS;
	game->sfx_flags = NULL_SFX_FLAGS;
}

void SandGame_ThinkEntities(SandGame* game, double dt) {
	static auto _dt = 0.0;
	static auto* _game = game;
	_dt = dt;
	_game = game;
	auto cb = [](Entity* e) {
		if (!SandGame_ShouldLoadNewLevel(_game))
			Entity_Think(e, _game, _dt);
		};
	SandGame_ForEachEntity(game, cb);
}

bool SandGame_IsFrozen(const SandGame* game) {
	return game->persistent->frozen_for_s > 0.0;
}

void SandGame_Update(SandGame* game, double dt) {
	std::uint32_t sector_x;
	std::uint32_t sector_y;
	double player_x_sand = 0;
	double player_y_sand = 0;
	double player_x = 0;
	double player_y = 0;

	SandGame_NewUpdate(game);

	if (SandGame_IsFrozen(game)) {
		game->persistent->frozen_for_s = std::max(0.0, game->persistent->frozen_for_s - dt);
		return;
	}

	AABB player_bbox = game->player.bbox;
	double player_w = AABB_GetWidth(&player_bbox);
	double player_h = AABB_GetHeight(&player_bbox);
	AABB_GetCornerCoords(&player_bbox, AABB_BOTTOM, &player_x, &player_y);		
	AABB_ScaleByReciprocal(&player_bbox, game->pit.grain_size);					// Convert to sand coordinates
	AABB_GetCornerCoords(&player_bbox, AABB_BOTTOM, &player_x_sand, &player_y_sand);		

	SandPit_WorldCoordsToSectorCoords(&game->pit, player_x_sand, player_y_sand, &sector_x, &sector_y);
	auto start_x = ((int)player_x / WINDOW_WIDTH) * WINDOW_WIDTH;
	auto start_y = ((int)player_y / WINDOW_HEIGHT) * WINDOW_HEIGHT;
	auto mouse_x = game->cursor_x + start_x;
	auto mouse_y = game->cursor_y + start_y;

	UpdateSandPit(
		&game->pit,
		GameActionFlags_Get(game->action_flags_held, ACTION_DBG_PLACE_SAND),
		GameActionFlags_Get(game->action_flags_pressed, ACTION_DBG_RESET),
		mouse_x,
		mouse_y,
		sector_x,
		sector_y
	);
	Player_UpdatePlayer(
		&game->player,
		game,
		GameActionFlags_Get(game->action_flags_held, ACTION_MOVE_LEFT),
		GameActionFlags_Get(game->action_flags_held, ACTION_MOVE_RIGHT),
		GameActionFlags_Get(game->action_flags_held, ACTION_JUMP),
		GameActionFlags_Get(game->action_flags_held, ACTION_FIRE),
		GameActionFlags_Get(game->action_flags_pressed, ACTION_FIRE),
		GameActionFlags_Get(game->action_flags_pressed, ACTION_SWITCH_FIRE_MODE),
		mouse_x,
		WINDOW_HEIGHT - mouse_y,
		dt
	);
	SandGame_ThinkEntities(game, dt);
}

Entity* SandGame_AddEntity(SandGame* game, void* entity, EntityType type) {
	auto ent = Entity_CreateFrom(entity, type);
	auto* list_element = SandGame_FreeStoreAllocEntity(game);
	*list_element = ent;
	SandGame_InsertEntityInList(game, list_element);
	Entity_Place(list_element, game);

	return list_element;
}

void SandGame_RemoveEntity(SandGame* game, Entity* entity) {
	Entity_Remove(entity, game);
	SandGame_RemoveEntityFromList(game, entity);
}

void SandGame_ForEachEntity(const SandGame* game, SandGameForEachEntityFn_t cb) {
	if (!game->entities_head)
		return;

	auto* ent = game->entities_head;
	ent = ent->_next;
	while (ent != game->entities_head) {
		cb(ent);
		ent = ent->_next;
	}
}

void SandGame_SetSFXFlag(SandGame* game, SoundFX sfx) {
	SoundFXFlags_Set(&game->sfx_flags, sfx);
}

void SandGame_SetNewLevelPath(SandGame* game, const char* new_level_path) {
	game->new_level_path = new_level_path;
}

const char* SandGame_GetNewLevelPath(const SandGame* game) {
	return game->new_level_path;
}

bool SandGame_ShouldLoadNewLevel(const SandGame* game) {
	return SandGame_GetNewLevelPath(game) != nullptr && !SandGame_IsFrozen(game);
}

void SandGame_NotifyLevelLoaded(SandGame* game) {
	game->new_level_path = nullptr;
}

void SandGame_FreezeFor(SandGame* game, double s) {
	game->persistent->frozen_for_s = s;
}

void SandGame_SetFXFlag(SandGame* game, RenderFX fx) {
	RenderFXFlags_Set(&game->fx_flags, fx);
}

void SandGame_SetToast(SandGame* game, const char* toast) {
	game->toast = toast;
	SandGame_SetFXFlag(game, FX_REFRESH_TOAST);
	SandGame_SetSFXFlag(game, SFX_TOAST_NOTIFY);
}



void RectangleObstacle_Create(EntityRectangleObstacle* rect, double top_left_x, double top_left_y, double w, double h, const GameColour colour) {
	rect->vtable = {
		RectangleObstacle_Place,
		RectangleObstacle_Remove,
		RectangleObstacle_Think
	};
	AABB_Create(&rect->aabb, top_left_x, top_left_y, w, h);
	rect->colour = colour;
}

void RectangleObstacle_Place(Entity* ent, SandGame* game) {
	auto* rect = &ent->entity.rect;
	auto aabb = rect->aabb;
	AABB_ScaleByReciprocal(&aabb, game->pit.grain_size);
	SandPit_PlaceSolidAABB(&game->pit, &aabb);
}

void RectangleObstacle_Remove(Entity* ent, SandGame* game) {
	auto* rect = &ent->entity.rect;
	SandPit_ClearRegion(&game->pit, &rect->aabb, true);
}

void RectangleObstacle_Think(Entity* rect, SandGame* game, double dt) {
	// pass
}

void HintBox_Create(EntityHintBox* box, const char* message, bool only_once, double top_left_x, double top_left_y, double w, double h) {
	box->vtable = {
		HintBox_Place,
		HintBox_Remove,
		HintBox_Think
	};
	AABB_Create(&box->aabb, top_left_x, top_left_y, w, h);
	box->message = message;
	box->triggered = false;
	box->only_once = only_once;
}

void HintBox_Place(Entity* box, SandGame* game) {
	// pass
}

void HintBox_Remove(Entity* box, SandGame* game) {
	// pass
}

void HintBox_Think(Entity* e, SandGame* game, double dt) {
	auto* box = &e->entity.hint_box;
	auto intersects = AABB_Intersects(&box->aabb, &game->player.bbox);
	if (!box->triggered && intersects) {
		box->triggered = true;
		SandGame_SetToast(game, box->message);
	} 
	if (!intersects && !box->only_once) {								// Setup hint for another trigger
		box->triggered = false;
	}
}

void Barrel_Create(EntityBarrel* barrel, double top_left_x, double bottom_y, double w, double h, GraphicResource idle_sprite, GraphicResource explode_sprite) {
	barrel->vtable = {
		Barrel_Place,
		Barrel_Remove,
		Barrel_Think
	};
	auto top_left_y = bottom_y + h;
	AABB_Create(&barrel->aabb, top_left_x, top_left_y, w, h);
	AABB_Create(&barrel->sight_aabb, top_left_x, WINDOW_HEIGHT, w, WINDOW_HEIGHT);			// FIXME
	barrel->idle_sprite = idle_sprite;
	barrel->explode_sprite = explode_sprite;
	barrel->active_sprite = idle_sprite;
	barrel->sprite_changed = false;
	barrel->fuse_lit = false;
}

void Barrel_Place(Entity* rect, SandGame* game) {
	// pass
}

void Barrel_Remove(Entity* rect, SandGame* game) {
	// pass
}

bool Barrel_FuseLit(const EntityBarrel* barrel) {
	return barrel->fuse_lit;
}

void Barrel_Explode(Entity* barrel, SandGame* game) {
	SandGame_SetFXFlag(game, FX_WHITE_FLASH);
	SandGame_RemoveEntity(game, barrel);
	SandGame_SetSFXFlag(game, SFX_BOOM);
	SetPlayerXVelocity(&game->player, 0.0);
}

void Barrel_ChangeGraphic(EntityBarrel* barrel, GraphicResource new_rsc) {
	barrel->active_sprite = new_rsc;
	barrel->sprite_changed = true;
}

void Barrel_LightFuse(EntityBarrel* barrel, SandGame* game) {
	barrel->fuse_lit = true;
	barrel->time_until_explosion_s = BARREL_EXPLOSION_DELAY_S;
	Barrel_ChangeGraphic(barrel, barrel->explode_sprite);
	SandGame_SetSFXFlag(game, SFX_FUSE);
}

void Barrel_Think(Entity* ent, SandGame* game, double dt) {
	auto* barrel = &ent->entity.barrel;
	barrel->sprite_changed = false;

	// Smothered
	auto* world = &game->pit;
	auto barrel_aabb = barrel->aabb;
	AABB_ScaleByReciprocal(&barrel_aabb, game->pit.grain_size);
	auto smother_query = SandPit_QueryRegion(world, &barrel_aabb);
	if (smother_query.grain_count >= 0.7 * (smother_query.w * smother_query.h)) {
		return;
	}

	// Explode check
	if (Barrel_FuseLit(barrel)) {
		barrel->time_until_explosion_s -= dt;
		if (barrel->time_until_explosion_s <= 0.0) {
			Barrel_Explode(ent, game);
		}
	}
	else {
		// Ignite check
		if (AABB_Intersects(&barrel->sight_aabb, &game->player.bbox)) {
			Barrel_LightFuse(barrel, game);
		}
	}
}

void LevelDoor_Create(EntityLevelDoor* door, double top_left_x, double top_left_y, double w, double h, const char* next_level_path) {
	door->vtable = {
		LevelDoor_Place,
		LevelDoor_Remove,
		LevelDoor_Think
	};
	AABB_Create(&door->aabb, top_left_x, top_left_y, w, h);
	door->next_level_path = next_level_path;
}

void LevelDoor_Place(Entity* ent, SandGame* game) {
}

void LevelDoor_Remove(Entity* ent, SandGame* game) {
}

void LevelDoor_Think(Entity* ent, SandGame* game, double dt) {
	auto* door = &ent->entity.door;
	if (AABB_Intersects(&door->aabb, &game->player.bbox)) {
		if (GameActionFlags_Get(game->action_flags_pressed, ACTION_INTERACT)) {
			// Load new level
			SandGame_SetSFXFlag(game, SFX_DOOR_OPEN);
			SandGame_SetFXFlag(game, FX_BLACK_FADE_IN_OUT);
			SandGame_FreezeFor(game, DOOR_LOADING_DELAY_S);
			SandGame_SetNewLevelPath(game, door->next_level_path);
		}
	}
}

Entity Entity_CreateFrom(void* instance, EntityType type)
{
	Entity result{};
	switch (type) {
	case ENTITY_RECTANGLE: { memcpy_s(&result, sizeof(result), instance, sizeof(EntityRectangleObstacle)); break; }
	case ENTITY_HINT_BOX: { memcpy_s(&result, sizeof(result), instance, sizeof(EntityHintBox)); break; }
	case ENTITY_BARREL: { memcpy_s(&result, sizeof(result), instance, sizeof(EntityBarrel)); break; }
	case ENTITY_LEVEL_DOOR: { memcpy_s(&result, sizeof(result), instance, sizeof(EntityLevelDoor)); break; }
	default: { assert("Entity_CreateFrom: Unaccounted entity type encountered." && false); }
	}

	result.type = type;
	return result;
}

void Entity_Place(Entity* _this, SandGame* pit) {
	_this->entity.vtable.place_fn(_this, pit);
}

void Entity_Remove(Entity* _this, SandGame* pit) {
	_this->entity.vtable.remove_fn(_this, pit);
}

void Entity_Think(Entity* _this, SandGame* game, double dt) {
	_this->entity.vtable.think_fn(_this, game, dt);
}


