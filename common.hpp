#pragma once
#include <cstdint>

constexpr std::uint32_t kSandS{ 3 };

template <typename T>
T Bounds(const T& v, T max, T min) {
	if (v > max)
		return max;
	else if (v < min)
		return min;
	return v;
}


