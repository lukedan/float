#pragma once

#include <iostream>
#include <functional>
#include <random>
#include <string_view>

#include "float_utils/utils.h"

void fuzz_binary_float_operator(
	std::function<float(float, float)> sys_ver,
	std::function<float(float, float)> my_ver,
	std::string_view test_name
) {
	auto test = [&](float x, float y) -> bool {
		const float hw_res = sys_ver(x, y);
		const float my_res = my_ver(x, y);

		const auto hw_bin = std::bit_cast<std::uint32_t>(hw_res);
		const auto my_bin = std::bit_cast<std::uint32_t>(my_res);
		if (hw_bin == my_bin) {
			return true;
		}

		// Filter out denorm
		const std::uint32_t he = float_parts::get_exponent(hw_res);
		if (he == 0) {
			return true;
		}

		std::cout <<
			"Hardware " << test_name << ": " << std::hex << hw_bin << std::dec << "  " << std::hexfloat << hw_res << "\n" <<
			"      My " << test_name << ": " << std::hex << my_bin << std::dec << "  " << std::hexfloat << my_res << "\n";

		return false;
	};

	std::default_random_engine rng(12345);
	for (std::uint32_t i = 1; ; ++i) {
		const float x = float_utils::random_float(rng);
		const float y = float_utils::random_float(rng);

		if (!test(x, y)) {
			std::cout <<
				"Iter " << i << ": " << x << " + " << y << "\n" <<
				"----------\n";
		}

		if (i % 100000000 == 0) {
			std::cout <<
				"Iter " << i << "\n" <<
				"----------\n";
		}
	}
}
