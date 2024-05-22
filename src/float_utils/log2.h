#pragma once

#include <cmath>
#include <limits>

#include "float_parts.h"
#include "utils.h"

namespace float_utils {
	template <std::uint32_t NewtonIterations = 0> inline float log2(float x) {
		if (x < 0.0f) {
			return std::numeric_limits<float>::quiet_NaN();
		}
		if (x == 0.0f) {
			return -std::numeric_limits<float>::infinity();
		}

		const std::int32_t exponent = float_parts::get_offset_exponent(x);
		const std::uint32_t fraction = float_parts::get_fraction(x);

		const float z = static_cast<float>(fraction) / (1u << float_parts::num_fraction_bits);
#define FLOAT_UTILS_LOG2_CHEBYSHEV
/*#define FLOAT_UTILS_LOG2_CUBIC*/
#if defined(FLOAT_UTILS_LOG2_CHEBYSHEV)
		// Chebyshev approximation of log2(x) on [0, 1]: y = 0.54311 + 0.49505 T(1, w) - 0.042469 T(2, w) + 0.0048576 T(3, w) - 0.00062481 T(4, w), w = x * 2 - 3
		const float y = fmaf(fmaf(fmaf(fmaf(-0.0799757f, z, 0.315395f), z, -0.672886f), z, 1.43728f), z, 0.00010859f);
#elif defined(FLOAT_UTILS_LOG2_CUBIC)
		// Cubic approximation of log2(x) on [0, 1]: y = 0.153912 x^3 - 1.02949 x^2 + 3.01072 x - 2.13381
		//                                             = 0.153912 z^3 - 0.567749 z^2 + 1.41348 z + 0.00133498, z = x - 1
		const float y = fmaf(fmaf(fmaf(0.153912f, z, -0.567749f), z, 1.41348f), z, 0.00133498f);
#else
		// Quadratic approximation of log2(x) on [0, 1]: y = -0.33688 x^2 + 1.9949 x - 1.64899
		//                                                 = -0.33688 z^2 + 1.32114 z + 0.00903059, z = x - 1
		const float y = fmaf(fmaf(-0.33688f, z, 1.32114f), z, 0.00903059f);
#endif

		float result = y + exponent;

		if constexpr (NewtonIterations > 0) {
			for (std::uint32_t iter = 0; iter < NewtonIterations; ++iter) {
				result -= 1.0f - x / std::exp2f(result);
			}
		}

		return result;
	}
}
