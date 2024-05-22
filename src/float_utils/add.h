#pragma once

#include <cstdint>
#include <utility>

#include "utils.h"
#include "float_parts.h"

namespace float_utils {
	template <rounding_mode Rounding = rounding_mode::system> inline float add(float x, float y) {
		std::uint32_t xe = float_parts::get_exponent(x);
		std::uint32_t ye = float_parts::get_exponent(y);

		if (xe < ye) {
			std::swap(xe, ye);
			std::swap(x, y);
		}
		if (ye == 0) {
			return x;
		}

		const bool xp = float_parts::get_sign(x);
		const bool yp = float_parts::get_sign(y);
		const std::uint32_t xf = float_parts::get_fraction(x) | (1u << float_parts::num_fraction_bits);
		const std::uint32_t yf = float_parts::get_fraction(y) | (1u << float_parts::num_fraction_bits);

		constexpr std::uint32_t base_offset = 29u - float_parts::num_fraction_bits;

		const std::uint32_t xfshift_unsigned = xf << base_offset;
		const std::uint32_t xfshift = xp ? ~xfshift_unsigned + 1u : xfshift_unsigned;

		const int yfshiftl_bits = std::max(static_cast<int>(base_offset) - static_cast<int>(xe - ye), -31);
		const std::uint32_t yfshift_unsigned = yfshiftl_bits > 0 ? (yf << yfshiftl_bits) : (yf >> -yfshiftl_bits);
		const std::uint32_t yfshift = yp ? ~yfshift_unsigned + 1u : yfshift_unsigned;

		const std::uint32_t rf_signed = xfshift + yfshift;
		const bool rp = rf_signed & 0x80000000u;
		const std::uint32_t rf_unsigned = rp ? ~rf_signed + 1u : rf_signed;
		if (rf_unsigned == 0) {
			return 0.0f;
		}

		const std::uint32_t re_offset = std::countl_zero(rf_unsigned);
		const std::uint32_t rf_align = rf_unsigned << re_offset;

		const rounding_mode rounding = Rounding == rounding_mode::system ? get_system_rounding_mode() : Rounding;
		bool rounding_inc = false;
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
			rounding_inc = false;
			break;
		}

		const std::uint32_t rf = rf_align >> (31u - float_parts::num_fraction_bits);
		const std::uint32_t re = xe + 2 - re_offset;
		const float r_unrounded = float_parts::assemble(rp, re, rf);
		const auto rbin = std::bit_cast<std::uint32_t>(r_unrounded) + (rounding_inc ? 1u : 0u);
		return std::bit_cast<float>(rbin);
	}
	template <rounding_mode RoundingMode = rounding_mode::system> inline float sub(float x, float y) {
		return add<RoundingMode>(x, -y);
	}
}
