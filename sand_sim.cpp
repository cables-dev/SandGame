#include "sand_sim.hpp"
#include <memory>
#include <chrono>
#include <iostream>

constexpr std::int32_t MAX_GRAIN_VELOCITY{ 12 };
constexpr auto SAND_PIT_CELL_ID_MASK = 0b11;
constexpr auto SAND_PIT_CELL_ID_SHIFT = 0;
constexpr auto SAND_PIT_CELL_PARTICLE_PRESENT_MASK = 0b100;
constexpr auto SAND_PIT_CELL_PARTICLE_PRESENT_SHIFT = 2;
constexpr auto SAND_PIT_CELL_SOLID_MASK = 0b1000;
constexpr auto SAND_PIT_CELL_SOLID_SHIFT = 3;
constexpr auto SAND_PIT_CELL_SIMULATE_FLAG_MASK = 0b10000;
constexpr auto SAND_PIT_CELL_SIMULATE_FLAG_SHIFT = 4;
constexpr auto SAND_PIT_CELL_X_VELOCITY_MASK = 0b1111100000;
constexpr auto SAND_PIT_CELL_X_VELOCITY_SHIFT = 5;
constexpr auto SAND_PIT_CELL_Y_VELOCITY_MASK = 0b111110000000000;
constexpr auto SAND_PIT_CELL_Y_VELOCITY_SHIFT = 10;

constexpr auto SAND_PIT_CELL_LIST_HEAD_FLAG_MASK = 0b1000000000000000;
constexpr auto SAND_PIT_CELL_LIST_HEAD_FLAG_SHIFT = 15;
constexpr auto SAND_PIT_CELL_LIST_NEXT_IDX_MASK = 0b1111111111111111111111110000000000000000;											// We can support 72 screens of 1920 * 1080 3x3 grains.
constexpr auto SAND_PIT_CELL_LIST_NEXT_IDX_SHIFT = 16;
constexpr auto SAND_PIT_CELL_LIST_PREV_IDX_MASK = 0b1111111111111111111111110000000000000000000000000000000000000000;
constexpr auto SAND_PIT_CELL_LIST_PREV_IDX_SHIFT = 40;

inline std::uint32_t SandPit_CoordsToIndex(const SandPit* pit, std::uint32_t x, std::uint32_t y) {
	return y * pit->w + x;
}

void SandPit_IndexToCoords(const SandPit* pit, std::uint32_t index, std::uint32_t* x, std::uint32_t* y) {
	*x = (index % pit->w);
	*y = (index / pit->w);
}

inline void SandPitCell_SetListHeadFlag(SandPitCell* cell, bool state) {
	if (state == true) {
		*cell |= SAND_PIT_CELL_LIST_HEAD_FLAG_MASK;
	}
	else {
		*cell &= ~SAND_PIT_CELL_LIST_HEAD_FLAG_MASK;
	}
}

inline bool SandPitCell_IsListHead(SandPitCell cell) {
	return cell & SAND_PIT_CELL_LIST_HEAD_FLAG_MASK;
}

inline std::uint32_t SandPitCell_GetNextIdx(SandPitCell cell) {
	return (cell & SAND_PIT_CELL_LIST_NEXT_IDX_MASK) >> SAND_PIT_CELL_LIST_NEXT_IDX_SHIFT;
}

inline void SandPitCell_SetNextIdx(SandPitCell* cell, std::uint64_t next) {
	*cell &= ~SAND_PIT_CELL_LIST_NEXT_IDX_MASK;
	auto fixed_next = next & (SAND_PIT_CELL_LIST_NEXT_IDX_MASK >> SAND_PIT_CELL_LIST_NEXT_IDX_SHIFT);
	*cell |= fixed_next << SAND_PIT_CELL_LIST_NEXT_IDX_SHIFT;
}

inline std::uint32_t SandPitCell_GetPrevIdx(SandPitCell cell) {
	return (cell & SAND_PIT_CELL_LIST_PREV_IDX_MASK) >> SAND_PIT_CELL_LIST_PREV_IDX_SHIFT;
}

