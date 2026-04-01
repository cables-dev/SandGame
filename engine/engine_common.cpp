#include "engine_common.hpp"
#include <cstdlib>
#include <cassert>

constexpr auto SECONDS_IN_HOURS = 60.0 * 60.0;
constexpr auto SECONDS_IN_MINUTES = 60.0;
constexpr auto MILLISECONDS_IN_SECONDS = 1000.0;

void EngineTime_Readout(const EngineTime* time, int* out_hours, int* out_minutes, int* out_seconds, int* out_milliseconds) {
	auto raw_seconds = time->seconds;
	auto remaining_minutes_plus_seconds = (int)(fmod(raw_seconds, SECONDS_IN_HOURS));
	auto remaining_seconds = (int)(fmod(raw_seconds, SECONDS_IN_MINUTES));
	if (out_hours)
		*out_hours = (int)(time->seconds / SECONDS_IN_HOURS);
	if (out_minutes) {
		*out_minutes = remaining_minutes_plus_seconds - remaining_seconds;
	}
	if (out_seconds) {
		*out_seconds = remaining_seconds;
	} 
	if (out_milliseconds) {
		*out_milliseconds = (raw_seconds - floor(raw_seconds)) * MILLISECONDS_IN_SECONDS;
	}
}

float EngineTime_GetTotalHours(const EngineTime* time) {
	return time->seconds / SECONDS_IN_HOURS;
}

float EngineTime_GetTotalMinutes(const EngineTime* time) {
	return time->seconds / SECONDS_IN_MINUTES;
}

float EngineTime_GetTotalSeconds(const EngineTime* time) {
	return time->seconds;
}

float EngineTime_GetTotalMilliseconds(const EngineTime* time) {
	return time->seconds * MILLISECONDS_IN_SECONDS;
}

EngineTime Engine_GetFrameTime() {
	return EngineTime{ GetFrameTime() };
}
 
void AABB_Create(AABB* aabb, double x, double y, double w, double h) {
	aabb->top_left_x = x;
	aabb->top_left_y = y;
	aabb->half_w = w / 2.0;
	aabb->half_h = h / 2.0;
}

void AABB_MoveBy(AABB* aabb, double dx, double dy) {
	aabb->top_left_x += dx;
	aabb->top_left_y += dy;
}

void AABB_GetCornerCoords(const AABB* aabb, AABBCorner corner, double* out_x, double* out_y) {
	if (!out_x || !out_y) return;

	switch (corner) {
	case AABB_TOP_LEFT: { *out_x = aabb->top_left_x; *out_y = aabb->top_left_y; break; }
	case AABB_TOP: { *out_x = aabb->top_left_x + aabb->half_w; *out_y = aabb->top_left_y; break; }
	case AABB_TOP_RIGHT: { *out_x = aabb->top_left_x + 2.0 * aabb->half_w; *out_y = aabb->top_left_y; break; }
	case AABB_MIDDLE_LEFT: { *out_x = aabb->top_left_x; *out_y = aabb->top_left_y - aabb->half_h; break; }
	case AABB_MIDDLE: { *out_x = aabb->top_left_x + aabb->half_w; *out_y = aabb->top_left_y - aabb->half_h; break; }
	case AABB_MIDDLE_RIGHT: { *out_x = aabb->top_left_x + 2.0 * aabb->half_w; *out_y = aabb->top_left_y - aabb->half_h; break; }
	case AABB_BOTTOM_LEFT: { *out_x = aabb->top_left_x; *out_y = aabb->top_left_y - 2.0 * aabb->half_h; break; }
	case AABB_BOTTOM: { *out_x = aabb->top_left_x + aabb->half_w; *out_y = aabb->top_left_y - 2.0 * aabb->half_h; break; }
	case AABB_BOTTOM_RIGHT: { *out_x = aabb->top_left_x + 2.0 * aabb->half_w; *out_y = aabb->top_left_y - 2.0 * aabb->half_h; break; }
	default: assert("AABB_GetCornerCoords: Unaccounted AABB corner case." && false); // Error
	}
}

void AABB_ScaleByReciprocal(AABB* aabb, double x) {
	aabb->top_left_x /= x;
	aabb->top_left_y /= x;
	aabb->half_w /= x;
	aabb->half_h /= x;
}

bool AABB_Intersects(const AABB* aabb1, const AABB* aabb2) {
    auto left1 = aabb1->top_left_x;
    auto right1 = left1 + (2.0 * aabb1->half_w);
    auto left2 = aabb2->top_left_x;
    auto right2 = left2 + (2.0 * aabb2->half_w);
    auto top1 = aabb1->top_left_y;
    auto bottom1 = top1 - (2.0 * aabb1->half_h);
    auto top2 = aabb2->top_left_y;
    auto bottom2 = top2 - (2.0 * aabb2->half_h);

    bool collision_x = (left1 < right2) && (left2 < right1);
    bool collision_y = (bottom1 < top2) && (bottom2 < top1);

    return collision_x && collision_y;
}

bool AABB_ContainsPoint(const AABB* aabb, double x, double y) {
	auto w = AABB_GetWidth(aabb);
	auto h = AABB_GetHeight(aabb);
	auto domain = x <= aabb->top_left_x + w && x >= aabb->top_left_x;
	auto range = y <= aabb->top_left_y + h && x >= aabb->top_left_y;
	return domain && range;
}

double AABB_GetWidth(const AABB* aabb) {
	return 2.0 * aabb->half_w;
}

double AABB_GetHeight(const AABB* aabb) {
	return 2.0 * aabb->half_h;
}

// Streets are saying this is faster than rand...
static std::uint32_t random_seed = 0xFAFFAC;

void PseudoRandom_Seed(std::uint32_t seed) {
	random_seed = seed;
}

std::uint32_t PseudoRandom_GetU32() {
	random_seed ^= random_seed << 13;
    random_seed ^= random_seed >> 17;
    random_seed ^= random_seed << 5;
    return random_seed;
}
