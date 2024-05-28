#pragma once

#include "float_parts.h"

namespace float_utils {
	[[nodiscard]] constexpr bool is_inf(float x) {
		return
			(std::bit_cast<std::uint32_t>(x) & float_parts::exponent_mask) == float_parts::exponent_mask &&
			float_parts::get_fraction(x) == 0;
	}

	[[nodiscard]] constexpr bool is_nan(float x) {
		return
			(std::bit_cast<std::uint32_t>(x) & float_parts::exponent_mask) == float_parts::exponent_mask &&
			float_parts::get_fraction(x) != 0;
	}
}
