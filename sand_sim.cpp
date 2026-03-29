#include "sand_sim.hpp"
#include <memory>
#include <chrono>
#include <iostream>
#include <cassert>

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

void SandPit_IndexToCoords(const SandPit* pit, std::uint32_t index, std::uint32_t* out_x, std::uint32_t* out_y) {
	*out_x = (index % pit->w);
	*out_y = (index / pit->w);
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

void SandPitCell_CreateListHead(
	SandPitCell* result,
	TWO_BIT_ID id, 
	bool simulate_flag, 
	std::int32_t x_vel, 
	std::int32_t y_vel,
	std::uint32_t index
) {
	*result = SandPitCell_Create(id, simulate_flag, x_vel, y_vel);
	SandPitCell_SetVisible(result, false);
	SandPitCell_SetListHeadFlag(result, true);
	SandPitCell_SetNextIdx(result, index);
	SandPitCell_SetPrevIdx(result, index);
}

bool SandPitSector_IsInBounds(const SandPitSector* sect, std::int64_t index) {
	return index < (sect->w * sect->h);
}

bool SandPitSector_IsInBounds(const SandPitSector* sect, std::int64_t x, std::int64_t y) {
	return x >= 0 && y >= 0 && x < sect->w && y < sect->h;
}

const SandPitCell* SandPitSector_Get(const SandPitSector* sect, std::uint32_t x, std::uint32_t y) {
	assert(SandPitSector_IsInBounds(sect, x, y) && "SandPitSector_Get: Out of bounds access.");
	return &sect->world[y * sect->w + x];
}

SandPitCell* SandPitSector_Get(SandPitSector* sect, std::uint32_t x, std::uint32_t y) {
	assert(SandPitSector_IsInBounds(sect, x, y) && "SandPitSector_Get: Out of bounds access.");
	return &sect->world[y * sect->w + x];
}

const SandPitCell* SandPitSector_Get(const SandPitSector* sect, std::uint32_t index) {
	assert(SandPitSector_IsInBounds(sect, index) && "SandPitSector_Get: Out of bounds access.");
	return &sect->world[index];
}

SandPitCell* SandPitSector_Get(SandPitSector* sect, std::uint32_t index) {
	assert(SandPitSector_IsInBounds(sect, index) && "SandPitSector_Get: Out of bounds access.");
	return &sect->world[index];
}

// Does not place cell in list.
void SandPitSector_DirectSet(SandPitSector* sect, std::uint32_t index, SandPitCell cell) {
	assert(SandPitSector_IsInBounds(sect, index) && "SandPitSector_Set: Out of bounds access.");
	sect->world[index] = cell;
}

// Does not place cell in list.
void SandPitSector_DirectSet(SandPitSector* sect, std::uint32_t x, std::uint32_t y, SandPitCell cell) {
	assert(SandPitSector_IsInBounds(sect, x, y) && "SandPitSector_Set: Out of bounds access.");
	sect->world[y * sect->w + x] = cell;
}

std::uint32_t SandPitSector_CoordsToIndex(SandPitSector* sect, std::uint32_t x, std::uint32_t y) {
	return y * sect->w + x;
}

// Will do nothing if there is no grain at (x, y).
void SandPitSector_RemoveGrainIfExists(SandPitSector* sector, std::int64_t x, std::int64_t y) {
	if (!SandPitSector_IsInBounds(sector, x, y))
		assert(false);
	assert(SandPitSector_IsInBounds(sector, x, y) && "SandPitSector_Set: Out of bounds access.");
	auto doomed_idx = SandPitSector_CoordsToIndex(sector, x, y);
	auto* cell = SandPitSector_Get(sector, doomed_idx);

	// Say goodbye to the hundred acre wrist...
	if (!SandPitCell_GetParticlePresent(*cell))
		return;

	// Remove from list
	SandPitCell_SetVisible(cell, false);
	auto prev_idx = SandPitCell_GetPrevIdx(*cell);
	auto next_idx = SandPitCell_GetNextIdx(*cell);
	auto* prev = SandPitSector_Get(sector, prev_idx);
	auto* next = SandPitSector_Get(sector, next_idx);
	SandPitCell_SetNextIdx(prev, next_idx);
	SandPitCell_SetPrevIdx(next, prev_idx);
}

void SandPitSector_AddGrain(
	SandPitSector* sect,
	std::uint32_t local_x,
	std::uint32_t local_y,
	SandPitCell grain 
) {
	auto grain_index = SandPitSector_CoordsToIndex(sect, local_x, local_y);
	SandPitSector_RemoveGrainIfExists(sect, local_x, local_y);				// Remove grain if needed

	if (sect->sand_head == nullptr) {					// Should never happen since we create a list head in the constructor...
		SandPitCell_SetListHeadFlag(&grain, true);
		SandPitCell_SetNextIdx(&grain, grain_index);
		SandPitCell_SetPrevIdx(&grain, grain_index);
		sect->sand_head = SandPitSector_Get(sect, local_x, local_y);
		sect->sand_head_index = grain_index;
	}
	else {
		auto* head = sect->sand_head;
		auto head_index = sect->sand_head_index;

		// Insert new particle
		auto head_next_index = SandPitCell_GetNextIdx(*head);
		auto* head_next = SandPitSector_Get(sect, head_next_index);
		SandPitCell_SetNextIdx(&grain, head_next_index);
		SandPitCell_SetPrevIdx(&grain, head_index);
		SandPitCell_SetPrevIdx(head_next, grain_index);
		SandPitCell_SetNextIdx(head, grain_index);
	}

	SandPitSector_DirectSet(sect, grain_index, grain);
}

void SandPitSector_TransformAbsoluteCoordsToLocal(
	const SandPitSector* sect,
	std::uint32_t x,
	std::uint32_t y,
	std::uint32_t* out_local_x,
	std::uint32_t* out_local_y
) {
	*out_local_x = x - sect->origin_x;
	*out_local_y = y - sect->origin_y;
}

void SandPitSector_IndexToCoords(
	const SandPitSector* pit, 
	std::uint32_t index, 
	std::uint32_t* out_x, 
	std::uint32_t* out_y
) {
	*out_x = (index % pit->w);
	*out_y = (index / pit->w);
}

SandPitSector* SandPit_GetSector(SandPit* pit, std::uint32_t sector_x, std::uint32_t sector_y) {
	return &pit->sectors[sector_y * pit->num_screens_horizontal + sector_x];
}

void SandPitSector_Init(
	SandPitSector* sect, 
	std::uint32_t origin_x_absolute,
	std::uint32_t origin_y_absolute,
	std::uint32_t sector_width, 
	std::uint32_t sector_height
) {
	SandPitCell sand_head;

	sect->origin_x = origin_x_absolute;
	sect->origin_y = origin_y_absolute;
	sect->w = sector_width;
	sect->h = sector_height;

	// Create sand list head in top left corner
	auto sand_head_index = SandPitSector_CoordsToIndex(sect, 0, sector_height - 1);
	SandPitCell_CreateListHead(&sand_head, 0, false, 0, 0, sand_head_index);
	SandPitSector_DirectSet(sect, sand_head_index, sand_head);
	sect->sand_head = SandPitSector_Get(sect, sand_head_index);
	sect->sand_head_index = sand_head_index;
}

void SandPit_InitSectors(
	SandPit* pit, 
	std::uint32_t num_sectors_horizontal, 
	std::uint32_t num_sectors_vertical
) {
	const auto sectors_size = sizeof(SandPitSector) * num_sectors_vertical * num_sectors_horizontal;
	pit->sectors = (SandPitSector*)(malloc(sectors_size));
	memset(pit->sectors, 0, sectors_size);

	for (int sect_y = 0; sect_y < num_sectors_vertical; sect_y++) {
		for (int sect_x = 0; sect_x < num_sectors_horizontal; sect_x++) {
			auto* sector = SandPit_GetSector(pit, sect_x, sect_y);
			SandPitSector_Init(
				sector, 
				sect_x * PIT_SECTOR_WIDTH, 
				sect_y * PIT_SECTOR_HEIGHT, 
				PIT_SECTOR_WIDTH, 
				PIT_SECTOR_HEIGHT
			);
		}
	}
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
	SandPitCell sand_head; 

	pit->num_screens_horizontal = num_screens_horizontal;
	pit->num_screens_vertical = num_screens_vertical;
	pit->w = num_screens_horizontal * w;
	pit->h = num_screens_vertical * h;
	pit->grain_size = grain_size;
	pit->sector_w = w;
	pit->sector_h = h;
	pit->stubbornness = stubbornness;

	SandPit_InitSectors(pit, num_screens_horizontal, num_screens_vertical);
}

void SandPit_Destroy(SandPit* pit) {
	free(pit->sectors);
}

void SandPitSector_Clear(
	SandPitSector* sector
) {
	for (int y = 0; y < sector->h; y++) {
		for (int x = 0; x < sector->w; x++) {
			auto* grain = SandPitSector_Get(sector, x, y);
			if (
				SandPitCell_GetParticlePresent(*grain) && 
				!SandPitCell_IsSolid(*grain) &&
				!SandPitCell_IsListHead(*grain)
			) {
				SandPitSector_RemoveGrainIfExists(sector, x, y);
			}
		}
	}
}

void SandPit_Clear(
	SandPit* pit,
	std::uint32_t sector_x,
	std::uint32_t sector_y
) {
	auto* sector = SandPit_GetSector(pit, sector_x, sector_y);
	SandPitSector_Clear(sector);
}

bool SandPit_IsInBounds(const SandPit* pit, std::int64_t x, std::int64_t y) {
	return (x >= 0 && x < pit->w) && (y >= 0 && y < pit->h);
}

SandPitSector* SandPit_GetSectorAtWorldCoords(SandPit* pit, std::uint32_t x, std::uint32_t y) {
	auto sect_y = y / pit->sector_h;
	auto sect_x = x / pit->sector_w;
	return &pit->sectors[sect_y * pit->num_screens_horizontal + sect_x];
}

const SandPitSector* SandPit_GetSectorAtWorldCoords(const SandPit* pit, std::uint32_t x, std::uint32_t y) {
	auto sect_y = y / pit->sector_h;
	auto sect_x = x / pit->sector_w;
	return &pit->sectors[sect_y * pit->num_screens_horizontal + sect_x];
}

const SandPitSector* SandPit_GetSector(const SandPit* pit, std::uint32_t sector_x, std::uint32_t sector_y) {
	return &pit->sectors[sector_y * pit->num_screens_horizontal + sector_x];
}

const SandPitCell* SandPit_Get(const SandPit* pit, std::uint32_t x, std::uint32_t y) {
	std::uint32_t local_x;
	std::uint32_t local_y;
	auto* sector = SandPit_GetSectorAtWorldCoords(pit, x, y);
	SandPitSector_TransformAbsoluteCoordsToLocal(sector, x, y, &local_x, &local_y);
	return SandPitSector_Get(sector, local_x, local_y);
}

//SandPitCell* SandPit_Get(SandPit* pit, std::uint32_t x, std::uint32_t y) {
//	return &pit->world[y * pit->w + x];
//}

//void SandPit_Set(SandPit* pit, std::uint32_t x, std::uint32_t y, SandPitCell cell) {
//	pit->world[y * pit->w + x] = cell;
//}

//void SandPit_Set(SandPit* pit, std::uint32_t index, SandPitCell cell) {
//	pit->world[index] = cell;
//}

// Will do nothing if there is no grain at (x, y).
void SandPit_RemoveGrainAt(SandPit* pit, std::uint32_t x, std::uint32_t y) {
	auto* sector = SandPit_GetSectorAtWorldCoords(pit, x, y);
	auto local_x = 0u;
	auto local_y = 0u;
	SandPitSector_TransformAbsoluteCoordsToLocal(sector, x, y, &local_x, &local_y);
	SandPitSector_RemoveGrainIfExists(sector, local_x, local_y);
}

void SandPitSector_ClearCell(SandPitSector* sect, std::uint32_t x, std::uint32_t y) {
	sect->world[y * sect->w + x] &= ~SAND_PIT_CELL_PARTICLE_PRESENT_MASK;
}

void SandPit_MoveTo(
	SandPit* pit, 
	std::uint32_t from_x, 
	std::uint32_t from_y,
	std::uint32_t to_x, 
	std::uint32_t to_y, 
	SandPitCell val
) {
	if (!(SandPit_IsInBounds(pit, from_x, from_y) && SandPit_IsInBounds(pit, to_x, to_y)))
		assert(false);

	// Have we moved?
	if (from_x == to_x && from_y == to_y)
		return;

	std::uint32_t local_x;
	std::uint32_t local_y;
	std::uint32_t new_idx;

	auto* from_sector = SandPit_GetSectorAtWorldCoords(pit, from_x, from_y);
	SandPitSector_TransformAbsoluteCoordsToLocal(from_sector, from_x, from_y, &local_x, &local_y);
	auto* old_grain = SandPitSector_Get(from_sector, local_x, local_y);

	if (!SandPitCell_GetParticlePresent(*old_grain))
		return;

	auto* to_sector = SandPit_GetSectorAtWorldCoords(pit, to_x, to_y);
	if (from_sector != to_sector) {		// x-sector transition
		// Remove grain from previous sector
		SandPitSector_TransformAbsoluteCoordsToLocal(from_sector, from_x, from_y, &local_x, &local_y);
		SandPitSector_RemoveGrainIfExists(from_sector, local_x, local_y);

		SandPitSector_TransformAbsoluteCoordsToLocal(to_sector, to_x, to_y, &local_x, &local_y);
		new_idx = SandPitSector_CoordsToIndex(to_sector, local_x, local_y);

		SandPitSector_AddGrain(to_sector, local_x, local_y, val);
		to_sector->need_sim = true;
	}
	else {
		SandPitSector_TransformAbsoluteCoordsToLocal(from_sector, to_x, to_y, &local_x, &local_y);
		new_idx = SandPitSector_CoordsToIndex(from_sector, local_x, local_y);

		// Point adjacent grains to new location
		auto prev_idx = SandPitCell_GetPrevIdx(val);
		auto* prev = SandPitSector_Get(from_sector, prev_idx);
		SandPitCell_SetNextIdx(prev, new_idx);
		auto next_idx = SandPitCell_GetNextIdx(val);
		auto* next = SandPitSector_Get(from_sector, next_idx);
		SandPitCell_SetPrevIdx(next, new_idx);

		SandPitSector_DirectSet(from_sector, new_idx, val);
	}

	SandPitCell_SetVisible(old_grain, false);
}

// Returns true if (x, y) is out-of-bounds.
bool SandPitSector_IsCellOccupied(const SandPitSector* sect, const SandPit* parent, std::int64_t local_x, std::int64_t local_y) {
	// out-of-bounds
	if (!SandPit_IsInBounds(parent, local_x + sect->origin_x, local_y + sect->origin_y))
		return true;

	auto* grain = SandPitSector_Get(sect, local_x, local_y);
	return SandPitCell_GetParticlePresent(*grain) || SandPitCell_IsListHead(*grain);				// Don't overwrite the list head!
}

// Returns true if (x, y) is out-of-bounds.
bool SandPit_IsCellOccupied(const SandPit* pit, std::uint32_t x, std::uint32_t y) {
	if (!SandPit_IsInBounds(pit, x, y))
		return true;

	auto* grain = SandPit_Get(pit, x, y);
	return SandPitCell_GetParticlePresent(*grain) || SandPitCell_IsListHead(*grain);				// Don't overwrite the list head!
}

// Add grain to internal list. Will insert grain into world.
void SandPit_AddGrain(SandPit* pit, std::uint32_t x, std::uint32_t y, SandPitCell grain) {
	std::uint32_t local_x;
	std::uint32_t local_y;

	if (!SandPit_IsInBounds(pit, x, y))
		return;

	auto* sector = SandPit_GetSectorAtWorldCoords(pit, x, y);
	SandPitSector_TransformAbsoluteCoordsToLocal(sector, x, y, &local_x, &local_y);
	SandPitSector_AddGrain(sector, local_x, local_y, grain);
}

// Returns true on success
bool SandPit_PlaceGrain(SandPit* pit, std::uint32_t x, std::uint32_t y, int particle_id) {
	if (!SandPit_IsInBounds(pit, x, y))
		return false;

	std::uint32_t local_x;
	std::uint32_t local_y;
	auto* sector = SandPit_GetSectorAtWorldCoords(pit, x, y);
	if (SandPit_IsCellOccupied(pit, x, y))
		return false;

	SandPitSector_TransformAbsoluteCoordsToLocal(sector, x, y, &local_x, &local_y);
	SandPitSector_AddGrain(sector, local_x, local_y, SandPitCell_Create(particle_id, sector->simulate_tick, 0, 0, false));
	sector->need_sim = true;
	return true;
}

void SandPit_PlaceSolid(SandPit* pit, std::uint32_t x, std::uint32_t y) {
	std::uint32_t local_x;
	std::uint32_t local_y;

	if (!SandPit_IsInBounds(pit, x, y))
		return;

	if (SandPit_IsCellOccupied(pit, x, y))
		return;

	auto* sector = SandPit_GetSectorAtWorldCoords(pit, x, y);
	SandPitSector_TransformAbsoluteCoordsToLocal(sector, x, y, &local_x, &local_y);
	SandPitSector_DirectSet(sector, local_x, local_y, SandPitCell_Create(0, sector->simulate_tick, 0, 0, true));
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
	return (std::uint8_t)(PseudoRandom_GetU32());
}

void SandPitSector_DoCollisions(
	SandPitSector* sector, 
	std::uint32_t x, 
	std::uint32_t y, 
	std::uint32_t epoch
) {
	auto* grain = SandPitSector_Get(sector, x, y);
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
	if (SandPitSector_IsInBounds(sector, next_x, next_y)) {
		auto* grain_ptr = grain;
		auto* collision = SandPitSector_Get(sector, next_x, next_y);
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
			if (!SandPitSector_IsInBounds(sector, next_x, next_y)) {
				return;
			}

			grain_ptr = collision;
			collision = SandPitSector_Get(sector, next_x, next_y);
		}
	}
}

