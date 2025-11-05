// Sorin Jayaweera
// 909 957 6074
// sojayaweera@g.hmc.edu 
// 9/13/2025

module count_module(input logic reset, output logic [31:0] counter, output logic int_osc);

	//HSOSC #(.CLKHF_DIV(2'b01)) hf_osc(.CLKHFPU(1'b1), .CLKHFEN(1'b1), .CLKHF(int_osc));
	HSOSC #(.CLKHF_DIV(2'b01)) hf_osc(.CLKHFPU(1'b1), .CLKHFEN(1'b1), .CLKHF(int_osc));

	// slow clock logic
	always_ff@(posedge int_osc) begin
		if(reset) begin
				counter <= 32'b0;
			end
		else begin
			counter <= counter + 1'b1;
		end
	end
	
endmodule


