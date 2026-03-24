#pragma once
#include <cstdint>

constexpr std::uint32_t kSandS{ 3 };
constexpr std::uint32_t LOWEST_Y_COORDINATE{ 0 };

template <typename T>
T Bounds(const T& v, T max, T min) {
	if (v > max)
		return max;
	else if (v < min)
		return min;
	return v;
}


