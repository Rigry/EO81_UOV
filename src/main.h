#pragma once

#define ADR(reg) GET_ADR(In_regs, reg)

#include "bit_set.h"
#include "pin.h"
#include "buttons.h"
#include "modbus_slave.h"
#include "modbus_master.h"
#include "hysteresis.h"


using EPRA1  = mcu::PA0;        using E   = mcu::PB3;       
using EPRA2  = mcu::PA1;        using RW  = mcu::PD2;       
using EPRA3  = mcu::PA2;        using RS  = mcu::PC12;      
using EPRA4  = mcu::PA3;        using DB4 = mcu::PB4;       
using EPRA5  = mcu::PA4;        using DB5 = mcu::PB5;
using EPRA6  = mcu::PA5;        using DB6 = mcu::PB6;    
using EPRA7  = mcu::PA6;        using DB7 = mcu::PB7;
using EPRA8  = mcu::PA7;
using EPRA9  = mcu::PC4;
using EPRA10 = mcu::PC5;

using BTN_UP   = mcu::PB8;      using LED1 = mcu::PB12;
using BTN_DOWN = mcu::PB9;      using LED2 = mcu::PC6;
using BTN_UF   = mcu::PB1;      using LED3 = mcu::PC7;
using BTN_UZG  = mcu::PB0;      using LED4 = mcu::PA12;
using R_UF     = mcu::PB10;     using LED5 = mcu::PC9;
using R_UZG    = mcu::PB11;

using TX_master  = mcu::PC10;   using TX_slave  = mcu::PA9;
using RX_master  = mcu::PC11;   using RX_slave  = mcu::PA10;
using RTS_master = mcu::PA15;   using RTS_slave = mcu::PA8;
using LED_master = mcu::PC9;    using LED_slave = mcu::PA12;

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
      uint16_t uv_level_min          = 40;  
      uint16_t qty_uv_lamps          = 5;
      uint16_t uv_level_highest      = 0x0100;
      uint16_t model_number          = 0;
      uint16_t temperature_recovery  = 20; 
   } flash;

// void process()
// {
//    

//    struct In_regs {
   
//       UART::Settings uart_set;         // 0
//       uint16_t modbus_address;         // 1
//       uint16_t password;               // 2
//       uint16_t factory_number;         // 3
//       uint16_t max_temperature;        // 4
//       uint16_t uv_level_min;           // 5
//       uint16_t qty_uv_lamps;           // 6
//       uint16_t uv_level_highest;       // 7
//       std::array<uint16_t, 112> hours; // 8
   
//    }__attribute__((packed));


//    struct Out_regs {
   
//       uint16_t device_code;            // 0
//       uint16_t factory_number;         // 1
//       UART::Settings uart_set;         // 2
//       uint16_t modbus_address;         // 3
//       bool     work_flags;             // 4
//       uint16_t temperature;            // 5
//       uint16_t max_temperature;        // 6
//       uint16_t uv_level;               // 7
//       uint16_t uv_level_min;           // 8
//       uint16_t qty_uv_lamps;           // 9
//       uint16_t uv_level_highest;       // 10
//       Bit_set<112> lamp;               // 11
//       std::array<uint16_t, 112> hours; // 18

//    }__attribute__((packed));

//    mcu::Register<1,  0> qty_lamps_1;
//    mcu::Register<2,  0> qty_lamps_2;
//    mcu::Register<3,  0> qty_lamps_3;
//    mcu::Register<4,  0> qty_lamps_4;
//    mcu::Register<5,  0> qty_lamps_5;
//    mcu::Register<6,  0> qty_lamps_6;
//    mcu::Register<7,  0> qty_lamps_7;
//    mcu::Register<8,  0> qty_lamps_8;
//    mcu::Register<9,  0> qty_lamps_9;
//    mcu::Register<10, 0> qty_lamps_10;
//    mcu::Register<11, 0> qty_lamps_11;
//    mcu::Register<12, 0> qty_lamps_12;
//    mcu::Register<13, 0> qty_lamps_13;
//    mcu::Register<14, 0> qty_lamps_14;
//    mcu::Register<15, 0> qty_lamps_15;

//    mcu::Register<1,  1, Bit_set<10>> bad_lamps_1;
//    mcu::Register<2,  1, Bit_set<10>> bad_lamps_2;
//    mcu::Register<3,  1, Bit_set<10>> bad_lamps_3;
//    mcu::Register<4,  1, Bit_set<10>> bad_lamps_4;
//    mcu::Register<5,  1, Bit_set<10>> bad_lamps_5;
//    mcu::Register<6,  1, Bit_set<10>> bad_lamps_6;
//    mcu::Register<7,  1, Bit_set<10>> bad_lamps_7;
//    mcu::Register<8,  1, Bit_set<10>> bad_lamps_8;
//    mcu::Register<9,  1, Bit_set<10>> bad_lamps_9;
//    mcu::Register<10, 1, Bit_set<10>> bad_lamps_10;
//    mcu::Register<11, 1, Bit_set<10>> bad_lamps_11;
//    mcu::Register<12, 1, Bit_set<10>> bad_lamps_12;
//    mcu::Register<13, 1, Bit_set<10>> bad_lamps_13;
//    mcu::Register<14, 1, Bit_set<10>> bad_lamps_14;
//    mcu::Register<15, 1, Bit_set<10>> bad_lamps_15;
   
