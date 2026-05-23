#include <stdio.h>
#include "xil_printf.h"
#include "xparameters.h"
#include "sleep.h"
#include "PmodOLED.h"

// Fallback logic for platform naming conventions
#ifndef XPAR_PMODOLED_0_AXI_LITE_GPIO_BASEADDR
#define XPAR_PMODOLED_0_AXI_LITE_GPIO_BASEADDR XPAR_PMODOLED_0_BASEADDR
#endif
#ifndef XPAR_PMODOLED_0_AXI_LITE_SPI_BASEADDR
#define XPAR_PMODOLED_0_AXI_LITE_SPI_BASEADDR (XPAR_PMODOLED_0_BASEADDR + 0x10000)
#endif

// External UART functions provided by Xilinx standalone BSP
extern char inbyte(void);
extern void outbyte(char c);

PmodOLED myDevice;

int main() {
    sleep(1);

    // Initialize OLED hardware
    OLED_Begin(&myDevice, 
               XPAR_PMODOLED_0_AXI_LITE_GPIO_BASEADDR, 
               XPAR_PMODOLED_0_AXI_LITE_SPI_BASEADDR, 
               0, 
               0);  
    
    OLED_Clear(&myDevice);
    OLED_SetCursor(&myDevice, 0, 0);
    OLED_PutString(&myDevice, "UART Terminal Mode");
    OLED_SetCursor(&myDevice, 0, 1);
    OLED_PutString(&myDevice, "Awaiting Input...");

    // Welcome message on the PC Serial Monitor
    xil_printf("\r\n====================================================\r\n");
    xil_printf("             ZedBoard UART -> OLED Terminal        \r\n");
    xil_printf("====================================================\r\n");
    xil_printf("Instructions: Type a message (up to 16 chars) and press ENTER.\r\n\r\n");
    xil_printf("Type here > ");

    char inputBuffer[17]; // 16 characters maximum per OLED line + 1 null terminator
    int charCount = 0;

    while(1) {
        // Read a single character from the PC serial terminal (blocking wait)
        char typedChar = inbyte();

        // Check if user pressed ENTER (Carriage Return or Newline)
        if (typedChar == '\r' || typedChar == '\n') {
            inputBuffer[charCount] = '\0'; // Null-terminate the string

            // Clear OLED and print the new string
            OLED_Clear(&myDevice);
            OLED_SetCursor(&myDevice, 0, 0);
            OLED_PutString(&myDevice, "OLED Received:");
            OLED_SetCursor(&myDevice, 0, 2);
            OLED_PutString(&myDevice, inputBuffer);

            // Reset terminal prompt on PC
            xil_printf("\r\nSent to OLED: \"%s\"\r\n", inputBuffer);
            xil_printf("\r\nType here > ");
            
            charCount = 0; // Reset buffer index for next message
        }
        // Check for Backspace (so you can fix typos!)
        else if ((typedChar == '\b' || typedChar == 127) && charCount > 0) {
            charCount--;
            // Erase character from PC terminal screen visually
            outbyte('\b');
            outbyte(' ');
            outbyte('\b');
        }
        // Normal characters (only accept up to 16 characters to prevent line clipping)
        else if (charCount < 16 && typedChar >= 32 && typedChar <= 126) {
            inputBuffer[charCount] = typedChar;
            charCount++;
            outbyte(typedChar); // Echo character back to PC screen
        }
    }

    return 0;
}