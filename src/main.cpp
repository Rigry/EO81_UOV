#define STM32F103xB
#define F_OSC   8000000UL
#define F_CPU   72000000UL

#define HEAP_SIZE 1024

#include "init_clock.h"
#include "timers.h"
#include "literals.h"
#include "menu.h"
#include "button.h"
#include "uart.h"


/// эта функция вызывается первой в startup файле
extern "C" void init_clock() { init_clock<F_OSC, F_CPU>(); }

using E     = mcu::PB3;       
using RW    = mcu::PD2;       
using RS    = mcu::PC12;      
using DB4   = mcu::PB4;       
using DB5   = mcu::PB5;
using DB6   = mcu::PB6;    
using DB7   = mcu::PB7;
using Up    = mcu::PB8;
using Down  = mcu::PB9;
using US    = mcu::PB0; 

struct Flash_data {
    uint16_t factory_number = 0;
    UART::Settings uart_set = {
        .parity_enable  = false,
        .parity         = USART::Parity::even,
        .data_bits      = USART::DataBits::_8,
        .stop_bits      = USART::StopBits::_1,
        .baudrate       = USART::Baudrate::BR9600,
        .res            = 0
    };
    uint8_t  modbus_address        = 1;
    uint16_t max_temperature       = 55;
    int uv_level_min               = 40;  
    uint16_t qty_uv_lamps          = 5;
    uint16_t uv_level_highest      = 0x0100;
    int      model_number          = 0;
    uint16_t temperature_recovery  = 20; 
} flash;


int main()
{
    auto up    = Button<Up>();
    auto down  = Button<Down>();
    auto enter = Tied_buttons(up, down);
    constexpr auto hd44780_pins = HD44780_pins<RS, RW, E, DB4, DB5, DB6, DB7>{};
    [[maybe_unused]] auto _ = Menu(hd44780_pins, up, down, enter, flash);

    while (1) {
        __WFI();
    }

}
