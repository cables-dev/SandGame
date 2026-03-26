#include "game.hpp"
#include <cstdlib>
#include <vcruntime_string.h>			// memset
#include <cassert>

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
}

void SandGame_Create(SandGame* game, double player_x, double player_y, double player_w, double player_h, std::uint32_t pit_w, std::uint32_t pit_h, std::uint32_t pit_num_screens_horizontal, std::uint32_t pit_num_screens_vertical, std::uint32_t pit_stubbornness, std::uint16_t pit_grain_size, std::uint32_t max_entities) {
	Player_Create(&game->player, player_x, player_y, player_w, player_h);
	SandPit_Create(&game->pit, pit_w, pit_h, pit_num_screens_horizontal, pit_num_screens_vertical, pit_stubbornness, pit_grain_size);
	SandGame_CreateEntityList(game, max_entities);
}

void SandGame_Destroy(SandGame* game) {
	SandPit_Destroy(&game->pit);
	SandGame_DestroyEntityList(game);
}

void UpdateSandPit(SandPit* pit, bool place_button, bool action_button, bool clear_pit, int mouse_x, int mouse_y) {
	auto mouse_x_raw = mouse_x;
	auto mouse_y_raw = mouse_y;
	auto mouse_x_corrected = mouse_x_raw / pit->grain_size;
	auto mouse_y_corrected = mouse_y_raw / pit->grain_size;

	if (place_button) {
		PlaceSandCircle(pit, mouse_x_corrected, PIT_HEIGHT - (mouse_y_corrected), 6);
	}
	if (action_button) {
		VacuumSand(pit, mouse_x_corrected, PIT_HEIGHT - mouse_y_corrected, 32);
	}
	if (clear_pit) {
		SandPit_Clear(pit);
	}

	SandPit_SimulateStep(pit);
}

void SandGame_Simulate(SandGame* game, double dt) {
	double player_x = 0;
	double player_y = 0;
	AABB_GetCornerCoords(&game->player.bbox, AABB_BOTTOM, &player_x, &player_y);
	double player_w = AABB_GetWidth(&game->player.bbox);
	double player_h = AABB_GetHeight(&game->player.bbox);

	auto start_x = ((int)player_x / WINDOW_WIDTH) * WINDOW_WIDTH;
	auto start_y = ((int)player_y / WINDOW_HEIGHT) * WINDOW_HEIGHT;

	auto mouse_x = GetMouseX() + start_x;
	auto mouse_y = GetMouseY() + start_y;

	UpdateSandPit(
		&game->pit, 
		IsMouseButtonDown(MOUSE_BUTTON_LEFT), 
		IsMouseButtonDown(MOUSE_BUTTON_RIGHT), 
		IsKeyPressed(KEY_R),
		mouse_x,
		mouse_y
	);
	Player_UpdatePlayer(
		&game->player,
		&game->pit,
		IsKeyDown(KEY_A),
		IsKeyDown(KEY_D),
		IsKeyDown(KEY_SPACE),
		IsKeyDown(KEY_W), 
		IsKeyPressed(KEY_W), 
		IsKeyPressed(KEY_LEFT_CONTROL),
		start_x + GetMouseX(),
		WINDOW_HEIGHT - GetMouseY() + start_y,
		dt
	);
}

Entity* SandGame_AddEntity(SandGame* game, void* entity, EntityType type) {
	auto ent = Entity_CreateFrom(entity, type);
	auto* list_element = SandGame_FreeStoreAllocEntity(game);
	*list_element = ent;
	SandGame_InsertEntityInList(game, list_element);
	Entity_Place(list_element, &game->pit);

	return list_element;
}

void SandGame_RemoveEntity(SandGame* game, Entity* entity) {
	Entity_Remove(entity, &game->pit);
	Entity_Destroy(entity);
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


