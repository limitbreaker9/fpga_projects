#include <stdio.h>
#include "xil_printf.h"
#include "xil_io.h"
#include "sleep.h"

// ============================================================================
// 1. HARDWARE BASE ADDRESS & BITMAP REGISTER CONFIGURATIONS
// ============================================================================
#define OLED_BASE_ADDR      0x43C00000 

#define REG_CTRL            0  
#define REG_DATA            4  

#define BIT_VDD             (1 << 0)  
#define BIT_VBAT            (1 << 1)  
#define BIT_RES             (1 << 2)  
#define BIT_DC              (1 << 3)  
#define BIT_START           (1 << 4)  
#define BIT_BUSY            (1 << 31) 

// Global tracking variables
uint32_t current_ctrl_state = (BIT_VDD | BIT_VBAT); 
uint8_t current_page = 0;
uint8_t current_column = 0;

// External functions for Serial IO
extern char inbyte(void);
extern void outbyte(char c);

// ============================================================================
// 2. EMBEDDED STANDARD 5x8 ASCII FONT BITMAP TABLE
// ============================================================================
const uint8_t font_5x8[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x08, 0x2A, 0x1C, 0x2A, 0x08}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x00, 0x08, 0x14, 0x22, 0x41}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x41, 0x22, 0x14, 0x08, 0x00}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x01, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x3A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x03, 0x04, 0x78, 0x04, 0x03}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // Backslash
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // f
    {0x08, 0x14, 0x54, 0x54, 0x3C}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z
    {0x08, 0x6E, 0x41, 0x41, 0x00}, // {
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // |
    {0x00, 0x41, 0x41, 0x6E, 0x08}, // }
    {0x04, 0x02, 0x04, 0x08, 0x04}, // ~
    {0x7F, 0x41, 0x41, 0x41, 0x7F}  // Box
};

// ============================================================================
// 3. LOW LEVEL CONTROLLER FUNCTIONS
// ============================================================================
void OLED_WriteByte(uint8_t byte, uint8_t is_data) {
    while (Xil_In32(OLED_BASE_ADDR + REG_CTRL) & BIT_BUSY);
    Xil_Out32(OLED_BASE_ADDR + REG_DATA, (uint32_t)byte);

    if (is_data) current_ctrl_state |= BIT_DC;
    else         current_ctrl_state &= ~BIT_DC;

    current_ctrl_state &= ~BIT_START;
    Xil_Out32(OLED_BASE_ADDR + REG_CTRL, current_ctrl_state);
    
    Xil_Out32(OLED_BASE_ADDR + REG_CTRL, current_ctrl_state | BIT_START);
    Xil_Out32(OLED_BASE_ADDR + REG_CTRL, current_ctrl_state & ~BIT_START);

    while (Xil_In32(OLED_BASE_ADDR + REG_CTRL) & BIT_BUSY);
}

void OLED_FillScreen(uint8_t pattern) {
    OLED_WriteByte(0x21, 0); OLED_WriteByte(0x00, 0); OLED_WriteByte(0x7F, 0);
    OLED_WriteByte(0x22, 0); OLED_WriteByte(0x00, 0); OLED_WriteByte(0x03, 0);

    for(int i = 0; i < 512; i++) {
        OLED_WriteByte(pattern, 1);
    }
    current_page = 0;
    current_column = 0;
}

void OLED_Initialize(void) {
    current_ctrl_state = BIT_VDD | BIT_VBAT;
    Xil_Out32(OLED_BASE_ADDR + REG_CTRL, current_ctrl_state);
    usleep(20000);

    current_ctrl_state &= ~BIT_VDD;
    Xil_Out32(OLED_BASE_ADDR + REG_CTRL, current_ctrl_state);
    usleep(20000);

    current_ctrl_state |= BIT_RES;
    Xil_Out32(OLED_BASE_ADDR + REG_CTRL, current_ctrl_state);
    usleep(20000);

    OLED_WriteByte(0xAE, 0); // Display OFF
    OLED_WriteByte(0xD5, 0); OLED_WriteByte(0x80, 0); 
    OLED_WriteByte(0xA8, 0); OLED_WriteByte(0x1F, 0); 
    OLED_WriteByte(0xD3, 0); OLED_WriteByte(0x00, 0); 
    OLED_WriteByte(0x40, 0); 
    OLED_WriteByte(0x8D, 0); OLED_WriteByte(0x14, 0); 
    OLED_WriteByte(0x20, 0); OLED_WriteByte(0x00, 0); 
    OLED_WriteByte(0xA1, 0); 
    OLED_WriteByte(0xC8, 0); 
    OLED_WriteByte(0xDA, 0); OLED_WriteByte(0x02, 0); 
    OLED_WriteByte(0x81, 0); OLED_WriteByte(0x8F, 0); 
    OLED_WriteByte(0xD9, 0); OLED_WriteByte(0xF1, 0); 
    OLED_WriteByte(0xDB, 0); OLED_WriteByte(0x40, 0); 
    OLED_WriteByte(0xA4, 0); 
    OLED_WriteByte(0xA6, 0); 

    OLED_FillScreen(0x00); // Clear screen completely before turning on

    current_ctrl_state &= ~BIT_VBAT;
    Xil_Out32(OLED_BASE_ADDR + REG_CTRL, current_ctrl_state);
    usleep(100000);

    OLED_WriteByte(0xAF, 0); // Display ON
}