int SandPit_GetRandomWindImpulse() {
	// Either -1, 0 or 1
	auto result = SandPit_PseudoRand() % (12 + 1);			// Random number from 0 - 12
	result -= 6;							// Random number from -6 to 6
	return result / 5;						
}

// Returns true if there was movement.
bool SandPitSector_SimulateGrain(
	SandPitSector* sect, 
	SandPit* parent,
	std::uint32_t local_x, 
	std::uint32_t local_y, 
	bool simulate_tick, 
	std::uint32_t epoch
) {
	std::uint32_t next_local_x;
	std::uint32_t next_local_y;
	auto abs_x = local_x + sect->origin_x;
	auto abs_y = local_y + sect->origin_y;
	auto* grain = SandPitSector_Get(sect, local_x, local_y);

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
	std::int64_t next_x = (std::int64_t)local_x + dx;
	std::int64_t next_y = (std::int64_t)local_y + dy;
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
			if (!SandPit_IsCellOccupied(parent, next_x + sect->origin_x, next_y + sect->origin_y - 1)) {
				next_y -= 1;

				if (!SandPit_IsCellOccupied(parent, next_x  + sect->origin_x + wind, next_y + sect->origin_y)) {
					next_x += wind;
				}
				is_moving = true;
			}
			// Is our pillar unstable?
			else if (!SandPit_IsCellOccupied(parent, next_x + wind + sect->origin_x, next_y - parent->stubbornness + sect->origin_y)) {
				next_x += wind;
				is_moving = true;
			}
		}
	}

	if (SandPit_IsInBounds(parent, next_x + sect->origin_x, next_y + sect->origin_y)) {
		if (!is_moving) {
			SandPitCell_MarkAsSimulated(grain, simulate_tick);			// CRITICAL: Mark we have simulated this grain.
			return false;
		}
		else if (!SandPit_IsCellOccupied(parent, next_x + sect->origin_x, next_y + sect->origin_y)) {
			SandPitCell_MarkAsSimulated(grain, simulate_tick);			// CRITICAL: Mark we have simulated this grain.
			SandPit_MoveTo(parent, abs_x, abs_y, next_x + sect->origin_x, next_y + sect->origin_y, *grain);
			//SandPit_ClearCell(pit, x, y);
			return true;
		}
	} else {
		// Slam against parent boundaries.
		auto abs_next_x = (std::int64_t)(next_x + sect->origin_x);
		auto abs_next_y = (std::int64_t)(next_y + sect->origin_y);
		// Convert to signed to provent underflow being set to max value.
		auto abs_fixed_x = Bounds(abs_next_x, (std::int64_t)parent->w - 1, 0ll);
		if (abs_fixed_x != abs_next_x)
			SandPitCell_SetXVelocity(grain, 0);

		auto abs_fixed_y = Bounds(abs_next_y, (std::int64_t)parent->h - 1, 0ll);
		if (abs_fixed_y != abs_next_y)
			SandPitCell_SetYVelocity(grain, 0);

		SandPitCell_MarkAsSimulated(grain, simulate_tick);			// CRITICAL: Mark we have simulated this grain.
		if (
			(abs_fixed_x != abs_next_x || abs_fixed_y != abs_next_y) && 
			!SandPit_IsCellOccupied(parent, abs_fixed_x, abs_fixed_y)
		) {
			SandPitSector_TransformAbsoluteCoordsToLocal(sect, abs_fixed_x, abs_fixed_y, &local_x, &local_y);
			SandPit_MoveTo(parent, abs_x, abs_y, abs_fixed_x, abs_fixed_y, *grain);
			//SandPit_ClearCell(pit, x, y);
			return true;
		}
	}
	return true;			// ? 
}

