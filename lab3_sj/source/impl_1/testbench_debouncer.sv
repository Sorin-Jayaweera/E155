// Sorin Jayaweera
// 909 957 6074
// sojayaweera@g.hmc.edu 
// 9/13/2025

module testbench_debouncer(	);
	
	logic clk;
	logic reset;

 
	logic [31:0] counter;
	logic pressed;
	logic [3:0] itemp;
	logic [3:0] i0;
	logic [3:0] i1;
	
	debouncer dut(clk, counter,pressed,reset,i0,i1);

	initial
		begin
				reset = 1'b1;
				assert(counter == 32'b0) else $error("Bad Reset");
				
				#5
				reset = 0;
				
				// press F
				#5
				pressed = 1;
				itemp = 4'b1111;
				assert(i0 == itemp);
				
				// release F, see that i0 holds F
				#5;
				pressed = 1'b0;
				itemp = 4'b0000;
				assert(i0 == 4'b1111);
				
				//check that debounce timer is active and not accepting since state changes
				#5; 
				pressed = 1'b1;
				itemp = 4'b0110;
				assert(i0 == 4'b1111);				
				
		end


	always begin
		clk = 0; #1; clk = 1; #1;
	end

endmodule


//module count_module(input logic reset, output logic [31:0] counter, output logic int_osc);
//
//	HSOSC #(.CLKHF_DIV(2'b01)) hf_osc(.CLKHFPU(1'b1), .CLKHFEN(1'b1), .CLKHF(int_osc));

	// slow clock logic
//	always_ff@(posedge int_osc) begin
//		if(reset) begin
//				counter <= 32'b0;
//			end
//		else begin
//			counter <= counter + 1'b1;
//		end
//	end
//	
//endmodule
