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

		const std::uint32_t yfshiftr_bits = std::min(xe - ye, 31u);
		const bool yftrunc = yf & ((1u << yfshiftr_bits) - 1);
		const std::uint32_t yfv_unsigned = yf >> yfshiftr_bits;
		const std::uint32_t yfv = xp == yp ? yfv_unsigned : ~yfv_unsigned + 1u;

		const std::uint32_t rf_raw = xf + yfv;
		if (rf_raw == 0) {
			return 0.0f;
		}

		const std::uint32_t re_offset = std::countl_zero(rf_raw);
		const std::uint32_t rf_align = rf_raw << re_offset;
		const bool rftrunc = rf_align & ((1u << (31 - float_parts::num_fraction_bits)) - 1u);
		const std::uint32_t rf = rf_align >> (31u - float_parts::num_fraction_bits);
		const std::uint32_t re = xe + (30 - re_offset - float_parts::num_fraction_bits);

		const bool is_inf = (re >= (1u << float_parts::num_exponent_bits) - 1);

		const rounding_mode rounding = Rounding == rounding_mode::system ? get_system_rounding_mode() : Rounding;
		std::uint32_t rounding_inc = 0;
		switch (rounding) {
		case rounding_mode::downward:
			// TODO
			break;
		case rounding_mode::upward:
			// TODO
			break;
		case rounding_mode::nearest:
			// TODO: Does not match hardware results when the discarded bits are exactly 100... (e.g. 321.65 + 354.31)
			rounding_inc = rf_align & (0x80000000u >> (float_parts::num_fraction_bits + 1));
			break;
		case rounding_mode::toward_zero:
			rounding_inc = (xp != yp && !rftrunc && yftrunc) ? -1 : 0;
			if (is_inf) {
				constexpr float maxv = std::numeric_limits<float>::max();
				return xp ? -maxv : maxv;
			}
			break;
		}

		const std::uint32_t rbin = float_parts::assemble_bits(xp, re, rf) + rounding_inc;
		return std::bit_cast<float>(rbin);
	}
	template <rounding_mode RoundingMode = rounding_mode::system> inline float sub(float x, float y) {
		return add<RoundingMode>(x, -y);
	}
}
