module top(
    input  logic reset_in,    // Active LOW (pressed = 0)
    input  logic square,
    output logic [11:0] led
);

    //===========================================
    // INTERNAL SIGNALS
    //===========================================
    logic clk;           // 48MHz Internal Clock
    logic reset;         // Active High Reset
    logic square_sync;   // Synchronized Input
    
    // Control Signals (From FSM)
    logic is_collecting;
    logic is_latching;
    
    // Data Signals
    logic [7:0] edge_count_latched;
    logic [3:0] target_bucket;

    // Invert Reset (Button is Active Low)
    assign reset = ~reset_in;

    //===========================================
    // 1. CLOCK GENERATION (HSOSC)
    //===========================================
    HSOSC #(.CLKHF_DIV("0b00")) hf_osc (
        .CLKHFPU(1'b1),
        .CLKHFEN(1'b1),
        .CLKHF(clk)
    );

    //===========================================
    // 2. INPUT SYNCHRONIZER
    //===========================================
    synchronizer sync_inst (
        .clk(clk),
        .reset(reset),
        .async_in(square),
        .sync_out(square_sync)
    );

    //===========================================
    // 3. STATE MACHINE (Timing & Control)
    //===========================================
    fsm_control fsm_inst (
        .clk(clk),
        .reset(reset),
        .is_collecting(is_collecting),
        .is_latching(is_latching)
    );

    //===========================================
    // 4. PULSE COUNTER (Edge Detect + Counting)
    //===========================================
    pulse_counter counter_inst (
        .clk(clk),
        .reset(reset),
        .signal_in(square_sync),
        .enable(is_collecting),
        .latch(is_latching),
        .count_out(edge_count_latched)
    );

    //===========================================
    // 5. LED DECODER (Combinational Logic)
    //===========================================
    led_decoder decoder_inst (
        .edge_count_latched(edge_count_latched),
        .bucket(target_bucket)
    );

    //===========================================
    // 6. LED DIMMER (Fading & PWM)
    //===========================================
    led_dimmer dimmer_inst (
        .clk(clk),
        .reset(reset),
        .update_strobe(is_latching), // When high, update brightness
        .active_bucket(target_bucket),
        .led_out(led)
    );

endmodule