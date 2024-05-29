#include <iostream>
#include <limits>

#include "float_utils/conversions.h"

int main() {
	// int to float conversion
	for (std::int32_t i = -std::numeric_limits<std::int32_t>::min(); ; ++i) {
		const float hw_f = static_cast<float>(i);
		const float my_f = float_utils::to_float(i);

		if (std::bit_cast<std::uint32_t>(hw_f) != std::bit_cast<std::uint32_t>(my_f)) {
			std::cout <<
				"Not equal: " << i << "\n" <<
				"Hardware conversion: " << hw_f << " " << std::bit_cast<std::uint32_t>(hw_f) << "\n" <<
				"      My conversion: " << my_f << " " << std::bit_cast<std::uint32_t>(my_f) << "\n" <<
				"----------\n";
		}

		if (i % 100000000 == 0) {
			std::cout << "int -> float: Tested " << i << "\n";
		}

		if (i == std::numeric_limits<std::int32_t>::max()) {
			break;
		}
	}

	std::cout << "\n----------\n\n";

	// float to int conversion
	for (std::uint32_t i = 0; ; ++i) { // Test all bit patterns
		const float fv = std::bit_cast<float>(i);
		const std::optional<std::int32_t> my_i = float_utils::to_int(fv);
		if (my_i) { // Avoid undefined behavior due to overflowing
			const auto hw_i = static_cast<std::int32_t>(fv);

			if (std::bit_cast<std::uint32_t>(hw_i) != std::bit_cast<std::uint32_t>(my_i.value())) {
				std::cout <<
					"Not equal: " << fv << "\n" <<
					"Hardware conversion: " << hw_i << "\n" <<
					"      My conversion: " << my_i.value() << "\n" <<
					"----------\n";
			}
		}

		if (i % 100000000 == 0) {
			std::cout << "float -> int: Tested " << i << "\n";
		}

		if (i == std::numeric_limits<std::uint32_t>::max()) {
			break;
		}
	}

	return 0;
}