//    mcu::Register<16, 0> uv_level;
//    mcu::Register<16, 1> temperature;

//    decltype(auto) master = mcu::make<mcu::Periph::USART3, TX_master, RX_master, RTS_master, LED_master>
//       (
//          50,           flash.uart_set,
//          qty_lamps_1,  bad_lamps_1,
//          qty_lamps_2,  bad_lamps_2,
//          qty_lamps_3,  bad_lamps_3,
//          qty_lamps_4,  bad_lamps_4,
//          qty_lamps_5,  bad_lamps_5,
//          qty_lamps_6,  bad_lamps_6,
//          qty_lamps_7,  bad_lamps_7,
//          qty_lamps_8,  bad_lamps_8,
//          qty_lamps_9,  bad_lamps_9,
//          qty_lamps_10, bad_lamps_10,
//          qty_lamps_11, bad_lamps_11,
//          qty_lamps_12, bad_lamps_12,
//          qty_lamps_13, bad_lamps_13,
//          qty_lamps_14, bad_lamps_14,
//          qty_lamps_15, bad_lamps_15,
//          uv_level,     temperature
//       );
//    decltype(auto) slave = Modbus_slave<In_regs, Out_regs>::
//          make<mcu::Periph::USART1, TX_slave, RX_slave, RTS_slave, LED_slave>(flash.modbus_address, flash.uart_set);

//    slave.outRegs.device_code       = 8; 
//    slave.outRegs.factory_number    = flash.factory_number;
//    slave.outRegs.uart_set          = flash.uart_set;
//    slave.outRegs.modbus_address    = flash.modbus_address;
//    slave.arInRegsMax[ADR(uart_set)]= 0b11111111;
//    slave.inRegsMin.modbus_address  = 1;
//    slave.inRegsMax.modbus_address  = 255;
//    slave.inRegsMin.qty_uv_lamps    = 0;
//    slave.inRegsMax.qty_uv_lamps    = 10;

//    String_buffer lcd;
//    HD44780::make<RS, RW, E, DB4, DB5, DB6, DB7>(lcd.get_buffer());

//    // slave reaction
//    slave ([&](uint16_t registrAddress) {
//          static bool unblock = false;
//       switch (registrAddress) {
//          case ADR(uart_set):
//             flash.uart_set
//                = slave.outRegs.uart_set
//                = slave.inRegs.uart_set;
//          break;
//          case ADR(modbus_address):
//             flash.modbus_address 
//                = slave.outRegs.modbus_address
//                = slave.inRegs.modbus_address;
//          break;
//          case ADR(password):
//             unblock = slave.inRegs.password == 208;
//          break;
//          case ADR(factory_number):
//             if (unblock) {
//                unblock = false;
//                flash.factory_number 
//                   = slave.outRegs.factory_number
//                   = slave.inRegs.factory_number;
//             }
//             unblock = true;
//          break;
//          case ADR (max_temperature):
//             flash.max_temperature
//             = slave.outRegs.max_temperature
//             = slave.inRegs.max_temperature;
//          break;
//          case ADR (uv_level_min):
//             flash.uv_level_min
//             = slave.outRegs.uv_level_min
//             = slave.inRegs.uv_level_min;
//          break;
//          case ADR (qty_uv_lamps):
//             flash.qty_uv_lamps
//             = slave.outRegs.qty_uv_lamps
//             = slave.inRegs.qty_uv_lamps;
//          break;
//          case ADR (uv_level_highest):
//             flash.uv_level_highest
//             = slave.outRegs.uv_level_highest
//             = slave.inRegs.uv_level_highest;
//          break;
//          case ADR (hours):

//          break;
//       }

//    });

   // auto& lamp_1  = Pin::make<EPRA1,  PinMode::Input>();
   // auto& lamp_2  = Pin::make<EPRA2,  PinMode::Input>();
   // auto& lamp_3  = Pin::make<EPRA3,  PinMode::Input>();
   // auto& lamp_4  = Pin::make<EPRA4,  PinMode::Input>();
   // auto& lamp_5  = Pin::make<EPRA5,  PinMode::Input>();
   // auto& lamp_6  = Pin::make<EPRA6,  PinMode::Input>();
   // auto& lamp_7  = Pin::make<EPRA7,  PinMode::Input>();
   // auto& lamp_8  = Pin::make<EPRA8,  PinMode::Input>();
   // auto& lamp_9  = Pin::make<EPRA9,  PinMode::Input>();
   // auto& lamp_10 = Pin::make<EPRA10, PinMode::Input>();
   
   // auto& us = Pin::make<R_UZG, PinMode::Output>();
   // auto& uv = Pin::make<R_UF,  PinMode::Output>();

   // auto& up    = Button::make<BTN_UP  >();
   // auto& down  = Button::make<BTN_DOWN>();
   // auto& us_on = Button::make<BTN_UZG>(); 
   // auto& uv_on = Button::make<BTN_UF >();

   // auto& uv_led = Pin::make<LED1, PinMode::Output>();
   // auto& us_led = Pin::make<LED2, PinMode::Output>();
   // auto& alarm  = Pin::make<LED3, PinMode::Output>();

   // Hysteresis<uint16_t> heat;
// }
