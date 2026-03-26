#include "player.hpp"
#include "common.hpp"
#include <Math.h>
#include <utility>
#include <cstdlib>

PlayerFireType NextFireType(PlayerFireType type) {
	return (PlayerFireType)((type + 1) % FIRE_TYPE_MAX);
}

void SetPlayerXVelocity(Player* player, double to) {
	player->x_speed = Bounds(to, PLAYER_MAX_SPEED, -PLAYER_MAX_SPEED);
}

void IncrementPlayerXVelocity(Player* player, double by) {
	SetPlayerXVelocity(player, player->x_speed + by);
}

void SetPlayerYVelocity(Player* player, double to) {
	player->y_speed = Bounds(to, PLAYER_MAX_SPEED, -PLAYER_MAX_SPEED);
}

void IncrementPlayerYVelocity(Player* player, double by) {
	SetPlayerYVelocity(player, player->y_speed + by);
}

void ApplyPlayerFriction(Player* player, float dt) {
	// Slow to a halt
	auto friction_delta_speed = (player->x_speed > 0) ? -PLAYER_HORIZONTAL_SLOW_SPEED * dt : PLAYER_HORIZONTAL_SLOW_SPEED * dt;
	if (-PLAYER_HORIZONTAL_SLOW_SPEED_THRESH <= player->x_speed && player->x_speed <= PLAYER_HORIZONTAL_SLOW_SPEED_THRESH)
		SetPlayerXVelocity(player, 0.0);
	else
		IncrementPlayerXVelocity(player, friction_delta_speed);
}

void DoPlayerMovement(Player* player, SandPit* world, bool left, bool right, bool jump, float dt) {
	// Buttons
	if (left) {
		auto delta_speed = (player->x_speed > 0) ? PLAYER_HORIZONTAL_SLOW_SPEED : PLAYER_HORIZONTAL_SPEED;
		IncrementPlayerXVelocity(player, -delta_speed * dt);
	}
	else if (right) {
		auto delta_speed = (player->x_speed < 0) ? PLAYER_HORIZONTAL_SLOW_SPEED : PLAYER_HORIZONTAL_SPEED;
		IncrementPlayerXVelocity(player, delta_speed * dt);
	}
	else {
		ApplyPlayerFriction(player, dt);
	}

	auto player_bbox_sand_coords = player->bbox;
	AABB_ScaleByReciprocal(&player_bbox_sand_coords, world->grain_size);
	auto player_w_sand = AABB_GetWidth(&player_bbox_sand_coords);
	auto player_h_sand = AABB_GetHeight(&player_bbox_sand_coords);

	double player_x_sand = 0;
	double player_y_sand = 0;
	AABB_GetCornerCoords(&player_bbox_sand_coords, AABB_BOTTOM, &player_x_sand, &player_y_sand);
	double prev_player_x_sand = player_x_sand;
	double prev_player_y_sand = player_y_sand;

	auto projected_player_x_sand = player_x_sand + (player->x_speed / world->grain_size) * dt;
	auto sand_query = SandPit_QueryRegion(world, (projected_player_x_sand - player_w_sand / 2), (player_y_sand + player_h_sand), player_w_sand, player_h_sand);

	if (sand_query.any_solid) {		// Did we collide with a wall?
		// Collide with wall
		SetPlayerXVelocity(player, 0.0);
	}
	else if (sand_query.any_grain) {		// Did we intersect with a piece of sand?
		// Can we autowalk over it?
		auto overhead_sand_difference = sand_query.highest_grain_y - player_y_sand;
		if (overhead_sand_difference < PLAYER_AUTO_JUMP_HEIGHT) {
			player_y_sand = sand_query.highest_grain_y;
			player_x_sand = projected_player_x_sand;
		}
		else {
			// Collide with wall
			SetPlayerXVelocity(player, 0.0);
		}
	}
	else {
		player_x_sand = projected_player_x_sand;
	}

	auto is_falling = player_y_sand > 0 && (sand_query.grain_count + sand_query.solid_count == 0);
	if (is_falling)
		IncrementPlayerYVelocity(player, -G * dt);
	else if (jump) {
		SetPlayerYVelocity(player, PLAYER_JUMP_SPEED);
		IncrementPlayerXVelocity(player, (player->x_speed > 0) ? PLAYER_JUMP_SPEED_BOOST_X * dt : -PLAYER_JUMP_SPEED_BOOST_X * dt);
		// If our framerate is too high we will not move far enough to leave the ground,
		// so we give the player a little extra nudge.
		player_y_sand += PLAYER_JUMP_EPSILON * world->grain_size;
	}

	player_y_sand += (player->y_speed / world->grain_size) * dt;

	// Snap y coordinate if we are intersecting with sand 
	auto sand_query_top = SandPit_QueryRegion(world, (player_x_sand - player_w_sand / 2), (player_y_sand + player_h_sand + 2), player_w_sand, player_h_sand );
	auto sand_query_bottom = SandPit_QueryRegion(world, (player_x_sand - player_w_sand / 2), player_y_sand, player_w_sand, 1);

	// Hitting our head
	if (sand_query_top.any_solid) {
		SetPlayerYVelocity(player, 0.0);
		player_y_sand = std::min(player_y_sand - 2, (sand_query_top.lowest_solid_y - player_h_sand));
	} else if (!jump && (sand_query_bottom.any_grain || sand_query_bottom.any_solid)) {	
		SetPlayerYVelocity(player, 0.0);
		player_y_sand = std::max(sand_query_bottom.highest_grain_y, sand_query_bottom.highest_solid_y) + 1;
	}

	player_y_sand = std::max(player_y_sand, (double)(LOWEST_Y_COORDINATE / world->grain_size));				// Prevent from falling under the map 
	if (player_x_sand - player_w_sand / 2.0 <= 0) {
		player_x_sand = player_w_sand / 2.0;
		SetPlayerXVelocity(player, 0.0);
	}
	if (player_x_sand + player_w_sand / 2.0 >= world->w) {
		player_x_sand = world->w - player_w_sand / 2.0;
		SetPlayerXVelocity(player, 0.0);
	}

	auto dx = player_x_sand - prev_player_x_sand;
	auto dy = player_y_sand - prev_player_y_sand;
	AABB_MoveBy(&player->bbox, dx * world->grain_size, dy * world->grain_size);
}

