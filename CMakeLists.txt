cmake_minimum_required(VERSION 3.5)
project(float_testbed)

set(COMMON_HEADERS
	"src/float_utils/add.h"
	"src/float_utils/float_parts.h"
	"src/float_utils/log2.h"
	"src/float_utils/utils.h" "src/float_utils/mul.h")

function(add_exec EXEC_NAME)
	set(PROJ_NAME exec_${EXEC_NAME})

	add_executable(${PROJ_NAME})
	target_sources(${PROJ_NAME}
		PRIVATE
			"src/${PROJ_NAME}.cpp"
			${COMMON_HEADERS})
	target_compile_features(${PROJ_NAME} PRIVATE cxx_std_20)
endfunction()

add_exec(add)
add_exec(log2)
