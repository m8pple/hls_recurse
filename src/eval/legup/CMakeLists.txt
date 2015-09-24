
get_property(LLVM_SHORTEN_LABELS_LIB TARGET LLVMShortenLabels PROPERTY LOCATION)

function(add_legup_hls_design DESIGN_NAME)

configure_file(legup_${DESIGN_NAME}.cpp ${DESIGN_NAME}/build/legup_${DESIGN_NAME}.cpp)
configure_file(legup.makefile ${DESIGN_NAME}/build/legup.makefile)
configure_file(Makefile.common ${DESIGN_NAME}/build/Makefile.common COPYONLY)

add_custom_command(OUTPUT ${DESIGN_NAME}/build/${DESIGN_NAME}.v
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/build/legup_${DESIGN_NAME}.cpp
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/build/legup.makefile
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/build/Makefile.common
    WORKING_DIRECTORY ${DESIGN_NAME}/build
    COMMAND make -B -f legup.makefile all
)

add_custom_target(eval_legup_${DESIGN_NAME}_synth
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/build/legup_${DESIGN_NAME}.cpp
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}/build/legup.makefile
    DEPENDS ${DESIGN_NAME}/build/${DESIGN_NAME}.v
)

endfunction(add_legup_hls_design)

add_legup_hls_design(fib)
add_legup_hls_design(fft)
