
set(VIVADO_HLS /cygdrive/C/Usr/Xilinx2015.2/Vivado_HLS/2015.2
    CACHE FILEPATH "Location of Vivado HLS root")

set(VIVADO_ROOT /cygdrive/C/Usr/Xilinx2015.2/Vivado/2015.2
    CACHE FILEPATH "Location of Vivado root")

# This is used to get relative paths from build dir to source dir, otherwise
# things break due to /cygpath prefix on cygwin which VHLS doesn't understand.
file(RELATIVE_PATH HLS_RECURSE_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(eval_vhls_synth)

function(add_vivado_hls_core DESIGN_NAME)

add_custom_command(OUTPUT ${DESIGN_NAME}/synthesise.tcl
    DEPENDS gen_synthesise_tcl.sh
    COMMAND mkdir -p ${DESIGN_NAME}/build/synth
    COMMAND echo $$(pwd)
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gen_synthesise_tcl.sh ${DESIGN_NAME} ${HLS_RECURSE_SOURCE_DIR} "" > ${DESIGN_NAME}/synthesise.tcl
    WORKING_DIRECTORY .
)

add_custom_command(OUTPUT ${DESIGN_NAME}/build/synth/${DESIGN_NAME}/sim/syn/vhdl/vhls_${DESIGN_NAME}.vhd
    DEPENDS ${DESIGN_NAME}/synthesise.tcl
    WORKING_DIRECTORY ${DESIGN_NAME}/build/synth
    COMMAND ${VIVADO_HLS}/bin/vivado_hls  -f ../../synthesise.tcl
)

add_custom_target(eval_vhls_${DESIGN_NAME}_synth
    DEPENDS ${DESIGN_NAME}/synthesise.tcl
    DEPENDS ${DESIGN_NAME}/build/synth/${DESIGN_NAME}/sim/syn/vhdl/vhls_${DESIGN_NAME}.vhd
)

add_dependencies(eval_vhls_synth eval_vhls_${DESIGN_NAME}_synth)


add_custom_command(OUTPUT ${DESIGN_NAME}/simulate.tcl
    DEPENDS gen_simulate_tcl.sh
    DEPENDS eval_vhls_${DESIGN_NAME}_synth
    COMMAND mkdir -p ${DESIGN_NAME}/build/sim
    COMMAND echo "Making simulate.tcl"
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gen_simulate_tcl.sh ${DESIGN_NAME} ${HLS_RECURSE_SOURCE_DIR} . > ${DESIGN_NAME}/simulate.tcl
    WORKING_DIRECTORY .
)

add_custom_command(OUTPUT ${DESIGN_NAME}/simulate.log
    DEPENDS ${DESIGN_NAME}/simulate.tcl
    DEPENDS eval_vhls_${DESIGN_NAME}_synth
    DEPENDS ${DESIGN_NAME}/vhls_${DESIGN_NAME}_tb.vhd
    WORKING_DIRECTORY ${DESIGN_NAME}/build/sim
    COMMAND ls ../..
    COMMAND ${VIVADO_ROOT}/bin/vivado -mode batch -source ../../simulate.tcl | tee ../../simulate.log
)

add_custom_target(eval_vhls_${DESIGN_NAME}_sim
    DEPENDS ${DESIGN_NAME}/simulate.tcl
    DEPENDS ${DESIGN_NAME}/simulate.log
)

endfunction(add_vivado_hls_core)


add_vivado_hls_core(sudoku)
add_vivado_hls_core(fib)
add_vivado_hls_core(fft_indexed)
add_vivado_hls_core(ackerman)
add_vivado_hls_core(mmm_indexed)
add_vivado_hls_core(sort_indexed)
add_vivado_hls_core(strassen_indexed)
add_vivado_hls_core(miser_indexed)
add_vivado_hls_core(sum_indexed)
