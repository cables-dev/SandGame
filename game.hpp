#pragma once
#include "common.hpp"
#include "sand_sim.hpp"

constexpr auto PLAYER_HORIZONTAL_SPEED = 700.0;
constexpr auto PLAYER_HORIZONTAL_SLOW_SPEED = 1200.0;
constexpr auto PLAYER_HORIZONTAL_SLOW_SPEED_THRESH = 0.03 * PLAYER_HORIZONTAL_SLOW_SPEED;
constexpr auto PLAYER_MAX_SPEED = 1500.0;
constexpr auto PLAYER_AUTO_JUMP_HEIGHT = 3.0;
constexpr auto PLAYER_JUMP_SPEED = 400.0;
constexpr auto PLAYER_JUMP_EPSILON = SAND_SIZE / 2.0;
constexpr auto PLAYER_JUMP_SPEED_BOOST_X = 40.0;
constexpr auto PLAYER_WIDTH = 18;
constexpr auto PLAYER_HEIGHT = 30;
constexpr auto G = 900.0;
constexpr auto PLAYER_SAND_CAPACITY = 5000;
constexpr auto BARREL_EXPLOSION_DELAY_S = 0.5;
constexpr auto DOOR_LOADING_DELAY_S = 1.5;
constexpr auto DEFAULT_MAX_ENTITIES = 100;

struct Entity;						
struct Player;						
struct EntityVTable;						
enum EntityType;

enum PlayerFireType {			// Don't change order...
	FIRE_TYPE_STREAM,
	FIRE_TYPE_BURST,
	FIRE_TYPE_VACUUM,
	FIRE_TYPE_MAX
};

struct Player {
	AABB bbox{};
	double x_speed{};
	double y_speed{};
	bool do_jump{};
	PlayerFireType fire_mode{};
	int ammo{};
};

struct SandGamePersistentState {
	double frozen_for_s = 0.0;
};

struct SandGame {
	SandGamePersistentState* persistent = nullptr;			// Data that should be saved between levels
	RenderFXFlags fx_flags = NULL_FX_FLAGS;
	SoundFXFlags sfx_flags = NULL_SFX_FLAGS;
	GameActionFlags action_flags_pressed = NULL_ACTION_FLAGS;
	GameActionFlags action_flags_held = NULL_ACTION_FLAGS;;
	int cursor_x;
	int cursor_y;
	Player player;
	SandPit pit;
	Entity* entities = nullptr;						// TODO: make static array with compile-time size
	Entity* entities_head = nullptr;
	Entity* entities_top = nullptr;					// Knuth-style fixed-width free-store
	Entity* entities_avail = nullptr;
	std::uint32_t max_entities;
	const char* toast = "";
	const char* level_buffer = nullptr;		// Must be kept alive so entities can reference internal strings.
	const char* new_level_path = nullptr;
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
	SandGamePersistentState* persistent = nullptr
);
void SandGame_Destroy(SandGame* game, SandGamePersistentState** out_persistent_state);
void SandGame_Destroy(SandGame* game);
void SandGame_Update(SandGame* game, double dt);
Entity* SandGame_AddEntity(SandGame* game, void* entity, EntityType type);		// "entity" will be copied into a local buffer.
void SandGame_RemoveEntity(SandGame* game, Entity* entity);
void SandGame_ForEachEntity(const SandGame* game, SandGameForEachEntityFn_t cb);
void SandGame_SetToast(SandGame* game, const char* toast);
void SandGame_SetFXFlag(SandGame* game, RenderFX fx);
void SandGame_SetSFXFlag(SandGame* game, SoundFX sfx);
void SandGame_SetNewLevelPath(SandGame* game, const char* new_level_path);
const char* SandGame_GetNewLevelPath(const SandGame* game);						// nullptr if empty
bool SandGame_ShouldLoadNewLevel(const SandGame* game);
void SandGame_NotifyLevelLoaded(SandGame* game);
void SandGame_FreezeFor(SandGame* game, double s);


// ======== ENTITY CODE ======== 
using EntityPlaceFn_t = void(*)(Entity* _this, SandGame* game);
using EntityRemoveFn_t = void(*)(Entity* _this, SandGame* game);
using EntityThinkFn_t = void(*)(Entity* _this, SandGame* game, double dt);
struct EntityVTable {						// Each new entity must have this as their first member.
	EntityPlaceFn_t place_fn;
	EntityRemoveFn_t remove_fn;
	EntityThinkFn_t think_fn;
};

// ======== ENTITIES ======== 
struct EntityRectangleObstacle {
	EntityVTable vtable;						// !Important 
	AABB aabb;
	GameColour colour;
};
void RectangleObstacle_Create(EntityRectangleObstacle* rect, double top_left_x, double top_left_y, double w, double h, const GameColour colour);
void RectangleObstacle_Place(Entity* rect, SandGame* game);
void RectangleObstacle_Remove(Entity* rect, SandGame* game);
void RectangleObstacle_Think(Entity* rect, SandGame* game, double dt);

struct EntityHintBox {
	EntityVTable vtable;						// !Important 
	AABB aabb;
	const char* message;
	bool triggered;
	bool only_once;
};
void HintBox_Create(EntityHintBox* box, const char* message, bool only_once, double top_left_x, double top_left_y, double w, double h);
void HintBox_Place(Entity* rect, SandGame* game);
void HintBox_Remove(Entity* rect, SandGame* game);
void HintBox_Think(Entity* rect, SandGame* game, double dt);

struct EntityBarrel {
	EntityVTable vtable;						// !Important 
	AABB aabb;
	AABB sight_aabb;
	GraphicResource idle_sprite;
	GraphicResource explode_sprite;
	GraphicResource active_sprite;
	bool sprite_changed;
	bool fuse_lit;
	float time_until_explosion_s;
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

struct EntityLevelDoor {
	EntityVTable vtable;						// !Important 
	AABB aabb;
	const char* next_level_path;
};
void LevelDoor_Create(
	EntityLevelDoor* door,
	double top_left_x,
	double top_left_y,
	double w, double h,							// w,h should probably be fixed	
	const char* next_level_path
);
void LevelDoor_Place(Entity* ent, SandGame* game);
void LevelDoor_Remove(Entity* ent, SandGame* game);
void LevelDoor_Think(Entity* ent, SandGame* game, double dt);


// ======== ENTITIES ======== 



// For external code (i.e the renderer)
enum EntityType {
	ENTITY_RECTANGLE,
	ENTITY_HINT_BOX,
	ENTITY_BARREL,
	ENTITY_LEVEL_DOOR,
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
	} entity;
	EntityType type;
	Entity* _next;
	Entity* _prev;
};
Entity Entity_CreateFrom(void* instance, EntityType type);		
void Entity_Place(Entity* _this, SandGame* game);
void Entity_Remove(Entity* _this, SandGame* game);
void Entity_Think(Entity* _this, SandGame* game, double dt);


// ======= PLAYER ======= 