void DoPlayerFireSand(Player* player, SandPit* world, bool fire_held, bool fire_press, bool switch_fire_mode, int mouse_x, int mouse_y) {
	if (switch_fire_mode) {
		player->fire_mode = NextFireType(player->fire_mode);
	}
	if (!fire_held && !fire_press)
		return;

	mouse_x /= world->grain_size;
	mouse_y /= world->grain_size;
	
	auto player_bbox_sand_coords = player->bbox;
	AABB_ScaleByReciprocal(&player_bbox_sand_coords, world->grain_size);
	auto player_w_sand = AABB_GetWidth(&player_bbox_sand_coords);
	auto player_h_sand = AABB_GetHeight(&player_bbox_sand_coords);

	double player_x_sand = 0;
	double player_y_sand = 0;
	AABB_GetCornerCoords(&player_bbox_sand_coords, AABB_BOTTOM, &player_x_sand, &player_y_sand);

	bool lhs = mouse_x < (player_x_sand);
	auto sand_x = (lhs) ? (player_x_sand - player_w_sand) : (player_x_sand + player_w_sand);
	auto sand_y = (player_y_sand + 4);
	auto sight_vec_x = (double)mouse_x - (double)player_x_sand;
	auto sight_vec_y = (double)mouse_y - (double)player_y_sand;

	if (sight_vec_y > 0 && sight_vec_x <= 0 && sight_vec_x > -80) {
		sight_vec_x = -80;
	}
	else if (sight_vec_y > 0 && sight_vec_x >= 0 && sight_vec_x < 80) {
		sight_vec_x = 80;
	}
	auto sight_vec_mag = sqrt(sight_vec_x * sight_vec_x + sight_vec_y * sight_vec_y);
	auto unit_x = sight_vec_x / sight_vec_mag;
	auto unit_y = sight_vec_y / sight_vec_mag;
	if (player->fire_mode == FIRE_TYPE_STREAM && fire_held) {
		PlaceSandCircle(world, sand_x, sand_y, 1);
		BlowSand(
			world,
			sand_x,
			sand_y,
			2,
			unit_x * 8,
			unit_y * 8
		);
	}
	else if (player->fire_mode == FIRE_TYPE_BURST && fire_press) {
		auto x_offset = (lhs) ? -7 : 7;
		PlaceSandCircle(world, (sand_x + x_offset), sand_y, 6);
		BlowSand(
			world,
			(sand_x + x_offset),
			sand_y,
			8,
			unit_x * 12,
			unit_y * 12 
		);
	}
}

void Player_Create(Player* player, double player_x, double player_y, double player_w, double player_h) {
	AABB_Create(&player->bbox, player_x, player_y, player_w, player_h);
	player->x_speed = 0.0;
	player->y_speed = 0.0;
	player->do_jump = false;
	player->fire_mode = FIRE_TYPE_STREAM;
}

void Player_UpdatePlayer(
	Player* player, 
	SandPit* world, 
	bool left, 
	bool right, 
	bool jump, 
	bool fire_held, 
	bool fire_press, 
	bool switch_fire_mode,
	int mouse_x,
	int mouse_y,
	float dt
) {
	DoPlayerMovement(player, world, left, right, jump, dt);
	DoPlayerFireSand(player, world, fire_held, fire_press, switch_fire_mode, mouse_x, mouse_y);
}
