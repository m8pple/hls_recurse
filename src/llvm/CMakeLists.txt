cmake_minimum_required(VERSION 2.8.12)

include(LLVMConfig.cmake)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

add_subdirectory(shorten_labels)
