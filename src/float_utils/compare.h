#pragma once

#include "float_parts.h"
#include "category.h"

namespace float_utils {
	namespace _details {
		[[nodiscard]] constexpr bool any_nan(float x, float y) {
			return float_utils::is_nan(x) || float_utils::is_nan(y);
		}

		[[nodiscard]] constexpr bool equal_to_no_nan(float x, float y) {
			const auto ix = std::bit_cast<std::uint32_t>(x);
			const auto iy = std::bit_cast<std::uint32_t>(y);
			if ((ix & ~float_parts::sign_mask) == 0 && (iy & ~float_parts::sign_mask) == 0) {
				return true; // +0 == -0
			}
			return ix == iy;
		}

		[[nodiscard]] constexpr bool greater_than_no_nan(float x, float y) {
			const auto absix = std::bit_cast<std::uint32_t>(x) & ~float_parts::sign_mask;
			const auto absiy = std::bit_cast<std::uint32_t>(y) & ~float_parts::sign_mask;
			if (absix == 0 && absiy == 0) {
				return false; // +0 == -0
			}
			const bool xs = float_parts::get_sign(x);
			const bool ys = float_parts::get_sign(y);
			if (xs != ys) {
				return ys; // x > y if x is positive and y is negative
			}
			return xs != (absix > absiy);
		}
	}

	[[nodiscard]] constexpr bool equal_to(float x, float y) {
		if (_details::any_nan(x, y)) {
			return false;
		}
		return _details::equal_to_no_nan(x, y);
	}
	[[nodiscard]] constexpr bool not_equal_to(float x, float y) {
		if (_details::any_nan(x, y)) {
			return false;
		}
		return !_details::equal_to_no_nan(x, y);
	}

	[[nodiscard]] constexpr bool greater_than(float x, float y) {
		if (_details::any_nan(x, y)) {
			return false;
		}
		return _details::greater_than_no_nan(x, y);
	}
}
