module fsm_control (
    input  logic clk,
    input  logic reset,
    output logic is_collecting,
    output logic is_latching
);
    // 100ms window at 48 MHz
    localparam [25:0] WINDOW_CYCLES = 26'd4_800_000;

    typedef enum logic [1:0] { COLLECTING, LATCHING } state_t;
    state_t state;
    logic [25:0] timer;

    assign is_collecting = (state == COLLECTING);
    assign is_latching   = (state == LATCHING);

    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            state <= COLLECTING;
            timer <= 26'd0;
        end else begin
            case (state)
                COLLECTING: begin
                    if (timer >= WINDOW_CYCLES - 1) state <= LATCHING;
                    else timer <= timer + 26'd1;
                end
                LATCHING: begin
                    timer <= 26'd0;
                    state <= COLLECTING;
                end
                default: state <= COLLECTING;
            endcase
        end
    end
endmodule