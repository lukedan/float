#include <iostream>

#include "float_utils/log2.h"

int main() {
	for (float x; ; ) {
		std::cout << "x = ";
		std::cin >> x;
		if (!std::cin) {
			break;
		}

		std::cout <<
			"STL log: " << std::log2f(x) << "\n" <<
			"Custom log: " << float_utils::log2(x) << "\n" <<
			"\n";
	}
	return 0;
}
