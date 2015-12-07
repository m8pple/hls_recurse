
#get_property(LLVM_SHORTEN_LABELS_LIB TARGET LLVMShortenLabels PROPERTY LOCATION)
set(LLVM_SHORTEN_LABELS_LIB $<TARGET_FILE:LLVMShortenLabels>)

add_custom_target(eval_legup_sim)
add_custom_target(eval_legup_ip_impl)
add_custom_target(eval_legup_ip_synth)

add_custom_target(eval_legup_short_cut_sim)
add_custom_target(eval_legup_short_cut_ip_impl)
add_custom_target(eval_legup_short_cut_ip_synth)

function(add_legup_hls_design DESIGN_NAME)

configure_file(legup_generic.cpp ${DESIGN_NAME}/build/legup_${DESIGN_NAME}.cpp)

## I've spent too long messing with cmake now:
##  - "file" will expand generator names
##  - "configure_file" will expand variables
##  - there appears to be no way to do _both_
## I'm hardcoding the relative path to LLVMShortenPaths.so, as I
## just don't care any more.

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

add_dependencies(eval_legup_ip_synth
    eval_legup_${DESIGN_NAME}_ip_synth
)

add_custom_command(OUTPUT ${DESIGN_NAME}/ip/top.sta.rpt
    DEPENDS eval_legup_${DESIGN_NAME}_ip_synth
    DEPENDS ${DESIGN_NAME}/ip/legup_${DESIGN_NAME}.v
    WORKING_DIRECTORY ${DESIGN_NAME}/ip
    COMMAND cp /home/legup/legup-4.0/ip/libs/altera/altfp*.v .
    COMMAND make p
    COMMAND make q
    COMMAND make f
)

add_custom_target(eval_legup_${DESIGN_NAME}_ip_impl
    DEPENDS ${DESIGN_NAME}/ip/top.sta.rpt
)

add_dependencies(eval_legup_ip_impl
    eval_legup_${DESIGN_NAME}_ip_impl
)

###############################################
## Shortcut stuff

configure_file(legup_generic.cpp ${DESIGN_NAME}_short_cut/build/legup_${DESIGN_NAME}.cpp)

configure_file(legup.makefile ${DESIGN_NAME}_short_cut/build/makefile)

configure_file(config.tcl ${DESIGN_NAME}_short_cut/build/config.tcl)
configure_file(Makefile_short_cut.common ${DESIGN_NAME}_short_cut/build/Makefile.common COPYONLY)

add_custom_command(OUTPUT ${DESIGN_NAME}_short_cut/build/legup_${DESIGN_NAME}.v
    DEPENDS LLVMShortenLabels
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}_short_cut/build/legup_${DESIGN_NAME}.cpp
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}_short_cut/build/makefile
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}_short_cut/build/Makefile.common
    WORKING_DIRECTORY ${DESIGN_NAME}_short_cut/build
    COMMAND make  all
)

add_custom_target(eval_legup_${DESIGN_NAME}_short_cut_synth
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}_short_cut/build/legup_${DESIGN_NAME}.cpp
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}_short_cut/build/makefile
    DEPENDS ${DESIGN_NAME}_short_cut/build/legup_${DESIGN_NAME}.v
)

add_custom_command(OUTPUT ${DESIGN_NAME}_short_cut/build/legup_${DESIGN_NAME}_simulation.log
    DEPENDS eval_legup_${DESIGN_NAME}_short_cut_synth
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}_short_cut/build//makefile
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}_short_cut/build/Makefile.common
    WORKING_DIRECTORY ${DESIGN_NAME}_short_cut/build
    COMMAND make  v | tee legup_${DESIGN_NAME}_simulation.log
)

add_custom_target(eval_legup_${DESIGN_NAME}_short_cut_sim
    DEPENDS ${DESIGN_NAME}_short_cut/build/legup_${DESIGN_NAME}_simulation.log
)

add_dependencies(eval_legup_short_cut_sim eval_legup_${DESIGN_NAME}_short_cut_sim)

configure_file(ip_config.tcl ${DESIGN_NAME}_short_cut/ip/ip_config.tcl)
configure_file(Makefile.common ${DESIGN_NAME}_short_cut/ip/Makefile.common COPYONLY)
configure_file(ip_legup.makefile ${DESIGN_NAME}_short_cut/ip/makefile)

configure_file(legup_ip_generic.cpp ${DESIGN_NAME}_short_cut/ip/legup_${DESIGN_NAME}.cpp)

add_custom_command(OUTPUT ${DESIGN_NAME}_short_cut/ip/legup_${DESIGN_NAME}.v
    DEPENDS LLVMShortenLabels
    #DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}_short_cut/ip/legup_${DESIGN_NAME}.cpp
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}_short_cut/ip/makefile
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}_short_cut/ip/Makefile.common
    WORKING_DIRECTORY ${DESIGN_NAME}_short_cut/ip
    COMMAND make  all
)

add_custom_target(eval_legup_${DESIGN_NAME}_short_cut_ip_synth
    DEPENDS ${DESIGN_NAME}_short_cut/ip/legup_${DESIGN_NAME}.v
)

add_dependencies(eval_legup_short_cut_ip_synth
    eval_legup_${DESIGN_NAME}_short_cut_ip_synth
)

add_custom_command(OUTPUT ${DESIGN_NAME}_short_cut/ip/top.sta.rpt
    DEPENDS eval_legup_${DESIGN_NAME}_ip_synth
    DEPENDS ${DESIGN_NAME}_short_cut/ip/legup_${DESIGN_NAME}.v
    WORKING_DIRECTORY ${DESIGN_NAME}_short_cut/ip
    COMMAND cp /home/legup/legup-4.0/ip/libs/altera/altfp*.v .
    COMMAND make p
    COMMAND make q
    COMMAND make f
)

add_custom_target(eval_legup_${DESIGN_NAME}_short_cut_ip_impl
    DEPENDS ${DESIGN_NAME}_short_cut/ip/top.sta.rpt
)

add_dependencies(eval_legup_short_cut_ip_impl
    eval_legup_${DESIGN_NAME}_short_cut_ip_impl
)



endfunction(add_legup_hls_design)

add_legup_hls_design(fib)
add_legup_hls_design(fib_man)
add_legup_hls_design(ackerman)
add_legup_hls_design(ackerman_man)
add_legup_hls_design(sum)
add_legup_hls_design(sum_man)
add_legup_hls_design(sum_indexed)
add_legup_hls_design(sum_indexed_man)
add_legup_hls_design(fft)
add_legup_hls_design(fft_man)
add_legup_hls_design(fft_indexed)
add_legup_hls_design(fft_indexed_man)
add_legup_hls_design(sort)
add_legup_hls_design(sort_man)
add_legup_hls_design(sort_indexed)
add_legup_hls_design(sort_indexed_man)
add_legup_hls_design(sudoku)
add_legup_hls_design(sudoku_man)

## Runs out of resources, due to massive RAMs I think
#add_legup_hls_design(strassen)
#add_legup_hls_design(strassen_man)

add_legup_hls_design(strassen_indexed_v2)
add_legup_hls_design(strassen_indexed_v2_man)
#add_legup_hls_design(miser)  ## Doesn't work, some kind of internal cast error
#add_legup_hls_design(miser_man)
add_legup_hls_design(miser_indexed)
add_legup_hls_design(miser_indexed_man)

#add_legup_hls_design(tiled_mmm)
#add_legup_hls_design(tiled_mmm_indexed)
