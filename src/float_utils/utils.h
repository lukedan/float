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
		nearest_tie_to_even,
		nearest_tie_to_infinity,
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
			return rounding_mode::nearest_tie_to_even; // Default for IEEE 754
		case FE_TOWARDZERO:
			return rounding_mode::toward_zero;
		}
		std::abort();
		return rounding_mode::system;
	}
	constexpr inline int to_fe_rounding_mode(rounding_mode mode) {
		switch (mode) {
		case rounding_mode::downward:
			return FE_DOWNWARD;
		case rounding_mode::upward:
			return FE_UPWARD;
		case rounding_mode::nearest_tie_to_even:
			[[fallthrough]];
		case rounding_mode::nearest_tie_to_infinity:
			return FE_TONEAREST;
		case rounding_mode::toward_zero:
			return FE_TOWARDZERO;
		}
		return FE_TOWARDZERO;
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

	float round_result(
		rounding_mode rounding,
		bool rp, std::uint32_t re, std::uint32_t rf,
		std::uint32_t truncated_bits, bool is_inf
	) {
		std::uint32_t rounding_inc = 0;
		switch (rounding) {
		case rounding_mode::downward:
			if (rp) { // Result is negative
				// Round up - increment if there are truncated bits, either from the result or from y if it has the
				// same sign as x
				if (truncated_bits) {
					rounding_inc = 1;
				}
			} else { // !rp, result is positive
				// Effectively round towards zero
				if (is_inf) {
					return std::numeric_limits<float>::max();
				}
			}
			break;
		case rounding_mode::upward:
			// Same as rounding_mode::downward, but with signs flipped
			if (!rp) {
				if (truncated_bits) {
					rounding_inc = 1;
				}
			} else { // rp
				if (is_inf) {
					return -std::numeric_limits<float>::max();
				}
			}
			break;
		case rounding_mode::nearest_tie_to_even:
			[[fallthrough]];
		case rounding_mode::nearest_tie_to_infinity:
			{
				if (truncated_bits == 0x80000000u) {
					if (rounding == rounding_mode::nearest_tie_to_even) {
						rounding_inc = (rf & 1u) ? 1u : 0u;
					} else {
						rounding_inc = 1; // Not verified - no hardware implementation
					}
				} else {
					rounding_inc = (truncated_bits & 0x80000000u) ? 1 : 0;
				}
			}
			break;
		case rounding_mode::toward_zero:
			// Truncate inf to maximum non-inf value, but otherwise nothing to do
			if (is_inf) {
				constexpr float maxv = std::numeric_limits<float>::max();
				return rp ? -maxv : maxv;
			}
			break;
		}

		// Handle proper inf by zeroing the fraction
		if (is_inf) {
			rf = 0;
			rounding_inc = 0;
		}
		return std::bit_cast<float>(float_parts::assemble_bits(rp, re, rf) + rounding_inc);
	}
}
