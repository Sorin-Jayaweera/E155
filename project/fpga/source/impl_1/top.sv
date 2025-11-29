// Main module
module top(
    input  logic reset_in,    // Active LOW (pressed = 0, released = 1)
    input  logic square,
    output logic [11:0] led
);
    logic int_osc;
    logic reset;              // Active HIGH internally
    
    // Invert reset so everything uses positive edge
    assign reset = ~reset_in;
    
	localparam [25:0] WINDOW_CYCLES = 26'd4_800_000;  // 100ms at 48 MHz
    
    logic [25:0] timer;
    logic [9:0] edge_count;
    logic [9:0] edge_count_display;
    logic collecting;
    
    logic square_sync, square_prev;
    logic square_edge;
    logic reset_released;
    
    HSOSC #(.CLKHF_DIV("0b00")) hf_osc (
        .CLKHFPU(1'b1),
        .CLKHFEN(1'b1),
        .CLKHF(int_osc)
    );
    
    // Synchronize square wave input
    synchronizer square_synchronizer (
        .clk(int_osc),
        .async_in(square),
        .sync_out(square_sync)
    );
    
    // Edge detection on square wave
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            square_prev <= 1'b0;
        end else begin
            square_prev <= square_sync;
        end
    end
    
    assign square_edge = ~square_sync & square_prev;  // FALLING edge
    
    // Detect when reset is released
    logic reset_prev;
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            reset_prev <= 1'b1;
        end else begin
            reset_prev <= reset;
        end
    end
    
    assign reset_released = reset_prev & ~reset;
    
    //===========================================
    // STATE MACHINE: Collection Control
    //===========================================
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            collecting <= 1'b0;
        end else if (reset_released) begin
            collecting <= 1'b1;
        end else if (collecting && timer >= WINDOW_CYCLES - 1) begin
            collecting <= 1'b0;
        end
    end
    
    //===========================================
    // DATAPATH: Timer
    //===========================================
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            timer <= 26'd0;
        end else if (reset_released) begin
            timer <= 26'd0;
        end else if (collecting) begin
            timer <= timer + 26'd1;
        end
    end
    
    //===========================================
    // DATAPATH: Edge Counter
    //===========================================
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            edge_count <= 10'd0;
        end else if (reset_released) begin
            edge_count <= 10'd0;
        end else if (collecting && square_edge) begin
            edge_count <= edge_count + 10'd1;
        end
    end
    
    //===========================================
    // DATAPATH: Display Latch
    //===========================================
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            edge_count_display <= 10'd0;
        end else if (timer == WINDOW_CYCLES - 2) begin
            edge_count_display <= edge_count;
        end
    end
    
    //===========================================
    // OUTPUT: LED Display - ALL HIGH DURING RESET
    //===========================================
    always_ff @(posedge int_osc, posedge reset) begin
        if (reset) begin
            led <= 12'b111111111111;  // ALL LEDs ON when reset pressed
        end else begin
            led[0] <= 1'b0;           // LED[0] unused (off)
            led[1] <= collecting;     // LED[1] = collecting pulse
            // 10-bit count: MSB on LED[2], LSB on LED[11]
            led[11:2] <= {edge_count_display[0], edge_count_display[1], 
                          edge_count_display[2], edge_count_display[3],
                          edge_count_display[4], edge_count_display[5],
                          edge_count_display[6], edge_count_display[7],
                          edge_count_display[8], edge_count_display[9]};
        end
    end
endmodule