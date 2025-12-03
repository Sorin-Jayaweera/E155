module pulse_counter (
    input  logic clk,
    input  logic reset,
    input  logic signal_in,      // Expects synchronized input
    input  logic enable,         // High during COLLECTING
    input  logic latch,          // High during LATCHING
    output logic [7:0] count_out
);
    logic signal_prev;
    logic is_falling_edge;
    logic [7:0] internal_count;

    // Edge Detection
    always_ff @(posedge clk, posedge reset) begin
        if (reset) signal_prev <= 1'b0;
        else       signal_prev <= signal_in;
    end
    assign is_falling_edge = ~signal_in & signal_prev;

    // Counting and Latching
    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            internal_count <= 8'd0;
            count_out <= 8'd0;
        end else begin
            // 1. Latch Phase: Output data, reset internal counter
            if (latch) begin
                count_out <= internal_count;
                internal_count <= 8'd0; 
            end 
            // 2. Collection Phase: Increment on edge
            else if (enable && is_falling_edge) begin
                internal_count <= internal_count + 8'd1;
            end
        end
    end
endmodule