inline void SandPitCell_SetPrevIdx(SandPitCell* cell, std::uint64_t prev) {
	*cell &= ~SAND_PIT_CELL_LIST_PREV_IDX_MASK;
	auto fixed_next = prev & (SAND_PIT_CELL_LIST_PREV_IDX_MASK >> SAND_PIT_CELL_LIST_PREV_IDX_SHIFT);
	*cell |= fixed_next << SAND_PIT_CELL_LIST_PREV_IDX_SHIFT;
}

inline TWO_BIT_ID SandPitCell_GetId(SandPitCell cell) {
	return cell & SAND_PIT_CELL_ID_MASK;
}

inline void SandPitCell_SetId(SandPitCell* cell, TWO_BIT_ID id) {
	*cell &= ~SAND_PIT_CELL_ID_MASK;
	*cell |= (static_cast<unsigned long long>(id % 4)) << SAND_PIT_CELL_ID_SHIFT;
}

inline bool SandPitCell_GetParticlePresent(SandPitCell cell) {
	return cell & SAND_PIT_CELL_PARTICLE_PRESENT_MASK;
}

inline void SandPitCell_SetParticlePresent(SandPitCell* cell, bool val) {
	if (val == true) {
		*cell |= SAND_PIT_CELL_PARTICLE_PRESENT_MASK;
	}
	else {
		*cell &= ~SAND_PIT_CELL_PARTICLE_PRESENT_MASK;
	}
}

inline bool SandPitCell_GetSimulateFlag(SandPitCell cell) {
	return cell & SAND_PIT_CELL_SIMULATE_FLAG_MASK;
}

inline int SandPitCell_GetXVelocity(SandPitCell cell) {
	std::int32_t val = (cell & SAND_PIT_CELL_X_VELOCITY_MASK) >> SAND_PIT_CELL_X_VELOCITY_SHIFT;
	return (val << 27) >> 27;		// Sign-extend
}

inline void SandPitCell_SetXVelocity(SandPitCell* cell, int32_t v) {
    int64_t fixed_v = (v& (SAND_PIT_CELL_X_VELOCITY_MASK >> SAND_PIT_CELL_X_VELOCITY_SHIFT)); 
    *cell &= ~SAND_PIT_CELL_X_VELOCITY_MASK;
    *cell |= (fixed_v << SAND_PIT_CELL_X_VELOCITY_SHIFT);
}

inline int SandPitCell_GetYVelocity(SandPitCell cell) {
	std::int32_t val = (cell & SAND_PIT_CELL_Y_VELOCITY_MASK) >> SAND_PIT_CELL_Y_VELOCITY_SHIFT;
	return (val << 27) >> 27;		// Sign-extend
}

inline void SandPitCell_SetYVelocity(SandPitCell* cell, int32_t v) {
    int64_t fixed_v = (v& (SAND_PIT_CELL_Y_VELOCITY_MASK >> SAND_PIT_CELL_Y_VELOCITY_SHIFT)); 
    *cell &= ~SAND_PIT_CELL_Y_VELOCITY_MASK;
    *cell |= (fixed_v << SAND_PIT_CELL_Y_VELOCITY_SHIFT);
}

inline void SandPitCell_SetVisible(SandPitCell* cell, bool vis) {
	if (vis == true) {
		*cell |= SAND_PIT_CELL_PARTICLE_PRESENT_MASK;
	}
	else {
		*cell &= ~SAND_PIT_CELL_PARTICLE_PRESENT_MASK;
	}
}

inline bool SandPitCell_IsVisible(SandPitCell cell) {
	return cell & SAND_PIT_CELL_PARTICLE_PRESENT_MASK;
}

inline bool SandPitCell_IsSolid(SandPitCell cell) {
	return cell & SAND_PIT_CELL_SOLID_MASK;
}

inline void SandPitCell_SetSolid(SandPitCell* cell, bool val) {
	if (val == true) {
		*cell |= SAND_PIT_CELL_SOLID_MASK;
	}
	else {
		*cell &= ~SAND_PIT_CELL_SOLID_MASK;
	}
}

void SandPitCell_MarkAsSimulated(SandPitCell* grain, bool simulate_tick) {
	if (simulate_tick) {
		*grain |= SAND_PIT_CELL_SIMULATE_FLAG_MASK;
	}
	else {
		*grain &= ~SAND_PIT_CELL_SIMULATE_FLAG_MASK;
	}
}

