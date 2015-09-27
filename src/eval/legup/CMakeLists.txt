
#get_property(LLVM_SHORTEN_LABELS_LIB TARGET LLVMShortenLabels PROPERTY LOCATION)
set(LLVM_SHORTEN_LABELS_LIB $<TARGET_FILE:LLVMShortenLabels>)

add_custom_target(eval_legup_sim)

function(add_legup_hls_design DESIGN_NAME)

configure_file(legup_generic.cpp ${DESIGN_NAME}/build/legup_${DESIGN_NAME}.cpp)

## I've spent too long messing with cmake now:
##  - file will expand generator names
##  - configure_file will expand variables
##  - there appears to be no way to do _both_
## I'm hardcoding the relative path to LLVMShortenPaths.so, as I
## just don't care any more.

#file(GENERATE
#    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/build/legup.makefile
#    INPUT ${CMAKE_CURRENT_SOURCE_DIR}/legup.makefile.1
#)
configure_file(legup.makefile ${DESIGN_NAME}/build/makefile)

configure_file(config.tcl ${DESIGN_NAME}/build/config.tcl)
configure_file(Makefile.common ${DESIGN_NAME}/build/Makefile.common COPYONLY)

add_custom_command(OUTPUT ${DESIGN_NAME}/build/legup_${DESIGN_NAME}.v
    DEPENDS LLVMShortenLabels
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/build/legup_${DESIGN_NAME}.cpp
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/build/makefile
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/build/Makefile.common
    WORKING_DIRECTORY ${DESIGN_NAME}/build
    COMMAND make  all
)

add_custom_target(eval_legup_${DESIGN_NAME}_synth
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/build/legup_${DESIGN_NAME}.cpp
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/build/makefile
    DEPENDS ${DESIGN_NAME}/build/legup_${DESIGN_NAME}.v
)

add_custom_command(OUTPUT ${DESIGN_NAME}/build/legup_${DESIGN_NAME}_simulation.log
    DEPENDS eval_legup_${DESIGN_NAME}_synth
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/build//makefile
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/build/Makefile.common
    WORKING_DIRECTORY ${DESIGN_NAME}/build
    COMMAND make  v | tee legup_${DESIGN_NAME}_simulation.log
)

add_custom_target(eval_legup_${DESIGN_NAME}_sim
    DEPENDS ${DESIGN_NAME}/build/legup_${DESIGN_NAME}_simulation.log
)

add_dependencies(eval_legup_sim eval_legup_${DESIGN_NAME}_sim)

configure_file(ip_config.tcl ${DESIGN_NAME}/ip/ip_config.tcl)
configure_file(Makefile.common ${DESIGN_NAME}/ip/Makefile.common COPYONLY)
configure_file(ip_legup.makefile ${DESIGN_NAME}/ip/makefile)

configure_file(legup_ip_generic.cpp ${DESIGN_NAME}/ip/legup_${DESIGN_NAME}.cpp)

add_custom_command(OUTPUT ${DESIGN_NAME}/ip/legup_${DESIGN_NAME}.v
    DEPENDS LLVMShortenLabels
    #DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/ip/legup_${DESIGN_NAME}.cpp
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/ip/makefile
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/ip/Makefile.common
    WORKING_DIRECTORY ${DESIGN_NAME}/ip
    COMMAND make  all
)

add_custom_target(eval_legup_${DESIGN_NAME}_ip_synth
    DEPENDS ${DESIGN_NAME}/ip/legup_${DESIGN_NAME}.v
)

endfunction(add_legup_hls_design)

add_legup_hls_design(fib)
add_legup_hls_design(ackerman)
add_legup_hls_design(sum)
add_legup_hls_design(sum_indexed)
add_legup_hls_design(fft)
add_legup_hls_design(fft_indexed)
add_legup_hls_design(sort)
add_legup_hls_design(sort_indexed)
add_legup_hls_design(sudoku)  ## Doesn't work due to incorrect addressing
add_legup_hls_design(strassen)
add_legup_hls_design(strassen_indexed_v2)
add_legup_hls_design(miser)  ## Doesn't work, some kind of internal cast error
add_legup_hls_design(miser_indexed)
add_legup_hls_design(tiled_mmm)
add_legup_hls_design(tiled_mmm_indexed)
