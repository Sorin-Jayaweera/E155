module top(
     input   logic reset,
     output  logic led
);

   logic int_osc;
   logic [24:0] counter;
  
   // Internal high-speed oscillator
   HSOSC #(.CLKHF_DIV(2'b01)) 
         hf_osc (.CLKHFPU(1'b1), .CLKHFEN(1'b1), .CLKHF(int_osc));
  
   // Counter
   always_ff @(posedge int_osc) begin
     if(reset == 0)  counter <= 0;
     else            counter <= counter + 1;
   end
  
   // Assign LED output
   assign led = counter[24];
  
endmodule