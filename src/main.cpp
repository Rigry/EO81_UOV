#define STM32F103xB
#define F_OSC   8000000UL
#define F_CPU   72000000UL

#define HEAP_SIZE 1024

#include "init_clock.h"
#include "constant.h"
#include "timers.h"
#include "literals.h"
#include "menu.h"
#include "button.h"
#include "modbus_slave.h"
#include "modbus_master.h"
#include "hysteresis.h"
#include "utils.h"
#include "flash.h"
#include "lamps.h"


/// эта функция вызывается первой в startup файле
extern "C" void init_clock() { init_clock<F_OSC, F_CPU>(); }

using TX_master  = mcu::PC10;   using TX_slave  = mcu::PA9;
using RX_master  = mcu::PC11;   using RX_slave  = mcu::PA10;
using RTS_master = mcu::PA15;   using RTS_slave = mcu::PA8;
using LED_master = mcu::PC9;    using LED_slave = mcu::PA12;

using E       = mcu::PB3;
using RW      = mcu::PD2;
using RS      = mcu::PC12;
using DB4     = mcu::PB4;
using DB5     = mcu::PB5;
using DB6     = mcu::PB6;
using DB7     = mcu::PB7;
using Up      = mcu::PB8;
using Down    = mcu::PB9;
using US_BTN  = mcu::PB0;
using UV_BTN  = mcu::PB1;
using UV_CTRL = mcu::PB10;
using US_CTRL = mcu::PB11;

using LED1 = mcu::PB12;
using LED2 = mcu::PC6;
using LED3 = mcu::PC7;

