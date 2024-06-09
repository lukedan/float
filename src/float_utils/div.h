#pragma once

#include <algorithm>
#include <bit>

#include "float_parts.h"
#include "utils.h"

namespace float_utils {
	template <rounding_mode Rounding> float div(float x, float y) {
		const auto xfrac = static_cast<std::uint64_t>(
			float_parts::get_fraction(x) | (1u << float_parts::num_fraction_bits)
		);
		const auto yfrac = static_cast<std::uint64_t>(
			float_parts::get_fraction(y) | (1u << float_parts::num_fraction_bits)
		);
		const std::int32_t xe = float_parts::get_offset_exponent(x);
		const std::int32_t ye = float_parts::get_offset_exponent(y);

		const std::uint64_t xfrac_align = xfrac << (64 - (float_parts::num_fraction_bits + 1));
		const std::uint64_t rfrac_raw = xfrac_align / yfrac;
		const std::uint64_t rrem = xfrac_align - rfrac_raw * yfrac;

		const std::uint32_t rfzeros = std::countl_zero(rfrac_raw);
		const std::uint32_t rfshiftr_bits = 64 - (float_parts::num_fraction_bits + 1) - rfzeros;
		// Set the lowest bit to 1 to indicate if there's a remainder
		const auto truncated_bits = static_cast<std::uint32_t>((rfrac_raw << (32u - rfshiftr_bits)) | (rrem > 0 ? 1 : 0));

		const std::int32_t re_raw =
			(xe - ye + float_parts::num_fraction_bits - rfzeros) +
			static_cast<std::int32_t>(float_parts::exponent_offset);
		const bool is_inf = re_raw >= (1 << float_parts::num_exponent_bits) - 1;

		const auto re = static_cast<std::uint32_t>(std::clamp<std::int32_t>(
			re_raw, 0, (1 << float_parts::num_exponent_bits) - 1
		));
		auto rf = static_cast<std::uint32_t>(rfrac_raw >> rfshiftr_bits);
		const bool rp = float_parts::get_sign(x) != float_parts::get_sign(y);

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
		if (is_inf) {
			rf = 0;
			rounding_inc = 0;
		}
		return std::bit_cast<float>(float_parts::assemble_bits(rp, re, rf) + rounding_inc);
	}
}
