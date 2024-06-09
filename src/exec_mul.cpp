#include <iostream>

#include "float_utils/mul.h"

#include "fuzz.h"

constexpr auto rounding_mode = float_utils::rounding_mode::nearest_tie_to_even;

int main() {
	std::fesetround(float_utils::to_fe_rounding_mode(rounding_mode));
	fuzz_binary_float_operator(
		[](float x, float y) { return x * y; },
		float_utils::mul<rounding_mode>,
		"mul"
	);
	return 0;
}
