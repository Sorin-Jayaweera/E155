// Sorin Jayaweera
// 909 957 6074
// sojayaweera@g.hmc.edu 
// 9/9/2025

module top( 
	input logic [3:0] col
	input logic reset,
	output logic sel, 
	output logic [3:0] row,
	output logic nsel, 
	output logic [6:0] segout,
	);

	HSOSC hf_osc (.CLKHFPU(1'b1), .CLKHFEN(1'b1), .CLKHF(int_osc));

	logic [3:0] iActive;

	logic [31:0] counter = 0;
	
	logic [3:0] itemp;
	logic [3:0] i0;
	logic [3:0] i1;
		logic pressed;
	logic accepting;
	logic timepassed;
	
	logic [31:0] countstart;

	// Look up table for the 7 segment displays
	segLUT lut(.s(iActive),.seg(segout));

	// always have the digit READY to push to i0 in itemp
	keypad_handler keypad(counter, col, row, pressed, itemp); 
	
	assign timepassed = countstart - counter > 100800;// 0.042 (42ms) * 24000000 (cycles per second)
	
	// choosing which set of connections for the resource use
	// sequential logic
	always_ff@(posedge int_osc) begin
		
		if(reset == 1) begin
				iActive = 4'b0000;
			end
		else begin
				iActive <= sel ? i0 : i1; //choosing for the display
			end
		end
				
	always_ff@(posedge int_osc) begin
		if(reset == 1) begin
				accepting = 1'b1;
				i0 = 4'b0000;
				i1 = 4'b0000;
			end
		else if (accepting && pressed) begin
				i1 = i0;
				i0 = itemp;
				accepting = 1'b0;
				countstart = counter;
			end
		else if (!accepting && !pressed && timepassed) begin
				accepting = 1'b1;
			end
		end
			
	// select as a slow clock logic
	always_ff@(posedge int_osc) begin
		if(reset) begin
				counter <= 25'b0;
			end
		else begin
			counter <= counter + 1'b1;
		end
	end

	assign sel = counter[18];// 80 hz
	assign nsel = !sel;
endmodule