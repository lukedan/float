#pragma once

#include <bit>
#include <optional>

#include "float_parts.h"
#include "utils.h"

namespace float_utils {
	template <
		rounding_mode RoundingMode = rounding_mode::nearest_tie_to_even
	> constexpr float to_float(std::int32_t sx) {
		if (sx == 0) {
			return 0.0f;
		}
		const bool sign = sx < 0;
		const auto bx = std::bit_cast<std::uint32_t>(sx);
		// Since negating a negative number may overflow, we do it manually here
		const auto x = sign ? ~bx + 1 : bx;
		// Find the highest bit
		const std::uint32_t bits = 32u - std::countl_zero(x);
		// Compute exponent based on highest bit
		const std::uint32_t exponent = bits - 1 + float_parts::exponent_offset;

		std::uint32_t fraction;
		std::uint32_t round_inc = 0;
		if (bits <= float_parts::num_fraction_bits + 1u) {
			fraction = x << (float_parts::num_fraction_bits + 1u - bits);
		} else {
			const std::uint32_t shr_bits = bits - float_parts::num_fraction_bits - 1u;
			// Truncation means rounding towards zero
			fraction = x >> shr_bits;

			const rounding_mode rounding =
				RoundingMode == rounding_mode::system ? get_system_rounding_mode() : RoundingMode;
			switch (rounding) {
			case rounding_mode::downward:
				// Round negative numbers up
				if (sign) {
					if (x & ((1u << shr_bits) - 1u)) {
						round_inc = 1;
					}
				}
				break;
			case rounding_mode::upward:
				// Round positive numbers up
				if (!sign) {
					if (x & ((1u << shr_bits) - 1u)) {
						round_inc = 1;
					}
				}
				break;
			case rounding_mode::nearest_tie_to_even:
				{
					const std::uint32_t tie_bit = 1u << (shr_bits - 1u);
					if (x & tie_bit) {
						if ((x & (tie_bit - 1u)) == 0) {
							// A tie - round to nearest value with an even least significant digit
							round_inc = (x & (tie_bit << 1u)) == 0 ? 0u : 1u;
						} else {
							round_inc = 1;
						}
					}
				}
				break;
			case rounding_mode::nearest_tie_to_infinity:
				if (x & (1u << (shr_bits - 1u))) {
					round_inc = 1;
				}
				break;
			case rounding_mode::toward_zero:
				break;
			}
		}
		const std::uint32_t res_unsigned = float_parts::assemble_bits(false, exponent, fraction) + round_inc;
		return std::bit_cast<float>(sign ? float_parts::sign_mask | res_unsigned : res_unsigned);
	}

	constexpr std::optional<std::int32_t> to_int(float f) {
		const bool sign = float_parts::get_sign(f);
		const std::uint32_t offset_exponent = float_parts::get_exponent(f);
		if (offset_exponent < float_parts::exponent_offset) {
			// Value too small; truncate to 0
			return 0;
		}
		const std::uint32_t exponent = offset_exponent - float_parts::exponent_offset;
		if (exponent >= 32) { // Value too large
			return std::nullopt;
		}

		const std::uint32_t fraction = float_parts::get_fraction(f) | (1u << float_parts::num_fraction_bits);
		if (exponent <= float_parts::num_fraction_bits) {
			const std::uint32_t abs_value = fraction >> (float_parts::num_fraction_bits - exponent);
			return std::bit_cast<std::int32_t>(sign ? ~abs_value + 1 : abs_value);
		} else {
			const std::uint32_t abs_value = fraction << (exponent - float_parts::num_fraction_bits);
			// We need additional checks because signed integers have a smaller, asymmetric value range
			if (sign) {
				if (abs_value > 0x80000000u) { // Value too large
					return std::nullopt;
				}
				return std::bit_cast<std::int32_t>(~abs_value + 1u);
			} else {
				if (abs_value >= 0x80000000u) { // Value too large
					return std::nullopt;
				}
				return std::bit_cast<std::int32_t>(abs_value);
			}
		}
	}
}
