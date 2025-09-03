-L work
-reflib pmi_work
-reflib ovi_ice40up


"C:/Users/sojayaweera/E155/lab1/fpga/sevensegdisp/source/impl_1/lab1_sj_v1.sv" 
"C:/Users/sojayaweera/E155/lab1/fpga/sevensegdisp/source/impl_1/testbench.sv" 
-sv
-optionset VOPTDEBUG
+noacc+pmi_work.*
+noacc+ovi_ice40up.*

-vopt.options
  -suppress vopt-7033
-end

-gui
-top testbench
-vsim.options
  -suppress vsim-7033,vsim-8630,3009,3389
-end

-do "view wave"
-do "add wave /*"
-do "run 100 ns"