SandPitCell SandPitCell_Create(TWO_BIT_ID id, bool simulate_flag, std::int32_t x_vel, std::int32_t y_vel, bool is_solid = false) {
	SandPitCell result = 0;
	SandPitCell_SetId(&result, (id % 4));
	SandPitCell_SetParticlePresent(&result, true);
	SandPitCell_MarkAsSimulated(&result, simulate_flag);
	SandPitCell_SetSolid(&result, is_solid);
	SandPitCell_SetXVelocity(&result, Bounds(x_vel, MAX_GRAIN_VELOCITY, -MAX_GRAIN_VELOCITY));
	SandPitCell_SetYVelocity(&result, Bounds(y_vel, MAX_GRAIN_VELOCITY, -MAX_GRAIN_VELOCITY));
	return result;
}

SandPitCell SandPitCell_CreateListHead(
	TWO_BIT_ID id, 
	bool simulate_flag, 
	std::int32_t x_vel, 
	std::int32_t y_vel,
	std::uint32_t index
) {
	SandPitCell result = SandPitCell_Create(id, simulate_flag, x_vel, y_vel);
	SandPitCell_SetVisible(&result, false);
	SandPitCell_SetListHeadFlag(&result, true);
	SandPitCell_SetNextIdx(&result, index);
	SandPitCell_SetPrevIdx(&result, index);
	return result;
}

void SandPit_Create(
	SandPit* pit, 
	std::uint32_t w, 
	std::uint32_t h, 
	std::uint32_t num_screens_horizontal, 
	std::uint32_t num_screens_vertical, 
	std::uint32_t stubbornness, 
	std::uint16_t grain_size
) {
	pit->num_screens_horizontal = num_screens_horizontal;
	pit->num_screens_vertical = num_screens_vertical;

	auto world_size = sizeof(SandPitCell) * num_screens_horizontal * num_screens_vertical * w * h;
	auto* world = (SandPitCell*)(malloc(world_size));
	memset(world, 0, world_size);

	pit->world = world;
	pit->grain_size = grain_size;
	pit->w = num_screens_horizontal * w;
	pit->h = num_screens_vertical * h;
	pit->stubbornness = stubbornness;
	pit->need_sim = false;

	// Create sand list head
	auto sand_head_index = SandPit_CoordsToIndex(pit, 0, h - 1);
	auto sand_head = SandPitCell_CreateListHead(0, false, 0, 0, sand_head_index);
	pit->world[sand_head_index] = sand_head;
	pit->sand_head = &pit->world[sand_head_index];
	pit->sand_head_index = sand_head_index;
}

void SandPit_Destroy(SandPit* pit)
{
	free(pit->world);
}

void SandPit_Clear(SandPit* pit)
{
	auto world_bytes = sizeof(SandPitCell) * pit->w * pit->h;
	memset(pit->world, 0, world_bytes);

	// Create sand list head
	auto sand_head_index = SandPit_CoordsToIndex(pit, 0, pit->h - 1);
	auto sand_head = SandPitCell_CreateListHead(0, false, 0, 0, sand_head_index);
	pit->world[sand_head_index] = sand_head;
	pit->sand_head = &pit->world[sand_head_index];
	pit->sand_head_index = sand_head_index;
}

bool SandPit_IsInBounds(const SandPit* pit, std::uint32_t x, std::uint32_t y) {
	return (x >= 0 && x < pit->w) && (y >= 0 && y < pit->h);
}

SandPitCell* SandPit_Get(SandPit* pit, std::uint32_t x, std::uint32_t y) {
	return &pit->world[y * pit->w + x];
}

const SandPitCell& SandPit_Get(const SandPit* pit, std::uint32_t x, std::uint32_t y) {
	return pit->world[y * pit->w + x];
}

