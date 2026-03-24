#include "sand_sim.hpp"
#include <memory>
#include <chrono>
#include <iostream>

constexpr std::int32_t MAX_GRAIN_VELOCITY{ 12 };
constexpr auto SAND_PIT_CELL_ID_MASK = 0b11;
constexpr auto SAND_PIT_CELL_ID_SHIFT = 0;
constexpr auto SAND_PIT_CELL_PARTICLE_PRESENT_MASK = 0b100;
constexpr auto SAND_PIT_CELL_PARTICLE_PRESENT_SHIFT = 2;
constexpr auto SAND_PIT_CELL_RESERVED_MASK = 0b1000;
constexpr auto SAND_PIT_CELL_RESERVED_SHIFT = 3;
constexpr auto SAND_PIT_CELL_SIMULATE_FLAG_MASK = 0b10000;
constexpr auto SAND_PIT_CELL_SIMULATE_FLAG_SHIFT = 4;
constexpr auto SAND_PIT_CELL_X_VELOCITY_MASK = 0b1111100000;
constexpr auto SAND_PIT_CELL_X_VELOCITY_SHIFT = 5;
constexpr auto SAND_PIT_CELL_Y_VELOCITY_MASK = 0b111110000000000;
constexpr auto SAND_PIT_CELL_Y_VELOCITY_SHIFT = 10;
constexpr auto SAND_PIT_CELL_X_LOSS_NUMERATOR_MASK = 0b111000000000000000;
constexpr auto SAND_PIT_CELL_X_LOSS_NUMERATOR_SHIFT = 15;
constexpr auto SAND_PIT_CELL_Y_LOSS_NUMERATOR_MASK = 0b111000000000000000000;
constexpr auto SAND_PIT_CELL_Y_LOSS_NUMERATOR_SHIFT = 18;

inline TWO_BIT_ID SandPitCell_GetId(SandPitCell cell) {
	return cell & SAND_PIT_CELL_ID_MASK;
}

inline bool SandPitCell_GetParticlePresent(SandPitCell cell) {
	return cell & SAND_PIT_CELL_PARTICLE_PRESENT_MASK;
}

inline bool SandPitCell_GetSimulateFlag(SandPitCell cell) {
	return cell & SAND_PIT_CELL_SIMULATE_FLAG_MASK;
}

inline int SandPitCell_GetXVelocity(SandPitCell cell) {
	std::int32_t val = (cell & SAND_PIT_CELL_X_VELOCITY_MASK) >> SAND_PIT_CELL_X_VELOCITY_SHIFT;
	return (val << 27) >> 27;		// Sign-extend
}

inline void SandPitCell_SetXVelocity(SandPitCell* cell, int32_t v) {
    int32_t fixed_v = (v& (SAND_PIT_CELL_X_VELOCITY_MASK >> SAND_PIT_CELL_X_VELOCITY_SHIFT)); 
    *cell &= ~SAND_PIT_CELL_X_VELOCITY_MASK;
    *cell |= (fixed_v << SAND_PIT_CELL_X_VELOCITY_SHIFT);
}

inline int SandPitCell_GetYVelocity(SandPitCell cell) {
	std::int32_t val = (cell & SAND_PIT_CELL_Y_VELOCITY_MASK) >> SAND_PIT_CELL_Y_VELOCITY_SHIFT;
	return (val << 27) >> 27;		// Sign-extend
}

inline void SandPitCell_SetYVelocity(SandPitCell* cell, int32_t v) {
    int32_t fixed_v = (v& (SAND_PIT_CELL_Y_VELOCITY_MASK >> SAND_PIT_CELL_Y_VELOCITY_SHIFT)); 
    *cell &= ~SAND_PIT_CELL_Y_VELOCITY_MASK;
    *cell |= (fixed_v << SAND_PIT_CELL_Y_VELOCITY_SHIFT);
}

inline int SandPitCell_GetXLossNumerator(SandPitCell cell) {
	return (cell & SAND_PIT_CELL_X_LOSS_NUMERATOR_MASK) >> SAND_PIT_CELL_X_LOSS_NUMERATOR_SHIFT ;
}