using EPRA1  = mcu::PA0;
using EPRA2  = mcu::PA1;
using EPRA3  = mcu::PA2;
using EPRA4  = mcu::PA3;
using EPRA5  = mcu::PA4;
using EPRA6  = mcu::PA5;
using EPRA7  = mcu::PA6;
using EPRA8  = mcu::PA7;
using EPRA9  = mcu::PC4;
using EPRA10 = mcu::PC5;



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
        uint8_t  max_temperature       = 55;
        uint8_t  uv_level_min          = 40;
        Quantity quantity = {
            .lamps = 5,
            .extantions = 0
        };
        uint16_t uv_level_highest      = 0x0100;
        uint8_t  model_number          = 0;
        uint8_t  temperature_recovery  = 20;
        Exsist exist = {
            .board_sensor = false,
            .temp_sensor  = false,
            .uv_sensor    = false
        };
        Count count = {
            .on        = 0,
            .reset_all = 0,
            .reset_one = 0,
            .reset_log = 0
        };
    } flash;

    [[maybe_unused]] auto _ = Flash_updater<
          mcu::FLASH::Sector::_127
        , mcu::FLASH::Sector::_126
    >::make (&flash);

    struct In_regs {
        UART::Settings uart_set;         // 0
        uint16_t modbus_address;         // 1
        uint16_t password;               // 2
        uint16_t factory_number;         // 3
        uint16_t max_temperature;        // 4
        uint16_t uv_level_min;           // 5
        uint16_t qty_uv_lamps;           // 6 // FIX убрать для редактирования
        uint16_t uv_level_highest;       // 7
        std::array<uint16_t, glob::max_lamps> hours;  // 8
    }__attribute__((packed));

    struct Out_regs {
        uint16_t       device_code;        // 0
        uint16_t       factory_number;     // 1
        UART::Settings uart_set;           // 2
        uint16_t       modbus_address;     // 3
        Flags          work_flags;         // 4
        uint16_t       temperature;        // 5
        uint16_t       max_temperature;    // 6
        uint16_t       uv_level;           // 7
        uint16_t       uv_level_min;       // 8
        Quantity       quantity;           // 9
        uint16_t       uv_level_highest;   // 10
        std::array<uint16_t, glob::max_extantions+1> bad_lamps; // 11
        std::array<uint16_t, glob::max_lamps> hours;       // 22
    }; // __attribute__((packed)); // TODO error: cannot bind packed field 

    // оптимизировал, неудобно отлаживать, потому volatile
    volatile decltype(auto) modbus_slave = Modbus_slave<In_regs, Out_regs>::make <
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
    modbus_slave.outRegs.quantity          = flash.quantity;
    modbus_slave.arInRegsMax[ADR(uart_set)]= 0b11111111;
    modbus_slave.inRegsMin.modbus_address  = 1;
    modbus_slave.inRegsMax.modbus_address  = 255;
    modbus_slave.inRegsMin.qty_uv_lamps    = 1;
    modbus_slave.inRegsMax.qty_uv_lamps    = 10;

    struct {
        // TODO пока без модулей расширения
        // Register<1,  Modbus_function::read_03, 0> qty_lamps_1;
        // Register<2,  Modbus_function::read_03, 0> qty_lamps_2;
        // Register<3,  Modbus_function::read_03, 0> qty_lamps_3;
        // Register<4,  Modbus_function::read_03, 0> qty_lamps_4;
        // Register<5,  Modbus_function::read_03, 0> qty_lamps_5;
        // Register<6,  Modbus_function::read_03, 0> qty_lamps_6;
        // Register<7,  Modbus_function::read_03, 0> qty_lamps_7;
        // Register<8,  Modbus_function::read_03, 0> qty_lamps_8;
        // Register<9,  Modbus_function::read_03, 0> qty_lamps_9;
        // Register<10, Modbus_function::read_03, 0> qty_lamps_10;
        // Register<11, Modbus_function::read_03, 0> qty_lamps_11;
        // Register<12, Modbus_function::read_03, 0> qty_lamps_12;
        // Register<13, Modbus_function::read_03, 0> qty_lamps_13;
        // Register<14, Modbus_function::read_03, 0> qty_lamps_14;
        // Register<15, Modbus_function::read_03, 0> qty_lamps_15;

        // Register<1,  Modbus_function::read_03, 1, Bit_set<10>> bad_lamps_1;
        // Register<2,  Modbus_function::read_03, 1, Bit_set<10>> bad_lamps_2;
        // Register<3,  Modbus_function::read_03, 1, Bit_set<10>> bad_lamps_3;
        // Register<4,  Modbus_function::read_03, 1, Bit_set<10>> bad_lamps_4;
        // Register<5,  Modbus_function::read_03, 1, Bit_set<10>> bad_lamps_5;
        // Register<6,  Modbus_function::read_03, 1, Bit_set<10>> bad_lamps_6;
        // Register<7,  Modbus_function::read_03, 1, Bit_set<10>> bad_lamps_7;
        // Register<8,  Modbus_function::read_03, 1, Bit_set<10>> bad_lamps_8;
        // Register<9,  Modbus_function::read_03, 1, Bit_set<10>> bad_lamps_9;
        // Register<10, Modbus_function::read_03, 1, Bit_set<10>> bad_lamps_10;
        // Register<11, Modbus_function::read_03, 1, Bit_set<10>> bad_lamps_11;
        // // Register<12, Modbus_function::read_03, 1> bad_lamps_12;
        // Register<13, Modbus_function::read_03, 1> bad_lamps_13;
        // Register<14, Modbus_function::read_03, 1> bad_lamps_14;
        // Register<15, Modbus_function::read_03, 1> bad_lamps_15;
        
        // FIX 12 временно, при отладке была плата с таким адресом
        Register<12, Modbus_function::read_03, 0> uv_level;
        Register<12, Modbus_function::read_03, 1> temperature;
    } modbus_master_regs;

    decltype(auto) modbus_master = make_modbus_master <
          mcu::Periph::USART3
        , TX_master
        , RX_master
        , RTS_master
    > (100_ms, flash.uart_set, modbus_master_regs); // FIX flash.uart_set placeholder

    auto up    = Button<Up>();
    auto down  = Button<Down>();
    auto enter = Tied_buttons(up, down);
    constexpr auto hd44780_pins = HD44780_pins<RS, RW, E, DB4, DB5, DB6, DB7>{};
    [[maybe_unused]] auto menu = Menu(hd44780_pins, up, down, enter, flash, modbus_slave.outRegs);


    // алиасы
    auto& temperature = (uint16_t&)modbus_master_regs.temperature;
    auto& uv_level    = (uint16_t&)modbus_master_regs.uv_level;
    auto& uv_level_percent = modbus_slave.outRegs.uv_level;
    auto& work_flags = modbus_slave.outRegs.work_flags;


    auto& alarm_led = Pin::make<LED3, mcu::PinMode::Output>();

    auto overheat = Hysteresis(temperature, flash.temperature_recovery, flash.max_temperature);

    // UV control
    // TODO уточнить логику у Олега
    auto& uv     = Pin::make<UV_CTRL, mcu::PinMode::Output>();
    auto& uv_led = Pin::make<LED1   , mcu::PinMode::Output>();
    auto  uv_button = Button<UV_BTN>();

    auto on_uv = [&](bool on = true){
        flash.count.on += int(on and not uv);
        uv = uv_led = work_flags.uv_on = on;
    };

    uv_button.set_down_callback([&]{
        if (overheat) {
            work_flags.uv_started = false;
            return;
        }
        work_flags.uv_started ^= 1;
    });

    // US control
    // TODO уточнить логику у Олега
    auto& us     = Pin::make<US_CTRL, mcu::PinMode::Output>();
    auto& us_led = Pin::make<LED2   , mcu::PinMode::Output>();
    auto  us_button = Button<US_BTN>();

    auto on_us = [&](bool on = true){
        us = us_led = work_flags.us_on = on;
    };

    us_button.set_down_callback([&]{
        if (overheat) {
            work_flags.us_started = false;
            return;
        }
        work_flags.us_started ^= 1;
    });

    // Определение плохих ламп
    Lamps::make<
        EPRA1,EPRA2,EPRA3,EPRA4,EPRA5,EPRA6,EPRA7,EPRA8,EPRA9,EPRA10
    >(modbus_slave.outRegs.bad_lamps[0], flash.quantity.lamps);


    while (1) {
        modbus_master();
        modbus_slave([](auto){});

        modbus_slave.outRegs.temperature = temperature;

        work_flags.overheat = overheat;

        set_if_greater (flash.uv_level_highest, uv_level);
        uv_level_percent = uv_level * 100 / flash.uv_level_highest;
        work_flags.uv_low_level = uv_level_percent < flash.uv_level_min;
        
        on_us (work_flags.us_started and not overheat);
        on_uv (work_flags.uv_started and not overheat);

        // TODO add other alarms уточнить у Олега
        alarm_led = work_flags.overheat or work_flags.uv_low_level;

        __WFI();
    }
}
