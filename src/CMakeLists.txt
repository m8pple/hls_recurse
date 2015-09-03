cmake_minimum_required (VERSION 3.2)
project (hls_recursion)

set (CMAKE_CXX_STANDARD 11)

include_directories( ../include )

# Creates an executable and adds it as a test.
function(add_executable_test PREFIX NAME)
add_executable(${NAME}_test ${NAME}_test.cpp)
add_test(${PREFIX}_${NAME}_test ${EXECUTABLE_OUTPUT_PATH}/${NAME}_test)
endfunction(add_executable_test)

enable_testing()

add_subdirectory(examples)
add_subdirectory(core)
add_subdirectory(mutual)