inline int SandPitCell_GetYLossNumerator(SandPitCell cell) {
	return (cell & SAND_PIT_CELL_Y_LOSS_NUMERATOR_MASK) >> SAND_PIT_CELL_Y_LOSS_NUMERATOR_SHIFT ;
}

inline SandPitCell SandPitCell_Create(TWO_BIT_ID id, bool simulate_flag, std::int32_t x_vel, std::int32_t y_vel) {
	SandPitCell result = 0;
	result |= (id % 4) << SAND_PIT_CELL_ID_SHIFT;
	result |= 1 << SAND_PIT_CELL_PARTICLE_PRESENT_SHIFT;
	result |= (simulate_flag) << SAND_PIT_CELL_SIMULATE_FLAG_SHIFT;
	result |= Bounds(x_vel, MAX_GRAIN_VELOCITY, -MAX_GRAIN_VELOCITY) << SAND_PIT_CELL_X_VELOCITY_SHIFT;
	result |= Bounds(y_vel, MAX_GRAIN_VELOCITY, -MAX_GRAIN_VELOCITY) << SAND_PIT_CELL_Y_VELOCITY_SHIFT;
	return result;
}

void SandPit_Create(
	SandPit& pit, 
	std::uint32_t w, 
	std::uint32_t h, 
	std::uint32_t stubbornness, 
	std::int16_t grain_x
)
{
	// Allocate and Zero sand world.
	auto world_bytes = sizeof(SandPitCell) * w * h;
	auto* world = (SandPitCell*)(malloc(world_bytes));

	pit.world = world;
	pit.w = w;
	pit.h = h;
	pit.stubbornness = stubbornness;

	SandPit_Clear(pit);
}

void SandPit_Destroy(SandPit& pit)
{
	free(pit.world);
}

void SandPit_Clear(SandPit& pit)
{
	auto world_bytes = sizeof(SandPitCell) * pit.w * pit.h;
	memset(pit.world, 0, world_bytes);
}

bool SandPit_IsInBounds(const SandPit& pit, std::uint32_t x, std::uint32_t y) {
	return (x >= 0 && x < pit.w) && (y >= 0 && y < pit.h);
}

SandPitCell SandPit_Get(SandPit& pit, std::uint32_t x, std::uint32_t y) {
	return pit.world[y * pit.w + x];
}

void SandPit_Set(SandPit& pit, std::uint32_t x, std::uint32_t y, SandPitCell val) {
	pit.world[y * pit.w + x] = val;
}

void SandPit_ClearCell(SandPit& pit, std::uint32_t x, std::uint32_t y) {
	pit.world[y * pit.w + x] &= ~SAND_PIT_CELL_PARTICLE_PRESENT_MASK;
}

const SandPitCell& SandPit_Get(const SandPit& pit, std::uint32_t x, std::uint32_t y) {
	return pit.world[y * pit.w + x];
}

// Returns true if (x, y) is out-of-bounds.
bool SandPit_IsCellOccupied(const SandPit& pit, std::uint32_t x, std::uint32_t y) {
	if (!SandPit_IsInBounds(pit, x, y))
		return true;

	auto& grain = SandPit_Get(pit, x, y);
	return SandPitCell_GetParticlePresent(grain);
}

void SandPit_PlaceGrain(SandPit& pit, std::uint32_t x, std::uint32_t y, int particle_id) {
	if (SandPit_IsInBounds(pit, x, y))
		SandPit_Set(pit, x, y, SandPitCell_Create(particle_id, pit.simulate_tick, 0, 0));
}

void SandPitCell_MarkAsSimulated(SandPitCell* grain, bool simulate_tick) {
	if (simulate_tick) {
		*grain |= SAND_PIT_CELL_SIMULATE_FLAG_MASK;
	}
	else {
		*grain &= ~SAND_PIT_CELL_SIMULATE_FLAG_MASK;
	}
}

