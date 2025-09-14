-L work
-reflib pmi_work
-reflib ovi_ice40up


"C:/Users/sojayaweera/E155/lab3_sj/source/impl_1/top.sv" 
"C:/Users/sojayaweera/E155/lab3_sj/source/impl_1/sevensegLUT.sv" 
"C:/Users/sojayaweera/E155/lab3_sj/source/impl_1/keypad_handler.sv" 
-sv
-optionset VOPTDEBUG
+noacc+pmi_work.*
+noacc+ovi_ice40up.*

-vopt.options
  -suppress vopt-7033
-end

-gui
-top top
-vsim.options
  -suppress vsim-7033,vsim-8630,3009,3389
-end

-do "view wave"
-do "add wave /*"
-do "run 100 ns"
