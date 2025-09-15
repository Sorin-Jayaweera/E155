// Sorin Jayaweera
// 909 957 6074
// sojayaweera@g.hmc.edu 
// 9/9/2025

module top( 
	input  logic reset,
	input  logic [3:0] colunstable,	
	output logic sel, 
	output logic [3:0] row,
	output logic nsel, 
	output logic [6:0] segout,
	output logic debugger
	);
	
	// post synchronizer
	logic [3:0] col; 
	
	
	logic [3:0] iActive; // feeding into the 7 segment LUT
	logic [3:0] itemp; // holding the number to push to i0 or i1
	logic [3:0] i0; // active display numbers
	logic [3:0] i1; // same
	
	logic int_osc;
		logic pressed;
	logic accepting;
	logic timepassed;
	logic pressedtimepassed;
	
	logic [31:0] counter;
	logic [31:0] countstart;
	logic [31:0] pressedcountstart;
	
	assign debugger = pressedtimepassed;
	////////////////////////////////////////////////////////////////////////////////////////////////
	// MODULES
	// Synchronizer, Seven Segment look up table, keypad handler, high frequency clock and counter generation
	// Look up table for the 7 segment displays
	
	sevensegLUT lut(.s(iActive),.seg(segout));

	// always have the digit READY to push to i0 in itemp
	keypad_handler keypad(.counter(counter),.col(col),.row(row),.pressed(pressed),.bin(itemp)); 
	
	// setup the clock
	count_module clocker(.reset(reset),.counter(counter),.int_osc(int_osc));
	
	// synchronize inputs
	synchronizer colsyncer(.clk(int_osc),.unstableval(colunstable),.stableval(col));
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Time Multiplexing Seven Segment Display
	assign timepassed = (counter - countstart ) > 6000000;// 0.042 (42ms) * 12000000 (cycles per second) (HFOSC at half speed)
	assign pressedtimepassed = (counter - pressedcountstart) > 6000000;
	// choosing which set of connections for the resource use
	// sequential logic
	always_ff@(posedge int_osc) begin
		
		if(reset == 1) begin
				iActive = 4'b0000;
			end
		else begin
				iActive =  sel ? i0 : i1; //choosing for the display 
			end
		end
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Debounce State Logic
	//	
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
				pressedcountstart = counter;
			end
		else if(!accepting && !pressed && pressedtimepassed && timepassed)begin
				accepting = 1'b1;
			end
		end
			
	////////////////////////////////////////////////////////////////////////////////////////////////

	assign sel = counter[18];// 80 hz
	assign nsel = !sel;
endmodule