std::uint8_t SandPit_PseudoRand() {
	return rand();
}

void SandPit_DoCollisions(std::uint32_t x, std::uint32_t y, SandPit& pit, std::uint32_t epoch) {
	auto grain = SandPit_Get(pit, x, y);
	// If there is no particle here, or this cell has already been simulated, we 
	// have no work to do.
	if (!SandPitCell_GetParticlePresent(grain)) {
		return;
	}

	auto x_vel = SandPitCell_GetXVelocity(grain);
	auto y_vel = SandPitCell_GetYVelocity(grain);
	auto dx = (std::abs(x_vel) >= epoch) ? Bounds(x_vel, 1, -1) : 0;
	auto dy = (std::abs(y_vel) >= epoch) ? Bounds(y_vel, 1, -1) : 0;
	auto is_moving = dy != 0 || dx != 0;
	if (!is_moving)
		return;

	auto next_x = x + dx;
	auto next_y = y + dy;
	if (SandPit_IsInBounds(pit, next_x, next_y)) {
		SandPitCell grain_ptr = grain;
		SandPitCell collision = SandPit_Get(pit, next_x, next_y);
		grain_ptr &= ~SAND_PIT_CELL_X_LOSS_NUMERATOR_MASK;
		grain_ptr &= ~SAND_PIT_CELL_Y_LOSS_NUMERATOR_MASK;
		int collisions = 1;
		while (SandPitCell_GetParticlePresent(collision)) {
			auto x_loss = 0; 
			auto y_loss = 0; 
			auto x_loss_numerator = SandPitCell_GetXLossNumerator(grain_ptr);
			auto y_loss_numerator = SandPitCell_GetYLossNumerator(grain_ptr);
			if (x_loss_numerator >= 4) {
				grain_ptr &= ~SAND_PIT_CELL_X_LOSS_NUMERATOR_MASK;
				x_loss_numerator = 0;
				x_loss = Bounds(SandPitCell_GetXVelocity(grain_ptr), 1, -1);
			}
			else if (y_loss_numerator >= 4) {
				grain_ptr &= ~SAND_PIT_CELL_Y_LOSS_NUMERATOR_MASK;
				y_loss_numerator = 0;
				y_loss = Bounds(SandPitCell_GetYVelocity(grain_ptr), 1, -1);
			}

			auto grain_x_loss = 0l;
			auto grain_y_loss = 0l;
			auto collision_x_loss = 0l;
			auto collision_y_loss = 0l;
			auto grain_x_vel = SandPitCell_GetXVelocity(grain_ptr);
			auto grain_y_vel = SandPitCell_GetYVelocity(grain_ptr);
			auto collision_x_vel = SandPitCell_GetXVelocity(collision);
			auto collision_y_vel = SandPitCell_GetYVelocity(collision);
			if (collisions > 3 && (std::abs(collision_x_vel) <= 1) && std::abs(collision_y_vel) <= 1) {
				grain_x_loss = Bounds(collision_x_vel, 1, -1);
				grain_y_loss = Bounds(collision_y_vel, 1, -1);
				collision_x_loss = Bounds(grain_x_vel, 1, -1);
				collision_y_loss = Bounds(grain_y_vel, 1, -1);
			}
			SandPitCell_SetXVelocity(&collision, grain_x_vel - collision_x_loss);
			SandPitCell_SetYVelocity(&collision, grain_y_vel - collision_y_loss);
			SandPitCell_SetXVelocity(&grain_ptr, collision_x_vel - grain_x_loss);
			SandPitCell_SetYVelocity(&grain_ptr, collision_y_vel - grain_y_loss);

			collision &= ~SAND_PIT_CELL_X_LOSS_NUMERATOR_MASK;
			collision &= ~SAND_PIT_CELL_Y_LOSS_NUMERATOR_MASK;
			collision |= (x_loss_numerator + 1) << SAND_PIT_CELL_X_LOSS_NUMERATOR_SHIFT;
			collision |= (y_loss_numerator + 1) << SAND_PIT_CELL_Y_LOSS_NUMERATOR_SHIFT;

			SandPit_Set(pit, x, y, grain_ptr);
			SandPit_Set(pit, next_x, next_y, collision);

			++collisions;
			x = next_x;
			y = next_y;
			next_x += dx;
			next_y += dy;
			if (!SandPit_IsInBounds(pit, next_x, next_y)) {
				return;
			}

			grain_ptr = collision;
			collision = SandPit_Get(pit, next_x, next_y);
		}
	}
}

