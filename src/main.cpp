#define STM32F103xB
#define F_OSC   8000000UL
#define F_CPU   72000000UL

#define HEAP_SIZE 1024

#include "init_clock.h"
#include "timers.h"
#include "literals.h"
#include "menu.h"
#include "button.h"
#include "modbus_slave.h"


/// эта функция вызывается первой в startup файле
extern "C" void init_clock() { init_clock<F_OSC, F_CPU>(); }

using TX_master  = mcu::PC10;   using TX_slave  = mcu::PA9;
using RX_master  = mcu::PC11;   using RX_slave  = mcu::PA10;
using RTS_master = mcu::PA15;   using RTS_slave = mcu::PA8;
using LED_master = mcu::PC9;    using LED_slave = mcu::PA12;

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

int main()
{
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
        int      uv_level_min          = 40;  
        uint16_t qty_uv_lamps          = 5;
        uint16_t uv_level_highest      = 0x0100;
        int      model_number          = 0;
        uint16_t temperature_recovery  = 20;
    } flash; // FIX (placeholder)

    struct In_regs {
        UART::Settings uart_set;         // 0
        uint16_t modbus_address;         // 1
        uint16_t password;               // 2
        uint16_t factory_number;         // 3
        uint16_t max_temperature;        // 4
        uint16_t uv_level_min;           // 5
        uint16_t qty_uv_lamps;           // 6
        uint16_t uv_level_highest;       // 7
        std::array<uint16_t, 112> hours; // 8
    }__attribute__((packed));


    struct Out_regs {
        uint16_t       device_code;        // 0
        uint16_t       factory_number;     // 1
        UART::Settings uart_set;           // 2
        uint16_t       modbus_address;     // 3
        uint16_t       work_flags;         // 4
        uint16_t       temperature;        // 5
        uint16_t       max_temperature;    // 6
        uint16_t       uv_level;           // 7
        uint16_t       uv_level_min;       // 8
        uint16_t       qty_uv_lamps;       // 9
        uint16_t       uv_level_highest;   // 10
        Bit_set<112>   lamp;               // 11
        std::array<uint16_t, 112> hours;   // 18
    }; // __attribute__((packed)); // TODO error: cannot bind packed field 

    decltype(auto) modbus_slave = Modbus_slave<In_regs, Out_regs>::make <
          mcu::Periph::USART1
        , TX_slave
        , RX_slave
        , RTS_slave
    >(flash.modbus_address, flash.uart_set);

    #define ADR(reg) GET_ADR(In_regs, reg)
    modbus_slave.outRegs.device_code       = 8; 
    modbus_slave.outRegs.factory_number    = flash.factory_number;
    modbus_slave.outRegs.uart_set          = flash.uart_set;
    modbus_slave.outRegs.modbus_address    = flash.modbus_address;
    modbus_slave.arInRegsMax[ADR(uart_set)]= 0b11111111;
    modbus_slave.inRegsMin.modbus_address  = 1;
    modbus_slave.inRegsMax.modbus_address  = 255;
    modbus_slave.inRegsMin.qty_uv_lamps    = 0;
    modbus_slave.inRegsMax.qty_uv_lamps    = 10;

    auto up    = Button<Up>();
    auto down  = Button<Down>();
    auto enter = Tied_buttons(up, down);
    constexpr auto hd44780_pins = HD44780_pins<RS, RW, E, DB4, DB5, DB6, DB7>{};
    [[maybe_unused]] auto _ = Menu(hd44780_pins, up, down, enter, flash, modbus_slave.outRegs);

    while (1) {
        __WFI();
    }

}
