#pragma once
#include "engine/engine.hpp"
#include "common.hpp"
#include "sand_sim.hpp"

constexpr auto PLAYER_HORIZONTAL_SPEED = 700.0;
constexpr auto PLAYER_HORIZONTAL_SLOW_SPEED = 1200.0;
constexpr auto PLAYER_HORIZONTAL_SLOW_SPEED_THRESH = 0.03 * PLAYER_HORIZONTAL_SLOW_SPEED;
constexpr auto PLAYER_MAX_SPEED = 1900.0;
constexpr auto PLAYER_IS_GOING_FAST_PERCENT = 0.80;
constexpr auto PLAYER_IS_GOING_FAST_VELOCITY_THRESHHOLD_SQUARED = PLAYER_MAX_SPEED * PLAYER_MAX_SPEED * PLAYER_IS_GOING_FAST_PERCENT ;
constexpr auto PLAYER_AUTO_JUMP_HEIGHT = 3.0;
constexpr auto PLAYER_JUMP_SPEED = 400.0;
constexpr auto PLAYER_JUMP_EPSILON = SAND_SIZE / 2.0;
constexpr auto PLAYER_JUMP_SPEED_BOOST_X = 40.0;
constexpr auto PLAYER_WIDTH = 24;
constexpr auto PLAYER_HEIGHT = 40;
constexpr auto MIN_DISTANCE_FOR_SAND_PLACE_MODE = 100;
constexpr auto G = 900.0;
constexpr auto PLAYER_SAND_CAPACITY = 75000;
constexpr auto BARREL_EXPLOSION_DELAY_S = 0.5;
constexpr auto DOOR_LOADING_DELAY_S = 1.5;
constexpr auto DEFAULT_MAX_ENTITIES = 1000;

struct Entity;						
struct Player;						
struct EntityVTable;						
enum EntityType;

enum PlayerFireType {			// Don't change order...
	FIRE_TYPE_STREAM,
	FIRE_TYPE_PLACE,
	FIRE_TYPE_VACUUM,
	FIRE_TYPE_MAX
};

enum EntityDirection {
	ENTITY_FACING_LEFT,
	ENTITY_FACING_RIGHT
};

struct Player {
	AABB bbox{};
	double x_speed{};
	double y_speed{};
	bool do_jump{};
	PlayerFireType fire_mode{};
	int ammo{};
	EntityDirection direction = ENTITY_FACING_RIGHT;
};

struct SandGamePersistentState {
	double elapsed_s{};
	std::uint32_t door_lock_flags{};
};

struct SandGame {
	SandGamePersistentState* persistent = nullptr;			// Data that should be saved between levels
	RenderFXFlags fx_flags = NULL_FX_FLAGS;
	SoundFXFlags sfx_flags = NULL_SFX_FLAGS;
	GameActionFlags action_flags_pressed = NULL_ACTION_FLAGS;
	GameActionFlags action_flags_held = NULL_ACTION_FLAGS;
	double frozen_for_s = 0.0;
	int cursor_x{};
	int cursor_y{};
	Player player{};
	SandPit pit{};
	Entity* entities = nullptr;						// TODO: make static array with compile-time size
	Entity* entities_head = nullptr;
	Entity* entities_top = nullptr;					// Knuth-style fixed-width free-store
	Entity* entities_avail = nullptr;
	std::uint32_t max_entities;
	const char* toast = nullptr;
	NEEDS_FREE const char* level_buffer = nullptr;  // The currently loaded level file. Must be kept alive so entities can reference internal strings like string toast messages. 
	const char* new_level_path = nullptr;
	bool do_time_tick = true;
	int skip_frame = 3;								// This fixes a bug I cba to find
};
using SandGameForEachEntityFn_t = void(*)(Entity*);

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
	std::uint32_t max_entities,
	SandGamePersistentState* persistent = nullptr,
	bool do_timer = true 
);
void SandGame_Destroy(SandGame* game, SandGamePersistentState** out_persistent_state);
void SandGame_Destroy(SandGame* game);
void SandGame_ReceiveInput(
	SandGame* game,
	GameActionFlags* pressed,
	GameActionFlags* held,
	int cursor_x,
	int cursor_y 
);
void SandGame_Update(
	SandGame* game, 
	GameActionFlags* pressed, 
	GameActionFlags* held, 
	int cursor_x, 
	int cursor_y, 
	EngineTime dt
);
void SandGame_ScreenCoordsToWorldCoords(
	const SandGame* game,
	int screen_x,
	int screen_y,
	int* out_world_x,
	int* out_world_y
);
Entity* SandGame_AddEntity(SandGame* game, void* entity, EntityType type);		// "entity" will be copied into a local buffer.
void SandGame_RemoveEntity(SandGame* game, Entity* entity);
void SandGame_ForEachEntity(const SandGame* game, SandGameForEachEntityFn_t cb);
void SandGame_SetToast(SandGame* game, const char* toast, bool do_sound=true);
void SandGame_SetFXFlag(SandGame* game, RenderFXFlag fx);
void SandGame_SetSFXFlag(SandGame* game, SoundFXFlag sfx, bool to=true);
void SandGame_SetNewLevelPath(SandGame* game, const char* new_level_path);
const char* SandGame_GetNewLevelPath(const SandGame* game);						// nullptr if empty
bool SandGame_ShouldLoadNewLevel(const SandGame* game);
void SandGame_NotifyLevelLoaded(SandGame* game);
void SandGame_FreezeFor(SandGame* game, double s);
double SandGame_GetElapsedSeconds(const SandGame* game);
bool SandGame_IsLockFlagUnLocked(const SandGame* game, int lock_flag);
void SandGame_SetUnLockFlag(const SandGame* game, int lock_flag, bool to=true);
int SandGame_GetNumEntities(const SandGame* game);