int SandPit_GetRandomWindImpulse() {
	// Either -1, 0 or 1
	auto result = rand() % (12+1);			// Random number from 0 - 12
	result -= 6;							// Random number from -6 to 6
	return result / 5;						
}

void SandPit_SimulateGrain(std::uint32_t x, std::uint32_t y, SandPit& pit, bool simulate_tick, std::uint32_t epoch) {
	auto grain = SandPit_Get(pit, x, y);
	// If there is no particle here, or this cell has already been simulated, we 
	// have no work to do.
	if (!SandPitCell_GetParticlePresent(grain) || SandPitCell_GetSimulateFlag(grain) == simulate_tick) {
		return;
	}

	// Scan for objects below us.
	auto x_vel = SandPitCell_GetXVelocity(grain);
	auto y_vel = SandPitCell_GetYVelocity(grain);
	auto dx = (std::abs(x_vel) >= epoch) ? Bounds(x_vel, 1, -1) : 0;
	auto dy = (std::abs(y_vel) >= epoch) ? Bounds(y_vel, 1, -1) : 0;
	std::int64_t next_x = (std::int64_t)x + dx;
	std::int64_t next_y = (std::int64_t)y + dy;
	auto is_moving = dy != 0 || dx != 0;

	if (is_moving) {
		if (epoch == 0) {
			// Air resistance
			auto ease_x_vel = Bounds(-x_vel, 1, -1);			// TODO: friction coeff
			auto ease_y_vel = Bounds(-y_vel, 1, -1);
			x_vel += ease_x_vel;
			y_vel += ease_y_vel;
			SandPitCell_SetXVelocity(&grain, x_vel);
			SandPitCell_SetYVelocity(&grain, y_vel);
		}
	}

	if (epoch == 0 && dy == 0) {
		auto wind = SandPit_GetRandomWindImpulse();		// Wind
		// Are we falling?
		if (!SandPit_IsCellOccupied(pit, next_x, next_y - 1)) {
			next_y -= 1;

			if (!SandPit_IsCellOccupied(pit, next_x + wind, next_y)) {
				next_x += wind;
			}
			is_moving = true;
		}
		// Is our pillar unstable?
		else if (!SandPit_IsCellOccupied(pit, next_x + wind, next_y - pit.stubbornness)) {
			next_x += wind;
			is_moving = true;
		}
	}

	if (SandPit_IsInBounds(pit, next_x, next_y)) {
		if (!is_moving) {
			SandPitCell_MarkAsSimulated(&grain, simulate_tick);			// CRITICAL: Mark we have simulated this grain.
			SandPit_Set(pit, x, y, grain);
		}
		else if (!SandPit_IsCellOccupied(pit, next_x, next_y)) {
			SandPitCell_MarkAsSimulated(&grain, simulate_tick);			// CRITICAL: Mark we have simulated this grain.
			SandPit_Set(pit, next_x, next_y, grain);
			SandPit_ClearCell(pit, x, y);
		}
	} else {
		// Slam against boundaries.
		// Convert to signed to provent underflow being set to max value.
		auto fixed_x = Bounds((std::int64_t)next_x, (std::int64_t)pit.w - 1, 0ll);
		if (fixed_x != next_x)
			SandPitCell_SetXVelocity(&grain, 0);
		auto fixed_y = Bounds((std::int64_t)next_y, (std::int64_t)pit.h - 1, 0ll);
		if (fixed_y != next_y)
			SandPitCell_SetYVelocity(&grain, 0);

		SandPitCell_MarkAsSimulated(&grain, simulate_tick);			// CRITICAL: Mark we have simulated this grain.
		if ((fixed_x != next_x || fixed_y != next_y) && !SandPit_IsCellOccupied(pit, fixed_x, fixed_y)) {
			SandPit_Set(pit, fixed_x, fixed_y, grain);
			SandPit_ClearCell(pit, x, y);
		}
	}
}

