cmake_minimum_required(VERSION 3.5)
project(float_testbed)

set(COMMON_HEADERS
	"src/float_utils/add.h"
	"src/float_utils/category.h"
	"src/float_utils/compare.h"
	"src/float_utils/conversions.h"
	"src/float_utils/float_parts.h"
	"src/float_utils/log2.h"
	"src/float_utils/mul.h"
	"src/float_utils/rcp.h"
	"src/float_utils/rounding.h"
	"src/float_utils/utils.h"
	"src/fuzz.h")

function(add_exec EXEC_NAME)
	set(PROJ_NAME exec_${EXEC_NAME})

	add_executable(${PROJ_NAME})
	target_sources(${PROJ_NAME}
		PRIVATE
			"src/${PROJ_NAME}.cpp"
			${COMMON_HEADERS})
	target_compile_features(${PROJ_NAME} PRIVATE cxx_std_20)
endfunction()

add_exec(conversion)
add_exec(rounding)
add_exec(add)
add_exec(mul)
add_exec(rcp)
add_exec(log2)