void SandPitSector_NextTick(SandPitSector* sect) {
	sect->simulate_tick = !sect->simulate_tick;
}

bool SandPit_IsSectorInBounds(
	SandPit* pit,
	std::uint32_t sector_x,
	std::uint32_t sector_y
) {
	return sector_x < pit->num_screens_horizontal && sector_y < pit->num_screens_vertical;
}

void SandPit_SimulateStep(
	SandPit* pit, 
	std::uint32_t sector_x, 
	std::uint32_t sector_y
) {
	bool any_movement{};
	std::uint32_t x{};
	std::uint32_t y{};

	if (!SandPit_IsSectorInBounds(pit, sector_x, sector_y))
		assert(false);

	auto* sector = SandPit_GetSector(pit, sector_x, sector_y);
	if (sector->sand_head == nullptr || sector->need_sim == false)
		return;

	for (int epoch = MAX_GRAIN_VELOCITY - 1; epoch >= 0; epoch--) {
		auto node_index = SandPitCell_GetNextIdx(*sector->sand_head);
		auto* node = SandPitSector_Get(sector, node_index);
		while (!SandPitCell_IsListHead(*node)) {
			SandPitSector_IndexToCoords(sector, node_index, &x, &y);
			node_index = SandPitCell_GetNextIdx(*node);
			node = SandPitSector_Get(sector, node_index);

			SandPitSector_DoCollisions(sector, x, y, epoch);
		} 

		SandPitSector_NextTick(sector);

		node_index = SandPitCell_GetNextIdx(*sector->sand_head);
		node = SandPitSector_Get(sector, node_index);
		while (!SandPitCell_IsListHead(*node)) {
			SandPitSector_IndexToCoords(sector, node_index, &x, &y);
			node_index = SandPitCell_GetNextIdx(*node);
			node = SandPitSector_Get(sector, node_index);

			// Spread over two lines to prevent short-circuit evaulation.
			auto movement = SandPitSector_SimulateGrain(sector, pit, x, y, sector->simulate_tick, epoch);
			any_movement = any_movement || movement;				
		} 
	}

	sector->need_sim = any_movement;
}

