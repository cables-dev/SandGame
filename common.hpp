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
constexpr float BARREL_EXPLOSION_DELAY_S{ 0.5 };

using RenderFXFlags = std::uint32_t;
using SoundFXFlags = std::uint32_t;
constexpr RenderFXFlags NULL_FX_FLAGS = 0;
constexpr SoundFXFlags NULL_SFX_FLAGS = 0;

enum RenderFX {
	FX_REFRESH_TOAST = 1 << 0,
	FX_WHITE_FLASH = 1 << 1,
	FX_MAX = 1 << 2// Remeber to update me!
};
enum AudioResource : std::uint32_t {			// Index into AudioData.sounds
	AUDIO_RSC_TOAST_NOTIFY,
	AUDIO_RSC_JUMP,
	AUDIO_RSC_MAX
};
enum SoundFX {
	SFX_TOAST_NOTIFY = 1 << AUDIO_RSC_TOAST_NOTIFY,
	SFX_JUMP = 1 << AUDIO_RSC_JUMP,
	SFX_MAX = 1 << 2// Remeber to update me!
};
enum GraphicResource : std::uint32_t {
	GRAPHIC_RSC_BARREL_CIGARETTE_IDLE,			// Index into RenderData.graphics
	GRAPHIC_RSC_BARREL_CIGARETTE_EXPLODE,		
	GRAPHIC_RSC_BARREL_KURT_IDLE,		
	GRAPHIC_RSC_BARREL_KURT_EXPLODE,
	GRAPHIC_RSC_BARREL_TERRORIST_IDLE,		
	GRAPHIC_RSC_BARREL_TERRORIST_EXPLODE,
	GRAPHIC_RSC_MAX,
};

bool RenderFXFlags_Get(RenderFXFlags flags, RenderFX flag);
void RenderFXFlags_Set(RenderFXFlags* flags, RenderFX flag);
bool SoundFXFlags_Get(SoundFXFlags flags, SoundFX flag);
void SoundFXFlags_Set(SoundFXFlags* flags, SoundFX flag);



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