// Will do nothing if there is no grain at (x, y).
void SandPit_RemoveGrainAt(SandPit* pit, std::uint32_t x, std::uint32_t y) {
	auto doomed_idx = SandPit_CoordsToIndex(pit, x, y);
	auto* cell = &pit->world[doomed_idx];

	if (!SandPitCell_GetParticlePresent(*cell))
		return;

	// Update list
	SandPitCell_SetVisible(cell, false);
	auto prev_idx = SandPitCell_GetPrevIdx(*cell);
	auto next_idx = SandPitCell_GetNextIdx(*cell);
	auto* prev = &pit->world[prev_idx];
	auto* next = &pit->world[next_idx];
	SandPitCell_SetNextIdx(prev, next_idx);
	SandPitCell_SetPrevIdx(next, prev_idx);
}

void SandPit_MoveTo(SandPit* pit, std::uint32_t x, std::uint32_t y, SandPitCell val) {
	auto new_idx = SandPit_CoordsToIndex(pit, x, y);

	// If there is a grain in our spot we must remove it from the list.
	SandPit_RemoveGrainAt(pit, x, y);

	// Update list pointers
	auto prev_idx = SandPitCell_GetPrevIdx(val);
	auto* prev = &pit->world[prev_idx];
	SandPitCell_SetNextIdx(prev, new_idx);
	auto next_idx = SandPitCell_GetNextIdx(val);
	auto* next = &pit->world[next_idx];
	SandPitCell_SetPrevIdx(next, new_idx);

	pit->world[new_idx] = val;
}

void SandPit_ClearCell(SandPit* pit, std::uint32_t x, std::uint32_t y) {
	pit->world[y * pit->w + x] &= ~SAND_PIT_CELL_PARTICLE_PRESENT_MASK;
}

// Returns true if (x, y) is out-of-bounds.
bool SandPit_IsCellOccupied(const SandPit* pit, std::uint32_t x, std::uint32_t y) {
	if (!SandPit_IsInBounds(pit, x, y))
		return true;

	auto& grain = SandPit_Get(pit, x, y);
	return SandPitCell_GetParticlePresent(grain) || SandPitCell_IsListHead(grain);				// Don't overwrite the list head!
}

// Add grain to internal list. Will insert grain into world.
void SandPit_InsertGrainInList(SandPit* pit, std::uint32_t x, std::uint32_t y, SandPitCell grain) {
	if (!SandPit_IsInBounds(pit, x, y))
		return;

	auto grain_index = SandPit_CoordsToIndex(pit, x, y);
	if (pit->sand_head == nullptr) {
		SandPitCell_SetListHeadFlag(&grain, true);
		SandPitCell_SetNextIdx(&grain, grain_index);
		SandPitCell_SetPrevIdx(&grain, grain_index);
		pit->sand_head = SandPit_Get(pit, x, y);
		pit->sand_head_index = grain_index;
	}
	else {
		auto* head = pit->sand_head;
		auto head_index = pit->sand_head_index;

		// Insert new particle
		auto head_next_index = SandPitCell_GetNextIdx(*head);
		auto* head_next = &pit->world[head_next_index];
		SandPitCell_SetNextIdx(&grain, head_next_index);
		SandPitCell_SetPrevIdx(&grain, head_index);
		SandPitCell_SetPrevIdx(head_next, grain_index);
		SandPitCell_SetNextIdx(head, grain_index);
	}

	// If there is a grain in our spot we must remove it from the list.
	SandPit_RemoveGrainAt(pit, x, y);
	pit->world[grain_index] = grain;
}

void SandPit_PlaceGrain(SandPit* pit, std::uint32_t x, std::uint32_t y, int particle_id) {
	if (SandPit_IsCellOccupied(pit, x, y))
		return;

	auto grain = SandPitCell_Create(particle_id, pit->simulate_tick, 0, 0, false);
	SandPit_InsertGrainInList(pit, x, y, grain);
	pit->need_sim = true;
}

void SandPit_PlaceSolid(SandPit* pit, std::uint32_t x, std::uint32_t y) {
	if (SandPit_IsCellOccupied(pit, x, y))
		return;

	auto grain = SandPitCell_Create(0, pit->simulate_tick, 0, 0, true);
	auto grain_index = SandPit_CoordsToIndex(pit, x, y);
	pit->world[grain_index] = grain;
}

