#pragma once

#include <cstdint>
#include <bit>

namespace float_parts {
	constexpr std::uint32_t num_fraction_bits = 23;
	constexpr std::uint32_t num_exponent_bits = 8;

	constexpr std::uint32_t fraction_mask = (1u << num_fraction_bits) - 1;
	constexpr std::uint32_t exponent_mask = ((1u << num_exponent_bits) - 1) << num_fraction_bits;

	constexpr std::uint32_t exponent_offset = 127;

	[[nodiscard]] constexpr inline bool get_sign(float x) {
		return std::bit_cast<std::uint32_t>(x) & (1u << 31);
	}
	[[nodiscard]] constexpr inline std::uint32_t get_exponent(float x) {
		return (std::bit_cast<std::uint32_t>(x) & exponent_mask) >> num_fraction_bits;
	}
	[[nodiscard]] constexpr inline std::uint32_t get_fraction(float x) {
		return std::bit_cast<std::uint32_t>(x) & fraction_mask;
	}

	[[nodiscard]] constexpr inline std::int32_t get_offset_exponent(float x) {
		return static_cast<std::int32_t>(get_exponent(x)) - exponent_offset;
	}

	[[nodiscard]] constexpr inline float assemble(bool sign, std::uint32_t exponent, std::uint32_t fraction) {
		const std::uint32_t repr =
			(sign ? 0x80000000u : 0u) |
			((exponent << num_fraction_bits) & exponent_mask) |
			(fraction & fraction_mask);
		return std::bit_cast<float>(repr);
	}
}
