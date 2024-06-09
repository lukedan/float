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
		const bool rp = float_parts::get_sign(x) != float_parts::get_sign(y);
		const auto re = static_cast<std::uint32_t>(std::clamp<std::int32_t>(
			re_raw, 0, (1 << float_parts::num_exponent_bits) - 1
		));
		const auto rf = static_cast<std::uint32_t>(rfrac_raw >> rfshiftr_bits);

		// Round and return
		const rounding_mode rounding = Rounding == rounding_mode::system ? get_system_rounding_mode() : Rounding;
		return round_result(rounding, rp, re, rf, truncated_bits, is_inf);
	}
}
