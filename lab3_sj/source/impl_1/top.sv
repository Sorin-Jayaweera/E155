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
	
	
	logic [3:0] itemp; // holding the number to push to i0 or i1
	
	logic [3:0] iActive; // feeding into the 7 segment LUT
	logic [3:0] i0; // active display numbers
	logic [3:0] i1; // same
	
	logic int_osc;
	logic [31:0] counter;	
	logic pressed;
	
	// post synchronizer
	logic [3:0] col; 
	

	assign debugger = pressed;//!pressed & timepassed & cycleflag == entering;//!pressed & pressedtimepassed & cycleflag == exiting;//cycleflag == accepting;//!pressed & timepassed & cycleflag == entering;//!pressed & pressedtimepassed & cycleflag == exiting;//cycleflag == exiting;//pressedtimepassed && timepassed;

	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// MODULES
	// Synchronizer, Seven Segment look up table, keypad handler, high frequency clock and counter generation
	
	// setup the clock
	count_module clocker(.reset(reset),.counter(counter),.int_osc(int_osc));
	
	// synchronize inputs
	synchronizer colsyncer(.clk(counter[0]),.unstableval(colunstable),.stableval(col));

	// Look up table for the 7 segment displays
	sevensegLUT lut(.s(iActive),.seg(segout));

	// always have the digit READY to push to i0 in itemp
	keypad_handler keypad(.counter(counter),.col(col),.reset(reset),.row(row),.pressed(pressed),.bin(itemp)); 
	
	// state machine for debounce
	debouncer deb(int_osc, counter, pressed,reset,itemp,i0,i1);
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Time Multiplexing Seven Segment Display
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

	assign sel = counter[18];// 90 hz
	assign nsel = !sel;
endmodule