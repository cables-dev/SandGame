#include "player.hpp"
#include "common.hpp"
#include <Math.h>
#include <utility>
#include <cstdlib>

void PlaceSandCircle(SandPit* p, std::int32_t x, std::int32_t y, std::int32_t r) {
	for (auto i_x = -r; i_x < r; i_x++) {
		for (auto i_y = -r; i_y < r; i_y++) {
			if (i_x * i_x + i_y * i_y < r * r) {
				SandPit_PlaceGrain(*p, x + i_x, y + i_y, rand() % 4);
			}
		}
	}
}

void VacuumSand(SandPit* p, std::int32_t x, std::int32_t y, std::int32_t r) {
	for (auto i_x = -r; i_x < r; i_x++) {
		for (auto i_y = -r; i_y < r; i_y++) {
			if (i_x * i_x + i_y * i_y < r * r) {
				SandPit_AddImpulse(*p, x + i_x, y + i_y, (i_x > 0) ? -1 : 1, (i_y > 0) ? -1 : 1);
			}
		}
	}
}

void BlowSand(SandPit* p, std::int32_t x, std::int32_t y, std::int32_t r, std::int32_t v_x, std::int32_t v_y) {
	for (auto i_x = -r; i_x < r; i_x++) {
		for (auto i_y = -r; i_y < r; i_y++) {
			if (i_x * i_x + i_y * i_y < r * r) {
				SandPit_AddImpulse(*p, x + i_x, y + i_y, v_x, v_y);
			}
		}
	}
}

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

	auto projected_player_x = player->x + player->x_speed * dt;
	auto sand_query = SandPit_QueryRegion(world, (projected_player_x - player->w / 2) / kSandS, (player->y + player->h) / kSandS, player->w / kSandS, player->h / kSandS);

	// Can we autowalk over this?
	if (sand_query.highest_grain_y != -1 && sand_query.highest_grain_y - (player->y / kSandS) < ((player->h * 0.7) / kSandS)) {
		player->x = projected_player_x;
		player->y = sand_query.highest_grain_y * kSandS;
	}
	else if (sand_query.highest_grain_y == -1) {
		player->x = projected_player_x;
	}
	else {
		SetPlayerXVelocity(player, 0.0);
	}

	auto is_falling = player->y > 0 && (sand_query.grain_count == 0);
	if (is_falling)
		IncrementPlayerYVelocity(player, -G * dt);
	else if (jump) {
		SetPlayerYVelocity(player, PLAYER_JUMP_SPEED);
		IncrementPlayerXVelocity(player, (player->x_speed > 0) ? PLAYER_JUMP_SPEED_BOOST_X * dt: -PLAYER_JUMP_SPEED_BOOST_X * dt);
	}

	player->y += player->y_speed * dt;
	player->y = std::max(player->y, (double)LOWEST_Y_COORDINATE);				// Prevent from falling under the map 

	// Snap y coordinate if we are intersecting with sand 
	auto sand_query_top = SandPit_QueryRegion(world, (player->x - player->w / 2) / kSandS, (player->y + player->h) / kSandS, player->w / kSandS, 2);
	auto sand_query_bottom = SandPit_QueryRegion(world, (player->x - player->w / 2) / kSandS, (player->y) / kSandS, player->w / kSandS, 1);
	if (!jump && sand_query_bottom.highest_grain_y != -1 && sand_query_top.highest_grain_y == -1) {
		SetPlayerYVelocity(player, 0.0);
		player->y = sand_query_bottom.highest_grain_y * kSandS + kSandS;
	}
}

void DoPlayerFireSand(Player* player, SandPit* world, bool fire_held, bool fire_press, bool switch_fire_mode, int mouse_x, int mouse_y) {
	if (switch_fire_mode) {
		player->fire_mode = NextFireType(player->fire_mode);
	}

	bool lhs = mouse_x < (player->x);
	auto sand_x = (lhs) ? (player->x - player->w) : (player->x + player->w);
	auto sand_y = (player->y + kSandS * 4);
	auto sight_vec_x = (double)mouse_x - (double)player->x;
	auto sight_vec_y = (double)mouse_y - (double)player->y;

	if (sight_vec_y > 0 && sight_vec_x <= 0 && sight_vec_x > -200) {
		sight_vec_x = -200;
	}
	else if (sight_vec_y > 0 && sight_vec_x >= 0 && sight_vec_x < 200) {
		sight_vec_x = 200;
	}
	auto sight_vec_mag = sqrt(sight_vec_x * sight_vec_x + sight_vec_y * sight_vec_y);
	auto unit_x = sight_vec_x / sight_vec_mag;
	auto unit_y = sight_vec_y / sight_vec_mag;
	if (player->fire_mode == FIRE_TYPE_STREAM && fire_held) {
		PlaceSandCircle(world, sand_x / kSandS, sand_y / kSandS, 1);
		BlowSand(
			world,
			sand_x / kSandS,
			sand_y / kSandS,
			2,
			unit_x * 8,
			unit_y * 8
		);
	}
	else if (player->fire_mode == FIRE_TYPE_BURST && fire_press) {
		auto x_offset = (lhs) ? -(int)(kSandS * 7) : (int)kSandS * 7;
		PlaceSandCircle(world, (sand_x + x_offset) / kSandS, sand_y / kSandS, 6);
		BlowSand(
			world,
			(sand_x + x_offset) / kSandS,
			(sand_y + 2 * kSandS)/ kSandS,
			8,
			unit_x * 12,
			unit_y * 12 
		);
	}
}

void UpdatePlayer(
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
