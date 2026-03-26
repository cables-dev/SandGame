#include "entity.hpp"
#include "game.hpp"
#include <corecrt_memcpy_s.h>
#include <cassert>

void RectangleObstacle_Create(EntityRectangleObstacle* rect, double top_left_x, double top_left_y, double w, double h, const GameColour colour) {
	rect->vtbl = {
		RectangleObstacle_Place,
		RectangleObstacle_Remove,
		RectangleObstacle_Think,
		RectangleObstacle_Destroy,
	};
	AABB_Create(&rect->aabb, top_left_x, top_left_y, w, h);
	rect->colour = colour;
}

void RectangleObstacle_Place(Entity* ent, SandPit* pit) {
	auto* rect = &ent->entity.rect;
	auto aabb = rect->aabb;
	AABB_ScaleByReciprocal(&aabb, pit->grain_size);
	SandPit_PlaceSolidAABB(pit, &aabb);
}

void RectangleObstacle_Remove(Entity* ent, SandPit* pit) {
	auto* rect = &ent->entity.rect;
	SandPit_ClearRegion(pit, &rect->aabb, true);
}

void RectangleObstacle_Think(Entity* rect, SandPit* pit, Player* player, double dt) {
	// pass
}

void RectangleObstacle_Destroy(Entity* rect) {
	// pass
}

Entity Entity_CreateFrom(void* instance, EntityType type)
{
	Entity result{};
	switch (type) {
	case ENTITY_RECTANGLE: { memcpy_s(&result, sizeof(result), instance, sizeof(EntityRectangleObstacle)); break; }
	default: { assert("Entity_CreateFrom: Unaccounted entity type encountered." && false); }
	}

	result.type = type;
	return result;
}

void Entity_Place(Entity* _this, SandPit* pit) {
	_this->entity.vtable.place_fn(_this, pit);
}

void Entity_Remove(Entity* _this, SandPit* pit) {
	_this->entity.vtable.remove_fn(_this, pit);
}

void Entity_Think(Entity* _this, SandPit* pit, Player* player, double dt) {
	_this->entity.vtable.think_fn(_this, pit, player, dt);
}

void Entity_Destroy(Entity* _this) {
	_this->entity.vtable.destroy_fn(_this);
}