void SandPit_AddImpulse(SandPit* pit, std::uint32_t x, std::uint32_t y, std::int32_t x_v, std::int32_t y_v) {
	std::uint32_t local_x;
	std::uint32_t local_y;

	if (!SandPit_IsInBounds(pit, x, y))
		return;

	auto* sector = SandPit_GetSectorAtWorldCoords(pit, x, y);
	SandPitSector_TransformAbsoluteCoordsToLocal(sector, x, y, &local_x, &local_y);
	auto* grain = SandPitSector_Get(sector, local_x, local_y);
	if (!SandPitCell_GetParticlePresent(*grain))
		return;

	SandPitCell_SetXVelocity(
		grain, 
		Bounds(SandPitCell_GetXVelocity(*grain) + x_v, MAX_GRAIN_VELOCITY, -MAX_GRAIN_VELOCITY)
	);
	SandPitCell_SetYVelocity(
		grain, 
		Bounds(SandPitCell_GetYVelocity(*grain) + y_v, MAX_GRAIN_VELOCITY, -MAX_GRAIN_VELOCITY)
	);

	sector->need_sim = true;
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

			auto* grain = SandPit_Get(pit, x, y);
			if (!clear_solids && SandPitCell_IsSolid(*grain))
				continue;

			SandPit_RemoveGrainAt(pit, x, y);
			//SandPit_ClearCell(pit, x, y);
		}
	}
}

