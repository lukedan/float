#include <iostream>

#include "float_utils/add.h"

constexpr auto rounding_mode = float_utils::rounding_mode::nearest_tie_to_even;

bool test(float x, float y) {
	const float hw_res = x + y;
	const float my_res = float_utils::add<rounding_mode>(x, y);

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
		"Hardware add: " << std::hex << hw_bin << std::dec << "  " << std::hexfloat << hw_res << "\n" <<
		"      My add: " << std::hex << my_bin << std::dec << "  " << std::hexfloat << my_res << "\n";

	return false;
}

int main() {
	std::fesetround(float_utils::to_fe_rounding_mode(rounding_mode));

	std::default_random_engine rng(12345);
	for (std::uint32_t i = 1; ; ++i) {
		const float x = float_utils::random_float(rng);
		const float y = float_utils::random_float(rng);

		if (!test(x, y)) {
			std::cout <<
				"Iter " << i << ": " << x << " + " << y << "\n" <<
				"----------\n";
		}

		if (i % 10000000 == 0) {
			std::cout <<
				"Iter " << i << "\n" <<
				"----------\n";
		}
	}

	return 0;
}
