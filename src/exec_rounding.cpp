#include <functional>
#include <iostream>

#include "float_utils/rounding.h"

void test_func(std::function<float(float)> sys_version, std::function<float(float)> my_version) {
	for (std::uint32_t i = 0; ; ++i) {
		const float x = std::bit_cast<float>(i);

		const float sys_v = sys_version(x);
		const float my_v = my_version(x);

		const bool bin_eq = std::bit_cast<std::uint32_t>(sys_v) == std::bit_cast<std::uint32_t>(my_v);
		const bool nan_eq = std::isnan(sys_v) == std::isnan(my_v);

		if (!nan_eq || (!std::isnan(sys_v) && !bin_eq)) {
			std::cout <<
				"Mismatch at " << x << ":\n" <<
				"System version: " << sys_v << "  " << std::bit_cast<std::uint32_t>(sys_v) << "\n"
				"    My version: " << my_v << "  " << std::bit_cast<std::uint32_t>(my_v) << "\n";
		}

		if (i % 1000000000 == 0) {
			std::cout << "Tested " << i << "\n";
		}

		if (i == std::numeric_limits<std::uint32_t>::max()) {
			break;
		}
	}
}

int main() {
	std::cout << "Testing trunc()\n";
	test_func(std::truncf, float_utils::trunc);
	std::cout << "----------\n";

	std::cout << "Testing round()\n";
	test_func(std::roundf, float_utils::round);
	std::cout << "----------\n";

	std::cout << "Testing floor()\n";
	test_func(std::floorf, float_utils::floor);
	std::cout << "----------\n";

	std::cout << "Testing ceil()\n";
	test_func(std::ceilf, float_utils::ceil);
	std::cout << "----------\n";

	return 0;
}
