`timescale 1 ns / 1 ps

module myoledctrl_slave_lite_v1_0_S00_AXI #
(
    parameter integer C_S_AXI_DATA_WIDTH = 32,
    parameter integer C_S_AXI_ADDR_WIDTH = 4
)
(
    // Users to add ports here
    output wire oled_sclk,
    output wire oled_sdin,
    output wire oled_dc,
    output wire oled_res,
    output wire oled_vbat,
    output wire oled_vdd,
    // User ports ends

    input wire  S_AXI_ACLK,
    input wire  S_AXI_ARESETN,
    input wire [C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_AWADDR,
    input wire [2 : 0] S_AXI_AWPROT,
    input wire  S_AXI_AWVALID,
    output wire  S_AXI_AWREADY,
    input wire [C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_WDATA,
    input wire [(C_S_AXI_DATA_WIDTH/8)-1 : 0] S_AXI_WSTRB,
    input wire  S_AXI_WVALID,
    output wire  S_AXI_WREADY,
    output wire [1 : 0] S_AXI_BRESP,
    output wire  S_AXI_BVALID,
    input wire  S_AXI_BREADY,
    input wire [C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_ARADDR,
    input wire [2 : 0] S_AXI_ARPROT,
    input wire  S_AXI_ARVALID,
    output wire  S_AXI_ARREADY,
    output wire [C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_RDATA,
    output wire [1 : 0] S_AXI_RRESP,
    output wire  S_AXI_RVALID,
    input wire  S_AXI_RREADY
);

// ============================================================
// AXI4-Lite signals
// ============================================================
reg [C_S_AXI_ADDR_WIDTH-1 : 0] axi_awaddr;
reg  axi_awready;
reg  axi_wready;
reg [1 : 0] axi_bresp;
reg  axi_bvalid;
reg [C_S_AXI_ADDR_WIDTH-1 : 0] axi_araddr;
reg  axi_arready;
reg [1 : 0] axi_rresp;
reg  axi_rvalid;

localparam integer ADDR_LSB          = (C_S_AXI_DATA_WIDTH/32) + 1;
localparam integer OPT_MEM_ADDR_BITS = 1;

reg [C_S_AXI_DATA_WIDTH-1:0] slv_reg0;
reg [C_S_AXI_DATA_WIDTH-1:0] slv_reg1;
reg [C_S_AXI_DATA_WIDTH-1:0] slv_reg2;
reg [C_S_AXI_DATA_WIDTH-1:0] slv_reg3;
integer byte_index;

assign S_AXI_AWREADY = axi_awready;
assign S_AXI_WREADY  = axi_wready;
assign S_AXI_BRESP   = axi_bresp;
assign S_AXI_BVALID  = axi_bvalid;
assign S_AXI_ARREADY = axi_arready;
assign S_AXI_RRESP   = axi_rresp;
assign S_AXI_RVALID  = axi_rvalid;

// ============================================================
// AXI Write State Machine
// ============================================================
reg [1:0] state_write;
reg [1:0] state_read;

localparam AXI_IDLE  = 2'b00;
localparam AXI_WADDR = 2'b10;
localparam AXI_WDATA = 2'b11;
localparam AXI_RADDR = 2'b10;
localparam AXI_RDATA = 2'b11;

always @(posedge S_AXI_ACLK) begin
    if (S_AXI_ARESETN == 1'b0) begin
        axi_awready  <= 0;
        axi_wready   <= 0;
        axi_bvalid   <= 0;
        axi_bresp    <= 0;
        axi_awaddr   <= 0;
        state_write  <= AXI_IDLE;
    end else begin
        case(state_write)
            AXI_IDLE: begin
                axi_awready <= 1'b1;
                axi_wready  <= 1'b1;
                state_write <= AXI_WADDR;
            end
            AXI_WADDR: begin
                if (S_AXI_AWVALID && S_AXI_AWREADY) begin
                    axi_awaddr <= S_AXI_AWADDR;
                    if (S_AXI_WVALID) begin
                        axi_awready <= 1'b1;
                        state_write <= AXI_WADDR;
                        axi_bvalid  <= 1'b1;
                    end else begin
                        axi_awready <= 1'b0;
                        state_write <= AXI_WDATA;
                        if (S_AXI_BREADY && axi_bvalid) axi_bvalid <= 1'b0;
                    end
                end else begin
                    if (S_AXI_BREADY && axi_bvalid) axi_bvalid <= 1'b0;
                end
            end
            AXI_WDATA: begin
                if (S_AXI_WVALID) begin
                    state_write <= AXI_WADDR;
                    axi_bvalid  <= 1'b1;
                    axi_awready <= 1'b1;
                end else begin
                    if (S_AXI_BREADY && axi_bvalid) axi_bvalid <= 1'b0;
                end
            end
            default: state_write <= AXI_IDLE;
        endcase
    end
end

// ============================================================
// AXI Register Write
// ============================================================
always @(posedge S_AXI_ACLK) begin
    if (S_AXI_ARESETN == 1'b0) begin
        slv_reg0 <= 0;
        slv_reg1 <= 0;
        slv_reg2 <= 0;
        slv_reg3 <= 0;
    end else begin
        if (S_AXI_WVALID) begin
            case ((S_AXI_AWVALID) ?
                   S_AXI_AWADDR[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] :
                   axi_awaddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB])
                2'h0: for (byte_index=0; byte_index<=(C_S_AXI_DATA_WIDTH/8)-1; byte_index=byte_index+1)
                          if (S_AXI_WSTRB[byte_index])
                              slv_reg0[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                2'h1: for (byte_index=0; byte_index<=(C_S_AXI_DATA_WIDTH/8)-1; byte_index=byte_index+1)
                          if (S_AXI_WSTRB[byte_index])
                              slv_reg1[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                2'h2: for (byte_index=0; byte_index<=(C_S_AXI_DATA_WIDTH/8)-1; byte_index=byte_index+1)
                          if (S_AXI_WSTRB[byte_index])
                              slv_reg2[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                2'h3: for (byte_index=0; byte_index<=(C_S_AXI_DATA_WIDTH/8)-1; byte_index=byte_index+1)
                          if (S_AXI_WSTRB[byte_index])
                              slv_reg3[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                default: begin
                    slv_reg0 <= slv_reg0;
                    slv_reg1 <= slv_reg1;
                    slv_reg2 <= slv_reg2;
                    slv_reg3 <= slv_reg3;
                end
            endcase
        end
    end
end

// ============================================================
// AXI Read State Machine
// ============================================================
always @(posedge S_AXI_ACLK) begin
    if (S_AXI_ARESETN == 1'b0) begin
        axi_arready <= 1'b0;
        axi_rvalid  <= 1'b0;
        axi_rresp   <= 1'b0;
        state_read  <= AXI_IDLE;
    end else begin
        case(state_read)
            AXI_IDLE: begin
                state_read  <= AXI_RADDR;
                axi_arready <= 1'b1;
            end
            AXI_RADDR: begin
                if (S_AXI_ARVALID && S_AXI_ARREADY) begin
                    state_read  <= AXI_RDATA;
                    axi_araddr  <= S_AXI_ARADDR;
                    axi_rvalid  <= 1'b1;
                    axi_arready <= 1'b0;
                end
            end
            AXI_RDATA: begin
                if (S_AXI_RVALID && S_AXI_RREADY) begin
                    axi_rvalid  <= 1'b0;
                    axi_arready <= 1'b1;
                    state_read  <= AXI_RADDR;
                end
            end
            default: state_read <= AXI_IDLE;
        endcase
    end
end

// ============================================================
// AXI Read Data — busy_reg injected into bit 31 of reg0
// ============================================================
assign S_AXI_RDATA =
    (axi_araddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] == 2'h0) ? {busy_reg, slv_reg0[30:0]} :
    (axi_araddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] == 2'h1) ? slv_reg1 :
    (axi_araddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] == 2'h2) ? slv_reg2 :
    (axi_araddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] == 2'h3) ? slv_reg3 : 32'd0;

// ============================================================
// USER LOGIC — OLED SPI Controller
// ============================================================
//
// slv_reg0 bit map:
//   [0] = VDD  (active LOW: write 0 = ON,  write 1 = OFF)
//   [1] = VBAT (active LOW: write 0 = ON,  write 1 = OFF)
//   [2] = RES  (active HIGH: write 1 = normal, write 0 = reset)
//   [3] = DC   (0 = command, 1 = data)
//   [4] = START (pulse high to begin one SPI byte transfer)
// slv_reg1[7:0] = byte to transmit (MSB first — D7 sent first)
// slv_reg0[31]  = busy_reg (read-only, injected into RDATA above)
//
// SSD1306 SPI timing (from datasheet):
//   Clock idle LOW (CPOL=0)
//   Data sampled on RISING edge of SCLK (CPHA=0)  <-- SPI MODE 0
//   Data must be stable BEFORE the rising edge
//   So: data is SET on falling edge, sampled by SSD1306 on rising edge
// ============================================================

assign oled_vdd  = slv_reg0[0];
assign oled_vbat = slv_reg0[1];
assign oled_res  = slv_reg0[2];
assign oled_dc   = slv_reg0[3];

wire        start_transfer = slv_reg0[4];
wire [7:0]  tx_data        = slv_reg1[7:0];

// ============================================================
// EDGE DETECTOR FOR START BIT
// ============================================================
reg start_d1;
reg start_d2;
wire start_pulse;

always @(posedge S_AXI_ACLK) begin
    if (S_AXI_ARESETN == 1'b0) begin
        start_d1 <= 1'b0;
        start_d2 <= 1'b0;
    end else begin
        start_d1 <= start_transfer;
        start_d2 <= start_d1;
    end
end

// Pulse goes high for exactly 1 clock cycle when start_transfer changes 0 -> 1
assign start_pulse = start_d1 & ~start_d2;
// ============================================================

reg [7:0]  shift_reg;  // MSB is always on the wire (oled_sdin = shift_reg[7])
reg [3:0]  bit_cnt;    // counts down from 8 to 0
reg [1:0]  spi_state;
reg        sclk_reg;
reg        busy_reg;

// ---- Clock divider ----
// 100 MHz / 32 = 3.125 MHz SPI clock
// Each spi_tick is one half-period toggle point
reg [4:0] clk_div;
always @(posedge S_AXI_ACLK) begin
    if (S_AXI_ARESETN == 1'b0) clk_div <= 5'd0;
    else                       clk_div <= clk_div + 1'b1;
end
wire spi_tick = (clk_div == 5'd0);

localparam S_IDLE  = 2'd0;
localparam S_SHIFT = 2'd1;
localparam S_DONE  = 2'd2;

// ---- SPI state machine ----
// SPI Mode 0: CPOL=0 (clock idles LOW), CPHA=0 (data sampled on rising edge)
//
// Sequence per bit:
//   1. SCLK=0, SDIN = next bit (MSB of shift_reg) — data set up
//   2. SCLK=1 — SSD1306 samples SDIN on this rising edge
//   3. Shift register rotates left (next bit ready)
//   4. SCLK=0 — back to idle, repeat for next bit
//
// So within S_SHIFT:
//   - On rising edge of spi_tick when sclk_reg==0: raise SCLK (SSD1306 samples current bit)
//   - On rising edge of spi_tick when sclk_reg==1: lower SCLK, shift data, decrement counter

always @(posedge S_AXI_ACLK) begin
    if (S_AXI_ARESETN == 1'b0) begin
        spi_state <= S_IDLE;
        sclk_reg  <= 1'b0;
        busy_reg  <= 1'b0;
        bit_cnt   <= 4'd0;
        shift_reg <= 8'd0;
    end else begin
        case (spi_state)

            S_IDLE: begin
                sclk_reg <= 1'b0;
                busy_reg <= 1'b0;
                // CHANGED: Use start_pulse instead of start_transfer
                if (start_pulse == 1'b1) begin
                    shift_reg <= tx_data;  // load MSB first
                    bit_cnt   <= 4'd8;     // 8 bits to send
                    busy_reg  <= 1'b1;
                    spi_state <= S_SHIFT;
                end
            end

            S_SHIFT: begin
                if (spi_tick) begin
                    if (sclk_reg == 1'b0) begin
                        // SCLK going HIGH — SSD1306 samples shift_reg[7] on this edge
                        sclk_reg <= 1'b1;
                    end else begin
                        // SCLK going LOW — shift data, decrement counter
                        sclk_reg  <= 1'b0;
                        shift_reg <= {shift_reg[6:0], 1'b0}; // shift left, next bit to [7]
                        bit_cnt   <= bit_cnt - 1'b1;
                        if (bit_cnt == 4'd1) begin
                            // This was the last bit
                            spi_state <= S_DONE;
                        end
                    end
                end
            end

            S_DONE: begin
                // Clock is already low. Wait one tick then release busy.
                if (spi_tick) begin
                    sclk_reg  <= 1'b0;
                    busy_reg  <= 1'b0;
                    spi_state <= S_IDLE;
                end
            end

            default: spi_state <= S_IDLE;

        endcase
    end
end

// SDIN always drives the MSB of shift_reg
// Data is stable before the rising edge because we set it on the falling edge
assign oled_sclk = sclk_reg;
assign oled_sdin = shift_reg[7];

// ============================================================
// End user logic
// ============================================================

endmodule