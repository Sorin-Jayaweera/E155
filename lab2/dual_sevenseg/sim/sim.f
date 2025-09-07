-L work
-reflib pmi_work
-reflib ovi_ice40up


"C:/Users/sojayaweera/E155/lab2/dual_sevenseg/source/top/top.sv" 
"C:/Users/sojayaweera/E155/lab2/dual_sevenseg/source/top/segLUT.sv" 
"C:/Users/sojayaweera/E155/lab2/dual_sevenseg/source/top/test_sum.sv" 
-sv
-optionset VOPTDEBUG
+noacc+pmi_work.*
+noacc+ovi_ice40up.*

-vopt.options
  -suppress vopt-7033
-end

-gui
-top test_sum
-vsim.options
  -suppress vsim-7033,vsim-8630,3009,3389
-end

-do "view wave"
-do "add wave /*"
-do "run 100 ns"
