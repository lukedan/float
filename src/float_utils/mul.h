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
		const auto rf = static_cast<std::uint32_t>(rf_raw >> rf_shiftr);
		const auto truncated_bits = static_cast<std::uint32_t>(rf_raw << (32 - rf_shiftr));

		const bool rp = xp != yp;
		const std::int32_t re_raw =
			xe + ye + (rf_extra_bit ? 1 : 0) + static_cast<std::int32_t>(float_parts::exponent_offset);
		if (re_raw <= 0) {
			return 0.0f;
		}
		const bool is_inf = re_raw >= (1u << float_parts::num_exponent_bits) - 1;
		const std::uint32_t re = std::min(
			static_cast<std::uint32_t>(re_raw),
			(1u << float_parts::num_exponent_bits) - 1
		);

		// Round and return
		const rounding_mode rounding = Rounding == rounding_mode::system ? get_system_rounding_mode() : Rounding;
		return round_result(rounding, rp, re, rf, truncated_bits, is_inf);
	}
}
