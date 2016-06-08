set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

# This avoids a spurious symbol missing error when loading into opt
# http://lists.cs.uiuc.edu/pipermail/llvmdev/2013-June/062939.html
set(CMAKE_CXX_FLAGS "-fno-rtti ${CMAKE_CXX_FLAGS}")

add_library(LLVMDeRecurse MODULE DeRecurse.cpp)


