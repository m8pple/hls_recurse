cmake_minimum_required (VERSION 3.2)
project (hls_recursion)

set (CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# It is common to label loops in HLS code, but it isn't
# a problem if we don't use them.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-label")

include_directories( ../include )

# Creates an executable and adds it as a test.
function(add_executable_test PREFIX NAME)
add_executable(${NAME}_test ${NAME}_test.cpp)
add_test(${PREFIX}_${NAME}_test ${EXECUTABLE_OUTPUT_PATH}/${NAME}_test)
endfunction(add_executable_test)

enable_testing()

if(IS_DIRECTORY /home/legup/legup-4.0/llvm)
    add_subdirectory(llvm)
endif(IS_DIRECTORY /home/legup/legup-4.0/llvm)

add_subdirectory(examples)
add_subdirectory(core)
add_subdirectory(mutual)
add_subdirectory(eval)