// ======== ENTITY CODE ======== 
using EntityPlaceFn_t = void(*)(Entity* _this, SandGame* game);
using EntityRemoveFn_t = void(*)(Entity* _this, SandGame* game);
using EntityThinkFn_t = void(*)(Entity* _this, SandGame* game, double dt);
using EntityGetAABBFn_t = AABB*(*)(Entity* _this);
struct EntityVTable {						// Each new entity must have this as their first member.
	EntityPlaceFn_t place_fn;
	EntityRemoveFn_t remove_fn;
	EntityThinkFn_t think_fn;
	EntityGetAABBFn_t get_aabb_fn;
};

// ======== ENTITIES ======== 
struct EntityRectangleObstacle {
	EntityVTable vtable;						// !Important 
	AABB aabb{};
	GameColour colour{};
	GraphicResource graphic{};
};
void RectangleObstacle_Create(
	EntityRectangleObstacle* rect, 
	double top_left_x, 
	double top_left_y, 
	double w, 
	double h, 
	const GameColour colour
);
void RectangleObstacle_Create(
	EntityRectangleObstacle* rect, 
	double top_left_x, 
	double top_left_y, 
	double w, 
	double h, 
	const GameColour colour,
	GraphicResource graphic
);
void RectangleObstacle_Place(Entity* rect, SandGame* game);
void RectangleObstacle_Remove(Entity* rect, SandGame* game);
void RectangleObstacle_Think(Entity* rect, SandGame* game, double dt);
AABB* RectangleObstacle_GetAABB(Entity* rect);

struct EntityHintBox {
	EntityVTable vtable;						// !Important 
	AABB aabb{};
	const char* message{};
	bool already_triggered{};
	bool only_once{};
	AudioResource audio_rsc{};
};
void HintBox_Create(
	EntityHintBox* box, 
	const char* message, 
	bool only_once, 
	double top_left_x, 
	double top_left_y, 
	double w, 
	double h
);
void HintBox_Create(
	EntityHintBox* box,
	const char* message,
	bool only_once,
	double top_left_x,
	double top_left_y,
	double w,
	double h,
	AudioResource rsc
);
void HintBox_Place(Entity* rect, SandGame* game);
void HintBox_Remove(Entity* rect, SandGame* game);
void HintBox_Think(Entity* rect, SandGame* game, double dt);
AABB* HintBox_GetAABB(Entity* rect);

struct EntityBarrel {
	EntityVTable vtable;						// !Important 
	AABB aabb{};
	AABB sight_aabb{};
	GraphicResource idle_sprite{-1};
	GraphicResource explode_sprite{-1};
	GraphicResource active_sprite{-1};
	bool sprite_changed{};
	bool fuse_lit{};
	bool defused = false;
	float time_until_explosion_s{};
};
void Barrel_Create(
	EntityBarrel* box, 
	double top_left_x, 
	double bottom_y, 
	double w, double h,							// w,h should probably be fixed
	GraphicResource idle_sprite, 
	GraphicResource explode_sprite
);		
void Barrel_Place(Entity* rect, SandGame* game);
void Barrel_Remove(Entity* rect, SandGame* game);
void Barrel_Think(Entity* rect, SandGame* game, double dt);
AABB* Barrel_GetAABB(Entity* rect);

