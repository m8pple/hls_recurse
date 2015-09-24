NAME=legup_${DESIGN_NAME}
LEVEL=/home/legup/legup-4.0/examples

SRCS=legup_${DESIGN_NAME}.cpp

SHORTEN_LABELS_LIB := ${LLVM_SHORTEN_LABELS_LIB}

CPPFLAGS += -std=c++11 -I ${CMAKE_SOURCE_DIR}/../include -DNDEBUG=1 -O3
CPPFLAGS += -D__SYNTHESIS__=1

include Makefile.common
