#pragma once

#include <cstdlib>
#include <cfenv>
#include <cmath>
#include <random>

#include "float_parts.h"

namespace float_utils {
	enum class rounding_mode {
		downward,
		upward,
		nearest,
		toward_zero,

		system
	};
	constexpr inline rounding_mode fe_rounding_to_rounding_mode(int rounding) {
		switch (rounding) {
		case FE_DOWNWARD:
			return rounding_mode::downward;
		case FE_UPWARD:
			return rounding_mode::upward;
		case FE_TONEAREST:
			return rounding_mode::nearest;
		case FE_TOWARDZERO:
			return rounding_mode::toward_zero;
		}
		std::abort();
		return rounding_mode::system;
	}
	inline rounding_mode get_system_rounding_mode() {
		return fe_rounding_to_rounding_mode(std::fegetround());
	}

	constexpr inline float fmaf(float a, float b, float c) {
#ifdef FP_FAST_FMAF
		return std::fmaf(a, b, c);
#else
		return a * b + c;
#endif
	}

	// Generates any valid floating point bit pattern with equal probability
	template <typename Rng> constexpr float random_float(Rng &rng) {
		using uniform_uint32 = std::uniform_int_distribution<std::uint32_t>;

		uniform_uint32 exponent_dist(1u, (1u << float_parts::num_exponent_bits) - 2u);
		uniform_uint32 fraction_dist(0u, float_parts::fraction_mask);
		uniform_uint32 sign_dist(0u, 1u);

		const std::uint32_t s = sign_dist(rng);
		const std::uint32_t e = exponent_dist(rng);
		const std::uint32_t f = fraction_dist(rng);

		return float_parts::assemble(s != 0, e, f);
	}
}
