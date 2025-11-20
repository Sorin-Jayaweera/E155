// 12 LEDs: LED[0]=reset status, LED[1]=collecting status, LED[11:2]=10-bit count
module top(
    input  logic reset,
    input  logic square,
    output logic [11:0] led  // 12 LEDs
);
    logic int_osc;
    localparam [25:0] WINDOW_CYCLES = 26'd480_000;  // 10ms at 48 MHz
    
    logic [25:0] timer;
    logic [9:0] edge_count;         // 10-bit counter (0-1023)
    logic [9:0] edge_count_display;
    logic collecting;
    
    logic square_sync1, square_sync2, square_prev;
    logic reset_sync1, reset_sync2, reset_prev;
    logic square_edge, reset_released;
    
    HSOSC #(.CLKHF_DIV("0b00")) hf_osc (
        .CLKHFPU(1'b1),
        .CLKHFEN(1'b1),
        .CLKHF(int_osc)
    );
    
    // Synchronize inputs
    always_ff @(posedge int_osc) begin
        square_sync1 <= square;
        square_sync2 <= square_sync1;
        square_prev <= square_sync2;
        
        reset_sync1 <= reset;
        reset_sync2 <= reset_sync1;
        reset_prev <= reset_sync2;
    end
    
    assign square_edge = ~square_sync2 & square_prev;  // FALLING edge
    assign reset_released = reset_sync2 & ~reset_prev;
    
    // Collection state
    always_ff @(posedge int_osc) begin
        if (~reset_sync2) begin
            collecting <= 1'b0;
        end else if (reset_released) begin
            collecting <= 1'b1;
        end else if (collecting && timer >= WINDOW_CYCLES - 1) begin
            collecting <= 1'b0;
        end
    end
    
    // Timer
    always_ff @(posedge int_osc) begin
        if (~reset_sync2 || reset_released) begin
            timer <= 26'd0;
        end else if (collecting) begin
            timer <= timer + 26'd1;
        end
    end
    
    // Edge counter (10 bits)
    always_ff @(posedge int_osc) begin
        if (~reset_sync2 || reset_released) begin
            edge_count <= 10'd0;
        end else if (collecting && square_edge) begin
            edge_count <= edge_count + 10'd1;
        end
    end
    
    // Latch display at end
    always_ff @(posedge int_osc) begin
        if (~reset_sync2) begin
            edge_count_display <= 10'd0;
        end else if (timer == WINDOW_CYCLES - 1) begin
            edge_count_display <= edge_count;
        end
    end
    
    // LED display
    always_ff @(posedge int_osc) begin
        led[0] <= reset_sync2;      // Leftmost: reset status
        led[1] <= collecting;       // Second: collecting status
        led[11:2] <= edge_count_display;  // Remaining 10 LEDs: binary count
    end
endmodule