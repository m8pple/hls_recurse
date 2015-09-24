#!/bin/bash

# $1 = Project name
# $2 = Sources dir (e.g. src/eval/vhls)
# $3 = Build dir (e.g. build/eval/vhls)

echo "create_project -force ${1}_impl ${1}_impl -part xc7vx485tffg1157-1"

echo "add_files ${3}/../synth/${1}/sim/syn/vhdl"

echo "foreach f [glob -nocomplain \"${3}/../synth/${1}/sim/syn/vhdl/*.tcl\"] { source \$f }"

echo "update_compile_order -fileset sources_1"
echo "update_compile_order -fileset sim_1"

#echo "launch_runs synth_1"
#echo "open_run synth_1 -name synth_1"

echo "synth_design -name synth_1"
echo "create_clock -period 6.66666 -name ap_clk -waveform {0.000 3.33333} [get_ports ap_clk]"

echo "file mkdir fib_impl/fib_impl.srcs/constrs_1"
echo "file mkdir fib_impl/fib_impl.srcs/constrs_1/new"
echo "close [ open fib_impl/fib_impl.srcs/constrs_1/new/constraints.xdc w ]"
echo "add_files -fileset constrs_1 fib_impl/fib_impl.srcs/constrs_1/new/constraints.xdc"
echo "set_property target_constrs_file fib_impl/fib_impl.srcs/constrs_1/new/constraints.xdc [current_fileset -constrset]"
echo "save_constraints -force"

echo "reset_run synth_1"

echo "synth_design -name synth_1"
echo "report_utilization -file ../../synth_utilisation.txt -name utilization_1"

echo "opt_design"
echo "report_utilization -file ../../opt_utilisation.txt -name utilization_1"

echo "place_design"
echo "route_design"
echo "report_utilization -file ../../impl_utilisation.txt -name utilization_1"
echo "report_timing_summary -delay_type min_max -report_unconstrained -check_timing_verbose -max_paths 10 -nworst 3 -input_pins -name timing_1 -file ../../impl_timing.txt"

echo "write_checkpoint -force ../../impl_checkpoint.dcp"