struct EntityLevelDoor {
	EntityVTable vtable;						// !Important 
	AABB aabb{};
	const char* next_level_path{};
	int lock_flag{};
	int unlock_flag{};
};
void LevelDoor_Create(
	EntityLevelDoor* door,
	double top_left_x,
	double top_left_y,
	double w, double h,							// w,h should probably be fixed	
	const char* next_level_path,
	int lock_flag = -1,
	int unlock_flag = -1
);
void LevelDoor_Place(Entity* ent, SandGame* game);
void LevelDoor_Remove(Entity* ent, SandGame* game);
void LevelDoor_Think(Entity* ent, SandGame* game, double dt);
bool LevelDoor_HasLock(EntityLevelDoor* door);
bool LevelDoor_IsUnlocked(EntityLevelDoor* door, const SandGame* game);
AABB* LevelDoor_GetAABB(Entity* door);

constexpr auto LADYBIRD_WIDTH = 15;
constexpr auto LADYBIRD_HEIGHT = 7;
constexpr auto LADYBIRD_MOVE_TRY_DELAY_S = 2.0;
constexpr auto LADYBIRD_MOVE_FOR_S = 1.0;
constexpr auto LADYBIRD_SPEED_X = 3.0;
constexpr auto LADYBIRD_VISION_DISTANCE = 500.0;
constexpr auto LADYBIRD_DESPAWN_DISTANCE = 2000.0;
constexpr auto LADYBIRD_FLIGHT_SPEED = 400.0;
enum LadybirdState {
	LADYBIRD_IDLE,
	LADYBIRD_MOVING,
	LADYBIRD_SHOCKED,
	LADYBIRD_FLIGHT
};
struct EntityLadybird {
	EntityVTable vtable;						// !Important 
	AABB aabb{};
	LadybirdState state = LADYBIRD_IDLE;
	EntityDirection direction = ENTITY_FACING_LEFT;
	double last_move = 9.0;
	bool new_state = false;
	double time_until_move = 0.0;
	double move_to_x{};
	double move_to_y{};
};
void Ladybird_Create(EntityLadybird* ladybird, double x, double y);
void Ladybird_GetFeet(const EntityLadybird* ladybird, double* out_x, double* out_y);
void Ladybird_Place(Entity* ent, SandGame* game);
void Ladybird_Remove(Entity* ent, SandGame* game);
void Ladybird_Think(Entity* ent, SandGame* game, double dt);
AABB* Ladybird_GetAABB(Entity* ent);
// ======== ENTITIES ======== 



// For external code (i.e the renderer)
enum EntityType : std::uint32_t {
	ENTITY_RECTANGLE,
	ENTITY_HINT_BOX,
	ENTITY_BARREL,
	ENTITY_LEVEL_DOOR,
	ENTITY_LADYBIRD,
	ENTITY_MAX
};
struct Entity {
	// Polymorphism m'chud
	union {							// !Important: Add new entities here so we know how much space to reserve.
		EntityVTable vtable;
		EntityRectangleObstacle rect;
		EntityHintBox hint_box;
		EntityBarrel barrel;
		EntityLevelDoor door;
		EntityLadybird ladybird;
	} entity;
	EntityType type;
	Entity* _next{};
	Entity* _prev{};
};

Entity Entity_CreateFrom(void* instance, EntityType type);		
void Entity_Place(Entity* _this, SandGame* game);
void Entity_Remove(Entity* _this, SandGame* game);
void Entity_Think(Entity* _this, SandGame* game, double dt);
AABB* Entity_GetAABB(Entity* _this);

// ======= PLAYER ======= 
EntityDirection Player_GetDirection(const Player* player);
bool Player_IsMoving(const Player* player);
bool Player_IsGoingFast(const Player* player);
int PlaceSandCircle(SandPit* p, std::int32_t x, std::int32_t y, std::int32_t r, SandPitQueryResult* out_region_query = nullptr);
