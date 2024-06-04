#include <iostream>

#include "float_utils/rcp.h"

template <std::uint32_t NewtonIterations = 1> float rcp_custom(float x, std::uint32_t c) {
	const bool xp = float_parts::get_sign(x);
	const std::int32_t xe = float_parts::get_offset_exponent(x);
	const std::uint32_t xf = float_parts::get_fraction(x);

	const auto re = static_cast<std::uint32_t>(static_cast<std::int32_t>(float_parts::exponent_offset) - xe - 1);
	const auto rf = c - xf; // Linear approximation of 1/xf
	float result = float_parts::assemble(xp, re, rf);

	for (std::uint32_t i = 0; i < NewtonIterations; ++i) {
		result = result * (2.0f - result * x);
	}

	return result;
}

template <std::uint32_t NewtonIterations> void search_for_constant() {
	std::uint32_t best_c = 0xFFFFFFFFu;
	float min_max_diff = std::numeric_limits<float>::max();
	for (std::uint32_t c = 0; ; ++c) {
		float max_diff = 0.0f;
		for (std::uint32_t i = 0; i < (1u << float_parts::num_exponent_bits) - 1; ++i) {
			const float v = float_parts::assemble(false, float_parts::exponent_offset, i);
			const float hw_rcp = 1.0f / v;
			const float my_rcp = rcp_custom<NewtonIterations>(v, c);
			if (!std::isfinite(my_rcp)) {
				max_diff = std::numeric_limits<float>::max();
				break;
			} else {
				max_diff = std::max(max_diff, std::abs(hw_rcp - my_rcp));
			}
		}
		if (max_diff < min_max_diff) {
			min_max_diff = max_diff;
			best_c = c;
		}
		if (c % 10000000 == 0) {
			std::cout << "c = " << c << ",  best c: " << best_c << ",  value = " << min_max_diff << "\n";
		}
	}
	std::cout << "Best c: " << best_c << ",  value = " << min_max_diff << "\n";
}

template <std::uint32_t NewtonIterations> void test() {
	// Test all positive non-denorm finite floating point numbers
	float max_error = 0.0f;
	float max_error_val = 0.0f;
	for (std::uint32_t i = 0x00800000u; i <= 0x7F7FFFFFu; ++i) {
		if (i % 100000000 == 0) {
			std::cout << "Tested " << i << " (" << std::bit_cast<float>(i) << "),  max error: " << max_error * 100.0f << "% at " << max_error_val << "\n";
		}

		const float x = std::bit_cast<float>(i);
		const float hw_rcp = 1.0f / x;
		const float my_rcp = float_utils::rcp<NewtonIterations>(x);
		// Ignore denorms
		if (float_parts::get_exponent(hw_rcp) == 0) {
			continue;
		}
		// Report non-finite numbers
		if (!std::isfinite(my_rcp)) {
			std::cout << x << " produces non-finite numbers\n";
			continue;
		}

		const float diff = std::abs(hw_rcp - my_rcp);
		const float quotient = diff / hw_rcp;
		if (quotient > max_error) {
			max_error = quotient;
			max_error_val = x;
		}
	}
	std::cout << "Max error: " << max_error * 100.0f << "% at " << max_error_val <<"\n";
}

int main() {
	/*
	search_for_constant<0>();
	search_for_constant<1>();
	search_for_constant<2>();
	test<0>();
	test<1>();
	test<2>();
	*/

	while (true) {
		float x;
		std::cout << "x = ";
		std::cin >> x;
		std::cout <<
			"Hardware reciprocal: " << 1.0f / x << "\n" <<
			"      My reciprocal: " << float_utils::rcp<2>(x) << "\n";
	}

	return 0;
}
