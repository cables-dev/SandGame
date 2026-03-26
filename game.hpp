#pragma once
#include "entity.hpp"
#include "common.hpp"
#include "player.hpp"
#include "sand_sim.hpp"

struct SandGame {
	Player player;
	SandPit pit;
	Entity* entities;
	Entity* entities_head = nullptr;
	Entity* entities_top;					// Knuth-style fixed-width free-store
	Entity* entities_avail = nullptr;
	std::uint32_t max_entities;
};

void SandGame_Create(
	SandGame* game, 
	double player_x,
	double player_y,
	double player_w,
	double player_h,
	std::uint32_t pit_w,
	std::uint32_t pit_h,
	std::uint32_t pit_num_screens_horizontal, 
	std::uint32_t pit_num_screens_vertical, 
	std::uint32_t pit_stubbornness, 
	std::uint16_t pit_grain_size,
	std::uint32_t max_entities
);
void SandGame_Destroy(SandGame* game);
void SandGame_Simulate(SandGame* game, double dt);
Entity* SandGame_AddEntity(SandGame* game, void* entity, EntityType type);		// "entity" will be copied into a local buffer.
void SandGame_RemoveEntity(SandGame* game, Entity* entity);

using SandGameForEachEntityFn_t = void(*)(Entity*);
void SandGame_ForEachEntity(const SandGame* game, SandGameForEachEntityFn_t cb);
