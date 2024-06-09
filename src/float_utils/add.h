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
		// Record any 1 bits that have been truncated from y during the shift
		std::uint32_t truncated_bits = yfshiftr_bits == 0 ? 0 : (yf << (32 - yfshiftr_bits));
		std::uint32_t yfv_pos = yf >> yfshiftr_bits;
		// In the case that y is subtracted from x, increment y's fraction and negate the truncated the bits so that
		// we always round towards the positive direction. This simplifies rounding by a lot
		if (truncated_bits && xp != yp) {
			++yfv_pos;
			truncated_bits = ~truncated_bits + 1u;
		}

		// Resulting fraction, guaranteed to be larger than 0 due to the swap
		// Negate y's fraction if the signs are different
		const std::uint32_t rf_raw = xp == yp ? xf + yfv_pos : xf - yfv_pos;
		if (rf_raw == 0) {
			return 0.0f;
		}

		const std::uint32_t re_offset = std::countl_zero(rf_raw);
		if (re_offset < 32 - (float_parts::num_fraction_bits + 1)) {
			// In this case, we have produced extra bits. Merge them into the truncated bits
			truncated_bits =
				(rf_raw << (re_offset + float_parts::num_fraction_bits + 1)) |
				(truncated_bits >> (32 - (re_offset + float_parts::num_fraction_bits + 1)));
		}

		const bool rp = xp;
		const std::uint32_t re = xe + (30 - re_offset - float_parts::num_fraction_bits);
		const std::uint32_t rf = (rf_raw << re_offset) >> (31u - float_parts::num_fraction_bits);
		const bool is_inf = (re >= (1u << float_parts::num_exponent_bits) - 1);

		// Round and return
		const rounding_mode rounding = Rounding == rounding_mode::system ? get_system_rounding_mode() : Rounding;
		return round_result(rounding, rp, re, rf, truncated_bits, is_inf);
	}
	template <rounding_mode RoundingMode = rounding_mode::system> inline float sub(float x, float y) {
		return add<RoundingMode>(x, -y);
	}
}
