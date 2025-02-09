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
#include "work_count.h"


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
        uint8_t baudrate  = 0;
        uint8_t parity    = 0; 
        uint8_t stop_bits = 0;
        uint8_t  max_temperature       = 55;
        uint8_t  uv_level_min          = 40;
        Quantity quantity = {
            .lamps = 5,
            .extantions = 0
        };
        uint8_t qty_lamps_uov          = 5;
        uint8_t qty_lamps_ext_1        = 0;
        uint8_t qty_lamps_ext_2        = 0;
        uint16_t uv_level_highest      = 0x0100;
        uint8_t  model_number          = 0;
        uint8_t  temperature_recovery  = 20;
        Exsist exist = {
            .temp_sensor  = true,
            .uv_sensor    = true,
            .dry_contacts = true
        };
        Count count = {
            .on        = 0,
            .reset_all = 0,
            .reset_one = 0,
            .reset_log = 0
        };
        bool automatic = false;
        bool reassignment = false;
    } flash;

    [[maybe_unused]] auto _ = Flash_updater<
          mcu::FLASH::Sector::_120
        , mcu::FLASH::Sector::_119
    >::make (&flash);

    struct Hold_regs {
        uint16_t modbus_address;         // 0
        uint16_t baudrate;               // 1
        uint16_t parity;                 // 2
        uint16_t stop_bits;              // 3
        uint16_t service;                // 4 code = 0x534D
        uint16_t save_set;               // 5 code = 0x5356
        uint16_t reset_max;              // 6 code = 0x524D or 21069
        uint16_t password;               // 7
        uint16_t factory_number;         // 8
        uint16_t max_temperature;        // 9
        uint16_t uv_level_min;           // 10
        // uint16_t qty_uv_lamps;           // 11 // FIX убрать для редактирования
        std::array<uint16_t, glob::max_extantions+1> reset_hours;  // 11
    }__attribute__((packed));

    struct Input_regs {
        uint16_t device_code;        // 0
        Flags    work_flags;         // 1
        uint16_t temperature;        // 2
        uint16_t uv_level;           // 3
        Quantity quantity;           // 4
        uint16_t uv_level_highest;   // 5
        uint16_t version_device;     // 6
        uint16_t version_lib;        // 7
        std::array<uint16_t, glob::max_extantions+1> bad_lamps; // 8
        std::array<uint16_t, glob::max_lamps> hours;       // 19
    }; // __attribute__((packed)); // TODO error: cannot bind packed field 

    // колбеки для коилов далее
    constexpr auto coils_qty {2};
    // неудобно отлаживать, потому volatile
    volatile decltype(auto) modbus_slave = Modbus_slave<Hold_regs, Input_regs, coils_qty>::make <
          mcu::Periph::USART1
        , TX_slave
        , RX_slave
        , RTS_slave
    >(flash.modbus_address, flash.uart_set);

    struct Reg {
        
        Register<1,  Modbus_function::read_03,  4> lamp_1;
        Register<1,  Modbus_function::read_03,  5> lamps_1;
        Register<1,  Modbus_function::read_03,  6> bad_lamps_1;
        Register<1,  Modbus_function::read_03,  7> hours_1;
        Register<1,  Modbus_function::write_16, 3> reset_hours_1;
        Register<1,  Modbus_function::write_16, 4> n_lamp_1;

        Register<2,  Modbus_function::read_03,  4> lamp_2;
        Register<2,  Modbus_function::read_03,  5> lamps_2;
        Register<2,  Modbus_function::read_03,  6> bad_lamps_2;
        Register<2,  Modbus_function::read_03,  7> hours_2;
        Register<2,  Modbus_function::write_16, 3> reset_hours_2;
        Register<2,  Modbus_function::write_16, 4> n_lamp_2;

        Register<10, Modbus_function::read_04,  0> uv_level;
        Register<10, Modbus_function::read_04,  1> temperature;

        Register<101, Modbus_function::read_03,  4> control_read;
        Register<101, Modbus_function::write_16, 4> control_write;
    } modbus_master_regs;

    decltype(auto) volatile modbus_master = make_modbus_master <
          mcu::Periph::USART3
        , TX_master
        , RX_master
        , RTS_master
    > (100_ms, flash.uart_set_master, modbus_master_regs); // FIX flash.uart_set placeholder


    volatile bool ext_1{flash.quantity.extantions};
    volatile bool ext_2{flash.quantity.extantions == 2};
    if(flash.quantity.extantions == 0) {
        modbus_master_regs.lamps_1.disable       = true;
        modbus_master_regs.bad_lamps_1.disable   = true;
        modbus_master_regs.lamp_1.disable        = true;
        modbus_master_regs.hours_1.disable       = true;
        modbus_master_regs.reset_hours_1.disable = true;
        modbus_master_regs.n_lamp_1.disable      = true;

        modbus_master_regs.lamps_2.disable       = true;
        modbus_master_regs.bad_lamps_2.disable   = true;
        modbus_master_regs.lamp_2.disable        = true;
        modbus_master_regs.hours_2.disable       = true;
        modbus_master_regs.reset_hours_2.disable = true;
        modbus_master_regs.n_lamp_2.disable      = true;
    } else if(flash.quantity.extantions == 1) {
        modbus_master_regs.lamp_2.disable        = true;
        modbus_master_regs.lamps_2.disable       = true;
        modbus_master_regs.bad_lamps_2.disable   = true;
        modbus_master_regs.hours_2.disable       = true;
        modbus_master_regs.reset_hours_2.disable = true;
        modbus_master_regs.n_lamp_2.disable      = true;
    } else {}

    // подсчёт часов работы
    auto work_count = Work_count{
          modbus_slave.input_Regs.bad_lamps[0]
        , modbus_slave.input_Regs.hours
        , flash.qty_lamps_uov
    };

    [[maybe_unused]] auto __ = Safe_flash_updater<
          mcu::FLASH::Sector::_89
        , mcu::FLASH::Sector::_115
    >::make (work_count.get_data());

    #define ADR(reg) GET_ADR(Hold_regs, reg)
    modbus_slave.input_Regs.device_code    = 8;
    modbus_slave.holdRegs.factory_number   = flash.factory_number;
    modbus_slave.holdRegs.modbus_address   = flash.modbus_address;
    modbus_slave.holdRegs.baudrate         = flash.baudrate;
    modbus_slave.holdRegs.parity           = flash.parity;
    modbus_slave.holdRegs.stop_bits        = flash.stop_bits;
    modbus_slave.input_Regs.quantity       = flash.quantity;
    for (auto i{0}; i < 10; i++)
        modbus_slave.input_Regs.hours[i] = work_count.get_hours(i);
    //modbus_slave.arholdRegsMax[ADR(uart_set)]= 0x0F;
    modbus_slave.holdRegsMin.modbus_address  = 1;
    modbus_slave.holdRegsMax.modbus_address  = 247;
    // modbus_slave.holdRegsMin.qty_uv_lamps    = 1;
    // modbus_slave.holdRegsMax.qty_uv_lamps    = 10;
    

    auto up    = Button<Up>();
    auto down  = Button<Down>();
    auto enter = Tied_buttons(up, down);
    constexpr auto hd44780_pins = HD44780_pins<RS, RW, E, DB4, DB5, DB6, DB7>{};
    [[maybe_unused]] auto menu = Menu (
        hd44780_pins, up, down, enter
        , flash
        , modbus_slave.input_Regs
        , work_count
        , modbus_master_regs
    );

    modbus_master_regs.control_write = 0;
    modbus_master_regs.control_read = 0;

    // алиасы
    auto& temperature = (uint16_t&)modbus_master_regs.temperature;
    auto& uv_level    = (uint16_t&)modbus_master_regs.uv_level;
    auto& uv_level_percent = modbus_slave.input_Regs.uv_level;
    auto& work_flags = modbus_slave.input_Regs.work_flags;
    // auto& control_read  = (Control&)modbus_master_regs.control_read;

    auto& alarm_led = Pin::make<LED3, mcu::PinMode::Output>();

    auto overheat = Hysteresis(temperature, flash.temperature_recovery, flash.max_temperature);

    // UV control
    auto& uv     = Pin::make<UV_CTRL, mcu::PinMode::Output>();
    auto& uv_led = Pin::make<LED2   , mcu::PinMode::Output>();
    auto  uv_button = Button<UV_BTN>();

    auto on_uv = [&](bool on = true){
        if (on and not uv) {
            flash.count.on++;
            work_count.start();
        }
        if (not on and uv)
            work_count.stop();
        uv = uv_led = work_flags.uv_on = on;
    };

    uv_button.set_down_callback([&]{
        if (overheat) {
            work_flags.uv_started = false;
            return;
        }
        if(not work_flags.distance)
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
        if (not work_flags.distance)
            work_flags.us_started ^= 1;
    });

    // управление по модбас
    modbus_slave.force_single_coil_05[0] = [&](bool on) {
        if (on and not overheat and not work_flags.distance)
            work_flags.us_started = true;
        if (not on)
            work_flags.us_started = false;
    };
    modbus_slave.force_single_coil_05[1] = [&](bool on) {
        if (on and not overheat and not work_flags.distance)
            work_flags.uv_started = true;
        if (not on)
            work_flags.uv_started = false;
    };

    // Определение плохих ламп
    // if(flash.reassignment) {
    //     Lamps::make<
    //         EPRA1,EPRA2,EPRA3,EPRA4,EPRA5,EPRA6,EPRA7,EPRA8,EPRA9
    //     >(modbus_slave.input_Regs.bad_lamps[0], flash.qty_lamps_uov);
    // } else {
        Lamps::make<
            EPRA1,EPRA2,EPRA3,EPRA4,EPRA5,EPRA6,EPRA7,EPRA8,EPRA9,EPRA10
        >(modbus_slave.input_Regs.bad_lamps[0], flash.qty_lamps_uov);
    // }

    uint16_t n_lamp_1{0};
    uint16_t n_lamp_2{0};

    modbus_master_regs.reset_hours_1 = 0;
    modbus_master_regs.reset_hours_2 = 0;

    bool service_mode{false};
    bool save_mode{false};

    while (1) {

        ext_1 = flash.quantity.extantions;
        ext_2 = flash.quantity.extantions == 2;

        modbus_master_regs.lamps_1.disable = not ext_1;

        if (not modbus_master_regs.lamps_1.disable or not modbus_master_regs.lamps_2.disable) {
            flash.qty_lamps_ext_1 = ext_1 ? modbus_master_regs.lamps_1 : 0;
            flash.qty_lamps_ext_2 = ext_2 ? modbus_master_regs.lamps_2 : 0;
            flash.quantity.lamps = flash.qty_lamps_uov + flash.qty_lamps_ext_1 + flash.qty_lamps_ext_2;
            // modbus_slave.input_Regs.quantity = flash.quantity;
            if(ext_1) {
                modbus_master_regs.lamps_1.disable = flash.qty_lamps_ext_1  > 0;
            } 
            if(ext_2) {
                modbus_master_regs.lamps_1.disable = flash.qty_lamps_ext_1  > 0;
                modbus_master_regs.lamps_2.disable = flash.qty_lamps_ext_2  > 0;
            }
        } else {
            flash.quantity.lamps = flash.qty_lamps_uov;
            // modbus_slave.input_Regs.quantity = flash.quantity;
        }

        if (ext_2) {

            if (modbus_master_regs.n_lamp_1 == modbus_master_regs.lamp_1) {
                modbus_slave.input_Regs.hours[flash.qty_lamps_uov + n_lamp_1++] = modbus_master_regs.hours_1;
                n_lamp_1 = n_lamp_1 > (flash.qty_lamps_ext_1 - 1) ? 0 : n_lamp_1;
                modbus_master_regs.n_lamp_1 = n_lamp_1;
            }

            if (modbus_master_regs.n_lamp_2 == modbus_master_regs.lamp_2) {
                modbus_slave.input_Regs.hours[flash.qty_lamps_uov + flash.qty_lamps_ext_1 + n_lamp_2++] = modbus_master_regs.hours_2;
                n_lamp_2 = n_lamp_2 > (flash.qty_lamps_ext_2 - 1) ? 0 : n_lamp_2;
                modbus_master_regs.n_lamp_2 = n_lamp_2;
            }
        }
        if (ext_1) {

            if (modbus_master_regs.n_lamp_1 == modbus_master_regs.lamp_1) {
                modbus_slave.input_Regs.hours[flash.qty_lamps_uov + n_lamp_1++] = modbus_master_regs.hours_1;
                n_lamp_1 = n_lamp_1 > (flash.qty_lamps_ext_1 - 1) ? 0 : n_lamp_1;
                modbus_master_regs.n_lamp_1 = n_lamp_1;
            }
        }

        modbus_slave.input_Regs.bad_lamps[1] = modbus_master_regs.bad_lamps_1;
        modbus_slave.input_Regs.bad_lamps[2] = modbus_master_regs.bad_lamps_2;

        modbus_master();

        modbus_slave([&](auto registr){
            static bool unblock = false;
            switch (registr) {
                case ADR(modbus_address):
                    if (service_mode) {
                        modbus_slave.holdRegs.modbus_address 
                            = modbus_slave.holdRegs.modbus_address;
                    }
                break;
                case ADR(baudrate):
                    if (service_mode) {
                       modbus_slave.holdRegs.baudrate = modbus_slave.holdRegs.baudrate;
                    }
                break;
                case ADR(parity):
                    if (service_mode) {
                       modbus_slave.holdRegs.parity = modbus_slave.holdRegs.parity;
                    }
                break;
                case ADR(stop_bits):
                    if (service_mode) {
                       modbus_slave.holdRegs.stop_bits = modbus_slave.holdRegs.stop_bits;
                    }
                break;
                case ADR(service):
                    if(modbus_slave.holdRegs.service == SERVICE_CODE) {
                       modbus_slave.holdRegs.service = SERVICE_CODE;
                       service_mode = true;
                    } else if(modbus_slave.holdRegs.service == 0){
                       modbus_slave.holdRegs.service = 0;
                       service_mode = false;
                    }
                break;
                case ADR(save_set):
                    if(modbus_slave.holdRegs.save_set == SAVE_CODE and service_mode) {
                       save_mode = true;
                    }
                break;
                case ADR(password):
                    unblock = modbus_slave.holdRegs.password == 1207;
                break;
                case ADR(factory_number):
                    if (unblock) {
                        unblock = false;
                        flash.factory_number 
                            = modbus_slave.holdRegs.factory_number;
                    }
                    // unblock = true;
                break;
                case ADR(max_temperature):
                    if (service_mode) {
                        flash.max_temperature = modbus_slave.holdRegs.max_temperature;
                    }
                break;
                case ADR(uv_level_min):
                    if (service_mode) {
                        flash.uv_level_min = modbus_slave.holdRegs.uv_level_min;
                    }
                break;
                // case ADR(qty_uv_lamps):
                //     flash.quantity.lamps 
                //     = modbus_slave.input_Regs.quantity.lamps
                //     = modbus_slave.holdRegs.qty_uv_lamps;
                // break;
                case ADR(reset_max):
                    if(modbus_slave.holdRegs.reset_max == RESET_CODE) {
                        flash.uv_level_highest = 0;
                        modbus_slave.holdRegs.reset_max = 0;
                    } else {
                        modbus_slave.holdRegs.reset_max = 0xFFFF;
                    }
                break;
                case ADR(reset_hours): // TODO без плат расширения
                    work_count.reset_by_mask(modbus_slave.holdRegs.reset_hours[0]);
                break;
            } // switch

            if(save_mode) {
                if(flash.modbus_address != modbus_slave.holdRegs.modbus_address) {
                    flash.modbus_address = modbus_slave.holdRegs.modbus_address;
                }

                if(flash.baudrate != modbus_slave.holdRegs.baudrate) {
                    flash.baudrate = modbus_slave.holdRegs.baudrate;
                    if (modbus_slave.holdRegs.baudrate == 0)
                        flash.uart_set.baudrate = USART::Baudrate::BR9600;
                    else if (modbus_slave.holdRegs.baudrate == 1)
                        flash.uart_set.baudrate = USART::Baudrate::BR14400;
                    else if (modbus_slave.holdRegs.baudrate == 2)
                        flash.uart_set.baudrate = USART::Baudrate::BR19200;
                    else if (modbus_slave.holdRegs.baudrate == 3)
                        flash.uart_set.baudrate = USART::Baudrate::BR28800;
                    else if (modbus_slave.holdRegs.baudrate == 4)
                        flash.uart_set.baudrate = USART::Baudrate::BR38400;
                    else if (modbus_slave.holdRegs.baudrate == 5)
                        flash.uart_set.baudrate = USART::Baudrate::BR57600;
                    else if (modbus_slave.holdRegs.baudrate == 6)
                        flash.uart_set.baudrate = USART::Baudrate::BR76800;
                    else if (modbus_slave.holdRegs.baudrate == 7)
                        flash.uart_set.baudrate = USART::Baudrate::BR115200;
                }
                if(flash.parity != modbus_slave.holdRegs.parity) {
                    flash.parity = modbus_slave.holdRegs.parity;
                    if (modbus_slave.holdRegs.parity == 0) {
                        flash.uart_set.parity_enable = false;
                        flash.uart_set.parity = USART::Parity::even;
                    } else if (modbus_slave.holdRegs.parity == 1) {
                        flash.uart_set.parity_enable = true;
                        flash.uart_set.parity = USART::Parity::even;
                    } else if (modbus_slave.holdRegs.parity == 2) {
                        flash.uart_set.parity_enable = true;
                        flash.uart_set.parity = USART::Parity::odd;
                    }
                }
                if(flash.stop_bits != modbus_slave.holdRegs.stop_bits) {
                    flash.stop_bits = modbus_slave.holdRegs.stop_bits;
                    if (modbus_slave.holdRegs.stop_bits == 0)
                        flash.uart_set.stop_bits = USART::StopBits::_1;
                    else if (modbus_slave.holdRegs.stop_bits == 1)
                        flash.uart_set.stop_bits = USART::StopBits::_2;
                }

                if(flash.max_temperature != modbus_slave.holdRegs.max_temperature) {
                    flash.max_temperature = modbus_slave.holdRegs.max_temperature;
                }

                if(flash.uv_level_min != modbus_slave.holdRegs.uv_level_min){
                    flash.uv_level_min = modbus_slave.holdRegs.uv_level_min;
                }

                save_mode = false;
                modbus_slave.holdRegs.save_set = 0;
            }
        }, [&](auto registr){});

        overheat.set_min(flash.temperature_recovery);
        overheat.set_max(flash.max_temperature);
        modbus_slave.input_Regs.temperature = temperature;
        modbus_slave.holdRegs.max_temperature = flash.max_temperature;
        modbus_slave.holdRegs.uv_level_min = flash.uv_level_min;
        modbus_slave.input_Regs.uv_level = uv_level_percent;
        modbus_slave.input_Regs.quantity = flash.quantity;
        modbus_slave.input_Regs.uv_level_highest = flash.uv_level_highest;
        

        work_flags.overheat = overheat;

        if (flash.exist.uv_sensor and uv) {
            set_if_greater (&flash.uv_level_highest, uv_level);
            uv_level_percent = uv_level * 100 / flash.uv_level_highest;
            work_flags.uv_low_level = work_flags.uv_on and uv_level_percent < flash.uv_level_min;
        } else {
            uv_level_percent = 0;
            work_flags.uv_low_level = false;
        }

        if(flash.exist.dry_contacts){
            modbus_master_regs.control_read.disable  = false;
            modbus_master_regs.control_write.disable = false;
            if(work_flags.distance) {
            if (modbus_master_regs.control_read & 0b001) {
                work_flags.us_started = 1;
            } else {
                work_flags.us_started = 0;
            }

            if (modbus_master_regs.control_read & 0b010) {
                work_flags.uv_started = 1;
            } else {
                work_flags.uv_started = 0;
            }
            }

            if (work_flags.uv_on) {
                modbus_master_regs.control_write |= (1 << 9);
            }else {
                modbus_master_regs.control_write &= ~(1 << 9);
            }
            if (work_flags.us_on) {
                modbus_master_regs.control_write |= (1 << 8);
            } else {
                modbus_master_regs.control_write &= ~(1 << 8);
            }
            if (work_flags.is_alarm()) {
                modbus_master_regs.control_write |= (1 << 10);
            } else {
                modbus_master_regs.control_write &= ~(1 << 10);
            }
            
            
        } else {
            modbus_master_regs.control_read.disable  = true;
            modbus_master_regs.control_write.disable = true;
         
        }

        work_flags.distance = modbus_master_regs.control_read & 0b100;

        work_flags.uv_low_level = work_flags.uv_on and uv_level_percent < flash.uv_level_min;


        on_us (work_flags.us_started and not overheat);
        on_uv (work_flags.uv_started and not overheat);

        // TODO пока без расширений
        work_flags.bad_lamps = work_flags.uv_on and (modbus_slave.input_Regs.bad_lamps[0] or modbus_slave.input_Regs.bad_lamps[1] or modbus_slave.input_Regs.bad_lamps[2]);

        alarm_led = work_flags.is_alarm();


        __WFI();
    }
}
