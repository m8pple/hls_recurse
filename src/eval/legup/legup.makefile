NAME=legup_${DESIGN_NAME}
LEVEL=/home/legup/legup-4.0/examples

LOCAL_CONFIG = -legup-config=config.tcl

SRCS=legup_${DESIGN_NAME}.cpp

#SHORTEN_LABELS_LIB := ${LLVM_SHORTEN_LABELS_LIB}
# Can't find a way to get cmake to tell us where this is _and_ expand variables
SHORTEN_LABELS_LIB := ${CMAKE_BINARY_DIR}/llvm/shorten_labels/libLLVMShortenLabels.so

CPPFLAGS += -std=c++11 -I ${CMAKE_SOURCE_DIR}/../include -DNDEBUG=1 -O3
CPPFLAGS += -D__SYNTHESIS__=1

include Makefile.common
