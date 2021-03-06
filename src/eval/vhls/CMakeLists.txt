
set(VIVADO_HLS /cygdrive/C/Usr/Xilinx2015.2/Vivado_HLS/2015.2
    CACHE FILEPATH "Location of Vivado HLS root")

set(VIVADO_ROOT /cygdrive/C/Usr/Xilinx2015.2/Vivado/2015.2
    CACHE FILEPATH "Location of Vivado root")

# This is used to get relative paths from build dir to source dir, otherwise
# things break due to /cygpath prefix on cygwin which VHLS doesn't understand.
file(RELATIVE_PATH HLS_RECURSE_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(eval_vhls_synth)
add_custom_target(eval_vhls_impl)
add_custom_target(eval_vhls_sim)
add_custom_target(eval_vhls)

add_custom_target(eval_vhls_short_cut_synth)
add_custom_target(eval_vhls_short_cut_impl)
add_custom_target(eval_vhls_short_cut_sim)
add_custom_target(eval_vhls_short_cut)

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

add_custom_command(
    OUTPUT ${DESIGN_NAME}/${DESIGN_NAME}_vhls.zip
    DEPENDS ${DESIGN_NAME}/build/synth/${DESIGN_NAME}/sim/syn/vhdl/vhls_${DESIGN_NAME}.vhd
    WORKING_DIRECTORY ${DESIGN_NAME}/build/synth/${DESIGN_NAME}/sim/syn/vhdl
    COMMAND zip ../../../../../../${DESIGN_NAME}_vhls.zip *.*
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

add_dependencies(eval_vhls_sim
    eval_vhls_${DESIGN_NAME}_sim
)

add_custom_command(OUTPUT ${DESIGN_NAME}/implement.tcl
    DEPENDS gen_implement_tcl.sh
    DEPENDS eval_vhls_${DESIGN_NAME}_synth
    WORKING_DIRECTORY .
    COMMAND mkdir -p ${DESIGN_NAME}/build/impl
    COMMAND echo "Making implement.tcl"
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gen_implement_tcl.sh ${DESIGN_NAME} ${HLS_RECURSE_SOURCE_DIR} . > ${DESIGN_NAME}/implement.tcl
)

add_custom_command(
    OUTPUT ${DESIGN_NAME}/implement.log
    OUTPUT ${DESIGN_NAME}/synth_utilisation.txt
    OUTPUT ${DESIGN_NAME}/opt_utilisation.txt
    OUTPUT ${DESIGN_NAME}/impl_utilisation.txt
    OUTPUT ${DESIGN_NAME}/impl_timing.txt
    OUTPUT ${DESIGN_NAME}/impl_checkpoint.dcp
    DEPENDS ${DESIGN_NAME}/implement.tcl
    DEPENDS eval_vhls_${DESIGN_NAME}_synth
    WORKING_DIRECTORY ${DESIGN_NAME}/build/impl
    COMMAND ls ../..
    COMMAND ${VIVADO_ROOT}/bin/vivado -mode batch -source ../../implement.tcl | tee ../../implement.log
)

add_custom_target(eval_vhls_${DESIGN_NAME}_impl
    DEPENDS ${DESIGN_NAME}/implement.tcl
    DEPENDS ${DESIGN_NAME}/implement.log
)

add_dependencies(eval_vhls_impl
    eval_vhls_${DESIGN_NAME}_impl
)

add_custom_command(
    OUTPUT ${DESIGN_NAME}/${DESIGN_NAME}_results.zip
    DEPENDS ${DESIGN_NAME}/${DESIGN_NAME}_vhls.zip
    DEPENDS ${DESIGN_NAME}/implement.log
    DEPENDS ${DESIGN_NAME}/synth_utilisation.txt
    DEPENDS ${DESIGN_NAME}/opt_utilisation.txt
    DEPENDS ${DESIGN_NAME}/impl_utilisation.txt
    DEPENDS ${DESIGN_NAME}/impl_timing.txt
    DEPENDS ${DESIGN_NAME}/impl_checkpoint.dcp
    WORKING_DIRECTORY ${DESIGN_NAME}
    COMMAND zip ${DESIGN_NAME}_results.zip *.txt *.log *.dcp *_vhls.zip
)

add_custom_target(eval_vhls_${DESIGN_NAME}
    DEPENDS ${DESIGN_NAME}/${DESIGN_NAME}_results.zip
)

add_dependencies(eval_vhls eval_vhls_${DESIGN_NAME})



##############################################################

add_custom_command(OUTPUT ${DESIGN_NAME}_short_cut/synthesise.tcl
    DEPENDS gen_synthesise_tcl_short_cut.sh
    COMMAND mkdir -p ${DESIGN_NAME}_short_cut/build/synth
    COMMAND echo $$(pwd)
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gen_synthesise_tcl_short_cut.sh ${DESIGN_NAME} ${HLS_RECURSE_SOURCE_DIR} "" > ${DESIGN_NAME}_short_cut/synthesise.tcl
    WORKING_DIRECTORY .
)

add_custom_command(OUTPUT ${DESIGN_NAME}_short_cut/build/synth/${DESIGN_NAME}_short_cut/sim/syn/vhdl/vhls_${DESIGN_NAME}.vhd
    DEPENDS ${DESIGN_NAME}_short_cut/synthesise.tcl
    WORKING_DIRECTORY ${DESIGN_NAME}_short_cut/build/synth
    COMMAND ${VIVADO_HLS}/bin/vivado_hls  -f ../../synthesise.tcl
)

add_custom_command(
    OUTPUT ${DESIGN_NAME}_short_cut/${DESIGN_NAME}_vhls.zip
    DEPENDS ${DESIGN_NAME}_short_cut/build/synth/${DESIGN_NAME}_short_cut/sim/syn/vhdl/vhls_${DESIGN_NAME}.vhd
    WORKING_DIRECTORY ${DESIGN_NAME}_short_cut/build/synth/${DESIGN_NAME}_short_cut/sim/syn/vhdl
    COMMAND zip ../../../../../../${DESIGN_NAME}_vhls.zip *.*
)

add_custom_target(eval_vhls_${DESIGN_NAME}_short_cut_synth
    DEPENDS ${DESIGN_NAME}_short_cut/synthesise.tcl
    DEPENDS ${DESIGN_NAME}_short_cut/build/synth/${DESIGN_NAME}_short_cut/sim/syn/vhdl/vhls_${DESIGN_NAME}.vhd
)

add_dependencies(eval_vhls_short_cut_synth eval_vhls_${DESIGN_NAME}_short_cut_synth)


add_custom_command(OUTPUT ${DESIGN_NAME}_short_cut/simulate.tcl
    DEPENDS gen_simulate_tcl.sh
    DEPENDS eval_vhls_${DESIGN_NAME}_short_cut_synth
    COMMAND mkdir -p ${DESIGN_NAME}_short_cut/build/sim
    COMMAND echo "Making simulate.tcl"
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gen_simulate_tcl.sh ${DESIGN_NAME} ${HLS_RECURSE_SOURCE_DIR} . > ${DESIGN_NAME}_short_cut/simulate.tcl
    WORKING_DIRECTORY .
)

configure_file( ${DESIGN_NAME}/vhls_${DESIGN_NAME}_tb.vhd ${DESIGN_NAME}_short_cut/vhls_${DESIGN_NAME}_tb.vhd )

add_custom_command(OUTPUT ${DESIGN_NAME}_short_cut/simulate.log
    DEPENDS ${DESIGN_NAME}_short_cut/simulate.tcl
    DEPENDS eval_vhls_${DESIGN_NAME}_short_cut_synth
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DESIGN_NAME}_short_cut/vhls_${DESIGN_NAME}_tb.vhd
    WORKING_DIRECTORY ${DESIGN_NAME}_short_cut/build/sim
    COMMAND ls ../..
    COMMAND ${VIVADO_ROOT}/bin/vivado -mode batch -source ../../simulate.tcl | tee ../../simulate.log
)

add_custom_target(eval_vhls_${DESIGN_NAME}_short_cut_sim
    DEPENDS ${DESIGN_NAME}_short_cut/simulate.tcl
    DEPENDS ${DESIGN_NAME}_short_cut/simulate.log
)

add_dependencies(eval_vhls_short_cut_sim
    eval_vhls_${DESIGN_NAME}_short_cut_sim
)

add_custom_command(OUTPUT ${DESIGN_NAME}_short_cut/implement.tcl
    DEPENDS gen_implement_tcl.sh
    DEPENDS eval_vhls_${DESIGN_NAME}_short_cut_synth
    WORKING_DIRECTORY .
    COMMAND mkdir -p ${DESIGN_NAME}_short_cut/build/impl
    COMMAND echo "Making implement.tcl"
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gen_implement_tcl.sh ${DESIGN_NAME} ${HLS_RECURSE_SOURCE_DIR} . > ${DESIGN_NAME}_short_cut/implement.tcl
)

add_custom_command(
    OUTPUT ${DESIGN_NAME}_short_cut/implement.log
    OUTPUT ${DESIGN_NAME}_short_cut/synth_utilisation.txt
    OUTPUT ${DESIGN_NAME}_short_cut/opt_utilisation.txt
    OUTPUT ${DESIGN_NAME}_short_cut/impl_utilisation.txt
    OUTPUT ${DESIGN_NAME}_short_cut/impl_timing.txt
    OUTPUT ${DESIGN_NAME}_short_cut/impl_checkpoint.dcp
    DEPENDS ${DESIGN_NAME}_short_cut/implement.tcl
    DEPENDS eval_vhls_${DESIGN_NAME}_short_cut_synth
    WORKING_DIRECTORY ${DESIGN_NAME}_short_cut/build/impl
    COMMAND ls ../..
    COMMAND ${VIVADO_ROOT}/bin/vivado -mode batch -source ../../implement.tcl | tee ../../implement.log
)

add_custom_target(eval_vhls_${DESIGN_NAME}_short_cut_impl
    DEPENDS ${DESIGN_NAME}_short_cut/implement.tcl
    DEPENDS ${DESIGN_NAME}_short_cut/implement.log
)

add_dependencies(eval_vhls_short_cut_impl
    eval_vhls_${DESIGN_NAME}_short_cut_impl
)

add_custom_command(
    OUTPUT ${DESIGN_NAME}_short_cut/${DESIGN_NAME}_results.zip
    DEPENDS ${DESIGN_NAME}_short_cut/${DESIGN_NAME}_vhls.zip
    DEPENDS ${DESIGN_NAME}_short_cut/implement.log
    DEPENDS ${DESIGN_NAME}_short_cut/synth_utilisation.txt
    DEPENDS ${DESIGN_NAME}_short_cut/opt_utilisation.txt
    DEPENDS ${DESIGN_NAME}_short_cut/impl_utilisation.txt
    DEPENDS ${DESIGN_NAME}_short_cut/impl_timing.txt
    DEPENDS ${DESIGN_NAME}_short_cut/impl_checkpoint.dcp
    WORKING_DIRECTORY ${DESIGN_NAME}
    COMMAND zip ${DESIGN_NAME}_results.zip *.txt *.log *.dcp *_vhls.zip
)

add_custom_target(eval_vhls_${DESIGN_NAME}_short_cut
    DEPENDS ${DESIGN_NAME}_short_cut/${DESIGN_NAME}_results.zip
)

add_dependencies(eval_vhls_short_cut eval_vhls_${DESIGN_NAME}_short_cut)



endfunction(add_vivado_hls_core)

add_vivado_hls_core(fib)
add_vivado_hls_core(fib_man)

add_vivado_hls_core(ackerman)
add_vivado_hls_core(ackerman_man)

add_vivado_hls_core(sum_indexed)
add_vivado_hls_core(sum_indexed_man)

add_vivado_hls_core(sort_indexed)
add_vivado_hls_core(sort_indexed_man)

add_vivado_hls_core(sudoku)
add_vivado_hls_core(sudoku_man)

add_vivado_hls_core(fft_indexed)
add_vivado_hls_core(fft_indexed_man)

#add_vivado_hls_core(tiled_mmm_indexed)

add_vivado_hls_core(strassen_indexed_v2)
add_vivado_hls_core(strassen_indexed_v2_man)

add_vivado_hls_core(naive_mmm_indexed)

add_vivado_hls_core(miser_indexed)
add_vivado_hls_core(miser_indexed_man)
