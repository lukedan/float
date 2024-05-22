#include <iostream>

#include "float_utils/add.h"

bool test(float x, float y) {
	const float hw_res = x + y;
	const float my_res = float_utils::add(x, y);

	std::cout <<
		"Hardware add: " << hw_res << "\n" <<
		"My add: " << my_res << "\n";

	const auto hw_bin = std::bit_cast<std::uint32_t>(hw_res);
	const auto my_bin = std::bit_cast<std::uint32_t>(my_res);
	if (hw_bin == my_bin) {
		return true;
	}

	std::cout <<
		"BINARY DIFFERENT\n" <<
		"Hardware binary: " << std::ios::binary << hw_bin << "\n" <<
		"My binary: " << std::ios::binary << my_bin;

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
	while (true) {
		const float x = float_utils::random_float(rng);
		const float y = float_utils::random_float(rng);

		std::cout <<
			"------------------\n" <<
			x << " + " << y << "\n";

		if (!test(x, y)) {
			std::getchar();
		}
	}

	return 0;
}
