#pragma once
#include <cstdint>

constexpr std::uint32_t SAND_SIZE{ 3 };
constexpr std::uint32_t LOWEST_Y_COORDINATE{ 0 };
constexpr std::uint32_t WINDOW_WIDTH{ 1800 };
constexpr std::uint32_t WINDOW_HEIGHT{ 900 };
constexpr std::uint32_t SAND_STUBBORNNESS{ 4 };
constexpr std::uint32_t PIT_WIDTH{ WINDOW_WIDTH / SAND_SIZE };
constexpr std::uint32_t PIT_HEIGHT{ WINDOW_HEIGHT / SAND_SIZE };
constexpr std::uint32_t NUM_SCREENS_HORIZONTAL{ 4 };				// Debug...
constexpr std::uint32_t NUM_SCREENS_VERTICAL{ 1 };


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
double AABB_GetWidth(const AABB* aabb);
double AABB_GetHeight(const AABB* aabb);
