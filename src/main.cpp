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
#include "safe_flash.h"
#include "lamps.h"
// #include "work_count.h"
#include <array>

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
        UART::Settings uart_set_master = {
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
            .lamps = 9,
            .extantions = 0
        };
        uint16_t uv_level_highest      = 0x0100;
        uint8_t  model_number          = 0;
        uint8_t  temperature_recovery  = 20;
        Exsist exist = {
            .temp_sensor  = true,
            .uv_sensor    = true
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
        std::array<uint16_t, glob::max_extantions+1> reset_hours;  // 8
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

    // колбеки для коилов далее
    constexpr auto coils_qty {2};
    // неудобно отлаживать, потому volatile
    volatile decltype(auto) modbus_slave = Modbus_slave<In_regs, Out_regs, coils_qty>::make <
          mcu::Periph::USART1
        , TX_slave
        , RX_slave
        , RTS_slave
    >(flash.modbus_address, flash.uart_set);

    struct {

        Register<10, Modbus_function::read_03, 0> uv_level;
        Register<10, Modbus_function::read_03, 1> temperature;

        Register<11, Modbus_function::read_04, 6> state_lamp_1;
        // Register<11, Modbus_function::read_04, 4> error_lamp_1;
        Register<12, Modbus_function::read_04, 6> state_lamp_2;
        Register<13, Modbus_function::read_04, 6> state_lamp_3;
        Register<14, Modbus_function::read_04, 6> state_lamp_4;
        Register<15, Modbus_function::read_04, 6> state_lamp_5;
        Register<16, Modbus_function::read_04, 6> state_lamp_6;
        Register<17, Modbus_function::read_04, 6> state_lamp_7;
        Register<18, Modbus_function::read_04, 6> state_lamp_8;
        Register<19, Modbus_function::read_04, 6> state_lamp_9;
        
        Register<11, Modbus_function::read_04, 9> lamp_hours_1;
        Register<12, Modbus_function::read_04, 9> lamp_hours_2;
        Register<13, Modbus_function::read_04, 9> lamp_hours_3;
        Register<14, Modbus_function::read_04, 9> lamp_hours_4;
        Register<15, Modbus_function::read_04, 9> lamp_hours_5;
        Register<16, Modbus_function::read_04, 9> lamp_hours_6;
        Register<17, Modbus_function::read_04, 9> lamp_hours_7;
        Register<18, Modbus_function::read_04, 9> lamp_hours_8;
        Register<19, Modbus_function::read_04, 9> lamp_hours_9;
        
        Register<11, Modbus_function::write_16, 0> on_off_1;
        Register<12, Modbus_function::write_16, 0> on_off_2;
        Register<13, Modbus_function::write_16, 0> on_off_3;
        Register<14, Modbus_function::write_16, 0> on_off_4;
        Register<15, Modbus_function::write_16, 0> on_off_5;
        Register<16, Modbus_function::write_16, 0> on_off_6;
        Register<17, Modbus_function::write_16, 0> on_off_7;
        Register<18, Modbus_function::write_16, 0> on_off_8;
        Register<19, Modbus_function::write_16, 0> on_off_9;

        Register<11, Modbus_function::write_16, 15> reset_resource_1;
        Register<12, Modbus_function::write_16, 15> reset_resource_2;
        Register<13, Modbus_function::write_16, 15> reset_resource_3;
        Register<14, Modbus_function::write_16, 15> reset_resource_4;
        Register<15, Modbus_function::write_16, 15> reset_resource_5;
        Register<16, Modbus_function::write_16, 15> reset_resource_6;
        Register<17, Modbus_function::write_16, 15> reset_resource_7;
        Register<18, Modbus_function::write_16, 15> reset_resource_8;
        Register<19, Modbus_function::write_16, 15> reset_resource_9;

        // Register<11, Modbus_function::read_03, 24> sec_to_start;
        // Register<11, Modbus_function::read_03, 23> power;

        // Register<11, Modbus_function::write_16, 23> set_power;
        // Register<11, Modbus_function::write_16, 24> set_sec_to_start;

        Register<11, Modbus_function::write_16, 13> service_1;
        Register<11, Modbus_function::write_16, 14> save_setting_1;
        Register<12, Modbus_function::write_16, 13> service_2;
        Register<12, Modbus_function::write_16, 14> save_setting_2;
        Register<13, Modbus_function::write_16, 13> service_3;
        Register<13, Modbus_function::write_16, 14> save_setting_3;
        Register<14, Modbus_function::write_16, 13> service_4;
        Register<14, Modbus_function::write_16, 14> save_setting_4;
        Register<15, Modbus_function::write_16, 13> service_5;
        Register<15, Modbus_function::write_16, 14> save_setting_5;
        Register<16, Modbus_function::write_16, 13> service_6;
        Register<16, Modbus_function::write_16, 14> save_setting_6;
        Register<17, Modbus_function::write_16, 13> service_7;
        Register<17, Modbus_function::write_16, 14> save_setting_7;
        Register<18, Modbus_function::write_16, 13> service_8;
        Register<18, Modbus_function::write_16, 14> save_setting_8;
        Register<19, Modbus_function::write_16, 13> service_9;
        Register<19, Modbus_function::write_16, 14> save_setting_9;
        
    } modbus_master_regs; 

    modbus_master_regs.reset_resource_1.disable = true;
    modbus_master_regs.reset_resource_2.disable = true;
    modbus_master_regs.reset_resource_3.disable = true;
    modbus_master_regs.reset_resource_4.disable = true;
    modbus_master_regs.reset_resource_5.disable = true;
    modbus_master_regs.reset_resource_6.disable = true;
    modbus_master_regs.reset_resource_7.disable = true;
    modbus_master_regs.reset_resource_8.disable = true;
    modbus_master_regs.reset_resource_9.disable = true;
    
    modbus_master_regs.service_1.disable = true;
    modbus_master_regs.save_setting_1.disable = true;
    modbus_master_regs.service_2.disable = true;
    modbus_master_regs.save_setting_2.disable = true;
    modbus_master_regs.service_3.disable = true;
    modbus_master_regs.save_setting_3.disable = true;
    modbus_master_regs.service_4.disable = true;
    modbus_master_regs.save_setting_4.disable = true;
    modbus_master_regs.service_5.disable = true;
    modbus_master_regs.save_setting_5.disable = true;
    modbus_master_regs.service_6.disable = true;
    modbus_master_regs.save_setting_6.disable = true;
    modbus_master_regs.service_7.disable = true;
    modbus_master_regs.save_setting_7.disable = true;
    modbus_master_regs.service_8.disable = true;
    modbus_master_regs.save_setting_8.disable = true;
    modbus_master_regs.service_9.disable = true;
    modbus_master_regs.save_setting_9.disable = true;

    decltype(auto) modbus_master = make_modbus_master <
          mcu::Periph::USART3
        , TX_master
        , RX_master
        , RTS_master
    > (100_ms, flash.uart_set_master, modbus_master_regs); // FIX flash.uart_set placeholder

    #define ADR(reg) GET_ADR(In_regs, reg)
    modbus_slave.outRegs.device_code       = 8;
    modbus_slave.outRegs.factory_number    = flash.factory_number;
    modbus_slave.outRegs.uart_set          = flash.uart_set;
    modbus_slave.outRegs.modbus_address    = flash.modbus_address;
    modbus_slave.outRegs.quantity          = flash.quantity;
    // for (auto i{0}; i < 10; i++)
    //     modbus_slave.outRegs.hours[i] = work_count.get_hours(i);
    modbus_slave.arInRegsMax[ADR(uart_set)]= 0x0F;
    modbus_slave.inRegsMin.modbus_address  = 1;
    modbus_slave.inRegsMax.modbus_address  = 255;
    modbus_slave.inRegsMin.qty_uv_lamps    = 1;
    modbus_slave.inRegsMax.qty_uv_lamps    = 9;
    

    auto up    = Button<Up>();
    auto down  = Button<Down>();
    auto enter = Tied_buttons(up, down);
    constexpr auto hd44780_pins = HD44780_pins<RS, RW, E, DB4, DB5, DB6, DB7>{};
    [[maybe_unused]] auto menu = Menu (
        hd44780_pins, up, down, enter
        , flash
        , modbus_slave.outRegs
        , modbus_master_regs
    );

    // алиасы
    auto& temperature = (uint16_t&)modbus_master_regs.temperature;
    auto& uv_level    = (uint16_t&)modbus_master_regs.uv_level;
    auto& uv_level_percent = modbus_slave.outRegs.uv_level;
    auto& work_flags = modbus_slave.outRegs.work_flags;


    auto& alarm_led = Pin::make<LED3, mcu::PinMode::Output>();

    auto overheat = Hysteresis(temperature, flash.temperature_recovery, flash.max_temperature);

    // UV control
    auto& uv     = Pin::make<UV_CTRL, mcu::PinMode::Output>();
    auto& uv_led = Pin::make<LED2   , mcu::PinMode::Output>();
    auto  uv_button = Button<UV_BTN>();

    auto on_uv = [&](bool on = true){
        if (on and not uv) 
            flash.count.on++;
        
        uv = uv_led = work_flags.uv_on = on;
        modbus_master_regs.on_off_1 = on;
        modbus_master_regs.on_off_2 = on;
        modbus_master_regs.on_off_3 = on;
        modbus_master_regs.on_off_4 = on;
        modbus_master_regs.on_off_5 = on;
        modbus_master_regs.on_off_6 = on;
        modbus_master_regs.on_off_7 = on;
        modbus_master_regs.on_off_8 = on;
        modbus_master_regs.on_off_9 = on;
    };

    uv_button.set_down_callback([&]{
        if (overheat) {
            work_flags.uv_started = false;
            return;
        }
        work_flags.uv_started ^= 1;
    });

    // US control
    auto& us     = Pin::make<US_CTRL, mcu::PinMode::Output>();
    auto& us_led = Pin::make<LED1   , mcu::PinMode::Output>();
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

    // управление по модбас
    modbus_slave.force_single_coil_05[0] = [&](bool on) {
        if (on and not overheat)
            work_flags.us_started = true;
        if (not on)
            work_flags.us_started = false;
    };
    modbus_slave.force_single_coil_05[1] = [&](bool on) {
        if (on and not overheat)
            work_flags.uv_started = true;
        if (not on)
            work_flags.uv_started = false;
    };

    std::array<bool, 9> state_lamps{false};

    // Определение плохих ламп
    Lamps::make(state_lamps, modbus_slave.outRegs.bad_lamps[0], flash.quantity.lamps);


    while (1) {

        modbus_slave.outRegs.hours[0] = modbus_master_regs.lamp_hours_1;
        modbus_slave.outRegs.hours[1] = modbus_master_regs.lamp_hours_2;
        modbus_slave.outRegs.hours[2] = modbus_master_regs.lamp_hours_3;
        modbus_slave.outRegs.hours[3] = modbus_master_regs.lamp_hours_4;
        modbus_slave.outRegs.hours[4] = modbus_master_regs.lamp_hours_5;
        modbus_slave.outRegs.hours[5] = modbus_master_regs.lamp_hours_6;
        modbus_slave.outRegs.hours[6] = modbus_master_regs.lamp_hours_7;
        modbus_slave.outRegs.hours[7] = modbus_master_regs.lamp_hours_8;
        modbus_slave.outRegs.hours[8] = modbus_master_regs.lamp_hours_9;

        state_lamps[0] = modbus_master_regs.state_lamp_1 != 7; // код рабочего состояния лампы
        state_lamps[1] = modbus_master_regs.state_lamp_2 != 7;
        state_lamps[2] = modbus_master_regs.state_lamp_3 != 7;
        state_lamps[3] = modbus_master_regs.state_lamp_4 != 7;
        state_lamps[4] = modbus_master_regs.state_lamp_5 != 7;
        state_lamps[5] = modbus_master_regs.state_lamp_6 != 7;
        state_lamps[6] = modbus_master_regs.state_lamp_7 != 7;
        state_lamps[7] = modbus_master_regs.state_lamp_8 != 7;
        state_lamps[8] = modbus_master_regs.state_lamp_9 != 7;

        modbus_master();
        modbus_slave([&](auto registr){
            static bool unblock = false;
            switch (registr) {
                case ADR(uart_set):
                    flash.uart_set
                        = modbus_slave.outRegs.uart_set
                        = modbus_slave.inRegs.uart_set;
                break;
                case ADR(modbus_address):
                    flash.modbus_address 
                        = modbus_slave.outRegs.modbus_address
                        = modbus_slave.inRegs.modbus_address;
                break;
                case ADR(password):
                    unblock = modbus_slave.inRegs.password == 208;
                break;
                case ADR(factory_number):
                    if (unblock) {
                        unblock = false;
                        flash.factory_number 
                            = modbus_slave.outRegs.factory_number
                            = modbus_slave.inRegs.factory_number;
                    }
                    unblock = true;
                break;
                case ADR(max_temperature):
                    flash.max_temperature 
                        = modbus_slave.outRegs.max_temperature
                        = modbus_slave.inRegs.max_temperature;
                break;
                case ADR(uv_level_min):
                    flash.uv_level_min 
                        = modbus_slave.outRegs.uv_level_min
                        = modbus_slave.inRegs.uv_level_min;
                break;
                // case ADR(qty_uv_lamps):
                //     flash.quantity.lamps 
                //     = modbus_slave.outRegs.quantity.lamps
                //     = modbus_slave.inRegs.qty_uv_lamps;
                // break;
                case ADR(uv_level_highest):
                    flash.quantity.lamps 
                    = modbus_slave.outRegs.uv_level_highest
                    = modbus_slave.inRegs.uv_level_highest;
                break;
                case ADR(reset_hours): // TODO без плат расширения
                // work_count.reset_by_mask(modbus_slave.inRegs.reset_hours[0]);
                break;
            } // switch
        }, [&](auto registr){}
        );

        overheat.set_min(flash.temperature_recovery);
        overheat.set_max(flash.max_temperature);
        modbus_slave.outRegs.temperature = temperature;
        modbus_slave.outRegs.max_temperature = flash.max_temperature;
        modbus_slave.outRegs.uv_level_min = flash.uv_level_min;
        modbus_slave.outRegs.uv_level = uv_level_percent;
        

        work_flags.overheat = overheat;

        if (flash.exist.uv_sensor and uv) {
            set_if_greater (&flash.uv_level_highest, uv_level);
            uv_level_percent = uv_level * 100 / flash.uv_level_highest;
            work_flags.uv_low_level = work_flags.uv_on and uv_level_percent < flash.uv_level_min;
        }

        on_us (work_flags.us_started and not overheat);
        on_uv (work_flags.uv_started and not overheat);

        // TODO пока без расширений
        work_flags.bad_lamps = work_flags.uv_on and modbus_slave.outRegs.bad_lamps[0];

        alarm_led = work_flags.is_alarm();


        __WFI();
    }
}
