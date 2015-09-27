source /home/legup/legup-4.0/examples/legup.tcl

#set_project CycloneV DE1-SoC ARM_Simple_Hybrid_System

# LEGUP-HACK
# Can't be set, as it causes legup to crash
#set_parameter LOCAL_RAMS 1
set_accelerator_function f2_${DESIGN_NAME}

set_parameter CLOCK_PERIOD 6.666