// ============================================================================
// 4. TEXT STREAM PROCESSING
// ============================================================================
void OLED_SetCursor(uint8_t page, uint8_t column) {
    if (page > 3) page = 0;
    if (column > 127) column = 0;
    
    current_page = page;
    current_column = column;

    OLED_WriteByte(0x21, 0);
    OLED_WriteByte(column, 0); 
    OLED_WriteByte(0x7F, 0);   

    OLED_WriteByte(0x22, 0);
    OLED_WriteByte(page, 0);   
    OLED_WriteByte(0x03, 0);   
}

void OLED_PrintChar(char c) {
    if (c == '\r' || c == '\n') {
        OLED_SetCursor((current_page + 1) % 4, 0);
        return;
    }

    if (current_column + 6 >= 128) {
        OLED_SetCursor((current_page + 1) % 4, 0);
    }

    uint8_t font_index = 0;
    if (c >= 0x20 && c <= 0x7E) {
        font_index = c - 0x20;
    } else {
        font_index = 0x7F - 0x20; 
    }

    for (int col = 0; col < 5; col++) {
        OLED_WriteByte(font_5x8[font_index][col], 1);
    }
    
    OLED_WriteByte(0x00, 1); 
    current_column += 6;
}

void OLED_PrintString(const char *str) {
    while (*str) {
        OLED_PrintChar(*str++);
    }
}

// ============================================================================
// 5. CORE EXECUTION ENTRIES (MENU SYSTEM)
// ============================================================================
int main() {
    char choice;
    char text_buffer;

    xil_printf("\n\r=== ZedBoard OLED Initialization ===\n\r");
    OLED_Initialize();
    
    // Print a quick welcome message on the OLED itself
    OLED_SetCursor(0, 0);
    OLED_PrintString("SYSTEM ONLINE!");
    OLED_SetCursor(1, 0);
    OLED_PrintString("READY FOR INPUT.");
    
    while(1) {
        xil_printf("\n\r\n\r=== OLED CONTROL MENU ===\n\r");
        xil_printf("1. Clear Screen (Black)\n\r");
        xil_printf("2. Fill Screen (Solid White)\n\r");
        xil_printf("3. Fill Screen (Checkerboard)\n\r");
        xil_printf("4. Type Text onto Screen\n\r");
        xil_printf("Enter choice (1-4): ");

        choice = inbyte();
        outbyte(choice); // Echo the choice back
        xil_printf("\n\r");

        switch(choice) {
            case '1':
                xil_printf("Clearing screen...\n\r");
                OLED_FillScreen(0x00);
                break;
            case '2':
                xil_printf("Filling screen...\n\r");
                OLED_FillScreen(0xFF);
                break;
            case '3':
                xil_printf("Drawing checkerboard...\n\r");
                OLED_FillScreen(0xAA);
                break;
            case '4':
                xil_printf("Type your message. Press ENTER to exit typing mode.\n\r> ");
                OLED_FillScreen(0x00); // Clear the screen first
                OLED_SetCursor(0, 0);
                
                while(1) {
                    text_buffer = inbyte();
                    if (text_buffer == '\r' || text_buffer == '\n') {
                        break; // Exit typing mode when Enter is pressed
                    }
                    outbyte(text_buffer); // Echo character to serial monitor
                    OLED_PrintChar(text_buffer); // Draw character on OLED
                }
                break;
            default:
                xil_printf("Invalid choice. Please press 1, 2, 3, or 4.\n\r");
                break;
        }
    }

    return 0;
}