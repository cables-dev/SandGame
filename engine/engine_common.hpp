#pragma once
#include "raylib.h"
#include <cstdint>

struct AABB {
	double top_left_x{};
	double top_left_y{};
	double half_w{};
	double half_h{};
};
enum AABBCorner {
	AABB_TOP_LEFT,
	AABB_TOP,
	AABB_TOP_RIGHT,
	AABB_MIDDLE_LEFT,
	AABB_MIDDLE,
	AABB_MIDDLE_RIGHT,
	AABB_BOTTOM_LEFT,
	AABB_BOTTOM,
	AABB_BOTTOM_RIGHT,
};
void AABB_Create(AABB* aabb, double top_left_x, double top_left_y, double w, double h);
void AABB_MoveBy(AABB* aabb, double dx, double dy);
void AABB_GetCornerCoords(const AABB* aabb, AABBCorner corner, double* out_x, double* out_y);
void AABB_ScaleByReciprocal(AABB* aabb, double x);
bool AABB_Intersects(const AABB* aabb1, const AABB* aabb2);
bool AABB_ContainsPoint(const AABB* aabb, double x, double y);
double AABB_GetWidth(const AABB* aabb);
double AABB_GetHeight(const AABB* aabb);

void PseudoRandom_Seed(std::uint32_t seed);
std::uint32_t PseudoRandom_GetU32();

template <typename T>
T Bounds(const T& v, T max, T min) {
	if (v > max)
		return max;
	else if (v < min)
		return min;
	return v;
}

struct GameColour {
	union {
		std::uint32_t rgba{};
		struct {
			std::uint8_t a;				// Back-to-front due to x86 endianness. Might lead to problemos...
			std::uint8_t b;
			std::uint8_t g;
			std::uint8_t r;
		};
	};
};

struct EngineTime {
	float seconds;
};

void EngineTime_Readout(const EngineTime* time, int* out_hours, int* out_minutes, int* out_seconds, int* out_milliseconds);			// This should probably be in game files since its too constrained to be an engine routine... oh well sauce
float EngineTime_GetTotalHours(const EngineTime* time);
float EngineTime_GetTotalMinutes(const EngineTime* time);
float EngineTime_GetTotalSeconds(const EngineTime* time);
float EngineTime_GetTotalMilliseconds(const EngineTime* time);
EngineTime Engine_GetFrameTime();