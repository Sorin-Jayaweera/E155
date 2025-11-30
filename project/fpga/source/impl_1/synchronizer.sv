// Synchronizer module - 2-stage synchronizer for any async input
module synchronizer (
    input  logic clk,
    input  logic reset,
    input  logic async_in,
    output logic sync_out
);
    logic sync1, sync2;
    
    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            sync1 <= 1'b0;
            sync2 <= 1'b0;
        end else begin
            sync1 <= async_in;
            sync2 <= sync1;
        end
    end
    
    assign sync_out = sync2;
endmodule