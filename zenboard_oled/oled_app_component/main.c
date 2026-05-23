#include "xparameters.h"
#include "xgpio.h"
#include "sleep.h"
#include "xuartps.h"

#define DC_PIN   0
#define RES_PIN  1
#define SCLK_PIN 2
#define SDIN_PIN 3
#define VBAT_PIN 4
#define VDD_PIN  5

XGpio Gpio;
u32 pin_state = 0;

// Standard 5x7 Font Table
const u8 FontLookup[128][5] = {

    [' '] = {0x00,0x00,0x00,0x00,0x00},

    ['A'] = {0x7E,0x11,0x11,0x11,0x7E},
    ['B'] = {0x7F,0x49,0x49,0x49,0x36},
    ['C'] = {0x3E,0x41,0x41,0x41,0x22},
    ['D'] = {0x7F,0x41,0x41,0x22,0x1C},
    ['E'] = {0x7F,0x49,0x49,0x49,0x41},
    ['F'] = {0x7F,0x09,0x09,0x09,0x01},
    ['G'] = {0x3E,0x41,0x49,0x49,0x7A},
    ['H'] = {0x7F,0x08,0x08,0x08,0x7F},
    ['I'] = {0x41,0x41,0x7F,0x41,0x41},
    ['J'] = {0x20,0x40,0x41,0x3F,0x01},
    ['K'] = {0x7F,0x08,0x14,0x22,0x41},
    ['L'] = {0x7F,0x40,0x40,0x40,0x40},
    ['M'] = {0x7F,0x02,0x0C,0x02,0x7F},
    ['N'] = {0x7F,0x04,0x08,0x10,0x7F},
    ['O'] = {0x3E,0x41,0x41,0x41,0x3E},
    ['P'] = {0x7F,0x09,0x09,0x09,0x06},
    ['Q'] = {0x3E,0x41,0x51,0x21,0x5E},
    ['R'] = {0x7F,0x09,0x19,0x29,0x46},
    ['S'] = {0x46,0x49,0x49,0x49,0x31},
    ['T'] = {0x01,0x01,0x7F,0x01,0x01},
    ['U'] = {0x3F,0x40,0x40,0x40,0x3F},
    ['V'] = {0x1F,0x20,0x40,0x20,0x1F},
    ['W'] = {0x7F,0x20,0x18,0x20,0x7F},
    ['X'] = {0x63,0x14,0x08,0x14,0x63},
    ['Y'] = {0x03,0x04,0x78,0x04,0x03},
    ['Z'] = {0x61,0x51,0x49,0x45,0x43}
};

void set_pin(int pin, int val)
{
    if(val)
        pin_state |= (1 << pin);
    else
        pin_state &= ~(1 << pin);

    XGpio_DiscreteWrite(&Gpio, 1, pin_state);
}

void spi_write(u8 data)
{
    for(int i = 0; i < 8; i++)
    {
        set_pin(SCLK_PIN, 0);

        for(volatile int d=0; d<4; d++);

        set_pin(SDIN_PIN, (data & 0x80) ? 1 : 0);

        data <<= 1;

        set_pin(SCLK_PIN, 1);

        for(volatile int d=0; d<4; d++);
    }
}

void write_cmd(u8 cmd)
{
    set_pin(DC_PIN, 0);
    spi_write(cmd);
}

void write_data(u8 data)
{
    set_pin(DC_PIN, 1);
    spi_write(data);
}

void oled_clear()
{
    for(u8 page = 0; page < 4; page++)
    {
        write_cmd(0xB0 + page);

        write_cmd(0x00);
        write_cmd(0x10);

        for(u16 col = 0; col < 128; col++)
        {
            write_data(0x00);
        }
    }
}

void oled_print_char(char c)
{
    // Convert lowercase to uppercase
    if(c >= 'a' && c <= 'z')
    {
        c = c - 32;
    }

    // Ignore unsupported chars
    if(c < 32 || c > 127)
    {
        return;
    }

    for(int i = 0; i < 5; i++)
    {
        write_data(FontLookup[(u8)c][i]);
    }

    // Space between letters
    write_data(0x00);
}

void oled_init()
{
    set_pin(VDD_PIN, 0);
    set_pin(VBAT_PIN, 0);
    set_pin(RES_PIN, 1);

    usleep(1000);

    set_pin(VDD_PIN, 1);

    usleep(10000);

    write_cmd(0xAE);

    set_pin(RES_PIN, 0);
    usleep(10000);

    set_pin(RES_PIN, 1);
    usleep(10000);

    write_cmd(0xD5);
    write_cmd(0x80);

    write_cmd(0xA8);
    write_cmd(0x1F);

    write_cmd(0xD3);
    write_cmd(0x00);

    write_cmd(0x40);

    write_cmd(0x8D);
    write_cmd(0x14);

    write_cmd(0x20);
    write_cmd(0x02);

    write_cmd(0xA1);
    write_cmd(0xC8);

    write_cmd(0xDA);
    write_cmd(0x02);

    write_cmd(0x81);
    write_cmd(0x8F);

    write_cmd(0xD9);
    write_cmd(0xF1);

    write_cmd(0xDB);
    write_cmd(0x40);

    write_cmd(0xA4);
    write_cmd(0xA6);

    set_pin(VBAT_PIN, 1);

    usleep(100000);

    write_cmd(0xAF);

    usleep(1000);
}

int main()
{
    XGpio_Initialize(&Gpio, XPAR_XGPIO_0_BASEADDR);

    XGpio_SetDataDirection(&Gpio, 1, 0x00);

    oled_init();

    oled_clear();

    // Set cursor to top-left
    write_cmd(0xB0);

    write_cmd(0x00);
    write_cmd(0x10);

    int current_column = 0;
    int current_page = 0;

    while(1)
    {
        // Check UART receive
        if(XUartPs_IsReceiveData(XPAR_XUARTPS_0_BASEADDR))
        {
            char input_char;

            input_char =
                XUartPs_ReadReg(
                    XPAR_XUARTPS_0_BASEADDR,
                    XUARTPS_FIFO_OFFSET
                );

            // Enter key clears screen
            if(input_char == '\r' || input_char == '\n')
            {
                oled_clear();

                write_cmd(0xB0);

                write_cmd(0x00);
                write_cmd(0x10);

                current_column = 0;
                current_page = 0;
            }
            else
            {
                // Next line
                if(current_column >= 120)
                {
                    current_column = 0;

                    current_page += 2;

                    if(current_page > 2)
                    {
                        oled_clear();

                        current_page = 0;
                    }

                    write_cmd(0xB0 + current_page);

                    write_cmd(0x00);
                    write_cmd(0x10);
                }

                oled_print_char(input_char);

                current_column += 6;
            }
        }
    }

    return 0;
}