void SandPit_WorldCoordsToSectorCoords(const SandPit* pit, std::uint32_t world_x, std::uint32_t world_y, std::uint32_t* out_sector_x, std::uint32_t* out_sector_y) {
	*out_sector_x = world_x / pit->sector_w;
	*out_sector_y = world_y / pit->sector_h;
}

TWO_BIT_ID SandPit_GetIdAt(const SandPit* pit, std::uint32_t x, std::uint32_t y) {
	if (!SandPit_IsInBounds(pit, x, y))
		return PARTICLE_NOT_PRESENT;

	auto* grain = SandPit_Get(pit, x, y);
	if (SandPitCell_GetParticlePresent(*grain)) {
		return SandPitCell_GetId(*grain);
	}
	return PARTICLE_NOT_PRESENT;
}

void SandPit_ForEachGrain(
	const SandPit* pit, 
	std::uint32_t sector_x,
	std::uint32_t sector_y,
	SandPitForEachGrainCallback_t callback
) {
	std::uint32_t x{};
	std::uint32_t y{};
	auto* sector = SandPit_GetSector(pit, sector_x, sector_y);
	auto node_index = SandPitCell_GetNextIdx(*sector->sand_head);
	auto* node = SandPitSector_Get(sector, node_index);

	while (!SandPitCell_IsListHead(*node)) {
		SandPitSector_IndexToCoords(sector, node_index, &x, &y);
		callback(SandPitCell_GetId(*node), x, y);

		node_index = SandPitCell_GetNextIdx(*node);
		node = SandPitSector_Get(sector, node_index);
	} 
}

SandPitQueryResult SandPit_QueryRegion(const SandPit* pit, const AABB* aabb) {
	return SandPit_QueryRegion(pit, aabb->top_left_x, aabb->top_left_y, 2.0 * aabb->half_w, 2.0 * aabb->half_h);
}

SandPitQueryResult SandPit_QueryRegion(
	const SandPit* pit, 
	std::uint32_t x0, 
	std::uint32_t y0, 
	std::uint32_t w, 
	std::uint32_t h
) {
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
				auto* grain = SandPit_Get(pit, x, y);
				auto is_grain = !SandPitCell_IsSolid(*grain);
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