void SandPit_PlaceSolidAABB(SandPit* pit, const AABB* aabb) {
	double start_y = 0;
	double end_y = 0;
	double start_x = 0;
	double end_x = 0;
	AABB_GetCornerCoords(aabb, AABB_BOTTOM_LEFT, &start_x, &start_y);
	AABB_GetCornerCoords(aabb, AABB_TOP_RIGHT, &end_x, &end_y);

	for (int y = start_y; y < end_y; y++) {
		for (int x = start_x; x < end_x; x++) {
			SandPit_PlaceSolid(pit, x, y);
		}
	}
}

std::uint8_t SandPit_PseudoRand() {
	return rand();
}

void SandPit_DoCollisions(std::uint32_t x, std::uint32_t y, SandPit* pit, std::uint32_t epoch) {
	auto* grain = SandPit_Get(pit, x, y);
	// If there is no particle here, or this cell has already been simulated, we 
	// have no work to do.
	if (!SandPitCell_GetParticlePresent(*grain)) {
		return;
	}

	auto x_vel = SandPitCell_GetXVelocity(*grain);
	auto y_vel = SandPitCell_GetYVelocity(*grain);
	auto dx = (std::abs(x_vel) >= epoch) ? Bounds(x_vel, 1, -1) : 0;
	auto dy = (std::abs(y_vel) >= epoch) ? Bounds(y_vel, 1, -1) : 0;
	auto is_moving = dy != 0 || dx != 0;
	if (!is_moving)
		return;

	auto next_x = x + dx;
	auto next_y = y + dy;
	if (SandPit_IsInBounds(pit, next_x, next_y)) {
		auto* grain_ptr = grain;
		auto* collision = SandPit_Get(pit, next_x, next_y);
		int collisions = 1;
		while (SandPitCell_GetParticlePresent(*collision)) {
			bool is_collision_solid = SandPitCell_IsSolid(*collision);

			auto grain_x_loss = 0l;
			auto grain_y_loss = 0l;
			auto collision_x_loss = 0l;
			auto collision_y_loss = 0l;

			auto grain_x_vel = SandPitCell_GetXVelocity(*grain_ptr);
			auto grain_y_vel = SandPitCell_GetYVelocity(*grain_ptr);
			auto collision_x_vel = (is_collision_solid) ? 0 : SandPitCell_GetXVelocity(*collision);
			auto collision_y_vel = (is_collision_solid) ? 0 : SandPitCell_GetYVelocity(*collision);
			if (collisions > 3 && (std::abs(collision_x_vel) <= 1) && std::abs(collision_y_vel) <= 1) {
				grain_x_loss = Bounds(collision_x_vel, 1, -1);
				grain_y_loss = Bounds(collision_y_vel, 1, -1);
				collision_x_loss = Bounds(grain_x_vel, 1, -1);
				collision_y_loss = Bounds(grain_y_vel, 1, -1);
			}
			if (!is_collision_solid) {
				SandPitCell_SetXVelocity(collision, grain_x_vel - collision_x_loss);
				SandPitCell_SetYVelocity(collision, grain_y_vel - collision_y_loss);
			}
			SandPitCell_SetXVelocity(grain_ptr, collision_x_vel - grain_x_loss);
			SandPitCell_SetYVelocity(grain_ptr, collision_y_vel - grain_y_loss);

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

// Returns true if there was movement.
bool SandPit_SimulateGrain(std::uint32_t x, std::uint32_t y, SandPit* pit, bool simulate_tick, std::uint32_t epoch) {
	auto* grain = SandPit_Get(pit, x, y);
	// If there is no particle here, or this cell has already been simulated, or this
	// cell is occupied by a solid, we have no work to do.
	if (
		!SandPitCell_GetParticlePresent(*grain) || 
		SandPitCell_GetSimulateFlag(*grain) == simulate_tick ||
		SandPitCell_IsSolid(*grain)
	) {
		return false;
	}

	// Scan for objects below us.
	auto x_vel = SandPitCell_GetXVelocity(*grain);
	auto y_vel = SandPitCell_GetYVelocity(*grain);
	auto dx = (std::abs(x_vel) >= epoch) ? Bounds(x_vel, 1, -1) : 0;
	auto dy = (std::abs(y_vel) >= epoch) ? Bounds(y_vel, 1, -1) : 0;
	std::int64_t next_x = (std::int64_t)x + dx;
	std::int64_t next_y = (std::int64_t)y + dy;
	auto is_moving = dy != 0 || dx != 0;

	if (epoch == 0) {
		if (is_moving) {
			// Air resistance
			auto ease_x_vel = Bounds(-x_vel, 1, -1);			// TODO: friction coeff
			auto ease_y_vel = Bounds(-y_vel, 1, -1);
			x_vel += ease_x_vel;
			y_vel += ease_y_vel;
			SandPitCell_SetXVelocity(grain, x_vel);
			SandPitCell_SetYVelocity(grain, y_vel);
		}

		if (dy == 0) {
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
			else if (!SandPit_IsCellOccupied(pit, next_x + wind, next_y - pit->stubbornness)) {
				next_x += wind;
				is_moving = true;
			}
		}
	}

	if (SandPit_IsInBounds(pit, next_x, next_y)) {
		if (!is_moving) {
			SandPitCell_MarkAsSimulated(grain, simulate_tick);			// CRITICAL: Mark we have simulated this grain.
			return false;
		}
		else if (!SandPit_IsCellOccupied(pit, next_x, next_y)) {
			SandPitCell_MarkAsSimulated(grain, simulate_tick);			// CRITICAL: Mark we have simulated this grain.
			SandPit_MoveTo(pit, next_x, next_y, *grain);
			SandPit_ClearCell(pit, x, y);
			return true;
		}
	} else {
		// Slam against boundaries.
		// Convert to signed to provent underflow being set to max value.
		auto fixed_x = Bounds((std::int64_t)next_x, (std::int64_t)pit->w - 1, 0ll);
		if (fixed_x != next_x)
			SandPitCell_SetXVelocity(grain, 0);
		auto fixed_y = Bounds((std::int64_t)next_y, (std::int64_t)pit->h - 1, 0ll);
		if (fixed_y != next_y)
			SandPitCell_SetYVelocity(grain, 0);

		SandPitCell_MarkAsSimulated(grain, simulate_tick);			// CRITICAL: Mark we have simulated this grain.
		if ((fixed_x != next_x || fixed_y != next_y) && !SandPit_IsCellOccupied(pit, fixed_x, fixed_y)) {
			SandPit_MoveTo(pit, fixed_x, fixed_y, *grain);
			SandPit_ClearCell(pit, x, y);
			return true;
		}
	}
}

void SandPit_NextTick(SandPit* pit) {
	pit->simulate_tick = !pit->simulate_tick;
}

void SandPit_SimulateStep(SandPit* pit) {
	if (pit->sand_head == nullptr || pit->need_sim == false)
		return;

	bool any_movement{};
	std::uint32_t x{};
	std::uint32_t y{};
	for (int epoch = MAX_GRAIN_VELOCITY - 1; epoch >= 0; epoch--) {
		auto node_index = SandPitCell_GetNextIdx(*pit->sand_head);
		auto* node = &pit->world[node_index];
		while (!SandPitCell_IsListHead(*node)) {
			SandPit_IndexToCoords(pit, node_index, &x, &y);
			node_index = SandPitCell_GetNextIdx(*node);
			node = &pit->world[node_index];

			SandPit_DoCollisions(x, y, pit, epoch);
		} 

		SandPit_NextTick(pit);

		node_index = SandPitCell_GetNextIdx(*pit->sand_head);
		node = &pit->world[node_index];
		while (!SandPitCell_IsListHead(*node)) {
			SandPit_IndexToCoords(pit, node_index, &x, &y);
			node_index = SandPitCell_GetNextIdx(*node);
			node = &pit->world[node_index];

			// Spread over two lines to prevent short-circuit evaulation.
			auto movement = SandPit_SimulateGrain(x, y, pit, pit->simulate_tick, epoch);
			any_movement = any_movement || movement;				
		} 
	}

	pit->need_sim = any_movement;
}

void SandPit_AddImpulse(SandPit* pit, std::uint32_t x, std::uint32_t y, std::int32_t x_v, std::int32_t y_v)
{
	if (!SandPit_IsInBounds(pit, x, y))
		return;

	auto* grain = SandPit_Get(pit, x, y);
	if (!SandPitCell_GetParticlePresent(*grain))
		return;

	SandPitCell_SetXVelocity(grain, 
		Bounds(SandPitCell_GetXVelocity(*grain) + x_v, MAX_GRAIN_VELOCITY, -MAX_GRAIN_VELOCITY 
	));
	SandPitCell_SetYVelocity(grain, 
		Bounds(SandPitCell_GetYVelocity(*grain) + y_v, MAX_GRAIN_VELOCITY, -MAX_GRAIN_VELOCITY
	));

	pit->need_sim = true;
}

void SandPit_ClearRegion(SandPit* pit, const AABB* aabb, bool clear_solids) {
	double start_y = 0;
	double end_y = 0;
	double start_x = 0;
	double end_x = 0;
	AABB_GetCornerCoords(aabb, AABB_BOTTOM_LEFT, &start_x, &start_y);
	AABB_GetCornerCoords(aabb, AABB_TOP_RIGHT, &end_x, &end_y);

	for (int y = start_y; y < end_y; y++) {
		for (int x = start_x; x < end_x; x++) {
			if (!SandPit_IsInBounds(pit, x, y))
				continue;

			auto grain = pit->world[y * pit->w + x];
			if (!clear_solids && SandPitCell_IsSolid(grain))
				continue;

			SandPit_RemoveGrainAt(pit, x, y);
			SandPit_ClearCell(pit, x, y);
		}
	}
}

TWO_BIT_ID SandPit_GetIdAt(const SandPit* pit, std::uint32_t x, std::uint32_t y) {
	if (!SandPit_IsInBounds(pit, x, y))
		return PARTICLE_NOT_PRESENT;

	auto grain = SandPit_Get(pit, x, y);
	if (SandPitCell_GetParticlePresent(grain)) {
		return SandPitCell_GetId(grain);
	}
	return PARTICLE_NOT_PRESENT;
}

void SandPit_ForEachGrain(const SandPit* pit, SandPitForEachGrainCallback_t callback) {
	auto node_index = SandPitCell_GetNextIdx(*pit->sand_head);
	auto* node = &pit->world[node_index];
	std::uint32_t x{};
	std::uint32_t y{};
	while (!SandPitCell_IsListHead(*node)) {
		SandPit_IndexToCoords(pit, node_index, &x, &y);
		callback(SandPitCell_GetId(*node), x, y);

		node_index = SandPitCell_GetNextIdx(*node);
		node = &pit->world[node_index];
	} 
}

SandPitQueryResult SandPit_QueryRegion(const SandPit* pit, const AABB* aabb) {
	return SandPit_QueryRegion(pit, aabb->top_left_x, aabb->top_left_y, 2.0 * aabb->half_w, 2.0 * aabb->half_h);
}

SandPitQueryResult SandPit_QueryRegion(const SandPit* pit, std::uint32_t x0, std::uint32_t y0, std::uint32_t w, std::uint32_t h) {
	auto result = SandPitQueryResult{};
	result.x0 = x0;
	result.y0 = y0;
	result.w = w;
	result.h = h;
	result.highest_grain_y = -1;
	result.lowest_grain_y = 0b01111111111111111111111111111111;
	result.highest_solid_y = -1;
	result.lowest_solid_y = 0b01111111111111111111111111111111;

	for (int y = y0; y >= (int)y0 - (int)h; y--) {
		for (int x = x0; x <= (int)x0 + (int)w; x++) {
			if (SandPit_IsInBounds(pit, x, y) && SandPit_IsCellOccupied(pit, x, y)) {
				auto grain = SandPit_Get(pit, x, y);
				auto is_grain = !SandPitCell_IsSolid(grain);
				result.any_grain |= is_grain;
				result.any_solid |= !is_grain;

				if (is_grain) {
					result.grain_count += 1;
					result.highest_grain_y = std::max(result.highest_grain_y, y);
					result.lowest_grain_y = std::min(result.lowest_grain_y, y);
				} else {
					result.solid_count += 1;
					result.highest_solid_y = std::max(result.highest_solid_y, y);
					result.lowest_solid_y = std::min(result.lowest_solid_y, y);
				}
			}
		}
	}
	return result;
}
