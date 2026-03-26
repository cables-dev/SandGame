#pragma once
#include "common.hpp"
#include "player.hpp"
#include "sand_sim.hpp"

struct Entity;						
using EntityPlaceFn_t = void(*)(Entity* _this, SandPit* pit);
using EntityRemoveFn_t = void(*)(Entity* _this, SandPit* pit);
using EntityThinkFn_t = void(*)(Entity* _this, SandPit* pit, Player* player, double dt);
using EntityDestroyFn_t = void(*)(Entity* _this);
struct EntityVTable {						// Each new entity must have this as their first member.
	EntityPlaceFn_t place_fn;
	EntityRemoveFn_t remove_fn;
	EntityThinkFn_t think_fn;
	EntityDestroyFn_t destroy_fn;
};

// ======== ENTITIES ======== 
struct EntityRectangleObstacle {
	EntityVTable vtbl;						// !Important 
	AABB aabb;
	GameColour colour;
};
void RectangleObstacle_Create(EntityRectangleObstacle* rect, double top_left_x, double top_left_y, double w, double h, const GameColour colour);
void RectangleObstacle_Place(Entity* rect, SandPit* world);
void RectangleObstacle_Remove(Entity* rect, SandPit* world);
void RectangleObstacle_Think(Entity* rect, SandPit* world, Player* player, double dt);
void RectangleObstacle_Destroy(Entity* rect);





// For external code (i.e the renderer)
enum EntityType {
	ENTITY_RECTANGLE,
	ENTITY_MAX
};
struct Entity {
	// Polymorphism m'chud
	union {							// !Important: Add new entities here so we know how much space to reserve.
		EntityVTable vtable;
		EntityRectangleObstacle rect;
	} entity;
	EntityType type;
	Entity* _next;
	Entity* _prev;
};
Entity Entity_CreateFrom(void* instance, EntityType type);		
void Entity_Place(Entity* _this, SandPit* pit);
void Entity_Remove(Entity* _this, SandPit* pit);
void Entity_Think(Entity* _this, SandPit* pit, Player* player, double dt);
void Entity_Destroy(Entity* _this);

