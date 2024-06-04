#pragma once

#include "float_parts.h"

namespace float_utils {
	template <std::uint32_t NewtonIterations = 1> float rcp(float x) {
		// Constant used for linear approximation of 1/xf, best when using different number of Newton iterations
		constexpr std::uint32_t magic_0 = 0x7FFF81u;
		constexpr std::uint32_t magic_1 = 0x7FF4B0u;
		constexpr std::uint32_t magic_2 = 0x7CBC40u;
		constexpr std::uint32_t magic =
			NewtonIterations == 0 ? magic_0 :
			NewtonIterations == 1 ? magic_1 :
			magic_2;

#if 0
		// Verbose version
		const bool xp = float_parts::get_sign(x);
		const std::int32_t xe = float_parts::get_offset_exponent(x);
		const std::uint32_t xf = float_parts::get_fraction(x);

		const auto re = static_cast<std::uint32_t>(static_cast<std::int32_t>(float_parts::exponent_offset) - xe - 1);
		const auto rf = magic - xf;
		float result = float_parts::assemble(xp, re, rf);
#else
		constexpr std::uint32_t full_magic = ((float_parts::exponent_offset * 2 - 1) << float_parts::num_fraction_bits) + magic;
		float result = std::bit_cast<float>(full_magic - std::bit_cast<std::uint32_t>(x));
#endif

		for (std::uint32_t i = 0; i < NewtonIterations; ++i) {
			result = result * (2.0f - result * x);
		}

		return result;
	}
}
