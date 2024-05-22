#include <iostream>

#include "float_utils/add.h"

bool test(float x, float y) {
	const float hw_res = x + y;
	const float my_res = float_utils::add(x, y);

	const auto hw_bin = std::bit_cast<std::uint32_t>(hw_res);
	const auto my_bin = std::bit_cast<std::uint32_t>(my_res);
	if (hw_bin == my_bin) {
		return true;
	}

	// Filter out nan/inf/denorm
	const std::uint32_t he = float_parts::get_exponent(hw_res);
	if (he == float_parts::exponent_mask || he == 0) {
		return true;
	}

	std::cout <<
		"Hardware add: " << std::hex << hw_bin << std::dec << "  " << std::hexfloat << hw_res << "\n" <<
		"      My add: " << std::hex << my_bin << std::dec << "  " << std::hexfloat << my_res << "\n";

	return false;
}

int main() {
	/*std::fesetround(FE_TONEAREST);*/ // Does not quite match
	std::fesetround(FE_TOWARDZERO);

	/*
	for (float x, y; ; ) {
		std::cout << "x, y = ";
		std::cin >> x >> y;
		if (!std::cin) {
			break;
		}

		test(x, y);

		std::cout << "\n\n";
	}
	*/

	std::default_random_engine rng(12345);
	for (std::uint32_t i = 0; ; ++i) {
		const float x = float_utils::random_float(rng);
		const float y = float_utils::random_float(rng);

		if (!test(x, y)) {
			std::cout << "Iter " << i << ": " << x << " + " << y << "\n";
			std::getchar();
			std::cout << "----------\n";
		}

		if (i % 1000000 == 0) {
			std::cout <<
				"Iter " << i << "\n" <<
				"----------\n";
		}
	}

	return 0;
}
