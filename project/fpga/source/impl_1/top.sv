// 11/19/2025
// Sorin Jayaweera
// Frequency counter with LED bucket display
// Counts pulses in 5 ms window and lights up corresponding frequency bucket LED

module top(
    input  logic reset,
    input  logic square,
    output logic [10:0] led  // led[9:0] = frequency buckets, led[10] = no signal
);

    // Parameters
    localparam CLK_FREQ = 48_000_000;           // 48 MHz
    localparam WINDOW_MS = 5;                    // 5 ms measurement window
    localparam WINDOW_CYCLES = CLK_FREQ * WINDOW_MS / 1000;  // 240,000 cycles
    
    // Internal signals
    logic int_osc;
    logic [17:0] timer;                          // Count to 240,000 (needs 18 bits)
    logic [13:0] pulse_count;                    // Count pulses (up to 10,000, needs 14 bits)
    logic [13:0] pulse_count_latched;            // Store count at end of window
    
    // Synchronizer for square wave input (prevent metastability)
    logic square_sync1, square_sync2;
    logic square_prev;
    logic square_edge;                           // Rising edge detector
    
    // Instantiate 48 MHz oscillator
    HSOSC #(
        .CLKHF_DIV("0b00")  // 48 MHz
    ) hf_osc (
        .CLKHFPU(1'b1),     // Power up
        .CLKHFEN(1'b1),     // Enable
        .CLKHF(int_osc)     // Clock output
    );
    
    // Synchronize input square wave to internal clock (2-stage synchronizer)
    always_ff @(posedge int_osc or posedge reset) begin
        if (reset) begin
            square_sync1 <= 0;
            square_sync2 <= 0;
        end else begin
            square_sync1 <= square;
            square_sync2 <= square_sync1;
        end
    end
    
    // Detect rising edge of synchronized square wave
    always_ff @(posedge int_osc or posedge reset) begin
        if (reset) begin
            square_prev <= 0;
        end else begin
            square_prev <= square_sync2;
        end
    end
    
    assign square_edge = square_sync2 & ~square_prev;
    
    // Timer for 5 ms window
    always_ff @(posedge int_osc or posedge reset) begin
        if (reset) begin
            timer <= 0;
        end else begin
            if (timer >= WINDOW_CYCLES - 1) begin
                timer <= 0;
            end else begin
                timer <= timer + 1;
            end
        end
    end
    
    // Pulse counter
    always_ff @(posedge int_osc or posedge reset) begin
        if (reset) begin
            pulse_count <= 0;
        end else begin
            if (timer == 0) begin
                pulse_count <= 0;  // Reset at start of new window
            end else if (square_edge) begin
                pulse_count <= pulse_count + 1;
            end
        end
    end
    
    // Latch pulse count at end of window (one cycle before timer resets)
    always_ff @(posedge int_osc or posedge reset) begin
        if (reset) begin
            pulse_count_latched <= 0;
        end else begin
            if (timer == WINDOW_CYCLES - 1) begin
                pulse_count_latched <= pulse_count;
            end
        end
    end
    
    // Decode pulse count to LED bucket (only one LED lights up)
    always_ff @(posedge int_osc or posedge reset) begin
        if (reset) begin
            led <= 11'b100_0000_0000;  // No signal LED on at reset
        end else begin
            if (timer == 0) begin  // Update LEDs at start of new window
                // Determine which bucket based on pulse count
                // Buckets are linearly spaced: 198 Hz per bucket = 990 pulses per 5ms
                if (pulse_count_latched == 0) begin
                    led <= 11'b100_0000_0000;  // led[10] = no signal
                end else if (pulse_count_latched < 1090) begin
                    led <= 11'b000_0000_0001;  // led[0] = 20-218 Hz
                end else if (pulse_count_latched < 2080) begin
                    led <= 11'b000_0000_0010;  // led[1] = 218-416 Hz
                end else if (pulse_count_latched < 3070) begin
                    led <= 11'b000_0000_0100;  // led[2] = 416-614 Hz 
                end else if (pulse_count_latched < 4060) begin
                    led <= 11'b000_0000_1000;  // led[3] = 614-812 Hz
                end else if (pulse_count_latched < 5050) begin
                    led <= 11'b000_0001_0000;  // led[4] = 812-1010 Hz
                end else if (pulse_count_latched < 6040) begin
                    led <= 11'b000_0010_0000;  // led[5] = 1010-1208 Hz
                end else if (pulse_count_latched < 7030) begin
                    led <= 11'b000_0100_0000;  // led[6] = 1208-1406 Hz
                end else if (pulse_count_latched < 8020) begin
                    led <= 11'b000_1000_0000;  // led[7] = 1406-1604 Hz
                end else if (pulse_count_latched < 9010) begin
                    led <= 11'b001_0000_0000;  // led[8] = 1604-1802 Hz
                end else begin
                    led <= 11'b010_0000_0000;  // led[9] = 1802-2000 Hz
                end
            end
        end
    end

endmodule