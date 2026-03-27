#pragma once
#include <cstdint>
#include "common.hpp"

#define TWO_BIT_ID std::int8_t 
#define PARTICLE_NOT_PRESENT -1 
using SandPitCell = std::uint64_t;
struct SandPit {
	bool simulate_tick{};		
	std::uint32_t w{};			
	std::uint32_t h{};
	std::uint32_t num_screens_horizontal{};			
	std::uint32_t num_screens_vertical{};
	std::int32_t grain_size{};			// The side of a square grain in pixels. Signed to prevent many later conversions.
	std::uint32_t stubbornness{};		// The height a pillar of sand can reach before it collapses.
	bool need_sim{};

	SandPitCell* sand_head{};
	std::uint32_t sand_head_index{};
	SandPitCell* world{};
};
struct SandPitQueryResult {
	std::uint32_t x0{};
	std::uint32_t y0{};
	std::uint32_t w{};
	std::uint32_t h{};
	std::uint32_t grain_count{};
	std::uint32_t solid_count{};
	std::int32_t highest_grain_y{};		// -1 if none present.
	std::int32_t lowest_grain_y{};		// -1 if none present.
	std::int32_t highest_solid_y{};		// -1 if none present.
	std::int32_t lowest_solid_y{};		// -1 if none present.
	bool any_grain{};
	bool any_solid{};
};
using SandPitForEachGrainCallback_t = void(*)(TWO_BIT_ID, std::uint32_t, std::uint32_t);

void SandPit_Create(
	SandPit* pit, 
	std::uint32_t w,		// Units of grain_size
	std::uint32_t h,		// Units of grain_size
	std::uint32_t num_screens_horizontal, 
	std::uint32_t num_screens_vertical, 
	std::uint32_t stubbornness, 
	std::uint16_t grain_size
);
void SandPit_Destroy(SandPit* pit);
void SandPit_Clear(SandPit* pit);
void SandPit_SimulateStep(SandPit* pit);
void SandPit_PlaceGrain(SandPit* pit, std::uint32_t x, std::uint32_t y, int particle_id);
void SandPit_PlaceSolid(SandPit* pit, std::uint32_t x, std::uint32_t y);
void SandPit_PlaceSolidAABB(SandPit* pit, const AABB* aabb);
void SandPit_AddImpulse(SandPit* pit, std::uint32_t x, std::uint32_t y, std::int32_t x_v, std::int32_t y_v);
void SandPit_ClearRegion(SandPit* pit, const AABB* aabb, bool clear_solids);
void SandPit_ForEachGrain(const SandPit* pit, SandPitForEachGrainCallback_t callback);
SandPitQueryResult SandPit_QueryRegion(
	const SandPit* pit, 
	const AABB* aabb
);
SandPitQueryResult SandPit_QueryRegion(
	const SandPit* pit, 
	std::uint32_t x0,	// Top-Left corner
	std::uint32_t y0, 
	std::uint32_t w, 
	std::uint32_t h
);



void PlaceSandCircle(SandPit* p, std::int32_t x, std::int32_t y, std::int32_t r);
void VacuumSand(SandPit* p, std::int32_t x, std::int32_t y, std::int32_t r);
void BlowSand(SandPit* p, std::int32_t x, std::int32_t y, std::int32_t r, std::int32_t v_x, std::int32_t v_y);