void SandPit_NextTick(SandPit& pit) {
	pit.simulate_tick = !pit.simulate_tick;
}


void SandPit_SimulateStep(SandPit& pit) {
	auto start = std::chrono::system_clock::now();

	for (int epoch = MAX_GRAIN_VELOCITY - 1; epoch >= 0; epoch--) {
		for (int y = 0; y < pit.h; y++) {
			for (int x = 0; x < pit.w; x++) {
				SandPit_DoCollisions(x, y, pit, epoch);
			}
		}
		SandPit_NextTick(pit);
		for (int y = 0; y < pit.h; y++) {
			for (int x = 0; x < pit.w; x++) {
				SandPit_SimulateGrain(x, y, pit, pit.simulate_tick, epoch);
			}
		}
	}

	auto finish = std::chrono::system_clock::now();
	auto duration = finish - start;
	std::cout << "Took: " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms\n";
}

void SandPit_AddImpulse(SandPit& pit, std::uint32_t x, std::uint32_t y, std::int32_t x_v, std::int32_t y_v)
{
	if (!SandPit_IsInBounds(pit, x, y))
		return;

	auto grain = SandPit_Get(pit, x, y);
	if (!SandPitCell_GetParticlePresent(grain))
		return;

	SandPitCell_SetXVelocity(&grain, Bounds(
		SandPitCell_GetXVelocity(grain) + x_v, MAX_GRAIN_VELOCITY, -MAX_GRAIN_VELOCITY 
	));
	SandPitCell_SetYVelocity(&grain, 
		Bounds(SandPitCell_GetYVelocity(grain) + y_v, MAX_GRAIN_VELOCITY, -MAX_GRAIN_VELOCITY
	));
	SandPit_Set(pit, x, y, grain);
}

void SandPit_ClearRegion(SandPit& pit, std::uint32_t x0, std::uint32_t y0, std::uint32_t w, std::uint32_t h)
{
	for (int y = y0; y < y0 + h; y++) {
		for (int x = x0; x < x0 + w; x++) {
			SandPit_ClearCell(pit, x, y);
		}
	}
}

TWO_BIT_ID SandPit_GetIdAt(const SandPit& pit, std::uint32_t x, std::uint32_t y)
{
	auto grain = SandPit_Get(pit, x, y);
	if (SandPitCell_GetParticlePresent(grain)) {
		return SandPitCell_GetId(grain);
	}
	return PARTICLE_NOT_PRESENT;
}

bool SandPit_AnyInRegion(const SandPit& pit, std::uint32_t x0, std::uint32_t y0, std::uint32_t w, std::uint32_t h)
{
	for (int y = y0; y < y0 + h; y++) {
		for (int x = x0; x < x0 + w; x++) {
			if (SandPit_IsCellOccupied(pit, x, y)) {
				return true;
			}
		}
	}

	return false;
}

SandPitQueryResult SandPit_QueryRegion(const SandPit* pit, std::uint32_t x0, std::uint32_t y0, std::uint32_t w, std::uint32_t h)
{
	auto result = SandPitQueryResult{};
	result.x0 = x0;
	result.y0 = y0;
	result.w = w;
	result.h = h;
	result.highest_grain_y = -1;

	for (int y = y0; y >= (int)y0 - (int)h; y--) {
		for (int x = x0; x <= (int)x0 + (int)w; x++) {
			if (SandPit_IsInBounds(*pit, x, y) && SandPit_IsCellOccupied(*pit, x, y)) {
				result.grain_count += 1;
				result.highest_grain_y = std::max(result.highest_grain_y, y);
			}
		}
	}
	return result;
}
