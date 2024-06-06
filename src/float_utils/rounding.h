#pragma once

#include <optional>

#include "float_parts.h"

namespace float_utils {
	float trunc(float x) {
		const std::int32_t xe = float_parts::get_offset_exponent(x);
		if (xe >= static_cast<std::int32_t>(float_parts::num_fraction_bits)) {
			return x;
		}
		if (xe < 0) {
			return std::bit_cast<float>(std::bit_cast<std::uint32_t>(x) & 0x80000000u);
		}

		const std::uint32_t num_trunc_bits = float_parts::num_fraction_bits - static_cast<std::uint32_t>(xe);
		return std::bit_cast<float>(std::bit_cast<std::uint32_t>(x) & ~((1u << num_trunc_bits) - 1));
	}

	float round(float x) {
		const std::int32_t xe = float_parts::get_offset_exponent(x);
		if (xe >= static_cast<std::int32_t>(float_parts::num_fraction_bits)) {
			return x;
		}
		if (xe < -1) {
			return std::bit_cast<float>(std::bit_cast<std::uint32_t>(x) & 0x80000000u);
		}
		if (xe == -1) {
			return x > 0.0f ? 1.0f : -1.0f;
		}

		const std::uint32_t xf = float_parts::get_fraction(x) | (1u << float_parts::num_fraction_bits);
		const std::uint32_t num_trunc_bits = float_parts::num_fraction_bits - static_cast<std::uint32_t>(xe);
		const std::uint32_t result_bits = std::bit_cast<std::uint32_t>(x) & ~((1u << num_trunc_bits) - 1u);
		const std::uint32_t trunc_bits = xf << (32 - num_trunc_bits);
		if (trunc_bits & 0x80000000u) {
			return std::bit_cast<float>(result_bits + (1u << num_trunc_bits));
		}
		return std::bit_cast<float>(result_bits);
	}

	float floor(float x) {
		const std::int32_t xe = float_parts::get_offset_exponent(x);
		if (xe >= static_cast<std::int32_t>(float_parts::num_fraction_bits)) {
			return x;
		}
		if (std::bit_cast<std::uint32_t>(x) == 0x80000000u) {
			return x;
		}
		const bool xp = float_parts::get_sign(x);
		if (xe < 0) {
			return xp ? -1.0f : 0.0f;
		}

		const std::uint32_t xf = float_parts::get_fraction(x) | (1u << float_parts::num_fraction_bits);
		const std::uint32_t num_trunc_bits = float_parts::num_fraction_bits - static_cast<std::uint32_t>(xe);
		const std::uint32_t result_bits = std::bit_cast<std::uint32_t>(x) & ~((1u << num_trunc_bits) - 1u);
		if (xp) {
			const std::uint32_t trunc_bits = xf << (32 - num_trunc_bits);
			if (trunc_bits) {
				return std::bit_cast<float>(result_bits + (1u << num_trunc_bits));
			}
		}
		return std::bit_cast<float>(result_bits);
	}

	float ceil(float x) {
		return -floor(-x);
	}
}
