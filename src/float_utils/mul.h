#pragma once

#include "float_parts.h"
#include "utils.h"

namespace float_utils {
	template <rounding_mode Rounding = rounding_mode::system> float mul(float x, float y) {
		const bool xp = float_parts::get_sign(x);
		const bool yp = float_parts::get_sign(y);

		const std::int32_t xe = float_parts::get_offset_exponent(x);
		const std::int32_t ye = float_parts::get_offset_exponent(y);

		const std::uint32_t xf = float_parts::get_fraction(x) | (1u << float_parts::num_fraction_bits);
		const std::uint32_t yf = float_parts::get_fraction(y) | (1u << float_parts::num_fraction_bits);

		const std::uint64_t rf_raw = static_cast<std::uint64_t>(xf) * static_cast<std::uint64_t>(yf);
		const bool rf_extra_bit = rf_raw & (1ULL << (2 * float_parts::num_fraction_bits + 1));

		const std::uint32_t rf_shiftr = float_parts::num_fraction_bits + (rf_extra_bit ? 1 : 0);
		auto rf = static_cast<std::uint32_t>(rf_raw >> rf_shiftr);
		const auto truncated_bits = static_cast<std::uint32_t>(rf_raw << (32 - rf_shiftr));

		const std::int32_t re_raw =
			xe + ye + (rf_extra_bit ? 1 : 0) + static_cast<std::int32_t>(float_parts::exponent_offset);
		if (re_raw <= 0) {
			return 0.0f;
		}
		bool is_inf = false;
		if (re_raw >= (1u << float_parts::num_exponent_bits) - 1) {
			is_inf = true;
		}
		const std::uint32_t re = std::min(
			static_cast<std::uint32_t>(re_raw),
			(1u << float_parts::num_exponent_bits) - 1
		);
		const bool rp = xp != yp;

		// Handle rounding
		const rounding_mode rounding = Rounding == rounding_mode::system ? get_system_rounding_mode() : Rounding;
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
		if (rounding != rounding_mode::toward_zero) {
			if (is_inf) {
				rf = 0;
				rounding_inc = 0;
			}
		}

		const std::uint32_t rbin = float_parts::assemble_bits(rp, re, rf) + rounding_inc;
		return std::bit_cast<float>(rbin);
	}
}
