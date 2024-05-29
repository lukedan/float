#pragma once

#include <cstdint>
#include <utility>

#include "utils.h"
#include "float_parts.h"

namespace float_utils {
	template <rounding_mode Rounding = rounding_mode::system> inline float add(float x, float y) {
		std::uint32_t xe = float_parts::get_exponent(x);
		std::uint32_t ye = float_parts::get_exponent(y);

		// Shifted left one bit to ensure that we have all fraction bits
		// - If xe == ye, no valid digits will be generated after the last fraction bit
		// - if xe > ye, the position of the top bit will move right by at most 1
		std::uint32_t xf = (float_parts::get_fraction(x) << 1) | (2u << float_parts::num_fraction_bits);
		std::uint32_t yf = (float_parts::get_fraction(y) << 1) | (2u << float_parts::num_fraction_bits);

		// Swap if necessary to make sure that the absolute value of x is larger than that of y
		const bool swap_xy = xe == ye ? xf < yf : xe < ye;
		if (swap_xy) {
			std::swap(xe, ye);
			std::swap(xf, yf);
			std::swap(x, y);
		}
		if (ye == 0) {
			return x;
		}

		const bool xp = float_parts::get_sign(x);
		const bool yp = float_parts::get_sign(y);

		// y needs to be shifted right this many bits to align with x, clamped at 31
		const std::uint32_t yfshiftr_bits = std::min(xe - ye, 31u);
		// Whether any 1 bits have been truncated from y during the shift
		const bool yftrunc = yf & ((1u << yfshiftr_bits) - 1);
		const std::uint32_t yfv_pos = yf >> yfshiftr_bits;

		// Resulting fraction, guaranteed to be larger than 0 due to the swap
		// Negate y's fraction if the signs are different
		const std::uint32_t rf_raw = xp == yp ? xf + yfv_pos : xf - yfv_pos;
		if (rf_raw == 0) {
			return 0.0f;
		}

		const std::uint32_t re_offset = std::countl_zero(rf_raw);
		const std::uint32_t rf_align = rf_raw << re_offset;
		const bool rftrunc = rf_align & ((1u << (31 - float_parts::num_fraction_bits)) - 1u);
		const std::uint32_t rf = rf_align >> (31u - float_parts::num_fraction_bits);
		const std::uint32_t re = xe + (30 - re_offset - float_parts::num_fraction_bits);

		const bool is_inf = (re >= (1u << float_parts::num_exponent_bits) - 1);

		// Handle rounding
		const rounding_mode rounding = Rounding == rounding_mode::system ? get_system_rounding_mode() : Rounding;
		std::uint32_t rounding_inc = 0;
		switch (rounding) {
		case rounding_mode::downward:
			if (xp) { // x is negative
				// Round up - increment if there are truncated bits, either from the result or from y if it has the
				// same sign as x
				if (rftrunc || (yp && yftrunc)) {
					rounding_inc = 1;
				}
			} else { // !xp, x is positive
				// Effectively round towards zero
				if (yp && !rftrunc && yftrunc) {
					rounding_inc = -1;
				}
				if (is_inf) {
					return std::numeric_limits<float>::max();
				}
			}
			break;
		case rounding_mode::upward:
			// Same as rounding_mode::downward, but with signs flipped
			if (!xp) {
				if (rftrunc || (!yp && yftrunc)) {
					rounding_inc = 1;
				}
			} else { // xp
				if (!yp && !rftrunc && yftrunc) {
					rounding_inc = -1;
				}
				if (is_inf) {
					return -std::numeric_limits<float>::max();
				}
			}
			break;
		case rounding_mode::nearest_tie_to_even:
			// TODO
			rounding_inc = rf_align & (0x80000000u >> (float_parts::num_fraction_bits + 1));
			break;
		case rounding_mode::nearest_tie_to_infinity:
			// TODO
			break;
		case rounding_mode::toward_zero:
			// The sign does not matter - the only case where we need to further round down is when the number being
			// subtracted has truncated bits but the result doesn't
			rounding_inc = (xp != yp && !rftrunc && yftrunc) ? -1 : 0;
			// Truncate inf to maximum non-inf value
			if (is_inf) {
				constexpr float maxv = std::numeric_limits<float>::max();
				return xp ? -maxv : maxv;
			}
			break;
		}

		const std::uint32_t rbin = float_parts::assemble_bits(xp, re, rf) + rounding_inc;
		if (rounding != rounding_mode::toward_zero) {
			// Handle proper inf by zeroing the fraction
			if ((rbin & float_parts::exponent_mask) == float_parts::exponent_mask) {
				return std::bit_cast<float>(rbin & ~float_parts::fraction_mask);
			}
		}
		return std::bit_cast<float>(rbin);
	}
	template <rounding_mode RoundingMode = rounding_mode::system> inline float sub(float x, float y) {
		return add<RoundingMode>(x, -y);
	}
}
