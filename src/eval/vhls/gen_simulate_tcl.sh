#!/bin/bash

# $1 = Project name
# $2 = Sources dir (e.g. src/eval/vhls)
# $3 = Build dir (e.g. build/eval/vhls)

echo "create_project -force ${1}_sim ${1}_sim -part xc7vx485tffg1157-1"

echo "add_files ${3}/../synth/${1}/sim/syn/vhdl"
echo "add_files -norecurse ../../../${2}/${1}/vhls_${1}_tb.vhd"

echo "foreach f [glob \"${3}/../synth/${1}/sim/syn/vhdl/*.tcl\"] { source \$f }"

echo "update_compile_order -fileset sources_1"
echo "update_compile_order -fileset sim_1"

echo "launch_simulation"
echo "run -all"


