#pragma once
#include <cstdint>
#include "common.hpp"

// SandPitCell |id: 2|present: 1|simulate_flag:1|x_vel: 4|y_vel: 4|x_mv_loss: 3|y_mv_loss: 3|
#define TWO_BIT_ID std::int8_t 
#define PARTICLE_NOT_PRESENT -1 
using SandPitCell = std::uint32_t;

// Intended to be a semi-opaque structure where the particle buffer 
// can be readily exposed to the rendering engine.
struct SandPit {
	bool simulate_tick{};		
	std::uint32_t w{};			
	std::uint32_t h{};
	std::uint32_t num_screens_horizontal{};			
	std::uint32_t num_screens_vertical{};
	std::int32_t grain_size{};			// The side of a square grain in pixels. Signed to prevent many later conversions.
	std::uint32_t stubbornness{};		// The height a pillar of sand can reach before it collapses.
	SandPitCell* sand_head{};
	SandPitCell* world;
};

void SandPit_Create(
	SandPit& pit, 
	std::uint32_t w, 
	std::uint32_t h, 
	std::uint32_t num_screens_horizontal, 
	std::uint32_t num_screens_vertical, 
	std::uint32_t stubbornness, 
	std::int16_t grain_size
);
void SandPit_Destroy(SandPit& pit);
void SandPit_Clear(SandPit& pit);

void SandPit_PlaceGrain(SandPit& pit, std::uint32_t x, std::uint32_t y, int particle_id);
void SandPit_SimulateStep(SandPit& pit);
void SandPit_AddImpulse(
	SandPit& pit,
	std::uint32_t x,
	std::uint32_t y,
	std::int32_t x_v,
	std::int32_t y_v
);
void SandPit_ClearRegion(
	SandPit& pit, 
	std::uint32_t x0, 
	std::uint32_t y0, 
	std::uint32_t w, 
	std::uint32_t h
);

TWO_BIT_ID SandPit_GetIdAt(const SandPit& pit, std::uint32_t x, std::uint32_t y);

bool SandPit_AnyInRegion(
	const SandPit& pit, 
	std::uint32_t x0, 
	std::uint32_t y0, 
	std::uint32_t w, 
	std::uint32_t h
);
struct SandPitQueryResult {
	std::uint32_t x0{};
	std::uint32_t y0{};
	std::uint32_t w{};
	std::uint32_t h{};
	std::uint32_t grain_count{};
	std::int32_t highest_grain_y{};		// -1 if none present.
};

SandPitQueryResult SandPit_QueryRegion(
	const SandPit* pit, 
	std::uint32_t x0,	// Top-Left corner
	std::uint32_t y0, 
	std::uint32_t w, 
	std::uint32_t h
);
