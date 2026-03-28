#pragma once
#include <cstdint>
#include "common.hpp"

#define TWO_BIT_ID std::int8_t 
#define PARTICLE_NOT_PRESENT -1 
using SandPitCell = std::uint64_t;

constexpr std::uint32_t SAND_SIZE{ 3 };
constexpr std::uint32_t PIT_SECTOR_WIDTH{ WINDOW_WIDTH / SAND_SIZE };
constexpr std::uint32_t PIT_SECTOR_HEIGHT{ WINDOW_HEIGHT / SAND_SIZE };

struct SandPitSector {
	std::uint32_t origin_x{};
	std::uint32_t origin_y{};
	std::uint32_t w{};
	std::uint32_t h{};
	SandPitCell* sand_head{};
	std::uint32_t sand_head_index{};
	SandPitCell world[PIT_SECTOR_WIDTH * PIT_SECTOR_HEIGHT];	// 1.8 MB for 1920 x 1080 with 3 x 3 grains
	bool need_sim = false;
	bool simulate_tick = false;		
};

struct SandPit {
	std::uint32_t sector_w{};
	std::uint32_t sector_h{};
	std::uint32_t w{};			
	std::uint32_t h{};
	std::uint32_t num_screens_horizontal{};			
	std::uint32_t num_screens_vertical{};
	std::int32_t grain_size{};			// The side of a square grain in pixels. Signed to prevent many later conversions.
	std::uint32_t stubbornness{};		// The height a pillar of sand can reach before it collapses.

	SandPitSector* sectors = nullptr;
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
void SandPit_Clear(SandPit* pit, std::uint32_t sector_x, std::uint32_t sector_y);
void SandPit_SimulateStep(SandPit* pit, std::uint32_t sector_x, std::uint32_t sector_y);
void SandPit_PlaceGrain(SandPit* pit, std::uint32_t x, std::uint32_t y, int particle_id);
void SandPit_PlaceSolid(SandPit* pit, std::uint32_t x, std::uint32_t y);
void SandPit_PlaceSolidAABB(SandPit* pit, const AABB* aabb);
void SandPit_AddImpulse(SandPit* pit, std::uint32_t x, std::uint32_t y, std::int32_t x_v, std::int32_t y_v);
void SandPit_ClearRegion(SandPit* pit, const AABB* aabb, bool clear_solids);
void SandPit_WorldCoordsToSectorCoords(
	const SandPit* pit, 
	std::uint32_t world_x, 
	std::uint32_t world_y,
	std::uint32_t* out_sector_x,
	std::uint32_t* out_sector_y
);
void SandPit_ForEachGrain(
	const SandPit* pit, 
	std::uint32_t sector_x,
	std::uint32_t sector_y,
	SandPitForEachGrainCallback_t callback
);
SandPitQueryResult SandPit_QueryRegion(			// O(wh)
	const SandPit* pit, 
	const AABB* aabb
);
SandPitQueryResult SandPit_QueryRegion(			// O(wh)
	const SandPit* pit, 
	std::uint32_t x0,	// Top-Left corner
	std::uint32_t y0, 
	std::uint32_t w, 
	std::uint32_t h
);

